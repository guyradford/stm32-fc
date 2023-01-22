# STM32 - FC
STM32 based Quadcopter/Drone Flight Controller
___
**This project is work in progress and not complete! You have been warned :)** 

Over the coming months I will be building out the software required to pilot a quadcopter. I will endeavour to keep the software as tidy as possible and to list the components I am using and link out to documentation that I find helpful.

If you have any questions please reach out, this is only a hobby project, so please don't expect me to do it for you ;) But we can all learn from each other :)

Thank you

Guy
___

# Quadcopter 
* [Frame](docs/Frame.md)
* [Hardware](docs/Hardware.md)
* [Software](docs/Software.md)
* [Interfaces](docs/Interfaces.md)


___

# FC Operation

The FC has two modes:
1. Setup/Calibration - Used to program the ESC's
2. Normal Running mode

## Setup and Calibration Mode

To Enter Setup and Calibration mode, press and hold the "User Button" whilst turning on the power. If this switch is pressed at startup the software will enter Setup mode.

## Normal Running

1. 5 Seconds pause (LED: TBC)
2. Calibration mode (20 seconds, LED: Fast Red/Green)
   * Do not move the Quadcopter
   * Push both controller sticks to each corner for around 4 seconds to calibrate:
     * Top Left
     * Top right
     * Bottom Right
     * Bottom Left
     * Repeat until calibration mode complete.
3. 5 Seconds pause (LED: TBC)
4. Normal Running Pending Arming (LED: Slow Flash Green)
   * To Arm ensure the input_throttle is at Zero and Yaw fully left, LED  will turn solid green.
5. Fly and be merry or Crash and be sad :(
6. To de-arm:
   * Ensure Quadcopter is on the ground ;)
   * Throttle to Zero and Yaw fully right
   * LED will slow flash Green
7. Power off or back to 4.
   

___
# TODO

* Altitude does not appear to be working.
* ~~Red/Green LED Module (GPIO)~~
* LiPo Fuel gauge (ADC)
* ESC Output (Timers/PWM/DMA)
* 