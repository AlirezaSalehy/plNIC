/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : comm.h
* Author             : Systems LAB - IMS (Industrial & Multisegment Sector)
* Date First Issued  : 03/12/2008
* Description        : Application layer routine definitions
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
#ifndef __APPLICATION_H
#define __APPLICATION_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"
#include "interfaceconfig.h"

/* Private define ------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/

/* APPLICATION FRAME TYPE */
typedef enum 
{
  APP_DATA_FRAME                  = 0x00,
  APP_SERVICE_FRAME               = 0x01,
  APP_PING_FRAME                  = 0x02,
  APP_ERROR_FRAME                 = 0x03,
  APP_PROGRAMMING_FRAME           = 0x04,
  APP_ACK_FRAME                   = 0x05
}APP_ftype_t;

/* PROGRAMMING COMMANDS */
typedef enum
{
  PROG_CMD_IDLE                   = 0x00, // No command
  PROG_CMD_ENTER_PROG_MODE        = 0x01, // Enter the programming mode
  PROG_CMD_EXIT_PROG_MODE         = 0x02, // Exit from programming mode
  PROG_CMD_SET_DATA               = 0x03, // Write data into the internal flash
  PROG_CMD_GET_DATA               = 0x04, // Get data from internal flash
  PROG_CMD_CLEAR_DATA             = 0x05, // Clear data resetting the programming flag
  PROG_CMD_DEVICE_BLANK           = 0x06  // Device blank: run mode impossible
}APP_PROG_CMD_t;

/* PROGRAMMING GROUPS */
typedef enum
{
  PROG_GRP_DEVICE_DATA            = 0x00, // Device Data
  PROG_GRP_LL_STACK_PARAM         = 0x01, // Link layer stack parameters
  PROG_GRP_USER_DATA              = 0x02  // User program
}APP_PROG_GROUP_t;

/* APPLICATION ERRORS */
typedef enum
{
  APP_ERROR_NONE                  = 0x00, // No error
  APP_ERROR_GENERIC               = 0x01, // Generic communication error
  APP_ERROR_COMM_TIMEOUT          = 0x02, // Communication timeout error
  APP_ERROR_SERVICE_GRP_UNKNOWN   = 0x03, // Service group unknown error
  APP_ERROR_SERVICE_CMD_ERROR     = 0x04, // Service command error
  APP_ERROR_COMMUNICATION         = 0x05, // Communication error
  APP_ERROR_ISOLATED_NODE         = 0x06, // Node unreachable error
  APP_ERROR_HARDWARE              = 0x07, // Hardware error
  APP_ERROR_WRONG_PROG_COMMAND    = 0x08, // Wrong programming command error
  APP_ERROR_WRONG_PROG_GROUP      = 0x09, // Wrong programming group error  
  APP_ERROR_DEVICE_BLANK          = 0x0a, // Device blank
  APP_ERROR_RTC_ERROR             = 0x0b, // Error setting the system time
  APP_ERROR_WATCHDOG_DISABLED     = 0x0c, // Hardware reset impossible
  APP_ERROR_NODE_INIT_FAILED      = 0x0d, // Node initialization failure
  APP_ERROR_RTC_DISABLED          = 0x0e  // Internal RTC disabled
}APP_ERROR_t;

/* PERIPHERAL SOURCE TYPE */
typedef enum 
{
  SOURCE_COMM,
  SOURCE_USB,
  SOURCE_SPI,
  SOURCE_PLM
}APP_source_t;

/* USER FRAME STRUCTURE */
typedef struct
{
  APP_source_t  source;
  APP_ftype_t   type;
  bool          broadcast;
  u16           group;
  u32           address;
  uint8_t       len;
  uint8_t       data[USER_PAYLOAD_SIZE];
  APP_ERROR_t   error;
}APP_userdata_t;

/* USER COMMUNICATION FLAGS */
typedef enum
{
  USER_DATA_TRANSMISSION_START,
  USER_DATA_TRANSMISSION_END,
  USER_DATA_ARRIVED,
  USER_DATA_COMMUNICATION_ERROR,
  USER_DATA_BUSY,
  USER_DATA_IDLE    
}APP_userflag_t;

/* SERVICE COMMANDS */
typedef enum 
{
  /* NATIVE SERVICE COMMANDS */
  SERVICE_SOFTWARE_RESET      = 0x00,
  SERVICE_HARDWARE_RESET      = 0x01,
  SERVICE_PARAM_SET           = 0x02,
  SERVICE_PARAM_GET           = 0x03,
  SERVICE_INPUTS_GET          = 0x04,
  SERVICE_OUTPUTS_SET         = 0x05,
  SERVICE_FW_REL_GET          = 0x06,
  SERVICE_PLM_CLOCK_SET       = 0x07,
  SERVICE_PLM_CLOCK_GET       = 0x08,
  SERVICE_IO_CONFIG_SET       = 0x09,
  SERVICE_IO_CONFIG_GET       = 0x0a,
  SERVICE_NET_DISCOVER_REQ    = 0x0b,
  SERVICE_RFU_SET_IMG_HEADER  = 0x0c,
  SERVICE_RFU_SET_IMG_DATA    = 0x0d,
  SERVICE_RFU_SWAP_IMG        = 0x0e,
  
  /* USER DEFINED SERVICE COMANDS */
  // SERVICE_USER_CMD_xx       = 0x..,
}APP_SER_CMD_t;

/* TRANSIT FRAME TYPE */
typedef enum 
{
  APP_TF_DATA     = 0x00,
  APP_TF_ERROR    = 0x01,
  APP_TF_PROGR    = 0x02,
  APP_TF_RES      = 0x03,
  APP_TF_ACK      = 0x04,
  APP_TF_bACK     = 0x05,
  APP_TF_PING     = 0x06,
  APP_TF_SERVICE  = 0x07,
  APP_TF_UNKNOWN  = 0x7f
}APP_TF_t;

/* SERVICE FLAGS */
typedef struct
{
  APP_TF_t  type;
  uint8_t   FECcorrections;
  bool      wrongpostamble;
  bool      wrongCRC;
  bool      hopoverrun;
  bool      framerejected;
}APP_SER_FLAGS_t;

/* Exported variables --------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/* GENERAL */
void            APP_ApplicationInit(void);
bool            APP_ApplicationReady(void);
void            APP_StackUpdate(void);
bool            APP_DeviceAddressed(APP_userdata_t *frm);
void            APP_GetLocalAddress(uint16_t *group, uint32_t *address);
bool            APP_GetTransitFrameFlags(APP_SER_FLAGS_t *flags);
bool            APP_SoftwareResetRequested(void);

/* NETWORK */
APP_userflag_t  APP_ReceiveUserData(APP_userdata_t *userdata);
APP_userflag_t  APP_TransmitUserData(APP_userdata_t *userdata);

/* USER */
void            APP_GetUserSettings(u16 *usersettings);
void            APP_SetUserSettings(u16 *usersettings);

#endif /* __APPLICATION_H */

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/

