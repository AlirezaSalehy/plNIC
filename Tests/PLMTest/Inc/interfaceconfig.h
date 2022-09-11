/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : interfaceconfig.h
* Author             : Systems LAB - IMS (Industrial & Multisegment Sector)
* Date First Issued  : 10/02/2012
* Description        : User interface definition module
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
#ifndef __USER_INTERFACECONFIG_H
#define __USER_INTERFACECONFIG_H


/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"

/*============================================================================================================================================*/
/* APPLICATION PARAMETERS */
/*============================================================================================================================================*/

/**********************************************************************************************************************************************/
/* HARDWARE PINS ASSOCIATION */
/**********************************************************************************************************************************************/

/*----------------------------------------------------------------------------*/
/* LEDs port and pin definition                                               */
/*----------------------------------------------------------------------------*/
#define LED_RED_PORT     B
#define LED_RED_PIN      8

#define LED_GREEN_PORT   B
#define LED_GREEN_PIN    9

/*----------------------------------------------------------------------------*/
/* Inputs/Outputs port and pin definition                                     */
/*----------------------------------------------------------------------------*/
#define IO_1_PORT                     B     // GPIOB 
#define IO_1_PIN                      0     // GPIO_Pin_0
#define IO_2_PORT                     B     // GPIOB 
#define IO_2_PIN                      1     // GPIO_Pin_1
#define IO_3_PORT                     B     // GPIOB 
#define IO_3_PIN                      2     // GPIO_Pin_2
#define IO_4_PORT                     B     // GPIOB 
#define IO_4_PIN                      10    // GPIO_Pin_10
#define IO_5_PORT                     B     // GPIOB 
#define IO_5_PIN                      11    // GPIO_Pin_11
#define IO_6_PORT                     B     // GPIOB 
#define IO_6_PIN                      5     // GPIO_Pin_5
#define IO_7_PORT                     B     // GPIOB 
#define IO_7_PIN                      6     // GPIO_Pin_6
#define IO_8_PORT                     B     // GPIOB 
#define IO_8_PIN                      7     // GPIO_Pin_7

/* Default I/O pin definition (1 = output) */
#define IO_DEFAULT_CONFIG             0x00  // All IO's as inputs 
#define IO_DEFAULT_OUT_VAL            0x00  // All outputs cleared

/*----------------------------------------------------------------------------*/
/* Communication peripheral: USART (UNCOMMENT ALL FOR USE USART)              */
/*----------------------------------------------------------------------------*/
#define COMM_INTERFACE_TYPE           COMM_USART1   // COMM_USARTx - x = 1, 2, 3 
#define COMM_USART_TX_PORT            A             // GPIOA
#define COMM_USART_RX_PORT            A             // GPIOA 
#define COMM_USART_TX_PIN             9             // GPIO_Pin_9
#define COMM_USART_RX_PIN             10            // GPIO_Pin_10

#define COMM_USART_START_TX_TIMEOUT   50            // 50ms timeout for starting the communication
#define COMM_USART_BAUDRATE           9600          // USART baudrate

/*----------------------------------------------------------------------------*/
/* Communication peripheral: SPI (UNCOMMENT ALL FOR USE SPI) - NOT YET TESTED */
/*----------------------------------------------------------------------------*/
// PERIPHERAL SELECTION: SPI DEFINITIONS (UNCOMMEN FOR USE SPI)
//#define COMM_INTERFACE_TYPE         COMM_SPI1       // SPIx - x = 1, 2
//#define COMM_SPI_PORT               GPIOB
//#define COMM_SCK_PIN                GPIO_Pin_13
//#define COMM_MISO_PIN               GPIO_Pin_14
//#define COMM_MOSI_PIN               GPIO_Pin_15
//
//#define COMM_SPI_EDGE               SPI_CPHA_1Edge
//#define COMM_SPI_POL                SPI_CPOL_Low
//#define COMM_SPI_PRESCALER          SPI_BaudRatePrescaler_256
//#define COMM_SPI_FIRST_BIT          SPI_FirstBit_MSB
//#define COMM_SPI_NSS_MODE           SPI_NSS_Soft
////#define COMM_SPI_MISO_FLOATING      // Uncomment for configure MISO pin as Input Floating
//
//#define COMM_SPI_START_TX_TIMEOUT   300            // 0.3s timeout for starting the communication (set to 0 if not used)

