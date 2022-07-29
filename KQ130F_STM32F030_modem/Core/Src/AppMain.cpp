#include <cmsis_os.h>

#include "AppMain.hpp"
#include "PLModem.hpp"
#include "Encryption.h"


extern I2C_HandleTypeDef hi2c1;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

uint8_t helloWorld[] = "hello world!\n\r";

Enc::RSA rsaEnc();
PLM::PLModem<512> KQ130F(&huart2, [](uint8_t* frame, const uint8_t len)
								{ HAL_UART_Transmit(&huart1, (uint8_t*)"RX", 2, 500); });


void appMain(const void* args)
{

  while (1)
  {
	  HAL_GPIO_TogglePin(RX_LED_GPIO_Port, RX_LED_Pin);
	  osDelay(1000);

	  HAL_GPIO_TogglePin(TX_LED_GPIO_Port, TX_LED_Pin);
	  osDelay(1000);

	  HAL_UART_Transmit(&huart2, helloWorld, sizeof(helloWorld), 1000);
	  HAL_UART_Transmit(&huart1, helloWorld, sizeof(helloWorld), 1000);

  }
}


