# BSE_Project_PLM
The under developed plNIC will provide encryption, DMA, buffering techniques, CRC error checking and RTOS(FreeRTOS).

## Hardware

### KQ130-F
This PLM has fairly simple application and interface circuit, Though it is not provided with enough and clear documentations. Here is the pinout:
<img src="Docs/Imgs/KQ130-F_pinout.png" width="100%" height="100%">
As it's shown; there is 9 pins in total and that pins 1 and 2 are connected to AC powerline(No matter you connect phase wire or neutral wire to pin 1 or 2), pins 3, 4 and 5 provides power and pin 6 and 7 are used for data communication. 
 
<img src="Docs/Imgs/KQ130-F_Picture_Connection.png" width="100%" height="100%">
The Figure above shows connection for AT89C51 MCU which uses TTL logic level. Remember that this device suppots TTL logical voltage levels so you need a voltage devider if you want to use it with a 3.3v MCU like STM32 MCUs. I used <a href="https://en.wikipedia.org/wiki/Voltage_divider#:~:text=Resistor%20voltage%20dividers%20are%20commonly,signal%20attenuators%20at%20low%20frequencies." title="Resistor Voltage Devider">Resistor Voltage Devider</a> to convert 5v TTL to 3.3v in MCU rx <-> KQ130-F tx connection. However there is no need to do so for MCU tx <-> KQ130-F rx.

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





