/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : stk_l.h
* Author             : Systems LAB - IMS (Industrial & Multisegment Sector)
* Date First Issued  : 05/12/2008
* Description        : Link layer routine definitions
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
#ifndef __STK_L_H
#define __STK_L_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_plm.h"
#include "stk_p.h"

/* Private define ------------------------------------------------------------*/
#define DEVICE_ENCRYPTION_AES         // Enable the use of the encryption AES128
#define LL_HDR_SIZE             19    // Link layer header size (bytes extra added to user payload, apart the len)

/* Exported types ------------------------------------------------------------*/
typedef enum
{
  LLSME_NONE,
  LLSME_FRAME_DELIVERY_IMPOSSIBLE,
  LLSME_BAND_IN_USE_STUCKED,
  LLSME_WATCHDOG_TIMEOUT,
  LLSME_FRAME_RECEIVING_TIMEOUT,
  LLSME_FRAME_TRANSMITING_TIMEOUT,
  LLSME_FRAME_TRANSMISSION_ERROR,
} LL_Errors_t;

typedef enum 
{
  LLSM_NOT_STARTED,
  LLSM_IDLE,
  LLSM_DONE,
  LLSM_ERRORS,
  LLSM_RUNNING,
  LLSM_RESET
}LL_SMS_t;

typedef enum 
{
  LLSM_CLS_DATA_FRAME     = 0x00,
  LLSM_CLS_SERVICE_FRAME  = 0x01,
  LLSM_CLS_PING_FRAME     = 0x02,
  LLSM_CLS_ERROR_FRAME    = 0x03,
  LLSM_CLS_PROGR_FRAME    = 0x04,
  LLSM_CLS_RES_FRAME      = 0x05,
  LLSM_CLS_NONE           = 0x7f
}LL_SMSC_t;

typedef struct 
{
  u8 DEVICE_OPT_ACK;
  u8 DEVICE_OPT_bACK;
  u8 DEVICE_OPT_RPT;
  u8 DEVICE_OPT_RPTALL;
  u8 DEVICE_OPT_GRP;
  u8 DEVICE_OPT_ENC;
}LL_WM_t;

typedef struct 
{
  LL_SMS_t    operation;
  LL_Errors_t error;
}LLSM_t;

typedef struct 
{
  LL_SMSC_t type;
  u16       group;
  u32       address;
  u8        len;
  u8        buffer[PAYLOAD_SIZE];
}LLSMS_t;

typedef struct 
{
  bool  LastFrameRead;
  u8    type;
  u8    FECcorrections;
  bool  WrongPostamble;
  bool  WrongCRC;
  bool  hopoverrun;
  bool  rejected;
}IncFrameSTS_t;

typedef struct 
{
  u16 MIN_SLOT;
  u16 MAX_SLOT;
  u8  GLOBAL_TX_TO;
  u8  BC_GLOBAL_TX_TO;
  u32 ACTIVITY_TO;
  u8  WATCHDOG_TO;
  u8  DATATRANSFER_TO;
  u32 BANDINUSE_TO;
  u32 FRAME_TX_TO;
  u32 BCAST_TX_TO;
  u32 ACK_RX_TO;
  u32 bACK_RX_TO;
  u32 FRM_RX_TO;
  u32 NDX_TO;
  u8  MAX_ATTEMPT;
  u8  MAX_RPT_ATTEMPT;
  u16 TIME_SYNC;
}LL_settings_t;

typedef struct 
{
  u16           group;
  u32           address;
  u8            frameparam;
  u8            hoplevel;
  LL_settings_t setting;
#ifdef DEVICE_ENCRYPTION_AES 
  u8            AESk[AES_KEY_SIZE];
#endif
}LL_device_t;

/* Exported variables ---------------------------------------------------------*/
extern u16            LL_STACK_FW_RELEASE;
extern IncFrameSTS_t  LL_FrameRxStatus;
extern LL_device_t    LL_Device;
extern const u16      LL_TableCRC16[];

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

/* FRAME PARAMETER MASK */
/*
[LL_device_t].frameparam
-------------------------
 b7 b6 b5 b4 b3 b2 b1 b0 
-------------------------
 |  |  |  |  |  |  |  +-- 1 = Frame sent in broadcast
 |  |  |  |  |  |  +----- 1 = ACK frame requested               0 = ACK frame not requested
 |  |  |  |  |  +-------- 1 = bACK frame requested              0 = bACK frame not requested
 |  |  |  |  +----------- 1 = Dongle works also as a repetitor  0 = Dongle doesn't repead frames
 |  |  |  +-------------- 1 = Repeat all frames after ACK time  0 = Repeat only frames with no response
 |  |  +----------------- 1 = Grouping filter (subnet) enabled  0 = Grouping filter disabled
 |  +-------------------- 1 = Encrypted data                    0 = Clear data
 +----------------------- 1 = Reserved
*/
#define LL_FRM_PARAM_DISABLED   0x00  // Function disabled
#define LL_FRM_PARAM_BCAST      0x01  // (b0) Broadcast frame mask
#define LL_FRM_PARAM_ACK_REQ    0x02  // (b1) ACK frame mask
#define LL_FRM_PARAM_bACK_REQ   0x04  // (b2) bACK frame mask
#define LL_FRM_PARAM_REPEAT     0x08  // (b3) Repetition mask
#define LL_FRM_PARAM_REPEAT_ALL 0x10  // (b4) Repeat mode
#define LL_FRM_PARAM_GROUP      0x20  // (b5) Grouping mask
#define LL_FRM_PARAM_ENCRYPTION 0x40  // (b6) Data encryption
//#define LL_FRM_PARAM_           0x80  // (b7) Reserved

#define LL_BROADCAST_FLAG       0x80  // Flag added in the network frame type

/* Exported functions ------------------------------------------------------- */

/* GENERAL */
bool    LL_DeviceStackInit(LL_WM_t wm, LL_settings_t ds);
void    LL_DeviceStackStart(void);
void    LL_DeviceStackRestart(void);
void    LL_ClearRXFstatus(void);
void    LL_DeviceStackUpdate(void);
u16     LL_GetStackRelease(void);
LLSM_t  LL_GetLLSMStatus(void);
void    LL_ClearTimeoutTimer(void);
bool    LL_OperationTimeout(u32 tout);

/* NETWORK */
bool    LL_FrameSendRequest(LLSMS_t data);
LLSMS_t LL_GetIncomingFrame(void);

#endif /* __STK_L_H */

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
