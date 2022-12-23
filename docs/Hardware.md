
# Controller Hardware
## STM32 Nucleo - STM32L152RE
![STM32 Nucleo - STM32L152RE](https://m.media-amazon.com/images/I/51y0k1NVu6L._AC_SX425_.jpg)

* [Link to STM32 site for STM32L152RE](https://www.st.com/en/evaluation-tools/nucleo-l152re.html)
* [MBED site, great for pin out diagrams](https://os.mbed.com/platforms/ST-Nucleo-L152RE/)

This is only a 32MHz processor, I am hoping it will be enough, if not i'll need to upgrade, I have seen others use an F4 72MHz version.

# IMU - Inertial Measurement Unit

This device allows us to track the movement of the quadcopter in space allowing us to always know it's pitch, roll and yaw from an initial starting point.
This is important as it allows the software to compensate or even control the quadcopter.

## Waveshare 10 DOF IMU Sensor (D)

![10 DOF IMU Sensor (D)](docs/images/10-dof-imu-sensor-d-2.jpg)
* [Development Resources](https://www.waveshare.com/wiki/10_DOF_IMU_Sensor_(D))

Note: I think there is now a newer version of this sensor available!

This device has an I2C interface making it very easy to read the values, also included in the development resources is an IMU faction.
I have currently chosen to use the Waveshare IMU faction as it appear better than the version I was intending to write.
However, I suggest watching some YouTube videos to understand how it works and its limitations.

# RC Receiver


# Reg/Green LED Status Module

![KY-011 Two Colour Red and Green LED](docs/images/KY-011%20Two%20Colour%20Red%20and%20Green%20LED.png)
