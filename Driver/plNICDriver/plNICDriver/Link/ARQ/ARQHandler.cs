using plNICDriver.Link.Framing;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace plNICDriver.Link.ARQ
{
	// This is basicly selective repeate ARQ with window len of 1
	public class ARQHandler
	{
		struct WinElement
		{
			private static byte counter = 0;
			public bool filled;
			public byte wid;
			public byte rxid;
			public int numRetries;
			public DateTime timeStamp;

			public WinElement()
			{
				filled = false;
				wid = counter++;
				rxid = IDAllocation.IDAllocator.NO_ID;
				timeStamp = new DateTime();
				numRetries = 0;
			}
		}

		public static readonly byte CRC_LEN = 2;

		private int numRetries;
		private int timeOut;

		WinElement[] winElements;

		public static bool CheckCRC(byte[] payload)
		{
			var len = payload.Length;
			byte[] data = ExtractCRC(payload);

			ushort crcField = BitConverter.ToUInt16(payload, len - CRC_LEN);
			CalcCrc16(data, out ushort currCrc);

			return currCrc == crcField;
		}

		public static byte[] appendCRCField(byte[] bytes)
		{
			byte[] data = new byte[bytes.Length+CRC_LEN];
			CalcCrc16(bytes, out ushort crc);
			Array.Copy(bytes, 0, data, 0, bytes.Length);
			byte[] crcField = BitConverter.GetBytes(crc);
			Array.Copy(crcField, 0, data, bytes.Length, CRC_LEN);
			return data;
		}

		public static byte[] ExtractCRC(byte[] payload)
		{
			var len = payload.Length;
			if (len < CRC_LEN)
				throw new ArgumentException("To check CRC payload must be longer than 2");

			byte[] data = new byte[len - CRC_LEN];
			Array.Copy(payload, 0, data, 0, len - CRC_LEN);
			return data;
		}

		private static void CalcCrc16(byte[] dat, out ushort crc)
		{
			crc = NullFX.CRC.Crc16.ComputeChecksum(NullFX.CRC.Crc16Algorithm.Modbus, dat);
			//Console.WriteLine("datCrc CRC: {0:X4}", crc);
		}

		public ARQHandler(int numRetries, int timeOut, int numWidBits)
		{
			this.numRetries = numRetries;
			this.timeOut = timeOut;
			int numElems = (int)(Math.Pow(2, numWidBits));
			winElements = new WinElement[numElems];
			for (int i = 0; i < winElements.Length; i++)
				winElements[i] = new WinElement();
		}

		public void handleAckOfWid(in byte txid, in byte wid)
		{
			for (int i = 0; i < winElements.Length; i++)
				if (winElements[i].filled && winElements[i].wid == wid)
					if (winElements[i].rxid == txid || winElements[i].rxid == IDAllocation.IDAllocator.BROADCAST_ID)
						winElements[i].filled = false;
					else
						Console.WriteLine("Shoule not reach here in handleAckOfWid(...), incorrect txid");

			//Console.WriteLine("Shoule not reach here in handleAckOfWid(...), maybe ack after timeout");
		}

		public bool GetFreeWid(in byte rxid, out byte wid)
		{
			for (int i = 0; i < winElements.Length; i++)
				if (!winElements[i].filled)
				{
					ref WinElement windowIdElem = ref winElements[i];
					wid = windowIdElem.wid;
					windowIdElem.filled = true;
					windowIdElem.rxid = rxid;
					windowIdElem.timeStamp = DateTime.UtcNow;
					windowIdElem.numRetries = 1;
					return true;
				}
		
			wid = 0;
			return false;
		}

		public bool IsWidFree(byte wid)
		{
			for (int i = 0; i < winElements.Length; i++)
				if (!winElements[i].filled && winElements[i].wid == wid)
					return true;
			return false;
		}

		public void SetWidFree(byte wid)
		{
			for (int i = 0; i < winElements.Length; i++)
				if (winElements[i].filled && winElements[i].wid == wid)
					winElements[i].filled = false;
		}

		public bool IsMaxRetryExceeded(byte wid)
		{
			for (int i = 0; i < winElements.Length; i++)
				if (winElements[i].filled && winElements[i].wid == wid)
					if (winElements[i].numRetries > numRetries)
						return true;
			return false;
		}

		public bool GetNextTimedoutWid(in byte startwid, out byte tmoutwid)
		{
			for (int i = startwid; i < winElements.Length; i++)
				if (winElements[i].filled)
				{
					long timeSpanned = (long)(DateTime.UtcNow.Subtract(winElements[i].timeStamp).TotalMilliseconds);
					if (timeSpanned > timeOut)
					{
						winElements[i].numRetries += 1;
						winElements[i].timeStamp = DateTime.UtcNow;
						tmoutwid = winElements[i].wid;
						return true;
					}
				}

			tmoutwid = 0;
			return false;
		}

		internal void SetWidToTimeout(byte txid)
		{
			for (int i = 0; i < winElements.Length; i++)
				if (winElements[i].filled)
					winElements[i].timeStamp = DateTime.UtcNow.AddMilliseconds(timeOut * -2);
		}
	}
}
