using plNICDriver.Link.Framing;
using plNICDriver.Phy;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using static plNICDriver.Link.ARQ.ARQHandler;

namespace plNICDriver.Link.ARQ
{
	internal class WindowElement
	{
		private static readonly int timeoutDiv = 10;

		private static byte counter = 0;

		public readonly byte wid;
		public bool AckRecv { get; private set; }

		public bool filled { get; private set; }

		Frame.FrameType ft;
		byte txid;
		byte rxid;
		byte[] pld;
		int retries;

		object timeOutCounterObj;
		int timeOutCounter;
		FramingHandler serial;

		public WindowElement(FramingHandler txer)
		{
			this.serial = txer;

			filled = false;
			AckRecv = false;
			timeOutCounterObj = new object();

			wid = counter++;

			ft = Frame.FrameType.NCK;
			rxid = IDAllocation.IDAllocator.NO_ID;
			txid = IDAllocation.IDAllocator.NO_ID;
			pld = new byte[Frame.PAYLOAD_MAX_LEN];

			retries = 0;
		}

		public void Fill(Frame.FrameType Ft, byte txId, byte rxId, byte[] payLoad)
		{
			filled = true;
			ft = Ft;
			rxid = rxId;
			txid = txId;
			pld = new byte[payLoad.Length];
			Array.Copy(payLoad, 0, pld, 0, payLoad.Length);
			retries = 0;
		}

		private async Task<Link.Status> HandleARQ(int numRetries, int timeOut)
		{
			timeOutCounter = timeoutDiv;
			AckRecv = false;
			retries = 0;

			Random rand = new Random((int)DateTime.UtcNow.Ticks);

			while (true)
			{
				if (AckRecv)
				{
					filled = false;
					return Link.Status.Success;
				}

				if (timeOutCounter >= timeoutDiv)
				{
					timeOutCounter = 0;
					retries++;
					if (retries > numRetries)
						break;
					await serial.SendFrame(ft, txid, rxid, wid, pld, retries);
				}

				await Task.Delay(timeOut/timeoutDiv);
				timeOutCounter++;
			}

			filled = false;
			return Link.Status.Err_MaxRetryExceed;
		}

		public bool Check(Frame.FrameType fT, byte txId, byte rxId, byte wid)
		{
			AckRecv = false;

			if (filled)
			{
				if (this.txid == rxId && this.wid == wid)
				{
					if (fT == Frame.FrameType.ACK)
						AckRecv = true;

					else if (fT == Frame.FrameType.NCK)
						lock (timeOutCounterObj) { timeOutCounter = timeoutDiv; }
				}
			}

			return AckRecv;
		}

		public async Task<Link.Status> StartARQ(int numRetries, int timeOut)
		{
			var rez = await HandleARQ(numRetries, timeOut);
			return rez;
		}
	}
}
