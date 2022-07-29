# BSE_Project_PLM
The under developed plNIC will provide encryption, DMA, buffering techniques, CRC error checking and RTOS(FreeRTOS).

## Hardware

### KQ130-F
This PLM has faily simple application and circuit, Though it does not provided with enough and clear documentations. Here is the pinout:
<img src="Docs/Imgs/KQ130-F_pinout.png" width="100%" height="100%">
As its shown there is 9 pins in total that pins 1 and 2 are connected to AC powerline(No matter you connect phase or neutral to pin 1 or 2), pins 3, 4, 5 provides power and pin 6, 7 are for data communication. remember that this device suppots TTL logical volateg levels so you need a voltage devider for you want to use it with a 3.3v MCU like STM32 MCUs. I used <a href="https://en.wikipedia.org/wiki/Voltage_divider#:~:text=Resistor%20voltage%20dividers%20are%20commonly,signal%20attenuators%20at%20low%20frequencies." title="Resistor Voltage Devider">Resistor Voltage Devider</a> to convert 5v TTL to 3.3v in MCU rx <-> KQ130-F tx connection. However there is no need to do so for MCU tx <-> KQ130-F rx.
 
<img src="Docs/Imgs/KQ130-F_Picture_Connection.png" width="100%" height="100%">
Figure above showing connection for AT89C51 MCU which uses TTL logic level.

### STM32F030C8
#### DMA
#### CRC
#### UART

### Schematic

### Bill of Materials

### Final Module
The discussed plNIC soldered on double sided hole board as bellow:
<img src="Docs/Imgs/Modem_Physical_Picture.jpg" width="70%" height="70%">

## Software





