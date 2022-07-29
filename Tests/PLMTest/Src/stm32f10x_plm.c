/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : stm32f10x_plm.c
* Author             : Systems LAB - IMS (Industrial & Multisegment Sector)
* Date First Issued  : 14/06/2012
* Description        : PLM peripheral routines
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

#define ___USE_PLM_REDEF___

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_plm.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  uint8_t     PLM_F_len;
  uint8_t     PLM_F_Xmode;
  plm_flag_t  PLM_F_TXOK;
  plm_flag_t  PLM_F_RXOK;
  plm_flag_t  PLM_F_CommNotAllowed;
}PLM_NTWFlag_t;

/* Private define ------------------------------------------------------------*/
#define PLM_REG     0
#define PLM_DATA    1
#define PLM_LEN     2
#define PLM_BODY    3

#define HEADER_FRAME          0xC1
#define HEADER_WAKEUP_VALUE   0xAA
#define PL_HDR_SIZE           8       // Physical layer header size
#define DATAGRAM_MAX_LEN      (((PAYLOAD_SIZE + PLM_RESERVED_BYTES) * 2) + PL_HDR_SIZE)  // Max datagram value (248 bytes with user data len = 100 bytes)

/* HARDWARE INTERFACE DEFINITIONS */
void  _SPI_UIRQHandler(void);
void  _EXTI_UIRQHandler(void);

#if (PLM_SPI == 1)
  #define PLM_RXTX_Interrupt        SPI1_IRQHandler
#elif (PLM_SPI == 2)
  #define PLM_RXTX_Interrupt        SPI2_IRQHandler
#else
  #define PLM_RXTX_Interrupt        SPI3_IRQHandler
#endif

#define PLM_SCK_PIN_N     ((uint16_t)(GPIO_Pin_0 << PLM_SCK_PIN))
#define PLM_MISO_PIN_N    ((uint16_t)(GPIO_Pin_0 << PLM_MISO_PIN))
#define PLM_MOSI_PIN_N    ((uint16_t)(GPIO_Pin_0 << PLM_MOSI_PIN))
#define PLM_CDPD_PIN_N    ((uint16_t)(GPIO_Pin_0 << PLM_CDPD_PIN))
#define PLM_BU_PIN_N      ((uint16_t)(GPIO_Pin_0 << PLM_BU_PIN))
#define PLM_REGDATA_PIN_N ((uint16_t)(GPIO_Pin_0 << PLM_REGDATA_PIN))
#define PLM_RXTX_PIN_N    ((uint16_t)(GPIO_Pin_0 << PLM_RXTX_PIN))

#if (PLM_CDPD_PIN == 0)
  #define PLM_CDPD_Interrupt  EXTI0_IRQHandler
#elif (PLM_CDPD_PIN == 1)
  #define PLM_CDPD_Interrupt  EXTI1_IRQHandler
#elif (PLM_CDPD_PIN == 2)
  #define PLM_CDPD_Interrupt  EXTI2_IRQHandler
#elif (PLM_CDPD_PIN == 3)
  #define PLM_CDPD_Interrupt  EXTI3_IRQHandler
#elif (PLM_CDPD_PIN == 4)
  #define PLM_CDPD_Interrupt  EXTI4_IRQHandler
#elif (PLM_CDPD_PIN > 4) && (PLM_CDPD_PIN < 10)
  #define PLM_CDPD_Interrupt  EXTI9_5_IRQHandler
#else
  #define PLM_CDPD_Interrupt  EXTI15_10_IRQHandler
#endif

// SPI port
#define PLM_SPI_PORT_N      ((GPIO_TypeDef *) (GPIOA_BASE + ((GPIOB_BASE - GPIOA_BASE) * PLM_SPI_PORT)))
// CDPD port
#define PLM_CDPD_PORT_N     ((GPIO_TypeDef *) (GPIOA_BASE + ((GPIOB_BASE - GPIOA_BASE) * PLM_CDPD_PORT)))
// BU port
#define PLM_BU_PORT_N       ((GPIO_TypeDef *) (GPIOA_BASE + ((GPIOB_BASE - GPIOA_BASE) * PLM_BU_PORT)))
// REGDATA port
#define PLM_REGDATA_PORT_N  ((GPIO_TypeDef *) (GPIOA_BASE + ((GPIOB_BASE - GPIOA_BASE) * PLM_REGDATA_PORT)))
// RXTX port
#define PLM_RXTX_PORT_N     ((GPIO_TypeDef *) (GPIOA_BASE + ((GPIOB_BASE - GPIOA_BASE) * PLM_RXTX_PORT)))

/* Private variables ---------------------------------------------------------*/
SPI_TypeDef *   PLM_SPI_PERIPHERAL;
uint32_t        PLM_RCC_APBPeriph_SPI;

uint8_t         SPI_Buffer[DATAGRAM_MAX_LEN];
uint8_t         SPI_COMM_index;
uint32_t        SPI_MaxClockTime;
SPI_Mode_t      SPI_COMM_Mode;
PLM_NTWFlag_t   PLM_Flag;
uint16_t        SPI_MaxFrameCounter;
uint8_t         PLM_FEC_Corrections;
PLM_WP_TypeDef  PLM_WrongPostamble;
plmframe_t      PLMframe;
uint16_t        PLM_HeaderPreambleValue = HEADER_PREAMBLE_VALUE;

