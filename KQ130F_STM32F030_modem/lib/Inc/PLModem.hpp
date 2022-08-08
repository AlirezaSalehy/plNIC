/*
 * PLModem.h
 *
 *  Created on: Jul 28, 2022
 *      Author: lidoma
 */

#ifndef SRC_PLMODEM_H_
#define SRC_PLMODEM_H_


#include <stddef.h>
#include <stdint.h>
#include <functional>
#include <string.h>

#include "stm32f0xx_hal.h"

#include "Encryption.h"

// stop & wait ARQ
// DMA
// RX & TX Interrupt
// CRC Check
// AES128
namespace PLM {
	typedef enum {
		NONE = 0,
		RSA
	} EncMode;

	typedef enum {
		OK,
		SpaceNotAvailable,
		MaxRetriesExceeded,

	} NICStatus;

	typedef std::function<void(uint8_t* frame, const uint8_t len)> rxFunc;

	template <size_t N>
	class PLModem {
	private:
		/* Asynchronous Character Based transmission (Byte Stuffing) TODO HEADER and TAIL should be in ALL
		 * Master/Modem packets
		 *  handshake
		 *  	header(rxNetID|txNetID, packetLen MSGTYPE) | NetID(1-254) => may yield error or automatic, encMode, keys(pub, prv) or automatic
		 *  data transmission
		 * 		header(rxNetID|txNetID, packetLen, MSGTYPE) | (data) | tail(padding) // should be PACKET_MAX_SIZE bytes
		 *
		 * Modem/Modem packets
		 * 	handshake
		 * 		header(rxNetID|txNetID, packetLen, MSGTYPE) | public key |
		 * 	data transmission
		 * 		header(rxNetID|txNetID, packetLen, MSGTYPE) | (len, data) | tail(CRC)	 // should be atleast 3 bytes and max of PACKET_MAX_SIZE bytes
		 */

		static const int CRC_FEILD_LEN = 2;
		static const int NETID_FEILD_LEN = 1;
		static const int MSGLEN_FEILD_LEN = 1;
		static const int MSGTYP_FEILD_LEN = 1;

		static const int PACKET_MAX_SIZE = 32;
		static const int PAYLOAD_MAX_SIZE = PACKET_MAX_SIZE - CRC_FEILD_LEN - NETID_FEILD_LEN
													- MSGLEN_FEILD_LEN - MSGTYP_FEILD_LEN;


		// sizeof(Message)
		typedef struct {
			uint8_t netId;
			uint16_t datStart; // A pointer to somewhere in the buffer
			uint16_t datLen;
			uint8_t crc[CRC_FEILD_LEN];
		} Message;

		typedef struct {
			Message msg;
			uint8_t numRetries;
			uint8_t flags; // | 1b MSB (TX = 1,RX = 0) | 1b (free = 0 or occupied = 1) | 1b ack | 1b handshake | 1b autoIDAllocate |
			uint32_t rxTick;
		} MessageHandler;
		static const uint8_t TX_RX_MASK = (1 << 7);
		static const uint8_t EMP_OCP_MASK = (1 << 6);
		static const uint8_t DAT_MASK = (1 << 0); // piggy backing is possible, both ack and dat flags or set then
		static const uint8_t ACK_MASK = (1 << 1);
		static const uint8_t HAND_MASK = (1 << 2);
		static const uint8_t AUTO_ID_MASK = (1 << 3);

//		static const int

		static constexpr size_t BUFFER_LEN(size_t bfln) {
			size_t len = ((bfln-PACKET_MAX_SIZE*2) * PACKET_MAX_SIZE) / (sizeof(MessageHandler) + PACKET_MAX_SIZE);
			return len;
		}

		static constexpr size_t NUM_MESSAGES(size_t bfln) {
			size_t num = BUFFER_LEN((bfln-PACKET_MAX_SIZE*2))/PACKET_MAX_SIZE;
			return num;
		}

		uint8_t _netId;
		uint8_t _maxRetries;
		uint32_t _rxOutMicSec;

		MessageHandler _msgHndls[NUM_MESSAGES(N)];
		uint8_t _waitBuf[BUFFER_LEN(N)]; // Waiting space for tx and rx messages
		uint8_t _txBuf[PACKET_MAX_SIZE];
		uint8_t _rxBuf[PACKET_MAX_SIZE];
		osThreadId _handleId;
		Enc::Encryption _enc;

		bool _isTerminate;

		rxFunc _rxCallback;
		UART_HandleTypeDef* _modemCom;
		UART_HandleTypeDef* _dbugbCom;

