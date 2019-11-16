/*
 *
 *
 * I2C include set for *MT9P031*
 *
 *
 *
 */


#include "ppifcd_i2c.h"


extern int update(unsigned char, unsigned short);

extern int reset_cam(void);

extern int restart_cam(void);

extern int update_all(struct camconfig);

extern struct camconfig getcamconfig();
