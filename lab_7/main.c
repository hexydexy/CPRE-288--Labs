

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
#define _TESTTURN 0
#define _PART1 1
#define _PART2 2
#define _PART3 0
#define _PART4 0
#define COEFFICIENT 306999000
#define EXPONENT -2.371


int averageInt(int arrayElement1, int arrayElement2, int arrayElement3)
{
    return (int)(arrayElement1+arrayElement2+arrayElement3)/3;
}

int linearWidth(int theta, int distance)
{
    return (int)2*distance*tan((theta*(M_PI/180))/2);
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
  int linearWidth;
  int middlePoint;
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

	right_calibration_value = 348250; //Calibration for CyBot 2041-09 USE THIS ONE
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

#if _TESTTURN
//turn_clockwise(o_int, 82);
    move_forward(o_int, 50);

#endif

#if _PART1
    int avgArray [90];
    int IRdistance;
    int i;
    int arrayIdx = 0;
    int objectListIdx = 0;
    int objectCount = 0;
    char message[30];
    struct Object objectList[13];
    bool objMaking = false;

    //180 Degree Scan
    for (i = 0; i < 180; i += 2)
    {
        int temp = 0;
        cyBOT_Scan(i, &sensor_data);
        IRdistance = COEFFICIENT * (pow(sensor_data.IR_raw_val, EXPONENT));
        temp += IRdistance;

        cyBOT_Scan(i, &sensor_data);
        IRdistance = COEFFICIENT * (pow(sensor_data.IR_raw_val, EXPONENT));
        temp += IRdistance;

        cyBOT_Scan(i, &sensor_data);
        IRdistance = COEFFICIENT * (pow(sensor_data.IR_raw_val, EXPONENT));
        temp += IRdistance;

        avgArray[arrayIdx] = temp / 3;
        arrayIdx++;
    }

    for (i = 0; i < 90; i++)
    {
        int angle = i * 2;
//        printf("Array[%d] Array1:%d Array2:%d Array3:%d AvgArray:%d\n", i, array1[i],
//               array2[i], array3[i], avgArray[i]);

        //If the absolute difference between the next element and current element is less than 6, mark start angle of object
        if ((abs((avgArray[i + 1] - avgArray[i])) > 7) && (avgArray[i] < 70) && objMaking == false)
        {
            objectList[objectCount].startAngle = angle;
            objMaking = true; //Flagging that an object has started
        }

        //If the absolute difference between the last and current element is greater than 6, mark end angle of object
        if ((abs((avgArray[i] - avgArray[i - 1])) > 7) && (avgArray[i] < 70) && objMaking == true)
        {
            objectList[objectCount].endAngle = angle;
            objectCount++; //Increment the objectCount
            objMaking = false; //Flagging that an object has ended
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
        objectList[objectListIdx].middlePoint = (int) (objectList[objectListIdx].startAngle
                + objectList[objectListIdx].endAngle) / 2; //calculate midpoint of object
        cyBOT_Scan(objectList[objectListIdx].middlePoint, &sensor_data);
        objectList[objectListIdx].distance = sensor_data.sound_dist; //use sonar sensor to find the distance
        timer_waitMillis(500);
        objectList[i].linearWidth =  linearWidth(objectList[i].angularWidth,objectList[i].distance);

        sprintf(message, "Object @ Angle:%d Distance:%d LWidth:%d\n",
        objectList[objectListIdx].middlePoint,
        objectList[objectListIdx].distance,
        objectList[objectListIdx].linearWidth);

        uart_sendStr(message); //debugging + send to PuTTY
        //Turn angular width into linear width

        if (objectListIdx + 1 < objectCount) // Check bounds
        {
            if (objectList[objectListIdx].linearWidth < objectList[smallestWidthIdx].linearWidth)
            {
                smallestWidthIdx = objectListIdx; // Set smallestWidth based on condition
            }
        }
     cyBOT_Scan(objectList[smallestWidthIdx].middlePoint, &sensor_data);
    }


//	    oi_free(o_int);

#endif

#if _PART2
    if (objectList[smallestWidthIdx].startAngle < 90) //If smallest object is within the right bounded area of the roomba
    {
        turn_clockwise(o_int, (90 - objectList[smallestWidthIdx].startAngle - 8)); //Turn clockwise the difference between 90
        move_forward(o_int,(objectList[smallestWidthIdx].distance-12));
    }
    else if (objectList[smallestWidthIdx].startAngle > 90) //If smallest object is within the left bounded area of the roomba
    {
        turn_counterclockwise(o_int, (objectList[smallestWidthIdx].startAngle - 90 - 8));
        move_forward(o_int,(objectList[smallestWidthIdx].distance-12));
    }

#endif

#if _PART3
    //TODO
#endif

#if _PART4
    //TODO
#endif
}
