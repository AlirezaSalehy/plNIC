using Microsoft.Extensions.Logging;
using plNICDriver.Link.ARQ;
using plNICDriver.Link.Framing;
using plNICDriver.Link.IDAllocation;
using plNICDriver.Phy;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;


namespace plNICDriver.Link
{
	//` ![](9CAD81B7C71C743FEFFDA0569A3ADF11.png;;;0.02599,0.02932)

	public class Link : IDisposable
	{
		// TODO: Next we should complete receiver task so that after [discarding or] receiving
		// TODO: Piggy backing by making it possible to have multiple packet types at same time
		// TODO: SET A TIMER IF AFTER IPALLOCATION CONFLICT APPEARED
		// Calls related callback of ARQ or IdAlloc

		public enum Status
		{
			Success,
			Err_NoFreeWid,
			Err_MaxRetryExceed,
			Err_GenFailure,
			Err_NoFreeFrame,
			Err_NoOtherplNIC,
			Err_NotInitialized
		}


		private static readonly int NUM_FRAMES = 5;
		private static readonly int NUM_WIDS = 2;

		public delegate void OnRx(byte txid, byte[] pcktContent);

		private bool _terminate;

		private byte _id;
		private bool _isIdTaken;

		private Frame[] _frames;
		private Dictionary<byte, int> _widFrameIndex;

		private ARQHandler _handler;
		private IDAllocator _allocator;

		private PHYSerial _serial;
		private Thread _rxThread;
		private Thread _arqoutThread;
		private OnRx _rxCallback;

		ILogger<Link> _lg;

		/*
		 * For Auto Discovery
		 */
		Link()
		{
			throw new NotImplementedException();
		}

		public Link(ILoggerFactory loggerFactory, string portName, int numRetries, int timeout, int idTimeOutDecide, OnRx rx)
		{
			_lg = loggerFactory.CreateLogger<Link>();

			_frames = new Frame[NUM_FRAMES];
			for (int i = _frames.Length - 1; i >= 0; i--)
				_frames[i] = new Frame();
			_handler = new ARQHandler(numRetries, timeout, NUM_WIDS);
			_widFrameIndex = new Dictionary<byte, int>();

			_isIdTaken = false;
			_terminate = false;
			_rxCallback = rx;
			_id = IDAllocator.NO_ID;

			_serial = new PHYSerial(portName);
			_rxThread = new Thread(FrameReceiveTask);
			_arqoutThread = new Thread(ARQRetransmission);
			_allocator = new IDAllocator(idTimeOutDecide);

			_lg.LogTrace("Link initialized");
		}

		// In this function we try to allocate new id to our self
		public Status Begin(long timeOutMillis = 6000)
		{
			_rxThread.Start();
			_arqoutThread.Start();

			_allocator.NewIdPropose(out byte[] payload);
			ARQSendBlocking(Frame.FrameType.IdA, IDAllocator.BROADCAST_ID, payload);
			while (timeOutMillis > 0)
			{
				Thread.Sleep(10);
				timeOutMillis -= 10;	
			}

			if (_allocator.GetStatus() == IDAllocator.Status.Success)
			{
				_id = _allocator.GetProposedId();
				_isIdTaken = true;
				_lg.LogInformation("Successful id allocation: {0}", _id);
			} else
			{
				_id = IDAllocator.NO_ID;
				_isIdTaken = false;
			}
			_allocator.End();
			return (_id == IDAllocator.NO_ID) ? Status.Err_NotInitialized : Status.Success;
		}

		private void FrameReceiveTask()
		{
			Frame frame = new Frame();
			while (!_terminate)
			{
				if (_serial.ReceiveBytes(frame.txFrame, 0, Frame.HEADER_LEN) != IBasicPhy.Status.Success)
					continue;
				frame.GetHeader(out Frame.FrameType ft, out byte txid, out byte rxid, out byte wid);
				_lg.LogDebug("frame header received: {0}", frame.GetHeader());
				if (!(rxid == IDAllocator.BROADCAST_ID || rxid == _id)) // SKIP if not mine
				{
					_lg.LogWarning("Discarding the frame!");
					continue;

				}

				// Get remaining
				byte len = 0;
				frame.GetPLen(out len);
				if (_serial.ReceiveBytes(frame.txFrame, Frame.HEADER_LEN, len) != IBasicPhy.Status.Success)
					continue;

				var seed = (int)DateTime.UtcNow.Subtract(DateTime.UnixEpoch).TotalMilliseconds;
				Random random = new Random(seed);
				if (random.NextDouble() > 0.7)
				{
					_lg.LogError("Delibrately manipulating received frame");
					frame.ManiPulateData();
				}
				ARQReceive(frame);

				Thread.Sleep(10);
			}

		}

		private void ARQRetransmission()
		{
			byte startWid = 0;
			while (!_terminate)
			{
				Thread.Sleep(10);
				if (!_handler.GetNextTimedoutWid(startWid, out byte tmoutwid))
				{
					_lg.LogTrace("No timeout wid");
					continue;
				}

				if (_handler.IsMaxRetryExceeded(tmoutwid))
				{
					//_lg.LogWarning("Max number of retries exceeded");
					//_handler.SetWidFree(tmoutwid);
					continue;
				}

				if (!_widFrameIndex.ContainsKey(tmoutwid))
				{
					_lg.LogError("No such wid sent {0}", tmoutwid);
					continue;
				}

				_lg.LogWarning("Resending packet with wid {0}", tmoutwid);
				ARQResend(_widFrameIndex[tmoutwid]);
				startWid = tmoutwid;
			}
		}

