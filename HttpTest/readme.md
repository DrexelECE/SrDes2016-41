# HTTP Interface Emulator

To give commands to the RPi: 

To control things, send an HTTP GET request to this app /fly? with a comma-separated list of the new values, each in three-digit hex.  

This will return a JSON array of all of the current values, again in three-digit hex. 

For instance, the following GET will set all of the first five PWM values to 'full'. 

	http://10.0.0.10:8000/srDes/HttpTest/fly?fff,fff,fff,fff,fff 
	
To get an image, send an HTTP GET request to this app /image? followed by the number of the camera the image needs to come from.  Use 0 for down-ward facing and 1 for forward facing. 

For instance, the following GET will return the front-facing image. (For testing purposes, this endpoint simply returns the excessively generic image.) 

	http://10.0.0.10:8000/srDes/HttpTest/image?1
	
