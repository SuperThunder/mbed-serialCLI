## serialCLI for mbed
Goal is to allow for easy basic command parsing of the form
- get variablename
- set variablename value
- Where variablename is a command or parameter of some kind that will be set up per-program by attaching a user defined variableHandler
- A user-defined handler for each variablename is called with the context of whether a GET or SET was requested
- So 'variablename' doesn't have to correspond to an actual variable - 'get help' could print a help message and 'set mode lowpower' could completely change the operation of the device

My motivating use case is to set and get parameters in an 18650 battery charging system.

For now, effectively takes over the serial RX and uses the printf function to call for TX.

Currently uses mbed 5.15, will update to mbed 6 when that's less broken.

mbed 6 'please actually start working' list
- can't define or use strings in a loop
- can't use strings anywhere
- Serial libraries have completely changed and are now only file-like(?) except there are no examples or documentation on proper usage

License: GPL v3