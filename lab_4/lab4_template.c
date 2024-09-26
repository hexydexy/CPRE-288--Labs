/**
 * lab4_template.c
 * 
 * Template file for CprE 288 lab 4
 *
 * @author Zhao Zhang, Chad Nelson, Zachary Glanz
 * @date 08/14/2016
 */

#include "button.h"
#include "Timer.h"
#include "lcd.h"
#include "cyBot_uart.h"  // Functions for communiticate between CyBot and Putty (via UART)
                         // PuTTy: Buad=115200, 8 data bits, No Flow Control, No Party,  COM1

#warning "Possible unimplemented functions"
#define REPLACEME 0



int main(void) {
	button_init();
	lcd_init();
	cyBot_uart_init();            // Don't forget to initialze the cyBot UART before trying to use it
	
	// YOUR CODE HERE
	
	
	while(1)
	{
	
      button_getButton();
  


	
	}
	
}
