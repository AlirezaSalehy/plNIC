/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : comm.c
* Author             : Systems LAB - IMS (Industrial & Multisegment Sector)
* Date First Issued  : 03/12/2008
* Description        : Communication interface layer routines
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

#define ___USE_COMM_REDEF___

/* Includes ------------------------------------------------------------------*/
#include "comm.h"
#include "usb.h"
#include "stk_p.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* HARDWARE INTERFACE DEFINITIONS <!> DO NOT MODIFY */
#if (COMM_INTERFACE_TYPE == 1)
  #define COMM_INTERFACE              USART1
  #define COMM_USART_StackUpdate      USART1_IRQHandler
#elif (COMM_INTERFACE_TYPE == 2)
  #define COMM_INTERFACE              USART2
  #define COMM_USART_StackUpdate      USART2_IRQHandler
#elif (COMM_INTERFACE_TYPE == 3)
  #define COMM_INTERFACE              USART3
  #define COMM_USART_StackUpdate      USART3_IRQHandler
#elif (COMM_INTERFACE_TYPE == 4)
  #define COMM_INTERFACE              SPI1
  #define COMM_SPI_StackUpdate        SPI1_IRQHandler
  #define COMM_RCC_APBPeriphResetCmd  RCC_APB2PeriphResetCmd
  #define COMM_RCC_APBPeriph_SPI      RCC_APB2Periph_SPI1
#elif (COMM_INTERFACE_TYPE == 5)
  #define COMM_INTERFACE              SPI2
  #define COMM_SPI_StackUpdate        SPI2_IRQHandler
  #define COMM_RCC_APBPeriphResetCmd  RCC_APB1PeriphResetCmd
  #define COMM_RCC_APBPeriph_SPI      RCC_APB1Periph_SPI2
#endif


/* Private variables ---------------------------------------------------------*/
uint8_t COMM_buffer[2 * USER_PAYLOAD_SIZE];

bool    bCOMMFrameArrived = FALSE;
bool    bCOMMFrameTransmitted = TRUE;
bool    bHeaderFound = FALSE;
bool    bCOMMFrameTxOngoing = FALSE;
bool    bCOMMFrameRxOngoing = FALSE;
uint8_t nCommIndex = 0;

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void  COMM_USARTConfig(void);
void  COMM_SPIConfig(void);
void  COMM_StopFrameTransmission(void);

/* Private functions ---------------------------------------------------------*/



/*******************************************************************************
* Function Name  : COMM_SPIConfig 
* Description    : Configures the SPI used for the communication
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void COMM_SPIConfig(void)
{
#if (COMM_INTERFACE_TYPE == 4) || (COMM_INTERFACE_TYPE == 5)
  SPI_InitTypeDef   SPI_InitStructure;
  GPIO_InitTypeDef  GPIO_InitStructure;
  NVIC_InitTypeDef  NVIC_InitStructure;

  if (COMM_SPI_PORT == GPIOA)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);
  else if (COMM_SPI_PORT == GPIOB)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);
  else if (COMM_SPI_PORT == GPIOC)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOC, ENABLE);
  else if (COMM_SPI_PORT == GPIOD)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOD, ENABLE);
  else if (COMM_SPI_PORT == GPIOE)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOE, ENABLE);
  else if (COMM_SPI_PORT == GPIOF)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOF, ENABLE);
  else 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOG, ENABLE);
  
  if (COMM_INTERFACE_TYPE == COMM_SPI1)
  {
    RCC_APB2PeriphClockCmd(COMM_RCC_APBPeriph_SPI, ENABLE);
    NVIC_InitStructure.NVIC_IRQChannel = SPI1_IRQn;
  }
  else
  {
    RCC_APB1PeriphClockCmd(COMM_RCC_APBPeriph_SPI, ENABLE);
    NVIC_InitStructure.NVIC_IRQChannel = SPI2_IRQn;
  }
  
  /* Configure PLM SPI pins: SCK, MISO and MOSI */
  GPIO_InitStructure.GPIO_Pin = COMM_SCK_PIN | COMM_MOSI_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
