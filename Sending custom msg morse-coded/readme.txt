// SYSTEM BUS RADIO
// https://github.com/fulldecent/system-bus-radio
// Copyright 2016 William Entriken
// Wrapper C++ Server and Python Client Morse messages by Alberto Sanchez, Pedro J. Martinez, Jorge Rivera

This code uses "Using counter and threads".

It opens a socket listening at a specified port, 50007, but you can change it.

When it receives a message with format "duration-frequency" i.e.: "500-2500" (duration in milliseconds and frequency in hertz), calls "square_am_signal" function to get emitted the radiofrequency.

We have developed a Python client (sock_client.py). This code encodes custom string in morse code and send it to specified socket port (50007).

How to use?

1. Compile main.cpp
2. Run ./gmain. This process keeps listening at 50007 port (you can change it).
3. $ python sock_client.py "Message to be sent"
