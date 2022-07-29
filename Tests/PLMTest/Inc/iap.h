/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : iap.h
* Author             : Systems LAB - IMS (Industrial & Multisegment Sector)
* Date First Issued  : 10/12/2008
* Description        : IAP routine definitions
********************************************************************************
* History:
* 10/12/2008: V0.1
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __IAP_H
#define __IAP_H

/* Includes ------------------------------------------------------------------*/
#include "interfaceconfig.h"

/* Exported define ------------------------------------------------------------*/
#define DONGLE_CIS                ((u16)0xca00) // Value assumed as soon as is programmed
#define IAP_PAGE_SIZE             ((u16)0x400)  // 1K bytes
#define IAP_FLASH_WRITE_TOUT      2000          // Timeout (us) for writing each data into the flash memory (200ms max)

/* Exported types ------------------------------------------------------------*/

typedef struct
{
  /* DONGLE DATA */   
  u16 LocalGroup;                     // Local group 
  u32 LocalAddress;                   // Local address 
  u16 FirmwareRelease;                // xxx.yyy
  u8  WorkingMode;                    // Device working mode
  u8  HopLevel;                       // Device hop level (0 = unused)
  u16 IOConfiguration;                // Input Output configuration
  u8  AESkey[AES_KEY_SIZE];           // AES key (if enabled)
  
  /* LL STACK PARAMETERS */    
  u16 LL_SM_MIN_SLOT;                 // Minimum backoff time slot
  u16 LL_SM_MAX_SLOT;                 // Maximum backoff time slot
  u16 LL_SM_GLOBAL_TX_TO;             // Global timeout before frame retransmission if a process is not completed
  u16 LL_SM_BC_GLOBAL_TX_TO;          // 
  u16 LL_SM_ACTIVITY_TO__H;           // 
  u16 LL_SM_ACTIVITY_TO__L;           // 
  u16 LL_SM_WATCHDOG_TO;              // 
  u16 LL_SM_DATATRANSFER_TO;          // 
  u16 LL_SM_BANDINUSE_TO__H;          // 
  u16 LL_SM_BANDINUSE_TO__L;          // 
  u16 LL_SM_FRAME_TX_TO__H;           // 
  u16 LL_SM_FRAME_TX_TO__L;           // 
  u16 LL_SM_BCAST_TX_TO__H;           // 
  u16 LL_SM_BCAST_TX_TO__L;           // 
  u16 LL_SM_ACK_RX_TO__H;             // 
  u16 LL_SM_ACK_RX_TO__L;             // 
  u16 LL_SM_bACK_RX_TO__H;            // 
  u16 LL_SM_bACK_RX_TO__L;            // 
  u16 LL_SM_FRM_RX_TO__H;             // 
  u16 LL_SM_FRM_RX_TO__L;             // 
  u16 LL_SM_NDX_TO__H;                // 
  u16 LL_SM_NDX_TO__L;                // 
  u16 LL_SM_MAX_ATTEMPT;              // 
  u16 LL_SM_RPT_ATTEMPT;              // Number of repetition attempt of a frame sensed with the same ID of one already processed. 0 = dongle is not a repeater
  u16 DEVICE_TIME_SYNC;               // Schedule time for sending a synchronization time frame (0 = feature disabled)
  
  /* USER PROGRAM */   
  u16 USER_SETTINGS[USER_SETTINGS_SIZE];  // User program, 2 byte data for each row
}DEVICE_Data_t;

/* Private variables ---------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern  DEVICE_Data_t DeviceData;

/* Exported functions ------------------------------------------------------- */
void  IAP_GetDeviceData(DEVICE_Data_t *pDeviceData);
bool  IAP_EraseDeviceData(u32 nDataAdd);
bool  IAP_SetDeviceData(DEVICE_Data_t *pDeviceData);

#endif /* __IAP_H */

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/

