using Microsoft.Extensions.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace plNICDriver.Net.Fragmentation
{
	internal class FragmentHandler
	{
		// TODO: Adding port to Transport layer

		private struct Connection
		{
			private int nextRxSid;
			private int nextTxSid;
			
			private Dictionary<int, Segment> onGoingRxSegments;
			private Dictionary<int, Segment> onGoingTxSegments;
			private Dictionary<int, int> txSegmentNextFid;

			private ILogger<Connection> _lg;

			public Connection(ILoggerFactory factory)
			{
				_lg = factory.CreateLogger<Connection>();
				nextRxSid = 0;
				nextTxSid = 0;
				onGoingRxSegments = new Dictionary<int, Segment>();
				onGoingTxSegments = new Dictionary<int, Segment>();
				txSegmentNextFid = new Dictionary<int, int>();
			}

			public bool IsRxFragmentValid(Fragment frag)
			{
				if (onGoingRxSegments.ContainsKey(frag.SID) || frag.SID == nextRxSid)
					return true;
				return false;
			}

			public void AddRxFragment(Fragment frag)
			{
				if (!IsRxFragmentValid(frag))
					return;

				else if (frag.SID == nextRxSid)
				{
					onGoingRxSegments.Add(frag.SID, new Segment(frag.SID));
					nextRxSid++;
					if (nextRxSid >= Fragment.NUM_SIDS)
						nextRxSid = 0;
				}

				_lg.LDebug($"Fragment received nextRxSid: {nextRxSid}");

				onGoingRxSegments[frag.SID].AddFragment(frag);
			}

			public int? AddTxSegment(byte[] msg)
			{
				Console.WriteLine("sdfsfds");
				if (onGoingTxSegments.Count > Fragment.NUM_SIDS)
					return null;

				Console.WriteLine("sdfsfdsdfdsfs");
				Thread.Sleep(100);


				var segSid = nextTxSid;
				onGoingTxSegments.Add(nextTxSid, new Segment(nextTxSid, msg));
				txSegmentNextFid.Add(segSid, 0);
				nextTxSid++;
				if (nextTxSid >= Fragment.NUM_SIDS)
					nextTxSid = 0;

				_lg.LDebug($"Segment for transmission nextTxSid: {nextTxSid}");

				return segSid;
			}

			public Segment? NextCmpltRxSegment()
			{
				foreach (var seg in onGoingRxSegments.Values)
					if (seg.IsValid())
					{
						Segment cmpltSeg = seg;
						onGoingRxSegments.Remove(seg.Sid);
						return cmpltSeg;
					}
				return null;
			}

			public Fragment? NextToTxFragment(int sid)
			{
				if (!HasTxSegment(sid))
					return null;
			
				var segFrags = onGoingTxSegments[sid].Frags;
				var nextFrag = segFrags[txSegmentNextFid[sid]];
				txSegmentNextFid[sid] += 1;
				if (txSegmentNextFid[sid] >= segFrags.Count)
				{
					onGoingTxSegments.Remove(sid);
					txSegmentNextFid.Remove(sid);
				}

				return nextFrag;
			}

			public Fragment? NextToTxFragment()
			{
				foreach (int sid in txSegmentNextFid.Keys)
					return NextToTxFragment(sid);

				return null;
			}

			public bool HasTxSegment(int sid)
			{
				return onGoingTxSegments.ContainsKey(sid);
			}
		}

		private Dictionary<int, Connection> connections;
		private Link.Link _link;
		private ILogger<FragmentHandler> _lg;
		private Transport.OnRxSegment _onRx;

		public FragmentHandler(ILoggerFactory factory, Link.Link link, Transport.OnRxSegment _onRx)
		{
			_lg = factory.CreateLogger<FragmentHandler>();
			connections = new Dictionary<int, Connection>();
			var endDevIds = Link.IDAllocation.IDAllocator.LINK_IDS;
			for (int i = endDevIds.End.Value-1; i >= 0; i--)
				connections.Add(i, new Connection());
			
			_link = link;
			this._onRx = _onRx;
		}

		public async Task<bool> SendSegment(byte txId, byte[] dat)
		{
			//_lg.LInformation("Trying to send 1");
			int? sid = connections[txId].AddTxSegment(dat);
			//_lg.LInformation("Trying to send 3");
			if (sid is null)
				return false;

			_lg.LInformation("Trying to send 2");

			int nnssid = sid ?? 00;
			var nextFrag = connections[txId].NextToTxFragment(nnssid);
			while (nextFrag is not null)
			{
				nextFrag.GetSerialized(out byte[] serialized);
				Console.WriteLine($"this is {serialized.Length} {serialized.ToStr()}");
				var status  = await _link.SendPacket(txId, serialized);
				_lg.LInformation("Trying to send");
				if (status == Link.Link.Status.Success)
					nextFrag = connections[txId].NextToTxFragment(nnssid);
			}

			return true;
		}

		public void OnRxFrames(byte txId, byte[] framePayload)
		{
			Console.WriteLine($"{framePayload.ToStr()}");
			Fragment frag = new Fragment(framePayload);
			_lg.LDebug($"Fragment received sid: {frag.SID}, fid: {frag.FID}, df: {frag.DF}");
			if (connections[txId].IsRxFragmentValid(frag))
				connections[txId].AddRxFragment(frag);
			var seg =  connections[txId].NextCmpltRxSegment();
			if (seg is null)
				return;

			var msgBytes = seg.Msg;
			if (msgBytes is null)
				return;

			_onRx(txId, msgBytes);
		}
	}
}
