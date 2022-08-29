using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace plNICDriver.Link.IDAllocation
{
	internal class IDAllocator
	{
		public static readonly byte BROADCAST_ID = 0b1111;
		public static readonly byte NO_ID = 0b0000;

		private bool isActive;

		private DateTime startTick;
		private long idDecideTimeoutMillis;

		private HashSet<byte> linkIds;
		private byte proposeId;

		public IDAllocator(long idDecideTimeoutMillis)
		{
			startTick = DateTime.UtcNow;
			proposeId = NO_ID;
			linkIds = new HashSet<byte>();
			this.idDecideTimeoutMillis = idDecideTimeoutMillis;
			isActive = false;
		}

		public enum Status
		{
			Success,
			Conflict,
			NoConflict,
			Waiting,
			NotActive
		};

		private void GenLinkIdPayload(out byte[] payload)
		{
			var seed = DateTime.UtcNow.Subtract(DateTime.UnixEpoch).Seconds;
			Random random = new Random(seed);
			payload = new byte[1];

			do
			{
				random.NextBytes(payload);
				payload[0] &= BROADCAST_ID;

			} while (payload[0] == BROADCAST_ID || payload[0] == NO_ID || linkIds.Contains(payload[0]));
		}

		public void NewIdPropose(out byte[] payload)
		{
			startTick = DateTime.UtcNow;
			GenLinkIdPayload(out payload);
			proposeId = payload[0];
			isActive = true;
		}

		public void End()
		{
			linkIds.Clear();
			isActive = false;
		}

		public Status CheckRecvId(byte recvedID)
		{
			if (!isActive)
				return Status.NotActive;

			var currTime = DateTime.UtcNow;
			var timeSpan = currTime.Subtract(startTick).TotalMilliseconds;
			linkIds.Add(recvedID);

			if (recvedID == proposeId)
				return Status.Conflict;
			if (timeSpan < idDecideTimeoutMillis)
				return Status.Waiting;
			
			return Status.NoConflict;
		}

		public Status GetStatus()
		{
			if (!isActive)
				return Status.NotActive;

			var currTime = DateTime.UtcNow;
			var timeSpan = currTime.Subtract(startTick).TotalMilliseconds;

			if (linkIds.Contains(proposeId))
				return Status.Conflict;
			if (timeSpan < idDecideTimeoutMillis)
				return Status.Waiting;

			return Status.Success;
		}
		public byte GetProposedId()
		{
			return proposeId;
		}
	}
}
