/*
 * Interface to the adp8860 charge pump for the eyelock RGB leds 
 * 
 * All functions return zero on success, negative value on failure.
 */

#ifndef ADP8860_H
#define ADP8860_H


#ifdef CMX_C1
#define I2C_BUS	"/dev/i2c-1"
#else
#define I2C_BUS	"/dev/i2c-3"
#endif

#define ADP8860_I2C_ADDR 	0x2A

#ifdef __cplusplus
extern "C" {
#endif

int rgb_led_read_reg(unsigned char reg, unsigned char *val);
int rgb_led_write_reg(unsigned char reg, unsigned char val);

int rgb_led_enable(int enable);
int rgb_led_set_color(unsigned char r, unsigned char g, unsigned char b);

#ifdef __cplusplus
}
#endif


#endif
