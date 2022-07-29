/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : stm32f10x_plm.h
* Author             : Systems LAB - IMS (Industrial & Multisegment Sector)
* Date First Issued  : 14/06/2012
* Description        : PLM peripheral routine definitions
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
#ifndef __STM32F10X_PLM_H
#define __STM32F10X_PLM_H

/* <!> INSERT THE DIRECTIVE #include "stm32f10x_plm.h" IN THE stm32f10x_it.c FILE */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"

/* Exported macro ------------------------------------------------------------*/

/* <!> DO NOT MODIFY */
#ifndef ___PORT_N_ASS__
#define ___PORT_N_ASS__
#define A   0   // GPIOA
#define B   1   // GPIOB
#define C   2   // GPIOC
#define D   3   // GPIOD
#define E   4   // GPIOE
#define F   5   // GPIOF
#define G   6   // GPIOG
#endif /* ___PORT_N_ASS_ */


/******************************************************************************/
/************************** HARDWARE PIN ASSOCIATION **************************/
/******************************************************************************/
#define PLM_SPI                 1   // SPI 1
#define PLM_SPI_PORT            A   // GPIOA
#define PLM_SCK_PIN             5   // GPIO_Pin_5
#define PLM_MISO_PIN            6   // GPIO_Pin_6
#define PLM_MOSI_PIN            7   // GPIO_Pin_7
#define PLM_CDPD_PORT           A   // GPIOA
#define PLM_CDPD_PIN            2   // GPIO_Pin_2
#define PLM_BU_PORT             A   // GPIOA
#define PLM_BU_PIN              3   // GPIO_Pin_3
#define PLM_REGDATA_PORT        A   // GPIOA
#define PLM_REGDATA_PIN         1   // GPIO_Pin_1
#define PLM_RXTX_PORT           A   // GPIOA
#define PLM_RXTX_PIN            4   // GPIO_Pin_4
/******************************************************************************/


/* PARAMETERS */
#define HEADER_PREAMBLE_VALUE   0xE389  // Hardware header/preamble value
#define PAYLOAD_SIZE            100     // Max user payload <!> Must not exceed 100 bytes. 
                                        // <!> If AES encryption is used, payload size cannot exceed (PAYLOAD_SIZE \ AES_KEY_SIZE) bytes 
                                        //     (96 bytes if AES_KEY_SIZE = 16)

/* <!> DO NOT MODIFY THIS VALUE */
#define PLM_RESERVED_BYTES      19      // Reserved bytes for data link layer added data (if used)

/* Exported types ------------------------------------------------------------*/
typedef enum 
{
  PLM_FLAG_FALSE = 0, 
  PLM_FLAG_TRUE = (!(PLM_FLAG_FALSE))
}plm_flag_t;

typedef struct
{
  uint8_t   framebuffer[PAYLOAD_SIZE + PLM_RESERVED_BYTES];
  uint8_t   len;
}plmframe_t;

typedef enum
{
  SPI_TX_MODE,
  SPI_RX_MODE,
  SPI_HDR_MODE
}SPI_Mode_t;

typedef struct
{ 
  /* Interrupt priorities */
  uint8_t         PLM_SPI_PreemptyPriority;
  uint8_t         PLM_SPI_SubPriority;
  uint8_t         PLM_CDPD_PreemptyPriority;
  uint8_t         PLM_CDPD_SubPriority;
  
}PLM_IntPriorityInitTypeDef;

typedef enum
{
  PLM_PB_FREQ_60KHz   = 0x10,
  PLM_PB_FREQ_66KHz   = 0x11,
  PLM_PB_FREQ_72KHz   = 0x12,
  PLM_PB_FREQ_76KHz   = 0x13,
  PLM_PB_FREQ_82KHz   = 0x14,
  PLM_PB_FREQ_86KHz   = 0x15,
  PLM_PB_FREQ_110KHz  = 0x16,
  PLM_PB_FREQ_132KHz  = 0x17
}PLMFreq_TypeDef;

typedef enum
{
  PLM_BAUDRATE_600  = 0x00,
  PLM_BAUDRATE_1200 = 0x08,
  PLM_BAUDRATE_2400 = 0x10,
  PLM_BAUDRATE_4800 = 0x18
}PLMBaudrate_TypeDef;

typedef enum
{
  PLM_DEVIATION_05  = 0x00,
  PLM_DEVIATION_1  = 0x20
}PLMDeviation_TypeDef;

typedef enum
{
  PLM_PRE_FILTER_OFF  = 0x00,
  PLM_PRE_FILTER_ON  = 0x80
}PLMPrefilter_TypeDef;

