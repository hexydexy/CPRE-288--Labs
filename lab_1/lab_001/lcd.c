/**
 * @file lcd.c
 *      _     ____ ____       
 *     | |   / ___|  _ \  ___ 
 *     | |  | |   | | | |/ __|
 *     | |__| |___| |_| | (__ 
 *     |_____\____|____(_)___|
 *                          
 * @author Noah Bergman, Eric Middleton
 * @brief Functions for displaying content on the 4x16 Character LCD Screen.
 * @version 1.2
 * @updated on 09/06/2024 by Kenneth J. Treadaway
 * @details
 * This module provides an interface for controlling a 4x16 character LCD display
 * connected to a TM4C123G LaunchPad microcontroller board.
 *
 * The main features include:
 * - LCD initialization with busy flag polling and timer delays for stability
 * - Sending commands (clear, home, cursor position, etc.) with busy flag polling and minimal delays
 * - Writing individual characters or strings with polling and appropriate data hold delays
 * - Formatted printing using lcd_printf() with busy flag polling
 * - Forceful clearing of the display with lcd_forceClear() function
 *
 * Now deprecated features include:
 * - LCD initialization with only timer delays (no busy flag polling)
 * - Sending commands (clear, home, cursor position, etc.) relying solely on timer delays
 * - Writing individual characters or strings without busy flag polling
 * - Formatted printing using lcd_printf() without busy flag polling
 *
 * While timer delays are still used in combination with busy flag polling for stability,
 * the older implementation relying entirely on delays is deprecated but retained for compatibility.
 *
 * Punch List:
 * 1. Optimize `lcd_forceClear` function to reduce potential issues during forced LCD resets.
 * 2. Add more robust error handling to detect command failures or hardware malfunctions.
 * 3. Improve timer-based delay accuracy (especially in `lcd_sendNibble` function).
 * 4. Add support for dynamic resizing of the LCD screen (for different hardware).
 * 5. Optimize `lcd_putc` and `lcd_printf` for performance in high-throughput applications.
 * 6. Consider moving some functions to inline assembly for performance optimization.
 * 7. Evaluate using DMA to reduce CPU load when sending data to the LCD.
 * 8. Add an interrupt-driven mechanism for LCD writes instead of busy flag polling.
 * 9. Implement `lcd_scrollText` function to scroll text across the display as needed.
 * 10. Investigate compatibility with additional character LCD sizes (e.g., 20x4).
 *
 * @note While timer delays are still used in combination with busy flag polling for stability,
 *       the older implementation relying entirely on delays is deprecated but retained for compatibility. 
 */

#include "lcd.h"

// Bitmasks for setting individual bits in registers
#define BIT0        0x01  /**< Bit 0: Used in LCD Clear Command (HD_LCD_CLEAR) */
#define BIT1        0x02  /**< Bit 1: General purpose */
#define BIT2        0x04  /**< Bit 2: LCD Enable pin (EN) */
#define BIT3        0x08  /**< Bit 3: LCD Register Select pin (RS) */
#define BIT4        0x10  /**< Bit 4: LCD Read/Write pin (RW) */
#define BIT5        0x20  /**< Bit 5: General purpose */
#define BIT6        0x40  /**< Bit 6: General purpose */
#define BIT7        0x80  /**< Bit 7: LCD Busy Flag (BF) */

/**
 * @brief LCD control command definitions.
 * These commands are used to control the behavior and display of the LCD.
 */
