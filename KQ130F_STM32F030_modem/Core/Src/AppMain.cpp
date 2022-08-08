#include <cmsis_os.h>

#include "AppMain.hpp"
#include "Encryption.h"

extern I2C_HandleTypeDef hi2c1;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

extern IWDG_HandleTypeDef hiwdg;

extern RTC_HandleTypeDef hrtc;
extern CRC_HandleTypeDef hcrc;


uint8_t helloWorld[] = "hello world!\n\r";

Enc::RSA rsaEnc;
PLM::PLModem<1024> KQ130F(&huart2, &huart1, [](uint8_t* frame, const size_t len)
								{ HAL_UART_Transmit(&huart1, (uint8_t*)"RX", 2, 500); });


void defaultThreadTask(const void* params) {
  for(;;)
  {
	osDelay(1);

	HAL_IWDG_Refresh(&hiwdg);
  }
}

void StartplNICHandleTask(void const* params) {
	KQ130F.handle(params);
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
 UNUSED(huart);

	 KQ130F.rxInterrupt(huart);
}

void StartTestCaseTask(const void* args)
{
	uint8_t netId;
#ifdef TRANSMITTER
	netId = TRANSMITTER_NET_ID;
#elif RECEIVER
	netId = RECEIVER_NET_ID;
#endif

	KQ130F.begin(netId, rsaEnc, 3, 300000);
  while (1)
  {
#ifdef TRANSMITTER
	  KQ130F.send(RECEIVER_NET_ID, helloWorld, strlen((char*)helloWorld));

#elif RECEIVER

#endif

	  HAL_GPIO_TogglePin(RX_LED_GPIO_Port, RX_LED_Pin);
	  osDelay(1000);
  }
}


