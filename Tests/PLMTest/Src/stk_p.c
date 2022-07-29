/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : stk_p.c
* Author             : Systems LAB - IMS (Industrial & Multisegment Sector)
* Date First Issued  : 05/12/2008
* Description        : Phisical layer routines
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

#define ___USE_SYS_TICK___

/* Includes ------------------------------------------------------------------*/
#include "stk_p.h"
#include "stm32f10x_plm.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_bkp.h"
#include "stm32f10x_wwdg.h"
#include <time.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define PL_TimebaseTimerHandler       SysTick_Handler
void    _USysTickHandler(void);

#ifdef DEVICE_USE_RTC
  #define PL_RTC_Update               RTC_IRQHandler
  void    _URTC_RTC_IRQHandler(void);
#endif

#ifdef DEVICE_LSI_TIM_MEASURE
  #define PL_LSIint                   TIM5_IRQHandler
  void    _UTIM5_IRQHandler(void);
#endif


/* Private variables ---------------------------------------------------------*/
u32         PL_Counter100uS;
u16         PL_WDRefreshCounter;   /* 440 = 44 ms */
extern u16  SPI_MaxFrameCounter;
sytemdate_t sys_date;
bool        sys_nextday = FALSE;
bool        SYSTEM_RESET = FALSE;

__IO uint32_t LsiFreq = 40000;
#ifdef DEVICE_LSI_TIM_MEASURE
/* RTC Calibration variables */
__IO uint32_t TimingDelay = 0;
__IO uint16_t IC1ReadValue1 = 0, IC1ReadValue2 = 0;
__IO uint16_t CaptureNumber = 0;
__IO uint32_t Capture = 0;
#endif

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void  PL_InterruptConfig(void);
void  PL_StartTimer100uS(void);
u32   PL_DifferenceCounter(u32 counterX100us, u32 counterY100us);
u32   PL_TimeElapsed(u32 nStartTime);
void  PL_PLMConfig(void);
void  TIM5_ConfigForLSI(void);
uint32_t PL_RTC_WaitForLastTask(void);
uint32_t PL_RTC_WaitForSynchro(void);
bool  PL_InitRTC(void);
void  PL_WDInit(void);


/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : PL_StartTimer100uS
* Description    : Start the timebase timer (100us step)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PL_StartTimer100uS(void)
{
  /* SETUP e START SYS TICK TIMER */
  
  /*    The SysTick_Config() function is a CMSIS function which configure:
       - The SysTick Reload register with value passed as function parameter.
       - Configure the SysTick IRQ priority to the lowest value (0x0F).
       - Reset the SysTick Counter register.
       - Configure the SysTick Counter clock source to be Core Clock Source (HCLK).
       - Enable the SysTick Interrupt.
       - Start the SysTick Counter.
  
      The systick interrupt priority is changed after the SysTick_Config function 
      is called to the highest.
 */
  
  SysTick_Config(SystemCoreClock / SYS_TICK_CLK_DIVIDER); // 100 us
  NVIC_SetPriority(SysTick_IRQn, SYT_INT_PREEMPTY_PRIORITY); 
}

/*******************************************************************************
* Function Name  : PL_TimeElapsed
* Description    : Time elapsed from a starting time
* Input          : Starting time
* Output         : None
* Return         : Time elapsed
*******************************************************************************/
u32 PL_TimeElapsed(u32 nStartTime)
{
  return (PL_DifferenceCounter(nStartTime, PL_Counter100uS));
}

/*******************************************************************************
* Function Name  : PL_DifferenceCounter
* Description    : Calculate the difference between two counter variables
* Input          : Start time, end time
* Output         : None
* Return         : Calculated difference
*******************************************************************************/
u32 PL_DifferenceCounter(u32 counterX100us, u32 counterY100us)
{
 u32 difference;
 
 if (counterX100us > counterY100us) 
   difference = 600000 - counterX100us + counterY100us + 1;
 else	
   difference = counterY100us - counterX100us;
 
 return (difference);
}

/*******************************************************************************
* Function Name  : PL_DelayElapsed
* Description    : Verify if a specified delay is elapsed 
* Input          : Timestamp, set delay
* Output         : None
* Return         : TRUE (daly elapsed), FALSE (delay not elapsed)
*******************************************************************************/
bool PL_DelayElapsed(u32 timestamp, u32 delay)
{
  if (PL_DifferenceCounter(timestamp, PL_Counter100uS) >= delay)
    return TRUE;
 
 return FALSE;
}