#define HD_LCD_CLEAR        0x01  /**< Clears the LCD display, mapped to BIT0 */
#define HD_RETURN_HOME      0x02  /**< Returns the cursor to the home position, can map to BIT1 */
#define HD_CURSOR_SHIFT_DEC 0x05  /**< Shifts the cursor to the left (decrement) */
#define HD_CURSOR_SHIFT_INC 0x07  /**< Shifts the cursor to the right (increment) */
#define HD_DISPLAY_CONTROL  0x08  /**< Controls display settings (on/off, cursor, blink) */
#define HD_DISPLAY_ON       0x04  /**< Turns the display on */
#define HD_CURSOR_ON        0x02  /**< Turns the cursor on */
#define HD_BLINK_ON         0x01  /**< Enables cursor blinking */
#define HD_CURSOR_MOVE_LEFT 0x10  /**< Moves the cursor to the left */
#define HD_CURSOR_MOVE_RIGHT 0x14 /**< Moves the cursor to the right */
#define HD_DISPLAY_SHIFT_LEFT 0x18 /**< Shifts the entire display to the left */
#define HD_DISPLAY_SHIFT_RIGHT 0x1C /**< Shifts the entire display to the right */

#define LCD_WIDTH  20             /**< Number of characters per line on the LCD */
#define LCD_HEIGHT 4              /**< Number of lines on the LCD */
#define LCD_TOTAL_CHARS (LCD_WIDTH * LCD_HEIGHT) /**< Total characters that can be displayed */
#define LCD_DDRAM_WRITE 0x80      /**< Write command to set DDRAM address */
#define LCD_CGRAM_WRITE 0x40      /**< Write command to set CGRAM address */

#define EN_PIN      BIT2          /**< Enable pin */
#define RS_PIN      BIT3          /**< Register Select pin */
#define RW_PIN      BIT6          /**< Read/Write pin */
#define LCD_PORT_DATA GPIO_PORTF_DATA_R  /**< Data register for the LCD port */
#define LCD_PORT_CNTRL GPIO_PORTD_DATA_R /**< Control register for the LCD port */

/**
 * @brief Reverses the nibble of a byte.
 * @param x The byte to reverse the nibble of.
 * @return The byte with the nibble reversed.
 */
static uint8_t lcd_reverseNibble(uint8_t x) {
    return (((x & 0b0001) << 3) | ((x & 0b0010) << 1) | ((x & 0b0100) >> 1) | ((x & 0b1000) >> 3));
}

/**
 * @brief Checks if the LCD is busy.
 * @return True if the LCD is busy, false otherwise.
 */
static bool lcd_isBusy(void) {
    // Set the LCD to read mode
    LCD_PORT_CNTRL &= ~(RS_PIN);
    LCD_PORT_CNTRL |= RW_PIN;

    // Read the busy flag (bit 7)
    LCD_PORT_CNTRL |= EN_PIN;
    bool busy = (LCD_PORT_DATA & BIT7) != 0;
    LCD_PORT_CNTRL &= ~(EN_PIN);

    // Set the LCD back to write mode
    LCD_PORT_CNTRL &= ~(RW_PIN);

    return busy;
}

/**
 * @brief Waits until the LCD is not busy.
 */
static void lcd_waitUntilNotBusy(void) {
    while (lcd_isBusy()) {
        // Wait until the LCD is not busy
    }
}

/**
 * @brief Initializes the LCD display with busy flag polling.
 * 
 * This function sets up the GPIO pins for the LCD, configures the LCD for
 * 4-bit mode, and clears the display. Polls the busy flag to ensure proper
 * synchronization.
 */
void lcd_init(void) {
    SYSCTL_RCGCGPIO_R |= BIT3 | BIT5; // Turn on PORTD, PORTF system clock

    // Set port to output
    GPIO_PORTF_DIR_R |= 0x1E; // Pins 1:4
    GPIO_PORTF_DEN_R |= 0x1E;

    GPIO_PORTD_DIR_R |= (EN_PIN | RS_PIN | RW_PIN);
    GPIO_PORTD_DEN_R |= (EN_PIN | RS_PIN | RW_PIN);

    LCD_PORT_CNTRL &= ~(EN_PIN | RW_PIN | RS_PIN);

    // Delay 40ms after power applied
    timer_waitMillis(50);

    // Wake up sequence
    lcd_sendNibble(0x03);
    timer_waitMillis(10);
    lcd_sendNibble(0x03);
    timer_waitMicros(170);
    lcd_sendNibble(0x03);
    timer_waitMicros(170);
    lcd_sendNibble(0x02); // Function set 4-bit

    lcd_sendCommand(0x28); // Function 4-bit / 2-lines
    lcd_sendCommand(0x0F); // Display on, cursor on, blink on
    lcd_sendCommand(0x28); // Function 4-bit / 2-lines
    lcd_sendCommand(0x06); // Increment cursor, no display shift
    lcd_sendCommand(0x01); // Clear display
    lcd_waitUntilNotBusy();
}