/*----------------------------------------------------------------------------*/
/* Communication peripheral: USB (UNCOMMENT ALL FOR USE USB)                  */
/*----------------------------------------------------------------------------*/
//#define COMM_INTERFACE_TYPE         COMM_USB      // USB2.0 - TO BE IMPLEMENTED


/**********************************************************************************************************************************************/
/* FIRMWARE RELEASE */
/**********************************************************************************************************************************************/
#define DEVICE_FIRMWARE_RELEASE_X     6   // Firmware Release (x).y
#define DEVICE_FIRMWARE_RELEASE_Y     7   // Firmware Release x.(y)

/**********************************************************************************************************************************************/
/* INTERNAL FLASH PROGRAMMING DEFINITIONS - <!> WARNING - MODIFY WITH CARE */
/**********************************************************************************************************************************************/
/*  +========================================+ [0x0807FC00]
    |    2 bytes - device programmed flag    |
    +----------------------------------------+ [0x0807FC02]
    |   48 bytes - device data               |  
    +----------------------------------------+ [0x0807FC32]
    |  100 bytes - link layer (stack) data   |  
    +----------------------------------------+ [0x0807FC96]
    |  874 bytes (max) - user program data   |
    +========================================+ [0x0807FFFF]
    
    MAX DATA SEGMENT FROM 0x0807FC00: 1K    (0x0807FFFF) */
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
//#define IAP_BASE_ADDRESS                ((u32)0x0807FC00)     // IAP base address (first location is used for device programmed flag)
//#define IAP_DATA_BASE_ADDRESS           ((u32)0x0807FC02)     // Device data base address 
//#define IAP_LL_PARAM_BASE_ADDRESS       ((u32)0x0807FC32)     // Link layer (stack) base address
//#define IAP_USER_SETTINGS_BASE_ADDRESS  ((u32)0x0807FC96)     // User program base address
//#define USER_SETTINGS_SIZE              10                    // 20 bytes of user settings (10 words, unsigned int)

/* 64Kb Flash Size */
//#define IAP_BASE_ADDRESS                ((u32)0x0800EA00)     // IAP base address (first location is used for device programmed flag)
//#define IAP_DATA_BASE_ADDRESS           ((u32)0x0800EA02)     // Device data base address 
//#define IAP_LL_PARAM_BASE_ADDRESS       ((u32)0x0800EA32)     // Link layer (stack) base address
//#define IAP_USER_SETTINGS_BASE_ADDRESS  ((u32)0x0800EA96)     // User program base address
#define IAP_BASE_ADDRESS                ((u32)0x0801F800)     // IAP base address (first location is used for device programmed flag)
#define IAP_DATA_BASE_ADDRESS           ((u32)0x0801F802)     // Device data base address 
#define IAP_LL_PARAM_BASE_ADDRESS       ((u32)0x0801F832)     // Link layer (stack) base address
#define IAP_USER_SETTINGS_BASE_ADDRESS  ((u32)0x0801F896)     // User program base address

#define USER_SETTINGS_SIZE              16                    // 32 bytes of user settings (16 words, unsigned int)
#define FIRMWARE_SIGNATURE_BUFFER_SIZE  4                     // 4 bytes for firmware signature (simple checksum in this version)

/**********************************************************************************************************************************************/
/* COMMUNICATION DEFINITION */
/**********************************************************************************************************************************************/
#define COMM_SEND_ACK_FRAME                     // Enable the ACK response frame from communication interface (RS232, SPI)
#define COMM_SEND_ERROR_FRAME                   // Enable the error response frame from communication interface (RS232, SPI)
#define APP_NTW_INDICATION_TOUT         0       // Timeout for "Indication" service (0 = disabled)
#define APP_NTW_REQUEST_TOUT            30000   // Timeout for "Request" service (30 s)

