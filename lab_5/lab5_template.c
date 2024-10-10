/**
 * lab5_template.c
 * 
 * Template file for CprE 288 lab 5
 *
 * @author Zhao Zhang, Chad Nelson, Zachary Glanz
 * @date 08/14/2016
 *
 * @author Phillip Jones, updated 6/4/2019
 */

#include "button.h"
#include "timer.h"
#include "lcd.h"
#include <math.h>

#include "cyBot_uart.h"  // Functions for communiticate between CyBot and Putty (via UART)
                         // PuTTy: Buad=115200, 8 data bits, No Flow Control, No Party,  COM1

#include "cyBot_Scan.h"  // For scan sensors 

#define REPLACEME 0


// Defined in button.c : Used to communicate information between the
// the interupt handeler and main.
extern volatile int button_event;
extern volatile int button_num;


int main(void) {
    timer_init();
	button_init();
	init_button_interrupts();
	lcd_init();
	char message [50];
	int previous_button = 0;
	 // Don't forget to initialze the cyBot UART before trying to use it
	

     cyBot_uart_init_clean();  // Clean UART initialization, before running your UART GPIO init code

	// Complete this code for configuring the  (GPIO) part of UART initialization
      SYSCTL_RCGCGPIO_R |= 0b000010;
      timer_waitMillis(1);            // Small delay before accessing device after turning on clock
      GPIO_PORTB_AFSEL_R |= 0b00000011;
      GPIO_PORTB_PCTL_R &= 0xFFFFFFF00;     // Force 0's in the disired locations
      GPIO_PORTB_PCTL_R |= 0x000000011;     // Force 1's in the disired locations
      GPIO_PORTB_DEN_R |= 0b00000011;
      GPIO_PORTB_DIR_R &= 0b11111100;      // Force 0's in the disired locations
      GPIO_PORTB_DIR_R |= 0b00000001;      // Force 1's in the disired locataions
    
     cyBot_uart_init_last_half();  // Complete the UART device initialization part of configuration
	
	// Initialze scan sensors
     cyBOT_init_Scan(0b0111);
     cyBOT_Scan_t sensor_data;




	// YOUR CODE HERE
//	while(true)
//	{
//	   button_num = 0;
//	   int i = 0;
//	    if (previous_button != button_num) {
//            if (button_event == 1) {
//                sprintf(message,"Button %d is pressed!\n", button_num);
//                while (message[i] != '\n') {
//                    cyBot_sendByte(message[i]);
//                    i++;
//                }
//                cyBot_sendByte('\n');
//            }
//       }
//            previous_button = button_num;
//	}

     // PART 3
     while (true) {
         int i = 0;
         int distance = 0;
         char input = cyBot_getByte_blocking();
         if (input == 'm') {
             cyBOT_Scan(90, &sensor_data);
//             distance = (-0.0346 * pow(sensor_data.IR_raw_val,3))+(4.5110 * pow(sensor_data.IR_raw_val,2))-(201.8037 * (sensor_data.IR_raw_val)) + 4229.0647;
//             distance = 1293.8669 + (6114.2814/sensor_data.IR_raw_val);
             distance = 6524.7095 * (pow(sensor_data.IR_raw_val,-0.4570));
             sprintf(message, "Raw IR: %d Distance[cm]: %d\n", sensor_data.IR_raw_val, distance);
         }
         while (message[i] != '\n') {
             cyBot_sendByte(message[i]);
             i++;
         }
         cyBot_sendByte('\n');
         input = '\0';
     }
}