/*******************************************************************************
* Function Name  : PL_TimebaseTimerHandler
* Description    : Handles the timebase counter 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PL_TimebaseTimerHandler(void)
{
  PL_WDUpdate();
  PL_Counter100uS++;  
  if (PL_Counter100uS > 600000)
    PL_Counter100uS = 0;
  _USysTickHandler();
}

/*******************************************************************************
* Function Name  : PL_InterruptConfig
* Description    : Configure interrupts and NVIC
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PL_InterruptConfig(void)
{
//  /* Set the Vector Table base address at 0x08000000, offset 0x0000 */
//  NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x0000);
  /* Configure the Priority Group */
  NVIC_PriorityGroupConfig(INTERFACE_PRIORITY_GROUP);
}

/*******************************************************************************
* Function Name  : PL_Init
* Description    : Initialize the phisical layer
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool PL_Init(void)
{
  PL_InterruptConfig();   // Initialize interrupts with the given priority
  PL_StartTimer100uS();   // Start timebase timer (100 usec)
  PL_Delay(500);          // Startup delay
  PL_PLMConfig();         // Configure the power line modem
#ifdef DEVICE_USE_WATCHDOG
  PL_WDInit();            // Watchdog timer initialization
#endif
#ifdef DEVICE_USE_RTC
  if (!PL_InitRTC())      // Initialize the internal rtc
    return FALSE; 
#endif
  return TRUE;
}

/*******************************************************************************
* Function Name  : PL_PLMConfig 
* Description    : Configure the PLM 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PL_PLMConfig(void)
{
  PLM_IntPriorityInitTypeDef PLM_InterruptInitStructure;
  PLM_InitTypeDef PLM_InitStructure;

  /* Configure the PLM device */
  PLM_InterruptInitStructure.PLM_SPI_PreemptyPriority = PLM_SPI_INT_PREEMPTY_PRIORITY;
  PLM_InterruptInitStructure.PLM_SPI_SubPriority = PLM_SPI_INT_SUB_PRIORITY;
  PLM_InterruptInitStructure.PLM_CDPD_PreemptyPriority = PLM_CDPD_PREEMPTY_PRIORITY;
  PLM_InterruptInitStructure.PLM_CDPD_SubPriority = PLM_CDPD_SUB_PRIORITY;
  PLM_PeripheralInit(&PLM_InterruptInitStructure);
  
  /* Configure the PLM parameters */
  PLM_InitStructure.PLM_HeaderPreamble = HEADER_PREAMBLE_VALUE;
  PLM_InitStructure.PLM_BandpassFrequency = PLM_PB_FREQ_132KHz; // default
  PLM_InitStructure.PLM_BaudRate = PLM_BAUDRATE_2400; // default
  PLM_InitStructure.PLM_Deviation = PLM_DEVIATION_05;
  PLM_InitStructure.PLM_PreFilter = PLM_PRE_FILTER_ON;
#ifdef PLM_SENS_HIGH
  PLM_InitStructure.PLM_Sensitivity = PLM_SENSITIVITY_HIGH;
#else
  PLM_InitStructure.PLM_Sensitivity = PLM_SENSITIVITY_NORMAL;
#endif
#ifdef PLM_FREQ_60KHZ
  PLM_InitStructure.PLM_BandpassFrequency = PLM_PB_FREQ_60KHz;
#elif defined(PLM_FREQ_66KHZ)
  PLM_InitStructure.PLM_BandpassFrequency = PLM_PB_FREQ_66KHz;
#elif defined(PLM_FREQ_72KHZ)
  PLM_InitStructure.PLM_BandpassFrequency = PLM_PB_FREQ_72KHz;
#elif defined(PLM_FREQ_76KHZ)
  PLM_InitStructure.PLM_BandpassFrequency = PLM_PB_FREQ_76KHz;
#elif defined(PLM_FREQ_82KHZ)
  PLM_InitStructure.PLM_BandpassFrequency = PLM_PB_FREQ_82KHz;
