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

![RobotAVR Front Photo](//szymonkatra.github.io/images/project/robotavr/photo_1.JPG)

Controls list and basic info:
* w - forward
* s - reverse
* a - turn left
* d - turn right
* x - change mode - bluetooth / sensor - default bluetooth
* + - speed up by 5 pulses per second (60 maximum)
* - - speed down by 5 pulses per second (10 minimum)
* m - change straight mode - left and right wheels works separately or jointly while driving straight - default separately
* c - calibrate while moving - yes / no - default no
* Default speed is 40 pulses per second