/*
 * Encryption.h
 *
 *  Created on: Jul 29, 2022
 *      Author: lidoma
 */

#ifndef ENCRYPTION_H_
#define ENCRYPTION_H_

#include <stddef.h>

#include "rsa.h"

namespace Enc {
	class Encryption {
	public:
		Encryption();
		virtual ~Encryption();

		virtual void encrypt(const uint8_t* plain, const size_t pLen, uint8_t* enc, size_t& eLen);
		virtual void decrypt(const uint8_t* enc, const size_t eLen, uint8_t* plain, size_t& pLen);
	};

	class RSA: public Encryption {
		struct
		RSA() {

		}
		RSA(unsigned long, unsigned long, unsigned long, unsigned long) {

		}

		void encrypt(const uint8_t* plain, const size_t pLen, uint8_t* enc, size_t& eLen);
		void decrypt(const uint8_t* enc, const size_t eLen, uint8_t* plain, size_t& pLen);
	};

	class AES: public Encryption {
		void encrypt(const uint8_t* plain, const size_t pLen, uint8_t* enc, size_t& eLen);
		void decrypt(const uint8_t* enc, const size_t eLen, uint8_t* plain, size_t& pLen);
	};
}



#endif /* ENCRYPTION_H_ */
