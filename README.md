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

As soon as this switch is flipped to "manual" or "automatic," the software setup procedure begins, which can be seen in Figure 2.

![image](https://user-images.githubusercontent.com/48131480/120053935-06665b80-bffb-11eb-8e6d-85e997d80409.png)

Figure 2. Flow diagram of the setup procedure

## Play button
The "play" button will pick the tonearm up from any point, and drop it at the beginning of a record. This will work using a slotted optical sensor that denotes the "play" position. If either the pick-up or play sensors are "high," then the tonearm will move left to find the edge of the record, otherwise it will move right.

The play routine, defined in the software, can be seen in Figure X.

## Pause button
The pause button will lift the tonearm up until the pause sensor becomes "low." When the pause button is pressed again, the tonearm will be gently set down on the record.

The pause routine, defined in the software, can be seen in Figure X.

## Home button
The "home" button will pick the tonearm up from any point, and drop it in its home position. This routine is also automatically called at the end of a record, when the "pick-up" sensor is set "high" and the turntable is set to "automatic."

The home routine, defined in the software, can be seen in Figure X.

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
- 2x 28BYJ-48 stepper motors
- 2x ULN2003 stepper motor drivers
- 3x slotted optical sensors
- 2x limit switches (I got them out of some computer mice...)

## Mechanical parts
- 2x 3/16" steel rods, 4" long
- 1x 3/16" steel rods, 3" long
- 4x 3/16" sleeve bearings
- many screws; (part # 91251A055 on McMaster)
  - ![image](https://user-images.githubusercontent.com/48131480/121616639-4254e400-ca31-11eb-9ad7-fcec2605ea7d.png)
- many nuts; 0-80, 5/32" wd., 3/64" ht., (part # 	90480A001 on McMaster)

## Miscellaneous parts
- 10x10mm heat sinks
- 16x4.5mm fan
- Thermal paste
- 3x OUTEMU (Gaote) Brown Switch 3 Pin Mechanical Keyswitch 
