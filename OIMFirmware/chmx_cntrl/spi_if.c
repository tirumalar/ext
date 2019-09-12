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

/* [] END OF FILE */

#define _MANID		 0xEF
#define PAGESIZE	 0x100

#define	MANID		 0x90
#define PAGEPROG     0x02
#define READDATA     0x03
#define FASTREAD	 0x0B
#define WRITEDISABLE 0x04
#define READSTAT1    0x05
#define READSTAT2	 0x35
#define WRITEENABLE  0x06
#define SECTORERASE  0x20
#define BLOCK32ERASE 0x52
#define CHIPERASE    0x60
#define SUSPEND      0x75
#define ID           0x90
#define RESUME       0x7A
#define JEDECID      0x9f
#define RELEASE      0xAB
#define POWERDOWN    0xB9
#define BLOCK64ERASE 0xD8

#define BUSY         0x01
#define WRTEN        0x02
#define SUS 		 0x40
#define DUMMYBYTE	 0xEE

#define arrayLen(x)  	(sizeof(x) / sizeof(*x))
#define lengthOf(x)  	(sizeof(x))/sizeof(byte)
#define maxAddress		capacity
#define NO_CONTINUE		0x00
#define PASS			0x01
#define FAIL			0x00


#include <system.h>
#define SPI_DEL 50
#include "altera_avalon_spi.h"

#define uint8 alt_u8
#define uint32 alt_u32

#define uint8_t alt_u8
#define uint32_t alt_u32

void DoSpiCMD( uint8 *wr_data, uint8 *rd_data, int write, int read)
{
    alt_avalon_spi_command(SPI_0_BASE,0,write,wr_data,read,rd_data,0);
    return;
}

void DoStartSpiCMD( uint8 *wr_data, uint8 *rd_data, int write, int read)
{
    alt_avalon_spi_command(SPI_0_BASE,0,write,wr_data,read,rd_data,ALT_AVALON_SPI_COMMAND_MERGE);

}

void SF_WriteEn()
{
    uint8 cmd;

    cmd = WRITEENABLE;
	DoSpiCMD(&cmd,0,1,0);
}
uint8 SF_readStat1(void)
    {
	uint8_t stat1=0xff;
    uint8_t cmd;

    cmd = READSTAT1;
	DoSpiCMD(&cmd,&stat1,1,1);
    return stat1;
}

uint8 SF_readStat2(void)
    {
	uint8_t stat1=0xff;
    uint8_t cmd;

    cmd = READSTAT2;
	DoSpiCMD(&cmd,&stat1,1,1);
    return stat1;
}


//#define DEBUG_FF
//#define FF_VERIFY
#define cli_printf printf

int SF_eraseSector(uint32_t address)
{
    uint8_t cmd[4],stat;
    int cnt=0;
 #ifdef DEBUG_FF
    cli_printf("erasing page %x \n",address);
#endif
    SF_WriteEn();

    cmd[0] = SECTORERASE;
    cmd[1] = address>>16;
    cmd[2] = address>>8;
    cmd[3] =0;
	DoSpiCMD(&cmd[0],0,4,0);

    while ((stat=SF_readStat1())&BUSY) cnt++;
#ifdef DEBUG_FF
     cli_printf("stat = %x %d\n",stat,cnt);
#endif
	return cnt;
}

#define PAGE_MASK  ~(0xff)
#define SECTOR_MASK ~(0xfff)
#define SECTOR_SIZE 0x1000
#define PAGE_SIZE   0x100


int SF_WritePage(uint8_t *data, uint32_t address)
{
    uint8_t cmd[4];
    int cnt=0;
 uint8  stat;
    address = address & PAGE_MASK;
    cmd[0] = PAGEPROG;
    cmd[1] = address>>16;
    cmd[2] = address>>8;
    cmd[3] =0;
#ifdef DEBUG_FF
    // cli_printf("writing page %x  %x\n",address,data[0]);
#endif
    SF_WriteEn();
	DoStartSpiCMD(&cmd[0],0,4,0);
    DoSpiCMD(data,0,PAGE_SIZE,0);
    while ((stat=SF_readStat1())&BUSY) cnt++;
#ifdef DEBUG_FF
    // cli_printf("stat = %x %d\n",stat,cnt);
#endif
    return cnt;
}

