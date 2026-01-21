# Obstacle-Avoidance Robot with Dual Control Modes

## Overview

This project is a **battery-powered, two-wheel differential drive robot** (rear motors + front caster) built around an **Arduino Uno**. The robot supports **two operating modes**, selectable via a physical switch:

1. **Autonomous Mode** – The robot drives forward on its own and avoids obstacles using ultrasonic sensors.
2. **Arcade (Bluetooth) Mode** – The robot is controlled like an RC car via a smartphone app (HC-05 Bluetooth), with ultrasonic sensors providing collision-avoidance overrides.

The entire system runs from **one battery pack** and uses a **soft-start / acceleration system** for smooth, reliable motor control.

---

## Key Features

* Single battery powers logic and motors
* Physical mode switch (safe default using internal pull-up)
* HC-05 Bluetooth joystick control
* Autonomous obstacle avoidance
* Front ultrasonic sensor mounted on a panning servo (180° scan)
* Rear ultrasonic sensor for reverse protection
* Soft-start / acceleration and smooth deceleration
* Safety overrides in both modes
* Differential drive with SN754410 H-bridge

---

## Hardware Components

* **Arduino Uno**
* **SN754410** dual H-bridge motor driver
* **2× DC motors** (rear drive wheels)
* **1× caster wheel** (front)
* **HC-05 Bluetooth module**
* **2× ultrasonic distance sensors** (HC-SR04 or equivalent)

  * Front sensor mounted on a servo
  * Rear sensor fixed
* **1× servo motor** (for front sensor panning)
* **SPST switch** (mode selection)
* **6× AA battery pack** (alkaline or NiMH recommended)
* Decoupling / bulk capacitors (recommended)

---

## Power Architecture

* Battery pack (~7–9 V) feeds:

  * **Arduino VIN**
  * **SN754410 motor supply (VCC2)**
* Arduino 5 V rail powers logic, Bluetooth, servo, and sensors
* **Common ground shared across all components**

> Motors are powered through the H-bridge only. The Arduino does not power motors directly.

---

## Motor Control Design

* **Direction control**: SN754410 input pins (1A/2A, 3A/4A)
* **Speed control**: PWM on enable pins (1,2EN and 3,4EN)
* **Soft-start system**:

  * Target motor speeds are ramped to actual PWM values
  * Reduces current spikes, wheel slip, and brownouts

---

## Operating Modes

### Autonomous Mode

* Default when mode switch is active
* Robot moves forward until an obstacle is detected
* Front sensor scans left/right using the servo
* Robot turns toward the direction with more free space
* Rear sensor prevents collisions when backing up

### Arcade (Bluetooth) Mode

* Controlled via smartphone joystick app
* Joystick values are mapped directly to motor speeds
* Ultrasonic sensors remain active as safety overrides:

  * Blocks forward motion if an obstacle is too close
  * Blocks reverse motion if rear obstacle is detected

---

## Mode Selection

* Physical switch connected to an Arduino digital pin
* Uses **internal pull-up resistor** (`INPUT_PULLUP`)
* Logic:

  * **HIGH (default)** → Arcade Mode (safe)
  * **LOW (switch closed)** → Autonomous Mode

This ensures predictable behavior at power-up and during resets.

---

## Software Architecture

* Single Arduino sketch
* Clear separation of responsibilities:

  * Bluetooth input handling
  * Ultrasonic sensing
  * Motor direction control
  * Motor speed ramping
  * Mode logic
* Target-speed vs current-speed model for acceleration control

All pin numbers are defined as constants and can be reassigned easily.

---

## Safety & Reliability Features

* Soft-start and smooth deceleration
* Immediate emergency stop on obstacle detection
* No floating inputs (internal pull-ups used)
* Noise-tolerant design for motor-driven systems
* Stable default behavior on power-up

---

## Notes & Limitations

* SN754410 is functional but inefficient and can run warm under load
* No motor encoders (open-loop speed control)
* Bluetooth control depends on external app joystick mapping

---

## Possible Extensions

* Direction-change dead-time protection
* Bluetooth disconnect failsafe
* Battery voltage monitoring
* PID speed control with encoders
* Upgrade to a modern motor driver (TB6612FNG / DRV8833)

---

## Summary

This project demonstrates a **realistic mobile-robot control architecture**, combining manual and autonomous operation, sensor-based safety, and proper motor control practices. The design emphasizes robustness, simplicity, and good engineering fundamentals suitable for learning, prototyping, and further expansion.
