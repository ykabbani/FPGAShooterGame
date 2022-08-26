# RacingGameFPGA

## Project Description

This project consisted of creating an arcade style racing game that runs on a terasIC DE10-Standard FPGA Development Kit powered by an ARM Cortex A9 processor. The project was done as a final project submission in the Microprocessors and Microcontrollers course as a part of my undergraduate studies. 

### DE-10 Standard Development Board
<img src="https://user-images.githubusercontent.com/26261828/186829660-f3b767ad-0bf0-43a8-a0ae-a07c9c03fa7d.png" width="400" />

[Manufacturer's link](https://www.terasic.com.tw/cgi-bin/page/archive.pl?Language=English&CategoryNo=165&No=1081 "DE10-Standard FPGA Board")

The game consists of a car, represented by a rectangle, which is cpntrolled by the game's operator using input from the onboard accelerometer sensor. The player starts the race with 10 HP, or health points. Opponent car units the player is racing against spawn from the top of the screen at random locations, which the player has to avoid. Each time an opponent is hit, the player's health points are reduced by 1. The game speeds up over time. The game is over when the player loses all their HP and if their score is a highscore, it is saved.

The game utilized the following on-board peripherals:
- **128x64 LCD:** main display for the game 
- **10x LEDs:** player's HP bar
- **7-segment displays:** highscore | current player score
- **ADXL345 Digital accelerometer (I2C):** player input 
- **Push buttons:** start/pause

The game also made use of 2 onboard timers and 1 interrupt:
- **Timer 1:** used for updating the LCD display
- **Timer 2:** used for reading the accelerometer data
- **Interrupt**: triggered when a push button is pressed

## Game Demo

Game Demo                  |
:-------------------------:|
<video src="https://user-images.githubusercontent.com/26261828/186837343-4388c2fe-bf2e-4956-8fc2-c2a78ef73741.mp4" width="250" /> |




