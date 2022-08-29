using plNICDriver.Link.ARQ;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace plNICDriver.Link.Framing
{

	//` ![](ED6D459F5E8088B10469C782A55F30FB.png;;;0.03705,0.02610)

	// Generic Link Frame is consisted of Header + Payload
	public class Frame
	{
		private struct FieldInfo
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

		private enum Fields
		{
			Flag,
			PLen,
			TxId,
			RxId,
			Rerv,
			WnId,
		};

		public enum FrameType // Resides in Flag
		{
			NCK = 0b000,
			ACK = 0b001,
			SDU = 0b010,
			IdA = 0b100,
		};

		private static readonly FieldInfo[] FIELD_INFOS = new FieldInfo[]{
			new FieldInfo(0b111, 0, 5),
			new FieldInfo(0b11111, 0, 0),
			new FieldInfo(0b1111, 1, 4),
			new FieldInfo(0b1111, 1, 0),
			new FieldInfo(0b1111, 2, 4),
			new FieldInfo(0b1111, 2, 0),
		};

		public static readonly byte HEADER_LEN = 3;
		public static readonly byte PAYLOAD_MAX_LEN = (byte)Math.Pow(2, 5);
		public static readonly byte FRAME_MAX_LEN = (byte)(PAYLOAD_MAX_LEN + HEADER_LEN);

		public bool filled;
		public byte[] txFrame;
		public long txUnixMillis;

		public static void GetHeader(byte[] hdr, out FrameType frameType, out byte txId, out byte rxId, out byte wid)
		{
			byte tempFT;
			GetField(hdr, Fields.Flag, out tempFT);
			frameType = (FrameType)tempFT;

			GetField(hdr, Fields.TxId, out txId);
			GetField(hdr, Fields.RxId, out rxId);
			GetField(hdr, Fields.WnId, out wid);
		}

		private static void GetField(in byte[] hdr, in Fields field, out byte fieldVal)
		{
			int infoIdx = (int)field;
			ref FieldInfo fi = ref FIELD_INFOS[infoIdx];
			fieldVal = (byte)(hdr[fi.byteIdx] >> fi.bitIdx & fi.mask);
		}

		public Frame()
		{
			filled = false;
			txFrame = new byte[FRAME_MAX_LEN];
			txUnixMillis = 0;
		}

		public Frame(in Frame fr)
		{
			filled = fr.filled;
			txFrame = new byte[FRAME_MAX_LEN];
			Array.Copy(fr.txFrame, 0, txFrame, 0, FRAME_MAX_LEN);
			txUnixMillis = 0;
		}

		public void SetHeader(byte[] hd)
		{
			for (int i = 0; i < HEADER_LEN; i--)
				Console.WriteLine(hd[i]);
			Array.Copy(hd, txFrame, HEADER_LEN);
		}

		private void SetHeader(byte flags, byte len, byte txId, byte rxId, byte wid)
		{
			for (int i = HEADER_LEN - 1; i >= 0; i--) // First empty header
				txFrame[i] = 0;
			byte[] feildVals = new byte[] { flags, len, txId, rxId, wid };

			var valInfos = FIELD_INFOS.Zip(feildVals, (l, r) => new { info = l, val = r });
			foreach (var valInfo in valInfos)
				txFrame[valInfo.info.byteIdx] |= (byte)((valInfo.val & valInfo.info.mask) << valInfo.info.bitIdx);
		}

		private void SetField(Fields field, byte fieldVal)
		{
			int infoIdx = (int)field;
			ref FieldInfo fi = ref FIELD_INFOS[infoIdx];
			txFrame[fi.byteIdx] &= (byte)(~(fi.mask << fi.bitIdx) & 0xFF); // First empty field
			txFrame[fi.byteIdx] |= (byte)((fieldVal & fi.mask) << fi.bitIdx);
		}

		private void GetField(Fields field, out byte fieldVal)
		{
			GetField(txFrame, field, out fieldVal);
		}

		public void GetHeader(out FrameType frameType, out byte txId, out byte rxId, out byte wid)
		{
			GetHeader(txFrame, out frameType, out txId, out rxId, out wid);
		}

		public void GetPLen(out byte plen)
		{
			GetField(Fields.PLen, out plen);
		}

		private void RepackAckNck(FrameType type, byte? nRxId)
		{
			GetField(Fields.TxId, out byte txId);
			GetField(Fields.RxId, out byte rxId);
			GetField(Fields.WnId, out byte wid);

			if (nRxId != null)
				rxId = nRxId.Value;
			SetHeader((byte)type, 0, rxId, txId, wid);
			SetField(Fields.PLen, 0);
		}

		public void RepackToAckFrame(byte? rxId)
		{
			RepackAckNck(FrameType.ACK, rxId);
		}

		public void RepackToNckFrame(byte? rxId)
		{
			RepackAckNck(FrameType.NCK, rxId);
		}

		public void PackFrame(FrameType type, byte txId, byte rxId, in byte[]? dat, byte wid)
		{
			byte len = 0;
			if (dat is not null)
			{
				Array.Copy(dat, 0, txFrame, HEADER_LEN, dat.Length);
				len += (byte)dat.Length;
			}

			SetHeader((byte)type, len, txId, rxId, wid);
		}

		public void PackFrame(in byte[]? dat)
		{
			byte len = 0;
			if (dat is not null)
			{
				Array.Copy(dat, 0, txFrame, HEADER_LEN, dat.Length);
				len += (byte)dat.Length;
			}

			SetField(Fields.PLen, len);
		}

		public void GetSerialize(out byte[] frameBytes)
		{
			GetField(Fields.PLen, out byte len);
			byte frameLen = (byte)(len + HEADER_LEN);
			frameBytes = new byte[frameLen];
			Array.Copy(txFrame, 0, frameBytes, 0, frameLen);
		}

		public string GetHeader()
		{
			GetHeader(out FrameType ft, out byte tid, out byte rid, out byte wid);
			GetField(Fields.PLen, out byte plen);
			return string.Format("HEADER: TxId: {0}, RxId: {1}, FrTp: {2}, WId: {3}, PLen: {4}",
										tid, rid, ft.ToString(), wid, plen);
		}

		public override string ToString()
		{
			string desc = GetHeader();

			GetField(Fields.PLen, out byte plen);
			if (plen > 0)
			{
				string payload = Encoding.ASCII.GetString(txFrame, HEADER_LEN, plen);
				desc += String.Format("\nPAYLOAD: Data: {5}", payload);
			}
			return desc;
								
		}
		public byte[] GetPayload()
		{
			GetPLen(out byte len);
			byte[] payload = new byte[len];
			Array.Copy(txFrame, Frame.HEADER_LEN, payload, 0, len);
			return payload;
		}
		public void ManiPulateData()
		{
			GetPLen(out byte len);
			if (len == 0)
				return;
			var seed = DateTime.UtcNow.Subtract(DateTime.UnixEpoch).Seconds;
			Random random = new Random(seed);
			int randIndx = random.Next(HEADER_LEN, len);
			txFrame[randIndx] = (byte)random.Next(256);
		}
	}
}
