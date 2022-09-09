using System;
using System.Collections.Generic;
using System.Diagnostics.CodeAnalysis;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace plNICDriver.Net.Fragmentation
{
	internal class Segment : IComparable<Segment>, IEqualityComparer<Segment>	
	{
		internal static readonly int SEGMENT_MAX_LEN = Fragment.NUM_FIDS * Fragment.PAYLOAD_MAX_LEN;
		private SortedSet<Fragment> _fragments;

		public Segment( int sid, byte[] msg) : this(sid)
		{
			if (msg.Length > SEGMENT_MAX_LEN)
				msg = msg.Take(SEGMENT_MAX_LEN).ToArray();
			MLen = msg.Length;
			CreateFragments(msg);
		}

		public Segment(int sid)
		{
			_fragments = new SortedSet<Fragment>();
			Sid = sid;
		}

		private void CreateFragments(byte[] msg)
		{
			var fragmentId = 0;
			while (msg.Length - (fragmentId * Fragment.PAYLOAD_MAX_LEN) > Fragment.PAYLOAD_MAX_LEN)
			{
				var midFrag = new Fragment(0, Sid, fragmentId,
												msg[(fragmentId * Fragment.PAYLOAD_MAX_LEN)..((fragmentId + 1) * Fragment.PAYLOAD_MAX_LEN)]);
				_fragments.Add(midFrag);
				fragmentId++;
			}
			var lastFrag = new Fragment(1, Sid, fragmentId,
												msg[(fragmentId * Fragment.PAYLOAD_MAX_LEN)..msg.Length]);
			_fragments.Add(lastFrag);
		}

		public bool IsValid()
		{
			if (!_fragments.Any() || _fragments.Last().DF != 1)
				return false;

			var lastFragFid = _fragments.Last().FID;
			for (var i = 0; i < lastFragFid; i++)
				if (!_fragments.Where((frag) => { return frag.FID == i; }).Any())
					return false;

			MLen = _fragments.Last().FID * Fragment.PAYLOAD_MAX_LEN + _fragments.Last().MLEN;
			return true;
		}

		private byte[]? IntergateFragments()
		{
			if (!IsValid())
				return null;

			byte[] buf = new byte[MLen];
			var offset = 0;
			foreach (Fragment f in _fragments)
			{
				Array.Copy(f.MSG, 0, buf, offset, f.MLEN);
				offset += f.MLEN;
			}

			return buf;
		}

		public void AddFragment(Fragment f)
		{
			if (f.SID != Sid || _fragments.Contains(f))
				return;
			_fragments.Add(f);
		}

		public int CompareTo(Segment? other)
		{
			throw new NotImplementedException();
		}

		public bool Equals(Segment? x, Segment? y)
		{
			if (x == null && y == null)
				return true;

			if (x == null || y == null)
				return false;

			return x.Sid == y.Sid;
		}

		public int GetHashCode([DisallowNull] Segment obj)
		{
			return obj.Sid * 10;
		}

		internal int Sid { get; private set; }
		//internal int EndDevId { get; private set; }
		internal List<Fragment> Frags { get => _fragments.ToList(); }
		internal byte[]? Msg { get => IntergateFragments(); }
		public int MLen { get; private set; }
	}
}
