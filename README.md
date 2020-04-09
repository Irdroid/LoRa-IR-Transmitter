# LoRa Infrared Transmitter (Long Range Infrared Transmitter)
### Powered By:
[![N|Solid](https://www.irdroid.com/wp-content/themes/clean-simple-white/logo300.png)](https://irdroid.eu/shop)

The LoRa IR Transmitter Kit is a long range Infrared Transmitter development kit that allows you to transmit infrared signals by using the LoRaWAN technology and connect to The Things Network Cloud. The Kit is useful in situations where you have a network of infrared controlled displays and players that need remote control, but there is no internet connection available nearby. Every kit is programmed (The Pronto codes are stored in the EEPROM) with the desired PRONTO HEX codes prior to installation on site. The IR codes then can be executed by accessing The Things Network MQTT API and send commands for executing a particular IR code.

# Features:

- Long Range - up to 10km line of sight
- Connect to The Things Network Cloud with diverse integrations such as MQTT etc.
- Low Power Consumption (can operate on batteries)
- High Power (20dBm) RF output power
- Options : 868MHz (EU) , 433MHz (US)
- High Power IR Leds
- Includes Arduino UNO in the kit with enclosure
- Includes SX1276 LoRaWAN transceiver shield
- Includes Irdroid light shield
- Supports Uplink and Downlink messages
- Supports Pronto HEX codes 


# Specification

- SX1276 High Power LoRa Radio Transceiver 20dBm
- 3dBi ( 5dBi optional ) antenna SMA connector
- Arduino UNO R3 in the Kit
- Black Enclosure
- Irdroino Shield
- USB Cable

# Use cases

- Digital Signage Systems
- Group Remote control and command scheduling for information displays
- Automation

### Purchase a kit

[![N|Solid](https://www.irdroid.com/wp-content/themes/clean-simple-white/logo300.png)](https://irdroid.eu/shop)

### Installation

- Download and install Arduino IDE,
- Download the example arduino sketch for recording pronto HEX codes to the unit EEPROM.
- Upload your Pronto HEX codes to the unit via the utility in this repo "Software" section
- Download the LoRaWAN IR Transmitter client for Arduino
- Register an application and device in the Things Network
- Add APP credentials to the arduino code and flash it to the arduino
- Your device should appear in your console at the Things Network
  