#elif defined(PLM_FREQ_86KHZ)
  PLM_InitStructure.PLM_BandpassFrequency = PLM_PB_FREQ_86KHz;
#elif defined(PLM_FREQ_110KHZ)
  PLM_InitStructure.PLM_BandpassFrequency = PLM_PB_FREQ_110KHz;
#else // PLM_FREQ_132KHZ as default value
  PLM_InitStructure.PLM_BandpassFrequency = PLM_PB_FREQ_132KHz;
#endif

#ifdef PLM_600_BPS
  PLM_InitStructure.PLM_BaudRate = PLM_BAUDRATE_600;
#elif defined(PLM_1200_BPS)
  PLM_InitStructure.PLM_BaudRate = PLM_BAUDRATE_1200;
#elif defined(PLM_2400_BPS)
  PLM_InitStructure.PLM_BaudRate = PLM_BAUDRATE_2400;
#else // PLM_4800_BPS as default value
  PLM_InitStructure.PLM_BaudRate = PLM_BAUDRATE_4800;
#endif  
  
  PLM_Init(&PLM_InitStructure);
  
}

#ifdef DEVICE_LSI_TIM_MEASURE
/*******************************************************************************
* Function Name  : TIM5_ConfigForLSI
* Description    : Calibration procedure for LSI
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void TIM5_ConfigForLSI(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  TIM_ICInitTypeDef  TIM_ICInitStructure;

  /* Enable TIM5 clocks */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
  
  /* Enable the TIM5 Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Configure TIM5 prescaler */
  TIM_PrescalerConfig(TIM5, 0, TIM_PSCReloadMode_Immediate);

  /* Connect internally the TM5_CH4 Input Capture to the LSI clock output */
  GPIO_PinRemapConfig(GPIO_Remap_TIM5CH4_LSI, ENABLE);
  
  /* TIM5 configuration: Input Capture mode ---------------------
     The LSI oscillator is connected to TIM5 CH4
     The Rising edge is used as active edge,
     The TIM5 CCR4 is used to compute the frequency value 
  ------------------------------------------------------------ */
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
  TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
  TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
  TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV8;
  TIM_ICInitStructure.TIM_ICFilter = 0;
  TIM_ICInit(TIM5, &TIM_ICInitStructure);
  
  /* TIM10 Counter Enable */
  TIM_Cmd(TIM5, ENABLE);

  /* Reset the flags */
  TIM5->SR = 0;
    
  /* Enable the CC4 Interrupt Request */  
  TIM_ITConfig(TIM5, TIM_IT_CC4, ENABLE);  
}

/*******************************************************************************
* Function Name  : PL_WDInit
* Description    : PLM initialize the watchdog timer
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PL_LSIint(void)
{
  if (TIM_GetITStatus(TIM5, TIM_IT_CC4) != RESET)
  {    
    if(CaptureNumber == 0)
    {
      /* Get the Input Capture value */
      IC1ReadValue1 = TIM_GetCapture4(TIM5);
    }
    else if(CaptureNumber == 1)
    {
      /* Get the Input Capture value */
      IC1ReadValue2 = TIM_GetCapture4(TIM5); 
      
      /* Capture computation */
      if (IC1ReadValue2 > IC1ReadValue1)
      {
        Capture = (IC1ReadValue2 - IC1ReadValue1); 
      }
      else
      {
        Capture = ((0xFFFF - IC1ReadValue1) + IC1ReadValue2); 
      }
      /* Frequency computation */ 
      LsiFreq = (uint32_t) SystemCoreClock / Capture;
      LsiFreq *= 8;
    }
    
    CaptureNumber++;
    
    /* Clear TIM5 Capture compare interrupt pending bit */
    TIM_ClearITPendingBit(TIM5, TIM_IT_CC4);
  }
}
#endif

/*******************************************************************************
* Function Name  : PL_WDInit
* Description    : PLM initialize the watchdog timer
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PL_WDInit(void)
{
#ifdef DEVICE_LSI_TIM_MEASURE  
  u32 del = 600000;
  
  RCC_LSICmd(ENABLE);
  /* Wait till LSI is ready */
  while ((RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET)&&(del--));
  TIM5_ConfigForLSI();
  del = 600000;
  /* Wait until the TIM5 get 2 LSI edges */
  while((CaptureNumber != 2)&&(del--));
  /* Disable TIM5 CC4 Interrupt Request */
  TIM_ITConfig(TIM5, TIM_IT_CC4, DISABLE);
