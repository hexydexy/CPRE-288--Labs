/**
 * @file lcd.h
 * @brief Header file for the 4x16 Character LCD Screen driver.
 * @updated on 09/06/2024 by Kenneth J. Treadaway
 * @date 09/06/2024
 * @version 1.2
 */

#ifndef LCD_H
#define LCD_H

#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include "tm4c123gh6pm.h"

/**
 * @brief Initializes the LCD display with busy flag polling.
 * 
 * This function sets up the GPIO pins for the LCD, configures the LCD for
 * 4-bit mode, and clears the display. Polls the busy flag to ensure proper
 * synchronization.
 */
void lcd_init(void);
static void lcd_waitUntilNotBusy(void);
static void lcd_waitUntilNotBusy(void) ;
/**
 * @brief Sends a character to the LCD display with busy flag polling.
 * @param data The character to be displayed.
 */
void lcd_putc(char data);

/**
 * @brief Sends a null-terminated string to the LCD display with busy flag polling.
 * @param data The string to be displayed.
 */
void lcd_puts(const char data[]);

/**
 * @brief Sends a command to the LCD display with busy flag polling.
 * @param data The command to be executed.
 */
void lcd_sendCommand(uint8_t data);

/**
 * @brief Clears the LCD screen with busy flag polling.
 * @details This command takes over 1ms to complete.
 */
void lcd_clear(void);

/**
 * @brief Moves the cursor to the home position (0, 0) with busy flag polling.
 */
void lcd_home(void);

/**
 * @brief Moves the cursor to the specified line (1-based) with busy flag polling.
 * @param lineNum The line number (1-4) to move the cursor to.
 */
void lcd_gotoLine(uint8_t lineNum);

/**
 * @brief Sets the cursor position to the specified coordinates with busy flag polling.
 * @param x The column (0-19) to move the cursor to.
 * @param y The row (0-3) to move the cursor to.
 */
void lcd_setCursorPos(uint8_t x, uint8_t y);

/**
 * @brief Prints a formatted string to the LCD display with busy flag polling.
 * @param format The format string, similar to printf().
 * @param ... Additional arguments, depending on the format string.
 */
void lcd_printf(const char *format, ...);

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
bool lcd_forceClear(void);

#endif // LCD_H
