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
#include <project.h>

#define SET_V 40000
#define MAX_DUTY 66
//95




typedef struct
{
    unsigned char action;
    unsigned char hv_enable;
    unsigned char hv_volt;
    unsigned char led_enable;
    unsigned char trig_source;
    unsigned char i_set;
    unsigned char max_time;
    unsigned char min_v;
    unsigned char pwm;
    unsigned char max_duty;
    unsigned char version;
    unsigned char dvdt;
    unsigned char fill[19];
    
}SETTINGS;


#define LED_LEFT 1
#define LED_RIGHT 2
#define LED_FACE  4

volatile SETTINGS set;

volatile char in_trig=0;

#define I_TERM_DIV 1000 
#define P_TERM_DIV 200
int p_term_div=35;

int i_term_div =600;
#define MAX_I_TERM ((set.max_duty-5)*I_TERM_DIV)

// return the current voltage in mv
    static int i_term=0;
#define STORE_SIZE 100
int store[STORE_SIZE];
int st_idx=0;
static int last_val =0;

int DoVLoop(int v_set)
{
    int v;
    int err;
    int set_val;
    int val;
    static int old_v;
static int old_set_val=0;
    static int mv;
    // ADC_IsEndConversion(ADC_WAIT_FOR_RESULT);
    val = ADC_GetResult16(0) ;
    v = ADC_CountsTo_mVolts(0,val)*30;
    
    if (set.min_v>(v/1000))
       set.min_v=v/1000;
    
    // calculate the error in voltage
    err = v_set - v;
    
    // calculate the integral term
    i_term = i_term+err;
    if (i_term>MAX_I_TERM)
       i_term = MAX_I_TERM;
    if (i_term<0)
      i_term=0;
    
    // calculate the value
    set_val = i_term/i_term_div + err/p_term_div;
    
    if (set_val>(set.max_duty*4))
       set_val = (set.max_duty*4);
    if (set_val<1)
        set_val=1;
    set.pwm=set_val;
    
 #define PWM_BASELINE 55
 #define VIN  12000
 #define DV_RATE 1150
    set.max_duty=90;
    // this ensures that the voltage does not go up too fast
 // next version this should be done by limiting the inductor current 
    mv=(PWM_BASELINE+(v-VIN)/ (DV_RATE))*4;
    if (set_val>mv)
        set_val=mv;
    
    if (set_val>old_set_val)
       {
        //set_val = old_set_val+1;
        }
    old_v=v;
    PWM_CHARGE_WriteCompare(set_val);
    old_set_val=set_val;
    
    store[st_idx]=set_val;
    st_idx++;
    if (st_idx==STORE_SIZE) st_idx=0;
last_val = v;
    return v;
}
/*int i_i_term =0;
int i_i_term_div = 5;
int p_i_term_div = 1;
#define MAX_II_TERM 200
int DoILoop(int i_set)
{
    int v;
    int err;
    int set_val;
    int val;
    static int old_set_val;

    // ADC_IsEndConversion(ADC_WAIT_FOR_RESULT);
    val = ADC_GetResult16(0) ;
    v = ADC_CountsTo_mVolts(0,val)*30;
    
    if (set.min_v>(v/1000))
       set.min_v=v/1000;
    
    // calculate the error in voltage
    err = i_set - (v-last_val);
    last_val=v;
    // calculate the integral term
    i_i_term = i_i_term+err;
    if (i_i_term>MAX_II_TERM)
       i_i_term = MAX_II_TERM;
    if (i_i_term<0)
      i_i_term=0;
    
    // calculate the value
    set_val = i_i_term/i_i_term_div + err*p_i_term_div;
    
    if (set_val>(set.max_duty*4))
       set_val = (set.max_duty*4);
    if (set_val<1)
        set_val=1;
    set.pwm=set_val;
    
    
    if (set_val>old_set_val)
       {
        set_val = old_set_val+1;
        }
    PWM_CHARGE_WriteCompare(set_val);
    old_set_val=set_val;
    
    store[st_idx]=set_val;
    st_idx++;
    if (st_idx==STORE_SIZE) st_idx=0;
    return v;
}

*/
#define UA_PER_BIT 2.4
#define VI_RESISTOR 1000.0
#define CS_RESISTOR 0.2

#define V_PER_BIT UA_PER_BIT*VI_RESISTOR/1000000.0
#define DAC_I(i) ((float)i*CS_RESISTOR)/(V_PER_BIT)/10/2


volatile uint8 comBuff[32];

int t_count=0;
volatile uint8_t tick =0;
int set_v=0;



// HV task runs off a timer
// 12Mhz 40000 = 300 hz
#define TASK_TIME_MS  3.3333
#define RAMP_RATE_V_PER_MS  .05
#define RAMP_RATE  (1000*RAMP_RATE_V_PER_MS/TASK_TIME_MS)

//int rset=10;
void  HV_Task()
{
    static int last_v=0;
        if (set.hv_enable && (in_trig==0))
        {
            // this is done to minimize the ramp time
             //   val = ;PWM
      /*      last_v = ADC_CountsTo_mVolts(0, ADC_GetResult16(0))*30;
            if (last_v> set.hv_volt*1000)
               DoVLoop(set.hv_volt*1000);
            else
               DoILoop(2);
        */    
           
            if (set_v> set.hv_volt*1000)
                set_v=set.hv_volt*1000;
            
            if (set_v<set.hv_volt*1000)
                {
                set_v=set_v+set.dvdt;
                if (set_v >set.hv_volt*1000)
                    set_v=set.hv_volt*1000;
                }
            last_v=DoVLoop(set_v);
            
        }
        
     else
           {
            PWM_CHARGE_WriteCompare(1);
            set_v = ADC_CountsTo_mVolts(0, ADC_GetResult16(0))*30;
            i_term=0;
            //i_i_term=0;
            in_trig=0;
            store[st_idx]=-1;
            st_idx++;
            if (st_idx==STORE_SIZE)
               st_idx=0;
            }
}

