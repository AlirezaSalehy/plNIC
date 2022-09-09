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
			private IList<int> nextRxSid;
			private IList<int> nextTxSid;
			
			private Dictionary<int, Segment> onGoingRxSegments;
			private Dictionary<int, Segment> onGoingTxSegments;
			private Dictionary<int, int> txSegmentNextFid;

			private ILogger<Connection>? _lg;

			public Connection()
			{
				_lg = null;
				nextRxSid = new List<int>() { 0 };
				nextTxSid = new List<int>() { 0 };
				onGoingRxSegments = new Dictionary<int, Segment>();
				onGoingTxSegments = new Dictionary<int, Segment>();
				txSegmentNextFid = new Dictionary<int, int>();
			}

			public Connection(Connection prevConnecion)
			{
				_lg = prevConnecion._lg;
				nextRxSid = prevConnecion.nextRxSid;
				nextTxSid = prevConnecion.nextTxSid;
				onGoingRxSegments = prevConnecion.onGoingRxSegments;
				onGoingTxSegments = prevConnecion.onGoingTxSegments;
				txSegmentNextFid = prevConnecion.txSegmentNextFid;
			}

			public Connection(ILoggerFactory factory) : this()
			{
				_lg = factory.CreateLogger<Connection>();
			}

			public bool IsRxFragmentValid(Fragment frag)
			{
				if (onGoingRxSegments.ContainsKey(frag.SID) || frag.SID == nextRxSid[0])
					return true;
				return false;
			}

			public void AddRxFragment(Fragment frag)
			{
				if (!IsRxFragmentValid(frag))
					return;

				else if (frag.SID == nextRxSid[0])
				{
					onGoingRxSegments.Add(frag.SID, new Segment(frag.SID));
					nextRxSid[0]++;
					if (nextRxSid[0] >= Fragment.NUM_SIDS)
						nextRxSid[0] = 0;
				}

				_lg?.LDebug($"Fragment received nextRxSid: {nextRxSid[0]}");

				onGoingRxSegments[frag.SID].AddFragment(frag);
			}

			public int? AddTxSegment(byte[] msg)
			{
				lock (onGoingTxSegments)
					if (onGoingTxSegments.Count > Fragment.NUM_SIDS)
						return null;

				var segSid = nextTxSid[0];
				var toTxSeg = new Segment(segSid, msg);
				lock (onGoingTxSegments)
				{
					onGoingTxSegments.Add(segSid, toTxSeg);
					txSegmentNextFid.Add(segSid, 0);
					nextTxSid[0] += 1;
					if (nextTxSid[0] >= Fragment.NUM_SIDS)
						nextTxSid[0] = 0;
				}

				_lg?.LDebug($"Segment added to TX, SID: {toTxSeg.Sid}, " +
									$"numFrags : {toTxSeg.Frags.Count}" +
									$", nextTxSid: {nextTxSid[0]}");

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
				lock (onGoingTxSegments)
					if (!HasTxSegment(sid))
						return null;

				lock (onGoingTxSegments)
				{
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
				connections.Add(i, new Connection(factory));
			
			_link = link;
			this._onRx = _onRx;
		}

		public async Task<bool> SendSegment(byte txId, byte[] dat)
		{
			_lg.LInformation($"Sending segment with length: {dat.Length} to TxId: {txId}");
			int nnssid;
			int? sid;
			lock (connections)
			{
				sid = connections[txId].AddTxSegment(dat);
				if (sid is null)
					return false;
			}

			nnssid = sid ?? 0;
			Fragment? nextFrag = null;
			lock(connections)
				nextFrag = connections[txId].NextToTxFragment(nnssid);
			while (nextFrag is not null)
			{
				nextFrag.GetSerialized(out byte[] serialized);
				_lg.LInformation($"Sending fragment sid: {nextFrag.SID}, fid: " +
					$"{nextFrag.FID}, df: {nextFrag.DF} fragLen: {serialized.Length}");
				var status  = await _link.SendPacket(txId, serialized);
				if (status == Link.Link.Status.Success)
					lock (connections)
						nextFrag = connections[txId].NextToTxFragment(nnssid);
				else if (status == Link.Link.Status.Err_NoFreeWid)
				{
					_lg.LWarning($"No free id, waiting...");
					await Task.Delay(100);
				}
			}

			return true;
		}

		public void OnRxFrames(byte txId, byte[] framePayload)
		{
			Fragment frag = new Fragment(framePayload);
			_lg.LDebug($"Fragment received sid: {frag.SID}, fid: {frag.FID}," +
				$" df: {frag.DF} fragLen: {framePayload.Length}");
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
