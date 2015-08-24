# RobotAVR
Obstacle avoiding and remote controlled robot based on ATmega328P-PU. Circuit is built on prototype PCB boards.

Main parts:
* ATmega328P-PU
* HC-05 Bluetooth Module
* HC-SR04 Ultrasonic ranging module
* H-driver L293D
* 5V motors with 1:48 gear
* 65mm diameter wheels

This robot has currently two modes - remote controlling and obstacle avoiding.
Connect via bluetooth and control robot with 'w', 's', 'a', 'd' commands or 'x' to switch mode.

LFuse: F7
HFuse: D1
EFuse: 07

Fuse bit configuration included in Eclipse project.

![RobotAVR Front Photo](/photos/photo_1.JPG)