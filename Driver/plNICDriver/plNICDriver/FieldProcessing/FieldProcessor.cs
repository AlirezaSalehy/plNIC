using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace plNICDriver.FieldProcessing
{
	internal class FieldProcessor
	{
		private readonly FieldInfo[] FIELD_INFOS;

		public FieldProcessor(FieldInfo[] infos)
		{
			FIELD_INFOS = infos;
		}

		public byte GetField(in byte[] hdr, in int fieldIdx)
		{
			byte fieldVal;
			ref FieldInfo fi = ref FIELD_INFOS[fieldIdx];
			fieldVal = (byte)(hdr[fi.byteIdx] >> fi.bitIdx & fi.mask);
			return fieldVal;
		}

		public void SetField(in byte[] hdr, in int fieldIdx, byte fieldVal)
		{
			ref FieldInfo fi = ref FIELD_INFOS[fieldIdx];
			hdr[fi.byteIdx] &= (byte)(~(fi.mask << fi.bitIdx) & 0xFF); // First empty field
			hdr[fi.byteIdx] |= (byte)((fieldVal & fi.mask) << fi.bitIdx);
		}
	}
}