CY_ISR(isr_MYTIMER_Interrupt)
{
    TIMER_ClearInterrupt(TIMER_INTR_MASK_TC);
    tick=1;
    t_count++;

}



// opamp 2 is the right one


uint8 GetTrig(void)
{
       if (
            (set.trig_source&LED_FACE && TRIG_FACE_Read()) ||
            (set.trig_source&LED_RIGHT && TRIG_RIGHT_Read()) ||
            (set.trig_source&LED_LEFT && TRIG_LEFT_Read())  
          )
        return 1;
    else
       return 0;
 }

void Init_bb_i2c(uint8 *space);
void DriveLeft(uint8 on)
{
    if (on)
      {
      VREF_A_SetDriveMode(VREF_A_DM_ALG_HIZ);
      Opamp_1_Start();
     }
    else
    {
      VREF_A_SetDriveMode(VREF_A_DM_STRONG);
      Opamp_1_Stop();
    }
}
void DriveRight(uint8 on)
{
    if (on)
      {
      VREF_B_SetDriveMode(VREF_B_DM_ALG_HIZ);
      Opamp_2_Start();
     }
    else
    {
      VREF_B_SetDriveMode(VREF_B_DM_STRONG);
      Opamp_2_Stop();
    }
}
void DriveFace(uint8 on)
{
    if (on)
      {
      VREF_C_SetDriveMode(VREF_C_DM_ALG_HIZ);
      Opamp_FACE_Start();
     }
    else
    {
      VREF_C_SetDriveMode(VREF_C_DM_STRONG);
      Opamp_FACE_Stop();
    }
}


void TrigTask()
{
    int x;
      if (GetTrig())
            {        
            int v = DAC_I((float)set.i_set);
            PWM_CHARGE_WriteCompare(1);
            i_term=0;
            in_trig=1;
            IDAC_1_SetValue(v);
            IDAC_2_SetValue(v);
 
            if (set.led_enable&LED_LEFT)
                DriveLeft(1);
            if (set.led_enable&LED_RIGHT)
                DriveRight(1);
            
            if (set.led_enable&LED_FACE)
                DriveFace(1);
            for (x=0;x<set.max_time*100;x++)
                if (GetTrig()==0)
                   break;
    //        in_trig=0;
            DriveFace(0);
            DriveLeft(0);
            DriveRight(0);
             while(GetTrig());
            // use the upper 4 bits to enable the toggle mode
            set.led_enable=(set.led_enable&0xf0) | ( (set.led_enable^(set.led_enable>>4))&0xf);
 
            }
    
}

void trigger_ISR(void)
{
    TrigTask();
}

#define VERSION 6
int main()
{
    CyGlobalIntEnable; /* Enable global interrupts. */
    int x;
    int16 val;
    int v;
    int err;
    int i_term=0;
    int set_val;
    int z;
    
    
    PWM_CHARGE_Start();
    IDAC_1_Start();
    IDAC_2_Start();

    ADC_Start();
    ADC_StartConvert();
    z = DAC_I(4.0);
    
    
    // initial settings
    
    set.hv_enable=0;
    set.hv_volt=16;
    set.max_time=5;
    set.led_enable=LED_LEFT;
    set.trig_source=LED_LEFT;
    set.i_set=4;
    set.max_duty=MAX_DUTY;
    set.version=VERSION;
    set.dvdt=100;
    z=10;
    Opamp_1_Start();
    IDAC_1_SetValue(200);
    
    VREF_A_SetDriveMode(VREF_A_DM_STRONG);
    VREF_B_SetDriveMode(VREF_B_DM_STRONG);
    z=DAC_I(4);
    IDAC_1_SetValue(z);
    IDAC_2_SetValue(z);

  

    
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    DriveLeft(0);
    DriveRight(0);
    DriveFace(0);
    ADC_Start();
    TIMER_Start();
    isr_TIMER_Start();
    isr_TIMER_SetVector(isr_MYTIMER_Interrupt);
    
    for (z=0;z< 32;z++)
     comBuff[z]=z*2+10;
    
    //EZI2C_EzI2CSetBuffer1(32, 16, &set.action);
    //EZI2C_Start();
    //EZI2C_EzI2CSetBuffer1(32, 16, &set.action);
   isr_bbi2c_Start();
   isr_bbi2c_Enable();
//    IDAC8_1_Start();
 
//    OLD_SDA_Write(1);
   // Init_bb_i2c((uint8 *)&set.action);
     EZI2C_EzI2CSetBuffer1(32, 16, &set.action);
    EZI2C_Start();
    EZI2C_EzI2CSetBuffer1(32, 16, &set.action);
    while(0)
    {
    CyDelayUs(60000);   
    CyDelayUs(60000);
    DriveLeft(1);
    CyDelayUs(4000);  
    DriveLeft(0);
    CyDelayUs(60000);   
    CyDelayUs(60000);
    DriveRight(1);
    CyDelayUs(4000);  
    DriveRight(0);
    }
    for(;;)
    {
        /* Place your application code here. */
        if (tick)
         {
          tick=0;
          HV_Task();
        }
      Control_Trig_Write(set.trig_source);
           
            
    }
}

/* [] END OF FILE */
