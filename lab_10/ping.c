#include "ping.h"
#include "Timer.h"
#include "lcd.h"

volatile unsigned long START_TIME = 0;
volatile unsigned long END_TIME = 0;
volatile enum{LOW, HIGH, DONE} STATE = LOW;
int count = 0;
bool intflag = true;

void ping_init (void){

    SYSCTL_RCGCGPIO_R |= 0x02;
    SYSCTL_RCGCTIMER_R |= 0x08;

    while ((SYSCTL_PRGPIO_R & 0x02) != 0x02) {}

    GPIO_PORTB_DIR_R &= ~0x08;
    GPIO_PORTB_AFSEL_R |= 0x08;
    GPIO_PORTB_PCTL_R |= 0x00007000;
    GPIO_PORTB_PCTL_R &= 0xFFFF7FFF;
    GPIO_PORTB_DEN_R |= 0x08;

    TIMER3_CTL_R &= ~0x100;
    TIMER3_CFG_R = 0x4;
    TIMER3_TBMR_R = 0x07;
    TIMER3_CTL_R |= (0x0C00);
    TIMER3_TBPR_R = 0xFF;
    TIMER3_TBILR_R = 0xFFFF;
    TIMER3_ICR_R |= 0x400;
    TIMER3_IMR_R |= 0x400;
    NVIC_EN1_R = 0x00000010;
    NVIC_PRI9_R = 0x8;

    IntRegister(INT_TIMER3B, TIMER3B_Handler);
    IntMasterEnable();

    TIMER3_CTL_R |= 0x100;
}

void ping_trigger (void){
    STATE = LOW;
    TIMER3_CTL_R &= ~0x100;
    TIMER3_IMR_R = ~0x400;
    GPIO_PORTB_AFSEL_R &= ~0x08;
    GPIO_PORTB_DIR_R |= 0x08;

    GPIO_PORTB_DATA_R &= ~0x08;
    timer_waitMicros(5);
    GPIO_PORTB_DATA_R |= 0x08;
    timer_waitMicros(5);
    GPIO_PORTB_DATA_R &= ~0x08;

    GPIO_PORTB_DIR_R &= ~0x08;

    TIMER3_ICR_R |= 0x400;
    GPIO_PORTB_AFSEL_R |= 0x08;
    TIMER3_IMR_R |= 0x400;
    TIMER3_CTL_R |= 0x100;
}

void TIMER3B_Handler(void){

    if(TIMER3_MIS_R & 0x400) {
        TIMER3_ICR_R |= 0x400;
        if(STATE == LOW) {
            START_TIME = TIMER3_TBR_R;
            STATE = HIGH;
        }
        else if (STATE == HIGH) {
            END_TIME = TIMER3_TBR_R;
            STATE = DONE;
        }
    }

}

float ping_getDistance (void){
    ping_trigger();
    while (STATE != DONE) {}; //wait for pulse
    float timeDif = 0; //width
    int overflow = (END_TIME > START_TIME);
    if (START_TIME > END_TIME) {
        timeDif = START_TIME - END_TIME;
//        lcd_printf("Distance Cm: %.2f \nOverflow: %d \nWidth: %d", timeDif, count, START_TIME-END_TIME);
}
    else {
        timeDif = ((unsigned long) overflow << 24) + START_TIME - END_TIME;
        count++;
//        lcd_printf("Distance Cm: %.2f \nOverflow: %d \nWidth: %d", timeDif, count, START_TIME-END_TIME);

    }
    timeDif /= 16000000;
    timeDif = (timeDif/2)*343*100;
//    lcd_printf("Distance Cm: %.2f \nOverflow: %d \nWidth: %d", timeDif, count, START_TIME-END_TIME);
    return timeDif;
}
