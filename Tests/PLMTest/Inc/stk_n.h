/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : stk_n.h
* Author             : Systems LAB - IMS (Industrial & Multisegment Sector)
* Date First Issued  : 05/12/2008
* Description        : Transport layer routine definitions
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
#ifndef __STK_N_H
#define __STK_N_H

/* Includes ------------------------------------------------------------------*/
#include "stk_l.h"
#include "interfaceconfig.h"

/* Private define ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
typedef enum 
{
  N_SUCCESS,
  N_DOING,
  N_ERROR,
}NL_STS_t;

typedef enum
{
  NL_ERR_NONE,
  NL_ERR_NOT_READY,
  NL_ERR_NETWORK_TIMEOUT,
  NL_ERR_TRANSMISSION_ERROR,
  NL_ERR_COMMUNICATION_TIMEOUT,
  NL_ERR_LINE_ERROR,
  NL_ERR_TARGET_NOT_REACHABLE,
  NL_ERR_FRAME_CLASS_UNKNOWN,
  NL_ERR_GENERIC
}NL_ERR_t;

typedef struct
{
  NL_STS_t operation;
  NL_ERR_t error;
}NL_Status_t;

// Frame type
typedef enum 
{
  NL_CLS_PING_FRAME     = 0x00, // Ping frame
  NL_CLS_SERVICE_FRAME  = 0x01, // Service frame (user or native)
  NL_CLS_DATA_FRAME     = 0x02, // Generic data frame
  NL_CLS_ERROR_FRAME    = 0x03, // Error frame type (processed as data frame)
  NL_CLS_PROGR_FRAME    = 0x04, // Service frame type (processed as data frame)
  NL_CLS_RES_FRAME      = 0x05  // Request response frame type (processed as data frame)
}NL_Type_t;

typedef enum 
{
  NL_TF_DATA    = 0x00,
  NL_TF_ERROR   = 0x01,
  NL_TF_PROGR   = 0x02,
  NL_TF_RES     = 0x03,
  NL_TF_ACK     = 0x04,
  NL_TF_bACK    = 0x05,
  NL_TF_PING    = 0x06,
  NL_TF_SERVICE = 0x07,
  NL_TF_UNKNOWN = 0x7f
}NL_TransitFrame_t;

typedef struct 
{
  u16       group;
  u32       address;
  u8        framelen;
  NL_Type_t frametype;
  u8        databuffer[PAYLOAD_SIZE];
}NL_Data_t;

typedef struct 
{
  NL_TransitFrame_t type;
  u8                FECcorrections;
  bool              wrongpostamble;
  bool              wrongCRC;
  bool              hopoverrun;
  bool              framerejected;
}NL_FrameFlag_t;

typedef struct 
{
  u16 minslot;
  u16 maxslot;
  u8  globaltxto;
  u8  bcglobaltxto;
  u32 activityto;
  u8  watchdogto;
  u8  datatransferto;
  u32 bandinuseto;
  u32 frametxto;
  u32 bcasttxto;
  u32 ackrxto;
  u32 backrxto;
  u32 frmrxto;
  u32 ndxto;
  u8  maxattempt;
  u8  maxrptattempt;
  u16 timesync;
}NL_DLSP_t;

typedef enum
{
  F_BROADCAST  = 0x01,
  F_ACK        = 0x02,
  F_bACK       = 0x04, 
  F_REPEATER   = 0x08,
  F_GROUP      = 0x10
}NL_WMFlag_t;

typedef enum
{
  F_ENABLED,
  F_DISABLED
}NL_WMFlag_status_t;

/* Private variables ---------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/* GENERAL */
bool          NL_NetworkInit(LL_WM_t wm, LL_settings_t ds);
void          NL_NetworkRestart(void);
void          NL_DeviceStackUpdate(void);

/* NETWORKING */
NL_Status_t   NL_NetworkRequest(NL_Data_t *ntw_data, u32 tout);
NL_Status_t   NL_NetworkIndication(NL_Data_t *ntw_data, u32 tout);

/* NETWORK SERVICES */
uint8_t       NL_GetLocalWorkingMode(void);
uint8_t       NL_GetLocalHopLevel(void);
bool          NL_GetFrameRxFlags(NL_FrameFlag_t *flags);
void          NL_GetLocalAddress(u16 *nDevGrp, u32 *nDevAdd);
void          NL_SetLocalAddress(u16 nDevGrp, u32 nDevAdd);
void          NL_SetLocalWorkingMode(u8 wmode);
void          NL_SetLocalWorkingModeFlag(NL_WMFlag_t flag, NL_WMFlag_status_t sts);
void          NL_SetLocalHopLevel(u8 hlevel);
bool          NL_BrokenFrameArrived(void);
#ifdef DEVICE_ENCRYPTION_AES
void          NL_SetEncryptionKey(u8 *Ekey);
#endif

/* NETWORK SETTINGS */
void          NL_GetDataLinkStackParameters(NL_DLSP_t *llsp);
void          NL_SetDataLinkStackParameters(NL_DLSP_t llsp);
uint16_t      NL_GetStackFirmwareRelease(void);

#endif /* __STK_N_H */

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/

