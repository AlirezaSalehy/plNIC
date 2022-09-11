/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : comm.c
* Author             : Systems LAB - IMS (Industrial & Multisegment Sector)
* Date First Issued  : 03/12/2008
* Description        : Application layer 
********************************************************************************
* History:
* 03/12/2008: V0.1
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "application.h"
#include "dongle.h"
#include "comm.h"
#include "usb.h"
#include "iap.h"
#include "stk_n.h"
#include "fwupdate.h"

/* Private typedef -----------------------------------------------------------*/

// Application frame structure
typedef struct 
{
  APP_ftype_t type;
  u16         group;
  u32         address;
  uint8_t     len;
  uint8_t     buffer[USER_PAYLOAD_SIZE];
}APP_frame_t;

// Service request from PLM results
typedef enum
{
  APP_PLM_RES_DONE,
  APP_PLM_RES_CONN_REQUEST,
  APP_PLM_RES_USER_OVERRUN,
  APP_PLM_RES_FW_UPDATING,
  APP_PLM_RES_ERROR
}APP_PLM_res_t;

// Application layer state machine status
typedef enum 
{
  APP_IDLE,
  APP_INIT,
  APP_PROG_MODE,
  APP_CONN_REQUEST_DEL,
  APP_CONN_REQUEST
}APP_SM_Status_t;

/* Private define ------------------------------------------------------------*/

/* GENERAL */
#define APP_HEADER_SIZE         10      // Application layer header size: type[1], addressing[6], len[1], crc[2]
#define APP_BROADCAST           LL_BROADCAST_FLAG // Broadcast frame flag

/* COMMUNICATION TIMING */
#define APP_USART_SEND_TIMEOUT  1000    // 1s
#define APP_INFO_REQUEST_DELAY  100     // 0.1s
#define COMM_RX_FRAME_TIMEOUT   300     // 300 ms

/* SERVICE COMMANDS */
#define APP_SER_TIME_SYNC       0x79    // 'y'
#define APP_SER_GET_CMD_LEN     0x01    // Length of get commands (has no data apart the get group)

/* Private variables ---------------------------------------------------------*/

/* NETWORK STRUCTURES */
APP_userdata_t  APP_UserData;                         // User application layer data structure
APP_frame_t     APP_Frame;                            // Application layer data structure
NL_Data_t       APP_NW_Data;                          // Network layer data structure
NL_FrameFlag_t  APP_TFdataflags;                      // Flags of any incoming frame

bool            APP_DataFlagReady = FALSE;            // New flag data ready
bool            SOFTWARE_RESET_REQUEST = FALSE;       // Request to software reset the aplication
APP_SM_Status_t ApplicationStatus = APP_INIT;
APP_userflag_t  APP_UserCommStatus = USER_DATA_IDLE;  // User communication flag 
bool            APP_SYSTEM_TIME_UPDATED = FALSE;
bool            COMM_ACK_ENABLED = TRUE;
u32             APP_TimeStamp, APP_OneMin;
uint8_t         APP_minTimer = 0;
u16             APP_TimeSyncroTimer = 0;
bool            UpdateLEDon = FALSE;

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/* GENERAL FUNCTIONS */
LL_WM_t         APP_InitStackWorkingMode(void);
LL_settings_t   APP_InitStackDefaults(void);
bool            APP_DeviceProgrammed(void);
bool            APP_IsDeviceAddress(APP_frame_t *frm);
u16             APP_GetWord(uint8_t *buffer);
void            APP_SetWord(u16 wd, uint8_t *buffer);
u16             APP_GetNextWord(uint8_t *buffer, uint8_t *pos);
void            APP_SetNextWord(u16 wd, uint8_t *buffer, uint8_t *pos);
void            APP_SetErrorFrame(APP_frame_t *app_f, u16 errcode);
void            APP_SetUserData(APP_frame_t *adata, APP_source_t src);
void            APP_GetUserData(APP_userdata_t  *u_data, APP_frame_t *app_f);
APP_PLM_res_t   APP_TransferResult(APP_frame_t *app_frame);
APP_ERROR_t     APP_GetNetworkError(NL_ERR_t err);
u16             APP_CalcCRC16(uint8_t *buf, uint8_t nLen);
void            APP_UpdateRejectFrameStatistics(void);

/* COMMUNICATION FUNCTIONS */
bool            APP_COMM_FrameReceived(APP_frame_t *frame);
bool            APP_COMM_GetFrame(APP_frame_t *frame);
void            APP_COMM_SetFrame(APP_frame_t *frame);
bool            APP_COMM_SendFrame(APP_frame_t *frame, u16 tout_ms, bool uselocaladd);
bool            APP_COMM_SendErrorFrame(APP_ERROR_t errcode, u16 tout);
bool            APP_COMM_SendACKFrame(uint8_t ackGroup, uint8_t cmdEcho, u16 tout);

/* PROGRAMMING FUNCTIONS */
bool            APP_PROG_FrameArrived(void);
APP_PROG_CMD_t  APP_PROG_GetCommand(APP_frame_t *app_frame);
bool            APP_PROG_UploadDeviceData(APP_frame_t *app_frame);
bool            APP_PROG_DownloadDeviceData(APP_frame_t *app_frame);
APP_PLM_res_t   APP_PROG_PlmCommandExec(APP_frame_t *app_frame);
                  
/* SERVICE FUNCTIONS */
void            APP_SER_ResetTimeSyncroTimer(void);
void            APP_SER_SetTimeSynchroFrame(void);
bool            APP_SER_TimeSyncCheck(void);
void            APP_SER_GetLLStackParameters(void);
void            APP_SER_SetLLStackParameters(void);
bool            APP_SER_CommCommandExec(APP_frame_t *app_frame);
APP_PLM_res_t   APP_SER_PlmCommandExec(APP_frame_t *app_frame);
APP_SER_CMD_t   APP_SER_GetCommand(APP_frame_t *app_frame);
uint8_t*        APP_SER_GetDataAddress(APP_frame_t *app_frame);
uint16_t        APP_SER_GetFwUpdAddress(APP_frame_t *app_frame);
void            APP_SER_GetDongleInputs(APP_frame_t *app_frame);
void            APP_SER_SetIOConfig(uint8_t iocfg);
void            APP_SER_GetDongleIOconfig(APP_frame_t *app_frame);
void            APP_SER_GetFwRelease(APP_frame_t *app_frame, APP_SER_CMD_t cmd);

/* PLM FUNCTIONS */
bool            APP_PLM_SetNetworkData(APP_frame_t *adata, NL_Data_t *pldata);
bool            APP_PLM_GetNetworkData(NL_Data_t *pldata, APP_frame_t *adata);

/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : APP_InitStackWorkingMode
* Description    : Init stack working mode
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
LL_WM_t APP_InitStackWorkingMode(void)
{
  LL_WM_t wm;
  
  wm.DEVICE_OPT_ACK = LL_FRM_PARAM_DISABLED;
  wm.DEVICE_OPT_bACK = LL_FRM_PARAM_DISABLED;
  wm.DEVICE_OPT_RPT = LL_FRM_PARAM_DISABLED;
  wm.DEVICE_OPT_RPTALL = LL_FRM_PARAM_DISABLED;
  wm.DEVICE_OPT_GRP = LL_FRM_PARAM_DISABLED;
  wm.DEVICE_OPT_ENC = LL_FRM_PARAM_DISABLED;
  
  // ACK frames
  #ifdef DEVICE_WM_REQ_ACK
    wm.DEVICE_OPT_ACK = LL_FRM_PARAM_ACK_REQ;
  #endif
  // bACK frames
  #ifdef DEVICE_WM_REQ_bACK
      wm.DEVICE_OPT_bACK = LL_FRM_PARAM_bACK_REQ;
  #endif
  // Repetitions
  #ifdef DEVICE_WM_REPEATER
      wm.DEVICE_OPT_RPT = LL_FRM_PARAM_REPEAT;
  #endif
  #ifdef DEVICE_WM_REPEAT_ALL
      wm.DEVICE_OPT_RPTALL = LL_FRM_PARAM_REPEAT_ALL;
  #endif
  // Grouping
  #ifdef DEVICE_WM_GROUP_FILTER
      wm.DEVICE_OPT_GRP = LL_FRM_PARAM_GROUP;
  #endif
  // Encryption
  #ifdef DEVICE_WM_ENCRYPT_DATA
      wm.DEVICE_OPT_ENC = LL_FRM_PARAM_ENCRYPTION;
  #endif
      
  DeviceData.LocalGroup = DEVICE_DEFAULT_GROUP;                           // Local group 
  DeviceData.LocalAddress = DEVICE_DEFAULT_ADDRESS;                       // Local address 
  DeviceData.WorkingMode =   wm.DEVICE_OPT_ACK |  wm.DEVICE_OPT_bACK  |
                             wm.DEVICE_OPT_RPT |  wm.DEVICE_OPT_RPTALL |
                             wm.DEVICE_OPT_GRP | wm.DEVICE_OPT_ENC;       // Device working mode
  DeviceData.HopLevel = 0;                                                // Device hop level (0 = unused)
  
  // Update the device address
  NL_SetLocalAddress(DeviceData.LocalGroup, DeviceData.LocalAddress);
  // Update the device working mode
  NL_SetLocalWorkingMode(DeviceData.WorkingMode);
  // Update the device hop level
  NL_SetLocalHopLevel(DeviceData.HopLevel);
  
  return wm;
}

