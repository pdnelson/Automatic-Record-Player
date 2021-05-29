# Automatic-Turntable
This is a semi-automatic turntable that plays 7" records! The automatic capabilities will be powered by an Arduino Uno, 2 stepper motors, and a bunch of sensors.
Some planned features of this turntable include:
- Stereo RCA outputs for a receiver
- Able to fit any commercial cartridge
- 45 and 33-RPM speeds
- Semi-automatic
  - Returns tonearm to "home" position automatically after a record is finished
  - Other buttons that contain other pre-defined routines (play and pause) that the user must initiate
- Standard PC 3-prong female plug in the back to allow hookup to 120v or 230v households

The actual "turntable" part is yet to be designed; right now, I'm just working on the automatic functionality.

# Inputs and routines
The user has a total of four inputs they can use, each triggering their own functions. Most of these functions must be initiated by the user by either pressing a button or flipping a switch, though homing can also be done automatically, which will be explained in more detail later on.

## Automatic/manual switch
This is a 3-way switch with the center position being "off." Flipping the switch to the "up" position will set the turntable to automatic, while "down" will set it to manual. The turntable will automatically be homed upon flipping the switch to "automatic." Flipping it to "manual" will home the vertical axis, which will set the tonearm down in place where it currently is. The reason for this inclusion is to account for us not knowing what position the tonearm will be in when the device is turned on.

As soon as this switch is flipped to "manual" or "automatic," the software setup procedure begins, which can be seen in Figure 1.

![image](https://user-images.githubusercontent.com/48131480/120053935-06665b80-bffb-11eb-8e6d-85e997d80409.png)

Figure 1. Flow diagram of the setup procedure

## Play button
## Pause button
## Home button

Automatic or manual switch

# Current pin usage
- Digital
  - 0: Vertical stepper motor pin 1
  - 1: Vertical stepper motor pin 2
  - 2: Vertical stepper motor pin 3
  - 3: Vertical stepper motor pin 4
  - 4: Horizontal stepper motor pin 1
  - 5: Horizontal stepper motor pin 2
  - 6: Horizontal stepper motor pin 3
  - 7: Horizontal stepper motor pin 4
  - 8: Home button
  - 9: Home status LED
  - 10: Play button
  - 11: Play status LED
  - 12: Pause button
  - 13: Pause status LED

- Analog
  - A0: Vertical stepper upper limit
  - A1: Vertical stepper lower limit
  - A2: Horizontal home limit switch
  - A3: Horizontal "play" optical sensor (technically could be removed in favor of step count)
  - A4: Horizontal "pick up" optical sensor
  - A5: On - Auto/Manual

Note: Due to limitations, I had to use some of the analog lines for digital values.

# Parts list (so far)
- Arduino Uno
- Mean well RS-15-5 5V 3A power supply
- 2x 28BYJ-48 stepper motors
- 2x ULN2003 stepper motor drivers
- 4x slotted optical sensors
- 1x limit switche (I got them out of some computer mice...)
- 2x 3/16" steel rods, 4" long
- 4x 3/16" sleeve bearings
- 1x 12x13mm butt hinge
