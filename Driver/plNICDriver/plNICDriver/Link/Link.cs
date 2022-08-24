using plNICDriver.Link.ARQ;
using plNICDriver.Phy;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace plNICDriver.Link
{
	// Stop and Wait ARQ is implemented here 
	// using ACK and CRC checksome and retransmission, link id allocation
	// A ID frame is like  wid_.1|len_.7|0b0000 |0b1111 |randId_.4
	// A DAT frame is like wid_.1|len_.7|txId_.4|rxId_.4|daBytes_N|CRC_2 
	// A Ack frame is like wid_.1|len_.7|txId_.4|rxId_.4|
	// SO LEN MUST BE 0 OR N+2 which is >= 2
	public class Link : IDisposable
	{
		// TODO: Next we should complete receiver task so that after [discarding or] receiving
		// Calls related callback of ARQ or IdAlloc

		public enum Status
		{
			Success,
			Failure
		}

		public delegate void OnRx(byte[] pcktContent);

		private bool _terminate;

		private int _id;

		private LinkFrame[] frames;

		private ARQHandler _handler;

		private PHYSerial _serial;
		private Thread _rxThread;
		private OnRx _rxCallback;

		/*
		 * For Auto Discovery
		 */
		Link()
		{
			throw new NotImplementedException();
		}

		public Link(string portName, int numRetries, int timeout, OnRx rx)
		{
			LinkFrame[] frames = new LinkFrame[1];
			frames[0] = new ARQFrame();
			_handler = new ARQHandler(numRetries, timeout, frames);

			_terminate = false;
			_rxCallback = rx;
			_id = 0b0000;

			_serial = new PHYSerial(portName);
			_rxThread = new Thread(() =>_ReceiveTask());
		}

		// In this function we try to allocate new id to our self
		public Status Begin()
		{
			return Status.Failure;
		}

		private void _ReceiveTask()
		{
			ARQFrame frame = new ARQFrame();
			while (!_terminate)
			{
				byte[] header = new byte[ARQFrame.HEADER_LEN];
				if (_serial.ReceiveBytes(header, header.Length, 5000) != IBasicPhy.Status.Success)
					continue;
				frame.SetHeader(header);

				if (!frame.IsMine(_id)) // SKIP if not mine
					continue;

				// Get remaining
				byte len = 0;
				frame.GetPLen(out len);
				byte[] payloadPlusCRC = new byte[len];
				if (_serial.ReceiveBytes(payloadPlusCRC, payloadPlusCRC.Length, 5000) != IBasicPhy.Status.Success)
					continue;

				if (ARQHandler.IsAck(ref frame)) // ACK
				{
					var frame = _handler.frames[0];
					if (frame.txUnixMillis != 0 && frame.wid == wid) // if ack is for this and we are awaiting We can go for next packet and wid = !wid
					{
						frame.txUnixMillis = 0;
						wid++;
						wid
					}
				}
				else if (ARQHandler.IsIdAlloc(ref frame)) //  
				{ 
					int proposedId = payloadPlusCRC[0] & 0x0F;
					if (proposedId == _id) // Conflict return error
					{
						Console.WriteLine("id conflict happened");
					}										 

				} else if (ARQHandler.IsDat(ref frame))
				{
					// This is data packet, First check the crc and then ACK/NACK
				}

				// Send ack back
				if (ARQHandler.IsDat(ref frame) || ARQHandler.IsIdAlloc(ref frame))
				{ // Sending Conflict message or Sending ACK message
				  // Both are the same here

					byte txId = 0, rxId = 0;
					frame.GetIds(out txId, out rxId);
					_Send(txId, null);
				}

				Thread.Sleep(10);
			}

		}

		// This send will append header and net ids to messages and transimission timestamp and packet save
		private void _Send(int rxId, byte[]? bytes)
		{
			var frame = _handler.frames[0];
			int bytesLen = 0; // Corresponding to dat + crc 16 => >=2 or 0
			if (bytes is not null)
				bytesLen = bytes.Length;

			byte[] header = new byte[2] {0, 0};
			header[0] |= (byte)(frame.wid << 7);
			header[0] |= (byte)(bytesLen & (0xFF & ~(0x01 << 7)));

			header[1] |= (byte)((_id & 0x0F) << 4);
			header[1] |= (byte)(rxId & 0x0F);

			frame.txFrame[0] = header[0];
			frame.txFrame[1] = header[1];
			if (bytes is not null)
				Array.Copy(bytes, 0, frame.txFrame, 2, bytesLen);
			_serial.SendBytes(frame.txFrame, bytesLen + 2, 50);
			frame.txUnixMillis = DateTimeOffset.Now.ToUnixTimeMilliseconds();
		}

		public bool IsMine(int id)
		{
			byte txId = 0, rxId = 0;
			GetIds(out txId, out rxId);
			return (rxId == ARQHandler.BROADCAST_ID) || (rxId == id);
		}

		private void _ReSend()
		{
			var frame = _handler.frames[0];
			int len = frame.txFrame[0] & (0xFF & ~(0x01 << 7));
			_serial.SendBytes(frame.txFrame, len + 2, 50);
		}

		public Status SendPacket(int txId, byte[] bytes)
		{
			return Status.Failure;
		}

		public void Dispose()
		{
			_terminate = true;
			_rxThread?.Join();
			_serial.Dispose();
		}
	}
}
