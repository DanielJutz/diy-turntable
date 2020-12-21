# diy-turntable
A diy turntable using 3D printing and stepper motor control.

Hardware:<br/>
Nema17 Stepper motor<br/>
TMC2209 stepper driver<br/>
ATmega328 based Arduino<br/><br/>
Software:<br/>
arduino calculates crc checksum and sends serial messages to TMC2209. These configure the driver at power-up and set internal pulse generator speed.


![Alt text](turntable_render.png?raw=true "Title")
