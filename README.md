# Automatic Turntable Introduction
This project is currently under construction separately from this repository. This repository is now obsolete, and will be left in a non-functional state.

This is a fully automatic turntable that plays 7", 10" or 12" records! The automatic capabilities are powered by an Arduino Nano Every, 2 stepper motors, and a bunch of sensors.
Some notes on this turntable include (see "Features, User Inputs and Routines" section for more details):
- Stereo RCA outputs for a receiver
  - Includes Ground line
- Able to fit any commercial cartridge
- 16 2/3, 33 1/3, 45 and 78-RPM speeds
  - Digital or analog speed monitoring on 7-segment display or strobe disc, respectively
- Automatic functionality
  - Returns tonearm to "home" position automatically after a record is finished
  - Other buttons that contain other pre-defined routines (play and pause) that the user must initiate
  - Output port to allow another device to execute commands and read statuses
- Standard PC 3-prong female plug in the back to allow hookup to 120v or 230v households
- Counterbalance weight to fine-tune tonearm weight
- Skate/Anti-skate weight to fine-tune horizontal drift

# Demo Videos

I went over some information of this version of the turntable [in this video](https://www.youtube.com/watch?v=k4UXI1rkMYs). Further updates will be referenced on that channel.

# Features, User Inputs and Routines
The turntable has several inputs the user can use. Most of these functions must be initiated by the user by either pressing a button or flipping a switch, though homing can also be done automatically, which will be explained in more detail later on. Routine interrupt is currently not planned. This means that while one routine is running, none of the others can be executed for the duration of the currently-running routine.

## Automatic/manual switch
This is a 3-position switch with the center position being "off." Flipping the switch to the "up" position will set the turntable to automatic, while "down" will set it to manual. The turntable will automatically be homed upon flipping the switch to "automatic." Flipping it to "manual" will home the vertical axis, which will set the tonearm down in place where it currently is. The reason for this inclusion is to account for us not knowing what position the tonearm will be in when the device is turned on.

As soon as this switch is flipped to "manual" or "automatic," the software setup procedure begins.

## Play/Home button
The "play/home" button will pick the tonearm up from any point, and either drop it at the beginning of a record, or at the home position. If the tonearm is past the play position (i.e. on a record), then the button will home the tonearm. Otherwise, the button will drop it at the beginning of a record. These positions are determined using a slotted optical sensor.

## Pause button
The pause button will lift the tonearm up until the pause limit switch becomes "high." 

When the pause button is pressed again, if the tonearm is directly above "home" position, it will be set fown quickly until the lower limit switch becomes "high." Otherwise, if it is NOT over "home," it will be gently set down.

## Speed Selectors
There are two speed selector switches: The main selector switch, and the "alternate" speed button.

The main speed selector switch is a 3-position switch with the center position turning the turntable motor off, the lower position spinning it at 33 1/3-RPM, and the upper position spinning it at 45-RPM. When the "alternate" speed button is pressed, the switch's upper position will spin at 78-RPM, and the lower will spin at 16 2/3 RPM.

## Fine-Tune Speed Adjustment
On the right side of the record player, there are four potentiometers: One for each speed setting. This allows the user to perfectly dial in the speed they are looking to play at.

## Record size selector
The record size selector is a 3-position switch, with the top indicating 12", middle 10" and lower 7" records. This switch activates user-set calibration values for how many steps the tonearm should move past the play sensor to the edge of the record.

## Calibration Button
When the calibration button is pressed, the seven-segment display will display the active calibration value set by the record size selector. This value will indicate the number of steps taken once the tonearm moves past the play sensor during the play routine. In addition to showing the calibration value, this will also cause the pause button to increment the value, and the play button to decrement it. This can be used to fine-tune exact step calibrations, so the tonearm always lands on the correct spot of the record.

Releasing this button will save the setting to the Arduino's EEPROM. If the value is changed, saving will be indicated by the movement status LED flashing briefly.
