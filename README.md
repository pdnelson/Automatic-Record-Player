# Automatic-Turntable
This is heavily under construction as I continue to learn about the Arduio Uno and various electronics...

This will become a turntable that is controlled by stepper motors and sensors.

More details to come. I'm still in the process of planning out this design; this is just here so I can keep track of my code experimentation.

# (Planned) pin usage
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
  - 9: Play button
  - 10: Pause button
  - 11: Paused status LED
  - 12: Homing status LED
  - 13: Play status LED

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
- 2x 28BYJ-48 stepper motors
- 2x ULN2003 stepper motor drivers
- 2x slotted optical sensors
- 3x limit switches (I got them out of some computer mice...)
- 2x 3/16" steel rods, 4" long
- 4x 3/16" sleeve bearings
