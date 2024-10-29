

/**
 * main.c
 *
 * @author Winson Vetsavong & Miles Nichols
 * @date 10/17/2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "open_interface.h"
#include "timer.h"
#include "lcd.h"
#include "cyBot_Scan.h"
#include <inc/tm4c123gh6pm.h>
#include "driverlib/interrupt.h"
#include "uart.h"

#define _CALIBRATE 1
#define _TESTDISTANCE 0
#define _TESTAVERAGE 0
#define _PART1 1
#define _PART2 0
#define _PART3 0
#define _PART4 0
#define COEFFICIENT 19936200
#define EXPONENT -1.892


int averageInt(int arrayElement1, int arrayElement2)
{
    return (arrayElement1+arrayElement2)/2;
}

//int* average(int* intArray1, int* intArray2, int size) {
//    int i;
//    int* avgArray;
//
//    avgArray = (int*)malloc(size * sizeof(int));
//
//    for(i = 0; i < size; i++)
//    {
//        avgArray[i] = (intArray1[i]+intArray2[i])/2;
//    }
//    return avgArray;
//}

struct Object{
  int distance;
  int startAngle;
  int endAngle;
};

int main(void)
{
//    oi_t *o_int = oi_alloc();
//    oi_init(o_int);
	timer_init();
	lcd_init();
	uart_init();
	cyBOT_init_Scan(0b0111);
	cyBOT_Scan_t sensor_data;

	right_calibration_value = 253750; //Calibration for CyBot 11
	left_calibration_value = 1256500;


#if _CALIBRATE
	cyBOT_SERVO_cal();
#endif

#if _TEST
    char message[50];
    while (true)
    {
        int i = 0;
        int distance;
        char input = uart_receive();
        if (input == 'm')
        {
            cyBOT_Scan(90, &sensor_data);
            distance = COEFFICIENT * (pow(sensor_data.IR_raw_val,EXPONENT));
            sprintf(message, "Raw IR: %d\nDistance[cm]: %d\n",
                    sensor_data.IR_raw_val, distance);
            lcd_printf(message);
        }
        while (message[i] != '\n')
        {
            uart_sendChar(message[i]);
            i++;
        }
        uart_sendChar('\n');
        input = '\0';
    }
#endif

#if _TESTAVERAGE
    int array1 [] = {4,4,4,4,4,4};
    int array2 [] = {2,1,4,5,7,2};
    int avgArray [sizeof(array1)];
    int i;
    int size = 2;

    for(i = 0; i < 6; i++)
    {
        avgArray[i] = averageInt(array1[i],array2[i]);
    }

    for(i =0; i < 6; i++)
    {
        printf("%d\n", avgArray[i]);
    }

#endif

#if _PART1
    int array1[90];
    int array2[90];
    int avgArray [90];
    int distance;
    int i;
    int arrayIdx = 0;
    int objectListIndx = 0;
    int objectCount = 0;
    char message[50];
    struct Object objectList[4];

    //180 Degree Scan
    for (i = 0; i < 180; i += 2)
    {
        cyBOT_Scan(i, &sensor_data);
        distance = COEFFICIENT * (pow(sensor_data.IR_raw_val,EXPONENT));
        array1[arrayIdx] = distance;

        cyBOT_Scan(i, &sensor_data);
        distance = COEFFICIENT * (pow(sensor_data.IR_raw_val,EXPONENT));
        array2[arrayIdx] = distance;

        arrayIdx++;
    }
//average the two arrays
    for(i = 0; i < 90; i++)
    {
        avgArray[i] = averageInt(array1[i],array2[i]);
    }

    for (i = 0; i < 90; i++)
    {
        int angle = i * 2;
        printf("Array[%d] Array1:%d Array2:%d\n", i, array1[i], array2[i]);

        if ((avgArray[i + 1] - avgArray[i]) > 5)
        {
            objectList[i].startAngle = angle;
            objectCount++;
        }

        if ((avgArray[i] - avgArray[i + 1]) > 5)
        {
            objectList[i].endAngle = angle;
        }

        //Scan using Ping Sensor
        for (objectListIndx = 0; objectListIndx < objectCount; objectListIndx++)
        {
            int midpointAngle = (objectList[objectListIndx].startAngle+objectList[objectListIndx].endAngle) / 2;
            cyBOT_Scan(midpointAngle, &sensor_data);
            objectList[objectListIndx].distance = sensor_data.sound_dist;
        }



	    sprintf(message, "Object @ Angle: %d Distance:%d\n",angle, objectList[objectListIndx].distance);
	    uart_sendStr(message);


	}

#endif

#if _PART2
	//TODO
#endif

#if _PART3
    //TODO
#endif

#if _PART4
    //TODO
#endif
}
