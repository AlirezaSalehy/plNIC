/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : dongle.h
* Author             : Systems LAB - IMS (Industrial & Multisegment Sector)
* Date First Issued  : 10/02/2012
* Description        : Dongle hardware definition routines
********************************************************************************
* History:
* 10/02/2012: V0.1
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DONGLE_H
#define __DONGLE_H

/* Includes ------------------------------------------------------------------*/
#include "interfaceconfig.h"

/* Private define ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef enum 
{
  A_LED_ON,
  A_LED_OFF,
  A_LED_FLASH
}DH_LedAction_t;

typedef enum 
{
  A_LED_ERROR,
  A_LED_DATA,
  A_LED_BOTH
}DH_LedType_t;

/* Private variables ---------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void      DH_LED_Config(void);
void      DH_LED_Init(void);
void      DH_IO_Init(void);
void      DH_InOutConfig(uint8_t ioArray);

uint8_t   DH_GetInputs(void);
void      DH_SetOutputs(uint8_t outbuffer);
void      DH_SetOutput_N(uint8_t out, BitAction PinVal);
void      DH_GetSysTime(uint8_t *timebuffer);
bool      DH_SetSysTime(uint8_t *timebuffer);
bool      DH_NextDay(void);
void      DH_ClearNextDayFlag(void);
bool      DH_SysTimeAdjusted(void);


void      DH_Delay_ms(uint16_t ms);
bool      DH_DelayElapsed(uint32_t tstp, uint16_t ms); 
void      DH_SetTimeout(uint16_t sec);
bool      DH_TimeoutElapsed(void);
uint32_t  DH_Timestamp(void);
void      DH_FlashLED(DH_LedType_t nLTy, DH_LedAction_t nLAct);
void      DH_ShowLED(DH_LedType_t nLTy, DH_LedAction_t nLAct);
void      DH_ServiceDoneFlashLED(void);


#endif /* __DONGLE_H */

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