unsigned char LLucCRC[] = 
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x01, 0x00, 0x02, 0x00, 0x18, 0x00, 0x40,
  0x00, 0xc0, 0x03, 0x00, 0x00, 0x70, 0x04, 0x28,
  0x00, 0x01, 0x30, 0x1c, 0x00, 0x06, 0x80, 0x80,
  0x00, 0x10, 0x80, 0x80, 0x07, 0xa0, 0x00, 0x0a,
  0x00, 0x01, 0xe0, 0x02, 0x08, 0xc0, 0x50, 0x05,
  0x00, 0x0e, 0x03, 0x40, 0x60, 0x00, 0x38, 0x14,
  0x00, 0x01, 0x0c, 0x20, 0x00, 0x00, 0x00, 0x00
};

unsigned char tableFEC[] =
{
  0x3f, 0x06, 0x34, 0x0d, 0x29, 0x10, 0x22, 0x1b, 0x13, 0x2a, 0x18, 0x21, 0x05, 0x3c, 0x0e, 0x37, 
  0x1e, 0x27, 0x15, 0x2c, 0x08, 0x31, 0x03, 0x3a, 0x32, 0x0b, 0x39, 0x00, 0x24, 0x1d, 0x2f, 0x16, 
  0x04, 0x3d, 0x0f, 0x36, 0x12, 0x2b, 0x19, 0x20, 0x28, 0x11, 0x23, 0x1a, 0x3e, 0x07, 0x35, 0x0c, 
  0x25, 0x1c, 0x2e, 0x17, 0x33, 0x0a, 0x38, 0x01, 0x09, 0x30, 0x02, 0x3b, 0x1f, 0x26, 0x14, 0x2d, 
  0x30, 0x09, 0x3b, 0x02, 0x26, 0x1f, 0x2d, 0x14, 0x1c, 0x25, 0x17, 0x2e, 0x0a, 0x33, 0x01, 0x38, 
  0x11, 0x28, 0x1a, 0x23, 0x07, 0x3e, 0x0c, 0x35, 0x3d, 0x04, 0x36, 0x0f, 0x2b, 0x12, 0x20, 0x19, 
  0x0b, 0x32, 0x00, 0x39, 0x1d, 0x24, 0x16, 0x2f, 0x27, 0x1e, 0x2c, 0x15, 0x31, 0x08, 0x3a, 0x03, 
  0x2a, 0x13, 0x21, 0x18, 0x3c, 0x05, 0x37, 0x0e, 0x06, 0x3f, 0x0d, 0x34, 0x10, 0x29, 0x1b, 0x22, 
  0x21, 0x18, 0x2a, 0x13, 0x37, 0x0e, 0x3c, 0x05, 0x0d, 0x34, 0x06, 0x3f, 0x1b, 0x22, 0x10, 0x29,
  0x00, 0x39, 0x0b, 0x32, 0x16, 0x2f, 0x1d, 0x24, 0x2c, 0x15, 0x27, 0x1e, 0x3a, 0x03, 0x31, 0x08, 
  0x1a, 0x23, 0x11, 0x28, 0x0c, 0x35, 0x07, 0x3e, 0x36, 0x0f, 0x3d, 0x04, 0x20, 0x19, 0x2b, 0x12, 
  0x3b, 0x02, 0x30, 0x09, 0x2d, 0x14, 0x26, 0x1f, 0x17, 0x2e, 0x1c, 0x25, 0x01, 0x38, 0x0a, 0x33, 
  0x2e, 0x17, 0x25, 0x1c, 0x38, 0x01, 0x33, 0x0a, 0x02, 0x3b, 0x09, 0x30, 0x14, 0x2d, 0x1f, 0x26, 
  0x0f, 0x36, 0x04, 0x3d, 0x19, 0x20, 0x12, 0x2b, 0x23, 0x1a, 0x28, 0x11, 0x35, 0x0c, 0x3e, 0x07, 
  0x15, 0x2c, 0x1e, 0x27, 0x03, 0x3a, 0x08, 0x31, 0x39, 0x00, 0x32, 0x0b, 0x2f, 0x16, 0x24, 0x1d, 
  0x34, 0x0d, 0x3f, 0x06, 0x22, 0x1b, 0x29, 0x10, 0x18, 0x21, 0x13, 0x2a, 0x0e, 0x37, 0x05, 0x3c
};

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void        PLM_InitPLMVars(void);

void        PLM_SPI_Idle(void);
void        PLM_TXSelect(void);
void        PLM_RXSelect(void);
void        PLM_REGSelect(void);
void        PLM_DATASelect(void);
void        PLM_WaitSPICLKLow(void);

plm_flag_t  PLM_CheckPostamble(uint8_t len);
uint8_t     PLM_CheckFEC(void);
plm_flag_t  PLM_CorrectFEC(uint8_t *byteRx, uint8_t *ucFecRx);


