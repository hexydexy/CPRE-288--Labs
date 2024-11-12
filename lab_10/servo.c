#include "Timer.h"
#include "lcd.h"
#include "servo.h"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

unsigned long pwm_period = 0x4E200;
volatile int degree = 0;
volatile int clockwise;

void servo_init(void){

    SYSCTL_RCGCGPIO_R |= 0x02;
    SYSCTL_RCGCTIMER_R |= 0x02; //send clock to gpio B
    while ((SYSCTL_PRGPIO_R & 0x02) != 0x02) {};

    //Config PB5 as T1CCP1
    GPIO_PORTB_DIR_R &= ~0x20;
    GPIO_PORTB_AFSEL_R |= 0x20;
    GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R & 0xFF7FFFFF) | 0x00700000;
    GPIO_PORTB_DEN_R |= 0x20;


    //Timer 1, Timer B, T1CCP1, PB5
    TIMER1_CTL_R &= ~0x00000100; // Disabling the timer
    TIMER1_CFG_R = 0x00000004; //GPTM config
    TIMER1_TBMR_R &= ~(0x4); // clear TnCMR (bit 2) for edge count mode
    TIMER1_TBMR_R |= 0x2; //sets TnMR, bits 0 and 1 to 0x2 for periodic mode
    TIMER1_TBMR_R |= 0x8; //set TnAMS, bit 3 to 1 for PWM mode enable

    TIMER1_TBPR_R = pwm_period >> 16;
    TIMER1_TBILR_R = pwm_period & 0xFFFF;

    TIMER1_CTL_R |= 0x00000100; // enable timer

}



void servo_move(float degree){
//       float millis;
//       int high;
//       int low;
//       unsigned long low_hex;
//       millis = (0.0056 * degrees) + 1;
//       high = millis * 1000000 / 62.5;
//       low = 320000 - high;
//       TIMER1_TBMATCHR_R = low;
//       TIMER1_TBPMR_R = low >> 16;
    int counts = 151.59 * degree + 284366;
        TIMER1_TBMATCHR_R = counts;
        TIMER1_TBPMR_R = counts >> 16;
}

int button_Handler(int ButtonInput){

    switch(ButtonInput){
        case 4:

                servo_move(degree + 1);
                if(clockwise == 1){
                    degree = degree + 1;
                }
                else{
                    degree = degree - 1;
                }

            break;
        case 3:

                servo_move(degree + 5);
                if(clockwise == 1){
                    degree = degree + 5;
                }
                else{
                    degree = degree - 5;
                }

            break;
        case 2:
            if(clockwise == 1){
                clockwise = 0;
                TIMER1_TBMR_R = 0x1A;
                break;
            }
            clockwise = 1;
            TIMER1_TBMR_R = 0x0A;
            break;
        case 1:

            if(clockwise == 0){
                degree = 180;
                servo_move(180);
            }
            else{
                degree = 0;
                servo_move(0);
            }
            break;
    }
    if(degree > 180){
        degree = 180;
    }
    if (degree < 0){
        degree = 0;
    }
    return degree;

}