		// This send will append header and net ids to messages and transimission timestamp and packet save
		private void ARQReceive(in Frame frame)
		{
			frame.GetHeader(out Frame.FrameType ft, out byte txid, out byte rxid, out byte wid);
			if (ft == Frame.FrameType.ACK) // ACK if validated then Turn the timer off and set the wid free
			{
				_handler.handleAckOfWid(txid, wid);
				return;
			} // This it is not piggy backing so if else continues 

			byte[] payload = frame.GetPayload();

			bool isPacketValid = ARQHandler.CheckCRC(payload);

			if (_isIdTaken)
			{
				var seed = (int)DateTime.UtcNow.Subtract(DateTime.UnixEpoch).TotalMilliseconds; 
				Random random = new Random(seed + 30);
				if (random.NextDouble() > 0.4)
				{
					_lg.LogError("Delibrately not sending ack/nack");
				} 
				else
				{
					if (isPacketValid)
					{
						frame.RepackToAckFrame(_id);
						_lg.LogInformation($"Sending ACK for {wid}");
					}
					else
					{
						frame.RepackToNckFrame(_id);
						_lg.LogWarning($"Sending NACK for {wid}");
					}
					frame.GetSerialize(out byte[] ser);
					_serial.SendBytes(ser, 0, ser.Length);
				}
			}

			if (!isPacketValid)
				return;

			byte[] packet = ARQHandler.ExtractCRC(payload);

			if (ft == Frame.FrameType.IdA) //  
			{
				byte proposedId = (byte)(packet[0] & 0x0F);
				_handler.SetWidFree(wid);
				var stat = _allocator.CheckRecvId(proposedId);
				if (stat == IDAllocator.Status.Conflict)
				{
					_allocator.NewIdPropose(out byte[] pidPayload);
					ARQSendBlocking(Frame.FrameType.IdA, IDAllocator.BROADCAST_ID, pidPayload);
					_lg.LogWarning("Id conflict happened {0}", proposedId);

				}
				else
					_lg.LogDebug("IDAllocation status: {0}", stat.ToString());

			}
			else if (ft == Frame.FrameType.SDU)
			{
				// This is data packet, First check the crc and then ACK/NACKbyte[] 
				_rxCallback(txid, packet);
			}
			else if (ft == Frame.FrameType.NCK)
			{
				// This is data packet, First check the crc and then ACK/NACKbyte[] 
				_lg.LogWarning($"NACK received for wid: {wid}");
				_handler.SetWidToTimeout(txid);
			}
		}

		private bool GetFreeFrame(out int index)
		{
			for (int i = 0; i < _frames.Length; i++)
				if (!_frames[i].filled)
				{
					_frames[i].filled = true;
					index = i;
					return true;
				}

			index = -1;
			return false;
		}

		// This send will append header and net ids to messages and transimission timestamp and packet save
		private Status ARQSendBlocking(Frame.FrameType fr, byte rxId, byte[] bytes)
		{
			_id = _allocator.GetProposedId();

			if (!GetFreeFrame(out int index))
				return Status.Err_NoFreeFrame;

			if (!_handler.GetFreeWid(rxId, out byte wid))
				return Status.Err_NoFreeWid;
			bytes = ARQHandler.appendCRCField(bytes);

			_lg.LogDebug("Sending packet with len: {0}, val: {1}", bytes.Length, bytes);
			_widFrameIndex[wid] = index;
			_frames[index].PackFrame(fr, _id, rxId, bytes, wid);
			_frames[index].GetSerialize(out byte[] ser);
			_serial.SendBytes(ser, 0, ser.Length);

			bool maxRetiryExceeded = false;
			while (!_handler.IsWidFree(wid))
			{
				if (_handler.IsMaxRetryExceeded(wid))
				{
					maxRetiryExceeded = true;
					break;
				}
				Thread.Sleep(1);
			}

			_handler.SetWidFree(wid);
			_frames[index].filled = false;
			_widFrameIndex.Remove(wid);
			return maxRetiryExceeded ? Status.Err_MaxRetryExceed : Status.Success;
		}

		private void ARQResend(int index)
		{
			_frames[index].GetSerialize(out byte[] ser);
			_frames[index].GetHeader(out Frame.FrameType ft, out byte txid, out byte rxid, out byte wid);
			_serial.SendBytes(ser, 0, ser.Length);
		}

		public Status SendPacketBlocking(byte txId, byte[] bytes)
		{
			if (_id == IDAllocator.NO_ID)
				return Status.Err_NotInitialized;
			return ARQSendBlocking(Frame.FrameType.SDU, txId, bytes);
		}

		public Status SendPacketBlocking(byte[] bytes)
		{
			if (_id == IDAllocator.NO_ID)
				return Status.Err_NotInitialized;
			return ARQSendBlocking(Frame.FrameType.SDU, IDAllocator.BROADCAST_ID, bytes);
		}

		public void Dispose()
		{
			_terminate = true;
			_rxThread?.Join();
			_serial.Dispose();
		}
	}
}
