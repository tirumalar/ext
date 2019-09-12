#include "basicli.h"
#include "stdlib.h"
#include "stdio.h"
#include <system.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "priv/alt_file.h"
#ifdef ALT_USE_DIRECT_DRIVERS
#include "system.h"
#include "sys/alt_driver.h"
#include "sys/alt_stdio.h"
#include "priv/alt_file.h"
#include "unistd.h"
#endif

/* Nichestack definitions */
#include "ipport.h"
#include "libport.h"
#include "osport.h"
#include "b_timer.h"
#include "sys/ioctl.h"
#include "net.h"


#include "io.h"
#include "b_timer.h"

#include "io.h"
#include "altera_avalon_pio_regs.h"

#include "altera_eth_tse_regs.h"
#include "user_flash.h"
#include "simple_socket_server.h"
#include "data_Store.h"
#include "spi_if.h"


#include "accel.h"

#include "fixed_brd.h"

#include "led_control.h"
volatile int g_cam_set=0;
volatile int g_time_set=20;
#include "ffs/ff.h"
extern FATFS FatFs;		/* FatFs work area needed for each volume */

void f_formt(void)
    {
    BYTE work[FF_MAX_SS]; /* Work area (larger is better for processing time) */
    UINT bw;
    f_mount(&FatFs, "", 0);		/* Give a work area to the default drive */
    f_mkfs("", FM_ANY, 0, work, sizeof work);
    f_mount(&FatFs, "", 0);		/* Give a work area to the default drive */
    cli_printf("Format Complete..");

    }
void f_load_snd(int idx, char *p)
    {
    FIL Fil;			/* File object needed for each open file */
    BYTE work[FF_MAX_SS]; /* Work area (larger is better for processing time) */
    UINT bw;
    int err;
    cli_printf("Reading %s\n",p);
    if (f_open(&Fil, p, FA_READ) != FR_OK)
	{
	cli_printf("File not found\n");
	return;
	}
    if ((err=f_read(&Fil, a_store[idx].audio, AUDIO_STORE_SIZE, &bw))!=FR_OK)
	  cli_printf("Error reading %d\n",err);
    a_store[idx].len=bw;
    cli_printf("Read %d bytes\n",bw);
    f_close(&Fil);
    }
void load_flash(void)
{
	//user_flash_read(&shadowVals[NUM_FLASH_START], NUM_FLASH_SIZE*sizeof(int));
}
void save_flash(void)
{
	// user_flash_write(&shadowVals[NUM_FLASH_START], NUM_FLASH_SIZE*sizeof(int));
}


void save(void)
{
	char buffer[USER_PROG_SIZE];
	save_all(buffer, USER_PROG_SIZE);

	user_prog_write(buffer,USER_PROG_SIZE);
}
void clear_flash(void)
{
	char buffer[USER_PROG_SIZE];
	buffer[0]=0;

	user_prog_write(buffer,USER_PROG_SIZE);

}
#include "cam.h"
#include "boot_host/boot_host.h"
#define SEC_SIZE 4096
char tb[SEC_SIZE];
char *get_img_buffer(void);

extern unsigned long g_prog_base;
extern int           g_prog_address ;
extern int           g_prog_busy_val;
void psoc_prog(int psoc_id)
    {
    int ret_val;

    if (psoc_id==1)
	{
	g_prog_busy_val = 0xa2;
	g_prog_base = I2C_OPENCORES_2_BASE;
	g_prog_address = FIXED_ID;

	fixed_write( 0, FIXED_ACTION_REBOOT);
	cli_printf("reseting \n");
	for (ret_val=0; ret_val<100;ret_val++)
	    usleep(500);
	cli_printf("Programming \n");

	ret_val = program_psoc(data_store_info(DATA_STORE_IMAGE, NULL),
		data_store_get_cur_len(),
		0);
	cli_printf("Returned %d\n",ret_val);
	}
    if (psoc_id==2)
	{
	g_prog_busy_val = 0xb2;
	g_prog_base = I2C_OPENCORES_0_BASE;
	g_prog_address = PSOC_ID;
	psoc_write( 0, PSOC_ACTION_REBOOT);
	cli_printf("reseting \n");
	for (ret_val=0; ret_val<100;ret_val++)
	    usleep(500);
	cli_printf("Programming \n");

	ret_val = program_psoc(data_store_info(DATA_STORE_IMAGE, NULL),
		data_store_get_cur_len(),
		0);
	cli_printf("Returned %d\n",ret_val);
	}
    }
