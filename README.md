# Multitech-mDot-Test-Message
C++ code that runs on a Multitech mDot development board

Usage:

Go to http://developer.mbed.org, and click on Compiler (create an account if you don't already have one). Click 'New', and use the mDot_LoRa_Sleep_Example template.

It's important that you don't tick the 'Update this program and libraries to latest revision box'. At the time I worked on it, it was bugged, it could be fixed now.

Open main.cpp, remove all code, and paste the main.cpp from this repository in it. Then compile the program (click the button at the top).

Now insert the mDot development board, and copy the hex file to the corresponding USB drive.
