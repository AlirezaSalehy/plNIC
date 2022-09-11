/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : fwupdate.c
* Author             : Systems LAB - IMS (Industrial & Multisegment Sector)
* Date First Issued  : 14/06/2012
* Description        : Firmware update engine routines
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

/* Includes ------------------------------------------------------------------*/
#include "fwupdate.h"
#include "stm32f10x_flash.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define (From bootloader) ------------------------------------------*/
#define FLASH_BASE_ADDRESS      ((uint32_t)0x08000000)
#define VECTOR_TABLE_LEN        (0xEC) /* <!> FOR MIDDLE DENSITY DEVICES - UPDATE FOR OTHER DEVICES! */
#define BOOT_LOADER_SIZE        (0x1000)  /*  4 Kb (3 Kb Boot loader firmware -max-, 1 Kb boot loader data) */
#define IMAGE_MAX_SIZE          0xF000
#define BOOT_DATA_SIZE          (0x400)   /*  1 Kb */
#define FLASH_PAGE_SIZE         (0x400)   /*  1 Kb */
#define BOOT_DATA_ADDRESS       (FLASH_BASE_ADDRESS + BOOT_LOADER_SIZE - BOOT_DATA_SIZE)
#define ACTIVE_IMAGE_ADDRESS    (BOOT_DATA_ADDRESS)

#define FIRST_IMAGE_ADDRESS     (FLASH_BASE_ADDRESS + BOOT_LOADER_SIZE)
#define SECOND_IMAGE_ADDRESS    (FIRST_IMAGE_ADDRESS + IMAGE_MAX_SIZE)
#define DEFAULT_IMAGE_ADDRESS   FIRST_IMAGE_ADDRESS

/* Private variables ---------------------------------------------------------*/
bool FreeImageErased = FALSE;
uint8_t SignatureBuffer[FIRMWARE_SIGNATURE_BUFFER_SIZE];
uint16_t NewImageSize = 0;

/* hash key */
uint32_t h0, h1, h2, h3;

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
uint32_t  RFU_GetFreeImageAddress(void);
bool      RFU_EraseFreeImage(void);
bool      RFU_WriteSegment(uint32_t SegmentAddress, uint32_t SegemntWord);
uint32_t  RFU_GetWord(uint8_t *buffer, uint16_t *size);
bool      RFU_CheckImageSignature(uint32_t ImageAddress);


/*******************************************************************************
* Function Name  : RFU_SetImageData
* Description    : Write image data in the free image segment
* Input          : Image buffer, 
* Output         : None
* Return         : None
*******************************************************************************/
bool RFU_SetImageData(uint8_t *ImgSegmentBuffer, uint16_t ImgSegmentAddress, uint16_t ImgSegmentSize)
{
  uint32_t FreeImageAddress;
  uint32_t FirmwareSegment, ApplicationOffset, ImgSegmentIndex = 0;
    
  /* Get the free segment address */
  FreeImageAddress = RFU_GetFreeImageAddress();
  
  /* Check if the segment has to be erased */
  if (ImgSegmentAddress == 0)
  {
    /* First segment to write, erase fisrt the free image segment */
    if (!RFU_EraseFreeImage())
    {
      FreeImageErased = FALSE;
      return FALSE;
    }
    FreeImageErased = TRUE;
  }

  if (FreeImageErased)
  {
    if (ImgSegmentAddress == 0)
    {
      /* Get the stack pointer address (in RAM, no need to add offset) */
      FirmwareSegment = RFU_GetWord(ImgSegmentBuffer, &ImgSegmentSize);        
      /* Write stack pointer address (first location in vector table) */
      if (!RFU_WriteSegment(FreeImageAddress, FirmwareSegment))
        return FALSE;
      ImgSegmentAddress = 4;
      ImgSegmentIndex = 4;
    }
    
    while (ImgSegmentSize)
    {
      /* Get a firmware segment (4 bytes each) */
      FirmwareSegment = RFU_GetWord(ImgSegmentBuffer + ImgSegmentIndex, &ImgSegmentSize);
      
      /* Check if the address is within the vector table length, adding the right offset */
      if (ImgSegmentAddress < VECTOR_TABLE_LEN)
      {
        ApplicationOffset = (FreeImageAddress - FIRST_IMAGE_ADDRESS) + BOOT_LOADER_SIZE;
        if (FirmwareSegment != 0)
        {
          if (!RFU_WriteSegment(FreeImageAddress + ImgSegmentAddress, FirmwareSegment + ApplicationOffset))
            return FALSE;
        }
        else
        {
          if (!RFU_WriteSegment(FreeImageAddress + ImgSegmentAddress, 0))
            return FALSE;
        }
      }
      else
      {
        /* Write remaining firmware */
        if (!RFU_WriteSegment(FreeImageAddress + ImgSegmentAddress, FirmwareSegment))
            return FALSE;
      }
      ImgSegmentIndex += 4;
      ImgSegmentAddress += 4;
    }
  }
  return TRUE;
}

