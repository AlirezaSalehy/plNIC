using plNICDriver.FieldProcessing;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace plNICDriver.Link.Framing
{
	public class Frame
	{
		public enum FrameType // Resides in Flag
		{
			NCK = 0b000,
			ACK = 0b001,
			SDU = 0b010,
			IdA = 0b100,
		};

		private enum Fields
		{
			Flag,
			PLen,
			TxId,
			RxId,
			Hsh,
			WnId,
		};

		private static readonly FieldProcessor FIELD_PROCESSOR = new FieldProcessor(new FieldInfo[]{
			new FieldInfo(0b111, 0, 5),
			new FieldInfo(0b11111, 0, 0),
			new FieldInfo(0b1111, 1, 4),
			new FieldInfo(0b1111, 1, 0),
			new FieldInfo(0b1111, 2, 4),
			new FieldInfo(0b1111, 2, 0),
		});

		internal static readonly byte HEADER_LEN = 3;
		internal static readonly byte HASH_REPLICATE_LEN = 30;
		internal static readonly byte PAYLOAD_MAX_LEN = (byte)(Math.Pow(2, 5)-1-ARQ.ARQHandler.CRC_LEN);
		internal static readonly byte FRAME_MAX_LEN = (byte)(PAYLOAD_MAX_LEN + HEADER_LEN + ARQ.ARQHandler.CRC_LEN);

		public byte[] txFrame;

		private void SetField(Fields field, byte fieldVal)
		{
			FIELD_PROCESSOR.SetField(txFrame, ((int)field), fieldVal);
		}

		private byte GetField(in Fields field)
		{
			return FIELD_PROCESSOR.GetField(txFrame, ((int)field));	
		}

		internal void SetHeader(byte[] hd)
		{
			Array.Copy(hd, txFrame, HEADER_LEN);
		}

		private byte CalcHdrHash()
		{
			byte[] headerCopy = new byte[HEADER_LEN];
			Array.Copy(txFrame, 0, headerCopy, 0, HEADER_LEN);
			FIELD_PROCESSOR.SetField(headerCopy, ((int)Fields.Hsh), 0);
			byte[] headerReplicated = new byte[HASH_REPLICATE_LEN];
			for (int i = 0; i < HASH_REPLICATE_LEN/HEADER_LEN; i++)
				Array.Copy(headerCopy, 0, headerReplicated, i*HEADER_LEN, HEADER_LEN);
			var firstByte = SHA1.HashData(headerReplicated)[0];
			FIELD_PROCESSOR.SetField(headerCopy, ((int)Fields.Hsh), firstByte);
			return FIELD_PROCESSOR.GetField(headerCopy, ((int)Fields.Hsh)); ;
		}

		public Frame()
		{
			txFrame = new byte[FRAME_MAX_LEN];
		}

		public Frame(FrameType frameType, byte txId, byte rxId, byte wid, byte[]? dat, int tries = 1)
		{
			// Retries is a property of win element but for Back-off num of retries is needed
			Tries = tries;
			txFrame = new byte[FRAME_MAX_LEN];
			byte len = 0;
			if (dat is not null)
			{
				Array.Copy(dat, 0, txFrame, HEADER_LEN, dat.Length);
				len = (byte)dat.Length;
			}

			FrTp = frameType;
			TxId = txId;
			RxId = rxId;
			Wid = wid;
			PLen = len;
			SetField(Fields.Hsh, CalcHdrHash());
		}

		public Frame(in Frame fr)
		{
			Tries = fr.Tries;
			txFrame = new byte[FRAME_MAX_LEN];
			Array.Copy(fr.txFrame, 0, txFrame, 0, fr.PLen+HEADER_LEN);
		}


		internal int Tries
		{
			get;
			private set;
		}
		internal FrameType FrTp { 
			get { return ((FrameType)GetField(Fields.Flag)); }
			private set { SetField(Fields.Flag, (byte)value); }	
		}
		internal byte PLen { 
			get { return GetField(Fields.PLen); }
			private set { SetField(Fields.PLen, value); }
		}
		internal byte TxId { 
			get { return GetField(Fields.TxId);  }
			private set { SetField(Fields.TxId, value); }
		}
		internal byte RxId { 
			get { return GetField(Fields.RxId);  }
			private set { SetField(Fields.RxId, value); }
		}
		internal byte Wid { 
			get { return GetField(Fields.WnId);  }
			private set { SetField(Fields.WnId, value); }
		}
		internal byte Hsh
		{
			get { return GetField(Fields.Hsh); }
			private set { SetField(Fields.Hsh, value); }
		}

		internal bool IsValid(out byte calcdHash)
		{
			calcdHash = CalcHdrHash();
			return Hsh == calcdHash;
		}

		internal void GetSerialize(out byte[] frameBytes)
		{
			byte frameLen = (byte)(PLen + HEADER_LEN);
			frameBytes = new byte[frameLen];
			Array.Copy(txFrame, 0, frameBytes, 0, frameLen);
		}

		internal string GetHeader()
		{
			return $"HEADER: TxId: {TxId}, RxId: {RxId}, " +
						$"FrTp: {FrTp}, WId: {Wid}, PLen: {PLen}, Hsh: {Hsh}";
		}

		public override string ToString()
		{
			string desc = GetHeader();
			if (PLen > 0)
			{
				string payload = Encoding.ASCII.GetString(txFrame, HEADER_LEN, PLen);
				desc += $"\nPAYLOAD: Data: {5}";
			}
			return desc;
		}

		internal byte[] GetPayload()
		{
			byte[] payload = new byte[PLen];
			Array.Copy(txFrame, HEADER_LEN, payload, 0, PLen);
			return payload;
		}

		internal void ManiPulateData()
		{
			if (PLen == 0)
				return;
			var seed = DateTime.UtcNow.Subtract(DateTime.UnixEpoch).Seconds;
			Random random = new Random(seed);
			int randIndx = random.Next(HEADER_LEN, PLen);
			txFrame[randIndx] = (byte)random.Next(256);
		}
	}
}
