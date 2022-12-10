# STM32 - FC
STM32 based Quadcopter/Drone Flight Controller
___
This project is work in progress. 
Over the coming months I will be building out the software required to pilot a quadcopter. I will endeavour to keep the software as tidy as possible and to list the components I am using and link out to documentation that I find helpful.

If you have any questions please reach out, this is only a hobby project, so please don't expect me to do it for you ;) But we can all learn from each other :)

Thank you

Guy
___

# Hardware

## STM32 Nucleo - STM32L152RE 
![STM32 Nucleo - STM32L152RE](https://m.media-amazon.com/images/I/51y0k1NVu6L._AC_SX425_.jpg)

* [Link to STM32 site for STM32L152RE](https://www.st.com/en/evaluation-tools/nucleo-l152re.html)
* [MBED site, great for pin out diagrams](https://os.mbed.com/platforms/ST-Nucleo-L152RE/)

This is only a 32MHz processor, I am hoping it will be enough, if not i'll need to upgrade, I have seen others use an F4 72MHz version.

# IMU - Inertial Measurement Unit

This device allows us to track the movement of the quadcopter in space allowing us to always know it's pitch, roll and yaw from an initial starting point.
This is important as it allows the software to compensate or even control the quadcopter.

## Waveshare 10 DOF IMU Sensor (D)

![10 DOF IMU Sensor (D)](https://www.waveshare.com/media/catalog/product/cache/1/image/800x800/9df78eab33525d08d6e5fb8d27136e95/1/0/10-dof-imu-sensor-d-2.jpg =250x)
* [Development Resources](https://www.waveshare.com/wiki/10_DOF_IMU_Sensor_(D))

Note: I think there is now a newer version of this sensor available!

