

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
#include "movement.h"

#define _CALIBRATE 0
#define _TESTDISTANCE 0
#define _TESTAVERAGE 0
#define _PART1 1
#define _PART2 1
#define _PART3 0
#define _PART4 0
#define COEFFICIENT 306999000
#define EXPONENT -2.371


int averageInt(int arrayElement1, int arrayElement2, int arrayElement3)
{
    return (int)(arrayElement1+arrayElement2+arrayElement3)/3;
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
  int angularWidth;
};

int main(void)
{
    oi_t *o_int = oi_alloc();
    oi_init(o_int);
	timer_init();
	lcd_init();
	uart_init();
	cyBOT_init_Scan(0b0111);
	cyBOT_Scan_t sensor_data;

	right_calibration_value = 348250; //Calibration for CyBot 2041-09
	left_calibration_value = 1351000;


#if _CALIBRATE
	cyBOT_SERVO_cal();
#endif

#if _TESTDISTANCE
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
    int array3[90];
    int avgArray [90];
    int IRdistance;
    int i;
    int arrayIdx = 0;
    int objectListIdx = 0;
    int objectCount = 0;
    char message[30];
    struct Object objectList[10];
    bool objMaking = false;

    //180 Degree Scan
    for (i = 0; i < 180; i += 2)
    {
        cyBOT_Scan(i, &sensor_data);
        IRdistance = COEFFICIENT * (pow(sensor_data.IR_raw_val,EXPONENT));
        array1[arrayIdx] = IRdistance;

        cyBOT_Scan(i, &sensor_data);
        IRdistance = COEFFICIENT * (pow(sensor_data.IR_raw_val,EXPONENT));
        array2[arrayIdx] = IRdistance;

        cyBOT_Scan(i, &sensor_data);
        IRdistance = COEFFICIENT * (pow(sensor_data.IR_raw_val,EXPONENT));
        array3[arrayIdx] = IRdistance;

        arrayIdx++;
    }
//average the 3 arrays
        for (i = 0; i < 90; i++)
        {
            avgArray[i] = averageInt(array1[i], array2[i],array3[i]);
        }


    for (i = 0; i < 90; i++)
    {
        int angle = i * 2;
//        printf("Array[%d] Array1:%d Array2:%d Array3:%d AvgArray:%d\n", i, array1[i],
//               array2[i], array3[i], avgArray[i]);

        if ((abs((avgArray[i + 1] - avgArray[i])) < 6) && (avgArray[i] < 70) && objMaking == false)
        {
            objectList[objectCount].startAngle = angle;
            objMaking = true;
        }

        if ((abs((avgArray[i] - avgArray[i - 1])) > 6) && (avgArray[i] < 70) && objMaking == true)
        {
            objectList[objectCount].endAngle = angle;
            objectCount++;
            objMaking = false;
        }
    }

    //Scan using Ping Sensor
    //find linear width with start and end angles
    int temp = 0;
    for (temp = 0; temp < objectCount; temp++)
    {
        objectList[temp].angularWidth = (objectList[temp].endAngle
                - objectList[temp].startAngle);
    }

    int smallestWidthIdx = 0;
    for (objectListIdx = 0; objectListIdx < objectCount; objectListIdx++)
    {
        int midpointAngle = (objectList[objectListIdx].startAngle
                + objectList[objectListIdx].endAngle) / 2;
        cyBOT_Scan(midpointAngle, &sensor_data);
        objectList[objectListIdx].distance = sensor_data.sound_dist;
        sprintf(message, "Object @ Angle:%d Distance:%d\n",
                objectList[objectListIdx].startAngle,
                objectList[objectListIdx].distance); //TODO LOOK AT THIS
        uart_sendStr(message);
        if (objectListIdx + 1 < objectCount) // Check bounds
        {
            if (objectList[objectListIdx + 1].angularWidth > objectList[objectListIdx].angularWidth)
            {
                smallestWidthIdx = objectListIdx; // Set smallestWidth based on condition
            }
        }
     cyBOT_Scan(objectList[smallestWidthIdx].startAngle, &sensor_data);
    }


//	    oi_free(o_int);

#endif

#if _PART2
    if (objectList[smallestWidthIdx].startAngle < 90)
    {
        turn_clockwise(o_int, (90 - objectList[smallestWidthIdx].startAngle));
        move_forward(o_int,(objectList[smallestWidthIdx].distance-7));
    }
    else if (objectList[smallestWidthIdx].startAngle > 90)
    {
        turn_counterclockwise(o_int, (90 - objectList[smallestWidthIdx].startAngle));
        move_forward(o_int,(objectList[smallestWidthIdx].distance-7));
    }

#endif

#if _PART3
    //TODO
#endif

#if _PART4
    //TODO
#endif
}
