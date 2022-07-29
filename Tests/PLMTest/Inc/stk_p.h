/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : stk_p.h
* Author             : Systems LAB - IMS (Industrial & Multisegment Sector)
* Date First Issued  : 05/12/2008
* Description        : Phisical layer routine definitions
********************************************************************************
* History:
* 05/12/2008: V0.1
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STK_P_H
#define __STK_P_H

/* <!> INSERT THE DIRECTIVE #include "stk_p.h" IN THE stm32f10x_it.c FILE */

/* Includes ------------------------------------------------------------------*/
#include "interfaceconfig.h"

/* Private define ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef struct
{
  uint8_t   hh;
  uint8_t   mm;
  uint8_t   ss;
  uint16_t  year;
  uint8_t   month;
  uint8_t   day;
  uint8_t   weekday;
  uint8_t   yearday;
  uint8_t   isdst;
} sytemdate_t;


/* Exported variables ---------------------------------------------------------*/
extern bool  SYSTEM_RESET;

/* Exported constants --------------------------------------------------------*/
#define PL_RTC_NOT_INITIALIZED  0
#define PL_RTC_READY            1
#define PL_RTC_ERROR            2
#define PL_TIME_BUFFER_LEN      10

/* Exported macro ------------------------------------------------------------*/
#define _timestamp        PL_Counter100uS

// Redirection of sys tick handler
#ifndef ___USE_SYS_TICK___
#define SysTick_Handler   _USysTickHandler
#ifdef DEVICE_USE_RTC
#define RTC_IRQHandler    _URTC_RTC_IRQHandler
#endif
#ifdef DEVICE_LSI_TIM_MEASURE
#define TIM5_IRQHandler   _UTIM5_IRQHandler
#endif
#define ___USE_SYS_TICK___
#endif


/* Exported variables --------------------------------------------------------*/
extern  sytemdate_t sys_date;
extern  bool        sys_nextday;
extern  u32         PL_Counter100uS;

/* Exported functions ------------------------------------------------------- */

/* GENERAL */
bool PL_Init(void);
void PL_Delay(uint32_t n100u);
bool PL_DelayElapsed(uint32_t timestamp, uint32_t delay);
u16  PL_GetRND(uint32_t param);
#ifdef DEVICE_LSI_TIM_MEASURE
void PL_LSIint(void);
#endif
/* RTC */
void PL_RTC_LimitCheck(void);
bool PL_ConfigureRTC(uint32_t countval);
bool PL_RTC_SetTime(uint8_t *timebuffer);
void PL_RTC_GetTime(uint8_t *timebuffer);
void PL_RTC_Update(void);
void PL_WDUpdate(void);

#endif /* __STK_P_H */

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