typedef enum
{
  PLM_SENSITIVITY_NORMAL  = 0x00,
  PLM_SENSITIVITY_HIGH  = 0x40
}PLMSensitivity_TypeDef;

typedef enum
{
  PLM_POSTAMBLE_OK  = 0,
  PLM_POSTAMBLE_WRONG  = !0
}PLM_WP_TypeDef;

typedef struct
{
  PLMFreq_TypeDef         PLM_BandpassFrequency;
  PLMBaudrate_TypeDef     PLM_BaudRate;
  uint16_t                PLM_HeaderPreamble;
  PLMDeviation_TypeDef    PLM_Deviation;
  PLMPrefilter_TypeDef    PLM_PreFilter;
  PLMSensitivity_TypeDef  PLM_Sensitivity;
}PLM_InitTypeDef;


#define IS_PLM_FREQUENCY(FREQUENCY)     (((FREQUENCY) >= ((uint8_t)PLM_PB_FREQ_60KHz) && \
                                        ((FREQUENCY) <= ((uint8_t)PLM_PB_FREQ_132KHz))

#define IS_PLM_BAUDRATE(BAUDRATE)       (((BAUDRATE) == PLM_BAUDRATE_600) || \
                                        ((BAUDRATE) == PLM_BAUDRATE_1200) || \
                                        ((BAUDRATE) == PLM_BAUDRATE_2400) || \
                                        ((BAUDRATE) == PLM_BAUDRATE_4800))

#define IS_PLM_DEVIATION(DEVIATION)     (((DEVIATION) == PLM_DEVIATION_05) || \
                                        ((DEVIATION) == PLM_DEVIATION_1))

#define IS_PLM_PREFILTER(PREFILTER)     (((PREFILTER) == PLM_PRE_FILTER_OFF) || \
                                        ((PREFILTER) == PLM_PRE_FILTER_ON))

#define IS_PLM_SENSITIVITY(SENSITIVITY) (((SENSITIVITY) == PLM_SENSITIVITY_NORMAL) || \
                                        ((SENSITIVITY) == PLM_SENSITIVITY_HIGH))

/* Exported constants --------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void            PLM_PeripheralInit(PLM_IntPriorityInitTypeDef *PLM_PeripInitStruct);
void            PLM_Init(PLM_InitTypeDef *PLM_InitStruct);

void            PLM_Reset(void);
uint8_t         PLM_BandInUse(void);
uint8_t         PLM_FrameSent(void);
PLM_WP_TypeDef  PLM_GetWrongPostamble(void);
uint8_t         PLM_GetFECCorrections(void);
void            PLM_WriteRegisters(u8 *buffer, u8 len);
void            PLM_StartReadRegisters(void);
void            PLM_SendData(uint8_t *buffer, uint8_t len);
void            PLM_StartReceiveData(void);
uint8_t         PLM_FrameArrived(void);
uint8_t         PLM_ValidFrameArrived(void);
uint8_t         PLM_GetBufferLength(void);
uint8_t*        PLM_GetReceivedBufferPointer(void);

/* --------------------------------------------------------------------------- */
/* <!> DO NOT MODIFY */
/* --------------------------------------------------------------------------- */
#ifndef ___USE_PLM_REDEF___
#define ___USE_PLM_REDEF___
#if (PLM_SPI == 1)
  #define SPI1_IRQHandler     _SPI_UIRQHandler
#elif (PLM_SPI == 1)
  #define SPI2_IRQHandler     _SPI_UIRQHandler
#else
  #define SPI3_IRQHandler     _SPI_UIRQHandler
#endif

#if (PLM_CDPD_PIN == 0)
  #define EXTI0_IRQHandler  _EXTI_UIRQHandler
#elif (PLM_CDPD_PIN == 1)
  #define EXTI1_IRQHandler  _EXTI_UIRQHandler
#elif (PLM_CDPD_PIN == 2)
  #define EXTI2_IRQHandler  _EXTI_UIRQHandler
#elif (PLM_CDPD_PIN == 3)
  #define EXTI3_IRQHandler  _EXTI_UIRQHandler
#elif (PLM_CDPD_PIN == 4)
  #define EXTI4_IRQHandler  _EXTI_UIRQHandler
#elif (PLM_CDPD_PIN > 4) && (PLM_CDPD_PIN < 10)
  #define EXTI9_5_IRQHandler  _EXTI_UIRQHandler
#else
  #define EXTI15_10_IRQHandler  _EXTI_UIRQHandler
#endif

#endif /*  ___USE_PLM_REDEF___ */
#endif /* __STM32F10X_PLM_H */

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