/*******************************************************************************
* Function Name  : APP_InitStackDefaults
* Description    : Init default value for stack parameters
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
LL_settings_t APP_InitStackDefaults(void)
{
  LL_settings_t ds;
  
  // Link Layer stack default settings
  ds.MIN_SLOT = DEFAULT_MIN_SLOT;
  ds.MAX_SLOT = DEFAULT_MAX_SLOT;
  ds.GLOBAL_TX_TO = DEFAULT_NTW_P_GLOBAL_TX_TO;
  ds.BC_GLOBAL_TX_TO = DEFAULT_NTW_P_BC_GLOBAL_TX_TO;
  ds.ACTIVITY_TO = DEFAULT_NTW_P_ACTIVITY_TO;
  ds.WATCHDOG_TO = DEFAULT_NTW_P_WATCHDOG_TO;
  ds.DATATRANSFER_TO = DEFAULT_NTW_P_DATATRANSFER_TO;
  ds.BANDINUSE_TO = DEFAULT_NTW_P_BANDINUSE_TO;
  ds.FRAME_TX_TO = DEFAULT_NTW_P_FRAME_TX_TO;
  ds.BCAST_TX_TO = DEFAULT_NTW_P_BCAST_TX_TO;
  ds.ACK_RX_TO = DEFAULT_NTW_P_ACK_RX_TO;
  ds.bACK_RX_TO = DEFAULT_NTW_P_bACK_RX_TO;
  ds.FRM_RX_TO = DEFAULT_NTW_P_FRM_RX_TO;
  ds.NDX_TO = DEFAULT_NTW_P_NDX_TO;
  ds.MAX_ATTEMPT = DEFAULT_LL_MAX_ATTEMPT;
  ds.MAX_RPT_ATTEMPT = DEFAULT_MAX_RPT_ATTEMPT;
  ds.TIME_SYNC = DEFAULT_DEVICE_TIME_SYNC;

  DeviceData.LL_SM_MIN_SLOT = (u16)(DEFAULT_MIN_SLOT);
  DeviceData.LL_SM_MAX_SLOT = (u16)(DEFAULT_MAX_SLOT);
  DeviceData.LL_SM_GLOBAL_TX_TO = (u16)(DEFAULT_NTW_P_GLOBAL_TX_TO);
  DeviceData.LL_SM_BC_GLOBAL_TX_TO = (u16)(DEFAULT_NTW_P_BC_GLOBAL_TX_TO);
  DeviceData.LL_SM_ACTIVITY_TO__H = (u16)(DEFAULT_NTW_P_ACTIVITY_TO >> 16);
  DeviceData.LL_SM_ACTIVITY_TO__L = (u16)(DEFAULT_NTW_P_ACTIVITY_TO);
  DeviceData.LL_SM_WATCHDOG_TO = (u16)(DEFAULT_NTW_P_WATCHDOG_TO);
  DeviceData.LL_SM_DATATRANSFER_TO = (u16)(DEFAULT_NTW_P_DATATRANSFER_TO);
  DeviceData.LL_SM_BANDINUSE_TO__H = (u16)(DEFAULT_NTW_P_BANDINUSE_TO >> 16);
  DeviceData.LL_SM_BANDINUSE_TO__L = (u16)(DEFAULT_NTW_P_BANDINUSE_TO);
  DeviceData.LL_SM_FRAME_TX_TO__H = (u16)(DEFAULT_NTW_P_FRAME_TX_TO >> 16);
  DeviceData.LL_SM_FRAME_TX_TO__L = (u16)(DEFAULT_NTW_P_FRAME_TX_TO);
  DeviceData.LL_SM_BCAST_TX_TO__H = (u16)(DEFAULT_NTW_P_BCAST_TX_TO >> 16);
  DeviceData.LL_SM_BCAST_TX_TO__L = (u16)(DEFAULT_NTW_P_BCAST_TX_TO);
  DeviceData.LL_SM_ACK_RX_TO__H = (u16)(DEFAULT_NTW_P_ACK_RX_TO >> 16);
  DeviceData.LL_SM_ACK_RX_TO__L = (u16)(DEFAULT_NTW_P_ACK_RX_TO);
  DeviceData.LL_SM_bACK_RX_TO__H = (u16)(DEFAULT_NTW_P_bACK_RX_TO >> 16);
  DeviceData.LL_SM_bACK_RX_TO__L = (u16)(DEFAULT_NTW_P_bACK_RX_TO);
  DeviceData.LL_SM_FRM_RX_TO__H = (u16)(DEFAULT_NTW_P_FRM_RX_TO >> 16);
  DeviceData.LL_SM_FRM_RX_TO__L = (u16)(DEFAULT_NTW_P_FRM_RX_TO);
  DeviceData.LL_SM_NDX_TO__H = (u16)(DEFAULT_NTW_P_NDX_TO >> 16);
  DeviceData.LL_SM_NDX_TO__L = (u16)(DEFAULT_NTW_P_NDX_TO);
  DeviceData.LL_SM_MAX_ATTEMPT = (u16)(DEFAULT_LL_MAX_ATTEMPT);
  DeviceData.LL_SM_RPT_ATTEMPT = (u16)(DEFAULT_MAX_RPT_ATTEMPT);
  DeviceData.DEVICE_TIME_SYNC = (u16)(DEFAULT_DEVICE_TIME_SYNC);
  APP_SER_SetLLStackParameters(); // Update LL stack variables
      
  return ds;
}

/*******************************************************************************
* Function Name  : APP_ApplicationInit
* Description    : Init application state machine
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void APP_ApplicationInit(void)
{
  // Configure LEDs
  DH_LED_Config();
  if (NL_NetworkInit(APP_InitStackWorkingMode(), APP_InitStackDefaults()))
  {
    DH_LED_Init();
    COMM_Init();    
    DH_IO_Init();
  }
  else
  {
    DH_ShowLED(A_LED_DATA, A_LED_OFF);
    DH_ShowLED(A_LED_ERROR, A_LED_ON); 
    SYSTEM_RESET = TRUE;
    while(1);
  }
}

/*******************************************************************************
* Function Name  : APP_GetWord
* Description    : Returns a word (2 bytes) from a given buffer
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u16 APP_GetWord(uint8_t *buffer)
{
   return ((u16)((((u16)(buffer[0])) << 8) | (((u16)(buffer[1])) & 0x0ff)));
}

/*******************************************************************************
* Function Name  : APP_SetWord
* Description    : Returns a word (2 bytes) from a given buffer
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void APP_SetWord(u16 wd, uint8_t *buffer)
{
  buffer[0] = (uint8_t)((wd >> 8));
  buffer[1] = (uint8_t)wd;
}

/*******************************************************************************
* Function Name  : APP_DeviceProgrammed
* Description    : Check if the device has been programmed
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool APP_DeviceProgrammed(void)
{
  if ((*(vu16*)(IAP_BASE_ADDRESS)) == DONGLE_CIS)
    return TRUE;
  return FALSE;
}

/*******************************************************************************
* Function Name  : APP_IsDeviceAddress
* Description    : Check if the incoming frame is addressed to the dongle 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool APP_IsDeviceAddress(APP_frame_t *frm)
{
  u16 dGrp;
  u32 dAdd;
  
  NL_GetLocalAddress(&dGrp, &dAdd);
  if ((frm->group == dGrp) && (frm->address == dAdd) && !(frm->type & APP_BROADCAST))
    return TRUE;
  return FALSE;
}

/*******************************************************************************
* Function Name  : APP_DeviceAddressed
* Description    : Check if the user data are addressing the device
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool APP_DeviceAddressed(APP_userdata_t *frm)
{
  u16 dGrp;
  u32 dAdd;
  
  NL_GetLocalAddress(&dGrp, &dAdd);
  if ((frm->group == dGrp) && (frm->address == dAdd))
    return TRUE;
  return FALSE;
}

/*******************************************************************************
* Function Name  : APP_GetDeviceAddress
* Description    : Get the device address
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void APP_GetLocalAddress(uint16_t *group, uint32_t *address)
{
  NL_GetLocalAddress(group, address);
}

/*******************************************************************************
* Function Name  : APP_CalcCRC16
* Description    : Calculate a 16 bit CRC (X16 + X15 + X2 + 1)
* Input          : Buffer pointer, buffer length
* Output         : None
* Return         : Calculated CRC
*******************************************************************************/
u16 APP_CalcCRC16(uint8_t *buf, uint8_t nLen)
{
  u16 crc = 0;
  
  while (nLen--)
    crc = (crc >> 8) ^ LL_TableCRC16[(crc ^ (*(buf++))) & 0xff];
  
  return (crc);
}