/**********************************************************************************************************************************************/

/*============================================================================================================================================*/
/* DATA LINK STACK PARAMETERS */
/*============================================================================================================================================*/

/*-------------------------------------------------------------------------------------------------------------------------------------------*/
/* WORKING MODE DEFINITION */
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
#define DEVICE_DEFAULT_GROUP            0xffff
#define DEVICE_DEFAULT_ADDRESS          0xffffffff


/* Comment the definition for disable the equivalent feature */
#define DEVICE_USE_RTC                  // Use the RTC
//#define DEVICE_USE_EXTOSC_32KHZ         // Use the external 32.769 KHz oscillator for RTC functions
//#define DEVICE_USE_WATCHDOG             // Enable the watchdog 
//#define DEVICE_LSI_TIM_MEASURE          // Enable the internal oscillator calibration (if external oscillator is not used)

#define DEVICE_WM_REQ_ACK               // Enable the ACK frames transmission (link stack level)
#define DEVICE_WM_REQ_bACK              // Enable the back ACK (bACK) frames transmission (link stack level)
#define DEVICE_WM_REPEATER              // Enable the repeater mode
//#define DEVICE_WM_REPEAT_ALL            // Enable to repeat all frames after ACK tout (static repeater behaviour)
//#define DEVICE_WM_ENCRYPT_DATA          // Enable the data encryption
//#define DEVICE_WM_GROUP_FILTER          // Enable the grouping filter for subnets
//#define DEVICE_HOP_LEVEL                // Hoping mecanism (comment, 0 or 1 = disabled, higher value = higher level. Minimum allowed value = 2)

/* PLM communication frequency */
//#define PLM_FREQ_60KHZ                  // PLM communication frequency at 60 KHz
//#define PLM_FREQ_66KHZ                  // PLM communication frequency at 66 KHz
//#define PLM_FREQ_72KHZ                  // PLM communication frequency at 72 KHz
//#define PLM_FREQ_76KHZ                  // PLM communication frequency at 76 KHz
//#define PLM_FREQ_82KHZ                  // PLM communication frequency at 82.05 KHz
//#define PLM_FREQ_86KHZ                  // PLM communication frequency at 86 KHz
//#define PLM_FREQ_110KHZ                 // PLM communication frequency at 110 KHz
#define PLM_FREQ_132KHZ                 // PLM communication frequency at 132.6 KHz

/* PLM interface communication baudrate */
//#define PLM_600_BPS                       // PLM communication baudrate 600 bps
//#define PLM_1200_BPS                      // PLM communication baudrate 1200 bps
#define PLM_2400_BPS                      // PLM communication baudrate 2400 bps
//#define PLM_4800_BPS                      // PLM communication baudrate 4800 bps

/* PLM Sensitivity */
#define PLM_SENS_HIGH                     // High sensitivity
//#define PLM_SENS_NORMAL                   // Normal sensitivity

/*-------------------------------------------------------------------------------------------------------------------------------------------*/
/* AES KEY */
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
#define DEVICE_AES_KEY                  0x53,0x54,0x4d,0x2d,0x53,0x79,0x73,0x74,0x65,0x6d,0x4c,0x61,0x62,0x2d,0x43,0x52 // AES 128 key for data unlock 
#define AES_KEY_SIZE                    16    // AES 128 key size 

