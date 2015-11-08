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

// Set up a typical 4-pin ultrasound module with the trigger
// pin on pin 3 and the echo pin on pin 2.  On the uC32 pin 2 is
// Input Capture 1.  Check the manual for your board for which
// pins are Input Capture pins.
ICPing ping(3, 2);

void setup() {
    Serial.begin(115200);  
    ping.begin();
    // Free-run mode is keyed to the timer that drives the
    // IC module (Timer 3).  Every time the timer overflows it
    // executes a new ping.
    ping.enableFreeRunMode();
}


void loop() {
    // This will print the distance and a little bar graph
    // in a decent serial terminal.  In MPIDE you may well end up with
    // a bit of a mess.
    Serial.printf("\e[1;1HDistance: %dmm      \r\n", ping.getDistance());
    int pct = ping.getDistance() / 50;
    for (int i = 0; i < 80; i++) {
        if (i <= pct) {
            Serial.print("#");
        } else {
            Serial.print(" ");
        }
    }
}