/*******************************************************************************
* Function Name  : APP_COMM_GetFrame
* Description    : Get the arrived frame from COMM buffer
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool APP_COMM_GetFrame(APP_frame_t *frame)
{
  uint8_t *combuf;
  uint8_t i;
  u16 crc;
    
  combuf =  COMM_GetBufferPointer();
  i = combuf[0] - 2; // Entire len
  crc = ((u16)(combuf[i] << 8)) | combuf[i+1];
  if (crc == APP_CalcCRC16(combuf, i))
  {
    frame->len = combuf[0] - APP_HEADER_SIZE;
    frame->type = (APP_ftype_t)combuf[1];
    frame->group = APP_GetWord(combuf + 2);
    frame->address = (((u32)(APP_GetWord(combuf + 4))) << 16) | (((u32)(APP_GetWord(combuf + 6))) & 0x0ffff);
    for (i = 0; i < frame->len; i++)
      frame->buffer[i] = combuf[8+i];
    return TRUE;
  }
  return FALSE;
}

/*******************************************************************************
* Function Name  : APP_COMM_SetFrame
* Description    : Load a communcation frame into communication buffer
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void APP_COMM_SetFrame(APP_frame_t *frame)
{
  uint8_t *combuf;
  uint8_t i;
  u16 crc;
  
  combuf = COMM_GetBufferPointer();
  
  combuf[0] = frame->len + APP_HEADER_SIZE;
  combuf[1] = (uint8_t)(frame->type);
  
  APP_SetWord(frame->group, combuf + 2);
  APP_SetWord((u16)(frame->address >> 16), combuf + 4);
  APP_SetWord((u16)frame->address, combuf + 6);    
  for (i = 0; i < frame->len; i++)
    combuf[8 + i] = frame->buffer[i];
  crc = APP_CalcCRC16(combuf, i + 8);
  combuf[8 + i++] = (uint8_t)(crc >> 8);
  combuf[8 + i] = (uint8_t)crc;
}

/*******************************************************************************
* Function Name  : APP_PROG_FrameArrived
* Description    : Verify if a programming frame is arrived
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool APP_PROG_FrameArrived(void)
{
  if (APP_COMM_FrameReceived(&APP_Frame))
  {
    if (APP_Frame.type == APP_PROGRAMMING_FRAME)
      return TRUE;
  }
  return FALSE;
}

/*******************************************************************************
* Function Name  : APP_SER_GetCommand
* Description    : Return the service command
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
APP_SER_CMD_t APP_SER_GetCommand(APP_frame_t *app_frame)
{
  return ((APP_SER_CMD_t)(app_frame->buffer[0]));
}

/*******************************************************************************
* Function Name  : APP_SER_GetDataAddress
* Description    : Return the service data first location
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint8_t* APP_SER_GetDataAddress(APP_frame_t *app_frame)
{
  return (&(app_frame->buffer[1]));
}

/*******************************************************************************
* Function Name  : APP_SER_GetFwUpdAddress
* Description    : Return the segment address of the new
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t APP_SER_GetFwUpdAddress(APP_frame_t *app_frame)
{
  return (((uint16_t)(app_frame->buffer[1]) << 8) | (uint16_t)(app_frame->buffer[2]));
}

/*******************************************************************************
* Function Name  : APP_SER_GetDongleInputs
* Description    : Return the dongle inputs status (if configured)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void APP_SER_GetDongleInputs(APP_frame_t *app_frame)
{
  app_frame->buffer[1] = DH_GetInputs();
  app_frame->len = 2;
}

/*******************************************************************************
* Function Name  : APP_InitIO
* Description    : Initialize the Outputs status (if configured)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void APP_InitIO(void)
{
  DH_InOutConfig((uint8_t)(DeviceData.IOConfiguration));
  DH_SetOutputs(IO_DEFAULT_OUT_VAL);
}

/*******************************************************************************
* Function Name  : APP_SER_SetIOConfig
* Description    : Set and store the I/O configuration
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void APP_SER_SetIOConfig(uint8_t iocfg)
{
  DH_InOutConfig(iocfg);
  DeviceData.IOConfiguration = (uint16_t)iocfg;
  // Write data on flash
  IAP_SetDeviceData(&DeviceData);
}

/*******************************************************************************
* Function Name  : APP_SER_GetDongleIOconfig
* Description    : Return the dongle inputs status (if configured)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void APP_SER_GetDongleIOconfig(APP_frame_t *app_frame)
{
  app_frame->buffer[1] = (uint8_t)DeviceData.IOConfiguration;
  app_frame->len = 2;
}

/*******************************************************************************
* Function Name  : APP_SER_GetFwRelease
* Description    : Return the application or the stack firmware release
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void APP_SER_GetFwRelease(APP_frame_t *app_frame, APP_SER_CMD_t cmd)
{
    app_frame->buffer[1] = (uint8_t)(DeviceData.FirmwareRelease >> 8);
    app_frame->buffer[2] = (uint8_t)(DeviceData.FirmwareRelease);
    app_frame->buffer[3] = (uint8_t)(NL_GetStackFirmwareRelease() >> 8);
    app_frame->buffer[4] = (uint8_t)(NL_GetStackFirmwareRelease());
    app_frame->len = 5;
}

/*******************************************************************************
* Function Name  : APP_GetNextWord  
* Description    : Get 16 byt data from buffer
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
u16 APP_GetNextWord(uint8_t *buffer, uint8_t *pos)
{
  (*pos) += 2;
  return ((((u16)(buffer[(*pos)-2]))<<8)|(((u16)(buffer[(*pos)-1])) & 0x0ff));
}

/*******************************************************************************
* Function Name  : APP_SetNextWord  
* Description    : Set 16 bit data into a buffer
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void APP_SetNextWord(u16 wd, uint8_t *buffer, uint8_t *pos)
{
  buffer[(*pos)++] = (uint8_t)(wd >> 8);
  buffer[(*pos)++] = (uint8_t)(wd & 0x0ff);
}

/*******************************************************************************
* Function Name  : APP_PROG_GetCommand  
* Description    : Get the programming command
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
APP_PROG_CMD_t APP_PROG_GetCommand(APP_frame_t *app_frame)
{
  return ((APP_PROG_CMD_t)app_frame->buffer[0]);
}

/*******************************************************************************
* Function Name  : APP_PROG_UploadDeviceData
* Description    : Upload the dongle data into the communication buffer
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool APP_PROG_UploadDeviceData(APP_frame_t *app_frame)
{
  uint8_t i;
  uint8_t loc = 2;
  
  // Check wich data to transfer
  switch ((APP_PROG_GROUP_t)(app_frame->buffer[1]))
  {
    case PROG_GRP_DEVICE_DATA:
      APP_SetNextWord(DeviceData.LocalGroup, app_frame->buffer, &loc);
      APP_SetNextWord((u16)(DeviceData.LocalAddress >> 16), app_frame->buffer, &loc);
      APP_SetNextWord((u16)(DeviceData.LocalAddress & 0x0ffff), app_frame->buffer, &loc);
      APP_SetNextWord((((u16)(DeviceData.WorkingMode)) << 8) | ((u16)(DeviceData.HopLevel)), app_frame->buffer, &loc);
      app_frame->len = loc;
      return TRUE;
      
    case PROG_GRP_LL_STACK_PARAM:
      APP_SetNextWord(DeviceData.LL_SM_MIN_SLOT, app_frame->buffer,&loc);
      APP_SetNextWord(DeviceData.LL_SM_MAX_SLOT, app_frame->buffer, &loc);
      APP_SetNextWord(DeviceData.LL_SM_GLOBAL_TX_TO, app_frame->buffer, &loc);
      APP_SetNextWord(DeviceData.LL_SM_BC_GLOBAL_TX_TO, app_frame->buffer, &loc);
      APP_SetNextWord(DeviceData.LL_SM_ACTIVITY_TO__H, app_frame->buffer, &loc);
      APP_SetNextWord(DeviceData.LL_SM_ACTIVITY_TO__L, app_frame->buffer, &loc);
      APP_SetNextWord(DeviceData.LL_SM_WATCHDOG_TO, app_frame->buffer, &loc);
      APP_SetNextWord(DeviceData.LL_SM_DATATRANSFER_TO, app_frame->buffer, &loc);
      APP_SetNextWord(DeviceData.LL_SM_BANDINUSE_TO__H, app_frame->buffer, &loc);
      APP_SetNextWord(DeviceData.LL_SM_BANDINUSE_TO__L, app_frame->buffer, &loc);
      APP_SetNextWord(DeviceData.LL_SM_FRAME_TX_TO__H, app_frame->buffer, &loc);
      APP_SetNextWord(DeviceData.LL_SM_FRAME_TX_TO__L, app_frame->buffer, &loc);
      APP_SetNextWord(DeviceData.LL_SM_BCAST_TX_TO__H, app_frame->buffer, &loc);
      APP_SetNextWord(DeviceData.LL_SM_BCAST_TX_TO__L, app_frame->buffer, &loc);
      APP_SetNextWord(DeviceData.LL_SM_ACK_RX_TO__H, app_frame->buffer, &loc);
      APP_SetNextWord(DeviceData.LL_SM_ACK_RX_TO__L, app_frame->buffer, &loc);
      APP_SetNextWord(DeviceData.LL_SM_bACK_RX_TO__H, app_frame->buffer, &loc);
      APP_SetNextWord(DeviceData.LL_SM_bACK_RX_TO__L, app_frame->buffer, &loc);
      APP_SetNextWord(DeviceData.LL_SM_FRM_RX_TO__H, app_frame->buffer, &loc);
      APP_SetNextWord(DeviceData.LL_SM_FRM_RX_TO__L, app_frame->buffer, &loc);
      APP_SetNextWord(DeviceData.LL_SM_NDX_TO__H, app_frame->buffer, &loc);
      APP_SetNextWord(DeviceData.LL_SM_NDX_TO__L, app_frame->buffer, &loc);
      APP_SetNextWord(DeviceData.LL_SM_MAX_ATTEMPT, app_frame->buffer, &loc);
      APP_SetNextWord(DeviceData.LL_SM_RPT_ATTEMPT, app_frame->buffer, &loc);
      APP_SetNextWord(DeviceData.DEVICE_TIME_SYNC, app_frame->buffer, &loc);      
      app_frame->len = loc;
      return TRUE;
      
    case PROG_GRP_USER_DATA:
      for (i = 0; i < USER_SETTINGS_SIZE; i++)
        APP_SetNextWord(DeviceData.USER_SETTINGS[i], app_frame->buffer, &loc);
      app_frame->len = loc;
      return TRUE;
      
    default:
      return FALSE;
  }  
}

/*******************************************************************************
* Function Name  : APP_PROG_DownloadDeviceData
* Description    : Download the dongle data from the communication buffer
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool APP_PROG_DownloadDeviceData(APP_frame_t *app_frame)
{
  uint8_t i;
  uint8_t loc = 2;
  u16 addH, addL, wm;
  
  // Check wich data to transfer
  switch ((APP_PROG_GROUP_t)(app_frame->buffer[1]))
  {
    case PROG_GRP_DEVICE_DATA:
      DeviceData.LocalGroup = APP_GetNextWord(app_frame->buffer, &loc);
      addH = APP_GetNextWord(app_frame->buffer, &loc);
      addL = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LocalAddress = ((u32)(addH) << 16) | (u32)(addL);
      wm = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.WorkingMode = (uint8_t)(wm >> 8);
      DeviceData.HopLevel = (uint8_t)wm;
      // Update the device address
      NL_SetLocalAddress(DeviceData.LocalGroup, DeviceData.LocalAddress);
      // Update the device working mode
      NL_SetLocalWorkingMode(DeviceData.WorkingMode);
      // Update the device hop level
      NL_SetLocalHopLevel(DeviceData.HopLevel);
#ifdef DEVICE_ENCRYPTION_AES
      for (i = 0; i < AES_KEY_SIZE; i++)
        DeviceData.AESkey[i] = app_frame->buffer[loc + i];
      NL_SetEncryptionKey(DeviceData.AESkey);
#endif  
      return TRUE;
      
    case PROG_GRP_LL_STACK_PARAM:
      DeviceData.LL_SM_MIN_SLOT = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LL_SM_MAX_SLOT = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LL_SM_GLOBAL_TX_TO = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LL_SM_BC_GLOBAL_TX_TO = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LL_SM_ACTIVITY_TO__H = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LL_SM_ACTIVITY_TO__L = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LL_SM_WATCHDOG_TO = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LL_SM_DATATRANSFER_TO = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LL_SM_BANDINUSE_TO__H = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LL_SM_BANDINUSE_TO__L = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LL_SM_FRAME_TX_TO__H = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LL_SM_FRAME_TX_TO__L = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LL_SM_BCAST_TX_TO__H = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LL_SM_BCAST_TX_TO__L = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LL_SM_ACK_RX_TO__H = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LL_SM_ACK_RX_TO__L = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LL_SM_bACK_RX_TO__H = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LL_SM_bACK_RX_TO__L = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LL_SM_FRM_RX_TO__H = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LL_SM_FRM_RX_TO__L = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LL_SM_NDX_TO__H = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LL_SM_NDX_TO__L = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LL_SM_MAX_ATTEMPT = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.LL_SM_RPT_ATTEMPT = APP_GetNextWord(app_frame->buffer, &loc);
      DeviceData.DEVICE_TIME_SYNC = APP_GetNextWord(app_frame->buffer, &loc);
      APP_SER_SetLLStackParameters(); // Update LL stack variables
      return TRUE;
      
    case PROG_GRP_USER_DATA:
      for (i = 0; i < USER_SETTINGS_SIZE; i++)
        DeviceData.USER_SETTINGS[i] = APP_GetNextWord(app_frame->buffer, &loc);
      return TRUE;
      
    default:
      return FALSE;
  }  
}

/*******************************************************************************
* Function Name  : APP_GetTransitFrameFlags
* Description    : Get the transit frame flags
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool APP_GetTransitFrameFlags(APP_SER_FLAGS_t *flags)
{
  if (APP_DataFlagReady)
  {
    flags->type = (APP_TF_t)(APP_TFdataflags.type);
    flags->FECcorrections = APP_TFdataflags.FECcorrections;
    flags->wrongpostamble = APP_TFdataflags.wrongpostamble;
    flags->wrongCRC = APP_TFdataflags.wrongCRC;
    flags->hopoverrun = APP_TFdataflags.hopoverrun;
    flags->framerejected = APP_TFdataflags.framerejected;
    APP_DataFlagReady = FALSE;
    return TRUE;
  }
  return FALSE;
}

/*******************************************************************************
* Function Name  : APP_GetUserSettings
* Description    : Get user settings
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void APP_GetUserSettings(u16 *usersettings)
{
  uint8_t i;
  
  for (i = 0; i < USER_SETTINGS_SIZE; i++)
    usersettings[i] = DeviceData.USER_SETTINGS[i];
}

/*******************************************************************************
* Function Name  : Set user  
* Description    : Set user settings
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void APP_SetUserSettings(u16 *usersettings)
{
  uint8_t i;
  
  for (i = 0; i < USER_SETTINGS_SIZE; i++)
    DeviceData.USER_SETTINGS[i] = usersettings[i];

  // Store data in flash
  IAP_SetDeviceData(&DeviceData);
}

/*******************************************************************************
* Function Name  : APP_COMM_SendFrame  
* Description    : Send a frame
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool APP_COMM_SendFrame(APP_frame_t *frame, u16 tout_ms, bool uselocaladd)
{
  u32 tstmp;
  bool bres = TRUE;
  
  // Load the preset frame into communication buffer and add the CRC
  if (uselocaladd)
    NL_GetLocalAddress(&APP_Frame.group, &APP_Frame.address);
  APP_COMM_SetFrame(frame);
  
  tstmp = DH_Timestamp();

  // Start frame transmission
  bres = COMM_StartTransmission();
  
  while (!COMM_FrameTransmitted() && bres)
  {
    if (DH_DelayElapsed(tstmp, tout_ms))
      bres = FALSE;
  }
  
  COMM_EnableReceiver();
  return bres;
}

/*******************************************************************************
* Function Name  : APP_SetErrorFrame  
* Description    : Load the error frame structure
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void APP_SetErrorFrame(APP_frame_t *app_f, u16 errcode)
{
  app_f->type = APP_ERROR_FRAME;
  app_f->len = 2; 
  app_f->buffer[0] = (uint8_t)(errcode >> 8);  // Error code MSB
  app_f->buffer[1] = (uint8_t)(errcode);       // Error code LSB  
}

/*******************************************************************************
* Function Name  : APP_COMM_SendErrorFrame  
* Description    : Send an error frame
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool APP_COMM_SendErrorFrame(APP_ERROR_t errcode, u16 tout)
{ 
  DH_ShowLED(A_LED_ERROR, A_LED_FLASH);
#ifdef COMM_SEND_ERROR_FRAME  
  bool res;
  
  APP_SetErrorFrame(&APP_Frame, (u16)(errcode));
  res = APP_COMM_SendFrame(&APP_Frame, tout, TRUE);
  
  return (res);
#else  
  return TRUE;
#endif 
}

/*******************************************************************************
* Function Name  : APP_COMM_SendACKFrame 
* Description    : Send the ACK frame
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool APP_COMM_SendACKFrame(uint8_t ackGroup, uint8_t cmdEcho, u16 tout)
{
#ifdef COMM_SEND_ACK_FRAME
  bool res;
  
  APP_Frame.type = APP_ACK_FRAME;
  APP_Frame.len = 2; 
  APP_Frame.buffer[0] = ackGroup;  // ACK group 
  APP_Frame.buffer[1] = cmdEcho;   // Command echo
    
  res = APP_COMM_SendFrame(&APP_Frame, tout, TRUE);
  
  return (res);
#else
  return TRUE;
#endif        
}

/*******************************************************************************
* Function Name  : APP_COMM_FrameReceived  
* Description    : Verify the frame rx timeout in case of frame reception
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool APP_COMM_FrameReceived(APP_frame_t *frame)
{
  static bool tch = FALSE;
  static u32 tstp;
  
  if (COMM_FrameReceivingOngoing())
  {
    if (tch)
    {
      if (DH_DelayElapsed(tstp, COMM_RX_FRAME_TIMEOUT))
        COMM_ResetReceiver();
    }
    else
    {
      tch = TRUE;
      tstp = DH_Timestamp();
    }
  }
  else
    tch = FALSE;

  if (COMM_FrameArrived())
  {
    // Frame arrived. Check CRC
    if (APP_COMM_GetFrame(frame))
      return TRUE;
  }
  COMM_EnableReceiver();
  return FALSE;
}

/*******************************************************************************
* Function Name  : APP_PLM_SetNetworkData
* Description    : Load the application layer data in the network data structure
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool APP_PLM_SetNetworkData(APP_frame_t *adata, NL_Data_t *pldata)
{
  uint8_t i;
  
  switch (adata->type & (~APP_BROADCAST))
  {
    /* Type of data frame from application level: will be transferred from the target to user application level */
    case APP_DATA_FRAME:                        // User data frame addressed to a target PLM 
      pldata->frametype = NL_CLS_DATA_FRAME;    // Data frame type
      break;
    case APP_ERROR_FRAME:                       // User error frame from user application level addressed to a target PLM 
      pldata->frametype = NL_CLS_ERROR_FRAME;   // Error frame type
      break;
    case APP_PROGRAMMING_FRAME:                 // User programming frame addressed to a target PLM 
      pldata->frametype = NL_CLS_PROGR_FRAME;   // Programming frame type
      break;
    case APP_ACK_FRAME:                         // User acknowledgement frame addressed to a target PLM 
      pldata->frametype = NL_CLS_RES_FRAME;     // Request ACK frame type
      break;
    /* Service frame from application level: some of it will transferred to the user application level 
      (native service command will be executed at application stack level) */
    case APP_SERVICE_FRAME:
      pldata->frametype = NL_CLS_SERVICE_FRAME;
      break;
    /* Ping frame will be processed directly at data link layer. Any user acknowledgement frame will be received from user application level
       The ping notification is only the success of NL_NetworkRequest (N_SUCCESS) */
    case APP_PING_FRAME:
      pldata->frametype = NL_CLS_PING_FRAME;
      break;
    default:
    /* Other frame type are not allowed */
      return FALSE;
  }
  
  // Check if the broadcast flag have to be set
  if (adata->type & APP_BROADCAST)
    pldata->frametype |= APP_BROADCAST;

  // Load address  
  pldata->group = adata->group;
  pldata->address = adata->address;
  
  // Load buffer
  pldata->framelen = adata->len;
  for (i = 0; i < pldata->framelen; i++)
    pldata->databuffer[i] = adata->buffer[i];
  
  return TRUE;
}

