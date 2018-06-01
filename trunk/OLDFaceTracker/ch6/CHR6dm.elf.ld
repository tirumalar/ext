
/*
!!! Primary linker script file generated automatically !!!
This file contains the input and output files names
It also includes the secondary/main linker script that is specified in the Ride options (default or custom)
*/

SEARCH_DIR(".")
SEARCH_DIR("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease")
SEARCH_DIR("C:\Program Files (x86)\Raisonance\Ride\Lib\ARM")
INPUT("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\core_cm3.o")
INPUT("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\system_stm32f10x.o")
INPUT("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\misc.o")
INPUT("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\stm32f10x_adc.o")
INPUT("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\stm32f10x_dma.o")
INPUT("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\stm32f10x_exti.o")
INPUT("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\stm32f10x_flash.o")
INPUT("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\stm32f10x_gpio.o")
INPUT("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\stm32f10x_i2c.o")
INPUT("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\stm32f10x_rcc.o")
INPUT("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\stm32f10x_tim.o")
INPUT("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\stm32f10x_usart.o")
INPUT("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\main.o")
INPUT("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\chr6dm_ADC.o")
INPUT("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\chr6dm_config.o")
INPUT("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\chr6dm_HMC.o")
INPUT("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\chr6dm_packet_handler.o")
INPUT("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\chr6dm_startup.o")
INPUT("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\chr6dm_states.o")
INPUT("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\chr6dm_usart.o")
INPUT("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\matrix3x3.o")
INPUT("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\startup_stm32f10x_md.o")
EXTERN( __io_putchar )
EXTERN( _write )
GROUP(
"STM32x_io_putchar_thumb.a"
"std_sbrk_thumb.lib"
"smallprintf_thumb.a"
"libc.a"
"libm.a"

)
INCLUDE "C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\STM32F103_64K_20K_FLASH.ld"
OUTPUT("C:\Users\Caleb\Documents\Ride\projects\CHR6dmRelease\CHR6dm.elf")
