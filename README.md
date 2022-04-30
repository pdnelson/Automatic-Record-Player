# Automatic Turntable Introduction
This is a semi-automatic turntable that plays 7", 10" or 12" records! The automatic capabilities will be powered by an Arduino Nano Every, 2 stepper motors, and a bunch of sensors.
Some notes on this turntable include (see "Features, User Inputs and Routines" section for more details):
- Stereo RCA outputs for a receiver
  - Includes Ground line
- Able to fit any commercial cartridge
- 45 and 33-RPM speeds
  - Speed monitored on 7-segment display
- Semi-automatic
  - Returns tonearm to "home" position automatically after a record is finished
  - Other buttons that contain other pre-defined routines (play and pause) that the user must initiate
- Standard PC 3-prong female plug in the back to allow hookup to 120v or 230v households
- Counterbalance weight to fine-tune tonearm weight
- Skate/Anti-skate weight to fine-tune horizontal drift

# Demo Videos

A prototype can be seen [here](https://www.youtube.com/watch?v=AN1vcnQAMdQ). Please note that this prototype is outdated, and is NOT a comprehensive/full representation of all the features. All features present on this page are present on that prototype, with the exception of skate/anti-skate adjustment.

A direct audio capture demo can be seen [here](https://youtu.be/0yNxp464KpQ). Please note that "humming" heard in the background is from the stereo receiver I recorded from, and not the record player. Pitch imperfections are also present in this recording for the following reasons: The grooves of this record were pressed slightly off-center, and the pulleys driving the turntable are 3D-printed, so there is a lot of wow/flutter. I plan to address this issue by replacing the 3D-printed pulley with one turned on out of brass on a lathe, and turning the turntable itself out of wood on a lathe.

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

## Speed Selector
The speed selector switch is a 3-position switch with the center position turning the turntable motor off, the lower position spinning it at 33-RPM, and the upper position spinning it at 45-RPM.

## Fine-Tune Speed Adjustment
On the right side of the record player, there are two potentiometers: One for adjusting the 33-RPM speed setting, and the other for adjusting the 45-RPM speed setting. This allows the user to perfectly dial in the speed they are looking to play at.

If desired, the 33-RPM speed setting can also be used to play at 16-RPM by adjusting the 33-RPM potentiometer.

## Active Play Sensor Selector
There are three different sensors that can tell the tonearm where to set down on the record: One for 7", 10" and 12". This switch selects which one you want to be used,  wh the lower position selecting 7", middle selecting 10", and upper selecting 12".

If switching between play positions is not necessary, this setting can be set with a jumper on JP1. Connect the leftmost two pins of this jumper for the 7" sensor, no jumper for 10", and the rightmost two pins for 12"

## Calibration Potentiometers
The calibration potentiometers will push a play movement X steps past the selected play sensor, which allows the user to find-tune where the tonearm is being set down on the record.

Only ONE of these calibration potentiometers is active at once, determined by the Active Play Sensor Selector switch described above. 

## Display play calibration
Display play calibration will display the numeric value that corresponds to the rear calibration potentiometer that is currently selected (between 0 and 50, inclusive).

The selected potentiometer is determined by the Active Play Sensor Selector.

# Current pin usage
- Digital
  - D0: UNUSED
  - D1: UNUSED
  - D2: Input Multiplexer Selector A0
  - D3: Input Multiplexer Selector A1
  - D4: Input Multiplexer Selector A2
  - D5: Input Multiplexer Selector A3
  - D6: Input Multiplexer Output
  - D7: Motor Demultiplexer U13 pin A
  - D8: Motor Demultiplexer U12 pin A
  - D9: Motor Demultiplexer U11 pin A
  - D10: Motor Demultiplexer U10 pin A
  - D11: Motor Demultiplexers select (U10, U11, U12 and U14, pin S)
  - D12: Movement status LED
  - D13: Pause status LED

- Analog
  - A0: 7" position potentiometer
  - A1: 10" position potentiometer
  - A2: 12" position potentiometer
  - A3: Speed sensor (interrupt)
  - A4/SDA: 7-Segment Display Data
  - A5/SCL: 7-Segment Display Clock
  - A6: UNUSED
  - A7: Horizontal gearing solenoid

- Input Multiplexer
  - S1: Vertical lower limit
  - S2: Vertical upper limit
  - S3: Play/home button
  - S4: Pause button
  - S5: UNUSED
  - S6: UNUSED
  - S7: UNUSED
  - S8: Horizontal "home"/"play" optical sensor
  - S9: Record Size Selector 1
  - S10: Record Size Selector 2
  - S11: Horizontal "pickup" optical sensor
  - S12: UNUSED
  - S13: UNUSED
  - S14: Display calibration value on 7-segment display
  - S15: UNUSED
  - S16: Auto/Manual mode switch

- Motor Demultiplexer U10
  - Y0: Vertical stepper motor pin 1
  - Y1: Horizontal stepper motor pin 1

- Motor Demultiplexer U11
  - Y0: Vertical stepper motor pin 2
  - Y1: Horizontal stepper motor pin 2

- Motor Demultiplexer U12
  - Y0: Vertical stepper motor pin 3
  - Y1: Horizontal stepper motor pin 3

- Motor Demultiplexer U13
  - Y0: Vertical stepper motor pin 4
  - Y1: Horizontal stepper motor pin 4
