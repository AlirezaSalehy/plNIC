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
		// Calls related callback of ARQ or IdAlloc

		public enum Status
		{
			Success,
			Failure
		}


		private static readonly int NUM_FRAMES = 5;
		private static readonly int NUM_WIDS = 1;

		public delegate void OnRx(byte txid, byte[] pcktContent);

		private bool _terminate;

		private int _id;

		private Frame[] frames;

		private ARQHandler _handler;
		private IDAllocator _allocator;

		private PHYSerial _serial;
		private Thread _rxThread;
		private OnRx _rxCallback;

		ILoggerFactory loggerFactory = LoggerFactory.Create(builder => {
			builder.AddSimpleConsole((i) =>
			{
				i.ColorBehavior = Microsoft.Extensions.Logging.Console.LoggerColorBehavior.Enabled;
				i.TimestampFormat = "HH:mm:ss ";
			});
			builder.SetMinimumLevel(LogLevel.Trace);
		});
		ILogger<Link> lg;

		/*
		 * For Auto Discovery
		 */
		Link()
		{
			throw new NotImplementedException();
		}

		public Link(string portName, int numRetries, int timeout, OnRx rx)
		{
			lg = loggerFactory.CreateLogger<Link>();
			
			frames = new Frame[NUM_FRAMES];
			for (int i = frames.Length - 1; i >= 0; i--)
				frames[i] = new Frame();
			_handler = new ARQHandler(numRetries, timeout);

			_terminate = false;
			_rxCallback = rx;
			_id = 0b0000;

			_serial = new PHYSerial(portName);
			_rxThread = new Thread(_ReceiveTask);

			lg.LogTrace("Link initialized");
		}

		// In this function we try to allocate new id to our self
		public Status Begin()
		{
			_rxThread.Start();
			return Status.Failure;
		}

		private void _ReceiveTask()
		{
			Frame frame = new Frame();
			while (!_terminate)
			{
				if (_serial.ReceiveBytes(frame.txFrame, 0, Frame.HEADER_LEN) != IBasicPhy.Status.Success)
					continue;
				frame.GetHeader(out Frame.FrameType ft, out byte txid, out byte rxid, out byte wid);
				lg.LogDebug("frame header received: {0}", frame.GetHeader());
				if (!(rxid == IDAllocator.BROADCAST_ID || rxid == _id)) // SKIP if not mine
				{
					lg.LogWarning("Discarding the frame!");
					continue;

				} 

				// Get remaining
				byte len = 0;
				frame.GetPLen(out len);
				if (_serial.ReceiveBytes(frame.txFrame, Frame.HEADER_LEN, len) != IBasicPhy.Status.Success)
					continue;

				if (ft == Frame.FrameType.ACK) // ACK
				{
					//var frame = _handler.frames[0];
					//if (frame.txUnixMillis != 0 && frame.wid == wid) // if ack is for this and we are awaiting We can go for next packet and wid = !wid
					//{
					//	frame.txUnixMillis = 0;
					//	wid++;
					//	wid
					//}
				}
				else if (ft == Frame.FrameType.IdA) //  
				{ 
					//int proposedId = payloadPlusCRC[0] & 0x0F;
					//if (proposedId == _id) // Conflict return error
					//{
					//	Console.WriteLine("id conflict happened");
					//}										 

				} else if (ft == Frame.FrameType.SDU)
				{
					// This is data packet, First check the crc and then ACK/NACKbyte[] 
					byte[] packet = new byte[len];
					Array.Copy(frame.txFrame, Frame.HEADER_LEN, packet, 0, len);
					_rxCallback(txid, packet);
				}

				// Send ack back
				if (ft == Frame.FrameType.SDU || ft == Frame.FrameType.IdA)
				{ // Sending Conflict message or Sending ACK message
				  // Both are the same here

					frame.RepackToAckFrame();
					frame.GetSerialize(out byte[] ser);
					_serial.SendBytes(ser, 0, ser.Length);
				}

				Thread.Sleep(10);
			}

		}

		// This send will append header and net ids to messages and transimission timestamp and packet save
		private void _Send(int rxId, byte[]? bytes)
		{
			var frame = frames[0];
			int bytesLen = 0; // Corresponding to dat + crc 16 => >=2 or 0
			if (bytes is not null)
				bytesLen = bytes.Length;

			//_serial.SendBytes(frame.txFrame, bytesLen + 2, 50);
			frame.txUnixMillis = DateTimeOffset.Now.ToUnixTimeMilliseconds();
		}

		public Status SendPacket(int txId, byte[] bytes)
		{
			return Status.Failure;
		}

		public Status SendPacket(byte[] bytes)
		{
			lg.LogDebug("Sending packet with len: {0}, val: {1}", bytes.Length, bytes);
			frames[0].PackFrame(Frame.FrameType.IdA, IDAllocator.NO_ID, IDAllocator.BROADCAST_ID, bytes, 12);
			frames[0].GetSerialize(out byte[] ser);
			_serial.SendBytes(ser, 0, ser.Length);
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
