# raindrop

![raindrop](/assets/logo.png)

## Description

> To be written...

## Screenshots

![raindrop-screenshot-1](/assets/screenshot_1.png)
![raindrop-screenshot-2](/assets/screenshot_2.png)

## Get WorldWeatherOnline API key

To get the WorldWeatherOnline API key, you must create an account at

> https://developer.worldweatheronline.com/auth/register

## Configure raindrop

Edit <b>config.txt</b> file with your WorldWeatherOnline API key and a city of choice

## Dependencies

* curl >= 7.61.0

For Arch Linux, the following should be enough:

	$ sudo pacman -S curl
		
## Building and running

To build and run the program, simply type:

	$ make && make run
or

	$ make && ./raindrop
