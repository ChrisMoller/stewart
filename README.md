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
hardware and hardware2 directories.  These designs are functionally identical
except that the latter includes a small display screen showing things like
the platform position and orientation and jitter positional and rotational
parameters.  Both designs include both a hardware description, primarily in
the form of a diagram.json file, and the Arduino code, in sketch.ino files,
necessary to implement the necessary function.

The designs have idential controls, a pair of 3-axis joysticks, a pair of
buttons that determine the mode of the joysticks, and a Run/Pause button.

The joysticks operate in one of three modes.  With neither of the mode buttons
depressed, the x, y, and z axes of one of the joysticks controls the x, y, and z
jitter limits of the platform while the other joystick controls the pitch, roll,
and yaw limits.  Both set of limits are more or less hints--in operation, small
random perturbations, averaging zero, are applied to the specified limits.  If
a limit for a particular motion is set to zero, no jitter to that motion will
be applied.

If the Time button is pressed while the xyz joystick is manipulated, the attack
and decay characteristics of the jitters are set, making it possible, for
example, to simulate an abrupt twitch in some motion, followed be a slower
relaxation of that twitch.  The interval between twitches can also be set.  All
three parameters are randomly affected as above.

If the Position button is pressed, the joysticks set a base initial position
and orientation of the platform.  No random variation is applied.

The Run/Pause button does what you expect.


	

	