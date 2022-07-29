/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : dongle.c
* Author             : Systems LAB - IMS (Industrial & Multisegment Sector)
* Date First Issued  : 10/02/2012
* Description        : Dongle hardware routines
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

/* Includes ------------------------------------------------------------------*/
#include "dongle.h"
#include "stk_p.h"
#include <time.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define LED_RED_PORT_N    ((GPIO_TypeDef *)(GPIOA_BASE + ((GPIOB_BASE - GPIOA_BASE) * LED_RED_PORT)))
#define LED_GREEN_PORT_N  ((GPIO_TypeDef *)(GPIOA_BASE + ((GPIOB_BASE - GPIOA_BASE) * LED_GREEN_PORT)))
#define LED_RED_PIN_N     ((uint16_t)(GPIO_Pin_0 << LED_RED_PIN))
#define LED_GREEN_PIN_N   ((uint16_t)(GPIO_Pin_0 << LED_GREEN_PIN))
#define LED_RED_RCC       ((uint32_t)(RCC_APB2Periph_GPIOA << LED_RED_PORT))
#define LED_GREEN_RCC     ((uint32_t)(RCC_APB2Periph_GPIOA << LED_GREEN_PORT))

#define LED_RED_ON        (GPIO_ResetBits(LED_RED_PORT_N, LED_RED_PIN_N))
#define LED_RED_OFF       (GPIO_SetBits(LED_RED_PORT_N, LED_RED_PIN_N))
#define LED_GREEN_ON      (GPIO_ResetBits(LED_GREEN_PORT_N, LED_GREEN_PIN_N))
#define LED_GREEN_OFF     (GPIO_SetBits(LED_GREEN_PORT_N, LED_GREEN_PIN_N))
#define LED_ORANGE_ON     LED_RED_ON; LED_GREEN_ON
#define LED_ALL_OFF       LED_GREEN_OFF; LED_RED_OFF

/* Inputs Outputs pins */
#define IO_1_PORT_N       ((GPIO_TypeDef *)(GPIOA_BASE + ((GPIOB_BASE - GPIOA_BASE) * IO_1_PORT)))
#define IO_2_PORT_N       ((GPIO_TypeDef *)(GPIOA_BASE + ((GPIOB_BASE - GPIOA_BASE) * IO_2_PORT)))
#define IO_3_PORT_N       ((GPIO_TypeDef *)(GPIOA_BASE + ((GPIOB_BASE - GPIOA_BASE) * IO_3_PORT)))
#define IO_4_PORT_N       ((GPIO_TypeDef *)(GPIOA_BASE + ((GPIOB_BASE - GPIOA_BASE) * IO_4_PORT)))
#define IO_5_PORT_N       ((GPIO_TypeDef *)(GPIOA_BASE + ((GPIOB_BASE - GPIOA_BASE) * IO_5_PORT)))
#define IO_6_PORT_N       ((GPIO_TypeDef *)(GPIOA_BASE + ((GPIOB_BASE - GPIOA_BASE) * IO_6_PORT)))
#define IO_7_PORT_N       ((GPIO_TypeDef *)(GPIOA_BASE + ((GPIOB_BASE - GPIOA_BASE) * IO_7_PORT)))
#define IO_8_PORT_N       ((GPIO_TypeDef *)(GPIOA_BASE + ((GPIOB_BASE - GPIOA_BASE) * IO_8_PORT)))

#define IO_1_PIN_N        ((uint16_t)(GPIO_Pin_0 << IO_1_PIN))
#define IO_2_PIN_N        ((uint16_t)(GPIO_Pin_0 << IO_2_PIN))
#define IO_3_PIN_N        ((uint16_t)(GPIO_Pin_0 << IO_3_PIN))
#define IO_4_PIN_N        ((uint16_t)(GPIO_Pin_0 << IO_4_PIN))
#define IO_5_PIN_N        ((uint16_t)(GPIO_Pin_0 << IO_5_PIN))
#define IO_6_PIN_N        ((uint16_t)(GPIO_Pin_0 << IO_6_PIN))
#define IO_7_PIN_N        ((uint16_t)(GPIO_Pin_0 << IO_7_PIN))
#define IO_8_PIN_N        ((uint16_t)(GPIO_Pin_0 << IO_8_PIN))