/**
 * @brief Sends a character to the LCD display with busy flag polling.
 * @param data The character to be displayed.
 */
void lcd_putc(char data) {
    // Wait until LCD is not busy
    lcd_waitUntilNotBusy();

    // Select - Send Data
    LCD_PORT_CNTRL |= RS_PIN;  // RS = 1 for data
    LCD_PORT_CNTRL &= ~(RW_PIN); // RW = 0 for write

    // Send high nibble
    lcd_sendNibble(data >> 4);

    // Send lower nibble
    lcd_sendNibble(data & 0x0F);

    lcd_waitUntilNotBusy(); // Wait until the character is fully processed
}

/**
 * @brief Sends a null-terminated string to the LCD display with busy flag polling.
 * @param data The string to be displayed.
 */
void lcd_puts(const char data[]) {
    while (*data != '\0') {
        lcd_putc(*data); // Send individual characters
        data++;
    }
}

/**
 * @brief Sends a command to the LCD display with busy flag polling.
 * @param data The command to be executed.
 */
void lcd_sendCommand(uint8_t data) {
    lcd_waitUntilNotBusy(); // Wait until LCD is ready

    // Enable high
    LCD_PORT_CNTRL |= EN_PIN;
    LCD_PORT_CNTRL &= ~(RW_PIN | RS_PIN); // Write command

    // Send high nibble
    lcd_sendNibble(data >> 4);

    // Send lower nibble
    lcd_sendNibble(data & 0x0F);

    lcd_waitUntilNotBusy(); // Wait for the command to complete
}

/**
 * @brief Sends a 4-bit nibble to the LCD, then clears the port.
 * @param theNibble The nibble to be sent to the LCD.
 */
static void lcd_sendNibble(uint8_t theNibble) {
#ifdef IS_STEPPER_BOARD
    theNibble = lcd_reverseNibble(theNibble);
#endif
    LCD_PORT_CNTRL |= EN_PIN;
    LCD_PORT_DATA |= (theNibble & 0x0F) << 1; // PORTD1:4

    // Data hold time before clock = 40ns -- Change if faster clock
    timer_waitMicros(20);
    // Clock in data
    LCD_PORT_CNTRL &= ~(EN_PIN);

    timer_waitMicros(20);
    // Clear port
    LCD_PORT_DATA &= ~((0x0F) << 1);
}

/**
 * @brief Clears the LCD screen with busy flag polling.
 * @details This command takes over 1ms to complete.
 */
void lcd_clear(void) {
    lcd_sendCommand(HD_LCD_CLEAR);
    lcd_waitUntilNotBusy();
}

/**
 * @brief Moves the cursor to the home position (0, 0) with busy flag polling.
 */
void lcd_home(void) {
    lcd_sendCommand(HD_RETURN_HOME);
    lcd_waitUntilNotBusy();
}

/**
 * @brief Moves the cursor to the specified line (1-based) with busy flag polling.
 * @param lineNum The line number (1-4) to move the cursor to.
 */
void lcd_gotoLine(uint8_t lineNum) {
    // Address of the four line elements
    static const uint8_t lineAddress[] = {0x00, 0x40, 0x14, 0x54};

    lineNum = (0x03 & (lineNum - 1)); // Mask input for 0 - 3
    lcd_sendCommand(LCD_DDRAM_WRITE | lineAddress[lineNum]);
    lcd_waitUntilNotBusy();
}

