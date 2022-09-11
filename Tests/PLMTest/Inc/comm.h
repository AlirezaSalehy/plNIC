/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : comm.h
* Author             : Systems LAB - IMS (Industrial & Multisegment Sector)
* Date First Issued  : 03/12/2008
* Description        : USART Communication routine definitions
********************************************************************************
* History:
* 07/16/2007: V0.1
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMM_H
#define __COMM_H

/* <!> INSERT THE DIRECTIVE #include "comm.h" IN THE stm32f10x_it.c FILE */


/* Includes ------------------------------------------------------------------*/
#include "interfaceconfig.h"

/* Private define ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void      COMM_Init(void);
void      COMM_Reset(void);
bool      COMM_StartTransmission(void);
void      COMM_EnableReceiver(void);
void      COMM_ResetReceiver(void);
bool      COMM_TransmissionOngoing(void);
bool      COMM_FrameReceivingOngoing(void);
bool      COMM_FrameArrived(void);
bool      COMM_FrameTransmitted(void);
uint8_t*  COMM_GetBufferPointer(void);

// Peripheral specific
void      COMM_SPISetParam(void);






/* --------------------------------------------------------------------------- */
/* <!> DO NOT MODIFY */
/* --------------------------------------------------------------------------- */
#ifndef ___USE_COMM_REDEF___
#define ___USE_COMM_REDEF___
#if (COMM_INTERFACE_TYPE == 1)
  void  _COMM_USART_UIRQHandler(void);
  #define COMM_USART_StackUpdate  USART1_IRQHandler
#elif (COMM_INTERFACE_TYPE == 2)
  void  _COMM_USART_UIRQHandler(void);
  #define COMM_USART_StackUpdate  USART2_IRQHandler
#elif (COMM_INTERFACE_TYPE == 3)
  void  _COMM_USART_UIRQHandler(void);
  #define COMM_USART_StackUpdate  USART3_IRQHandler
#elif (COMM_INTERFACE_TYPE == 4)
  void  _COMM_SPI_UIRQHandler(void);
  #define COMM_SPI_StackUpdate  SPI1_IRQHandler
#elif (COMM_INTERFACE_TYPE == 5)
  void  _COMM_SPI_UIRQHandler(void);
  #define COMM_SPI_StackUpdate  SPI2_IRQHandler
#endif
#endif /* ___USE_COMM_REDEF___ */



#endif /* __COMM_H */

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