/*******************************************************************************
* Function Name  : RFU_SwapImageData
* Description    : Check if the new image data is correct, set the new 
*                  active image address and reset the microcontroller
* Input          : Image buffer, 
* Output         : None
* Return         : None
*******************************************************************************/
bool RFU_SwapImageData(void)
{
  FLASH_Status status = FLASH_BUSY;
  uint32_t NewActiveImage;
    
  /* Once firmware writing is completed, swap active image if the signature is correct */
  if (RFU_CheckImageSignature(RFU_GetFreeImageAddress()))
  {
    /* Get the new active address */
    NewActiveImage = RFU_GetFreeImageAddress();
    
    /* Unlock the Flash memory */
    FLASH_Unlock();
    
    /* Erase the bootloader data page */
    while (status == FLASH_BUSY)
      status = FLASH_ErasePage(BOOT_DATA_ADDRESS);
    if (status != FLASH_COMPLETE)
      return FALSE;
        
    /* Write in the active image address pointer the address of the new image */
    status = FLASH_BUSY;
    while (status == FLASH_BUSY)
      status = FLASH_ProgramWord(ACTIVE_IMAGE_ADDRESS, NewActiveImage);
    if (status != FLASH_COMPLETE)
      return FALSE;
        
    /* MICROCONTROLLER RESET - LOAD NEW FIRMWARE */
    NVIC_SystemReset();
    
    /* Never executed instruction because of reset */
    return TRUE;
  }
  return FALSE;
}

/*******************************************************************************
* Function Name  : RFU_GetImageSignature
* Description    : Store the new image signature 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RFU_GetImageSignature(uint8_t *signature)
{
  uint8_t i;
  
  for (i = 0; i < FIRMWARE_SIGNATURE_BUFFER_SIZE; i++)
    SignatureBuffer[i] = signature[i];
}

/*******************************************************************************
* Function Name  : RFU_GetImageSize
* Description    : Get the image size
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RFU_GetImageSize(uint8_t *size)
{
  NewImageSize = (((uint16_t)size[0]) << 8) | ((uint16_t)size[1]);
}

/*******************************************************************************
* Function Name  : RFU_CheckImageSignature
* Description    : Check the image signature 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool RFU_CheckImageSignature(uint32_t ImageAddress)
{
  /* Implements here hash key definition. Actually calculate just the sum of each byte */
  uint32_t checksum = 0, StoredCS = 0, BinSize, Segment;
  uint32_t i, ApplicationOffset, Ofs = 0, IntVect = 0;
  
  ApplicationOffset = ImageAddress - FIRST_IMAGE_ADDRESS + BOOT_LOADER_SIZE;
  Ofs = ((uint8_t)(ApplicationOffset >> 24) + (uint8_t)(ApplicationOffset >> 16) + 
                 (uint8_t)(ApplicationOffset >> 8) + (uint8_t)(ApplicationOffset));
  
    
  BinSize = NewImageSize / 4;
  for (i = 0; i < BinSize; i++)
  {
    Segment = *(uint32_t *)(ImageAddress + (i * 4));
    if ((i > 0) && ((i * 4) < VECTOR_TABLE_LEN) && Segment)
      IntVect++;
    checksum += ((uint8_t)(Segment >> 24) + (uint8_t)(Segment >> 16) + 
                 (uint8_t)(Segment >> 8) + (uint8_t)(Segment));
  }
  checksum -= (Ofs * IntVect);
  StoredCS = (((uint32_t)SignatureBuffer[0] << 24)) | (((uint32_t)SignatureBuffer[1] << 16)) | 
             (((uint32_t)SignatureBuffer[2] << 8)) | ((uint32_t)SignatureBuffer[3]);
  if (StoredCS != checksum)
    return FALSE;
  
  return TRUE;
}

/*******************************************************************************
* Function Name  : RFU_WriteSegment
* Description    : Write a word into flash
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool RFU_WriteSegment(uint32_t SegmentAddress, uint32_t SegemntWord)
{
   FLASH_Status status = FLASH_BUSY;
   
   while (status == FLASH_BUSY)
     status = FLASH_ProgramWord(SegmentAddress, SegemntWord);
   if (status != FLASH_COMPLETE)
      return FALSE;
   return TRUE;
}

/*******************************************************************************
* Function Name  : RFU_EraseFreeImage
* Description    : Erase the free image segment
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
bool RFU_EraseFreeImage(void)
{
  uint32_t FreeImageAddress = RFU_GetFreeImageAddress();
  FLASH_Status status = FLASH_BUSY;
  uint32_t i;
  
  /* Unlock the Flash Program Erase controller */
  FLASH_Unlock();
  
  /* Clear All pending flags */
  FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);  
  
  /* Erase entire free image area */
  for (i = 0; i < (IMAGE_MAX_SIZE / FLASH_PAGE_SIZE); i++)
  {
    while (status == FLASH_BUSY)
      status = FLASH_ErasePage(FreeImageAddress + (i * FLASH_PAGE_SIZE));
    if (status != FLASH_COMPLETE)
      return FALSE;
    status = FLASH_BUSY;
  }
  return TRUE;
}

/*******************************************************************************
* Function Name  : RFU_GetFreeImageAddress
* Description    : Get the free image address 
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint32_t RFU_GetFreeImageAddress(void)
{
  if (*((uint32_t *)ACTIVE_IMAGE_ADDRESS) == FIRST_IMAGE_ADDRESS)
    return (SECOND_IMAGE_ADDRESS);
  else
    return (FIRST_IMAGE_ADDRESS);
}

/*******************************************************************************
* Function Name  : RFU_GetWord
* Description    : Get a word from a buffer (little endian)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint32_t RFU_GetWord(uint8_t *buffer, uint16_t *size)
{ 
  uint32_t FwSeg = 0;
  uint8_t tot = 0;
  
  while ((*size)-- && (tot < 4))
  {
    FwSeg |= ((uint32_t)buffer[tot]) << (8 * tot);
    tot++;
  }
  (*size)++;
  return FwSeg;
}


/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/


