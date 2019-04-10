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

typedef struct
{
    unsigned char action;
    unsigned char chA;
    unsigned char chB;
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char mot_pos_lo;
    unsigned char mot_pos_hi;
    unsigned char mot_stat;
    unsigned char aud_set;
    unsigned char version;
    unsigned char plate_detect;
    unsigned char mot_min_speed_a;
    unsigned char mot_min_speed_d;
    unsigned char mot_accel;
    
    unsigned char fill[20];
    
}SETTINGS;
volatile SETTINGS set;

/* [] END OF FILE */
