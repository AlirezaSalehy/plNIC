/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : iap.c
* Author             : Systems LAB - IMS (Industrial & Multisegment Sector)
* Date First Issued  : 10/12/2008
* Description        : In Application Programming (IAP) routines
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

/******************************************************************************/


/* Includes ------------------------------------------------------------------*/
#include "iap.h"
#include "stk_p.h"
#include "stm32f10x_flash.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Privte variables ---------------------------------------------------------*/
DEVICE_Data_t DeviceData;

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
u32 IAP_GetNextLocation(u32 *add);

/* Private functions ---------------------------------------------------------*/
bool IAP_SetDataLocation(u32 address, u16 param);

/*******************************************************************************
* Function Name  : IAP_GetNextLocation
* Description    : Return the next progressive location
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u32 IAP_GetNextLocation(u32 *add)
{
  (*add)+=2;
  return (*add);
}

/*******************************************************************************
* Function Name  : IAP_GetDeviceData
* Description    : Get the dongle data stored in the IAP_DATA_BASE_ADDRESS sector
* Input          : Pointer to the data structure
* Output         : None
* Return         : None
*******************************************************************************/
void IAP_GetDeviceData(DEVICE_Data_t *pDeviceData)
{
  u8 i;
  u32 DATA_ADD, addH, addL, wm;
#ifdef DEVICE_ENCRYPTION_AES  
  u8 e, tempAES[AES_KEY_SIZE + 1];  
#endif
  /* Set the firmware release */
  pDeviceData->FirmwareRelease = (((u16)(DEVICE_FIRMWARE_RELEASE_X)) << 8) | (((u16)(DEVICE_FIRMWARE_RELEASE_Y)) & 0x0ff);
   
 /* Device data */
  DATA_ADD = IAP_DATA_BASE_ADDRESS;
  
  pDeviceData->LocalGroup = (*(vu16*)(DATA_ADD));
  addH = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  addL = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->LocalAddress = (addH << 16) | addL;
  wm = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->WorkingMode = (u8)(wm >> 8);
  pDeviceData->HopLevel = (u8)wm;
  pDeviceData->IOConfiguration = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));

#ifdef DEVICE_ENCRYPTION_AES
  /* AES key */
  e = (AES_KEY_SIZE / 2);
  if (AES_KEY_SIZE % 2)
    e += 1;
  for (i = 0; i < e; i++)
  {
    wm = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
    tempAES[2 * i] = (u8)(wm >> 8);
    tempAES[(2 * i) + 1] = (u8)wm;
  }
  for (i = 0; i < AES_KEY_SIZE; i++)
    pDeviceData->AESkey[i] = tempAES[i];
#endif
  
  /* LL Stack parameters */
  DATA_ADD = IAP_LL_PARAM_BASE_ADDRESS;
  
  pDeviceData->LL_SM_MIN_SLOT = (*(vu16*)(DATA_ADD));
  pDeviceData->LL_SM_MAX_SLOT = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->LL_SM_GLOBAL_TX_TO = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->LL_SM_BC_GLOBAL_TX_TO = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->LL_SM_ACTIVITY_TO__H = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->LL_SM_ACTIVITY_TO__L = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->LL_SM_WATCHDOG_TO = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->LL_SM_DATATRANSFER_TO = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->LL_SM_BANDINUSE_TO__H = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->LL_SM_BANDINUSE_TO__L = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->LL_SM_FRAME_TX_TO__H = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->LL_SM_FRAME_TX_TO__L = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->LL_SM_BCAST_TX_TO__H = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->LL_SM_BCAST_TX_TO__L = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->LL_SM_ACK_RX_TO__H = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->LL_SM_ACK_RX_TO__L = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->LL_SM_bACK_RX_TO__H = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->LL_SM_bACK_RX_TO__L = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->LL_SM_FRM_RX_TO__H = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->LL_SM_FRM_RX_TO__L = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->LL_SM_NDX_TO__H = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->LL_SM_NDX_TO__L = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->LL_SM_MAX_ATTEMPT = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->LL_SM_RPT_ATTEMPT = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  pDeviceData->DEVICE_TIME_SYNC = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
  
  /* User program */
  DATA_ADD = IAP_USER_SETTINGS_BASE_ADDRESS;
  pDeviceData->USER_SETTINGS[0] = (*(vu16*)(DATA_ADD));
  for (i = 1; i < USER_SETTINGS_SIZE; i++)
    pDeviceData->USER_SETTINGS[i] = (*(vu16*)(IAP_GetNextLocation(&DATA_ADD)));
}

