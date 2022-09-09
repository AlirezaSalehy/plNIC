using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace plNICDriver.FieldProcessing
{
	internal class FieldInfo
	{
		public byte mask;
		public int byteIdx;
		public int bitIdx;

		public FieldInfo(byte mask, int byteIdx, int bitIdx)
		{
			this.mask = mask;
			this.byteIdx = byteIdx;
			this.bitIdx = bitIdx;
		}
	}
}
