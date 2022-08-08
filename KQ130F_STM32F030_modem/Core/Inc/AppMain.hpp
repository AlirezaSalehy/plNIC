/*
 * main.hpp
 *
 *  Created on: Jul 29, 2022
 *      Author: lidoma
 */
#ifndef INC_APPMAIN_HPP_
#define INC_APPMAIN_HPP_

#include "main.h"
#include "PLModem.hpp"

#define TRANSMITTER 1
#define TRANSMITTER_NET_ID 2

//#define RECEIVER	1
#define RECEIVER_NET_ID 1

#ifdef __cplusplus
extern "C" {
#endif

void StartTestCaseTask(const void* params);
void defaultThreadTask(const void* params);
void StartplNICHandleTask(void const * params);

#ifdef __cplusplus
}
#endif

#endif /* INC_APPMAIN_HPP_ */
