using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace plNICDriver.Link.IDAllocation
{
	public class IDAllocator
	{
		public static readonly byte BROADCAST_ID = 0b1111;
		public static readonly byte NO_ID = 0b0000;
		private static readonly int BUS_CAP = 16 - 2;
		private static readonly int TIME_DIV = 16;

		private int idDecideTimeoutMillis;
		private byte _proposeId;
		private byte _id;

		private Status _finStat;
		private NegCommands _negCom;
		private NegStatus _negStat;
		private Dictionary<byte, NegStatus> linkIds;

		private Framing.FramingHandler _handler;

		public IDAllocator(long decdtout, Framing.FramingHandler fhndlr)
		{
			_id = NO_ID;
			_handler = fhndlr;
			
			linkIds = new Dictionary<byte, NegStatus>();
			for (byte i = 1; i <= BUS_CAP; i++)
				linkIds.Add(i, NegStatus.None);

			idDecideTimeoutMillis = ((int)decdtout);
		}

		public IDAllocator(int id, Framing.FramingHandler fhndlr) : this((long)0, fhndlr)
		{
			_id = ((byte)id);
			_negCom = NegCommands.Register;
		}

		private enum NegCommands
		{
			Propose = 1,
			Register,
			Conflict,
			Probe
		};

		private enum NegStatus
		{
			None,
			Conflict,
			Proposed,
			Registered,
		};

		public enum Status
		{
			Success,
			Waiting,
			Timedout,
			CapFilled
		};

		private bool NewRandId(out byte id)
		{
			id = NO_ID;
			lock (linkIds)
				if (linkIds.Count == BUS_CAP)
					return false;

			Random random = new Random((int)DateTime.Now.Ticks);
			var randByte = new byte[1];

			do
			{
				random.NextBytes(randByte);
				id = (byte)(randByte[0] & BROADCAST_ID);

				lock (linkIds)
					if (linkIds[id] != NegStatus.None)
						continue;

			} while (id == BROADCAST_ID || id == NO_ID);
			return true;
		}

		public async Task<bool> RxIdaFrame(byte txId, byte rxId, byte wid)
		{
			var negCom = (NegCommands)wid;			
			if (negCom == NegCommands.Conflict) // The second who tries to grab this id is the one who abandon
			{
				if (_proposeId == txId)
					_negStat = NegStatus.Conflict; // the sender should resolve it the one who receives this
				lock (linkIds)
					linkIds[txId] = NegStatus.Registered;
			}
			else if (negCom == NegCommands.Propose)
			{
				_negCom = NegCommands.Propose;
				if (txId == _proposeId && _proposeId != NO_ID) // If waiting or success
					await _handler.SendFrame(Framing.Frame.FrameType.IdA, _proposeId, txId, ((byte)NegCommands.Conflict), null);

				_negStat = NegStatus.None; // The one who receives the proposed id conflict does not handle it
			}
			else if (negCom == NegCommands.Probe)
			{
				if (_id != NO_ID) // If waiting or success
					await _handler.SendFrame(Framing.Frame.FrameType.IdA, _id, txId, ((byte)NegCommands.Probe), null);
			}
			else if (negCom == NegCommands.Register)
			{
				_negCom = NegCommands.Register;


			}

			return true;
		}

		public Status GetStatus()
		{
			return _finStat;
		}

		public byte GetId()
		{
			return _id;
		}

		private async Task<Status> NegotiateTask(long totalTimeOut)
		{
			int timeCounter = TIME_DIV;
			while (totalTimeOut > 0)
			{
				if (_negStat == NegStatus.Conflict)
					if (!NewRandId(out _proposeId))
						return Status.CapFilled;

				await _handler.SendFrame(Framing.Frame.FrameType.IdA, _proposeId, BROADCAST_ID, ((byte)NegCommands.Propose), null);

				//await Task.Delay(totalTimeOut / TIME_DIV);
				timeCounter++;
				//if (timeCounter == TIME_DIV)

			}

			return Status.Timedout;
		}

		public async Task<Status> NegotiateForId()
		{
			long totalTimeOut = 8000;
			if (_id != NO_ID)
				return Status.Success;

			_finStat = Status.Waiting;
			_negStat = NegStatus.Conflict;
			var negTask = Task.Run(() => { return NegotiateTask(totalTimeOut); });
			await negTask;
			return negTask.Result;
		}

		public bool ReadyForPDU()
		{
			return _id != NO_ID && _negCom == NegCommands.Register;
		}
	}
}
