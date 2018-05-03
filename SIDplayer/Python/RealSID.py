#!/usr/bin/env python
'''
RealSID.py Version 1.0
Based on Asbjorn Brask's RealSIDShield SID player Python script.
(https://github.com/atbrask/RealSIDShield)

Copyright (c) 2018, D.R.R. Bodor (lefinnois@lefinnois.net)
Copyright (c) 2013, A.T.Brask (atbrask[at]gmail[dot]com)
All rights reserved,
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
'''

import time
import serial
import argparse
import signal
import sys
from py65.devices import mpu6502

def signal_handler(signal, frame):
    print "You got Ctrl+C!"
    goodbye()

def goodbye():
    try:
        file
    except NameError:
        if args.verbose:
            print "Nothin to close (file)"
    else:
        if not file.closed:
            if args.verbose:
                print "Closing file {0}".format(file.name)
            file.close()
    try:
        ser
    except NameError:
        if args.verbose:
            print "Nothin to close (serial)"
    else:
        if(ser.isOpen() == True):
            if args.verbose:
                print "Reset SID"
            ser.write("R")
            if args.verbose:
                print "Closing serial port..."
            ser.flush()
            ser.close()
    print "Bye!"
    sys.exit(0)

# dump SID 1 memory mapped space
def getSIDstate(memory):
    return ''.join(["{0:02X}".format(byte) for byte in memory[0xD400:0xD419]])

# dump CIA control register
def getCIA1controlreg(memory):
    return ''.join(["{0:02X}".format(byte) for byte in memory[0xDD0D:0xDD0F]])

def runCPU(cpu, newpc, newa, newx, newy):
    cpu.pc = newpc
    cpu.a = newa
    cpu.x = newx
    cpu.y = newy
    cpu.sp = 0xFF

    running = True
    instructioncount = 0

    while running and instructioncount < 1000000:
        # Test for return instructions RTI (0x40 ReTurn from Interrupt) and RTS (0x60 ReTurn from Subroutine)
        if cpu.ByteAt(cpu.pc) in (0x40, 0x60) and cpu.sp == 0xFF:
            running = False

        # Test for BRK (0x00 BReaK)
        if cpu.ByteAt(cpu.pc) == 0x00:
            running = False

        # Step one instruction
        cpu.step()
        instructioncount += 1

        # Test for jump into Kernal interrupt handler exit
        if (cpu.ByteAt(0x01) & 0x07) != 0x5 and cpu.pc in (0xea31, 0xea81):
            running = False

