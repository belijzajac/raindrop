# raindrop

![raindrop](/assets/logo.png)

## Description

raindrop is a small program written in C++14 to obtain weather forecast of up to 10 days straight into your CLI. It was intended to be stylish and run alongside with neofetch (or screenfetch)

## Screenshots

![raindrop-screenshot-1](/assets/screenshot_1.png)
![raindrop-screenshot-2](/assets/screenshot_2.png)

## Dependencies

* curl >= 7.61.0

For Arch Linux, the following should be enough:

	$ sudo pacman -S curl
		
## Get WorldWeatherOnline API key

To get the WorldWeatherOnline API key, you must create an account at

> https://developer.worldweatheronline.com/auth/register

## Configure raindrop

1. Edit <b>config.txt</b> file with your WorldWeatherOnline API key and a city of choice

2. Create a directory <b>.raindrop</b> in <b>/home/your-username/</b>

3. Move <b>config.txt</b> to <b>/home/your-username/.raindrop/</b>

## Building and running

1. To build the program, simply type:

```
$ make
```
	
2. Move the built executable file to <b>/usr/local/bin/</b> by typing
```
$ sudo cp raindrop /usr/local/bin
```
3. Run the program by typing
```
$ raindrop
```
