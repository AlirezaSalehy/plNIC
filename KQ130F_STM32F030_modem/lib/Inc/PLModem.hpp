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

	typedef std::function<void(uint8_t* frame, const uint8_t len)> rxFunc;

	template <size_t N>
	class PLModem {
	private:
		/* Asynchronous Character Based transmission (Byte Stuffing) TODO HEADER and TAIL should be in ALL
		 * Master/Modem packets
		 *  handshake
		 *  	MSGTYPE, NetID(0-255) => may yeild error or automatic, encMode, keys(pub, prv) or automatic
		 *  data transmission
		 * 		header(MSGTYPE, rxNetID) | (len, data) | tail(padding) // should be 64 bytes
		 *
		 * Modem/Modem packets
		 * 	handshake
		 * 		header(MSGTYPE, rxNetID) | public key |
		 * 	data transmission
		 * 		header(MSGTYPE, rxNetID) | (len, data) | tail(CRC)	 // should be atleast 3 bytes and max of 64 bytes
		 */
		static const int PACKET_SIZE = 64;

		uint8_t _netId;
		uint8_t _maxRetries;
		uint8_t _buf[N];
		uint8_t _enc;

		rxFunc _rxCallback;
		UART_HandleTypeDef* _modemCom;


	public:
		PLModem(UART_HandleTypeDef* hu, rxFunc  func) {
			this->_modemCom = hu;
			this->_rxCallback = func;
		}

		bool begin(uint8_t netId, Enc::Encryption encMethod, uint8_t maxRetries) {
			this->_netId = netId;
			this->_maxRetries = maxRetries;
			this->_enc = encMethod;

	//		HAL_UART_Receive_DMA(_uh, _buf, PACKET_SIZE);
	//		HAL_CRC_Calculate(hcrc, pBuffer, BufferLength)
	//		this->_uh

			return false;
		}

	};

}



#endif /* SRC_PLMODEM_H_ */
