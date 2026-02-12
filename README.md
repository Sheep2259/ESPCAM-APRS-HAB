This code is for a espcam and sx1278 based superpressure picoballoon, 
designed to transmit images back to a computer over aprs and the internet
It uses mixed radix and base 91 encoding to efficiently use the aprs payload.

use RLNC erasure codes for images

IDEA: rather than having a large overhead in the data section of the packet for the coefficient, or even transmitting a seed for small pseudorandom generator, we can hash the latitude and longitude of the position sent with the packet, acting as a seed without taking any extra data.

Should switch aprs encoding to MicE to make that idea work better


things that need implementation


wspr functions in radio.cpp possibly

implement wraparound on Mixed radix encoding to avoid breaking from out of bounds data

not in this code but need aprs tracker website