/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : PLM_InitPLMVars
* Description    : Initialize global PLM vars
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PLM_InitPLMVars(void)
{
}

/*******************************************************************************
* Function Name  : PLM_PeripheralInit
* Description    : Associate the micro peripherals and pins to the ST7540
* Input          : PLM peripheral initializing structure
* Output         : None
* Return         : None
*******************************************************************************/
void PLM_PeripheralInit(PLM_IntPriorityInitTypeDef *PLM_PeripInitStruct)
{
  SPI_InitTypeDef   SPI_InitStructure;
  GPIO_InitTypeDef  GPIO_InitStructure;
  NVIC_InitTypeDef  NVIC_InitStructure;
  EXTI_InitTypeDef  EXTI_InitStructure;
  
  PLM_InitPLMVars();  
  
  /* PLM peripheral device initialization */

  /* Enable PLM SPI clocks */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | (RCC_APB2Periph_GPIOA << PLM_SPI_PORT), ENABLE);
  
  if (PLM_SPI == 1)
  {
    PLM_SPI_PERIPHERAL = SPI1;
    PLM_RCC_APBPeriph_SPI = RCC_APB2Periph_SPI1;
    RCC_APB2PeriphClockCmd(PLM_RCC_APBPeriph_SPI, ENABLE);
  }
  else if (PLM_SPI == 2)
  {
    PLM_SPI_PERIPHERAL = SPI2;
    PLM_RCC_APBPeriph_SPI = RCC_APB1Periph_SPI2;
    RCC_APB1PeriphClockCmd(PLM_RCC_APBPeriph_SPI, ENABLE);
  }
  else
  {
    PLM_SPI_PERIPHERAL = SPI3;
    PLM_RCC_APBPeriph_SPI = RCC_APB1Periph_SPI3;
    RCC_APB1PeriphClockCmd(PLM_RCC_APBPeriph_SPI, ENABLE);
    
  }
  
  if (PLM_SPI_PERIPHERAL == SPI1)
    NVIC_InitStructure.NVIC_IRQChannel = SPI1_IRQn;
  else if (PLM_SPI_PERIPHERAL == SPI2)
    NVIC_InitStructure.NVIC_IRQChannel = SPI2_IRQn;
  else
  {
#if defined (STM32F10X_HD) || (defined STM32F10X_HD_VL) || (defined STM32F10X_XL) || (defined STM32F10X_CL)
      NVIC_InitStructure.NVIC_IRQChannel = SPI3_IRQn;
#else
      // Could be an hardware fault if attempt to use SPI3 in a device which has no SPI3
      NVIC_InitStructure.NVIC_IRQChannel = 51;
#endif
  }
  
  /* Configure PLM SPI pins: SCK, MISO and MOSI */
  GPIO_InitStructure.GPIO_Pin = PLM_SCK_PIN_N | PLM_MISO_PIN_N | PLM_MOSI_PIN_N;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(PLM_SPI_PORT_N, &GPIO_InitStructure);

  /* Configure PLM functional pins */
  
  /* PLM CDPD_BIT, PLM BU_BIT */
  GPIO_InitStructure.GPIO_Pin = PLM_CDPD_PIN_N;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(PLM_CDPD_PORT_N, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = PLM_BU_PIN_N;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init(PLM_BU_PORT_N, &GPIO_InitStructure);
  
  /* PLM REGDATA_BIT */
  GPIO_InitStructure.GPIO_Pin = PLM_REGDATA_PIN_N;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(PLM_REGDATA_PORT_N, &GPIO_InitStructure);
  
  /* PLM RXTX_BIT*/
  GPIO_InitStructure.GPIO_Pin = PLM_RXTX_PIN_N;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(PLM_RXTX_PORT_N, &GPIO_InitStructure);
  
  /* Configure and enable PLM SPI interrupt ----------------------------------*/
  // NVIC_InitStructure.NVIC_IRQChannel = PLM_SPI_IRQ; 
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PLM_PeripInitStruct->PLM_SPI_PreemptyPriority;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = PLM_PeripInitStruct->PLM_SPI_SubPriority;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* PLM SPI configuration */ 
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(PLM_SPI_PERIPHERAL, &SPI_InitStructure);
  
  /* Enable PLM SPI RXNE interrupt */
  SPI_I2S_ITConfig(PLM_SPI_PERIPHERAL, SPI_I2S_IT_RXNE, ENABLE);
  
  PLM_SPI_Idle();

  /* Enable the EXTI Interrupt (CD_PD) <!> GPIO_Pin_0...15 is considered equal to EXTI_Line0...15 */
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Line = (uint32_t)(EXTI_Line0 << PLM_CDPD_PIN);
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* Configure the EXTI line handler priority */
  if (PLM_CDPD_PIN < 5)
    NVIC_InitStructure.NVIC_IRQChannel = (EXTI0_IRQn + PLM_CDPD_PIN);
  else if (PLM_CDPD_PIN < 10)
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
  else
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
  
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = PLM_PeripInitStruct->PLM_CDPD_PreemptyPriority;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = PLM_PeripInitStruct->PLM_CDPD_SubPriority;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* CDPD Interrupt definition */
  GPIO_EXTILineConfig(PLM_CDPD_PORT, (uint8_t)(PLM_CDPD_PIN));
}

