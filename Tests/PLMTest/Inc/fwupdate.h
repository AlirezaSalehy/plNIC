/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : fwupdate.h
* Author             : Systems LAB - IMS (Industrial & Multisegment Sector)
* Date First Issued  : 14/06/2012
* Description        : Firmware update engine definitions
********************************************************************************
* History:
* 14/06/2012: V0.1
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FW_UPDATE_H
#define __FW_UPDATE_H

/* Includes ------------------------------------------------------------------*/
#include "interfaceconfig.h"

/* Private define ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
void RFU_GetImageSize(uint8_t *size);
void RFU_GetImageSignature(uint8_t *signature);
bool RFU_SetImageData(uint8_t *ImgSegmentBuffer, uint16_t ImgSegmentAddress, uint16_t ImgSegmentSize);
bool RFU_SwapImageData(void);

#endif /* __FW_UPDATE_H */

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