/*******************************************************************************
* Function Name  : APP_PLM_GetNetworkData
* Description    : Load the network data in the application layer data structure
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool APP_PLM_GetNetworkData(NL_Data_t *pldata, APP_frame_t *adata)
{
  uint8_t i;
  
  switch (pldata->frametype & (~APP_BROADCAST))
  {
    case NL_CLS_DATA_FRAME:
      adata->type = APP_DATA_FRAME;
      break;
    case NL_CLS_ERROR_FRAME:
      adata->type = APP_ERROR_FRAME;
      break;
    case NL_CLS_PROGR_FRAME:
      adata->type = APP_PROGRAMMING_FRAME;
      break;
    case NL_CLS_RES_FRAME:
      adata->type = APP_ACK_FRAME;
      break;
    case NL_CLS_SERVICE_FRAME:
      adata->type = APP_SERVICE_FRAME;
      break;
  }
  
   // Check if the frame was sent in broadcast and set the broadcast flag consequently 
  if (pldata->frametype & APP_BROADCAST)
    adata->type |= APP_BROADCAST;
  
  // Laad address
  adata->group = pldata->group;
  adata->address = pldata->address;
  
  // Load buffer
  adata->len = pldata->framelen;
  for (i = 0; i <adata->len; i++)
    adata->buffer[i] = pldata->databuffer[i];
  
  return TRUE;
}

/*******************************************************************************
* Function Name  : APP_GetNetworkError
* Description    : Get the error type
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
APP_ERROR_t APP_GetNetworkError(NL_ERR_t err)
{
  switch (err)
  {
    case NL_ERR_NETWORK_TIMEOUT:
    case NL_ERR_COMMUNICATION_TIMEOUT:
      return APP_ERROR_COMM_TIMEOUT;
    case NL_ERR_LINE_ERROR:
    case NL_ERR_FRAME_CLASS_UNKNOWN:
      return APP_ERROR_COMMUNICATION;
    case NL_ERR_TARGET_NOT_REACHABLE:
      return APP_ERROR_ISOLATED_NODE;
    default:
      return APP_ERROR_GENERIC;
  }
}

/*******************************************************************************
* Function Name  : APP_SER_SetLLStackParameters
* Description    : Load dongle parameters and set the LL stack vars
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void APP_SER_SetLLStackParameters(void)
{
  NL_DLSP_t devparam;
  
  devparam.minslot = DeviceData.LL_SM_MIN_SLOT;
  devparam.maxslot = DeviceData.LL_SM_MAX_SLOT;
  devparam.globaltxto = (uint8_t)(DeviceData.LL_SM_GLOBAL_TX_TO);
  devparam.bcglobaltxto = (uint8_t)(DeviceData.LL_SM_BC_GLOBAL_TX_TO);
  devparam.activityto = (u32)((((u32)DeviceData.LL_SM_ACTIVITY_TO__H) << 16) | 
                            (u32)(DeviceData.LL_SM_ACTIVITY_TO__L));
  devparam.watchdogto = (uint8_t)(DeviceData.LL_SM_WATCHDOG_TO);
  devparam.datatransferto = (uint8_t)(DeviceData.LL_SM_DATATRANSFER_TO);
  devparam.bandinuseto = (u32)((((u32)DeviceData.LL_SM_BANDINUSE_TO__H) << 16) | 
                             (u32)(DeviceData.LL_SM_BANDINUSE_TO__L));
  devparam.frametxto = (u32)((((u32)DeviceData.LL_SM_FRAME_TX_TO__H) << 16) | 
                            (u32)(DeviceData.LL_SM_FRAME_TX_TO__L));
  devparam.bcasttxto = (u32)((((u32)DeviceData.LL_SM_BCAST_TX_TO__H) << 16) | 
                            (u32)(DeviceData.LL_SM_BCAST_TX_TO__L));
  devparam.ackrxto = (u32)((((u32)DeviceData.LL_SM_ACK_RX_TO__H) << 16) | 
                          (u32)(DeviceData.LL_SM_ACK_RX_TO__L));
  devparam.backrxto = (u32)((((u32)DeviceData.LL_SM_bACK_RX_TO__H) << 16) | 
                           (u32)(DeviceData.LL_SM_bACK_RX_TO__L));
  devparam.frmrxto = (u32)((((u32)DeviceData.LL_SM_FRM_RX_TO__H) << 16) | 
                          (u32)(DeviceData.LL_SM_FRM_RX_TO__L));
  devparam.ndxto = (u32)((((u32)DeviceData.LL_SM_NDX_TO__H) << 16) | 
                       (u32)(DeviceData.LL_SM_NDX_TO__L));
  devparam.maxattempt = (uint8_t)(DeviceData.LL_SM_MAX_ATTEMPT);
  devparam.maxrptattempt = (uint8_t)(DeviceData.LL_SM_RPT_ATTEMPT);
  devparam.timesync = DeviceData.DEVICE_TIME_SYNC;
  
  // Load data link stack parameters
  NL_SetDataLinkStackParameters(devparam);
}

/*******************************************************************************
* Function Name  : APP_SER_GetLLStackParameters
* Description    : 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void APP_SER_GetLLStackParameters(void)
{
  NL_DLSP_t devparam;
  
  // Load parameters
  NL_GetDataLinkStackParameters(&devparam);

  DeviceData.LL_SM_MIN_SLOT = devparam.minslot;
  DeviceData.LL_SM_MAX_SLOT = devparam.maxslot;
  DeviceData.LL_SM_GLOBAL_TX_TO = (u16)(devparam.globaltxto);
  DeviceData.LL_SM_BC_GLOBAL_TX_TO = (u16)(devparam.bcglobaltxto);
  DeviceData.LL_SM_ACTIVITY_TO__H = (u16)(devparam.activityto >> 16);
  DeviceData.LL_SM_ACTIVITY_TO__L = (u16)(devparam.activityto);
  DeviceData.LL_SM_WATCHDOG_TO = (u16)(devparam.watchdogto);
  DeviceData.LL_SM_DATATRANSFER_TO = (u16)(devparam.datatransferto);
  DeviceData.LL_SM_BANDINUSE_TO__H = (u16)(devparam.bandinuseto >> 16);
  DeviceData.LL_SM_BANDINUSE_TO__L = (u16)(devparam.bandinuseto);
  DeviceData.LL_SM_FRAME_TX_TO__H = (u16)(devparam.frametxto >> 16);
  DeviceData.LL_SM_FRAME_TX_TO__L = (u16)(devparam.frametxto);
  DeviceData.LL_SM_BCAST_TX_TO__H = (u16)(devparam.bcasttxto >> 16);
  DeviceData.LL_SM_BCAST_TX_TO__L = (u16)(devparam.bcasttxto);
  DeviceData.LL_SM_ACK_RX_TO__H = (u16)(devparam.ackrxto >> 16);
  DeviceData.LL_SM_ACK_RX_TO__L = (u16)(devparam.ackrxto);
  DeviceData.LL_SM_bACK_RX_TO__H = (u16)(devparam.backrxto >> 16);
  DeviceData.LL_SM_bACK_RX_TO__L = (u16)(devparam.backrxto);
  DeviceData.LL_SM_FRM_RX_TO__H = (u16)(devparam.frmrxto >> 16);
  DeviceData.LL_SM_FRM_RX_TO__L = (u16)(devparam.frmrxto);
  DeviceData.LL_SM_NDX_TO__H = (u16)(devparam.ndxto >> 16);
  DeviceData.LL_SM_NDX_TO__L = (u16)(devparam.ndxto);
  DeviceData.LL_SM_MAX_ATTEMPT = (u16)(devparam.maxattempt);
  DeviceData.LL_SM_RPT_ATTEMPT = (u16)(devparam.maxrptattempt);
  DeviceData.DEVICE_TIME_SYNC = devparam.timesync;
}

/*******************************************************************************
* Function Name  : APP_SER_ResetTimeSyncroTimer
* Description    : reset the supervisor timer variables
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void APP_SER_ResetTimeSyncroTimer(void)
{
  APP_minTimer = 0;
  APP_TimeSyncroTimer = 0;
  APP_OneMin = DH_Timestamp();
}

/*******************************************************************************
* Function Name  : APP_SER_TimeSyncCheck
* Description    : Check if a time syncronization frame has to be sent
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool APP_SER_TimeSyncCheck(void)
{
  if (DeviceData.DEVICE_TIME_SYNC)
  {
    if (APP_TimeSyncroTimer >= DeviceData.DEVICE_TIME_SYNC)
    {
      APP_SER_ResetTimeSyncroTimer();
      // Send time synchro frame in broadcast
      APP_SER_SetTimeSynchroFrame();
      return TRUE;
    }
    else
    {
      // 1 sec step
      if (DH_DelayElapsed(APP_OneMin, 1000))
      {
        APP_minTimer++;
        APP_OneMin = DH_Timestamp();
        if (APP_minTimer == 60)
        {
          // 1 min
          APP_minTimer = 0;
          APP_TimeSyncroTimer++;
        }
      }
    }
  }
  return FALSE;
}

/*******************************************************************************
* Function Name  : APP_UpdateRejectFrameStatistics
* Description    : Update transit frame statistics for rejected frames.
*                  If used willbe managed at USER lavel.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void APP_UpdateRejectFrameStatistics(void)
{
  if (NL_BrokenFrameArrived())
  {
    if (APP_DataFlagReady == FALSE)
    {
      if (NL_GetFrameRxFlags(&APP_TFdataflags))
        APP_DataFlagReady = TRUE;
    }
  }
}

/*******************************************************************************
* Function Name  : APP_ApplicationReady
* Description    : Return true when application is in idle mode 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool APP_ApplicationReady(void)
{
  if (ApplicationStatus == APP_IDLE)
    return TRUE;
  return FALSE;
}

/*******************************************************************************
* Function Name  : APP_StackUpdate  
* Description    : Application State Machine
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void APP_StackUpdate(void)
{
  NL_Status_t nNStatus;
  APP_PLM_res_t plm_res;
  bool PLM_PROG_FRAME = FALSE;
  
  switch (ApplicationStatus)
  {
    case APP_INIT:
      // Check if Device has been programmed before
      if (!APP_DeviceProgrammed())
      {
        // Device not yet programmed
        ApplicationStatus = APP_PROG_MODE;
        DH_ShowLED(A_LED_BOTH, A_LED_ON);
        //IAP_SetDeviceData(&DeviceData);
      }
      else
      {
        // Device already programmed: retrive dongle information
        IAP_GetDeviceData(&DeviceData); // Read dongle data at power on
        NL_SetLocalAddress(DeviceData.LocalGroup, DeviceData.LocalAddress);
        NL_SetLocalWorkingMode(DeviceData.WorkingMode);
        NL_SetLocalHopLevel(DeviceData.HopLevel);
        APP_InitIO();
        APP_SER_SetLLStackParameters();
        DH_ShowLED(A_LED_BOTH, A_LED_OFF);
        ApplicationStatus = APP_IDLE;
        APP_SER_ResetTimeSyncroTimer();
      }
      break;
      
    case APP_PROG_MODE:
      /* Accepts via power line only programming frames */
      nNStatus = NL_NetworkIndication(&APP_NW_Data, APP_NTW_INDICATION_TOUT);
      if (nNStatus.operation == N_SUCCESS)
      {
        // Load PLM data in the application structure
        APP_PLM_GetNetworkData(&APP_NW_Data, &APP_Frame);
        if (((APP_Frame.type & (~APP_BROADCAST)) == APP_SERVICE_FRAME) && 
            (APP_Frame.buffer[0] == (u8)(SERVICE_PARAM_SET)) && 
            (APP_Frame.buffer[1] == (u8)(PROG_GRP_DEVICE_DATA)))
        {
          // Modify service frame into programming frame type
          APP_Frame.type = APP_PROGRAMMING_FRAME;
          APP_Frame.buffer[0] = PROG_CMD_SET_DATA;
          PLM_PROG_FRAME = TRUE;
        }
      }
      // Verify and exec a programming frame if arrived
      if (APP_PROG_FrameArrived() || PLM_PROG_FRAME)
      {
        DH_Delay_ms(200); // 200 ms
        switch (APP_PROG_GetCommand(&APP_Frame))
        {
          case PROG_CMD_SET_DATA:
            // Store dongle data into the flash memory (and update ram value)
            if (APP_PROG_DownloadDeviceData(&APP_Frame))
            {
              // Write data on flash
              IAP_SetDeviceData(&DeviceData);
              // Send the ACK frame
              if (APP_COMM_SendACKFrame((uint8_t)(APP_PROGRAMMING_FRAME), (uint8_t)(PROG_CMD_SET_DATA), APP_USART_SEND_TIMEOUT))
                DH_ShowLED(A_LED_BOTH, A_LED_FLASH);
              else
                DH_ShowLED(A_LED_ERROR, A_LED_FLASH); // Communication hardware error
            }
            else
            {
              // Send wrong prog group error frame
              APP_COMM_SendErrorFrame(APP_ERROR_WRONG_PROG_GROUP, APP_USART_SEND_TIMEOUT);
            }
            if (PLM_PROG_FRAME)
            {
              PLM_PROG_FRAME = FALSE;
              APP_UserCommStatus = USER_DATA_IDLE;
              ApplicationStatus = APP_IDLE;
              NL_NetworkRestart();
            }
            break;
            
          case PROG_CMD_CLEAR_DATA:
            // Clear flash area
            IAP_EraseDeviceData(IAP_DATA_BASE_ADDRESS);
            // Update dongle structure with empty field
            IAP_GetDeviceData(&DeviceData);
            // Send the ACK frame
            if (APP_COMM_SendACKFrame((uint8_t)(APP_PROGRAMMING_FRAME), (uint8_t)(PROG_CMD_CLEAR_DATA), APP_USART_SEND_TIMEOUT))
              DH_ShowLED(A_LED_BOTH, A_LED_FLASH);
            else
              DH_ShowLED(A_LED_ERROR, A_LED_FLASH); // Communication hardware error
            break;
            
          case PROG_CMD_GET_DATA:
            // Recover Device data from memory
            if (APP_PROG_UploadDeviceData(&APP_Frame))
            {
              // Send the dongle data frame (already loaded from APP_PROG_UploadDeviceData() function)
              if (APP_COMM_SendFrame(&APP_Frame, APP_USART_SEND_TIMEOUT, TRUE))
                DH_ShowLED(A_LED_BOTH, A_LED_FLASH);
              else
                DH_ShowLED(A_LED_ERROR, A_LED_FLASH); // Communication hardware error
            }
            else
            {
              // Send wrong prog group error frame
              APP_COMM_SendErrorFrame(APP_ERROR_WRONG_PROG_GROUP, APP_USART_SEND_TIMEOUT);
            }
            break;
            
          case PROG_CMD_ENTER_PROG_MODE:
            APP_UserCommStatus = USER_DATA_BUSY;
            // Send PROG_CMD_ENTER_PROG_MODE ACK frame
            if (APP_COMM_SendACKFrame((uint8_t)(APP_PROGRAMMING_FRAME), (uint8_t)(PROG_CMD_ENTER_PROG_MODE), APP_USART_SEND_TIMEOUT))
              DH_ShowLED(A_LED_BOTH, A_LED_FLASH);
            else
              DH_ShowLED(A_LED_ERROR, A_LED_FLASH); // Communication hardware error
            break;
            
          case PROG_CMD_EXIT_PROG_MODE:
            // Exit from programming mode
            if (!APP_DeviceProgrammed())
              APP_COMM_SendErrorFrame(APP_ERROR_DEVICE_BLANK, APP_USART_SEND_TIMEOUT); // Device not yet programmed
            else
            {
              APP_UserCommStatus = USER_DATA_IDLE;
              ApplicationStatus = APP_IDLE;
              if (APP_COMM_SendACKFrame((uint8_t)(APP_PROGRAMMING_FRAME), (uint8_t)(PROG_CMD_EXIT_PROG_MODE), APP_USART_SEND_TIMEOUT))
                DH_ShowLED(A_LED_BOTH, A_LED_FLASH);
              else
                DH_ShowLED(A_LED_ERROR, A_LED_FLASH); // Communication hardware error
              
              // Restart the stack if any transit frame is detected during prog mode
              NL_NetworkRestart();
            }
            break;
            
          default:
            // Not recognized command, remain in APP_WAIT_PROG status
            DH_ShowLED(A_LED_DATA, A_LED_OFF);
            DH_ShowLED(A_LED_ERROR, A_LED_ON);
            COMM_EnableReceiver();
            break;
        }
      }
      else
        DH_ShowLED(A_LED_BOTH, A_LED_ON);
      break;
    
    case APP_IDLE:
      // Check for power line incoming data
      nNStatus = NL_NetworkIndication(&APP_NW_Data, APP_NTW_INDICATION_TOUT);
      if (nNStatus.operation == N_DOING)
      {
        /*******************************************************/
        /******************** PL - IDLE ************************/
        /*******************************************************/
        
        // Check if a frame is received from the COMM channel
        if (APP_COMM_FrameReceived(&APP_Frame))
        {
          /* Verify if the received data from the COMM channel are addressed to the device, 
             otherwise will be forwarded through PL */
          if (APP_IsDeviceAddress(&APP_Frame) || (APP_Frame.type == APP_PROGRAMMING_FRAME))
          {
            switch (APP_Frame.type)
            {
              case APP_PROGRAMMING_FRAME:
                DH_ShowLED(A_LED_BOTH, A_LED_ON);
                ApplicationStatus = APP_PROG_MODE;
                break;
                
              case APP_SERVICE_FRAME:
                /* If service command is not native will be notified and managed at the user application level */
                if (APP_SER_CommCommandExec(&APP_Frame))
                  DH_ShowLED(A_LED_BOTH, A_LED_FLASH);
                else
                  DH_ShowLED(A_LED_ERROR, A_LED_FLASH);
                break;
                
              case APP_DATA_FRAME:
              case APP_ERROR_FRAME:
                if (APP_UserCommStatus == USER_DATA_IDLE)
                {
                  DH_ShowLED(A_LED_DATA, A_LED_FLASH);
                  /* User data and user error data received from the COMM module 
                     are transferred to user application level */
                  APP_SetUserData(&APP_Frame, SOURCE_COMM);
                  COMM_EnableReceiver();
                }
                else
                  DH_ShowLED(A_LED_ERROR, A_LED_FLASH);
                break;
                
              case APP_PING_FRAME:
                /* Ping frame received from COMM module and addressed to the device 
                   are not transferred to the user application layer */
                if (APP_COMM_SendACKFrame((uint8_t)(APP_PING_FRAME), (uint8_t)(APP_PING_FRAME), APP_USART_SEND_TIMEOUT))
                  DH_ShowLED(A_LED_BOTH, A_LED_FLASH);
                else
                  DH_ShowLED(A_LED_ERROR, A_LED_FLASH);
                break;
                
              default:
                // Other frame type are ignored
                COMM_EnableReceiver();
                break;
            }
          }
          else
          {
            /* All the frames received from COMM channel addressed to another device 
               are directly forwarded throughout the PL, except for data frames wich are 
               treated ad user data */
            if ((APP_Frame.type == APP_DATA_FRAME) || (APP_Frame.type == APP_ACK_FRAME) || (APP_Frame.type == APP_ERROR_FRAME))
            {
              APP_SetUserData(&APP_Frame, SOURCE_COMM);
              COMM_EnableReceiver();
            }
            else
            {
              ApplicationStatus = APP_CONN_REQUEST;
              // Load network data structure
              APP_PLM_SetNetworkData(&APP_Frame, &APP_NW_Data);
              // Switxh ON data led
              DH_ShowLED(A_LED_DATA, A_LED_ON);
              COMM_EnableReceiver();
            }
          }
        }
        
        // Update transit frame statistics. Can be managed at upper level.
        APP_UpdateRejectFrameStatistics();
               
        // Time clock synchronization signal transmission check
        if (APP_SER_TimeSyncCheck() && (ApplicationStatus == APP_IDLE))
        {
          ApplicationStatus = APP_CONN_REQUEST;
          // Load network data structure
          APP_PLM_SetNetworkData(&APP_Frame, &APP_NW_Data);
          // Flash service LED 
          DH_ShowLED(A_LED_BOTH, A_LED_FLASH);
        }
      }
      else if (nNStatus.operation == N_SUCCESS)
      {
        /*******************************************************/
        /**************** PL - DATA ARRIVED ********************/
        /*******************************************************/
        
        // Load PLM data in the application structure
        APP_PLM_GetNetworkData(&APP_NW_Data, &APP_Frame);
        
        // Data received for this dongle
        switch (APP_Frame.type & (~APP_BROADCAST))
        {
          case APP_PROGRAMMING_FRAME:
            // Programming command received via PLM doesn't leave the PLM in programming mode
            plm_res = APP_PROG_PlmCommandExec(&APP_Frame);
            switch (plm_res)
            {
              case APP_PLM_RES_DONE:
                // Command executed
                DH_ShowLED(A_LED_BOTH, A_LED_FLASH);
                break;
                
              case APP_PLM_RES_CONN_REQUEST:
                // Command received
                DH_ShowLED(A_LED_BOTH, A_LED_FLASH);
                // Dongle data request. Send back through PLM 
                ApplicationStatus = APP_CONN_REQUEST_DEL;
                APP_TimeStamp = DH_Timestamp();
                // Load network data structure
                APP_PLM_SetNetworkData(&APP_Frame, &APP_NW_Data);
                COMM_ACK_ENABLED = FALSE;
                break;
                
              default:
                // Programming command error: wrong programming command frame sent via PLM 
                DH_ShowLED(A_LED_ERROR, A_LED_FLASH);
                APP_SetErrorFrame(&APP_Frame, (u16)(APP_ERROR_WRONG_PROG_COMMAND));
                APP_PLM_SetNetworkData(&APP_Frame, &APP_NW_Data);
                ApplicationStatus = APP_CONN_REQUEST;
                break;
            }
            break;
                
          case APP_SERVICE_FRAME:
            /* If service command is not native will be notified and managed at the user application level */
            plm_res = APP_SER_PlmCommandExec(&APP_Frame);
            switch (plm_res)
            {
              case APP_PLM_RES_DONE:
                // Command executed
                DH_ShowLED(A_LED_BOTH, A_LED_FLASH);
                break;
                
              case APP_PLM_RES_FW_UPDATING:
                // Firmware updating
                if (UpdateLEDon)
                {
                  UpdateLEDon = FALSE;
                  DH_ShowLED(A_LED_BOTH, A_LED_OFF);
                }
                else
                {
                  UpdateLEDon = TRUE;
                  DH_ShowLED(A_LED_BOTH, A_LED_ON);
                }
                break;
                
              case APP_PLM_RES_CONN_REQUEST:
                // Dongle data request. Send back through PLM 
                ApplicationStatus = APP_CONN_REQUEST_DEL;
                APP_TimeStamp = DH_Timestamp();
                // Load network data structure
                APP_PLM_SetNetworkData(&APP_Frame, &APP_NW_Data);
                // Flash service LED 
                DH_ShowLED(A_LED_BOTH, A_LED_FLASH);
                break;
                
              case APP_PLM_RES_USER_OVERRUN:
                // Previous user service routine not managed
                DH_ShowLED(A_LED_ERROR, A_LED_FLASH);
                break;
                
              default:
                // Service request error
                DH_ShowLED(A_LED_ERROR, A_LED_FLASH);
                APP_SetErrorFrame(&APP_Frame, (u16)(APP_ERROR_SERVICE_CMD_ERROR));
                APP_PLM_SetNetworkData(&APP_Frame, &APP_NW_Data);
                ApplicationStatus = APP_CONN_REQUEST;
                break;
            }
            break;
            
          case APP_DATA_FRAME:
          case APP_ERROR_FRAME:
            if (APP_UserCommStatus == USER_DATA_IDLE)
            {
              DH_ShowLED(A_LED_DATA, A_LED_FLASH);
              /* User data and user error data received from the PL
                 are transferred to user application level */
              APP_SetUserData(&APP_Frame, SOURCE_PLM);
              // Update transit frame statistics. Can be managed at upper level.
              if (APP_DataFlagReady == FALSE)
              {
                if (NL_GetFrameRxFlags(&APP_TFdataflags))
                  APP_DataFlagReady = TRUE;
              }
            }
            else
              DH_ShowLED(A_LED_ERROR, A_LED_FLASH);
            break;
            
          default:
            // Other frame type are ignored
            break;
        }          
      }
      else if (nNStatus.operation == N_ERROR)
      {
        /*******************************************************/
        /***************** PL - NETWORK ERROR ******************/
        /*******************************************************/

        // Network data error
        ApplicationStatus = APP_IDLE;
        // LED indication and notification to user level
        DH_ShowLED(A_LED_ERROR, A_LED_FLASH);
        // Update transit frame statistics. Can be managed at upper level.
        APP_UpdateRejectFrameStatistics();
      }
      break;
      
    case APP_CONN_REQUEST_DEL:
      if (DH_DelayElapsed(APP_TimeStamp, APP_INFO_REQUEST_DELAY))
        ApplicationStatus = APP_CONN_REQUEST;
      break;
      
    case APP_CONN_REQUEST:
      // Connection request
      nNStatus = NL_NetworkRequest(&APP_NW_Data, APP_NTW_REQUEST_TOUT);
      if (nNStatus.operation == N_DOING)
      {
        /*******************************************************/
        /************ PL - DATA TRANSMISSION ONGOING ***********/
        /*******************************************************/
        
        if (APP_PROG_FrameArrived())
        {
          // If a programming frame is received from the COMM channel during transmission, the PLM operation are aborted
          NL_NetworkRestart();
          ApplicationStatus = APP_PROG_MODE;
          DH_ShowLED(A_LED_BOTH, A_LED_ON);
        }
        else
          COMM_EnableReceiver(); // Other frame type are ignored
      }
      else if (nNStatus.operation == N_SUCCESS)
      {
        /*******************************************************/
        /************ PL - DATA TRANSMISSION DONE **************/
        /*******************************************************/
        
        // Data successfully sent
        ApplicationStatus = APP_IDLE;
        // Restore the programmed working mode (if changed runtime by user)
        NL_SetLocalWorkingMode(DeviceData.WorkingMode);
        // Check if the request comes from user
        if (APP_UserCommStatus == USER_DATA_TRANSMISSION_START)
        {
          APP_UserCommStatus = USER_DATA_TRANSMISSION_END;
          APP_UserData.error = APP_ERROR_NONE;
        }
        if (COMM_ACK_ENABLED)
        {
          bool bres;
          
          if (APP_Frame.type == APP_PING_FRAME)
            bres = APP_COMM_SendACKFrame((uint8_t)(APP_PING_FRAME), (uint8_t)(APP_PING_FRAME), APP_USART_SEND_TIMEOUT);
          else
            bres = APP_COMM_SendACKFrame((uint8_t)(APP_DATA_FRAME), 0, APP_USART_SEND_TIMEOUT);            
          
          if (bres)
            DH_ShowLED(A_LED_BOTH, A_LED_OFF);    // Switch off LED indication
          else
            DH_ShowLED(A_LED_ERROR, A_LED_FLASH); // Communication hardware error
        }
        else
        {
          DH_ShowLED(A_LED_BOTH, A_LED_OFF);    // Switch off LED indication
          COMM_ACK_ENABLED = TRUE;
        }
      }
      else if (nNStatus.operation == N_ERROR)
      {
        /*******************************************************/
        /************ PL - DATA TRANSMISSION ERROR *************/
        /*******************************************************/
        
        // Communication error
        ApplicationStatus = APP_IDLE;
        // Restore the programmed working mode (if changed by user runtime)
        NL_SetLocalWorkingMode(DeviceData.WorkingMode);
        // Check if the request comes from user
        if (APP_UserCommStatus == USER_DATA_TRANSMISSION_START)
        {
          APP_UserCommStatus = USER_DATA_COMMUNICATION_ERROR;
          APP_UserData.error = APP_GetNetworkError(nNStatus.error);
        }
        else
          APP_COMM_SendErrorFrame(APP_ERROR_COMM_TIMEOUT, APP_USART_SEND_TIMEOUT);
        DH_ShowLED(A_LED_ERROR, A_LED_FLASH);
      }
      break;
      
    default:
      // Unrecognized status
      ApplicationStatus = APP_IDLE;
      break;
  }
  
  /*******************************************************/
  /********** DATA LINK STACK ENGINE UPDATE **************/
  /*******************************************************/
  NL_DeviceStackUpdate();
}