/*******************************************************************************
* Function Name  : PLM_Init
* Description    : Initialize the PLM module
* Input          : PLM initializing structure
* Output         : None
* Return         : None
*******************************************************************************/
void PLM_Init(PLM_InitTypeDef *PLM_InitStruct)
{
  uint8_t   PLM_reg[6];
  uint8_t   i, nAtt;
  uint32_t  nTout;
  
  /* Parameters verification */
  assert_param(IS_PLM_FREQUENCY(PLM_InitStruct->PLM_BandpassFrequency));
  assert_param(IS_PLM_BAUDRATE(PLM_InitStruct->PLM_BaudRate));
  assert_param(IS_PLM_DEVIATION(PLM_InitStruct->PLM_Deviation));
  assert_param(IS_PLM_PREFILTER(PLM_InitStruct->PLM_PreFilter));
  
  /*--------------------------------------------------------------
    Registers configuration for ST7540
    --------------------------------------------------------------
  
    +-------+--------+-------+
    | 0x93  |  0x90  |  0x17 |
    +-------+--------+-------+
    11110111 10010100 00010111 -> 0xF7 0x94 0x17
    |||||||| |||||||| ||||||||
    |||||||| |||||||| |||||+++- Frequency   : 132.5 KHz (default)
    |||||||| |||||||| |||++---- Baud Rate   : 2400 bps (default)
    |||||||| |||||||| ||+------ Deviation   : 0.5 (default)
    |||||||| |||||||| |+------- WatchDog    : Disabled
    |||||||| |||||||+ +-------- Tx Timeout  : Disabled
    |||||||| |||||++- --------- Freq D.T.   : 500 usec
    |||||||| ||||+--- --------- Reserved    : 0
    |||||||| ||++---- --------- Preamble    : With Conditioning
    |||||||| |+------ --------- Mains I.M.  : Synchronous
    |||||||+ +------- --------- Output Clock: Off
    ||||||+- -------- --------- Output V.L. : Off
    |||||+-- -------- --------- Header Rec. : Disabled
    ||||+--- -------- --------- Frame Len C.: Disabled
    |||+---- -------- --------- Header Len  : 16 bit
    ||+----- -------- --------- Extended Rgs: Enabled
    |+------ -------- --------- Sensitivity : Normal 
    +------- -------- --------- Input Filter: Enabled
  */
  
  /* Assign HEADER */
  PLM_HeaderPreambleValue = PLM_InitStruct->PLM_HeaderPreamble;
    
  /* Using Extended REGs */
  PLM_reg[0] = 0x01;
  PLM_reg[1] = (uint8_t)(PLM_InitStruct->PLM_HeaderPreamble >> 8);
  PLM_reg[2] = (uint8_t)(PLM_InitStruct->PLM_HeaderPreamble & 0x0FF);
  PLM_reg[3] = 0x37 | (uint8_t)(PLM_InitStruct->PLM_PreFilter)| (uint8_t)(PLM_InitStruct->PLM_Sensitivity);
  PLM_reg[4] = 0x94;
  PLM_reg[5] = (uint8_t)(PLM_InitStruct->PLM_BandpassFrequency & 0x07) | \
               (uint8_t)(PLM_InitStruct->PLM_BaudRate & 0x18) | \
               (uint8_t)(PLM_InitStruct->PLM_Deviation & 0x20);
  
  nAtt = 3; // Initialization attempts
  
  nTout = 0;
  while (nTout == 0)
  {
    /* Configuration Registers Transmission */
    PLM_WriteRegisters(PLM_reg, 6);
    nTout = 1000000;
    while((!PLM_Flag.PLM_F_TXOK)&&(nTout--));
    
    if (nTout)
    {
      /* Configuration Registers Verification */
      PLM_StartReadRegisters();
      
      nTout = 1000000;
      while((!PLM_Flag.PLM_F_RXOK)&&(nTout--));
      
      if (nTout)
      {
        for (i=0; i<6; i++)
        {
          if (PLM_reg[i] != SPI_Buffer[i])
          {
            nTout = 0;
            break;
          }
        }
      }
    }
    
    if (nTout == 0)
    {
      // Reset SPI clock bus
      SPI_Cmd(PLM_SPI_PERIPHERAL, DISABLE);
      if (PLM_SPI == 1)
        RCC_APB2PeriphResetCmd(PLM_RCC_APBPeriph_SPI, DISABLE);
      else
        RCC_APB1PeriphResetCmd(PLM_RCC_APBPeriph_SPI, DISABLE);
      if ((nAtt--) == 0)
      {
        // PLM hardware fault
        while(1);
      }
      if (PLM_SPI == 1)
        RCC_APB2PeriphResetCmd(PLM_RCC_APBPeriph_SPI, ENABLE);
      else
        RCC_APB1PeriphResetCmd(PLM_RCC_APBPeriph_SPI, ENABLE);
      nTout = 10000;
      while(nTout--);
      nTout = 0;
    }
  }
}

