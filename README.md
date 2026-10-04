# Runtime-Supervised-Water-Pump-Dry-Run-Protection-and-Logging-Unit
ESP32-based embedded system for real-time water pump dry-run protection using flow sensing, automatic fault shutdown, and time-stamped operational data logging.

## Problem

Water pump dry-run occurs when a pump operates without sufficient water flow. Continued operation under this condition can cause overheating, mechanical wear, seal damage, and eventual pump failure.

Low-cost pumping systems often lack automatic protection against dry-run conditions. This project addresses the problem by continuously monitoring actual water flow and automatically disconnecting the pump when a fault condition is confirmed.

## System Overview

The system uses an ESP32 as the main controller to monitor water flow through a YF-S401 Hall-effect flow sensor. The flow sensor generates pulses based on the water movement, which are counted by the ESP32 and used to determine the current flow condition.

The ESP32 controls the pump through a relay and provides local status information through an SSD1306 OLED display. A DS3231 RTC provides time information, while a microSD card stores operational data for later analysis.

<img width="501" height="655" alt="image" src="https://github.com/user-attachments/assets/96cdb6a2-c4bb-489f-b441-4b5b83715a19" />
