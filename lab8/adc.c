#include "Timer.h"
#include "lcd.h"
#include "adc.h"
#include <stdint.h>


void adc_init(void){
    //jessie says don't need pll
    SYSCTL_RCGCADC_R |= 0x00000001; //Enable the ADC clock using the RCGCADC register (module 0)
    SYSCTL_RCGCGPIO_R |= 0b000000010; //Enable port B for AIN10

    //analog input 10 for PB4
    GPIO_PORTB_AFSEL_R |= 0x10;     // Enable alternate function on PB4 (bit 4) 0b 0001 0000 = 0x 10
    GPIO_PORTB_DEN_R &= ~0x10;      // Disable digital function on PB4
    GPIO_PORTB_AMSEL_R |= 0x10;     // Enable analog function on PB4

    //config
    ADC0_ACTSS_R &= 0b11110111;           // Disable SS3 during configuration
    ADC0_EMUX_R &= 0xFFFF0FFF;         // Set SS3 to use software trigger (clear bits 15-12) 4bits: {10, EMUX2, EMUX1, don't set me! 10}
    ADC0_SSMUX3_R &= ~0xF;
    ADC0_SSMUX3_R |= 10;             // Set SS3 to sample AIN10 (PB4)
    ADC0_SSCTL3_R = 0x06;           // Set end of sequence and interrupt flag for SS3
    ADC0_SAC_R = 0x4;               // Enable 16x hardware averaging
    ADC0_ACTSS_R |= 0b00001000;            // Enable SS3
}

// Trigger ADC conversion and get the result
int ADC0_InSeq3(void) {
    ADC0_PSSI_R = 0x8;               // C Processor Sample Sequence Initiate to start conversion on SS3
    while ((ADC0_RIS_R & 0x8) == 0); // Wait for conversion to complete (Raw Interrupt Status && bit 4 (PB4) = 0
    int result = ADC0_SSFIFO3_R & 0xFFFF; // Read 12-bit result from SS3 FIFO
    ADC0_ISC_R = 0x8;                // Clear completion flag for SS3
    return result;
}

int adc_read(void){
        int i;
        int total = 0;
        for (i = 0; i < 16; i++) {
            total += ADC0_InSeq3();
        }
        return total / 16;  // Average result
}