#endif
  
  /* IWDG timeout equal to 250 ms (the timeout may varies due to LSI frequency dispersion) */
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  
  /* IWDG counter clock: LSI/32 */
  IWDG_SetPrescaler(IWDG_Prescaler_32);

  /* 
    Set counter reload value to obtain 250ms IWDG TimeOut.
     Counter Reload Value = 250ms/IWDG counter clock period
                          = 250ms / (LSI/32)
                          = 0.25s / (LsiFreq/32)
                          = LsiFreq/(32 * 4)
                          = LsiFreq/128
   */
  IWDG_SetReload(LsiFreq/128);
  /* Reload IWDG counter */
  IWDG_ReloadCounter();
  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();
}

/*******************************************************************************
* Function Name  : PL_WDInit
* Description    : PLM initialize the watchdog timer
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PL_WDUpdate(void)
{
#ifdef DEVICE_USE_WATCHDOG
  if (!SYSTEM_RESET)
    IWDG_ReloadCounter();
#endif
}

/*******************************************************************************
* Function Name  : PL_GetRND 
* Description    : Get an 8 bit value from ADC channel connected to a floating pin
* Input          : None
* Output         : None
* Return         : Value read
*******************************************************************************/
u16 PL_GetRND(u32 param)
{ 
  u16 nRNDVal;
  u8 i, *buffer;

  buffer = PLM_GetReceivedBufferPointer();
  nRNDVal = (u16)(param * PL_Counter100uS);
  for (i=0; i<100; i++)
    nRNDVal += (*buffer);

  return (nRNDVal+1);
}

/*******************************************************************************
* Function Name  : PL_Delay
* Description    : Static Delay
* Input          : Request delay (1 unit = 100us)
* Output         : None
* Return         : None
*******************************************************************************/
void PL_Delay (u32 n100u)
{
  u32 tTime;
  
  tTime = _timestamp;
  while(PL_DelayElapsed(tTime, n100u) == FALSE);
}

/*******************************************************************************
* Function Name  : PL_RTC_GetTime
* Description    : Adjust the RTC time value
* Input          : none
* Output         : None
* Return         : None
*******************************************************************************/
void PL_RTC_GetTime(u8 *timebuffer)
{
  timebuffer[0] = sys_date.hh;
  timebuffer[1] = sys_date.mm;
  timebuffer[2] = sys_date.ss;
  timebuffer[3] = (u8)(sys_date.year >> 8);
  timebuffer[4] = (u8)(sys_date.year);
  timebuffer[5] = sys_date.month;
  timebuffer[6] = sys_date.day;
}

/**
  * @brief  Waits until last write operation on RTC registers has finished.
  * @note   This function must be called before any write to RTC registers.
  * @param  None
  * @retval None
  */
uint32_t PL_RTC_WaitForLastTask(void)
{
  uint32_t del = 1000000;
  
  /* Loop until RTOFF flag is set */
  while (((RTC->CRL & RTC_FLAG_RTOFF) == (uint16_t)RESET) && del)
  {
    del--;
  }
  return del;
}

/**
  * @brief  Waits until the RTC registers (RTC_CNT, RTC_ALR and RTC_PRL)
  *   are synchronized with RTC APB clock.
  * @note   This function must be called before any read operation after an APB reset
  *   or an APB clock stop.
  * @param  None
  * @retval None
  */
uint32_t PL_RTC_WaitForSynchro(void)
{
  uint32_t del = 1000000;
  
  /* Clear RSF flag */
  RTC->CRL &= (uint16_t)~RTC_FLAG_RSF;
  /* Loop until RSF flag is set */
  while (((RTC->CRL & RTC_FLAG_RSF) == (uint16_t)RESET) && del)
  {
    del--;
  }
  return del;
}