/*******************************************************************************
* Function Name  : IAP_EraseDeviceData
* Description    : Erase the Flash page dedicated to the data storage
* Input          : Data address
* Output         : None
* Return         : None
*******************************************************************************/
bool IAP_EraseDeviceData(u32 nDataAdd)
{
  u32 tout;
  
  /* Unlock the Flash Program Erase controller */
  FLASH_Unlock();
  
  /* Clear All pending flags */
  FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	

  tout = _timestamp;
  
  /* Erase the dedicated FLASH page */
  while (FLASH_ErasePage(nDataAdd) != FLASH_COMPLETE)
  {
    if (PL_DelayElapsed(tout, IAP_FLASH_WRITE_TOUT))
      return FALSE;
  }
  return TRUE;
}

/*******************************************************************************
* Function Name  : IAP_SetDataLocation
* Description    : Write data in a specific location
* Input          : address, data
* Output         : None
* Return         : None
*******************************************************************************/
bool IAP_SetDataLocation(u32 address, u16 param)
{
  u32 tout;

  tout = _timestamp;
  while (FLASH_ProgramHalfWord(address, param) != FLASH_COMPLETE)
  {
    if (PL_DelayElapsed(tout, IAP_FLASH_WRITE_TOUT))
      return FALSE;
  }
  return TRUE;
}

/*******************************************************************************
* Function Name  : IAP_SetDeviceData
* Description    : Write the dongle data into the flash 
* Input          : Pointer to the data structure
* Output         : None
* Return         : None
*******************************************************************************/
bool IAP_SetDeviceData(DEVICE_Data_t *pDeviceData)
{
  u32 DATA_ADD;
  u8 i;
#ifdef DEVICE_ENCRYPTION_AES
  u8 e;
#endif
  
  /* IAP base data */  
  DATA_ADD = IAP_BASE_ADDRESS;
  
  /* Clear flash area */
  IAP_EraseDeviceData(DATA_ADD);
  
  if (!IAP_SetDataLocation(DATA_ADD, DONGLE_CIS)) // Device programmed flag
    return FALSE;
  
  /* Device data */
  DATA_ADD = IAP_DATA_BASE_ADDRESS;
      
  if (!IAP_SetDataLocation(DATA_ADD, pDeviceData->LocalGroup))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), (u16)(pDeviceData->LocalAddress >> 16)))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), (u16)(pDeviceData->LocalAddress & 0x0ffff)))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), (((u16)(pDeviceData->WorkingMode))<<8) | ((u16)(pDeviceData->HopLevel))))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), (u16)(pDeviceData->IOConfiguration)))
    return FALSE;

#ifdef DEVICE_ENCRYPTION_AES
  /* AES key */
  e = (AES_KEY_SIZE / 2);
  if (AES_KEY_SIZE % 2)
    e += 1;
  for (i = 0; i < e; i++)
  {
    if (!((AES_KEY_SIZE % 2) && (i == (e - 1))))
    {
      if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), (((u16)(pDeviceData->AESkey[2 * i]))<<8) | ((u16)(pDeviceData->AESkey[(2 * i) + 1]))))
        return FALSE;
    }
    else
    {
      if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), (u16)(pDeviceData->AESkey[2 * i])<<8))
        return FALSE;
    }
  }
#endif  
  /* LL Stack parameters */
  DATA_ADD = IAP_LL_PARAM_BASE_ADDRESS;
  
  if (!IAP_SetDataLocation(DATA_ADD, pDeviceData->LL_SM_MIN_SLOT))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->LL_SM_MAX_SLOT))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->LL_SM_GLOBAL_TX_TO))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->LL_SM_BC_GLOBAL_TX_TO))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->LL_SM_ACTIVITY_TO__H))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->LL_SM_ACTIVITY_TO__L))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->LL_SM_WATCHDOG_TO))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->LL_SM_DATATRANSFER_TO))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->LL_SM_BANDINUSE_TO__H))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->LL_SM_BANDINUSE_TO__L))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->LL_SM_FRAME_TX_TO__H))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->LL_SM_FRAME_TX_TO__L))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->LL_SM_BCAST_TX_TO__H))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->LL_SM_BCAST_TX_TO__L))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->LL_SM_ACK_RX_TO__H))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->LL_SM_ACK_RX_TO__L))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->LL_SM_bACK_RX_TO__H))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->LL_SM_bACK_RX_TO__L))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->LL_SM_FRM_RX_TO__H))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->LL_SM_FRM_RX_TO__L))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->LL_SM_NDX_TO__H))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->LL_SM_NDX_TO__L))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->LL_SM_MAX_ATTEMPT))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->LL_SM_RPT_ATTEMPT))
    return FALSE;
  if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->DEVICE_TIME_SYNC))
    return FALSE;

  /* User program */
  DATA_ADD = IAP_USER_SETTINGS_BASE_ADDRESS;
  
  // User program loading 
  if (!IAP_SetDataLocation(DATA_ADD, pDeviceData->USER_SETTINGS[0]))
    return FALSE;
  for (i = 1; i < USER_SETTINGS_SIZE; i++)
    if (!IAP_SetDataLocation(IAP_GetNextLocation(&DATA_ADD), pDeviceData->USER_SETTINGS[i]))
      return FALSE;
  
  return TRUE;
}

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/

