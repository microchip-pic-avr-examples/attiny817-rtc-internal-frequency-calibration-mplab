/**
 * \file main.c
 *
 * \author
 *      Microchip Technology: http://www.microchip.com \n
 *      Support at http://www.microchip.com/support/ \n
 *
 * Copyright (C) 2018 Microchip Technology. All rights reserved.
 *
 * \page License
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Microchip may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an
 * Microchip AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY Microchip "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL Microchip BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *****************************************************
 * Introduction
 * ============
 *
 * This application uses RTC, 32.768 kHz crystal, external clock, Event System and TCD
 * to measure the accuracy of the external crystal. The external clock is the reference
 * clock, which is used to measure the external crystal.
 *
 * RTC is clocked by the 32.768 kHz crystal, and it is configured to generate a periodic
 * event.
 *
 * TCD is clocked by the external clock, and it is configured to do a capture on the
 * RTC event. As the TCD frequency is much higher than the RTC event frequency, TCD will
 * overflow multiple times while waiting for the event. To keep track of the number of
 * overflows, an interrupt is executed on overflow. The sum of TCD overflows and the
 * capture value is used to calculate the frequency of the 32.768 kHz crystal.
 *
 * The _PROTECTED_WRITE macro is used for writing to change protected registers.
 *
 * Please ensure that the steps to connect the crystal to the device has been followed.
 * See the ATtiny817 Xplained Pro User Guide for more information.
 *
 * This setup requires an external clock connected to pin PA3, but it is also possible to
 * change the TCD clock source to the internal 16/20 MHz oscillator. Note that the
 * accuracy of the measurement will never be higher than the accuracy of the reference
 * used. As the accuracy of the 32.768 kHz crystal is higher than the accuracy of the
 * internal oscillator, this calculation will only give insight into the accuracy of the
 * internal oscillator. By regarding the 32.768 kHz (RTC clock source) as the reference,
 * and the internal oscillator (TCD clock source) as the frequency to measure, the
 * calculation will give the accuracy of the internal oscillator.
 *
 * Related documents / Application notes
 * -------------------------------------
 *
 * This application is described in the following application note: To be published
 *
 * Supported evaluation kit
 * ------------------------
 *
 *  - ATTiny817-XPRO
 *
 *
 * Running the demo
 * ----------------
 *
 *  1. Press Download Pack and save the .atzip file
 *  2. Import .atzip file into Atmel Studio 7, File->Import->Atmel Start Project.
 *  3. Follow the instructions in the ATtiny817 Xplained Pro User Guide to connect the crystal to the device.
 *  4. Build the project and program the supported evaluation board
 *****************************************************
 */
#include <avr/io.h>
#include <stdbool.h>
#include <avr/interrupt.h>

// The two values below will contain the result of the measurement
// tcd_isr_count contains the number of times TCD has overflown within the
// RTC period. tcd_capture_value contains the capture value of the TCD counter
// when the RTC overflows. The total number of cycles counted in an RTC
// period is given by tcd_isr_count * 2^12 + tcd_capture_value
volatile uint32_t tcd_isr_count     = 0;
volatile uint16_t tcd_capture_value = 0;

// The RTC period is set to be 1 second given that the crystal is accurate
#define RTC_PERIOD_VALUE 0x7fff;
volatile bool first = true;

ISR(TCD0_OVF_vect)
{
	tcd_isr_count++;
	TCD0_INTFLAGS = TCD_OVF_bm;
}

ISR(TCD0_TRIG_vect)
{
	TCD0_INTFLAGS = TCD_TRIGB_bm;
	if (first) {
		// Read out value to empty reg
		tcd_capture_value = TCD0_CAPTUREB;
		RTC_PER           = RTC_PERIOD_VALUE;
		TCD0_INPUTCTRLB   = 0;
		tcd_isr_count     = 0;
		first             = false;
		return;
	}

	tcd_capture_value = TCD0_CAPTUREB;

	TCD0_CTRLA = 0;
}

void TCD_frequency_calibration()
{
	/*
	 * The crystal is started at the beginning of main in order to give the crystal time to start.
	 * The startup time of the crystal is set to the max value to give the crystal time to settle.
	 * The startup time is based on counting 64K cycles of the crystal. With this startup time it
	 * will take approximately 2 seconds before the crystal is ready to be used by peripherals.
	 *
	 * The run in standby bit is set for the crystal. This will allow the crystal to run in standby
	 * sleep mode. In addition it will allow the crystal to start up even if no peripherals are
	 * requesting the clock.
	 */
	_PROTECTED_WRITE(CLKCTRL_XOSC32KCTRLA, CLKCTRL_ENABLE_bm | CLKCTRL_RUNSTDBY_bm | CLKCTRL_CSUT_64K_gc);

	// Event System
	// Configure RTC as event generator on event channel 3
	EVSYS_ASYNCCH3 = EVSYS_ASYNCCH3_RTC_OVF_gc;
	// Set TCD as event user of event channel 3
	EVSYS_ASYNCUSER7 = EVSYS_ASYNCUSER7_ASYNCCH3_gc;

	// TCD
	// Event setup, trigger on rising edge, capture, event input
	TCD0_EVCTRLB = TCD_EDGE_bm | TCD_ACTION_bm | TCD_TRIGEI_bm;
	// Interrupt on trigger and overflow
	TCD0_INTCTRL = TCD_TRIGB_bm | TCD_OVF_bm;
	// Set CMPBCLR, defines period
	TCD0_CMPBCLR = 0x0FFF;
	// Set input mode to EDGETRIG, 0x8
	TCD0_INPUTCTRLB = TCD_INPUTMODE3_bm;

	// RTC
	// The RTC_STATUS needs to be 0 before writing to the RTC (could be used for start-up time).
	while (RTC.STATUS != 0) {
	}
	// Set XOSC32K as input
	RTC_CLKSEL = RTC_CLKSEL_TOSC32K_gc;
	// Set a low value to sync TCD to RTC event. Once TCD is synced PER will be changed to a one second tick
	RTC_PER = 0x09;
	// Enable RTC. RTC will not start counting until XOSC32K is reported as stable
	RTC_CTRLA = RTC_RTCEN_bm;
	sei();

	// Wait for XOSC32K to be reported as stable
	while (!(CLKCTRL_MCLKSTATUS & CLKCTRL_XOSC32KS_bm))
		;

	// Select TCD clock source and enable
	TCD0_CTRLA = TCD_ENABLE_bm | TCD_CLKSEL_EXTCLK_gc;
	// TCD0_CTRLA = TCD_ENABLE_bm | TCD_CLKSEL_20MHZ_gc;
}

int main(void)
{
	TCD_frequency_calibration();

	while (1) {
	}
}
