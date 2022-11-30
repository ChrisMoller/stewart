# stewart

An opengl-based simulation of a Stewart platform and hardware and software
designs for actual Arduino-based Stewart platforms.

# OpenGL Simulation

The Stewart OpenGL simulator is intended to provide both a means of visualising
the operation of a Stewart platform, as well as experimenting with necessary and
useful controls for the environment for which this implementation is being
designed, and a verification of an essential algorithm that derives the servo
controls necessary to achieve desired platform positions and orientations.

In use, the OpenGL shows a 2D perspective projection of a 3D model of a Stewart
platform and animates that projection in response to control inputs described
below.  In addition, that animation can be captured as a video file.

Options:

	-w --width	Set window width, required argument.
	-h --height	Set window heigh, required argumentt
	-r --record	Start video recording, optional file name,
	   		defaults to stewart.mp4
	-d --demo	Puts the application into demo mode.


Runtime controls:

	ctrl-d/ctrl-D	  Decreases/increases view-from distance
	ctrl-a/ctrl-A	  Decreases/increases view azinuth (longitude).
	ctrl-e/ctrl-E	  Decreases/increases view elevation (latitude).
	ctrl-z/ctrl-Z	  Decreases/increases lookat z-axis location
	ctrl-left arrow	  Moves the lookat x-axis location.
	ctrl-right arrow  Moves the lookat x-axis location.
	ctrl-up arrow	  Moves the lookat y-axis location.
	ctrl-down arrow   Moves the lookat y-axis location.
	ctrl-home 	  resets the lookat location to the origin.
	
	r/R		  Decreases/increases the platform roll rotation.
	p/P	  	  Decreases/increases the platform pitch rotation.
	y/Y	  	  Decreases/increases the platform yaw rotation.
	d/u	  	  Decreases/increases the platform elevation.
	left arrow	  Moves the platform left.
	right arrow	  Moves the platform right.
	up arrow	  Moves the platform up.
	down arrow	  Moves the platform down.
	home 		  Resets the platform to level and centred.
	
	m		  Toggles animationvideo capture.

# Hardware

There are at present two proposed hardware designs, respectively in the
hardware3, and hardware4 directories.  Design 3 includes a physical control
panel and has been abandoned in favour of the hardware4 design.

Hardware4 uses the Arduino MKR 1010 Wifi device and is controlled through
WiFi--it appears on the local network and is controlled through a
Javascript/HTML web page that the Arduino, acting as a web server, creates.
This is, of course, far more flexible than a hardware control panel.  That web
page is still under development.
	

	