/*******************************************************************************
* Function Name  : PLM_FrameSent
* Description    : Get the frame sent flag
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint8_t PLM_FrameSent(void)
{
  return PLM_Flag.PLM_F_TXOK;
}

/*******************************************************************************
* Function Name  : PLM_CDPD_Interrupt
* Description    : Manage the CDPD interrupt line (falling edge)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PLM_CDPD_Interrupt(void)
{
  if (EXTI_GetITStatus((uint32_t)(EXTI_Line0 << PLM_CDPD_PIN)) != RESET)
  {
    if (PLM_Flag.PLM_F_CommNotAllowed == PLM_FLAG_FALSE)
    {
      // Falling Edge of CD_PD: Frame recognition start
      SPI_Cmd(PLM_SPI_PERIPHERAL, DISABLE);
      SPI_COMM_index = 0;
      SPI_I2S_ReceiveData(PLM_SPI_PERIPHERAL);
      /* Enable PLM SPI */
      SPI_Cmd(PLM_SPI_PERIPHERAL, ENABLE);
    }
    /* Clear the EXTI Line */
    EXTI_ClearITPendingBit((uint32_t)(EXTI_Line0 << PLM_CDPD_PIN));
  }
  /* If user uses the same int pin of a CDPD line, user routine is not 
     served because the correspondent bit has already been cleared */
#if (PLM_CDPD_PIN > 4) && (PLM_CDPD_PIN <= 15)
  _EXTI_UIRQHandler();
#endif
}


/*******************************************************************************
* Function Name  : PLM_CorrectFEC
* Description    : Chect and eventually corrects the forward errors (FEC) for a 
                   specified byte
* Input          : Received byte pointer, received FEC pointer
* Output         : None
* Return         : None
*******************************************************************************/
plm_flag_t PLM_CorrectFEC(uint8_t *byteRx, uint8_t *ucFecRx)
{
  uint8_t synd;	
  uint8_t i;	
  uint8_t ucFecCalc;	
  
  *ucFecRx = ~(*ucFecRx);
  *ucFecRx = (*ucFecRx) & 0x3f;	
  ucFecCalc = ((*byteRx)>>1);

  if(ucFecCalc & 0x40)
    ucFecCalc ^= 0x39;

  ucFecCalc = ((ucFecCalc << 1) | ((*byteRx) & 0x01));

  if(ucFecCalc & 0x40)
    ucFecCalc ^= 0x39;
  
  for(i=0; i<6; i++)
  {
    ucFecCalc = (ucFecCalc<<1) | ((((*ucFecRx) >> (5 - i)) & 0x01));
    if(ucFecCalc & 0x40)
      ucFecCalc ^= 0x39;
  }

  synd = ucFecCalc;
  synd &= 0x3f;
  if(synd)
  {
    (*byteRx) ^= LLucCRC[synd];
    return PLM_FLAG_TRUE;
  }
  return PLM_FLAG_FALSE;
}

/*******************************************************************************
* Function Name  : PLM_RXTX_Interrupt 
* Description    : Manage the CDPD SPI interrupt routine (on CDPD)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PLM_RXTX_Interrupt(void)
{
  // Transmission 
  if(SPI_I2S_GetITStatus(PLM_SPI_PERIPHERAL, SPI_I2S_IT_TXE) != RESET)
  {
    // End of transmission check 
    if(SPI_COMM_index == PLM_Flag.PLM_F_len)
    {
      /* Disable PLM SPI TXE interrupt */
      SPI_I2S_ITConfig(PLM_SPI_PERIPHERAL, SPI_I2S_IT_TXE, DISABLE);
      SPI_COMM_index++;
      return;
    }
    
    SPI_I2S_SendData(PLM_SPI_PERIPHERAL, SPI_Buffer[SPI_COMM_index++]);
  }
  
  // Reception
  if(SPI_I2S_GetITStatus(PLM_SPI_PERIPHERAL, SPI_I2S_IT_RXNE) != RESET)
  {
    // Reception
    if (SPI_COMM_Mode == SPI_TX_MODE)
    {
      // Read dummy byte and empty the register 
      SPI_I2S_ReceiveData(PLM_SPI_PERIPHERAL);
      if (SPI_COMM_index == (PLM_Flag.PLM_F_len+1))
      {
        SPI_COMM_index = 0;
        PLM_Flag.PLM_F_TXOK = PLM_FLAG_TRUE;
        PLM_SPI_Idle();
        SPI_COMM_Mode = SPI_HDR_MODE;
      }  
    }
    else
    {
      // Receiver or Header Mode
      SPI_Buffer[SPI_COMM_index++] = (uint8_t)SPI_I2S_ReceiveData(PLM_SPI_PERIPHERAL);
      
      if ((SPI_COMM_index == 1) && (PLM_Flag.PLM_F_Xmode == PLM_DATA))
      {
        if (SPI_Buffer[0] != HEADER_FRAME)
        {
          SPI_COMM_index = 0;
          PLM_SPI_Idle();
        }
        else
        {
          PLM_Flag.PLM_F_Xmode = PLM_LEN;
          // Header is filtererd
          SPI_COMM_index = 0;
          // Estimate the datalen. It will be tuned after
          SPI_MaxFrameCounter = DATAGRAM_MAX_LEN;
        }
      }
      
      // Read the datalen and check the FEC
      if ((SPI_COMM_index == 2) && (PLM_Flag.PLM_F_Xmode == PLM_LEN))
      {
        PLM_CorrectFEC(SPI_Buffer, (SPI_Buffer + 1));
        SPI_MaxFrameCounter = (SPI_Buffer[0] << 1) + 1; // payload + postamble byte
        SPI_COMM_index = 0;
        if (SPI_MaxFrameCounter > DATAGRAM_MAX_LEN)
        {
          PLM_Flag.PLM_F_Xmode = PLM_DATA;
          PLM_SPI_Idle();
          return;
        }
        PLM_Flag.PLM_F_Xmode = PLM_BODY;
      }
      
      // Verify the end of communication
      if (SPI_COMM_index > SPI_MaxFrameCounter)
      {
        PLM_Flag.PLM_F_len = ((SPI_MaxFrameCounter - 1) >> 1); // only payload len (without fec)
        PLM_Flag.PLM_F_CommNotAllowed = PLM_FLAG_TRUE;
        PLM_Flag.PLM_F_RXOK = PLM_FLAG_TRUE;
        PLM_SPI_Idle();
      }
    }
  }
}

