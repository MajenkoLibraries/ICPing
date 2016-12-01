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

#ifndef _ICPING_H
#define _ICPING_H

#if (ARDUINO >= 100)
# include <Arduino.h>
#else
# include <WProgram.h>
#endif

class ICPing {
    private:
        static bool _timerConfigured;
        static uint8_t _timerPrescale;
        volatile static double _usPerTick;
        static ICPing *_handlers[9];  // Could do with this being smaller?
        
        uint8_t _trig_pin;
        uint8_t _echo_pin;
        volatile p32_ic *_reg;
        volatile uint32_t _mm;
        p32_ioport *_trig_port;
        uint32_t _trig_mask;
        uint32_t _irq;
        uint32_t _vector;
        volatile bool _ready;
        volatile bool _first;
        volatile uint16_t _start;
        volatile uint16_t _end;
        volatile bool _freeRun;

        p32_ioport *getPortInformation(uint8_t pin, uint32_t *mask);

    public:
        ICPing(uint8_t trig, uint8_t echo) : _trig_pin(trig), _echo_pin(echo) {}
        void begin();
        void handleFreeRun();
        void handleInterrupt();
        void startPing();
        uint32_t getDistance();
        bool isReady();
        void enableFreeRunMode();

        static void __USER_ISR interruptHandler();
        static void __USER_ISR freeRunHandler();

};

#endif
