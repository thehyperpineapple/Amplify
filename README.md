# Amplify
### IoT Current Monitoring and Visualization System

This is an IoT project designed to monitor the current flowing through a 3-phase motor using an ESP32 development board and current transformers. The project's primary goal is to gather real-time current data through an edge node and transmit it a master node using LoRa. The master node will then upload to ThingSpeak, an IoT analytics platform, for further analysis and visualization. Additionally, A web application built with Python (Flask) allows users to conveniently monitor and visualize the current data collected from the motor.

## Features

- Real-time Current Monitoring: Amplify utilizes current transformers and an ESP32 board to accurately measure and monitor the current flowing through a 3-phase motor.

- ThingSpeak Integration: The collected current data is sent to ThingSpeak, where it can be stored, analyzed, and visualized in the form of charts and graphs.

- User-friendly Web App: The Python Flask-based web application provides an intuitive interface for users to view real-time and historical current data, allowing for easy tracking of the motor's performance.


## Hardware Requirements

Following hardware components are required

- ESP32 Development Board
- Current Transformers (3 for each phase)
- LoRa Module
- SD Card
- OLED Display (128 x 64 px)
- Wifi 

## Software Requirements

- Arduino IDE (ESP32 Programming)
- Python
- Flask
- ThingSpeak Account