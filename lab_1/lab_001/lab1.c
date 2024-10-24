//#include "lcd.h"
//#include "Timer.h"
//#include <string.h>
//#include "tm4c123gh6pm.h"
//#include <stdio.h>
//
//#define DELAY_TIME 300  // 300 milliseconds
//#define LCD_WIDTH 20    // Width of the LCD screen
//char message[] = "PLEASE END MY SUFFERING OH GOD ALL ALMIGHTY";
//
//void scrollBanner(void) {
//    int i;
//    int j;
//    int msgLen = strlen(message), totalLen = msgLen + LCD_WIDTH;
//    char display[LCD_WIDTH + 1];
//
//    while (1) {
//        // Step 1: Scroll message from the right
//        for (i = 0; i < totalLen; i++) {
//            for (j = 0; j < LCD_WIDTH; j++) {
//                int idx = i + j - LCD_WIDTH + 1;
//                display[j] = (idx >= 0 && idx < msgLen) ? message[idx] : ' ';
//            }
//            display[LCD_WIDTH] = '\0';   // Null-terminate the string
//            lcd_printf("%s", display);   // Display on the first line
//            timer_waitMillis(DELAY_TIME);  // Wait for 300ms
//
//            // Ensure the second line is always blank
//            lcd_gotoLine(2);
//            lcd_printf("");
//        }
//
//        // Step 2: Wait 300ms after the message is completely off the screen
//        timer_waitMillis(DELAY_TIME);
//    }
//}
//
//int main(void) {
//    timer_init();
//    lcd_init();
//    scrollBanner();
//}

#include "lcd.h"
#include "Timer.h"
#include <string.h>
#include "tm4c123gh6pm.h"
#include <stdio.h>

#define DELAY_TIME 300  // Delay between scroll updates (milliseconds)
#define LCD_WIDTH 20    // Width of the LCD screen
#define MESSAGE "Microcontrollers are lots of fun!"  // Scrolling message

void scrollBanner(void) {
    int messageLength = strlen(MESSAGE);
    char display[LCD_WIDTH + 1];  // Buffer to hold the visible portion of the message
int i;
int offset;
    while (1) {
        // Scroll message from right to left
        for (offset = 0; offset < messageLength + LCD_WIDTH; offset++) {
            // Fill display buffer with characters or spaces
            for (i = 0; i < LCD_WIDTH; i++) {
                int messageIndex = offset + i - LCD_WIDTH;
                display[i] = (messageIndex >= 0 && messageIndex < messageLength) ? MESSAGE[messageIndex] : ' ';
            }
            display[LCD_WIDTH] = '\0';  // Null-terminate the string

            // Wait for the LCD to be ready before displaying
            lcd_waitUntilNotBusy();
            lcd_printf("%s", display);  // Display the string on the LCD

            timer_waitMillis(DELAY_TIME);  // Delay for smooth scrolling

            // Clear second line to maintain clean display
            lcd_gotoLine(2);
            lcd_printf("");  // Ensure second line is blank
        }

        // Clear the screen briefly between loops
        lcd_clear();
        timer_waitMillis(DELAY_TIME);
    }
}

int main(void) {
    // Initialize timer and LCD
    timer_init();
    lcd_init();

    // Start scrolling the banner
    scrollBanner();

    return 0;
}