/*******************************************************************************
* Function Name  : APP_SER_SetTimeSynchroFrame
* Description    : Set time synchronization frame to send in broadcast
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void APP_SER_SetTimeSynchroFrame(void)
{
  APP_Frame.type = (APP_ftype_t)(APP_SERVICE_FRAME | APP_BROADCAST);
  APP_Frame.len = 4;
  APP_Frame.buffer[0] = APP_SER_TIME_SYNC;  // Service Command (tyme sinchronization request)
  DH_GetSysTime(APP_Frame.buffer + 1);      // Get the actual system time (hh, mm, ss -> 3 bytes)
}

/*******************************************************************************
* Function Name  : APP_GetUserData
* Description    : Load the exchange data structure with user data
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void APP_GetUserData(APP_userdata_t  *u_data, APP_frame_t *app_f)
{
  uint8_t i;
  
  app_f->type = u_data->type;  
  if (u_data->broadcast == TRUE)
    app_f->type |= APP_BROADCAST;
  
  app_f->group = u_data->group;
  app_f->address = u_data->address;
  app_f->len = u_data->len;
  
  for (i = 0; i < app_f->len; i++)
    app_f->buffer[i] = u_data->data[i];
}

/*******************************************************************************
* Function Name  : APP_SetUserData
* Description    : Load the received user data into the exchange data structure
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void APP_SetUserData(APP_frame_t *adata, APP_source_t src)
{
  uint8_t i;
  
  // Set the user data arrived flag
  APP_UserCommStatus = USER_DATA_ARRIVED;
  
  APP_UserData.source = src;
  APP_UserData.type = (APP_ftype_t)(((uint8_t)(adata->type)) & (~APP_BROADCAST));
  if (adata->type & APP_BROADCAST)
    APP_UserData.broadcast = TRUE;
  else
    APP_UserData.broadcast = FALSE;
  APP_UserData.group = adata->group;
  APP_UserData.address = adata->address;
  APP_UserData.len = adata->len;
  for (i = 0; i < APP_UserData.len; i++)
    APP_UserData.data[i] = adata->buffer[i];
  
  APP_UserData.error = APP_ERROR_NONE;
}

/*******************************************************************************
* Function Name  : APP_ReceiveUserData
* Description    : Receive the user data through the selected source
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
APP_userflag_t APP_ReceiveUserData(APP_userdata_t *userdata)
{
  uint8_t i;
  
  if (APP_UserCommStatus == USER_DATA_ARRIVED)
  {
    userdata->source = APP_UserData.source;
    userdata->type = APP_UserData.type;
    userdata->broadcast = APP_UserData.broadcast;
    userdata->group = APP_UserData.group;
    userdata->address = APP_UserData.address;
    userdata->len = APP_UserData.len;
    for (i = 0; i < APP_UserData.len; i++)
      userdata->data[i] = APP_UserData.data[i];
    
    APP_UserCommStatus = USER_DATA_IDLE;
    return USER_DATA_ARRIVED;
  }
  else if (APP_UserCommStatus == USER_DATA_COMMUNICATION_ERROR)
  {
    APP_UserCommStatus = USER_DATA_IDLE;
    return USER_DATA_COMMUNICATION_ERROR;
  }
  
  return USER_DATA_IDLE;
}

/*******************************************************************************
* Function Name  : APP_TransmitUserData
* Description    : Transmit the user data through the selected source
* Input          : user data structure, source (COMM or PLM)
* Output         : None
* Return         : None
*******************************************************************************/
APP_userflag_t APP_TransmitUserData(APP_userdata_t *userdata)
{
  uint8_t i;
  
  if ((APP_UserCommStatus == USER_DATA_ARRIVED) || (APP_UserCommStatus == USER_DATA_IDLE))
  {
    APP_UserData.source = userdata->source;
    APP_UserData.type = userdata->type;
    APP_UserData.broadcast = userdata->broadcast;
    APP_UserData.group = userdata->group;
    APP_UserData.address = userdata->address;
    APP_UserData.len = userdata->len;
    for (i = 0; i < APP_UserData.len; i++)
      APP_UserData.data[i] = userdata->data[i];
    
    // Load the exchange structure (application layer)
    APP_GetUserData(&APP_UserData, &APP_Frame);
        
    if (APP_UserData.source == SOURCE_PLM)
    {
      // Transmission via PLM 
      APP_UserCommStatus = USER_DATA_TRANSMISSION_START;
      APP_PLM_SetNetworkData(&APP_Frame, &APP_NW_Data);
      ApplicationStatus = APP_CONN_REQUEST; // Start the PLM data transmission 
    }
    else
    {
      // Transmission via COMM 
      APP_UserCommStatus = USER_DATA_IDLE;
      if (APP_COMM_SendFrame(&APP_Frame, APP_USART_SEND_TIMEOUT, FALSE))
        return USER_DATA_TRANSMISSION_END;
      return USER_DATA_COMMUNICATION_ERROR;
    }
  }
  else if (APP_UserCommStatus == USER_DATA_TRANSMISSION_END)
  {
    APP_UserCommStatus = USER_DATA_IDLE;
    return USER_DATA_TRANSMISSION_END;
  }
  else if (APP_UserCommStatus == USER_DATA_COMMUNICATION_ERROR)
  {
    APP_UserCommStatus = USER_DATA_IDLE;
    return USER_DATA_COMMUNICATION_ERROR;
  }
  
  return USER_DATA_TRANSMISSION_START;
}

