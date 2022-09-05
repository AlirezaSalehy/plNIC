using Microsoft.Extensions.Logging;
using Pastel;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace plNICDriver.Net
{
	// This class will implement Fragmentation and defragmentation based on the MTU of Link
	// Allocating net id to plNIC
	// Encryption handling
	public class NetPort : IDisposable
	{
		private static readonly int FRAME_TIMEOUT = 3500 * 2 + 300 * 2;
		private static readonly int DECIDE_TIMEOUT = 3000;
		private static readonly int NUM_RETRIES = 3;

		public delegate void OnRxSegment(byte txId, byte[] data); // this is fragmented and encypted

		private Link.Link _link;
		private ILogger<NetPort> _lg;
		private OnRxSegment _onRx;

		public NetPort(ILoggerFactory loggerFactory, string comPort, OnRxSegment onRx) : 
										this(loggerFactory, comPort, Link.IDAllocation.IDAllocator.NO_ID, onRx)	{}

		public NetPort(ILoggerFactory loggerFactory, string comPort, int id, OnRxSegment onRx)
		{
			_lg = loggerFactory.CreateLogger<NetPort>();
			_link = new Link.Link(loggerFactory, id, comPort, 
									NUM_RETRIES, FRAME_TIMEOUT,	DECIDE_TIMEOUT, OnRxFrame);
			_onRx = onRx;
			_lg.LInformation("NetPort layer is initialized");
		}

		public void Dispose()
		{
			_link.Dispose();
		}

		public async Task<bool> Begin()
		{
			return await _link.Begin();
		}

		public async Task<bool> SendSegment(byte txId, byte[] dat)
		{
			var res = await _link.SendPacket(txId, dat);
			return res == Link.Link.Status.Success;
		}

		public async Task<bool> SendSegment(byte[] dat)
		{
			return (await _link.SendPacket(dat)) == Link.Link.Status.Success;
		}

		private void OnRxFrame(byte txId, byte[] dat) // Here defragmentation and deciphering is done
		{
			_onRx(txId, dat);
		}
	}
}
