using Microsoft.Extensions.Logging;
using Pastel;
using plNICDriver.Link.ARQ;
using plNICDriver.Link.IDAllocation;
using plNICDriver.Phy;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static plNICDriver.Link.Framing.Frame;

namespace plNICDriver.Link.Framing
{

	//` ![](ED6D459F5E8088B10469C782A55F30FB.png;;;0.03705,0.02610)

	// TODO:
	// 1- To synchronize packet framing and parsing constant pattern should be available in frames to 
	// specify different boundaries of the frame, so that if misalignment happened can be recovered

	// Generic Link Frame is consisted of Header + Payload
	public class FramingHandler
	{
		private static readonly int MAX_FRAME_TX_TIME = 4000;

		public delegate void OnRxFrame(FrameType type, int txId, int rxId, byte[] dat, int wid);

		private PHYSerial _serial;
		private Thread _rxThread;
		private Thread _txThread;
		
		private OnRxFrame _onRxFrame;
		private bool _terminate;

		private object _busOccupiedlock = new object();
		private long _BusLastRxTick = 0;

		int frameCounter = 0;
		byte[] buffer = new byte[FRAME_MAX_LEN];

		private PriorityQueue<Frame, int> _txQueue = new PriorityQueue<Frame, int>();
		private IList<byte[]> _txedFrames = new List<byte[]>();
		ILogger<FramingHandler> _lg;

		public FramingHandler(ILoggerFactory loggerFactory, OnRxFrame rxFrame, string portName)
		{
			_lg = loggerFactory.CreateLogger<FramingHandler>();

			_serial = new PHYSerial(portName, OnBytesRx);
			_rxThread = new Thread(FrameReceiveTask);
			_txThread = new Thread(FramePriorityTransmissionTask);
			_terminate = false;
			_onRxFrame = rxFrame;
		}

		public void Begin()
		{
			_rxThread.Start();
			_txThread.Start();
		}

		private void OnBytesRx(byte[] bytes)
		{
			lock (_busOccupiedlock)
			{
				_BusLastRxTick = DateTime.UtcNow.Ticks;
				Array.Copy(bytes, 0, buffer, frameCounter, bytes.Length);
				frameCounter += bytes.Length;
				if (frameCounter >= FRAME_MAX_LEN)
					frameCounter = 0;
			}
			_lg.LogDebug($"bytes recv {buffer.ToStr(bytes.Length)}");
		}

		private void ReceiveBytes(byte[] bytes, int offset, int numBytes)
		{
			while (0 < numBytes)
			{
				lock (_busOccupiedlock)
					if (frameCounter >= numBytes)
					{
						Array.Copy(buffer, 0, bytes, offset, numBytes);
						frameCounter = frameCounter - numBytes;
						Array.Copy(buffer, numBytes, buffer, 0, frameCounter);
						break;
					}

				Thread.Yield();
			}
		}

		private async Task<bool> BackOff()
		{
			// Non persistent CSMA
			var toSend = false;
			while (!toSend)
			{
				// Fallback for a random time
				Random rand = new Random((int)DateTime.UtcNow.Ticks);
				var backOffTime = (int)(rand.NextDouble() * MAX_FRAME_TX_TIME);
				_lg.LWarning($"Back-off for " + $"{backOffTime} millis".PastelBg(Color.Maroon));
				await Task.Delay(backOffTime);

				// Check if possible to send
				lock (_busOccupiedlock)
				{
					var now = DateTime.UtcNow.Ticks;
					if ((now - _BusLastRxTick) / TimeSpan.TicksPerMillisecond > 130)
						toSend = true;
					else
						_lg.LWarning("Waiting, bus is " + "occupied".PastelBg(Color.Maroon));
				}
			}

			return true;
		}

		private async void FramePriorityTransmissionTask()
		{
			bool shouldTryBackOff = false;
			Frame nextFrame;
			while (!_terminate)
			{
				lock (_txQueue)
					if (_txQueue.Count > 0)
						shouldTryBackOff = true;
				
				if (shouldTryBackOff)
				{
					await BackOff(); // This way we give chance to late arriving lower priorities to be first

					lock (_txQueue)
						nextFrame = new Frame(_txQueue.Dequeue());

					nextFrame.GetSerialize(out byte[] payload);
					// TODO: Here we can use contant pattern boundaries and stuffing
					byte[] frameTotal = new byte[payload.Length + 1];
					frameTotal[0] = ((byte)payload.Length);
					Array.Copy(payload, 0, frameTotal, 1, payload.Length);
					_lg.LDebug($"header fields: {nextFrame.GetHeader()}");
					await _serial.SendBytes(frameTotal, 0, frameTotal.Length);
					
					lock (_txedFrames)
						_txedFrames.Add(payload);
				}

				shouldTryBackOff = false;
				Thread.Sleep(1);
			}
		}

		private void FrameReceiveTask()
		{
			Frame frm = new Frame();
			while (!_terminate)
			{
				ReceiveBytes(frm.txFrame, 0, 1);
				ReceiveBytes(frm.txFrame, 0, Frame.HEADER_LEN);
				
				_lg.LogDebug("Frame header received: {0}", frm.GetHeader());

				if (!frm.IsValid(out byte calcHash))
				{
					_lg.LError($"Invalid header, calculated Hsh {calcHash}");
					continue;
				} else
					_lg.LDebug("valid header");


				// Get remaining
				ReceiveBytes(frm.txFrame, Frame.HEADER_LEN, frm.PLen);

				//Random random = new Random((int)DateTime.Now.Ticks);
				//if (random.NextDouble() > 0.7)
				//{
				//	_lg.LogError("Delibrately manipulating received frame");
				//	frm.ManiPulateData();
				//}

				var payload = frm.GetPayload();
				_onRxFrame(frm.FrTp, frm.TxId, frm.RxId, payload, frm.Wid);

				Thread.Sleep(10);
			}
		}

		public async Task<bool> SendFrame(FrameType type, int txId, int rxId, int wid, byte[]? dat)
		{
			Frame frame = new Frame(type, (byte)txId, (byte)rxId, (byte)wid, dat);
			lock(_txQueue)
				_txQueue.Enqueue(frame, frame.PLen);

			while (true)
			{
				lock (_txedFrames)
				{
					for (int i = _txedFrames.Count - 1; i >= 0; i--)
					{
						if (_txedFrames[i].Length == frame.PLen+HEADER_LEN)
							if (!_txedFrames[i].Where((t, i) => t != frame.txFrame[i]).Any())
							{
								_txedFrames.RemoveAt(i);
								return true;
							}
					}
				}

				Thread.Sleep(1);
			}
			
			return true;
		}

		public void Dispose()
		{
			_serial.Dispose();
			_rxThread?.Join();
			_terminate = true;
		}
	}
}
