# Automatic Turntable Introduction
This is a semi-automatic turntable that plays 7" records! The automatic capabilities will be powered by an Arduino Uno, 2 stepper motors, and a bunch of sensors.
Some planned features of this turntable include:
- Stereo RCA outputs for a receiver
- Able to fit any commercial cartridge
- 45 and 33-RPM speeds
  - Speed monitored on 7-segment display
- Semi-automatic
  - Returns tonearm to "home" position automatically after a record is finished
  - Other buttons that contain other pre-defined routines (play and pause) that the user must initiate
- Standard PC 3-prong female plug in the back to allow hookup to 120v or 230v households

See a prototype video of this working [here](https://youtu.be/Y9oQI6XiWVw)

# Inputs and routines
The user has a total of five inputs they can use. Most of these functions must be initiated by the user by either pressing a button or flipping a switch, though homing can also be done automatically, which will be explained in more detail later on. Routine interrupt is currently not planned. This means that while one routine is running, none of the others can be executed for the duration of the currently-running routine.

## Automatic/manual switch
This is a 3-way switch with the center position being "off." Flipping the switch to the "up" position will set the turntable to automatic, while "down" will set it to manual. The turntable will automatically be homed upon flipping the switch to "automatic." Flipping it to "manual" will home the vertical axis, which will set the tonearm down in place where it currently is. The reason for this inclusion is to account for us not knowing what position the tonearm will be in when the device is turned on.

As soon as this switch is flipped to "manual" or "automatic," the software setup procedure begins.

## Play/Home button
The "play/home" button will pick the tonearm up from any point, and either drop it at the beginning of a record, or at the home position. If the tonearm is past the play position (i.e. on a record), then the button will home the tonearm. Otherwise, the button will drop it at the beginning of a record. These positions are determined using a slotted optical sensor.

## Pause button
The pause button will lift the tonearm up until the pause limit switch becomes "high." When the pause button is pressed again, the tonearm will be gently set down on the record.

## Speed Selector
Description to come. 

## Display play calibration
Description to come. 

# Current pin usage
- Digital
  - D0: Serial
  - D1: Speed sensor (interrupt)
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
  - A3: 33-RPM potentiometer adjustment motor
  - A4/SDA: 7-Segment Display Data
  - A5/SCL: 7-Segment Display Clock
  - A6: 45-RPM potentiometer adjustment motor
  - A7: Horizontal gearing solenoid

- Input Multiplexer
  - S1: Vertical lower limit
  - S2: Vertical upper limit
  - S3: Play/home button
  - S4: Pause button
  - S5: Horizontal "play" 7" optical sensor
  - S6: Horizontal "play" 10" optical sensor
  - S7: Horizontal "play" 12" optical sensor
  - S8: Horizontal "home" optical sensor
  - S9: Record Size Selector 1
  - S10: Record Size Selector 2
  - S11: Horizontal "pickup" optical sensor
  - S12: Auto/Manual mode switch
  - S13: UNUSED
  - S14: Display calibration value on 7-segment display
  - S15: Auto/Manual speed switch
  - S16: Target turntable speed selector

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