/*******************************************************************************
* Function Name  : PLM_SPI_Idle
* Description    : Sets the PLM in idle mode
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PLM_SPI_Idle(void)
{
  /* Disable PLM SPI */
  SPI_Cmd(PLM_SPI_PERIPHERAL, DISABLE);
  SPI_I2S_ITConfig(PLM_SPI_PERIPHERAL, SPI_I2S_IT_TXE, DISABLE);
  // Read dummy byte and empty the register 
  SPI_I2S_ReceiveData(PLM_SPI_PERIPHERAL);
  PLM_DATASelect();
  PLM_RXSelect();
}

/*******************************************************************************
* Function Name  : PLM_TXSelect
* Description    : TX mode
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PLM_TXSelect(void)
{
  GPIO_ResetBits(PLM_RXTX_PORT_N, PLM_RXTX_PIN_N);
}

/*******************************************************************************
* Function Name  : PLM_RXSelect
* Description    : RX mode
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PLM_RXSelect(void)
{
  GPIO_SetBits(PLM_RXTX_PORT_N, PLM_RXTX_PIN_N);
}

/*******************************************************************************
* Function Name  : PLM_REGSelect
* Description    : Register mode
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PLM_REGSelect(void)
{
  PLM_WaitSPICLKLow();
  GPIO_SetBits(PLM_REGDATA_PORT_N, PLM_REGDATA_PIN_N);
}

/*******************************************************************************
* Function Name  : PLM_DATASelect
* Description    : Data mode
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PLM_DATASelect(void)
{
  PLM_WaitSPICLKLow();
  GPIO_ResetBits(PLM_REGDATA_PORT_N, PLM_REGDATA_PIN_N);
}

/*******************************************************************************
* Function Name  : PLM_SendData
* Description    : Sends bytes through PLM 
* Input          : PLM mode, buffer pointer, buffer len, class value
* Output         : None
* Return         : None
*******************************************************************************/
void PLM_SendData(uint8_t *buffer, uint8_t len)
{
  uint16_t index = 0;
  
  PLM_Flag.PLM_F_CommNotAllowed = PLM_FLAG_TRUE;
  SPI_COMM_Mode = SPI_TX_MODE;
  PLM_Flag.PLM_F_TXOK = PLM_FLAG_FALSE;
  PLM_Flag.PLM_F_Xmode = PLM_DATA;
  
  // len will be doubled for FEC and added with the header size (10 bytes)
  PLM_Flag.PLM_F_len = (len << 1) + 10; 
  // HEDER
  SPI_Buffer[0] = HEADER_WAKEUP_VALUE; // 0xAA (1)
  SPI_Buffer[1] = HEADER_WAKEUP_VALUE; // 0xAA (2)
  SPI_Buffer[2] = HEADER_WAKEUP_VALUE; // 0xAA (3)
  SPI_Buffer[3] = HEADER_WAKEUP_VALUE; // 0xAA (4)
  SPI_Buffer[4] = (uint8_t)(PLM_HeaderPreambleValue>> 8); // (5)
  SPI_Buffer[5] = (uint8_t)(PLM_HeaderPreambleValue & 0x0FF); // (6)
  SPI_Buffer[6] = HEADER_FRAME; // (7)
  
  // Here is placed the frame length with the correspondent FEC
  SPI_Buffer[7] = len; // (8)
  SPI_Buffer[8] = tableFEC[len]; // (9)
  
  // Reset index
  index = 9;
  // DATA
  while(len--)
  {
    // Data
    SPI_Buffer[index++]= *(buffer++);
    // FEC
    SPI_Buffer[index] = tableFEC[SPI_Buffer[index-1]];
    index++;
  }
  
  // Postamble (10)
  if(SPI_Buffer[index-1] & 0x01)
    SPI_Buffer[index++] = 0x03;
  else 
    SPI_Buffer[index++] = 0x00;

  SPI_COMM_index = 1;
  
  PLM_DATASelect(); // REGDATA = 0
  PLM_TXSelect(); // Register TxRx reset to 0

  /* SPI sync */
  PLM_WaitSPICLKLow();
  
  /* Send byte through the PLM SPI peripheral */
  SPI_I2S_SendData(PLM_SPI_PERIPHERAL, SPI_Buffer[0]);
  /* Enable PLM SPI  */
  SPI_Cmd(PLM_SPI_PERIPHERAL, ENABLE);
  /* Enable PLM SPI TXE interrupt */
  SPI_I2S_ITConfig(PLM_SPI_PERIPHERAL, SPI_I2S_IT_TXE, ENABLE);
}

