#include "Timer.h"
#include "lcd.h"
#include "ping.h"
#include "adc.h"
#include "servo.h"
#include "button.h"
#include "movement.h"
#include "uart.h"
#include "open_interface.h"
#include <math.h>

#define _PART1 0
#define _PART2 0
#define _PART3 1
#define _TEST 0

int calculate_distanceLOG(int adc_value)
{
//    return (int)(exp(-(adc_value - 4537) / 1006.0));
    return (int) (209.8 - 28.11 * log(adc_value));
}

int calculate_distancePOW(int adc_value)
{
    return 19839 * (pow(adc_value, -1.031));
}

int scanIR(int degrees)
{
    servo_move(degrees);
    return adc_read();
}

float scanPING(int degrees)
{
    servo_move(degrees);
    return ping_getDistance();
}

struct Object
{
    int distance;
    int startAngle;
    int endAngle;
    int angularWidth;
    double linearWidth;
    int middlePoint;
};

int main(void)
{
    oi_t *o_int = oi_alloc();
    oi_init(o_int);
    timer_init();
    lcd_init();
    servo_init();
    ping_init();
    adc_init();
    button_init();
    init_button_interrupts();

    extern volatile int button_event;
    extern volatile int button_num;
    extern volatile int clockwise;
#if _TEST
    servo_move(0);
    timer_waitMillis(1000);
    servo_move(90);
#endif

#if _PART1 //make drawing
    servo_move(0);
#endif

#if _PART2
    servo_move(90);
    while(1){
           timer_waitMillis(100);
           if(clockwise){
               lcd_printf("%d \nClockwise", abs(180 - button_Handler(button_num)));

           }
           else{
               lcd_printf("%d \nCounter Clockwise", abs(180 - button_Handler(button_num)));
           }

       }
#endif

#if _PART3
    int irVal;
    int distance;

    int avgArray[90];
    int i;
    int arrayIdx = 0;
    int objectListIdx = 0;
    int objectCount = 0;
    struct Object objectList[13];
    bool objMaking = false;
    char message[30] = "";
    //180 Degree Scan
    for (i = 0; i < 180; i += 2)
    {
        int temp = 0;
        timer_waitMillis(100);
        irVal = scanIR(abs(180 - i));
        distance = calculate_distancePOW(irVal);
        temp += distance;

        timer_waitMillis(100);

        irVal = scanIR(abs(180 - i));
        distance = calculate_distancePOW(irVal);
        temp += distance;

        timer_waitMillis(100);

        irVal = scanIR(abs(180 - i));
        distance = calculate_distancePOW(irVal);
        temp += distance;

        avgArray[arrayIdx] = temp / 3;
        arrayIdx++;
    }

    for (i = 0; i < 90; i++)
    {
        timer_waitMillis(100);
        int angle = i * 2;
        //        printf("Array[%d] Array1:%d Array2:%d Array3:%d AvgArray:%d\n", i, array1[i],
        //               array2[i], array3[i], avgArray[i]);

        //If the absolute difference between the next element and current element is less than 8, mark start angle of object
        timer_waitMillis(100);

        if ((avgArray[i] < 65) && objMaking == false)
        {
            objectList[objectCount].startAngle = angle;
            objMaking = true; //Flagging that an object has started
        }

        //If the absolute difference between the last and current element is greater than 8, mark end angle of object
    else if ((avgArray[i] >= 65)
            && (objMaking == true))
    {
        objectList[objectCount].endAngle = angle;
        objectCount++; //Increment the objectCount
        objMaking = false;//Flagging that an object has ended
    }
}

//Scan using Ping Sensor
//find linear width with start and end angles
int temp = 0;
for (temp = 0; temp < objectCount; temp++)
{
    objectList[temp].angularWidth = (objectList[temp].endAngle
            - objectList[temp].startAngle);
    timer_waitMillis(500);
}

int smallestWidthIdx = 0;
for (objectListIdx = 0; objectListIdx < objectCount; objectListIdx++) //Might be the issue!
{
    objectList[objectListIdx].middlePoint =
            (int) (objectList[objectListIdx].startAngle
                    + objectList[objectListIdx].endAngle) / 2; //calculate midpoint of object
    timer_waitMillis(500);
    objectList[objectListIdx].distance = scanPING(
            abs(180 - objectList[objectListIdx].middlePoint)); //use sonar sensor to find the distance
    timer_waitMillis(500);
    objectList[objectListIdx].linearWidth = (2
            * objectList[objectListIdx].distance)
            * tan((objectList[objectListIdx].angularWidth * (M_PI / 180)) / 2);
    timer_waitMillis(500);

    sprintf(message, "Object @ Angle:%d Distance:%d LWidth:%.2f\n",
            objectList[objectListIdx].middlePoint,
            objectList[objectListIdx].distance,
            objectList[objectListIdx].linearWidth);

    uart_sendStr(message); //debugging + send to PuTTY
    //Turn angular width into linear width

    if (objectList[objectListIdx].linearWidth
            < objectList[smallestWidthIdx].linearWidth)
    {
        smallestWidthIdx = objectListIdx; // Set smallestWidth based on condition
    }

}

servo_move(abs(180-objectList[smallestWidthIdx].middlePoint)); //Point towards the smallest width object

if (objectList[smallestWidthIdx].startAngle < 90) //If smallest object is within the right bounded area of the roomba
{
    turn_clockwise(o_int, (90 - objectList[smallestWidthIdx].startAngle - 8)); //Turn clockwise the difference between 90
    move_forward(o_int, (objectList[smallestWidthIdx].distance - 13));
}
else if (objectList[smallestWidthIdx].startAngle > 90) //If smallest object is within the left bounded area of the roomba
{
    turn_counterclockwise(o_int,
                          (objectList[smallestWidthIdx].startAngle - 90 - 8));
    move_forward(o_int, (objectList[smallestWidthIdx].distance - 13));
}
#endif
oi_free(o_int);
}