/*******************************************************************************
* Function Name  : APP_SER_CommCommandExec
* Description    : Decode and execute the service command
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool APP_SER_CommCommandExec(APP_frame_t *app_frame)
{
  /* 
    Execute only native device service commands, all the other service commands are 
    transferred to the main and managed by the user program
  */
  switch (APP_SER_GetCommand(app_frame))
  {
    // Set data link stack parameters and upload the device structure
    case SERVICE_PARAM_SET:
      if (APP_PROG_DownloadDeviceData(app_frame))
      {
        // Write data on flash
        IAP_SetDeviceData(&DeviceData);
        return (APP_COMM_SendACKFrame((uint8_t)(APP_SERVICE_FRAME), (uint8_t)(SERVICE_PARAM_SET), APP_USART_SEND_TIMEOUT));
      }
      else
        return (APP_COMM_SendErrorFrame(APP_ERROR_SERVICE_GRP_UNKNOWN, APP_USART_SEND_TIMEOUT));
      
    // Load data link stack parameters from the device structure to the app_frame  
    case SERVICE_PARAM_GET:
      if (APP_PROG_UploadDeviceData(app_frame))
        return (APP_COMM_SendFrame(app_frame, APP_USART_SEND_TIMEOUT, TRUE));
      else
        return (APP_COMM_SendErrorFrame(APP_ERROR_SERVICE_GRP_UNKNOWN, APP_USART_SEND_TIMEOUT));
      
    // Update the system time
    case SERVICE_PLM_CLOCK_SET:
#ifdef DEVICE_USE_RTC
      APP_SYSTEM_TIME_UPDATED = TRUE;
      if (DH_SetSysTime(APP_SER_GetDataAddress(app_frame)))
        return (APP_COMM_SendACKFrame((uint8_t)(APP_SERVICE_FRAME), (uint8_t)(SERVICE_PLM_CLOCK_SET), APP_USART_SEND_TIMEOUT));
      else
        return (APP_COMM_SendErrorFrame(APP_ERROR_SERVICE_CMD_ERROR, APP_USART_SEND_TIMEOUT));
#else
      return (APP_COMM_SendErrorFrame(APP_ERROR_RTC_DISABLED, APP_USART_SEND_TIMEOUT));
#endif
      
    // Get the actual system time (hh, mm, ss)
    case SERVICE_PLM_CLOCK_GET:
#ifdef DEVICE_USE_RTC
      DH_GetSysTime(APP_SER_GetDataAddress(app_frame));
      app_frame->len = 4;
      return (APP_COMM_SendFrame(app_frame, APP_USART_SEND_TIMEOUT, TRUE));
#else
      return (APP_COMM_SendErrorFrame(APP_ERROR_RTC_DISABLED, APP_USART_SEND_TIMEOUT));
#endif
      
    // Get the status of the dongle inputs pin (if configured)
    case SERVICE_INPUTS_GET:
      APP_SER_GetDongleInputs(app_frame);
      return (APP_COMM_SendFrame(app_frame, APP_USART_SEND_TIMEOUT, TRUE));
      
    // Set the value of the output pins
    case SERVICE_OUTPUTS_SET:
      DH_SetOutputs(*(APP_SER_GetDataAddress(app_frame)));
      return (APP_COMM_SendACKFrame((uint8_t)(APP_SERVICE_FRAME), (uint8_t)(SERVICE_OUTPUTS_SET), APP_USART_SEND_TIMEOUT));
    
    // Set the IO port configuration
    case SERVICE_IO_CONFIG_SET:      
      DeviceData.IOConfiguration = *(APP_SER_GetDataAddress(app_frame));
      APP_SER_SetIOConfig(DeviceData.IOConfiguration);
      return (APP_COMM_SendACKFrame((uint8_t)(APP_SERVICE_FRAME), (uint8_t)(SERVICE_IO_CONFIG_SET), APP_USART_SEND_TIMEOUT));

    // Get the IO port configuration
    case SERVICE_IO_CONFIG_GET:
        APP_SER_GetDongleIOconfig(app_frame);
      return (APP_COMM_SendFrame(app_frame, APP_USART_SEND_TIMEOUT, TRUE));
      
    // Get the application or the stack firmware release
    case SERVICE_FW_REL_GET:
      APP_SER_GetFwRelease(app_frame, APP_SER_GetCommand(app_frame));
      return (APP_COMM_SendFrame(app_frame, APP_USART_SEND_TIMEOUT, TRUE));

    // Execute a software reset
    case SERVICE_SOFTWARE_RESET:
//      if (NL_NetworkInit(APP_InitStackWorkingMode(), APP_InitStackDefaults()))
//        return (APP_COMM_SendACKFrame((uint8_t)(APP_SERVICE_FRAME), (uint8_t)(SERVICE_SOFTWARE_RESET), APP_USART_SEND_TIMEOUT));
//      else
//        return (APP_COMM_SendErrorFrame(APP_ERROR_NODE_INIT_FAILED, APP_USART_SEND_TIMEOUT));
      APP_ApplicationInit();
      SOFTWARE_RESET_REQUEST = TRUE;
      ApplicationStatus = APP_INIT;
      return (APP_COMM_SendACKFrame((uint8_t)(APP_SERVICE_FRAME), (uint8_t)(SERVICE_SOFTWARE_RESET), APP_USART_SEND_TIMEOUT));
      
    // Execute an hardware reset if the watchdog is enabled
    case SERVICE_HARDWARE_RESET:
      NVIC_SystemReset();
      while(1);
    
    default:
      // Other service commands are considered user service commands and managed at main
      if (APP_UserCommStatus == USER_DATA_ARRIVED)
        return FALSE;
      APP_SetUserData(app_frame, SOURCE_COMM);
      COMM_EnableReceiver();
      return TRUE;
  }
}

