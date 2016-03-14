#!/usr/bin/env python
# -*- coding: utf-8 -*-

# SYSTEM BUS RADIO
# https://github.com/fulldecent/system-bus-radio
# Copyright 2016 William Entriken
# Python Client Morse messages by Alberto Sanchez, Pedro J. Martinez, Jorge Rivera

import socket
import sys
import time

HOST, PORT = "localhost", 50007
# frequency in hertz
freq = 2000
try:
    st = sys.argv[1]
except:
    st = "test"

def chartomorse(character):
    if character == 'a' or character == 'A':
        morsecode = '.-'
    if character == 'b' or character == 'B':
        morsecode = '-...'
    if character == 'c' or character == 'C':
        morsecode = '-.-.'
    if character == 'd' or character == 'D':
        morsecode = '-..'
    if character == 'e' or character == 'E':
        morsecode = '.'
    if character == 'f' or character == 'F':
        morsecode = '..-.'
    if character == 'g' or character == 'G':
        morsecode = '--.'
    if character == 'h' or character == 'H':
        morsecode = '....'
    if character == 'i' or character == 'I':
        morsecode = '..'
    if character == 'j' or character == 'J':
        morsecode = '.---'
    if character == 'k' or character == 'K':
        morsecode = '-.-'
    if character == 'l' or character == 'L':
        morsecode = '.-..'
    if character == 'm' or character == 'M':
        morsecode = '--'
    if character == 'n' or character == 'N':
        morsecode = '-.'
    if character == 'ñ' or character == 'Ñ':
        morsecode = '--.--'
    if character == 'o' or character == 'O':
        morsecode = '---'
    if character == 'p' or character == 'P':
        morsecode = '.--.'
    if character == 'q' or character == 'Q':
        morsecode = '--.-'
    if character == 'r' or character == 'R':
        morsecode = '.-.'
    if character == 's' or character == 'S':
        morsecode = '...'
    if character == 't' or character == 'T':
        morsecode = '_'
    if character == 'u' or character == 'U':
        morsecode = '..-'
    if character == 'v' or character == 'V':
        morsecode = '...-'
    if character == 'w' or character == 'W':
        morsecode = '.--'
    if character == 'x' or character == 'X':
        morsecode = '-..-'
    if character == 'y' or character == 'Y':
        morsecode = '-.--'
    if character == 'z' or character == 'Z':
        morsecode = '--..'
    if character == '0':
        morsecode = '-----'
    if character == '1':
        morsecode = '.----'
    if character == '2':
        morsecode = '..---'
    if character == '3':
        morsecode = '...--'
    if character == '4':
        morsecode = '....-'
    if character == '5':
        morsecode = '.....'
    if character == '6':
        morsecode = '-....'
    if character == '7':
        morsecode = '--...'
    if character == '8':
        morsecode = '---..'
    if character == '9':
        morsecode = '----.'
    if character == '.':
        morsecode = '.-.-.-'
    if character == ',':
        morsecode = '--..--'
    if character == '?':
        morsecode = '..--..'
    if character == '!':
        morsecode = '-.-.--'
    if character == ' ':
        morsecode = ' '
    return morsecode

def morsetoduration(char):

    duration = 1
    if char == "-":
        duration = 0.500
    if char == ".":
        duration = 1
    return duration

def sendMess(duration, frequency):

    s.send(b''+str(duration)+'-'+str(frequency))
    print str(duration)+'-'+str(frequency)


s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

result = ""
for letter in st: 
    result += chartomorse(letter)+" "
print result

# Dot 50ms, Dash 150ms, Space between symbols (sbs) 50ms,  Space between characters (sbc) 150ms,  Space between words (sbw) 350ms

dot = 50
dash = 3 * dot
sbs = dot
sbc = 3 * dot
sbw = 7 * dot

for chmorse in result:
    print chmorse
    if chmorse == ".":
        sendMess(dot, freq)
        time.sleep(float(dot+sbs)/1000)
    elif chmorse == "-":
        sendMess(dash, freq)
        time.sleep(float(dot+sbs)/1000)
    elif chmorse == " ":
        time.sleep(float(sbc)/1000)
    elif chmorse == "  ":
        time.sleep(float(sbw)/1000)

s.shutdown(socket.SHUT_RDWR)
s.close()
