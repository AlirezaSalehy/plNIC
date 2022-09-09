using plNICDriver.FieldProcessing;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace plNICDriver.Net.Fragmentation
{
	internal class Fragment : IComparable, IEqualityComparer
	{
		private enum Fields
		{
			DF, // Don't fragment flag
			SID, // Segment Id
			FID // Fragment Id which indicates order of fragments
		};

		private static readonly FieldProcessor FIELD_PROCESSOR = new FieldProcessor(new FieldInfo[]{
			new FieldInfo(0b1, 0, 7),
			new FieldInfo(0b1111, 0, 3),
			new FieldInfo(0b111, 0, 0),
		});

		private static readonly int FRAGMENT_MAX_LEN = Link.Framing.Frame.PAYLOAD_MAX_LEN;

		internal static readonly int NUM_FIDS = 8;
		internal static readonly int NUM_SIDS = 16;
		internal static readonly int HEADER_LEN = 1;
		internal static readonly int PAYLOAD_MAX_LEN = FRAGMENT_MAX_LEN - HEADER_LEN;

		private byte[] _txFragment;

		private void SetField(Fields field, byte fieldVal)
		{
			FIELD_PROCESSOR.SetField(_txFragment, (int)field, fieldVal);
		}

		private byte GetField(in Fields field)
		{
			return FIELD_PROCESSOR.GetField(_txFragment, (int)field);
		}

		// Segement maximum length is 248 = 31 * 8
		public Fragment(int dF, int sID, int fID, byte[] msg)
		{
			_txFragment = new byte[FRAGMENT_MAX_LEN];
			MLEN = msg.Length;
			DF = dF;
			SID = sID;
			FID = fID;
			Array.Copy(msg, 0, _txFragment, HEADER_LEN, MLEN);
		}

		public Fragment(byte[] framePayload)
		{
			_txFragment = new byte[FRAGMENT_MAX_LEN];
			Array.Copy(framePayload, _txFragment, framePayload.Length);
			MLEN = framePayload.Length - HEADER_LEN;
		}

		public void GetSerialized(out byte[] serial)
		{
			serial = new byte[MLEN + HEADER_LEN];
			Array.Copy(_txFragment, 0, serial, 0, serial.Length);
		}

		public int CompareTo(object? obj)
		{
			if (obj == null)
				return 1;

			return FID - ((Fragment)obj).FID;
		}

		public new bool Equals(object? x, object? y)
		{
			if (x == null && y == null)
				return true;

			if (x == null || y == null)
				return false;

			return (((Fragment)y).FID == ((Fragment)x).FID); // && (((Fragment)y).SID == ((Fragment)x).SID);
		}

		public int GetHashCode(object obj)
		{
			return FID;
		}

		internal int DF
		{
			get => GetField(Fields.DF);
			private set => SetField(Fields.DF, (byte)value);
		}
		internal int SID
		{
			get => GetField(Fields.SID);
			private set => SetField(Fields.SID, (byte)value);
		}
		internal int FID
		{
			get => GetField(Fields.FID);
			private set => SetField(Fields.FID, (byte)value);
		}
		internal int MLEN
		{
			get;
			private set;
		}
		internal byte[] MSG
		{
			get => _txFragment[HEADER_LEN..(MLEN+HEADER_LEN)];
			private set => Array.Copy(value, 0, _txFragment, HEADER_LEN, value.Length);
		}
	}
}