#ifdef COMM_SPI_MISO_FLOATING
  GPIO_Init(COMM_SPI_PORT, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = COMM_MISO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
#else
  GPIO_InitStructure.GPIO_Pin |= COMM_MISO_PIN;
#endif
  GPIO_Init(COMM_SPI_PORT, &GPIO_InitStructure);
  
  /* Configure and enable PLM SPI interrupt ----------------------------------*/
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = COMM_INT_PREEMPTY_PRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = COMM_INT_SUB_PRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  /* PLM SPI configuration */ 
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_InitStructure.SPI_NSS = COMM_SPI_NSS_MODE;
  SPI_InitStructure.SPI_CPOL = COMM_SPI_POL;
  SPI_InitStructure.SPI_CPHA = COMM_SPI_EDGE;
  SPI_InitStructure.SPI_BaudRatePrescaler = COMM_SPI_PRESCALER;
  SPI_InitStructure.SPI_FirstBit = COMM_SPI_FIRST_BIT;
  SPI_Init(COMM_INTERFACE, &SPI_InitStructure);
  
  /* SPI Idle */
  SPI_I2S_ITConfig(COMM_INTERFACE, SPI_I2S_IT_RXNE, ENABLE); // Enable PLM SPI RXNE interrupt 
  nCommIndex = 0;
  SPI_Cmd(COMM_INTERFACE, DISABLE);
  SPI_I2S_ITConfig(COMM_INTERFACE, SPI_I2S_IT_TXE, DISABLE); // Disable PLM SPI TXE interrupt 
  SPI_I2S_ReceiveData(COMM_INTERFACE);
#endif
}

/*******************************************************************************
* Function Name  : COMM_USARTConfig
* Description    : Configures the USART used for the communication
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void COMM_USARTConfig(void)
{
#if (COMM_INTERFACE_TYPE >= 1) && (COMM_INTERFACE_TYPE <= 3)
  NVIC_InitTypeDef  NVIC_InitStructure;  
  GPIO_InitTypeDef  GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  USART_ClockInitTypeDef USART_ClockInitStruct;
  
  if (COMM_INTERFACE == USART1)
  {
    /* Enable the USART1 and AFIO peripheral clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_AFIO, ENABLE);
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  }
  else if (COMM_INTERFACE == USART2)
  {
    /* Enable the USART2 and AFIO peripheral clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  }
  else
  {
    /* Enable the USART3 (as default id nothing defined) and AFIO peripheral clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
  }
  
  /* Enable the USART Interrupt */
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = COMM_INT_PREEMPTY_PRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = COMM_INT_SUB_PRIORITY;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
  
  //////////////////////////////////////////////////////////////////
  // Configure the GPIO ports( USART Transmit and Receive Lines) 
  //////////////////////////////////////////////////////////////////

#define COMM_USART_TX_PORT_N  ((GPIO_TypeDef *) (GPIOA_BASE + ((GPIOB_BASE - GPIOA_BASE) * COMM_USART_TX_PORT)))
#define COMM_USART_RX_PORT_N  ((GPIO_TypeDef *) (GPIOA_BASE + ((GPIOB_BASE - GPIOA_BASE) * COMM_USART_RX_PORT)))
#define COMM_USART_TX_PIN_N   ((uint16_t)(GPIO_Pin_0 << COMM_USART_TX_PIN))
#define COMM_USART_RX_PIN_N   ((uint16_t)(GPIO_Pin_0 << COMM_USART_RX_PIN))

  /* Configure the USART_Tx as Alternate function Push-Pull */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin =  COMM_USART_TX_PIN_N;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(COMM_USART_TX_PORT_N, &GPIO_InitStructure);
  
  /* Configure the USART_Rx as input floating */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_InitStructure.GPIO_Pin = COMM_USART_RX_PIN_N;
  GPIO_Init(COMM_USART_RX_PORT_N, &GPIO_InitStructure);
  
  /*-------------------------------------------------------
  USART configured as:
        - Word Length = 8 Bits
        - 1 Stop Bit
        - No parity
        - BaudRate = USART_BAUDRATE bps
        - Receive and transmit enabled
  -------------------------------------------------------*/
#ifndef COMM_USART_BAUDRATE
#define COMM_USART_BAUDRATE 9600
#endif
  
  USART_InitStructure.USART_BaudRate = COMM_USART_BAUDRATE;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No ;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(COMM_INTERFACE, &USART_InitStructure);
  
  USART_ClockInitStruct.USART_Clock = USART_Clock_Disable;
  USART_ClockInitStruct.USART_CPOL = USART_CPOL_Low;
  USART_ClockInitStruct.USART_CPHA = USART_CPHA_2Edge;
  USART_ClockInitStruct.USART_LastBit = USART_LastBit_Disable;  
  USART_ClockStructInit(&USART_ClockInitStruct);
 
  /* Enable the USART Receive register not empty interrupt */
  USART_ITConfig(COMM_INTERFACE, USART_IT_RXNE, ENABLE);
  
  /* Enable the USART */
  USART_Cmd(COMM_INTERFACE, ENABLE);  