int SF_ClearStat()
{
    uint8_t cmd[4];
    cmd[0] = 1;
    cmd[1] = 0;
    cmd[2] = 0;
    cmd[3] =0;
    SF_WriteEn();
    DoSpiCMD(cmd,0,4,0);
    while (SF_readStat1()&BUSY);
    return 1;
}

#ifdef FF_VERIFY
unsigned char v_buff[4096];
#endif
int SF_WriteSector(uint8_t *data, uint32_t address)
{
    int s;
    int x;
    uint32_t add_orig = address;
    uint8_t *data_orig=data;
    #ifdef DEBUG_FF
     cli_printf("Writing sector %x \n",address);
#ifdef DEBUG_L2
     if (address==0)
     	for (x=0;x<300;x++)
     		printf("%2x ",data[x]);
#endif
  #endif
    SF_eraseSector(address);
    for (s=0;s < 16;s++)
       {
        SF_WritePage(data,address);
        address+= PAGE_SIZE;
        data+=PAGE_SIZE;
        }
#ifdef FF_VERIFY
   SF_ReadSector(v_buff, add_orig);

data=data_orig;
   if (memcmp(v_buff,data,4096)==0)
{
    #ifdef DEBUG_FF
      cli_printf("Verify good\n");
#endif
;
}
else
   for (s=0;s<4096;s++)
     if(data[s]!=v_buff[s])
        {
            cli_printf("Error wrote %x  read %x  at %x\n",data[s],v_buff[s],s);
            cli_printf("r-1 %x  r+1%x\n",v_buff[s-1],v_buff[s+1]);
         //   break;
        }
#endif
    return 1;
}

int SF_ReadSector(uint8_t *data, uint32_t address)
{
    uint8_t cmd[4];
    int x;

    #ifdef DEBUG_FF
        cli_printf("reading sector %x \n",address);
#endif
    address = address & SECTOR_MASK;
    cmd[0] = READDATA;
    cmd[1] = address>>16;
    cmd[2] = address>>8;
    cmd[3] =0;
	DoStartSpiCMD(&cmd[0],0,4,0);
    DoSpiCMD(0,data,0,SECTOR_SIZE);
#ifdef DEBUG_L2
    if (address==0)
    	for (x=0;x<300;x++)
    		printf("%2x ",data[x]);

    cli_printf("Data at 0 %x at 512 %x 256 &x\n",data[0],data[512],data[256]);
#endif
    return 1;
}

uint8 buffer[4100];
    uint8 s;

void sf_reset()
{
   uint8_t cmd;

    cmd = 0x66;
	DoSpiCMD(&cmd,0,1,0);
    cmd = 0x99;
	DoSpiCMD(&cmd,0,1,0);

}



int sf_test2(void)
{
    int x;

    sf_reset();
    s= SF_readStat1();
    s= SF_readStat2();

    //while (1)
    {
        buffer[0] = JEDECID;
        DoSpiCMD(buffer,&buffer[1],1,3);
        SF_WriteEn();
    s= SF_readStat1();
    s= SF_readStat2();
    }
    //SF_ClearStat();

    s= SF_readStat1();

    SF_WriteEn();
    s= SF_readStat1();
    s= SF_readStat1();
    s= SF_readStat2();
    s= SF_readStat2();


    for (x=0; x< 4000;x++)
      buffer[x] = x;
    SF_eraseSector(0);
    SF_WritePage(buffer,0);

    for (x=0; x< 400;x++)
      buffer[x] = 0;

    SF_ReadSector(buffer,0);
    return buffer[1];

}


