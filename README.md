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
  - D1: Serial
  - D2: Input Multiplexer Selector A0
  - D3: Input Multiplexer Selector A1
  - D4: Input Multiplexer Selector A2
  - D5: Input Multiplexer Selector A3
  - D6: Input Multiplexer Output
  - D7: Motor Demultiplexer D1
  - D8: Motor Demultiplexer D2
  - D9: Motor Demultiplexer D3
  - D10: Motor Demultiplexer D4
  - D11: Motor Demultiplexer select (IN1, IN2, IN3 and IN4)
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
  - S13: Turntable speed sensor
  - S14: Display calibration value on 7-segment display
  - S15: Auto/Manual speed switch
  - S16: Target turntable speed selector

- Motor Demultiplexer
  - OUT S1A: Vertical stepper motor pin 1
  - OUT S1B: Horizontal stepper motor pin 1
  - OUT S2A: Vertical stepper motor pin 2
  - OUT S2B: Horizontal stepper motor pin 2
  - OUT S3A: Vertical stepper motor pin 3
  - OUT S3B: Horizontal stepper motor pin 3
  - OUT S4A: Vertical stepper motor pin 4
  - OUR S4B: Horizontal stepper motor pin 4

# Parts list (so far)
## Electrical parts (see BOM in Design/electrical for PCB parts list)
- Arduino Nano Every
- Mean well RS-15-5 5V 3A power supply
- 22 AWG Wire
- 1x ADA2776 5v solenoid
- 2x 28BYJ-48 stepper motors
- 2x ULN2003 stepper motor drivers
- slotted optical sensors (7x for full turntable, 5x for only 7")
- 2x micro limit switches
- breadboards (as many as you need)
- 2x LED lights (any color; these are to indicate movement and pause)
- 26 AWG stranded wire
- 3x blue cherry mx Mechanical Keyswitch
- 1x HYEJET-01 motor (uses M2 screws)
- 2x 1N4007 diodes
- 2x TIP120 transistors
- 1x MUX36S16DA SOIC 16-channel digital multiplexer
- 1x ADG333AB quad 2-channel analog demultiplexer
- [1x 0.056" 7-segment display w/ "backpack"](https://www.adafruit.com/product/879)
- 10k panel-mount linear potentiometer (3x for regular size turntable, 1x for only 7") ([P160KN2-0QA25B10K](https://www.digikey.com/en/products/detail/tt-electronics-bi/P160KN2-0QA25B10K/5957459))

## Mechanical parts
- 1x 9/32" diameter steel rod, between 4 and 5" long
- 1x 3/16" diameter steel rod, 5" long
- 1x 3/16" steel rods, 4" long
- 1x 1.8"x1.8" key stock, ~3" long
- many screws; 4-40 threading, 0.183" diameter head, 1/4" long
- a few more screws; 4-40 threading, 0.183" diameter head, 3/8" long
- many square nuts; 4-40 threading, 1/4" diameter, 3/32" thick
- washers; 18-8 Stainless Steel Washer for Number 10 Screw Size, 0.203" ID, 0.438" OD
- yet more screws; 2-56 threading, 1/2" long
- 5x5mm square nuts; 2-56 threading
- washers for no. 2 screw size, 0.094" inner diameter
- 3x 608RS ball bearings
- For turntable motor
  - 6x M2.5 screws; 8mm thread length
  - 6x felt cushioning washers (0.138" ID, 0.250" OD, 0.188" thickness (can be trimmed))
  - 12x felt cushioning washers (0.250" ID, 0.500" OD (can be trimmed), 0.25" (can be trimmed))

## Miscellaneous parts
- 10x10mm heat sinks
- Thermal paste
- Turntable belt (19cm folded length)
- RCA breakout connectors
- Any turntable cartridge that fits a standard headshell
- Turntable cartridge cable leads
- Tinted plexiglass (2370)