def playsid(filename, subtune, serialport, baudrate, verbose, info):
    global file
    try:
        file = open(filename, 'rb')
    except IOError:
        print "Could not open file!"
        goodbye()

    # dump file
    data = [ord(byte) for byte in file.read()]

    # get sid file type & version
    filetype = ''.join([chr(ch) for ch in data[0:4]])
    version = data[5]
    print "SID type: {0} (version {1})".format(filetype, version)
    if filetype == "RSID":
        print "Error: RSID files not compatible"
        goodbye()

    loadaddress = (data[8] << 8) | data[9]
    if verbose:
        print "Load address: {0:04X}".format(loadaddress)

    initaddress = (data[10] << 8) | data[11]
    if verbose:
        print "Init address: {0:04X}".format(initaddress)

    playaddress = (data[12] << 8) | data[13]
    if verbose:
        print "Play address: {0:04X}".format(playaddress)

    songs = (data[14] << 8) | data[15]
    defaultsong = (data[16] << 8) | data[17]
    print "Found {0} song(s) (default song is {1})".format(songs, defaultsong)
    if subtune == -1 or subtune > songs:
        subtune = defaultsong

    print "Title    : {0}".format(''.join([chr(ch) for ch in data[22:54]]))
    print "Author   : {0}".format(''.join([chr(ch) for ch in data[54:86]]))
    print "Released : {0}".format(''.join([chr(ch) for ch in data[86:118]]))

    # just displaying info, so stop here
    if info:
        goodbye()

    speed = (data[18] << 24) | (data[19] << 16) | (data[20] << 8) | data[21]
    if speed == 0:
        if verbose:
            print "Using vertical blank interrupt (50hz PAL)."
    else: #TODO make CIA 1 timer compatible
        print "Error: CIA 1 emulation timer required, file not compatible."
        goodbye()

    # Check load address
    dataoffset = (data[6] << 8) | data[7]

    if loadaddress == 0:
        print "Warning: SID has load address 0, reading from C64 binary data"
        loadaddress = data[dataoffset] | (data[dataoffset + 1] << 8)
        dataoffset += 2
        print "New load address is {0:04X}".format(loadaddress)

    # Check init address
    if initaddress == 0:
        print "Warning: SID has init address 0, cloning load address instead"
        initaddress = loadaddress
        print "New init address is {0:04X}".format(initaddress)

    # Setup memory
    memory = [0] * 0x10000  # 64Kb RAM to 0x00
    memory[0x01] = 0x37;

    # put SID data in memory
    for idx, byte in enumerate(data[dataoffset:]):
        memory[loadaddress + idx] = byte

    # create CPU
    cpu = mpu6502.MPU(memory)

    # Init SID tune (jump into SID data)
    print "Initializing song {0}...".format(subtune)
    runCPU(cpu, initaddress, subtune - 1, 0, 0)

    # Check play address
    if playaddress == 0:
        print "Warning: SID has play address 0, reading from interrupt vector"
        if (memory[0x01] & 0x07) == 0x5:
            playaddress = memory[0xfffe] | (memory[0xffff] << 8)
        else:
            playaddress = memory[0x314] | (memory[0x315] << 8)
        print "New play address is {0:04X}".format(playaddress)

    # Init RealSIDShield and wait for it to reset properly
    print "Using serial port {0} at {1} baud...".format(serialport, baudrate)
    print "Initializing serial connection with the Arduino..."

    global ser
    try:
        ser = serial.Serial(port = serialport, baudrate = baudrate, timeout = 1)
    except:
        print "Unable to open serial port !"
        goodbye()

    time.sleep(1.0)  # Arduino need time to reset
    ser.flushInput()

    # Play SID tune
    print "Playing..."
    frames = 0
    while(True):
        if ser.read() == '?':
            # print getCIA1controlreg(memory)[0] + " " + getCIA1controlreg(memory)[1] + " " + getCIA1controlreg(memory)[2]
            runCPU(cpu, playaddress, 0, 0, 0)
            ser.write(getSIDstate(memory) + "!")
            ser.flush()
            if verbose:
                print str(frames).rjust(8, ' ') + " | " \
                    + getSIDstate(memory)[0:13] + " | " \
                    + getSIDstate(memory)[14:27] + " | " \
                    + getSIDstate(memory)[28:41] + " | " \
                    + getSIDstate(memory)[42:49]
            frames += 1
        else:
            print "Arduino not ready..."

    goodbye()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Hardware SID Player 0.1 - Simple SID reader/sender (based on RealSID.py by atbrask)")

    parser.add_argument("filename", help="Input SID file")
    parser.add_argument("-p", "--serialport", default="/dev/ttyUSB0", help="The serial port the Arduino is connected to")
    parser.add_argument("-i", "--info", help="Don't play, juste display informations about the file", action='store_true')
    parser.add_argument("-v", "--verbose", help="Verbose mode (Default No)", action='store_true')
    parser.add_argument("-s", "--song", type=int, default=-1, help="The song number to be played (default is specified in the SID file)")
    parser.add_argument("-b", "--baudrate", type=int, default=115200, help="Specify baudrate (default is 115200)")
    args = parser.parse_args()

    signal.signal(signal.SIGINT, signal_handler)

    playsid(args.filename, args.song, args.serialport, args.baudrate, args.verbose, args.info)
