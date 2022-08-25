using System;
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

		public static readonly byte WIND_LEN = 1;


		public ARQHandler(int numRetries, int timeOut)
		{
			widCounter = 0;
			this.numRetries = numRetries;
			this.timeOut = timeOut;
		}


		//public ARQFrame[] GetTimedoutFrames()
		//{
		//	List<ARQFrame> indxs = new List<ARQFrame>();
		//	for (int i = 0; i < frames.Length; i++)
		//	{
		//		if (frames[i].filled && frames[i].txUnixMillis > timeOut)
		//			indxs.Add(frames[i]);
		//	}

		//	return indxs.ToArray();
		//}

		//private int[] EmptyFrames()
		//{
		//	List<int> indxs = new List<int>();
		//	for (int i = 0; i < frames.Length; i++)
		//		if (!frames[i].filled)
		//			indxs.Add(i);
		//	return indxs.ToArray();
		//}

		private void CalcCrc16(byte[] dat, out ushort crc)
		{
			var datCrc = NullFX.CRC.Crc16.ComputeChecksum(NullFX.CRC.Crc16Algorithm.Modbus, dat);
			crc = datCrc;
			Console.WriteLine("datCrc CRC: {0:X8}", datCrc);
		}

		//public bool IsWindIdAvailable()
		//{
		//	return frames.Any(x => !x.filled);
		//}
	}
}
