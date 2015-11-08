/*
 * Copyright (c) 2015, Majenko Technologies
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <ICPing.h>

void ICPing::begin() {
    pinMode(_trig_pin, OUTPUT);
    pinMode(_echo_pin, INPUT);
    _trig_port = getPortInformation(_trig_pin, &_trig_mask);
    _echo_port = getPortInformation(_echo_pin, &_echo_mask);

    uint16_t timer = digitalPinToTimerIC(_echo_pin);
    if (timer == 0) {
        // Not an IC pin!!!!
        return;
    }
    // Convert to a zero-based index of IC timer number
    timer = (timer >> _BN_TIMER_IC) - 1;
    _reg = (p32_ic *)(&IC1CON + (0x200 * timer));

    _reg->icxCon.reg = 0;
    _reg->icxCon.set = 1 << 9; // Capture rising edge first
    //_reg->icxCon.set = 1 << 5; // Interrupt on every second event
    _reg->icxCon.set = 1 << 2; // Mode 6:
    _reg->icxCon.set = 1 << 1; //   Simple capture starting with specific edge
    _freeRun = false;
    
    // We need to use timer 3 since timer 2 is used for
    // PWM and there is no other option available. All
    // IC pins will use the same timer base.
    // We need to ensure that it can run for at least 25ms before
    // the IC counter loops in 16-bit mode.
    if (!_timerConfigured) {
        uint32_t f_pb = getPeripheralClock();
        uint32_t baseclock = f_pb;
        if(baseclock / 40 > 65535) {
            baseclock = f_pb / 2;
            T3CONbits.TCKPS = 1;                
        }

        if(baseclock / 40 > 65535) {
            baseclock = f_pb / 4;
            T3CONbits.TCKPS = 2;                
        }

        if(baseclock / 40 > 65535) {
            baseclock = f_pb / 8;
            T3CONbits.TCKPS = 3;                
        }

        if(baseclock / 40 > 65535) {
            baseclock = f_pb / 16;
            T3CONbits.TCKPS = 4;                
        }

        if(baseclock / 40 > 65535) {
            baseclock = f_pb / 32;
            T3CONbits.TCKPS = 5;                
        }

        if(baseclock / 40 > 65535) {
            baseclock = f_pb / 64;
            T3CONbits.TCKPS = 6;                
        }

        if(baseclock / 40 > 65535) {
            baseclock = f_pb / 256;
            T3CONbits.TCKPS = 7;                
        }
        _usPerTick = (1000000.0 / baseclock);
        PR3 = 0xFFFF;
        T3CONbits.TON = 1;
        setIntVector(_TIMER_3_IRQ, ICPing::freeRunHandler);
        setIntPriority(_TIMER_3_IRQ, 2, 0);
        clearIntFlag(_TIMER_3_IRQ);
        setIntEnable(_TIMER_3_IRQ);
        _timerConfigured = true;
    }

    _ready = false;
    _irq = _INPUT_CAPTURE_1_IRQ + (timer * 4);
    setIntVector(_irq, ICPing::interruptHandler);
    setIntPriority(_irq, 4, 0);
    clearIntFlag(_irq);
    setIntEnable(_irq);
    _handlers[timer] = this;
    
}

p32_ioport *ICPing::getPortInformation(uint8_t pin, uint32_t *mask) {
    uint32_t portno = digitalPinToPort(pin);
    if (portno == NOT_A_PIN) {
        return NULL;
    }
    if (mask != NULL) {
        *mask = digitalPinToBitMask(pin);
    }
    return (p32_ioport *)portRegisters(portno);
}

void __USER_ISR ICPing::interruptHandler() {
    for (int i = 0; i < 9; i++) {
        if (_handlers[i] != NULL) {
            _handlers[i]->handleInterrupt();
        }
    }
}

void __USER_ISR ICPing::freeRunHandler() {
    clearIntFlag(_TIMER_3_IRQ);
    for (int i = 0; i < 9; i++) {
        if (_handlers[i] != NULL) {
            _handlers[i]->handleFreeRun();
        }
    }
}

void ICPing::handleFreeRun() {
    if (_freeRun) {
        startPing();
    }
}

void ICPing::handleInterrupt() {
    if (getIntFlag(_irq)) {
        clearIntFlag(_irq);
        if (_first) {
            _start = _reg->icxBuf.reg;
            _first = false;
            return;
        }

        _end = _reg->icxBuf.reg;
        // We have received our ping, so turn ourselves off.
        _reg->icxCon.clr = 1 << 15;

        uint16_t time = _end - _start;
        _ready = true;
        uint32_t us = time * _usPerTick;
        double mm = ((double)us / 5.82);
        _mm = (uint32_t)mm;
    }
}

void ICPing::startPing() {
    _reg->icxCon.set = 1 << 15; // Turn it on
    _trig_port->lat.set = _trig_mask;
    delayMicroseconds(10);
    _trig_port->lat.clr = _trig_mask;
    _first = true;
    _ready = false;
}

uint32_t ICPing::getDistance() {
    return _mm;
}

bool ICPing::isReady() {
    return _ready;
}

void ICPing::enableFreeRunMode() {
    _freeRun = true;
}

bool ICPing::_timerConfigured = false;
volatile double ICPing::_usPerTick = 0;
ICPing *ICPing::_handlers[9] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
