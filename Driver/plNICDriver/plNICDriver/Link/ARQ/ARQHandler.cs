using Microsoft.Extensions.Logging;
using plNICDriver.Link.Framing;

namespace plNICDriver.Link.ARQ
{
	// This is basicly selective repeate ARQ with window len of 1
	// We use task instead of thread because threads are heavier than task lots of overhead
	public class ARQHandler
	{
		private static readonly byte CRC_LEN = 2;
		public delegate void onRxARQFrame(Frame.FrameType ft, byte txId, byte rxId, byte[] payload);

		private int _numRetries;
		private int _timeOut;

		private WindowElement[] _winElements;
		private onRxARQFrame _onRx;
		private FramingHandler _txer;

		private ILogger<ARQHandler> _lg;

		public byte ACKResponderId { get; set; }

		private bool IsCrcValid(byte[] appended)
		{
			if (appended.Length < 3)
				return false;

			ExtractCRC(appended, out byte[] payload, out byte[] claimedCrc);
			CalcCrc16(payload, out byte[] currCrc);

			var isValid = !claimedCrc.Where((t, i) => t != currCrc[i]).Any();
			if (!isValid)
				_lg.LError($"crc claimed: {claimedCrc.ToStr()}, " +
							$"crc current: {currCrc.ToStr()}");

			return isValid;
		}

		private static void AppendCRCField(in byte[] raw, out byte[] appended)
		{
			appended = new byte[raw.Length + CRC_LEN];
			CalcCrc16(raw, out byte[] crcField);
			Array.Copy(raw, 0, appended, 0, raw.Length);
			Array.Copy(crcField, 0, appended, raw.Length, CRC_LEN);
		}

		private void ExtractCRC(in byte[] raw, out byte[] payload, out byte[] crc)
		{
			var len = raw.Length;
			if (len <= CRC_LEN)
				_lg.LError("SDU payload len must be > 2");

			payload = new byte[len - CRC_LEN];
			crc = new byte[CRC_LEN];

			Array.Copy(raw, 0, payload, 0, len - CRC_LEN);
			Array.Copy(raw, len - CRC_LEN, crc, 0, CRC_LEN);
		}

		// Instead of using ushort in all around the library, using byte[] will make it really simple to change
		// crc field. e.g. to 32 bit crc
		private static void CalcCrc16(byte[] dat, out byte[] crcBytes)
		{
			ushort crcField = NullFX.CRC.Crc16.ComputeChecksum(NullFX.CRC.Crc16Algorithm.Modbus, dat);
			crcBytes = BitConverter.GetBytes(crcField);
		}

		public ARQHandler(ILoggerFactory factory, onRxARQFrame onRx, FramingHandler txer, int numRetries, int timeOut, int numWidBits)
		{
			_numRetries = numRetries;
			_timeOut = timeOut;
			_onRx = onRx;
			_txer = txer;
			ACKResponderId = IDAllocation.IDAllocator.NO_ID;

			_lg = factory.CreateLogger<ARQHandler>();
			
			int numElems = (int)(Math.Pow(2, numWidBits));
			_winElements = new WindowElement[numElems];
			for (int i = 0; i < _winElements.Length; i++)
				_winElements[i] = new WindowElement(_txer);
			
			_lg.LogTrace($"ARQHandler created, #wids {numElems}");
		}

		public async void OnRxFrame(Frame.FrameType ft, int txid, int rxid, byte[] payload, int wid)
		{
			if (ft == Frame.FrameType.ACK || ft == Frame.FrameType.NCK) // ACK if validated then Turn the timer off and set the wid free
			{                                                           // This it is not piggy backing so if else continues 
				_winElements[wid].Check(ft, (byte)txid, (byte)rxid, (byte)wid);
				return;
			} // There is not CRC for it and instead header checksum which is done by framing component

			bool isFrameValid = IsCrcValid(payload);
			if (isFrameValid)
			{
				ExtractCRC(payload, out byte[] pldFld, out byte[] crcFld);
				_onRx(ft, ((byte)txid), ((byte)txid), pldFld);
			}

			// Sending ACK/NCK for SDU only
			if (ft == Frame.FrameType.SDU)
			{
				_lg.LDebug($"Sending ACK/NCK Stat: {isFrameValid} for {wid}");

				if (isFrameValid)
					await _txer.SendFrame(Frame.FrameType.ACK, ((byte)rxid), ((byte)txid), ((byte)wid), null);

				else
					await _txer.SendFrame(Frame.FrameType.NCK, ((byte)rxid), ((byte)txid), ((byte)wid), null);

			}
		}

		public async Task<bool> SendARQedFrame(Frame.FrameType ft, byte txId, byte rxId, byte[] payload)
		{
			if (ft == Frame.FrameType.SDU && payload.Length > 0)
			{
				var frees = _winElements.Where((winEl) => { return !winEl.filled; });
				if (!frees.Any())
					return false;
				var winEl = frees.First();

				AppendCRCField(payload, out byte[] appended);
				winEl.Fill(ft, txId, rxId, appended);

				var rez = await winEl.StartARQ(_numRetries, _timeOut);
				return rez;
			} 
			else 
				return false;
		}

		public void Dispose()
		{
			throw new NotImplementedException();
		}
	}
}
