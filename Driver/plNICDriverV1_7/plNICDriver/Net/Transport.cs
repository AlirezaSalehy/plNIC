using Microsoft.Extensions.Logging;
using Pastel;
using plNICDriverV1_7.Net.Encryption;
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
	//`![](5D533510B3B2024905CB9148794ED75C.png)
	public class Transport : IDisposable
	{
		private static readonly int FRAME_TIMEOUT = 3500 * 2 + 300 * 2;
		private static readonly int DECIDE_TIMEOUT = 3000;
		private static readonly int NUM_RETRIES = 3;

		public delegate void OnRxSegment(byte txId, byte[] data); // this is fragmented and encypted

		private Link.Link _link;
		private Fragmentation.FragmentHandler _fhandler;
		private ILogger<Transport> _lg;
		private OnRxSegment _onRx;

		public Transport(ILoggerFactory loggerFactory, string comPort, OnRxSegment onRx) : 
										this(loggerFactory, comPort, Link.IDAllocation.IDAllocator.NO_ID, onRx)	{}

		public Transport(ILoggerFactory loggerFactory, string comPort, int id, OnRxSegment onRx)
		{
			_lg = loggerFactory.CreateLogger<Transport>();
			_link = new Link.Link(loggerFactory, id, comPort, 
									NUM_RETRIES, FRAME_TIMEOUT,	DECIDE_TIMEOUT, OnRxFrame);
			_fhandler = new Fragmentation.FragmentHandler(loggerFactory, _link, OnRxSegmnt);
			_lg.LInformation("NetPort layer is initialized");
			_onRx = onRx;
		}

		public void Dispose()
		{
			_link.Dispose();
		}

		public async Task<bool> Begin()
		{
			return await _link.Begin();
		}

		public async Task<bool> SendSegment(byte txId, string dat)
		{
			var datBytes = Encoding.ASCII.GetBytes(dat);
			byte[] encSeg = AESOperation.Encrypt(dat);
			_lg.LDebug($"encSeg seg {encSeg.ToStr()}");
			return await _fhandler.SendSegment(txId, encSeg);
		}

		private void OnRxFrame(byte txId, byte[] dat) // Here defragmentation and deciphering is done
		{ 
			_fhandler.OnRxFrames(txId, dat);
		}

		private void OnRxSegmnt(byte txId, byte[] dat) // Here defragmentation and deciphering is done
		{
			_lg.LDebug($"encSeg seg {dat.ToStr()}");
			var plainSeg = AESOperation.Decrypt(dat);
			var plainInBytes = plainSeg.Select(c => (byte)c).ToArray();
			_onRx(txId, plainInBytes);
		}
	}
}
