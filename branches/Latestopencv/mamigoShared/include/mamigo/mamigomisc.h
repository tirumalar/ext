#ifndef MAMIGO_MISC
#define MAMIGO_MISC 1

#ifndef MAMIGO_VERIFY
#include <linux/kernel.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/blackfin.h>
#include <asm/dma.h>
#include <linux/kernel.h>
#include <asm/cacheflush.h>
#include <asm-blackfin/mach/dma.h>

#define LOG(fmt, arg...) printk("%li : ",MICROSEC());printk(fmt, ## arg);
#else
#include <sys/time.h>
#include <time.h>
int do_gettimeofday(struct timeval *tv);
int gradient_way1(unsigned char *buffer, int width, int height);
int noOp(unsigned char *buffer, int width, int height);
#endif


struct int_result_info{
	unsigned char *src;
	int what_res;
	long what_time;
};

unsigned long get_FreeAddressFromBank( char Bank);

unsigned long MICROSEC(void);
unsigned long tvdelta(struct timeval *t1, struct timeval *t2);

int fill_buffer_address(unsigned long * fill_queue, int count, unsigned int size);
int fill_3_banks_buffer_address(unsigned long *fill_queue, int count, unsigned int size);
int fill_single_bank_buffer_address(unsigned long *fill_queue, int count, unsigned int size);

void add_result(unsigned long Handle_to_Result, unsigned char * src, int what_res, long what_time );

int gradient_comp_mid(unsigned char *buffer, int width, int height);
int gradient_calc_without_mid_cols(unsigned char *buffer, int width, int height);
int gradient_way2(unsigned char *buffer, int width, int height);

#endif