/*******************************************************************************
* Function Name  : APP_SoftwareResetRequested
* Description    : Return the software reset request flag
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool APP_SoftwareResetRequested(void)
{
  if (SOFTWARE_RESET_REQUEST)
  {
    SOFTWARE_RESET_REQUEST = FALSE;
    return TRUE;
  }
  return FALSE;
}

/*******************************************************************************
* Function Name  : APP_TransferResult
* Description    : Return the GET result to user structure
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
APP_PLM_res_t APP_TransferResult(APP_frame_t *app_frame)
{
  if (APP_UserCommStatus == USER_DATA_ARRIVED)
    return APP_PLM_RES_USER_OVERRUN;
  APP_SetUserData(app_frame, SOURCE_PLM);
  return APP_PLM_RES_DONE;
}

/*******************************************************************************
* Function Name  : APP_SER_PlmCommandExec
* Description    : Decode and execute the service command received via PLM 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
APP_PLM_res_t APP_SER_PlmCommandExec(APP_frame_t *app_frame)
{
  /* 
    Execute only native device service commands, all the other service commands are 
    transferred to the main and managed by the user program
  */
  switch (APP_SER_GetCommand(app_frame))
  {
    // Set data link stack parameters and upload the device structure
    case SERVICE_PARAM_SET:
      if (APP_PROG_DownloadDeviceData(app_frame))
      {
        // Write data on flash
        IAP_SetDeviceData(&DeviceData);
        // Send the ACK frame
        return APP_PLM_RES_DONE;
      }
      else
        return APP_PLM_RES_ERROR;
      
    // Load data link stack parameters from the device structure to the app_frame  
    case SERVICE_PARAM_GET:
      if (app_frame->len > APP_SER_GET_CMD_LEN+1)
        return APP_TransferResult(app_frame); // Service get results from remote PLM are transferred to user
      else
      {
        if (APP_PROG_UploadDeviceData(app_frame))
          return APP_PLM_RES_CONN_REQUEST; // Request to send data via PLM 
        else
          return APP_PLM_RES_ERROR;
      }

    // Update the system time
    case SERVICE_PLM_CLOCK_SET:
      APP_SYSTEM_TIME_UPDATED = TRUE;
      if (DH_SetSysTime(APP_SER_GetDataAddress(app_frame)))
        return APP_PLM_RES_DONE;
      else
        return APP_PLM_RES_ERROR;
      
    // Get the actual system time (hh, mm, ss)
    case SERVICE_PLM_CLOCK_GET:
      if (app_frame->len > APP_SER_GET_CMD_LEN)
        return APP_TransferResult(app_frame); // Service get results from remote PLM are transferred to user
      else
      {
        DH_GetSysTime(APP_SER_GetDataAddress(app_frame));
        app_frame->len = 4;
        return APP_PLM_RES_CONN_REQUEST; // Request to send data via PLM 
      }
      
    // Get the application or the stack firmware release
    case SERVICE_FW_REL_GET:
      if (app_frame->len > APP_SER_GET_CMD_LEN)
        return APP_TransferResult(app_frame); // Service get results from remote PLM are transferred to user
      else
      {
        APP_SER_GetFwRelease(app_frame, APP_SER_GetCommand(app_frame));
        return APP_PLM_RES_CONN_REQUEST; // Request to send data via PLM 
      }
      
    // Network discover request
    case SERVICE_NET_DISCOVER_REQ:
      if (app_frame->len > APP_SER_GET_CMD_LEN)
        return APP_TransferResult(app_frame); // Service get results from remote PLM are transferred to user
      else
      { 
        app_frame->type = APP_SERVICE_FRAME;
        app_frame->buffer[1] = SERVICE_NET_DISCOVER_REQ;// Duplicate command
        app_frame->len = 2;
        DH_Delay_ms(1000); // 1s delay before discovery response 
        return APP_PLM_RES_CONN_REQUEST; // Request to send data via PLM 
      }
        
    // Get the status of the dongle inputs pin (if configured)
    case SERVICE_INPUTS_GET:
      if (app_frame->len > APP_SER_GET_CMD_LEN)
        return APP_TransferResult(app_frame); // Service get results from remote PLM are transferred to user
      else
      {
        APP_SER_GetDongleInputs(app_frame);
        return APP_PLM_RES_CONN_REQUEST; // Request to send data via PLM 
      }
      
    // Set the value of the output pins
    case SERVICE_OUTPUTS_SET:
      DH_SetOutputs(*(APP_SER_GetDataAddress(app_frame)));
      return APP_PLM_RES_DONE;  
      
    // Get the IO port configuration
    case SERVICE_IO_CONFIG_GET:
      if (app_frame->len > APP_SER_GET_CMD_LEN)
        return APP_TransferResult(app_frame); // Service get results from remote PLM are transferred to user
      else
      {
        APP_SER_GetDongleIOconfig(app_frame);
        return APP_PLM_RES_CONN_REQUEST; // Request to send data via PLM 
      }
      
    // Set the IO port configuration
    case SERVICE_IO_CONFIG_SET:
      DeviceData.IOConfiguration = *(APP_SER_GetDataAddress(app_frame));
      APP_SER_SetIOConfig(DeviceData.IOConfiguration);
      return APP_PLM_RES_DONE;

    // Remote Firmware Update: set signature and new image size
    case SERVICE_RFU_SET_IMG_HEADER:
      RFU_GetImageSize(APP_SER_GetDataAddress(app_frame));
      RFU_GetImageSignature(APP_SER_GetDataAddress(app_frame) + 2);
      return APP_PLM_RES_FW_UPDATING;

    // Remote Firmware Update: get image data
    case SERVICE_RFU_SET_IMG_DATA:
      if (RFU_SetImageData(APP_SER_GetDataAddress(app_frame) + 2, APP_SER_GetFwUpdAddress(app_frame), app_frame->len - 3))
        return APP_PLM_RES_FW_UPDATING;
      else
        return APP_PLM_RES_ERROR;

    // Remote Firmware Update: swap active image 
    case SERVICE_RFU_SWAP_IMG:
      if (RFU_SwapImageData())
        return APP_PLM_RES_DONE;
      else
        return APP_PLM_RES_ERROR;
      
    // Execute a software reset
    case SERVICE_SOFTWARE_RESET:
      APP_ApplicationInit();
      SOFTWARE_RESET_REQUEST = TRUE;
      ApplicationStatus = APP_INIT;
      return APP_PLM_RES_DONE;
    // Execute an hardware reset if the watchdog is enabled
    case SERVICE_HARDWARE_RESET:
      NVIC_SystemReset();
      while(1);
      
    default:
      // Other service commands are considered user service commands and managed at main
      if (APP_UserCommStatus == USER_DATA_ARRIVED)
        return APP_PLM_RES_USER_OVERRUN;
      APP_SetUserData(app_frame, SOURCE_PLM);
      return APP_PLM_RES_DONE;
  }
}