/**
 * @brief Sets the cursor position to the specified coordinates with busy flag polling.
 * @param x The column (0-19) to move the cursor to.
 * @param y The row (0-3) to move the cursor to.
 */
void lcd_setCursorPos(uint8_t x, uint8_t y) {
    static const uint8_t lineAddresses[] = {0x00, 0x40, 0x14, 0x54};

    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) {
        // Invalid coordinates
        return;
    }

    // Compute the location index
    uint8_t index = lineAddresses[y] + x;

    // Set the cursor index
    lcd_sendCommand(0x80 | index);
    lcd_waitUntilNotBusy();
}

/**
 * @brief Prints a formatted string to the LCD display with busy flag polling.
 * @param format The format string, similar to printf().
 * @param ... Additional arguments, depending on the format string.
 */
void lcd_printf(const char *format, ...) {
    static char lastBuffer[LCD_TOTAL_CHARS + 1];

    char buffer[LCD_TOTAL_CHARS + 1];
    va_list argList;
    va_start(argList, format);
    vsnprintf(buffer, LCD_TOTAL_CHARS + 1, format, argList);
    va_end(argList);

    if (strcmp(lastBuffer, buffer) == 0) {
        return;
    }

    strcpy(lastBuffer, buffer);
    lcd_clear();
    char *str = buffer;
    int charNum = 0;
    while (*str && charNum < LCD_TOTAL_CHARS) {
        if (*str == '\n') {
            // Fill remainder of line with spaces
            charNum += LCD_WIDTH - charNum % LCD_WIDTH;
        } else {
            lcd_putc(*str);
            charNum++;
        }

        str++;

        // Handle line address jumps
        if (charNum % LCD_WIDTH == 0) {
            switch (charNum / LCD_WIDTH) {
            case 1:
                lcd_gotoLine(2);
                break;
            case 2:
                lcd_gotoLine(3);
                break;
            case 3:
                lcd_gotoLine(4);
            }
        }
    }
}

/**
 * @brief Forcefully clears the LCD display and resets any buffered data.
 * 
 * This function immediately sends the clear screen command (0x01) to the LCD,
 * bypassing the busy flag check. It should be used sparingly, as it forces the 
 * LCD into a reset state, which might interrupt ongoing operations.
 * 
 * @note Use sparingly as this function forces the LCD into a reset state and 
 *       may cause display errors if used while the LCD is busy.
 * 
 * @return bool True if the force-clear operation was issued successfully.
 *              False if the LCD is still busy or if the clear command failed.
 */
bool lcd_forceClear(void) {
    // Force send the clear display command (0x01) immediately
    LCD_PORT_CNTRL |= EN_PIN;      // Enable high
    LCD_PORT_CNTRL &= ~(RW_PIN | RS_PIN); // RW = 0 (write), RS = 0 (command mode)

    // Send high nibble of the clear command (0x01)
    lcd_sendNibble(HD_LCD_CLEAR >> 4); // HD_LCD_CLEAR is 0x01
    timer_waitMicros(1);

    // Send low nibble of the clear command (0x01)
    lcd_sendNibble(HD_LCD_CLEAR & 0x0F);
    
    // Pulse the enable pin to register the command
    LCD_PORT_CNTRL |= EN_PIN;      // Pulse enable pin high
    timer_waitMicros(20);          // Short delay for processing
    LCD_PORT_CNTRL &= ~(EN_PIN);   // Pulse enable pin low

    timer_waitMillis(2);           // Wait 2ms for LCD to process the clear command
    
    // Check if the LCD is still busy after clearing
    if (lcd_isBusy()) {
        return false;   // Clear command failed (LCD still busy or in error state)
    }

    return true;  // Command issued successfully
}
