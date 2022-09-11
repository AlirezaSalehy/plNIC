#pragma once

class MessageHandler {
private:
	static const int CRC_FEILD_LEN = 2;
	static const int NETID_FEILD_LEN = 1;
	static const int MSGLEN_FEILD_LEN = 1;
	static const int MSGTYP_FEILD_LEN = 1;

	static const int PACKET_MAX_SIZE = 32;
	static const int PAYLOAD_MAX_SIZE = PACKET_MAX_SIZE - CRC_FEILD_LEN - NETID_FEILD_LEN
												- MSGLEN_FEILD_LEN - MSGTYP_FEILD_LEN;

	static const int CRC_FEILD_LEN = 2;
	static const int NETID_FEILD_LEN = 1;
	static const int MSGLEN_FEILD_LEN = 1;
	static const int MSGTYP_FEILD_LEN = 1;

	static const int PACKET_MAX_SIZE = 32;
	static const int PAYLOAD_MAX_SIZE = PACKET_MAX_SIZE - CRC_FEILD_LEN - NETID_FEILD_LEN
												- MSGLEN_FEILD_LEN - MSGTYP_FEILD_LEN;

	static const uint8_t DAT_MASK = (1 << 0); // piggy backing is possible, both ack and dat flags or set then
	static const uint8_t ACK_MASK = (1 << 1);
	static const uint8_t HAND_MASK = (1 << 2);
	static const uint8_t AUTO_ID_MASK = (1 << 3);
//		static const uint8_t PACK_WID_MASK = (1 << 4); Stop_wait_ARQ does not need Window cntr
	static const uint8_t NUM_OF_TX_RX_BUF = 4; // _txEncBuf _txSerBuf _rxEncBuf _rxDesBuf
//		static const int


	// sizeof(Message)
	typedef struct {
		uint8_t netId;     // RX or TX NetId based on the TX/RX Flag
		uint8_t flags; //  | 1b ack | 1b handshake | 1b autoIDAllocate |
		uint16_t datStart; // A pointer to somewhere in the buffer
		uint16_t datLen;
		uint8_t crc[CRC_FEILD_LEN];
	} Message;

	typedef struct {
		bool isOccupied;
		bool TxRx;

		uint8_t numRetries;
		uint32_t rxTick;

		Message msg;
	} MessageHandle;

public:
	static constexpr size_t BUFFER_LEN(size_t bfln) {
		size_t len = ((bfln-PACKET_MAX_SIZE*NUM_OF_TX_RX_BUF) * PACKET_MAX_SIZE) / (sizeof(MessageHandler) + PACKET_MAX_SIZE);
		return len;
	}

	static constexpr size_t NUM_MESSAGES(size_t bfln) {
		size_t num = BUFFER_LEN((bfln-PACKET_MAX_SIZE*NUM_OF_TX_RX_BUF))/PACKET_MAX_SIZE;
		return num;
	}


};



