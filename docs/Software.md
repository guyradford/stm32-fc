# Software Layout
I tend to separate my code into layers, for me there are three layers and the STM32 help keep this tidy.

**Layer 1 - HAL**: This is the Hardware Abstraction Layer, it uses the libraries provided by ST. It also sets up how the various interfaces are configured. What this layer is not responsible for is what is connection to the interface.

**Layer 2 - Hardware Layer:** This layer is concerned with the connected hardware and how it needs to be interfaced with.

**Layer 3 - Application:** This layer is the business logic, it does not care how to turn an LED on or off, only that the LED has to be turned on or off!

For Example, lets stick with a Green and Red status LED for this example.

**Layer 1:** Defines the GPIO Port and Pins for both the Red and Green LED
```c
#define LED_GREEN_Pin GPIO_PIN_5
#define LED_GREEN_GPIO_Port GPIOA
```
**Layer 2:** This layer has methods turn the LED(s) on or off. The exact way you use this later will depend on your use case.
```c
void HardwareLayer_LED_Green_On(void);
void HardwareLayer_LED_Green_Off(void);

# or
void HardwareLayer_LED_Green(bool state);

#or
void HardwareLayer_LED_State(ledEnum led, bool state);
```
**Layer 3:** This last layer now only has to call a nicely wrapped function, thus keeping you code much more readable.