		int _findOccupyEmptyMessage() {
			for (uint8_t i = 0; i < NUM_MESSAGES(N); i++)
				if (not(_msgHndls[i].flags & EMP_OCP_MASK)) {
					_msgHndls[i].flags |= EMP_OCP_MASK;
					return i;
				}
			return -1;
		}
		void _emptyOccupiedMessage(uint8_t index) {
			_msgHndls[index].flags = 0; // now is Simpler than &= ~(MessageHandler::EMP_OCP_MASK);
		}
		void _generateTXWaitingMsg(MessageHandler& msghd, const uint8_t netId, const uint8_t* dat, const uint8_t len) {
			msghd.flags |= TX_RX_MASK;
			msghd.flags &= ~(TX_RX_MASK | EMP_OCP_MASK);

			Message& msg = msghd.msg;
			msg.netId = netId;
			msg.datLen = len;
			memcpy(_waitBuf+(msg.datStart), dat, len);
		}
		bool _receivePacketRaw(uint8_t* src, Message& msg) {
			return true;
		}
		void _serializeMessage(uint8_t* dest, size_t& len, Message msg) {
			_generateCRC();
		}
		void _generateCRC(uint8_t* crc, const uint8_t* dat, const size_t len) {
		}
		bool _checkCRC(const uint8_t* crc, const uint8_t* dat, const size_t len) {
			return true;
		}
		bool _sendBasic(const uint8_t* buf, const uint8_t len) {
			_enc.encrypt(plain, pLen, enc, eLen)
			HAL_UART_Transmit(_modemCom, (uint8_t*)&len, 1, 1000);

			HAL_UART_Transmit(_modemCom, (uint8_t*)buf, len, 1000);
			return true;
		}

	public:
		PLModem(UART_HandleTypeDef* hu, UART_HandleTypeDef* dbg, rxFunc  func) :
			_isTerminate(false), _rxCallback(func), _modemCom(hu), _dbugbCom(dbg) {
			for (uint8_t i = 0; i < NUM_MESSAGES(N); i++)
				_msgHndls[i].msg.datStart = PACKET_MAX_SIZE*i;
		}

		bool begin(uint8_t netId, Enc::Encryption encMethod, uint8_t maxRetries, uint32_t rxOut) {
			this->_netId = netId;
			this->_maxRetries = maxRetries;
			this->_rxOutMicSec = rxOut;
			this->_enc = encMethod;

			HAL_UART_Receive_IT(_modemCom, _rxBuf, 1);

			return true;
		}

		// No need for separate MSGTYPE field?
		bool send(const uint8_t netid, const uint8_t* buf, const size_t len) {
			if (len > PAYLOAD_MAX_SIZE) {
//				_handleFragmentation();
			}
			int emptyMsgIdx = _findOccupyEmptyMessage();
			if (emptyMsgIdx == -1)
				return NICStatus::SpaceNotAvailable;
			_generateTXWaitingMsg(_msgHndls[emptyMsgIdx], netid, buf, len);
			osSignalSet(_handleId, 10); // Only one thread should be able to handle UART or otherwise there should be mutex
			return true;
		}

		// This function handle receive from other devices and checks if it is handshake or data receive or auto id protocol
		void handle(const void* args) {
			_handleId = osThreadGetId();
			uint8_t nextTXCheckIdx = 0; // Checks msgs in the buffer and if it was tx then if retry == 0 sends it, else
										// checks timeout and if it has exceeded then retransmits the packet and retry += 1
			while (not _isTerminate) {
				osEvent ev = osSignalWait(10, 0);
				if (ev.status != osEventSignal || ev.value.signals != 10)
					continue;

				for (uint8_t i = 0; i < NUM_MESSAGES(N); i++) {
					if (_msgHndls[i].flags )

					osKernelSysTick();
					osKernelSysTickMicroSec(100000);
					_sendBasic(buf, (uint8_t)len); // TODO here we cast to uint8_t but it might be bigger
					// Then we should wait for acknowledgment


				}
				if (_handleId) {

				}

				if (HAL_UART_Receive(_modemCom, _waitBuf, 5, 200) == HAL_StatusTypeDef::HAL_OK)
					this->_rxCallback(_waitBuf, 5);
				HAL_UART_Transmit(_dbugbCom, _waitBuf, 5, 200);

				osThreadYield();
			}

			osThreadTerminate(_handleId);
		}

		void rxInterrupt(UART_HandleTypeDef* huart) {
			if (huart == _modemCom) {
				// Try to parse the packet and copy it into _msgHndlrs, then call the signal function
				osSignalSet(_handleId, 10);
			}
		}

	};

}



#endif /* SRC_PLMODEM_H_ */