#endif
}

/*******************************************************************************
* Function Name  : COMM_Init
* Description    : Initialize the communication flags
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void COMM_Init(void)
{
#if (COMM_INTERFACE_TYPE >= 1) && (COMM_INTERFACE_TYPE <= 3)
  /* Configure the USART */
  COMM_USARTConfig();
#elif (COMM_INTERFACE_TYPE == 4) || (COMM_INTERFACE_TYPE == 5)
  COMM_SPIConfig();
#endif
  
  /* Initialize the communication flags */  
  bCOMMFrameArrived = FALSE;
  bCOMMFrameTxOngoing = FALSE;
  bCOMMFrameRxOngoing = FALSE;
}

/*******************************************************************************
* Function Name  : COMM_Reset
* Description    : Reset USART peripheral 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void COMM_Reset(void)
{
#if (COMM_INTERFACE_TYPE >= 1) && (COMM_INTERFACE_TYPE <= 3) 
  USART_Cmd(COMM_INTERFACE, DISABLE);
  USART_Cmd(COMM_INTERFACE, ENABLE);
#elif (COMM_INTERFACE_TYPE == 4) || (COMM_INTERFACE_TYPE == 5)
  SPI_Cmd(COMM_INTERFACE, DISABLE);
  COMM_RCC_APBPeriphResetCmd(COMM_RCC_APBPeriph_SPI, DISABLE);
  COMM_RCC_APBPeriphResetCmd(COMM_RCC_APBPeriph_SPI, ENABLE);
#endif
}

/*******************************************************************************
* Function Name  : COMM_StartTransmission 
* Description    : Starts the USART communication 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool COMM_StartTransmission(void)
{
#ifdef COMM_INTERFACE
  u32 tstmp;
  
  if (bCOMMFrameRxOngoing)
    return FALSE;
  else
  {
    bCOMMFrameTransmitted = FALSE;
    nCommIndex = 0;
    tstmp = _timestamp;
    /* Communication Start */
#if (COMM_INTERFACE_TYPE >= 1) && (COMM_INTERFACE_TYPE <= 3)
    // USART 
    USART_ITConfig(COMM_INTERFACE, USART_IT_TXE, ENABLE);
    while (!bCOMMFrameTxOngoing && COMM_USART_START_TX_TIMEOUT)
    {
      if (PL_DelayElapsed(tstmp, COMM_USART_START_TX_TIMEOUT))
      {
        /* Reset USART peripheral */
        USART_Cmd(COMM_INTERFACE, DISABLE);
        USART_Cmd(COMM_INTERFACE, ENABLE);
        bCOMMFrameTransmitted = TRUE;
        return FALSE;
      }
    }
    return TRUE;
#elif (COMM_INTERFACE_TYPE == 4) || (COMM_INTERFACE_TYPE == 5)
    // SPI
    /* Set transmission flag */
    bCOMMFrameTxOngoing = TRUE;
    /* Clear SPI rx buffer */
    SPI_I2S_ReceiveData(COMM_INTERFACE);
    /* Send byte through the PLM SPI peripheral */
    nCommIndex = 0;
    SPI_I2S_SendData(COMM_INTERFACE, COMM_buffer[nCommIndex++]);
    /* Enable PLM SPI  */
    SPI_Cmd(COMM_INTERFACE, ENABLE);
    /* Enable PLM SPI TXE interrupt */
    SPI_I2S_ITConfig(COMM_INTERFACE, SPI_I2S_IT_TXE, ENABLE);
    /* Wait the start of transmission */
    while (!bCOMMFrameTxOngoing && COMM_SPI_START_TX_TIMEOUT)
    {
      if (PL_DelayElapsed(tstmp, COMM_SPI_START_TX_TIMEOUT))
      {
        SPI_Cmd(COMM_INTERFACE, DISABLE);
        COMM_RCC_APBPeriphResetCmd(COMM_RCC_APBPeriph_SPI, DISABLE);
        COMM_RCC_APBPeriphResetCmd(COMM_RCC_APBPeriph_SPI, ENABLE);
        bCOMMFrameTransmitted = TRUE;
        return FALSE;
      }
    }
    return TRUE;
#else
    return FALSE;
#endif
  }
#else
    return FALSE;
#endif
}

