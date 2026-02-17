# Esp32-cam project for VMKS

This project is a simple smart watching system built with an ESP32-CAM, a PIR motion sensor, and battery power.

Features:

 - Live video streaming from the ESP32-CAM

 - Web interface to view the camera stream

 - Motion detection using a PIR sensor

 - Push notification via Pushover when movement is detected

  - Photo captured and sent with the notification

  - Battery powered (3.3V)

  - Optional 5V amplifier support


How It Works:

  - The ESP32-CAM connects to Wi-Fi and starts a web server

  - You can access the live camera stream from a website

  - The PIR sensor monitors for movement

  - When motion is detected:

  - A photo is captured

  - A Pushover notification is sent with the image

Hardware Used:

  - ESP32-CAM

  - PIR motion sensor

  - 3.3V battery

  - 5V amplifier


Configuration:

  - Before uploading the code, set:

      * Wi-Fi credentials

      * Pushover User Key

      * Pushover API Token

      * PIR sensor pin

Power:

  - ESP32-CAM runs on 3.3V

  - Ensure correct voltage regulation if using 5V components

Use Cases:

  - Home or office monitoring

  - Door or entrance surveillance

  - Remote camera watching