/* Private variables ---------------------------------------------------------*/
uint32_t dhtstamp;
uint16_t dhseconds = 0;
GPIO_TypeDef* port_arr[8] = {IO_1_PORT_N, IO_2_PORT_N, IO_3_PORT_N, IO_4_PORT_N, IO_5_PORT_N, IO_6_PORT_N, IO_7_PORT_N, IO_8_PORT_N};
uint16_t pin_arr[8] = {IO_1_PIN_N, IO_2_PIN_N, IO_3_PIN_N, IO_4_PIN_N, IO_5_PIN_N, IO_6_PIN_N, IO_7_PIN_N, IO_8_PIN_N};
uint8_t portn_arr[8] = {IO_1_PORT, IO_2_PORT, IO_3_PORT, IO_4_PORT, IO_5_PORT, IO_6_PORT, IO_7_PORT, IO_8_PORT};

bool SYS_TIME_ADJUSTED = FALSE;

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


/*******************************************************************************
* Function Name  : DH_Delay_ms
* Description    : 1 ms delay tick
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DH_Delay_ms(uint16_t ms)
{
  uint32_t del_us = ms * 10;
  PL_Delay(del_us);
}

/*******************************************************************************
* Function Name  : DH_DelayElapsed
* Description    : Verify if the given ms are elapsed
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool DH_DelayElapsed(uint32_t tstp, uint16_t ms)
{
  uint32_t tm = ms * 10;
  if (PL_DelayElapsed(tstp, tm))
    return TRUE;
  return FALSE;
}

/*******************************************************************************
* Function Name  : DH_ResetSecTimeout
* Description    : Reset the seconds timeout
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DH_SetTimeout(uint16_t sec)
{
  dhseconds = sec;
  dhtstamp = _timestamp;
}

/*******************************************************************************
* Function Name  : DH_TimeoutElapsed
* Description    : Verify if the given seconds are elapsed
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool DH_TimeoutElapsed(void)
{ 
  if (dhseconds > 0)
  {
    if (PL_DelayElapsed(dhtstamp, 10000))
    {
      dhseconds--;
      dhtstamp = _timestamp;
      if (dhseconds == 0)
        return TRUE;
      return FALSE;
    }
    else
      return FALSE;
  }
  return FALSE;
}

/*******************************************************************************
* Function Name  : DH_Timestamp
* Description    : Return the system timestamp
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint32_t DH_Timestamp(void)
{
  return _timestamp;
}

/*******************************************************************************
* Function Name  : DH_GetSysTime
* Description    : Return the system time
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DH_GetSysTime(uint8_t *timebuffer)
{
  uint8_t buffer[PL_TIME_BUFFER_LEN];
  
  PL_RTC_GetTime(buffer);
  
  /* Get only hh, mm, ss*/
  timebuffer[0] = buffer[0]; // hh
  timebuffer[1] = buffer[1]; // mm
  timebuffer[2] = buffer[2]; // ss
}

