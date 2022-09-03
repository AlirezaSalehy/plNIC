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
		/*
		 * For Auto Discovery
		 */
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

		private ARQHandler _arqh;
		private IDAllocator _allocator;
		private FramingHandler _fh;

		private OnRx _rxCallback;

		private ILogger<Link> _lg;

		public Link(ILoggerFactory loggerFactory, int id, string portName, int numRetries, int timeout, int idTimeOutDecide, OnRx rx)
		{
			_lg = loggerFactory.CreateLogger<Link>();
			_fh = new FramingHandler(loggerFactory, OnRxFrameDispatch, portName);

			_arqh = new ARQHandler(loggerFactory, 
									RxARQedFrame,
									_fh, 
									numRetries, timeout, NUM_WIDS);
			_allocator = new IDAllocator(id, _fh);
			_rxCallback = rx;
			_arqh.ACKResponderId = ((byte)id);

			_lg.LogTrace("Link initialized");
		}

		public void OnRxFrameDispatch(Frame.FrameType ft, int txid, int rxid, byte[] payload, int wid)
		{
			if (ft == Frame.FrameType.IdA) // IdA packets does not support NACK/ACK so is not supported by ARQ and
											// are handled separately
				_allocator.RxIdaFrame(((byte)txid), ((byte)rxid), ((byte)wid)).RunSynchronously();

			else if (_allocator.ReadyForPDU()) // SDU handling, active only when id is allocated
			{
				if (rxid == IDAllocator.BROADCAST_ID || rxid == _allocator.GetId()) // SKIP if not mine
					_arqh.OnRxFrame(ft, txid, rxid, payload, wid);
			}
		}

		// In this function we try to allocate new id to our self
		public async Task<bool> Begin()
		{
			_fh.Begin();
			IDAllocator.Status rez = await _allocator.NegotiateForId();
			return rez == IDAllocator.Status.Success;
		}

		void RxARQedFrame(Frame.FrameType ft, byte txId, byte rxId, byte[] payload)
		{
			// This is data packet, First check the crc and then ACK/NACKbyte[] 
			_rxCallback(txId, payload);
		}

		public async Task<Status> SendPacket(byte rxId, byte[] bytes)
		{
			if (!_allocator.ReadyForPDU())
				return Status.Err_NotInitialized;
			var rez = await _arqh.SendARQedFrame(Frame.FrameType.SDU, _allocator.GetId(), rxId, bytes);
			return rez ? Status.Success : Status.Err_GenFailure;
		}

		public async Task<Status> SendPacket(byte[] bytes)
		{
			if (!_allocator.ReadyForPDU())
				return Status.Err_NotInitialized;
			var rez = await _arqh.SendARQedFrame(Frame.FrameType.SDU, _allocator.GetId(), 
													IDAllocator.BROADCAST_ID, bytes);
			return rez ? Status.Success : Status.Err_GenFailure;
		}

		public void Dispose()
		{
			_fh.Dispose();
			_arqh.Dispose();
		}
	}
}
