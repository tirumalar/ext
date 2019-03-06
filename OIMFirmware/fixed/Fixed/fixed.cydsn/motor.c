/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include "project.h"
#include "settings.h"

int mot_start;
int mot_decel_start;
int mot_target,half_dist;
uint8 mot_active =0;
uint8 mot_homed=0;
int mot_pos;

#define ACCEL (set.mot_accel*10)
#define MAX_V 250
#define MIN_SPEED_ACCEL set.mot_min_speed_a//100
#define MIN_SPEED_DECEL set.mot_min_speed_d//100/2
#define HOME_SPEED set.mot_home_speed

#define AMPS_PER_BIT_DAC .0000024
#define DAC_R 4700
#define VOLTS_PER_BIT_DAC (AMPS_PER_BIT_DAC * DAC_R)
#define VOLTS_PER_BIT .00024
#define AMPS_PER_BIT VOLTS_PER_BIT

#define HOME_I 20
#define RUN_I 60
#define RUN_I_COUNTER_SPRING 60
#define MAX_ACTIVE 500000
#include "math.h"
#include "motor.h"

int MotGetPos(void)
{
    return mot_pos;
}

#define TO_HOME_STEP 50000
 int last_count;

void MotSetTrip(int val)
{
    int v1;
    v1= val>255?255:val;
    IDAC_1_SetValue(v1);
    IDAC_2_SetValue(val-v1);
}
void MotHome()
{
    int x,lp=0;
    
    int to=TO_HOME_STEP;
    int count;
    MotSetTrip(HOME_I);
   
    PWM_M_WriteCompare1(HOME_SPEED);
    CW_Write(1);
    STEP_COUNT_WriteCounter(1000);
   // while(((x=STEP_COUNT_ReadCounter())<10008) && (to--))
   //     CyDelayUs(100);
 #define MIN_HOMING_MOVES 10
        
    last_count=STEP_COUNT_ReadCounter()+MIN_HOMING_MOVES;
 
    //add timeout for homing
    while (1)
        {
         to--;
         CyDelayUs(50000);
         lp++;
        // wait some amount of time before we detect stop condition
         if (lp>2)
            {
             // stop if we over current
            //    if (1)
            if (Comp_ISENSE_GetCompare()==1)
               {
                mot_active=0;
                PWM_M_WriteCompare1(0);
                mot_pos=0;
                mot_homed=1;
                break;
               }
            // stop if we are no longer moving
           count=STEP_COUNT_ReadCounter();
           if ((count-last_count)< MIN_HOMING_MOVES)
                {
                mot_active=0;
                PWM_M_WriteCompare1(0);
                mot_pos=0;
                mot_homed=1;
                break;
                }
            last_count = count;           
            }
        if (to<0)
           {
            mot_pos=0;
            PWM_M_WriteCompare1(0);
            break;
            }
        }
}
unsigned short isqrt(unsigned long a) {
    unsigned long rem = 0;
    int root = 0;
    int i;

    for (i = 0; i < 16; i++) {
        root <<= 1;
        rem <<= 2;
        rem += a >> 30;
        a <<= 2;

        if (root < rem) {
            root++;
            rem -= root;
            root++;
        }
    }

    return (unsigned short) (root >> 1);
}
int active_count=0;

uint8 MotMoveRel(int steps)
{
    if ( mot_active)
      return 0;
    if (steps==0)
      return 1;
    active_count=0;
     IDAC_1_SetValue(RUN_I);
    mot_pos+=steps;
    if (steps>0)
    {
       CW_Write(0);
    MotSetTrip(RUN_I_COUNTER_SPRING);
    }
    else
       {
        CW_Write(1);
        steps=-steps;
        MotSetTrip(RUN_I);
       }
    
    STEP_COUNT_WriteCounter(10000);
    mot_start=STEP_COUNT_ReadCounter();
    mot_target = mot_start +steps;
    half_dist = mot_start +steps/2;
    active_count=0;
    mot_active=1;
    return 1;
}
int last_p, last_x;
int MotTask()
{
   int x,p;
    
   if (mot_active)
       {
        active_count++;
        x=STEP_COUNT_ReadCounter();
        if ((active_count>MAX_ACTIVE)||(Comp_ISENSE_GetCompare()==1))
           {
            mot_active=0;
            PWM_M_WriteCompare1(0);
            if ((Comp_ISENSE_GetCompare()==1))
               {
                void ShowMotError();
                ShowMotError();
                }
            
            return 0;
           }
       
       if (x<half_dist)
            p = isqrt((x-mot_start)*ACCEL*2)+MIN_SPEED_ACCEL;
       else
               if (mot_target-x>0)
                    p = isqrt((mot_target-x)*ACCEL*2)+MIN_SPEED_DECEL;
               else
                    p=0;
       if (p>MAX_V)
          p=MAX_V;
       PWM_M_WriteCompare1(p);
       if (x>=mot_target)
          {
          mot_active=0;
          PWM_M_WriteCompare1(0);
          }
         last_p=p;
         last_x=x;
        return 1;
     }

    return 0;
}

/* [] END OF FILE */