/*******************************************************************************
* Function Name  : DH_NextDay
* Description    : Return the next day flag
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool DH_NextDay(void)
{
  return sys_nextday;
}

/*******************************************************************************
* Function Name  : DH_ClearNextDayFlag
* Description    : Clear the next day flag 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DH_ClearNextDayFlag(void)
{
  sys_nextday = FALSE;
}

/*******************************************************************************
* Function Name  : DH_SysTimeAdjusted
* Description    : Notify if the system tyme has been adjusted
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool DH_SysTimeAdjusted(void)
{
  if (SYS_TIME_ADJUSTED)
  {
    SYS_TIME_ADJUSTED = FALSE;
    return TRUE;
  }
  return FALSE;
}

/*******************************************************************************
* Function Name  : DH_SetSysTime
* Description    : Update the system time
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool DH_SetSysTime(uint8_t *timebuffer)
{
//  uint32_t acttime;
//  struct tm *pLocalTime;
  uint8_t buffer[PL_TIME_BUFFER_LEN];

//  acttime = RTC_GetCounter();
//  pLocalTime = localtime(&acttime);

  /* Set only hh, mm, ss*/
  buffer[0] = timebuffer[0]; // hh
  buffer[1] = timebuffer[1]; // mm
  buffer[2] = timebuffer[2]; // ss
//  buffer[3] = (uint8_t)(pLocalTime->tm_year >> 8);
//  buffer[4] = (uint8_t)pLocalTime->tm_year;
//  buffer[5] = pLocalTime->tm_mon;
//  buffer[6] = pLocalTime->tm_mday;
//  buffer[7] = pLocalTime->tm_wday;
//  buffer[8] = pLocalTime->tm_yday;
//  buffer[9] = pLocalTime->tm_isdst;
  
  if (PL_RTC_SetTime(buffer))
  {
    SYS_TIME_ADJUSTED = TRUE;
    return TRUE;
  }
  return FALSE;
}

/*******************************************************************************
* Function Name  : DH_InOutConfig
* Description    : Configure IO pins as inputs (0) or outputs (1)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DH_InOutConfig(uint8_t ioArray)
{
  uint8_t i;
  GPIO_InitTypeDef  GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  for (i = 0; i < 8; i++)
  {
    RCC_APB2PeriphClockCmd((RCC_APB2Periph_GPIOA << portn_arr[i]), ENABLE);
    if (ioArray & 0x01)
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    else
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    
    GPIO_InitStructure.GPIO_Pin = pin_arr[i];    
    GPIO_Init(port_arr[i], &GPIO_InitStructure);
    
    ioArray >>= 1;
  }
}

/*******************************************************************************
* Function Name  : DH_IO_Init
* Description    : Input / Output pins initialize
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DH_IO_Init(void)
{
  uint8_t i;
 
  // RCC init
  for (i = 0; i < 8; i++)
      RCC_APB2PeriphClockCmd((RCC_APB2Periph_GPIOA << portn_arr[i]), ENABLE);
  
  // Set default IN/OUT pins
  DH_InOutConfig(IO_DEFAULT_CONFIG);
}

/*******************************************************************************
* Function Name  : DH_GetInputs
* Description    : Get the inputs value
* Input          : None
* Output         : bit 0 = In0; bit 1 = In1, ...
* Return         : None
*******************************************************************************/
uint8_t DH_GetInputs(void)
{
  uint8_t retval = 0, i, in;
  uint8_t nset = 0;
  
  for (in = 0; in < 8; in++)
  {
    for (i = 0; i < 6; i++)
    {
        if (GPIO_ReadInputDataBit(port_arr[in], pin_arr[in]) == Bit_SET)
          nset++;
        PL_Delay(1); // 100us
    }
    if (nset > 3)
      retval |= (1 << in);
    nset = 0;
  }
  return retval;
}

/*******************************************************************************
* Function Name  : DH_SetOutput_N
* Description    : Set output value for the specified output pins
* Input          : bit 0 = Out0; bit 1 = Out1, ...
* Output         : None
* Return         : None
*******************************************************************************/
void DH_SetOutput_N(uint8_t out, BitAction PinVal)
{
  if (PinVal == Bit_SET)
    GPIO_WriteBit(port_arr[out], pin_arr[out], Bit_SET);
  else
    GPIO_WriteBit(port_arr[out], pin_arr[out], Bit_RESET);
}