/*******************************************************************************
* Function Name  : PLM_WriteRegisters
* Description    : Sends register bytes through PLM SPI
* Input          : PLM mode, buffer pointer, buffer len, class value
* Output         : None
* Return         : None
*******************************************************************************/
void PLM_WriteRegisters(uint8_t *buffer, uint8_t len)
{
  uint16_t index = 0;
  
  PLM_Flag.PLM_F_CommNotAllowed = PLM_FLAG_TRUE;
  SPI_COMM_Mode = SPI_TX_MODE;
  PLM_Flag.PLM_F_TXOK = PLM_FLAG_FALSE;
  PLM_Flag.PLM_F_Xmode = PLM_REG; // Register access
  
  PLM_Flag.PLM_F_len = len;
  while(len--)
    SPI_Buffer[index++] = *(buffer++);

  SPI_COMM_index = 1;
  PLM_REGSelect(); // REGDATA = 1
  PLM_TXSelect(); // Register TxRx reset to 0

  /* SPI sync */
  PLM_WaitSPICLKLow();
  
  /* Send byte through the PLM SPI peripheral */
  SPI_I2S_SendData(PLM_SPI_PERIPHERAL, SPI_Buffer[0]);
  /* Enable PLM SPI  */
  SPI_Cmd(PLM_SPI_PERIPHERAL, ENABLE);
  /* Enable PLM SPI TXE interrupt */
  SPI_I2S_ITConfig(PLM_SPI_PERIPHERAL, SPI_I2S_IT_TXE, ENABLE);
}

/*******************************************************************************
* Function Name  : PLM_StartReceiveData 
* Description    : Reads bytes from PLM 
* Input          : PLM mode
* Output         : None
* Return         : None
*******************************************************************************/
void PLM_StartReceiveData(void)
{
  PLM_Flag.PLM_F_RXOK = PLM_FLAG_FALSE;
  PLM_Flag.PLM_F_Xmode = PLM_DATA;
  PLM_Flag.PLM_F_len = 0;

  SPI_COMM_index = 0;
  
  SPI_COMM_Mode = SPI_HDR_MODE;
  PLM_DATASelect();
  PLM_RXSelect(); // Register TxRx set to 1
  
  /* SPI sync */
  PLM_WaitSPICLKLow();
  PLM_Flag.PLM_F_CommNotAllowed = PLM_FLAG_FALSE;
}

/*******************************************************************************
* Function Name  : PLM_StartReadRegisters 
* Description    : Reads bytes from PLM 
* Input          : PLM mode
* Output         : None
* Return         : None
*******************************************************************************/
void PLM_StartReadRegisters(void)
{
  PLM_Flag.PLM_F_RXOK = PLM_FLAG_FALSE;
  PLM_Flag.PLM_F_Xmode = PLM_REG;
  PLM_Flag.PLM_F_len = 0;

  SPI_COMM_index = 0;
  SPI_COMM_Mode = SPI_RX_MODE;
  SPI_MaxFrameCounter = 6; // 6 registers 
  
  PLM_REGSelect(); // REGDATA = 1
  PLM_RXSelect(); // Register TxRx set to 1
  
  /* SPI sync */
  PLM_WaitSPICLKLow();

  SPI_Cmd(PLM_SPI_PERIPHERAL, ENABLE);
}