/*******************************************************************************
* Function Name  : COMM_StopFrameTransmission
* Description    : Force to stop actual frame transmission
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void COMM_StopFrameTransmission(void)
{
#if (COMM_INTERFACE_TYPE >= 1) && (COMM_INTERFACE_TYPE <= 3)
  USART_ITConfig(COMM_INTERFACE, USART_IT_TXE, DISABLE);
  bCOMMFrameTxOngoing = FALSE;
#endif
}

/*******************************************************************************
* Function Name  : COMM_ResetReceiver
* Description    : Force to stop receiving the incoming frame
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void COMM_ResetReceiver(void)
{  
  bCOMMFrameArrived = FALSE;
  bCOMMFrameRxOngoing = FALSE;
  nCommIndex = 0;
}

/*******************************************************************************
* Function Name  : COMM_FrameArrived
* Description    : Return the frame arrived flag
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool COMM_FrameArrived(void)
{
  return bCOMMFrameArrived;
}

/*******************************************************************************
* Function Name  : COMM_FrameTransmitted
* Description    : Return the frame transmitted flag
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool COMM_FrameTransmitted(void)
{
  return bCOMMFrameTransmitted;
}

/*******************************************************************************
* Function Name  : COMM_TransmissionOngoing
* Description    : Return the frame transmission flag
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool COMM_TransmissionOngoing(void)
{
  return bCOMMFrameTxOngoing;
}

/*******************************************************************************
* Function Name  : COMM_FrameReceivingOngoing
* Description    : Return the frame reception flag
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool COMM_FrameReceivingOngoing(void)
{
  return bCOMMFrameRxOngoing;
}

/*******************************************************************************
* Function Name  : COMM_EnableReceiver
* Description    : Clear the frame arrived flag
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void COMM_EnableReceiver(void)
{
  bCOMMFrameArrived = FALSE;
}

/*******************************************************************************
* Function Name  : COMM_GetBufferPointer
* Description    : Get the pointer to the USART buffer
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint8_t* COMM_GetBufferPointer(void)
{
  return COMM_buffer;
}

/*******************************************************************************
* Function Name  : COMM_SPI_StackUpdate 
* Description    : SPI Communication State Machine
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void COMM_SPI_StackUpdate(void)
{
#if (COMM_INTERFACE_TYPE == 4) || (COMM_INTERFACE_TYPE == 5)

  uint8_t nSPIData;
  static uint8_t nFrameLen = 0;
  
  /* SPI COMMUNICATION STATE MACHINE */ 
  
  ////////////////////////////////////////////////////////////
  // Data Byte Transmitted 
  ////////////////////////////////////////////////////////////
  
  if(SPI_I2S_GetITStatus(COMM_INTERFACE, SPI_I2S_IT_TXE) != RESET)
  { 
    if (!bCOMMFrameRxOngoing)
    {
      if (nCommIndex == 1)
        nFrameLen = COMM_buffer[nCommIndex-1];
      
      // Check data frame length
      if (nCommIndex == nFrameLen)
      {
        nCommIndex++;
        SPI_I2S_ITConfig(COMM_INTERFACE, SPI_I2S_IT_TXE, DISABLE);
        return;
      }
      /* Transmit next byte */
      SPI_I2S_SendData(COMM_INTERFACE, (uint16_t)(COMM_buffer[nCommIndex++]));
    }
    else
    {
      SPI_I2S_ITConfig(COMM_INTERFACE, SPI_I2S_IT_TXE, DISABLE);
      bCOMMFrameTxOngoing = FALSE;
      nCommIndex = 0;
    }
  }

  ////////////////////////////////////////////////////////////
  // Data Byte Received
  ////////////////////////////////////////////////////////////
  
  if(SPI_I2S_GetITStatus(COMM_INTERFACE, SPI_I2S_IT_RXNE) != RESET)
  {
    /* Byte reception in Tx mode */
    if (bCOMMFrameTxOngoing)
    {
      // Read dummy byte and empty the input register 
      SPI_I2S_ReceiveData(COMM_INTERFACE);
      if (nCommIndex == (nFrameLen + 1))
      {
        bCOMMFrameTxOngoing = FALSE;
        bCOMMFrameTransmitted = TRUE;
        nCommIndex = 0;
      }
    }
    else
    {      
      /* Byte reception in Rx mode */
      if (!bCOMMFrameArrived)
      {
        nSPIData = (uint8_t)SPI_I2S_ReceiveData(COMM_INTERFACE);
        COMM_buffer[nCommIndex++] = nSPIData;
        
        if (nCommIndex == 1)
        {
          /* Load frame len */
          nFrameLen = nSPIData; 
          bCOMMFrameRxOngoing = TRUE;
          if (nFrameLen > USER_PAYLOAD_SIZE)
          {
            /* Discharge frames bigger than maximum allowed */
            bCOMMFrameRxOngoing = FALSE;
            nCommIndex = 0;
            return;
          }
        }
        
        /* Check data frame length */
        if (nCommIndex >= nFrameLen)
        {
          bCOMMFrameRxOngoing = FALSE;
          nCommIndex = 0;
          bCOMMFrameArrived = TRUE;
          bCOMMFrameRxOngoing = FALSE;
         }
      }
      else
      {
        // Discharge received byte and reset indexes
        SPI_I2S_ReceiveData(COMM_INTERFACE);
        bCOMMFrameRxOngoing = FALSE;
        nCommIndex = 0;
        nFrameLen = 0;
      }
    }
  }
