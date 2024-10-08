

/**
 * @author Winson Vetsavong Miles Nichols
 * @Date 9/19/2024
 */

#include "cyBot_Scan.h"
#include "cyBot_uart.h"
#include "lcd.h"
#include "timer.h"

int main(void)
{
    /*
     * Initialize Variables
     */
    timer_init();
    lcd_init();
    oi_init();
    cyBot_uart_init();
    cyBOT_init_Scan(0b0111);
//    cyBOT_SERVO_cal(); //Calibration

    char message[100];

    right_calibration_value = 343000;//calibratation value 15903250 when right | Cybot 08
    left_calibration_value = 1351000;//cal val 1629250 left


    cyBOT_Scan_t sensor_data;

    oi_alloc(sensor_data);

//PART 2
    int i = 0;
    sprintf(message,"Degrees Distance [cm]\n"); //Print header
    while(message[i] != '\n') {
        cyBot_sendByte(message[i]);
        i++;
    }
    cyBot_sendByte('\n');


    for(i = 0; i <= 180; i = i+2) { //Increment from 0 to 180 degrees with 2 degree intervals
        cyBOT_Scan(i, &sensor_data);
        sprintf(message, "%d       %f\n", i, sensor_data.sound_dist);
        int j = 0;
            while(message[j] != '\n') {
                    cyBot_sendByte(message[j]);
                    j++;
                }
            cyBot_sendByte('\n');

    }
    oi_free(sensor_data);
}