/*******************************************************************************
* Function Name  : PLM_Reset
* Description    : Reset the PLM 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PLM_Reset(void)
{
  PLM_Flag.PLM_F_CommNotAllowed = PLM_FLAG_TRUE;
  
  PLM_Flag.PLM_F_RXOK = PLM_FLAG_FALSE;
  PLM_Flag.PLM_F_TXOK = PLM_FLAG_FALSE;  
  
  SPI_COMM_index = 0;
  SPI_COMM_Mode = SPI_HDR_MODE;
  PLM_SPI_Idle();
}

/*******************************************************************************
* Function Name  : PLM_CheckPostamble
* Description    : Check if the arrived SPI buffer postamble is correct
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
plm_flag_t PLM_CheckPostamble(uint8_t len)
{
  uint8_t ln;
  
  ln = len << 1;
  PLM_WrongPostamble = PLM_POSTAMBLE_OK;
  if((SPI_Buffer[ln]==0x03) && ((SPI_Buffer[ln-1]&0x01)==0x01)) 
    return PLM_FLAG_TRUE;
  if((SPI_Buffer[ln]==0x00) && ((SPI_Buffer[ln-1]&0x01)==0x00)) 
     return PLM_FLAG_TRUE;
  PLM_WrongPostamble = PLM_POSTAMBLE_WRONG;
  return PLM_FLAG_FALSE;
}

/*******************************************************************************
* Function Name  : PLM_CheckFEC
* Description    : Check the FEC for the entire received frame
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint8_t PLM_CheckFEC(void)
{
   uint8_t nDtBytes, i;
   uint8_t FECCorrections = 0;

   //nDtBytes = SPI_Buffer[14];
   nDtBytes = PLM_Flag.PLM_F_len * 2;
   for (i = 0; i < nDtBytes; i = i + 2)
   {
    if (PLM_CorrectFEC((SPI_Buffer + i), (SPI_Buffer + i + 1)))
      FECCorrections++;
   }
   return FECCorrections;
}

/*******************************************************************************
* Function Name  : PLM_GetBufferLength
* Description    : Return a length of the arrived buffer
* Input          : None
* Output         : None
* Return         : Frame lenght (0 = frame not arrived)
*******************************************************************************/
uint8_t PLM_GetBufferLength(void)
{
  if (PLM_Flag.PLM_F_RXOK)
    return PLMframe.len;
  return 0;
}

/*******************************************************************************
* Function Name  : PLM_GetWrongPostamble
* Description    : Return if the last received frame has a wrong postamble
* Input          : None
* Output         : None
* Return         : Frame lenght (0 = frame not arrived)
*******************************************************************************/
PLM_WP_TypeDef PLM_GetWrongPostamble(void)
{
  return PLM_WrongPostamble;
}

/*******************************************************************************
* Function Name  : PLM_GetFECCorrections
* Description    : Return the last received frame corrections
* Input          : None
* Output         : None
* Return         : Number of corrections
*******************************************************************************/
uint8_t PLM_GetFECCorrections(void)
{
  return PLM_FEC_Corrections;
}

/*******************************************************************************
* Function Name  : PLM_GetReceivedBufferPointer
* Description    : Return the pointer to the arrived buffer
* Input          : None
* Output         : None
* Return         : Buffer pointer
*******************************************************************************/
uint8_t*  PLM_GetReceivedBufferPointer(void)
{
  return PLMframe.framebuffer;
}

/*******************************************************************************
* Function Name  : PLM_ValidFrameArrived
* Description    : Checks if a valid frame is arrived, removing FEC and postamble
* Input          : None
* Output         : None
* Return         : Frame lenght (0=frame not arrived or not valid)
*******************************************************************************/
uint8_t PLM_ValidFrameArrived(void)
{
  uint8_t i;  
  
  PLMframe.len = 0;
  
  if (PLM_Flag.PLM_F_RXOK)
  {
    // postamble verification
    if (PLM_CheckPostamble(PLM_Flag.PLM_F_len))
    {
      // FEC corrections (if any)
      PLM_FEC_Corrections = PLM_CheckFEC();
      // Frame len assignment
      PLMframe.len = PLM_Flag.PLM_F_len;
      // FEC removal, buffer load
      for(i = 0; i < PLMframe.len; i++)
        PLMframe.framebuffer[i] = SPI_Buffer[i<<1];
    }
    else
      PLM_StartReceiveData(); // Wrong Postamble
  }
  
  return PLMframe.len;
}


/*******************************************************************************
* Function Name  : PLM_FrameArrived
* Description    : Checks if any frame is arrived
* Input          : None
* Output         : None
* Return         : Frame lenght (0=frame not arrived or not valid)
*******************************************************************************/
uint8_t PLM_FrameArrived(void)
{
  return PLM_Flag.PLM_F_RXOK;
}

/*******************************************************************************
* Function Name  : PLM_BandInUse
* Description    : Checks if the band is in use
* Input          : None
* Output         : None
* Return         : FLAG_TRUE (band in use), FLAG_FALSE (band not in use)
*******************************************************************************/
uint8_t PLM_BandInUse(void)
{
  uint8_t i;
  uint8_t nBU = 0;
  
  for (i=0; i<10; i++)
  {
    if (GPIO_ReadInputDataBit(PLM_BU_PORT_N, PLM_BU_PIN_N) == 0x01) // BU check
      nBU++;
  }
  if (nBU > 7)
    return (PLM_FLAG_TRUE);
  else
    return (PLM_FLAG_FALSE);
}

/*******************************************************************************
* Function Name  : PLM_WaitSPICLKLow 
* Description    : Wait until SCK is low for PLM streaming syncronization
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PLM_WaitSPICLKLow(void)
{
//  u32 tout = 100000;
  
  while (GPIO_ReadInputDataBit(PLM_SPI_PORT_N, PLM_SCK_PIN_N)>0);
//  {
//    tout--;
//  }
}


/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/

