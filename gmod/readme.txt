# To use this module do the following:


1. To load the pwm-module and have it set up the the GPIO,
   add the following lines to `\boot\config.txt` 
~~~
dtoverlay=pwm
dtparam=pin=18
dtparam=func=2
~~~
2. (and switch off analog audio), this should request gpio 18?

# Todo

- why is the pwm not mapped to memory even though the module is loaded and I call pwm request?
