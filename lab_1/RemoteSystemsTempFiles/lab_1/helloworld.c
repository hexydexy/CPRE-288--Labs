//*
// Word Banner
// @Author Winson Vetsavong
// @Author Miles
//

#include "Timer.h"
#include "lcd.h"
#include "string.h"

int main (void) {

	timer_init(); // Initialize Timer, needed before any LCD screen fucntions can be called 
	              // and enables time functions (e.g. timer_waitMillis)

	lcd_init();   // Initialize the the LCD screen.  This also clears the screen. 
//
//	char quote[] = "Microcontrollers are a lot of fun! ";
//	char banner[] = "                   ";
//
//	strcat(banner,quote);
//
//	while(true) {
//	    int i;
//	    int j;
//	    for (i = 0; i < 20; i++) {
//	        lcd_putc(banner[i]);
//	    }
//
//	    banner[strlen(quote)-1)] = banner[0];
//
//	    for( j = 0; j < strlen(quote); i++) {
//	        banner[i] = banner[i+1];
//	    }
//	    timer_waitMillis(2000);
//
//	    if (i == 20) {
//	        lcd_init();
//	    }
//
//	    }
    }
