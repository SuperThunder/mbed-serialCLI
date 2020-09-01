Goal is to allow for easy basic command parsing of the form
- get valuename
- set valuename value
- Where valuename is a command or parameter of some kind that will be set up per-program
- Once the value is set, it will be available to the running mbed program to check (could be in global or local scope)

For now, takes over the serial RX and requries a printf function to call for TX

Currently uses mbed 5.15, will update to mbed 6 when it's less broken.

License: GPL v3