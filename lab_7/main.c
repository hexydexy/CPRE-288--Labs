

/**
 * main.c
 *
 * @author Winson Vetsavong & Miles Nichols
 * @date 10/17/2024
 */

#include <stdint.h>
#include <stdbool.h>
#include "timer.h"
#include "lcd.h"
#include "cyBot_Scan.h"
#include <inc/tm4c123gh6pm.h>
#include "driverlib/interrupt.h"
#include "uart.h"

#define _CALIBRATE 0
#define _PART1 1
#define _PART2 0
#define _PART3 0
#define _PART4 0

int main(void)
{
	timer_init();
	lcd_init();
	uart_init();
	cyBOT_init_Scan(0b0111);
	cyBOT_Scan_t sensor_data;

	right_calibration_value = 337750; //Calibration for CyBot 9
	left_calibration_value = 1382500;

#if _CALIBRATE
	cyBOT_SERVO_cal();
#endif

#if _PART1
	int angle = 0;
	int array1 [90];
	int array2 [90];

	while(angle <= 180) {
	    cyBOT_Scan(angle, &sensor_data);
	    angle += 2;
	}
#endif


}
