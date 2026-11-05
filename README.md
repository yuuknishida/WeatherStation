# Real-Time Weather Station

A real-time embedded weather station built using the **TM4C123GXL LaunchPad** and a custom **Real-Time Operating System (RTOS)**. The system collects environmental data from multiple sensors, processes the data on the microcontroller, and provides real-time output through an OLED display, UART, and LED indicators.

## Overview

This project was developed as an embedded systems project for **ECE 554 – Embedded Systems** at the University of Michigan-Dearborn.

The weather station monitors:

- Wind speed
- Wind direction
- Temperature
- Humidity
- Air quality

A custom RTOS was developed to manage the system's concurrent tasks and timing requirements. Sensor data is processed locally on the **TM4C123GXL**, allowing the system to operate independently of external network infrastructure.

The system was designed with fault tolerance in mind. Invalid or disconnected sensor inputs are handled without crashing or halting the rest of the system.

## Features

- Real-time environmental monitoring
- Custom RTOS
- Periodic task scheduling
- Fixed-priority task scheduling
- SysTick-based timing
- PendSV-based context switching
- Mailbox-based inter-task communication
- Wind speed measurement using an analog anemometer
- Wind direction measurement using an RS-485/Modbus RTU wind vane
- Temperature and humidity monitoring using a DHT11
- Air quality monitoring using an MQ-135
- OLED display using SPI
- UART communication at 9600 baud
- LED compass for wind direction
- "No Wind" detection
- Sensor fault handling
- Sensor checksum and input validation
- Continuous operation during sensor faults

## System Architecture

The TM4C123GXL acts as the central processing unit for the entire weather station.