/*******************************************************************************
* Function Name  : DH_SetOutputs
* Description    : Set output value for the configured output pins
* Input          : bit 0 = Out0; bit 1 = Out1, ...
* Output         : None
* Return         : None
*******************************************************************************/
void DH_SetOutputs(uint8_t outbuffer)
{
  uint8_t out;
  
  for (out = 0; out < 8; out++)
  {
    if (outbuffer & 0x01)
      GPIO_WriteBit(port_arr[out], pin_arr[out], Bit_SET);
    else
      GPIO_WriteBit(port_arr[out], pin_arr[out], Bit_RESET);

    outbuffer >>= 1;
  }
}

/*******************************************************************************
* Function Name  : DH_LED_Config
* Description    : Configures the different GPIO ports pins for LEDs
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DH_LED_Config(void)
{
  // LEDs configuration
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(LED_RED_RCC, ENABLE);
  RCC_APB2PeriphClockCmd(LED_GREEN_RCC, ENABLE);
  
  GPIO_InitStructure.GPIO_Pin = LED_RED_PIN_N;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LED_RED_PORT_N, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = LED_GREEN_PIN_N;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(LED_GREEN_PORT_N, &GPIO_InitStructure);

  LED_RED_ON;
  LED_GREEN_ON;
}

/*******************************************************************************
* Function Name  : DH_LED_Init
* Description    : LEDs initialization sequence
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DH_LED_Init(void)
{
  uint8_t i;
  
  for (i=0; i<2; i++)
  {
    DH_ShowLED(A_LED_DATA, A_LED_ON);
    DH_ShowLED(A_LED_ERROR, A_LED_OFF);
    DH_Delay_ms(120);
    DH_ShowLED(A_LED_DATA, A_LED_OFF);
    DH_ShowLED(A_LED_ERROR, A_LED_ON);
    DH_Delay_ms(120);
    DH_ShowLED(A_LED_BOTH, A_LED_ON);
    DH_Delay_ms(120);
  }
  DH_ShowLED(A_LED_BOTH, A_LED_OFF);
}

/*******************************************************************************
* Function Name  : DH_ShowLED 
* Description    : LEDs indication management
* Input          : LED number, action to perform 
* Output         : None
* Return         : None
*******************************************************************************/
void DH_ShowLED(DH_LedType_t nLTy, DH_LedAction_t nLAct)
{
  uint8_t i;
  
  if ((nLTy == A_LED_ERROR) || (nLTy == A_LED_BOTH))
  {
    if (nLAct == A_LED_ON)
      LED_RED_ON;
    else
      LED_RED_OFF;
  }
  
  if ((nLTy == A_LED_DATA) || (nLTy == A_LED_BOTH))
  {
    if (nLAct == A_LED_ON)
      LED_GREEN_ON;
    else
      LED_GREEN_OFF;
  }
  
  if (nLAct == A_LED_FLASH)
  {
    LED_RED_OFF;
    LED_GREEN_OFF;
    for (i=0; i<2; i++)
    {
       if ((nLTy == A_LED_DATA) || (nLTy == A_LED_BOTH))
         LED_GREEN_ON;
       if ((nLTy == A_LED_ERROR) || (nLTy == A_LED_BOTH))
         LED_RED_ON;
       DH_Delay_ms(60);
       if ((nLTy == A_LED_DATA) || (nLTy == A_LED_BOTH))
         LED_GREEN_OFF;
       if ((nLTy == A_LED_ERROR) || (nLTy == A_LED_BOTH))
         LED_RED_OFF;
       DH_Delay_ms(40);
    }
  }
}

/*******************************************************************************
* Function Name  : DH_ServiceDoneFlashLED
* Description    : LEDs indication for service frame/ack/back execution
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DH_ServiceDoneFlashLED(void)
{
  LED_RED_ON;
  LED_GREEN_ON;
  DH_Delay_ms(70);
  LED_GREEN_OFF;
  LED_RED_OFF;
}

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/