void flash_prog(void)
    {
    int bytes_to_store=current_store_len;
    int write_address=0,x,z;
    char *p=get_img_buffer();


       while (bytes_to_store>0)
	{
	SF_WriteSector(p, write_address);
#if 0
	SF_ReadSector(tb, write_address);
	if (memcmp(tb,p,SEC_SIZE)!=0)
	    {
	    for (z=0;z<SEC_SIZE/8;z++)
		 cli_printf("Err %X %d %08x/%08x\n",write_address,z,tb[z],p[z]);
	    return;
	    }
#endif
	p+=SEC_SIZE;
	write_address+=SEC_SIZE;
	bytes_to_store-=SEC_SIZE;
	cli_printf("Writing %08x/%08x",write_address,bytes_to_store);
	}
    }
void send_udp(int cam);
void grab_send(int cam);
void CamSetTrig(int val);
void grab(int cam);
void cli_close(void);
int wcr(int cam, int reg, int val);
int rcr(int cam, int reg);
void cam_set_seed(int val);

extern char g_AudioMode ;

extern int aud_store_len;
extern char  aud_store[];

int aud_play_array_low( short *aud_buff,int samples);
void set_audio(int v)
{
/*	if (v==1)
	    {
	    aud_store_len=0;
	    g_AudioMode=v;
	    }
	if (v==2)
	    {
	    aud_play_array_low(aud_store,aud_store_len/2);
	    }
	    */
}


int g_audio_set=3;
void fixed_aud_set(int val)
    {
    g_audio_set=val;
    }
void play_snd(int slot)
    {
    int g_cam_set_store = g_cam_set;
    int g_time_set_store = g_time_set;

    cli_printf("playing %d %x %d at level %d",slot,a_store[slot].audio,a_store[slot].len/2,g_audio_set);
    g_cam_set=0;
    fixed_aud_set_low(g_audio_set);
    aud_play_array_low((short *)a_store[slot].audio,a_store[slot].len/2);
    fixed_aud_set_low(0);
    g_cam_set=g_cam_set_store;
    g_time_set=g_time_set_store;
    }

void load_user_code()
{
	char buffer[USER_PROG_SIZE];
	int x;
	user_prog_read(buffer, USER_PROG_SIZE);
	while ((x<USER_PROG_SIZE-1) && buffer[x]!=0xff)
		x++;
	buffer[x]=0;
	load_all(buffer);
}



#include "fixed_brd.h"

void fx_home(void)
{
	fixed_motor_home();
}
void dac_test(void);



void set_cam_mode(int a ,int b)
    {
    g_cam_set=a;
    g_time_set=b;
    }
void fx_abs(int steps)
{
	fixed_motor_abs(steps);
}
void fixed_motor_rel(int steps);
void fx_rel(int steps)
    {
 //   printf("Rel %d\n",steps);
    fixed_motor_rel( steps);
//    printf("Rel done\n");
    }

int tof_measure(void);
void tof(void)
    {
    cli_printf("%d",tof_measure());
    }
void set_cam_port_num(int cam, int port);

void accel(void)
    {
    float dat[14];
    int val;
    val = fixed_read_plate();

    LSM6DS3Read(dat);

    cli_printf("%2.3f %2.3f %2.3f %2.3f %d",dat[IDX_AX],dat[IDX_AY],dat[IDX_AZ],
	    atan2(dat[IDX_AY],dat[IDX_AZ]) *180.0/PI,val);
    }
void accel_temp(void)
    {
    float dat[14];
    LSM6DS3Read(dat);
    cli_printf("%3.3f",dat[IDX_TEMP]);
    }
extern int g_fcount;
extern int g_f_state;
void fr_stat(void)
    {
    cli_printf("%d %d\n",g_fcount,g_f_state);
    }
int tof_init(void);

int psoc_read_version(void);
#define FIRMWARE_VERSION_MAJOR 1
#define FIRMWARE_VERSION_MINOR 8
void ver(void)
    {
    cli_printf("FPGA Firmware Build: %s %s",__DATE__,__TIME__);
    cli_printf("FPGA VERSION   :%d.%d", FIRMWARE_VERSION_MAJOR,FIRMWARE_VERSION_MINOR);
    cli_printf("Fixed board Verson :%d",fixed_version());
    cli_printf("Cam Psoc Version   :%d", psoc_read_version());

    }