#endif
}

/*******************************************************************************
* Function Name  : COMM_USART_StackUpdate 
* Description    : USART Communication State Machine
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void COMM_USART_StackUpdate(void)
{
#if (COMM_INTERFACE_TYPE >= 1) && (COMM_INTERFACE_TYPE <= 3)
  uint8_t nUSARTData;
  static uint8_t nFrameLen = 0;
  
  /* USART COMMUNICATION STATE MACHINE */ 
  
  if(USART_GetITStatus(COMM_INTERFACE, USART_IT_RXNE) != RESET)
  {
    ////////////////////////////////////////////////////////////
    // Data Byte Received
    ////////////////////////////////////////////////////////////

    if ((!bCOMMFrameArrived) && (!bCOMMFrameTxOngoing))
    {
      nUSARTData = (unsigned char)(USART_ReceiveData(COMM_INTERFACE));      
      COMM_buffer[nCommIndex++] = nUSARTData;
      
      if (nCommIndex == 1)
      { 
        /* Load frame len */
        nFrameLen = nUSARTData;
        bCOMMFrameRxOngoing = TRUE;
        if (nFrameLen > USER_PAYLOAD_SIZE)
        {
          /* Discharge frames bigger than maximum allowed */
          bCOMMFrameRxOngoing = FALSE;
          USART_ClearITPendingBit(COMM_INTERFACE, USART_IT_RXNE);
          nCommIndex = 0;
          return;
        }
      }
      
      /* Check data frame length */
      if (nCommIndex >= nFrameLen)
      {
        bCOMMFrameRxOngoing = FALSE;
        nCommIndex = 0;
        bCOMMFrameArrived = TRUE;
        bCOMMFrameRxOngoing = FALSE;
       }
    }
    else
    {
      /* Discharge received byte and reset indexes */
      USART_ReceiveData(COMM_INTERFACE);
      bCOMMFrameRxOngoing = FALSE;
      nCommIndex = 0;
      nFrameLen = 0;
    }
    USART_ClearITPendingBit(COMM_INTERFACE, USART_IT_RXNE);
    return;
  }
  
  if(USART_GetITStatus(COMM_INTERFACE, USART_IT_TXE) != RESET)
  {
    ////////////////////////////////////////////////////////////
    // Data Byte Transmitted 
    ////////////////////////////////////////////////////////////
    
    if (!bCOMMFrameRxOngoing)
    {
      if (!bCOMMFrameTxOngoing)
      {
        bCOMMFrameArrived = FALSE;
        bCOMMFrameTxOngoing = TRUE;
        nCommIndex = 0;
      }
      if (nCommIndex == 1)
        nFrameLen = COMM_buffer[nCommIndex-1];
      
      USART_SendData(COMM_INTERFACE, (uint16_t)(COMM_buffer[nCommIndex++]));
      
      
      if (nCommIndex == nFrameLen)
      {
        nCommIndex = 0;
        USART_ITConfig(COMM_INTERFACE, USART_IT_TXE, DISABLE);
        bCOMMFrameTxOngoing = FALSE;
        bCOMMFrameTransmitted = TRUE;
      }
    }
    USART_ClearITPendingBit(COMM_INTERFACE, USART_IT_TXE);
    return;
  }
  nFrameLen = 0;
  nCommIndex = 0;
  bCOMMFrameRxOngoing = FALSE;
  bCOMMFrameTxOngoing = FALSE;
  USART_ReceiveData(COMM_INTERFACE);
  USART_ClearITPendingBit(COMM_INTERFACE, USART_IT_TXE);
  USART_ClearITPendingBit(COMM_INTERFACE, USART_IT_TC | USART_IT_CTS | USART_IT_LBD);
#endif
}


/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/

