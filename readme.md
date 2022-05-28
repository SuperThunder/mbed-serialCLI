
### Note: Better libraries exist
At the time that I made this I was having difficulty finding any interactive shells made for mbed. However, they do exist, but don't show up on google very well:
* shinichiro nakamura's Natural Tiny Shell mbed adaption by iotexpert (https://iotexpert.com/ntshell-for-mbed-os/)
* A (semi?) offical MbedOS command line?: https://github.com/PelionIoT/mbed-client-cli and https://os.mbed.com/docs/mbed-os/v6.15/mbed-os-api-doxy/ns__cmdline_8h.html#func-members. Not thread safe and not documented, but receives somewhat active updates? Genuinely confused why this wasn't made into a normal part of Mbed.
* shimniok SimpleShell (https://os.mbed.com/users/shimniok/code/SimpleShell/)
* murilopontes tinyshell (https://os.mbed.com/users/murilopontes/code/tinyshell/)
* vpcola SerialShell (https://os.mbed.com/users/vpcola/code/SerialShell/)
* ChuckM console.c demo for STM32F469 (https://github.com/ChuckM/stm32f469i/blob/master/demos/util/console.c)
* * These demos are libopencm3, but cover almost all the functionality of an STM32F4 so are very instructive anyway





Natural Tiny Shell in particular appears to be quite excellent and it's a shame it's not very well known.  
  
  
  
  
  

## serialCLI for mbed
Goal is to allow for easy basic command parsing of the form
- get variablename
- set variablename value
- Where variablename is a command or parameter of some kind that will be set up per-program by attaching a user defined variableHandler
- A user-defined handler for each variablename is called with the context of whether a GET or SET was requested
- So 'variablename' doesn't have to correspond to an actual variable - 'get help' could print a help message and 'set mode lowpower' could completely change the operation of the device

My motivating use case is to set and get parameters in an 18650 battery charging system, and be able to configure automated output.

main.cpp is a small demo program - library is in serialCLI folder. Current resource use is 130kb flash, 13kb memory; for reference the blinky example is 37kb flash / 9.5kb memory.

For now, effectively takes over the serial RX (read()) and uses the write() function to call for TX.

TODO:
- Consider attaching commands rather than variable names (get/set would be their own commands)
    - commands could live in their own header files per command or set of related commands
- Get/set should possibly go into a dictionary (of mixed types?) that the running program can read
    - Much cleaner than having to write a handler for each variable, but slower, much more RAM use, and less flexible


Currently uses mbed 5.15, will update to mbed 6 when that's less broken.

mbed 6 'please actually start working' list
- can't define or use strings in a loop
- can't use strings anywhere
- Serial libraries have completely changed and are now only file-like(?) except there are no examples or documentation on proper usage

License: GPL v3