/*-------------------------------------------------------------------------------------------------------------------------------------------*/
/* DEFAULT LINK LAYER STACK DATA VALUES - <!> WARNING - MODIFY WITH CARE */
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
#define	DEFAULT_MIN_SLOT                (u16)(3)      // 300 us
#define	DEFAULT_MAX_SLOT	        (u16)(500)    // 50 ms
#define DEFAULT_NTW_P_GLOBAL_TX_TO      (u8)(10)      // 10s: Timeout for global tx (20sec - 10sec step)
#define DEFAULT_NTW_P_BC_GLOBAL_TX_TO   (u8)(15)      // 15s: Timeout for broadcast frame retransmission
#define DEFAULT_NTW_P_WATCHDOG_TO       (u8)(6)       // 1min Watchdog timeout
#define DEFAULT_NTW_P_DATATRANSFER_TO   (u8)(3)       // 3s: Timeout for completing a datatransfer
#define DEFAULT_NTW_P_BANDINUSE_TO      (u32)(0)      // 0 = infinitive: Timeout for band in use
#define DEFAULT_NTW_P_FRAME_TX_TO       (u32)(12000)  // 1.2s: Timeout for sending a frame through SPI 
#define DEFAULT_NTW_P_BCAST_TX_TO       (u32)(120000) // 12s: Timeout for repeat a broadcast frame
#define DEFAULT_NTW_P_ACK_RX_TO         (u32)(2500)   // 250ms: Timeout for receiving an ACK {@ 2400bps PLM speed (worst case)}
#define DEFAULT_NTW_P_bACK_RX_TO        (u32)(2500)   // 250ms: Timeout for receiving a bACK {@ 2400bps PLM speed (worst case)}
#define DEFAULT_NTW_P_FRM_RX_TO         (u32)(0)      // O: Timeout for frame receptions (receiver)
#define DEFAULT_NTW_P_NDX_TO            (u32)(100)    // 10ms: Minimum delay before any transmission (for state machines status update)
#define DEFAULT_LL_MAX_ATTEMPT          (u8)(5)       // Max attempts if no activity is detected
#define DEFAULT_MAX_RPT_ATTEMPT         (u8)(0)       // Max repetition attempts (for processed ID, 0 = disabled)
#define DEFAULT_NTW_P_ACTIVITY_TO       (u32)(20000)  // 2s: Timeout for network inactivity (must be > than NTW_P_ACK_RX_TO)
#define DEFAULT_DEVICE_TIME_SYNC        (u16)(0)      // Time update sync frame inteval (0 = disabled)

/*-------------------------------------------------------------------------------------------------------------------------------------------*/
/* INTERRUPT PRIORITIES - <!> WARNING - MODIFY WITH CARE */
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
#define INTERFACE_PRIORITY_GROUP            NVIC_PriorityGroup_4
#define PLM_CDPD_SUB_PRIORITY               0
#define PLM_CDPD_PREEMPTY_PRIORITY          0
#define SYT_INT_PREEMPTY_PRIORITY           1
#define SYT_INT_SUB_PRIORITY                0
#define PLM_SPI_INT_PREEMPTY_PRIORITY       2
#define PLM_SPI_INT_SUB_PRIORITY            0
#define COMM_INT_PREEMPTY_PRIORITY          3
#define COMM_INT_SUB_PRIORITY               0
#define RTC_PREEMPTY_PRIORITY               4
#define RTC_SUB_PRIORITY                    0

/*-------------------------------------------------------------------------------------------------------------------------------------------*/
/* GENERAL - <!> WARNING - DO NOT MODIFY */
/*-------------------------------------------------------------------------------------------------------------------------------------------*/
#define USER_PAYLOAD_SIZE               100                                   // Max size for user opayload <!> Must not exceed 100 bytes
#define SYS_TICK_CLK_DIVIDER            10000                                 // Divider for sys tick 10000 = 100us
#define COMM_NONE                       0
#define COMM_USART1                     1                                     // USART 1 as COMM interface 
#define COMM_USART2                     2                                     // USART 2 as COMM interface 
#define COMM_USART3                     3                                     // USART 3 as COMM interface 
#define COMM_SPI1                       4                                     // SPI 1 as COMM interface 
#define COMM_SPI2                       5                                     // SPI 2 as COMM interface 
#define COMM_USB                        6                                     // USB as COMM interface 

typedef enum {FALSE = 0, TRUE = !FALSE} bool;
#ifndef ___PORT_N_ASS_
#define ___PORT_N_ASS_
#define A   0
#define B   1
#define C   2
#define D   3
#define E   4
#define F   5
#define G   6
#endif /* ___PORT_N_ASS_ */

/*============================================================================================================================================*/


#endif /* __USER_INTERFACECONFIG_H */

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
