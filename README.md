# 4-Floor Elevator Simulation on Proteus

## Overview
This is a fully working simulation of a 4-floor elevator built in Proteus. It uses an Arduino (ATmega328P) to control everything. It works exactly like a real elevator: it remembers button presses, picks people up on the way, and has a safety sensor to keep the doors from closing on anyone.


## Features
**Smart Memory:** The lift remembers all the buttons pressed. It finishes picking everyone up in one direction before turning around to go the other way.
**Door Safety Sensor:** If someone is blocking the door, an IR sensor catches it. The screen will say "DOOR BLOCKED!" and the motor will refuse to move until the person steps out of the way.
**Custom LCD Screen:** A 16x2 screen shows the current floor, whether the doors are open or closed, and uses custom-drawn arrows to show the direction it is moving.
**DC Motor:** A DC motor is simulated using an L293D driver to show how the mechanical part of the lift actually pulls the cabin up and down.
**Smart Wiring:** It uses an I2C module for the screen. This saves a lot of Arduino pins so there is enough room to wire all the inside and outside elevator buttons.

## Folders in this Repository
* `/src/` - This folder contains the raw Arduino code (`.ino` file).
* `/project_files/` - This folder contains the Proteus simulation file and the `.hex` file needed to run it.
* `report.pdf` - A short 2-page report explaining how I built and designed this project.
* `readme.md` - The instruction file you are reading right now!

## How to Run the Simulation
1. Open the Proteus project file located inside the `/project_files/` folder.
2. Right-click the **Arduino (ATmega328P)** chip on the schematic screen.
3. In the "Program File" box, click the folder icon and select the `.hex` file (also located in the `/project_files/` folder).
4. Click the **Play** button at the bottom left of the Proteus window.
5. **To test the safety sensor:** Press a floor button and wait for the lift to arrive. When the screen says `**DOORS OPEN**`, click the logic toggle on Pin 12 to change it from `1` to `0`. The screen will warn you that the door is blocked, and the elevator will wait for you to change it back to `1` before moving again!
