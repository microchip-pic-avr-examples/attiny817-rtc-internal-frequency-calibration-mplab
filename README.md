<!-- Please do not change this html logo with link -->
<a href="https://www.microchip.com" rel="nofollow"><img src="images/microchip.png" alt="MCHP" width="300"/></a>

# RTC Internal Frequency Calibration

This application uses RTC, 32.768 kHz crystal, external clock, Event System and TCD to measure the accuracy of the external crystal. The external clock is the reference clock, which is used to measure the external crystal. RTC is clocked by the 32.768 kHz crystal, and it is configured to generate a periodic event. TCD is clocked by the external clock, and it is configured to do a capture on the RTC event. As the TCD frequency is much higher than the RTC event frequency, TCD will overflow multiple times while waiting for the event. To keep track of the number of overflows, an interrupt is executed on overflow. The sum of TCD overflows and the capture value is used to calculate the frequency of the 32.768 kHz crystal.

This example is based on the Application Note [AN2711 - Real-Time Clock Calibration and Compensation on AVR® Microcontrollers](https://www.microchip.com/DS00002711), which should be referred to for a more detailed understanding of the concepts.

## Related Documentation

- [AN2711 - Real-Time Clock Calibration and Compensation on AVR® Microcontrollers](https://www.microchip.com/DS00002711)
- [ATtiny817 Device Page](https://www.microchip.com/wwwproducts/en/ATTINY817)

## Software Used

- [MPLAB® X IDE](http://www.microchip.com/mplab/mplab-x-ide) 5.40 or later
- [ATtiny DFP](http://packs.download.atmel.com/) 2.2.89 or later
- [MPLAB® XC8](http://www.microchip.com/mplab/compilers) 2.20 or a later
- [AVR/GNU C Compiler](https://www.microchip.com/mplab/avr-support/avr-and-arm-toolchains-c-compilers) 5.4.0 or later


## Hardware Used

- [ATtiny817 Xplained Pro](https://www.microchip.com/DevelopmentTools/ProductDetails/attiny817-xpro)
- Micro-USB cable (Type-A/Micro-B)
- External Clock Source

## Setup

1. The crystal on the ATtiny817 Xplained Pro board is by default not connected to the TOSC pins, as they are used for UART communication. To connect the pins, remove resistors R307 and R308 and place them on the footprints of R312 and R313. Refer to [ATtiny817 Xplained Pro User's Guide](https://www.microchip.com/DS50002684) for more information on how to do this.

2. This setup requires an external clock connected to pin PA3, but it is also possible to change the TCD clock source to the internal 16/20 MHz oscillator. Note that the accuracy of the measurement will never be higher than the accuracy of the reference used. As the accuracy of the 32.768 kHz crystal is higher than the accuracy of the internal oscillator, this calculation will only give insight into the accuracy of the internal oscillator. By regarding the 32.768 kHz (RTC clock source) as the reference, and the internal oscillator (TCD clock source) as the frequency to measure, the calculation will give the accuracy of the internal oscillator.

## Operation

1. Connect the board to the PC.

2. Download the zip file or clone the example to get the source code.

3. Open the project in MPLAB X IDE.

4. Build the solution and program the ATtiny817.

## Conclusion

This example has now illustrated how you can measure the accuracy of the external crystal
