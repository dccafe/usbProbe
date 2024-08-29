# usbProbe
A general purpose probe firmware for the MSP430F5529.

# General command format: 
  
  module action pin [options]

module    - gpio, timer, serial, adc
action    - read, write 
pin       - Px.y, where x is the port and y is the pin
options   - each module has a different option. Examples:
            -> gpio   read  Px.y [pu|pd]
            -> gpio   write Px.y <0|1>
            -> timer  off   Px.y
            -> timer  read  Px.y
            -> timer  write Px.y f:<freq> d:<duty cycle>
            -> serial read  Px.y UART <terminator> 
            -> serial write Px.y UART _msg_ 
            -> serial read  Px.y I2C  <addr> <size>
            -> serial write Px.y I2C  <addr> _msg_
            -> serial read  Px.y SPI  <size>
            -> serial write Px.y SPI  _msg_

          - _msg_ : < s:<string> | b:<size>:<binary data> >

Strings are expected to end in \n, binary data requires size specification in number of bytes

Examples: 

// Get an echo of send commands (for use in terminal, manually)
echo on
-> OK
echo off
-> OK

// Read buttons
gpio read P2.1
-> OK 0
gpio read P1.1
-> OK 1
// Turn on red LED
gpio write P1.0 1
-> OK

// generate a 128 Hz PWM with 25% of duty cycle
timer write P4.7 f:128 d:25
-> OK
timer write P4.7 f:1234567 d:25
-> ERROR  Requested frequency is too high

// measure PWM frequency and duty cycle
timer read P1.0
-> OK d:46 f:4305

serial write P3.3 UART s:This is a test
-> OK
serial read  P3.4 I2C 0x32 19
-> OK message from MSP430

adc read P6.0
-> OK 0x6E9

adc write P6.1
-> ERROR command not available
