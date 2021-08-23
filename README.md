# Automatic Turntable Introduction
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

# Layout
The layout is yet to be designed, but a very general prototype can be seen in Figure 1.

![image](https://cdn.discordapp.com/attachments/625801308854812684/848020821326037032/20210528_221130.jpg)
Figure 1. Prototype layout of the turntable.

# Inputs and routines
The user has a total of four inputs they can use, each triggering their own functions. Most of these functions must be initiated by the user by either pressing a button or flipping a switch, though homing can also be done automatically, which will be explained in more detail later on. Routine interrupt is currently not planned. This means that while one routine is running, none of the others can be executed for the duration of the currently-running routine.

## Automatic/manual switch
This is a 3-way switch with the center position being "off." Flipping the switch to the "up" position will set the turntable to automatic, while "down" will set it to manual. The turntable will automatically be homed upon flipping the switch to "automatic." Flipping it to "manual" will home the vertical axis, which will set the tonearm down in place where it currently is. The reason for this inclusion is to account for us not knowing what position the tonearm will be in when the device is turned on.

As soon as this switch is flipped to "manual" or "automatic," the software setup procedure begins.

## Play button
The "play" button will pick the tonearm up from any point, and drop it at the beginning of a record. This will work using a slotted optical sensor that denotes the "play" position. If either the pick-up or play sensors are "high," then the tonearm will move left to find the edge of the record, otherwise it will move right.

## Pause button
The pause button will lift the tonearm up until the pause limit switch becomes "high." When the pause button is pressed again, the tonearm will be gently set down on the record.

## Home button
The "home" button will pick the tonearm up from any point, and drop it in its home position. This routine is also automatically called at the end of a record, when the "pick-up" sensor is set "high" and the turntable is set to "automatic."

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
  - 9: Horizontal gearing solenoid
  - 10: Play button
  - 11: Movement status LED
  - 12: Pause button
  - 13: Pause status LED

- Analog
  - A0: Vertical upper (pause) sensor
  - A1: Vertical stepper lower (home) limit
  - A2: Horizontal home limit switch
  - A3: Horizontal "play" optical sensor (technically could be removed in favor of step count)
  - A4: Horizontal "pick up" optical sensor
  - A5: On - Auto/Manual

Note: Due to limitations, I had to use some of the analog lines for digital values.

# Parts list (so far)
## Electrical parts
- Arduino Uno
- Mean well RS-15-5 5V 3A power supply
- 1x ADA2776 5v solenoid
- 2x 28BYJ-48 stepper motors
- 2x ULN2003 stepper motor drivers
- 4x slotted optical sensors
- 2x micro limit switches
- breadboards (as many as you need)
- 2x LED lights (any color; these are to indicate movement and pause)
- 26 AWG stranded wire
- 3x blue cherry mx Mechanical Keyswitch
- 1x HYEJET-01 motor
- 2x 1N4007 diodes
- 2x TIP120 transistors

## Mechanical parts
- 1x 3/16" steel rods, 5" long
- 1x 3/16" steel rods, 4" long
- 1x 3/16" steel rods, 3" long
- 4x 3/16" sleeve bearings
- many screws; 4-40 threading, 0.183" diameter head, 1/4" long
- a few more screws; 4-40 threading, 0.183" diameter head, 3/8" long
- many square nuts; 4-40 threading, 1/4" diameter, 3/32" thick
- washers; 18-8 Stainless Steel Washer for Number 10 Screw Size, 0.203" ID, 0.438" OD
- yet more screws; 2-56 threading, 1/2" long
- 5x5mm square nuts; 2-56 threading
- washers for no. 2 screw size, 0.094" inner diameter

## Miscellaneous parts
- 10x10mm heat sinks
- Thermal paste