/*******************************************************************************
* Function Name  : PL_RTC_SetTime
* Description    : Adjust the RTC time value
* Input          : none
* Output         : None
* Return         : None
*******************************************************************************/
bool PL_RTC_SetTime(u8 *timebuffer)
{
  u32 acttime;
  struct tm *pLocalTime;
  
  acttime = RTC_GetCounter();
  pLocalTime = localtime(&acttime);
    
  pLocalTime->tm_hour = (int)(timebuffer[0]);     /* hours since midnight (0,23)      */
  pLocalTime->tm_min = (int)(timebuffer[1]);      /* minutes after the hour (0,59)    */
  pLocalTime->tm_sec = (int)(timebuffer[2]);      /* seconds after the minute (0,61)  */
//  pLocalTime.tm_year = (int)((((int)(timebuffer[3]) << 8) | ((int)(timebuffer[4])))-1900); /* years since 1900 */
//  pLocalTime.tm_mon = (int)(timebuffer[5] - 1);  /* months since January (0,11)      */ 
//  pLocalTime.tm_mday = (int)(timebuffer[6]);     /* day of the month (1,31)          */
//  pLocalTime.tm_wday  = (int)(timebuffer[7]);    /* days since Sunday (0,6)          */
//  pLocalTime.tm_yday  = (int)(timebuffer[8]);    /* days since January 1 (0,365)     */
//  pLocalTime.tm_isdst = (int)(timebuffer[9]);    /* Daylight Saving Time flag        */
  
  /* Update RTC counter */
  RTC_SetCounter(mktime(pLocalTime));
  
  /* Wait until last write operation on RTC registers has finished */
  if (PL_RTC_WaitForLastTask() == 0)
      return FALSE;
  
  /* Change the current time */
  sys_date.hh = timebuffer[0];
  sys_date.mm = timebuffer[1];
  sys_date.ss = timebuffer[2];
//  sys_date.year = (u16)(((u16)(timebuffer[3]) << 8) | ((u16)(timebuffer[4])));
//  sys_date.month = timebuffer[5];
//  sys_date.day = timebuffer[6];
//  sys_date.weekday = timebuffer[7];
//  sys_date.yearday = timebuffer[8];
//  sys_date.isdst = timebuffer[9];
  
  return TRUE;
}

#ifdef DEVICE_USE_RTC
/*******************************************************************************
* Function Name  : PL_RTC_LimitCheck
* Description    : Verify RTC registers limits
* Input          : none
* Output         : None
* Return         : None
*******************************************************************************/
void PL_RTC_Update(void)
{
  u32 acttime;
  struct tm* pLocalTime;
  
  if (RTC_GetITStatus(RTC_IT_SEC) != RESET)
  {
    acttime = RTC_GetCounter();
    pLocalTime = localtime(&acttime);
    
    sys_date.hh     = (u8)pLocalTime->tm_hour;            /* hours since midnight (0,23)      */
    sys_date.mm     = (u8)pLocalTime->tm_min;             /* minutes after the hour (0,59)    */
    sys_date.ss     = (u8)pLocalTime->tm_sec;             /* seconds after the minute (0,61)  */
    sys_date.year   = (u16)pLocalTime->tm_year + 1900;    /* years since 1900                 */
    sys_date.month  = (u8)pLocalTime->tm_mon + 1;         /* months since January (0,11)      */
    sys_date.day    = (u8)pLocalTime->tm_mday;            /* day of the month (1,31)          */
    
    if ((sys_date.hh == 0)&&(sys_date.mm == 0)&&(sys_date.ss == 0))
      sys_nextday = TRUE;
    
    /* Clear the RTC Second interrupt */
    RTC_ClearITPendingBit(RTC_IT_SEC);   
  }
}
#endif

