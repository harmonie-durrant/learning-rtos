# Learning RTOS

In this repo I am trying to learn more about RTOS, as I go I will be documenting chllenges with both software and hardware.

# Blinky

Blinky is the name of a popular program beginers do, just like the "Hello world" program. It consists of flashing an LED on and off. In this example I have the onboard led of my esp32 that I simply turn on and off.

# Temperature

In this script I lernt how to easily connect the esp32 to my home network, and how to read the current temperature/humitity from a DHT sensor.

I plan to use the network connectivity to show the temperature on a simple callable api.

# Home Assistant - LED

In this project I connect the esp32 to Home Assistant using MQTT to control it's internal LED just like a smart light.

# Home Assistant - Temperature

In this project I expand on my **Home Assistant - LED** project and send the readings of my DHT sensor to Home Assistant.

# Home Assistant - motion-light

In this project I expand on my **Home Assistant - LED** project and use a PIR sensor to turn on the internal LED of the esp32 when motion is detected. I also have an override switch to ignore the sensor and turn the light on and off manually through Home Assistant.
