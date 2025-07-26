
# STM32 Atmospheric Data Display Project

## Overview

This project uses an **STM32 microcontroller** to display real-time atmospheric data on an **LCD module**.

## Features

- Reads atmospheric data (e.g., temperature, humidity, pressure) from sensors
- Displays data in real-time on a standard character or graphic LCD
- Optional UART interface for serial monitoring and debugging

## Components

- STM32 **NUCLEO-F103RB** 
- LCD display **SZH-EK101** (I2C-based LCD)
- Atmospheric sensor (DHT22)
- Power supply
- UART-to-USB (optional, for serial output to PC)

## Functionality

- Continuously reads data from the atmospheric sensor
- Updates the LCD every second with current readings
- Can be extended to log data or send it via UART

## Example Display (on LCD)
<img width="1561" height="1178" alt="image" src="https://github.com/user-attachments/assets/e93f21ca-f8ac-4d75-99bb-afbade678305" />


## Source 

https://github.com/MYaqoobEmbedded/STM32-Tutorials/blob/master/Tutorial%2025%20-%20DHT22%20Temperature%20Sensor/MY_DHT22.c
https://github.com/SayidHosseini/STM32LiquidCrystal_I2C