/*******************************************************************************
* Function Name  : PL_InitRTC
* Description    : Configure the internal RTC
* Input          : none
* Output         : None
* Return         : None
*******************************************************************************/
bool PL_InitRTC(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  struct tm tLocalTime;
  
  /* Enable the RTC Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = RTC_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = RTC_PREEMPTY_PRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = RTC_SUB_PRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* Enable PWR and BKP clocks */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
  /* Allow access to BKP Domain */
  PWR_BackupAccessCmd(ENABLE);
  BKP_ClearFlag();

#ifdef DEVICE_USE_EXTOSC_32KHZ
  if (BKP_ReadBackupRegister(BKP_DR1) == 0xA5A5)
  {
    /* Check if the Power On Reset flag is set */
    if ((RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)||(RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET))
    {
      // Reset or power off occurred
    }

    /* Wait for RTC registers synchronization */
    if (PL_RTC_WaitForSynchro() != 0)
    {
      /* Enable the RTC Second */
      RTC_ITConfig(RTC_IT_SEC, ENABLE);
      /* Wait until last write operation on RTC registers has finished */
      if (PL_RTC_WaitForLastTask() != 0)
      {
        /* Clear reset flags */
        RCC_ClearFlag();
        return TRUE;
      }
    }
  }
#endif
  /* Backup data register value is not correct or not yet programmed (when
     the first time the program is executed) */

  /* Date-Time Initialization: 16 December 2011 15:20:00 */  
  tLocalTime.tm_hour  = 15;   /* hours since midnight (0,23)      */
  tLocalTime.tm_min   = 20;   /* minutes after the hour (0,59)    */
  tLocalTime.tm_sec   = 0;    /* seconds after the minute (0,61)  */
  tLocalTime.tm_mday  = 16;   /* day of the month (1,31)          */
  tLocalTime.tm_mon   = 11;   /* months since January (0,11)      */ 
  tLocalTime.tm_year  = 111;  /* years since 1900                 */
  tLocalTime.tm_wday  = 2;    /* days since Sunday (0,6)          */
  tLocalTime.tm_yday  = 346;  /* days since January 1 (0,365)     */
  tLocalTime.tm_isdst = 0;    /* Daylight Saving Time flag        */
  
  /* RTC Configuration */
  if (!PL_ConfigureRTC(mktime(&tLocalTime)))
    return FALSE;
  
  /* Clear reset flags */
  RCC_ClearFlag();
  return TRUE;
}
  
/*******************************************************************************
* Function Name  : PL_ConfigureRTC
* Description    : Configure the RTC if the backup registers are not initialized
* Input          : Request delay (1 unit = 100us)
* Output         : None
* Return         : None
*******************************************************************************/
bool PL_ConfigureRTC(u32 countval)
{
  u32 nRTCdel;
  
  /* Enable PWR and BKP clocks */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
  /* Allow access to BKP Domain */
  PWR_BackupAccessCmd(ENABLE);
  /* Reset Backup Domain */
  BKP_DeInit();

  nRTCdel = 1000000;
#ifdef  DEVICE_USE_EXTOSC_32KHZ
  /* Enable the LSE OSC */
  RCC_LSEConfig(RCC_LSE_ON);
  /* Wait till LSE is ready */
  while ((RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) && nRTCdel)
    nRTCdel--;
#else
  /* Enable the LSI OSC */
  RCC_LSICmd(ENABLE);
  /* Wait till LSI is ready */
  while ((RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET) && nRTCdel)
    nRTCdel--;
#endif
  
  if (nRTCdel)
  {
#ifdef  DEVICE_USE_EXTOSC_32KHZ
    /* Select LSE as RTC Clock Source */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
#else
    /* Select LSI as RTC Clock Source */
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
#endif
    /* Enable RTC Clock */
    RCC_RTCCLKCmd(ENABLE);
    /* Wait for RTC registers synchronization */
    if (PL_RTC_WaitForSynchro() == 0)
      return FALSE;
    /* Wait until last write operation on RTC registers has finished */
    if (PL_RTC_WaitForLastTask() == 0)
      return FALSE;
    /* Enable the RTC Second */
    RTC_ITConfig(RTC_IT_SEC, ENABLE);
    /* Wait until last write operation on RTC registers has finished */
    if (PL_RTC_WaitForLastTask() == 0)
      return FALSE;
    /* Set RTC prescaler: set RTC period to 1sec */
#ifdef  DEVICE_USE_EXTOSC_32KHZ
    RTC_SetPrescaler(32767); /* RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1) */
#else
    RTC_SetPrescaler(40000);
#endif
    /* Wait until last write operation on RTC registers has finished */
    if (PL_RTC_WaitForLastTask() == 0)
      return FALSE;
    /* Set time registers to 00:00:00; configuration done via gui */
    RTC_SetCounter(countval);
    /* Wait until last write operation on RTC registers has finished */
    if (PL_RTC_WaitForLastTask() == 0)
      return FALSE;
    /* Write RTC flag in backup register */
    BKP_WriteBackupRegister(BKP_DR1, 0xA5A5);
    return TRUE;
  }
  return FALSE;
}


/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/

