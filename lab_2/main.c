/**
 * @Author Winson Vetsavong
 * @Author Miles Nichols
 *
 * @Date 9/12/2024
 */

#include "open_interface.h"
#include "movement.h"
#include "CyBot_uart.h"
#include "lcd.h"

void main()
{
    oi_t *sensor_data = oi_alloc();
    oi_init(sensor_data);
    cyBot_uart_init();
    lcd_init();

//    int i;
//
//     double sum = 0;
//     oi_setWheels(150, 150); // move forward; reduced speed
//
//     while (sum < 1000) {
//         oi_update(sensor_data);
//         sum += sensor_data->distance;
//     }
//
//     oi_setWheels(0, 0); // stop
//     oi_free(sensor_data);

// Part 2

//     for(i = 0; i < 4; i++) {
//         move_forward(sensor_data, 100.0);
//         turn_clockwise(sensor_data, 82.0);
//     }

//Part 3

//    int sum = 0;
//    int millimeters = 2000;
//
//    oi_setWheels(250, 250);s
//    while (sum < millimeters)
//    {
//        oi_update(sensor_data);
//        sum += sensor_data->distance;
//        if (sensor_data->bumpLeft)
//        {
//            move_backward(sensor_data, 15);
//            sum -= 150;
//            turn_clockwise(sensor_data, 82);
//            move_forward(sensor_data, 25);
//            turn_counterclockwise(sensor_data, 82);
//            oi_setWheels(250,250);
//        }
//        else if (sensor_data->bumpRight)
//        {
//            move_backward(sensor_data, 15);
//            sum -= 150;
//            turn_counterclockwise(sensor_data, 82);
//            move_forward(sensor_data, 25);
//            turn_clockwise(sensor_data, 82);
//            oi_setWheels(250,250);
//        }
//    }
//    oi_setWheels(0, 0);
//    oi_free(sensor_data);
//    return;

//     part 4

    while(true) {
        char c = cyBot_getByte();

        if (c == 'm') {
            lcd_puts("Received an M");
        } else if (c == 'w'){
            move_forward(sensor_data,50.0);
        } else if (c == 's') {
            move_backward(sensor_data,50.0);
        } else if(c == 'a') {
            turn_counterclockwise(sensor_data,82);
        } else if(c == 'd') {
            turn_clockwise(sensor_data,82);
        } else if (c == 'n') {
            break;
        }
    }
}