int fx_plate()
    {
    int val;
    val = fixed_read_plate();
    if (val)
	 val =1;
    cli_printf("Plate=%d",val);
    }
BLINK_START
#if WITH_LISTV
    INT_BLINK(listv,0) BLINK_DONE
#endif
#if WITH_LISTF
    INT_BLINK(listf,0)  BLINK_DONE
#endif

   // VOID_BLINK(b_on_time,3) BLINKINT, BLINKINT, BLINKSTRING BLINK_DONE
  //  VOID_BLINK(menu,0) BLINK_DONE
   // VOID_BLINK(send_udp,1) BLINKINT BLINK_DONE
    VOID_BLINK(CamSetTrig,1) BLINKINT BLINK_DONE
  //  VOID_BLINK(grab,1) BLINKINT BLINK_DONE
   // VOID_BLINK(grab_send,1) BLINKINT BLINK_DONE
   // VOID_BLINK(cli_close,0) BLINK_DONE
    INT_BLINK(rcr,2) BLINKINT, BLINKINT  BLINK_DONE
    INT_BLINK(wcr,3) BLINKINT,  BLINKINT, BLINKINT  BLINK_DONE
    INT_BLINK(psoc_read,1) BLINKINT  BLINK_DONE
    INT_BLINK(psoc_write,2) BLINKINT,  BLINKINT  BLINK_DONE
   // VOID_BLINK(save,0) BLINK_DONE
    VOID_BLINK(fixed_set_rgb,3) BLINKINT, BLINKINT, BLINKINT BLINK_DONE
    VOID_BLINK(fixed_set_rgbm,4) BLINKINT, BLINKINT, BLINKINT, BLINKINT BLINK_DONE

    VOID_BLINK(fx_home,0) BLINK_DONE
    VOID_BLINK(fx_abs,1) BLINKINT BLINK_DONE
    VOID_BLINK(fx_rel,1) BLINKINT BLINK_DONE
  //  VOID_BLINK(dac_test,0) BLINK_DONE
    VOID_BLINK(set_audio,1) BLINKINT BLINK_DONE
    VOID_BLINK(set_cam_mode,2) BLINKINT ,BLINKINT BLINK_DONE
    VOID_BLINK(fixed_aud_set,1) BLINKINT BLINK_DONE
   // VOID_BLINK(hprint,1) BLINKINT BLINK_DONE
    VOID_BLINK(tof,0) BLINK_DONE
    VOID_BLINK(accel,0) BLINK_DONE
    VOID_BLINK(fr_stat,0) BLINK_DONE
    // VOID_BLINK(tof_init,0) BLINK_DONE

    VOID_BLINK(play_snd,1) BLINKINT BLINK_DONE
    VOID_BLINK(data_store_set,1)BLINKINT BLINK_DONE
    VOID_BLINK(set_cam_port_num,2)  BLINKINT ,BLINKINT BLINK_DONE
    VOID_BLINK(flash_prog,0) BLINK_DONE
    VOID_BLINK(ver,0) BLINK_DONE
    VOID_BLINK (accel_temp,0) BLINK_DONE
    VOID_BLINK (fx_mot_set,3) BLINKINT ,BLINKINT,BLINKINT BLINK_DONE
    INT_BLINK (fx_plate,0) BLINK_DONE
    VOID_BLINK (psoc_prog,1) BLINKINT BLINK_DONE
    VOID_BLINK (cam_set_seed,1) BLINKINT BLINK_DONE
    VOID_BLINK (f_load_snd,2)  BLINKINT ,BLINKSTRING BLINK_DONE
    VOID_BLINK (f_formt,0) BLINK_DONE

    BLINK_END

extern    int d_lev ;
OS_EVENT *parse_Mutex;
void cli_mutex_pend()
    {
    INT8U return_code;
    OSMutexPend(parse_Mutex, 0, &return_code);
    if (return_code)
       printf("error cli mutex %d\n",return_code);
    }
void cli_mutex_release()
    {
    OSMutexPost(parse_Mutex);
    }
int basicli_init(void)
{
    INT8U err;
	parse_Mutex= OSMutexCreate(PARSE_PROG_MUTEX_PRI, &err);
	printf("parse_Mutex mutex err %d\n",err);

	call_func("", NULL, -2);
	load_flash();
	load_user_code();
	parse_prog("boot ");

//	BLINK_VAR_INT(d_lev);
	ftos = 0; /* initialize the FOR stack index */
	gtos = 0; /* initialize the GOSUB stack index */
    return 0;
}
