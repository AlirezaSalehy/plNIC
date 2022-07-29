/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : AES128.h
* Author             : Systems LAB - IMS (Industrial & Multisegment Sector)
* Date First Issued  : 03/06/2012
* Description        : Cryptography routine definitions
********************************************************************************
* History:
* 03/06/2012: V0.1
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __AES_128_H
#define __AES_128_H

/* Includes ------------------------------------------------------------------*/
#include "interfaceconfig.h"

/* Exported define -----------------------------------------------------------*/
// AES only supports Nb = 4 
#define Nb                    4                     // number of columns in the state & expanded key
#define Nk                    4                     // number of columns in a key
#define Nr                    10                    // number of rounds in encryption

#define AES_BUFFER_SIZE       USER_PAYLOAD_SIZE + 50
#define EXPANDED_KEY_SIZE     (4 * Nb * (Nr + 1))

/* Exported types ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
uint8_t CRY_EncriptData(uint8_t* buffer, uint8_t buffersize, uint8_t *AES128key);
uint8_t CRY_DecriptData(uint8_t* buffer, uint8_t buffersize, uint8_t *AES128key);


#endif /*__AES_128_H*/

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