/*******************************************************************************
* Function Name  : APP_PROG_PlmCommandExec
* Description    : Decode and execute the programming command received via PLM 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
APP_PLM_res_t APP_PROG_PlmCommandExec(APP_frame_t *app_frame)
{
  /* Execute the programming commands and returns in idle mode */
  switch (APP_PROG_GetCommand(app_frame))
  {
    case PROG_CMD_SET_DATA:
      // Store dongle data into the flash memory (and update ram value)
      if (APP_PROG_DownloadDeviceData(app_frame))
      {
        // Write data on flash
        IAP_SetDeviceData(&DeviceData);
        // Send the ACK frame
        return APP_PLM_RES_DONE;
      }
      else
        return APP_PLM_RES_ERROR;
          
    case PROG_CMD_CLEAR_DATA:
      // Clear flash area
      IAP_EraseDeviceData(IAP_DATA_BASE_ADDRESS);
      // Update dongle structure with empty field
      IAP_GetDeviceData(&DeviceData);
      // Send the ACK frame
      return APP_PLM_RES_DONE;
      
    case PROG_CMD_GET_DATA:
      // Recover Device data from memory
      if (APP_PROG_UploadDeviceData(&APP_Frame))
        return APP_PLM_RES_CONN_REQUEST;
      else
        return APP_PLM_RES_ERROR;
      
    case PROG_CMD_ENTER_PROG_MODE:
    case PROG_CMD_EXIT_PROG_MODE:
      // Enter and exit from prog mode is not allowed if received via PLM 
      return APP_PLM_RES_DONE;
      
    default:
      // Not recognized command, remain in APP_WAIT_PROG status
      return APP_PLM_RES_ERROR;
  }
}



/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/

