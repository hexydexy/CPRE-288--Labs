/*
 * button.c
 *
 *  Created on: Jul 18, 2016
 *      Author: Eric Middleton, Zhao Zhang, Chad Nelson, & Zachary Glanz.
 *
 *  @edit: Lindsey Sleeth and Sam Stifter on 02/04/2019
 *  @edit: Phillip Jone 05/30/2019: Mearged Spring 2019 version with Fall 2018
 */
 


//The buttons are on PORTE 3:0
// GPIO_PORTE_DATA_R -- Name of the memory mapped register for GPIO Port E, 
// which is connected to the push buttons
#include "button.h"

//Global variables
volatile int button_event;
volatile int button_num;


/**
 * Initialize PORTE and configure bits 0-3 to be used as inputs for the buttons.
 */
void button_init() {
	static uint8_t initialized = 0;
	SYSCTL_RCGCGPIO_R |= 0b010000;
	GPIO_PORTE_DIR_R &= 0b00000000;
	GPIO_PORTE_DEN_R |= 0b00001111;

	//Check if already initialized
	if(initialized){
		return;
	}

	// delete warning after implementing 
//	#warning "Unimplemented function: void button_init()"
	
	// Reading: To initialize and configure GPIO PORTE, visit pg. 656 in the 
	// Tiva datasheet.
	
	// Follow steps in 10.3 for initialization and configuration. Some steps 
	// have been outlined below.
	
	// Ignore all other steps in initialization and configuration that are not 
	// listed below. You will learn more about additional steps in a later lab.

	// 1) Turn on PORTE system clock, do not modify other clock enables
	//SYSCTL_RCGCGPIO_R |=

	// 2) Set the buttons as inputs, do not modify other PORTE wires
	// GPIO_PORTE_DIR_R &=
	
	// 3) Enable digital functionality for button inputs, 
	//    do not modify other PORTE enables
	//GPIO_PORTE_DEN_R |=

	
	initialized = 1;
}

/**
 * Initialize and configure PORTE interupts
 */
void init_button_interrupts() {

    // In order to configure GPIO ports to detect interrupts, you will need to visit pg. 656 in the Tiva datasheet.
    // Notice that you already followed some steps in 10.3 for initialization and configuration of the GPIO ports in the function button_init().
    // Additional steps for setting up the GPIO port to detect interrupts have been outlined below.
    // TODO: Complete code below

    // 1) Mask the bits for pins 0-3
    GPIO_PORTE_IM_R &= 0b11110000;

    // 2) Set pins 0-3 to use edge sensing
    GPIO_PORTE_IS_R &= 0b11110000;

    // 3) Set pins 0-3 to use both edges. We want to update the LCD
    //    when a button is pressed, and when the button is released.
    GPIO_PORTE_IBE_R |= 0b00001111;

    // 4) Clear the interrupts
    GPIO_PORTE_ICR_R = 0b11111111;

    // 5) Unmask the bits for pins 0-3
    GPIO_PORTE_IM_R |= 0b11111111;

    // TODO: Complete code below
    // 6) Enable GPIO port E interrupt
    NVIC_EN0_R |= 0x00000010;

    // Bind the interrupt to the handler.
    IntRegister(INT_GPIOE, gpioe_handler);
}


/**
 * Interrupt handler -- executes when a GPIO PortE hardware event occurs (i.e., for this lab a button is pressed)
 */
void gpioe_handler() {


    // Clear interrupt status register
    GPIO_PORTE_ICR_R = 0xFF;
//    update button_event = 1;
    button_num = button_getButton();
    button_event = 1;
}




/**
 * Returns the position of the rightmost button being pushed.
 * @return the position of the righttmost button being pushed. 4 is the rightmost button, 1 is the leftmost button.  0 indicates no button being pressed
 */

uint8_t button_getButton() {
    int button = 0;

       if(!(GPIO_PORTE_DATA_R & 0b00001000)){
	    button = 4;
	} else if (!(GPIO_PORTE_DATA_R & 0b00000100)) {
	    button = 3;
	} else if (!(GPIO_PORTE_DATA_R & 0b00000010)) {
	    button = 2;
	} else if (!(GPIO_PORTE_DATA_R & 0b00000001)) {
	    button = 1;
	}
	
	return button; // EDIT ME
}





