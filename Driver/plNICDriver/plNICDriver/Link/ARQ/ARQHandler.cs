﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace plNICDriver.Link.ARQ
{
	// This is basicly selective repeate ARQ with window len of 1
	public class ARQHandler
	{
		public int numRetries;
		public int timeOut;

		public int widCounter;
		public ARQFrame[] frames;

		public static readonly byte WIND_LEN = 1;
		public static readonly byte BROADCAST_ID = 0X0F;
		public static readonly byte NO_ID = 0b0000;

		public ARQHandler(int numRetries, int timeOut)
		{
			widCounter = 0;
			this.numRetries = numRetries;
			this.timeOut = timeOut;
			frames = new ARQFrame[WIND_LEN];
		}

		public ARQFrame[] GetTimedoutFrames()
		{
			List<ARQFrame> indxs = new List<ARQFrame>();
			for (int i = 0; i < frames.Length; i++)
			{
				if (frames[i].filled && frames[i].txUnixMillis > timeOut)
					indxs.Add(frames[i]);
			}

			return indxs.ToArray();
		}

		public void GenFrameAck(ref ARQFrame frame, out ARQFrame ackFrame)
		{
			ackFrame = new ARQFrame(new byte[1], 0);
			byte wid, len = 0, txid, rxid;
			frame.GetWId(out wid);
			frame.GetIds(out txid, out rxid);
			ackFrame.SetHeader(wid, len, rxid, txid);
		}

		public bool GenDataFrame(int rxId, byte[]? bytes, out ARQFrame? frame)
		{
			frame = null;
			if (!IsFrameAvailable())
				return false;

			int firstEmpty = EmptyFrames()[0];
			frame = frames[firstEmpty];

			return true;
		}

		private int[] EmptyFrames()
		{
			List<int> indxs = new List<int>();
			for (int i = 0; i < frames.Length; i++)
				if (!frames[i].filled)
					indxs.Add(i);
			return indxs.ToArray();
		}

		private void CalcCrc16(byte[] dat, out ushort crc)
		{
			var datCrc = NullFX.CRC.Crc16.ComputeChecksum(NullFX.CRC.Crc16Algorithm.Modbus, dat);
			crc = datCrc;
			Console.WriteLine("datCrc CRC: {0:X8}", datCrc);
		}

		public bool IsFrameAvailable()
		{
			return frames.Any(x => !x.filled);
		}

		public static bool IsAck(ref ARQFrame frame)
		{
			byte len = 0;
			frame.GetPLen(out len);
			return len == 0;
		}

		public static bool IsIdAlloc(ref ARQFrame frame)
		{
			byte len = 0;
			frame.GetPLen(out len);
			byte txId, rxId;
			frame.GetIds(out txId, out rxId);
			return len == 1 && txId == NO_ID;
		}

		public static bool IsDat(ref ARQFrame frame)
		{
			byte len = 0;
			frame.GetPLen(out len);
			byte txId, rxId;
			frame.GetIds(out txId, out rxId);
			return len >= 2 && txId != NO_ID;
		}
	}

}