ArduinoCNC
==========

CNC controller based on Arduino UNO

This was created in 2011 as an final project for Embedded Development-course (Sulautetut Prosessorijärjestelmät) in University of Turku

Hardware:
  - 2 x 4-wire step motors
  - 2 x L298 H-bridges
  - 4094 8-Bit Shift Register/Latch with 3-STATE Outputs
  - Arduino UNO

Software:
  - cnc.c implements controller behaviour, handling H-bridges and receiving commands
  - cnc.py implements UI methods to send commands over serial

Demo: http://youtu.be/KV8qeGFaG2A
