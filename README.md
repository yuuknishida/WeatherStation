# Real-Time Weather Station

A bare metal weather station running on a custom preemptive RTOS kernel written from scratch for the ARM Cortex-M4. it reads wind speed, wind direction, temperature, humidity, and air quality. then shows the results on an OLED, an 8-LED compass, and a UART serial console

## The project features:
- A Real Time Operating System kernel written in C WITH: per-task stacks, PendSV context switching, SVC-based launch
- 1 ms SysTick with fixed prioirty preemptive scheduling
- 5 periodic tasks
- Modbus RTU over RS-485 for the wind vane
- Bit-banged DHT11 single-wire protocol with checksum validation
- 16-sample averaged ADC with startup calibration
- SSD1306 OLED driver over SPI 

## Hardware
1. Anenometer (Analog (0-5 V) 12-bit ADC
2. Wind Vane (Modbus RTU) / RS-485
3. DHT11 (Single-wire GPIO), 40-bit protocol
4. MQ-135 Analog (0-5V), 12-bit ADC
5. LED compass (GPIO 8 LEDS)

## Tasks                
Task | Period   | Description
| :--- | :---: | :-- |
| Task_AirQuality      | 100 ms    | Reads MQ-135 via ADC0 SS2 |
| Task_WindDirection   | 110 ms    | Modbus requet to vane, updates LED compass |
| Task_WindSpeed       | 150 ms    | 16-sample average ADC -> mph, with debounce |
| Task_DisplayUpdate   | 900 ms    | snapshot -> OLED -> UART |
| Task_TempHumidity    | 2000 ms   | DHT11 40-bit read |

## How the RTOS Works
1. Task Creation: each task gets its own 128-word stack, pre-loaded with a fake exception frame (xPSR, PC, LR, R0-R12)
2. Launch OS_Launch triggers SVC #9
3. Tick: SysTick fires every 1ms, decrements sleep counters, wakes sleeping tasks, and pends PendSV
4. Scheduling: the lowest index READY task runs.

## Timing results      
Task | Period | Execution
| :--- | :---: | :-- |
| Task_AirQuality      | 100 ms    | 6 ms |
| Task_WindDirection   | 110 ms    | 47 ms |
| Task_WindSpeed       | 150 ms    | 59 ms |
| Task_DisplayUpdate   | 900 ms    | 54 ms |
| Task_TempHumidity    | 2000 ms   | 146 ms |

For the test we conducted 7 tests.
Test 1: High speed with constant direction
Test 2: High speed with changes in direction
Test 3: Air quality sensor manipulation (Normal, Poor)
Test 4: DHT sensor 
Test 5: Outdoor location test
Test 6: Direction sensor manipulation to confirm LED function as intended
Test 7: Sensor fault testing
