/*
 * system.h - SOPC Builder system and BSP software package information
 *
 * Machine generated for CPU 'nios2_qsys_0' in SOPC Builder design 'tb2_qs'
 * SOPC Builder design path: ../../tb2_qs.sopcinfo
 *
 * Generated: Mon Apr 15 10:52:11 EDT 2019
 */

/*
 * DO NOT MODIFY THIS FILE
 *
 * Changing this file will have subtle consequences
 * which will almost certainly lead to a nonfunctioning
 * system. If you do modify this file, be aware that your
 * changes will be overwritten and lost when this file
 * is generated again.
 *
 * DO NOT MODIFY THIS FILE
 */

/*
 * License Agreement
 *
 * Copyright (c) 2008
 * Altera Corporation, San Jose, California, USA.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This agreement shall be governed in all respects by the laws of the State
 * of California and by the laws of the United States of America.
 */

#ifndef __SYSTEM_H_
#define __SYSTEM_H_

/* Include definitions from linker script generator */
#include "linker.h"


/*
 * CPU configuration
 *
 */

#define ALT_CPU_ARCHITECTURE "altera_nios2_gen2"
#define ALT_CPU_BIG_ENDIAN 0
#define ALT_CPU_BREAK_ADDR 0x00020020
#define ALT_CPU_CPU_ARCH_NIOS2_R1
#define ALT_CPU_CPU_FREQ 112500000u
#define ALT_CPU_CPU_ID_SIZE 1
#define ALT_CPU_CPU_ID_VALUE 0x00000000
#define ALT_CPU_CPU_IMPLEMENTATION "fast"
#define ALT_CPU_DATA_ADDR_WIDTH 0x1b
#define ALT_CPU_DCACHE_BYPASS_MASK 0x80000000
#define ALT_CPU_DCACHE_LINE_SIZE 32
#define ALT_CPU_DCACHE_LINE_SIZE_LOG2 5
#define ALT_CPU_DCACHE_SIZE 8192
#define ALT_CPU_EXCEPTION_ADDR 0x00800020
#define ALT_CPU_FLASH_ACCELERATOR_LINES 0
#define ALT_CPU_FLASH_ACCELERATOR_LINE_SIZE 0
#define ALT_CPU_FLUSHDA_SUPPORTED
#define ALT_CPU_FREQ 112500000
#define ALT_CPU_HARDWARE_DIVIDE_PRESENT 0
#define ALT_CPU_HARDWARE_MULTIPLY_PRESENT 1
#define ALT_CPU_HARDWARE_MULX_PRESENT 0
#define ALT_CPU_HAS_DEBUG_CORE 1
#define ALT_CPU_HAS_DEBUG_STUB
#define ALT_CPU_HAS_EXTRA_EXCEPTION_INFO
#define ALT_CPU_HAS_ILLEGAL_INSTRUCTION_EXCEPTION
#define ALT_CPU_HAS_JMPI_INSTRUCTION
#define ALT_CPU_ICACHE_LINE_SIZE 32
#define ALT_CPU_ICACHE_LINE_SIZE_LOG2 5
#define ALT_CPU_ICACHE_SIZE 4096
#define ALT_CPU_INITDA_SUPPORTED
#define ALT_CPU_INST_ADDR_WIDTH 0x1b
#define ALT_CPU_NAME "nios2_qsys_0"
#define ALT_CPU_NUM_OF_SHADOW_REG_SETS 0
#define ALT_CPU_OCI_VERSION 1
#define ALT_CPU_RESET_ADDR 0x05000000


/*
 * CPU configuration (with legacy prefix - don't use these anymore)
 *
 */

#define NIOS2_BIG_ENDIAN 0
#define NIOS2_BREAK_ADDR 0x00020020
#define NIOS2_CPU_ARCH_NIOS2_R1
#define NIOS2_CPU_FREQ 112500000u
#define NIOS2_CPU_ID_SIZE 1
#define NIOS2_CPU_ID_VALUE 0x00000000
#define NIOS2_CPU_IMPLEMENTATION "fast"
#define NIOS2_DATA_ADDR_WIDTH 0x1b
#define NIOS2_DCACHE_BYPASS_MASK 0x80000000
#define NIOS2_DCACHE_LINE_SIZE 32
#define NIOS2_DCACHE_LINE_SIZE_LOG2 5
#define NIOS2_DCACHE_SIZE 8192
#define NIOS2_EXCEPTION_ADDR 0x00800020
#define NIOS2_FLASH_ACCELERATOR_LINES 0
#define NIOS2_FLASH_ACCELERATOR_LINE_SIZE 0
#define NIOS2_FLUSHDA_SUPPORTED
#define NIOS2_HARDWARE_DIVIDE_PRESENT 0
#define NIOS2_HARDWARE_MULTIPLY_PRESENT 1
#define NIOS2_HARDWARE_MULX_PRESENT 0
#define NIOS2_HAS_DEBUG_CORE 1
#define NIOS2_HAS_DEBUG_STUB
#define NIOS2_HAS_EXTRA_EXCEPTION_INFO
#define NIOS2_HAS_ILLEGAL_INSTRUCTION_EXCEPTION
#define NIOS2_HAS_JMPI_INSTRUCTION
#define NIOS2_ICACHE_LINE_SIZE 32
#define NIOS2_ICACHE_LINE_SIZE_LOG2 5
#define NIOS2_ICACHE_SIZE 4096
#define NIOS2_INITDA_SUPPORTED
#define NIOS2_INST_ADDR_WIDTH 0x1b
#define NIOS2_NUM_OF_SHADOW_REG_SETS 0
#define NIOS2_OCI_VERSION 1
#define NIOS2_RESET_ADDR 0x05000000


/*
 * Define for each module class mastered by the CPU
 *
 */

#define __ALTERA_AVALON_JTAG_UART
#define __ALTERA_AVALON_NEW_SDRAM_CONTROLLER
#define __ALTERA_AVALON_ONCHIP_MEMORY2
#define __ALTERA_AVALON_PIO
#define __ALTERA_AVALON_SGDMA
#define __ALTERA_AVALON_SPI
#define __ALTERA_AVALON_TIMER
#define __ALTERA_ETH_TSE
#define __ALTERA_MSGDMA
#define __ALTERA_NIOS2_GEN2
#define __ALTERA_ONCHIP_FLASH
#define __I2C_OPENCORES
#define __PTG_AVALON_VIDEO_DMA_CONTROLLER
#define __SOUND
#define __UDP_PAYLOAD_INSERTER


/*
 * System configuration
 *
 */

#define ALT_DEVICE_FAMILY "MAX 10"
#define ALT_ENHANCED_INTERRUPT_API_PRESENT
#define ALT_IRQ_BASE NULL
#define ALT_LOG_PORT "/dev/null"
#define ALT_LOG_PORT_BASE 0x0
#define ALT_LOG_PORT_DEV null
#define ALT_LOG_PORT_TYPE ""
#define ALT_NUM_EXTERNAL_INTERRUPT_CONTROLLERS 0
#define ALT_NUM_INTERNAL_INTERRUPT_CONTROLLERS 1
#define ALT_NUM_INTERRUPT_CONTROLLERS 1
#define ALT_STDERR "/dev/null"
#define ALT_STDERR_BASE 0x0
#define ALT_STDERR_DEV null
#define ALT_STDERR_TYPE ""
#define ALT_STDIN "/dev/null"
#define ALT_STDIN_BASE 0x0
#define ALT_STDIN_DEV null
#define ALT_STDIN_TYPE ""
#define ALT_STDOUT "/dev/null"
#define ALT_STDOUT_BASE 0x0
#define ALT_STDOUT_DEV null
#define ALT_STDOUT_TYPE ""
#define ALT_SYSTEM_NAME "tb2_qs"


/*
 * altera_iniche configuration
 *
 */

#define DHCP_CLIENT
#define INCLUDE_TCP
#define INICHE_DEFAULT_IF "NOT_USED"
#define IP_FRAGMENTS


/*
 * eth_tse_0 configuration
 *
 */

#define ALT_MODULE_CLASS_eth_tse_0 altera_eth_tse
#define ETH_TSE_0_BASE 0x11000
#define ETH_TSE_0_ENABLE_MACLITE 0
#define ETH_TSE_0_FIFO_WIDTH 32
#define ETH_TSE_0_IRQ -1
#define ETH_TSE_0_IRQ_INTERRUPT_CONTROLLER_ID -1
#define ETH_TSE_0_IS_MULTICHANNEL_MAC 0
#define ETH_TSE_0_MACLITE_GIGE 0
#define ETH_TSE_0_MDIO_SHARED 0
#define ETH_TSE_0_NAME "/dev/eth_tse_0"
#define ETH_TSE_0_NUMBER_OF_CHANNEL 1
#define ETH_TSE_0_NUMBER_OF_MAC_MDIO_SHARED 1
#define ETH_TSE_0_PCS 0
#define ETH_TSE_0_PCS_ID 0
#define ETH_TSE_0_PCS_SGMII 0
#define ETH_TSE_0_RECEIVE_FIFO_DEPTH 512
#define ETH_TSE_0_REGISTER_SHARED 0
#define ETH_TSE_0_RGMII 0
#define ETH_TSE_0_SPAN 1024
#define ETH_TSE_0_TRANSMIT_FIFO_DEPTH 2048
#define ETH_TSE_0_TYPE "altera_eth_tse"
#define ETH_TSE_0_USE_MDIO 1


/*
 * hal configuration
 *
 */

#define ALT_INCLUDE_INSTRUCTION_RELATED_EXCEPTION_API
#define ALT_MAX_FD 32
#define ALT_SYS_CLK SYS_TIMER
#define ALT_TIMESTAMP_CLK none


/*
 * i2c_opencores_0 configuration
 *
 */

#define ALT_MODULE_CLASS_i2c_opencores_0 i2c_opencores
#define I2C_OPENCORES_0_BASE 0x40
#define I2C_OPENCORES_0_IRQ 8
#define I2C_OPENCORES_0_IRQ_INTERRUPT_CONTROLLER_ID 0
#define I2C_OPENCORES_0_NAME "/dev/i2c_opencores_0"
#define I2C_OPENCORES_0_SPAN 32
#define I2C_OPENCORES_0_TYPE "i2c_opencores"


/*
 * i2c_opencores_1 configuration
 *
 */

#define ALT_MODULE_CLASS_i2c_opencores_1 i2c_opencores
#define I2C_OPENCORES_1_BASE 0x400
#define I2C_OPENCORES_1_IRQ 2
#define I2C_OPENCORES_1_IRQ_INTERRUPT_CONTROLLER_ID 0
#define I2C_OPENCORES_1_NAME "/dev/i2c_opencores_1"
#define I2C_OPENCORES_1_SPAN 32
#define I2C_OPENCORES_1_TYPE "i2c_opencores"


/*
 * i2c_opencores_2 configuration
 *
 */

#define ALT_MODULE_CLASS_i2c_opencores_2 i2c_opencores
#define I2C_OPENCORES_2_BASE 0x200
#define I2C_OPENCORES_2_IRQ 0
#define I2C_OPENCORES_2_IRQ_INTERRUPT_CONTROLLER_ID 0
#define I2C_OPENCORES_2_NAME "/dev/i2c_opencores_2"
#define I2C_OPENCORES_2_SPAN 32
#define I2C_OPENCORES_2_TYPE "i2c_opencores"


/*
 * i2c_opencores_3 configuration
 *
 */

#define ALT_MODULE_CLASS_i2c_opencores_3 i2c_opencores
#define I2C_OPENCORES_3_BASE 0x600
#define I2C_OPENCORES_3_IRQ 12
#define I2C_OPENCORES_3_IRQ_INTERRUPT_CONTROLLER_ID 0
#define I2C_OPENCORES_3_NAME "/dev/i2c_opencores_3"
#define I2C_OPENCORES_3_SPAN 32
#define I2C_OPENCORES_3_TYPE "i2c_opencores"


/*
 * jtag_uart_0 configuration
 *
 */

#define ALT_MODULE_CLASS_jtag_uart_0 altera_avalon_jtag_uart
#define JTAG_UART_0_BASE 0x114b0
#define JTAG_UART_0_IRQ 9
#define JTAG_UART_0_IRQ_INTERRUPT_CONTROLLER_ID 0
#define JTAG_UART_0_NAME "/dev/jtag_uart_0"
#define JTAG_UART_0_READ_DEPTH 64
#define JTAG_UART_0_READ_THRESHOLD 8
#define JTAG_UART_0_SPAN 8
#define JTAG_UART_0_TYPE "altera_avalon_jtag_uart"
#define JTAG_UART_0_WRITE_DEPTH 64
#define JTAG_UART_0_WRITE_THRESHOLD 8


/*
 * msgdma_0_csr configuration
 *
 */

#define ALT_MODULE_CLASS_msgdma_0_csr altera_msgdma
#define MSGDMA_0_CSR_BASE 0x40000
#define MSGDMA_0_CSR_BURST_ENABLE 0
#define MSGDMA_0_CSR_BURST_WRAPPING_SUPPORT 0
#define MSGDMA_0_CSR_CHANNEL_ENABLE 0
#define MSGDMA_0_CSR_CHANNEL_ENABLE_DERIVED 0
#define MSGDMA_0_CSR_CHANNEL_WIDTH 8
#define MSGDMA_0_CSR_DATA_FIFO_DEPTH 128
#define MSGDMA_0_CSR_DATA_WIDTH 32
#define MSGDMA_0_CSR_DESCRIPTOR_FIFO_DEPTH 256
#define MSGDMA_0_CSR_DMA_MODE 1
#define MSGDMA_0_CSR_ENHANCED_FEATURES 0
#define MSGDMA_0_CSR_ERROR_ENABLE 0
#define MSGDMA_0_CSR_ERROR_ENABLE_DERIVED 0
#define MSGDMA_0_CSR_ERROR_WIDTH 8
#define MSGDMA_0_CSR_IRQ 11
#define MSGDMA_0_CSR_IRQ_INTERRUPT_CONTROLLER_ID 0
#define MSGDMA_0_CSR_MAX_BURST_COUNT 2
#define MSGDMA_0_CSR_MAX_BYTE 2048
#define MSGDMA_0_CSR_MAX_STRIDE 1
#define MSGDMA_0_CSR_NAME "/dev/msgdma_0_csr"
#define MSGDMA_0_CSR_PACKET_ENABLE 1
#define MSGDMA_0_CSR_PACKET_ENABLE_DERIVED 1
#define MSGDMA_0_CSR_PREFETCHER_ENABLE 0
#define MSGDMA_0_CSR_PROGRAMMABLE_BURST_ENABLE 0
#define MSGDMA_0_CSR_RESPONSE_PORT 2
#define MSGDMA_0_CSR_SPAN 32
#define MSGDMA_0_CSR_STRIDE_ENABLE 0
#define MSGDMA_0_CSR_STRIDE_ENABLE_DERIVED 0
#define MSGDMA_0_CSR_TRANSFER_TYPE "Unaligned Accesses"
#define MSGDMA_0_CSR_TYPE "altera_msgdma"


/*
 * msgdma_0_descriptor_slave configuration
 *
 */

#define ALT_MODULE_CLASS_msgdma_0_descriptor_slave altera_msgdma
#define MSGDMA_0_DESCRIPTOR_SLAVE_BASE 0x48000
#define MSGDMA_0_DESCRIPTOR_SLAVE_BURST_ENABLE 0
#define MSGDMA_0_DESCRIPTOR_SLAVE_BURST_WRAPPING_SUPPORT 0
#define MSGDMA_0_DESCRIPTOR_SLAVE_CHANNEL_ENABLE 0
#define MSGDMA_0_DESCRIPTOR_SLAVE_CHANNEL_ENABLE_DERIVED 0
#define MSGDMA_0_DESCRIPTOR_SLAVE_CHANNEL_WIDTH 8
#define MSGDMA_0_DESCRIPTOR_SLAVE_DATA_FIFO_DEPTH 128
#define MSGDMA_0_DESCRIPTOR_SLAVE_DATA_WIDTH 32
#define MSGDMA_0_DESCRIPTOR_SLAVE_DESCRIPTOR_FIFO_DEPTH 256
#define MSGDMA_0_DESCRIPTOR_SLAVE_DMA_MODE 1
#define MSGDMA_0_DESCRIPTOR_SLAVE_ENHANCED_FEATURES 0
#define MSGDMA_0_DESCRIPTOR_SLAVE_ERROR_ENABLE 0
#define MSGDMA_0_DESCRIPTOR_SLAVE_ERROR_ENABLE_DERIVED 0
#define MSGDMA_0_DESCRIPTOR_SLAVE_ERROR_WIDTH 8
#define MSGDMA_0_DESCRIPTOR_SLAVE_IRQ -1
#define MSGDMA_0_DESCRIPTOR_SLAVE_IRQ_INTERRUPT_CONTROLLER_ID -1
#define MSGDMA_0_DESCRIPTOR_SLAVE_MAX_BURST_COUNT 2
#define MSGDMA_0_DESCRIPTOR_SLAVE_MAX_BYTE 2048
#define MSGDMA_0_DESCRIPTOR_SLAVE_MAX_STRIDE 1
#define MSGDMA_0_DESCRIPTOR_SLAVE_NAME "/dev/msgdma_0_descriptor_slave"
#define MSGDMA_0_DESCRIPTOR_SLAVE_PACKET_ENABLE 1
#define MSGDMA_0_DESCRIPTOR_SLAVE_PACKET_ENABLE_DERIVED 1
#define MSGDMA_0_DESCRIPTOR_SLAVE_PREFETCHER_ENABLE 0
#define MSGDMA_0_DESCRIPTOR_SLAVE_PROGRAMMABLE_BURST_ENABLE 0
#define MSGDMA_0_DESCRIPTOR_SLAVE_RESPONSE_PORT 2
#define MSGDMA_0_DESCRIPTOR_SLAVE_SPAN 16
#define MSGDMA_0_DESCRIPTOR_SLAVE_STRIDE_ENABLE 0
#define MSGDMA_0_DESCRIPTOR_SLAVE_STRIDE_ENABLE_DERIVED 0
#define MSGDMA_0_DESCRIPTOR_SLAVE_TRANSFER_TYPE "Unaligned Accesses"
#define MSGDMA_0_DESCRIPTOR_SLAVE_TYPE "altera_msgdma"


/*
 * onchip_flash_0 configuration
 *
 */

#define ALT_MODULE_CLASS_onchip_flash_0 altera_onchip_flash
#define ONCHIP_FLASH_0_BASE 0x5000000
#define ONCHIP_FLASH_0_BYTES_PER_PAGE 4096
#define ONCHIP_FLASH_0_IRQ -1
#define ONCHIP_FLASH_0_IRQ_INTERRUPT_CONTROLLER_ID -1
#define ONCHIP_FLASH_0_NAME "/dev/onchip_flash_0"
#define ONCHIP_FLASH_0_READ_ONLY_MODE 0
#define ONCHIP_FLASH_0_SECTOR1_ENABLED 1
#define ONCHIP_FLASH_0_SECTOR1_END_ADDR 0x3fff
#define ONCHIP_FLASH_0_SECTOR1_START_ADDR 0
#define ONCHIP_FLASH_0_SECTOR2_ENABLED 1
#define ONCHIP_FLASH_0_SECTOR2_END_ADDR 0x7fff
#define ONCHIP_FLASH_0_SECTOR2_START_ADDR 0x4000
#define ONCHIP_FLASH_0_SECTOR3_ENABLED 1
#define ONCHIP_FLASH_0_SECTOR3_END_ADDR 0x3bfff
#define ONCHIP_FLASH_0_SECTOR3_START_ADDR 0x8000
#define ONCHIP_FLASH_0_SECTOR4_ENABLED 0
#define ONCHIP_FLASH_0_SECTOR4_END_ADDR 0xffffffff
#define ONCHIP_FLASH_0_SECTOR4_START_ADDR 0xffffffff
#define ONCHIP_FLASH_0_SECTOR5_ENABLED 0
#define ONCHIP_FLASH_0_SECTOR5_END_ADDR 0xffffffff
#define ONCHIP_FLASH_0_SECTOR5_START_ADDR 0xffffffff
#define ONCHIP_FLASH_0_SPAN 245760
#define ONCHIP_FLASH_0_TYPE "altera_onchip_flash"


/*
 * onchip_memory2_1 configuration
 *
 */

#define ALT_MODULE_CLASS_onchip_memory2_1 altera_avalon_onchip_memory2
#define ONCHIP_MEMORY2_1_ALLOW_IN_SYSTEM_MEMORY_CONTENT_EDITOR 0
#define ONCHIP_MEMORY2_1_ALLOW_MRAM_SIM_CONTENTS_ONLY_FILE 0
#define ONCHIP_MEMORY2_1_BASE 0x8000
#define ONCHIP_MEMORY2_1_CONTENTS_INFO ""
#define ONCHIP_MEMORY2_1_DUAL_PORT 0
#define ONCHIP_MEMORY2_1_GUI_RAM_BLOCK_TYPE "AUTO"
#define ONCHIP_MEMORY2_1_INIT_CONTENTS_FILE "tb2_qs_onchip_memory2_1"
#define ONCHIP_MEMORY2_1_INIT_MEM_CONTENT 0
#define ONCHIP_MEMORY2_1_INSTANCE_ID "NONE"
#define ONCHIP_MEMORY2_1_IRQ -1
#define ONCHIP_MEMORY2_1_IRQ_INTERRUPT_CONTROLLER_ID -1
#define ONCHIP_MEMORY2_1_NAME "/dev/onchip_memory2_1"
#define ONCHIP_MEMORY2_1_NON_DEFAULT_INIT_FILE_ENABLED 0
#define ONCHIP_MEMORY2_1_RAM_BLOCK_TYPE "AUTO"
#define ONCHIP_MEMORY2_1_READ_DURING_WRITE_MODE "DONT_CARE"
#define ONCHIP_MEMORY2_1_SINGLE_CLOCK_OP 0
#define ONCHIP_MEMORY2_1_SIZE_MULTIPLE 1
#define ONCHIP_MEMORY2_1_SIZE_VALUE 8192
#define ONCHIP_MEMORY2_1_SPAN 8192
#define ONCHIP_MEMORY2_1_TYPE "altera_avalon_onchip_memory2"
#define ONCHIP_MEMORY2_1_WRITABLE 1


/*
 * pio_0 configuration
 *
 */

#define ALT_MODULE_CLASS_pio_0 altera_avalon_pio
#define PIO_0_BASE 0x60
#define PIO_0_BIT_CLEARING_EDGE_REGISTER 0
#define PIO_0_BIT_MODIFYING_OUTPUT_REGISTER 1
#define PIO_0_CAPTURE 0
#define PIO_0_DATA_WIDTH 16
#define PIO_0_DO_TEST_BENCH_WIRING 0
#define PIO_0_DRIVEN_SIM_VALUE 0
#define PIO_0_EDGE_TYPE "NONE"
#define PIO_0_FREQ 112500000
#define PIO_0_HAS_IN 1
#define PIO_0_HAS_OUT 1
#define PIO_0_HAS_TRI 0
#define PIO_0_IRQ -1
#define PIO_0_IRQ_INTERRUPT_CONTROLLER_ID -1
#define PIO_0_IRQ_TYPE "NONE"
#define PIO_0_NAME "/dev/pio_0"
#define PIO_0_RESET_VALUE 0
#define PIO_0_SPAN 32
#define PIO_0_TYPE "altera_avalon_pio"


/*
 * ptg_avalon_video_dma_controller_0 configuration
 *
 */

#define ALT_MODULE_CLASS_ptg_avalon_video_dma_controller_0 ptg_avalon_video_dma_controller
#define PTG_AVALON_VIDEO_DMA_CONTROLLER_0_BASE 0x4c080
#define PTG_AVALON_VIDEO_DMA_CONTROLLER_0_IRQ -1
#define PTG_AVALON_VIDEO_DMA_CONTROLLER_0_IRQ_INTERRUPT_CONTROLLER_ID -1
#define PTG_AVALON_VIDEO_DMA_CONTROLLER_0_NAME "/dev/ptg_avalon_video_dma_controller_0"
#define PTG_AVALON_VIDEO_DMA_CONTROLLER_0_SPAN 32
#define PTG_AVALON_VIDEO_DMA_CONTROLLER_0_TYPE "ptg_avalon_video_dma_controller"


/*
 * ptg_avalon_video_dma_controller_1 configuration
 *
 */

#define ALT_MODULE_CLASS_ptg_avalon_video_dma_controller_1 ptg_avalon_video_dma_controller
#define PTG_AVALON_VIDEO_DMA_CONTROLLER_1_BASE 0x49000
#define PTG_AVALON_VIDEO_DMA_CONTROLLER_1_IRQ -1
#define PTG_AVALON_VIDEO_DMA_CONTROLLER_1_IRQ_INTERRUPT_CONTROLLER_ID -1
#define PTG_AVALON_VIDEO_DMA_CONTROLLER_1_NAME "/dev/ptg_avalon_video_dma_controller_1"
#define PTG_AVALON_VIDEO_DMA_CONTROLLER_1_SPAN 32
#define PTG_AVALON_VIDEO_DMA_CONTROLLER_1_TYPE "ptg_avalon_video_dma_controller"


/*
 * ptg_avalon_video_dma_controller_2 configuration
 *
 */

#define ALT_MODULE_CLASS_ptg_avalon_video_dma_controller_2 ptg_avalon_video_dma_controller
#define PTG_AVALON_VIDEO_DMA_CONTROLLER_2_BASE 0x49100
#define PTG_AVALON_VIDEO_DMA_CONTROLLER_2_IRQ -1
#define PTG_AVALON_VIDEO_DMA_CONTROLLER_2_IRQ_INTERRUPT_CONTROLLER_ID -1
#define PTG_AVALON_VIDEO_DMA_CONTROLLER_2_NAME "/dev/ptg_avalon_video_dma_controller_2"
#define PTG_AVALON_VIDEO_DMA_CONTROLLER_2_SPAN 32
#define PTG_AVALON_VIDEO_DMA_CONTROLLER_2_TYPE "ptg_avalon_video_dma_controller"


/*
 * sdram configuration
 *
 */

#define ALT_MODULE_CLASS_sdram altera_avalon_new_sdram_controller
#define SDRAM_BASE 0x800000
#define SDRAM_CAS_LATENCY 2
#define SDRAM_CONTENTS_INFO
#define SDRAM_INIT_NOP_DELAY 0.0
#define SDRAM_INIT_REFRESH_COMMANDS 2
#define SDRAM_IRQ -1
#define SDRAM_IRQ_INTERRUPT_CONTROLLER_ID -1
#define SDRAM_IS_INITIALIZED 1
#define SDRAM_NAME "/dev/sdram"
#define SDRAM_POWERUP_DELAY 100.0
#define SDRAM_REFRESH_PERIOD 31.25
#define SDRAM_REGISTER_DATA_IN 1
#define SDRAM_SDRAM_ADDR_WIDTH 0x15
#define SDRAM_SDRAM_BANK_WIDTH 2
#define SDRAM_SDRAM_COL_WIDTH 8
#define SDRAM_SDRAM_DATA_WIDTH 32
#define SDRAM_SDRAM_NUM_BANKS 4
#define SDRAM_SDRAM_NUM_CHIPSELECTS 1
#define SDRAM_SDRAM_ROW_WIDTH 11
#define SDRAM_SHARED_DATA 0
#define SDRAM_SIM_MODEL_BASE 0
#define SDRAM_SPAN 8388608
#define SDRAM_STARVATION_INDICATOR 0
#define SDRAM_TRISTATE_BRIDGE_SLAVE ""
#define SDRAM_TYPE "altera_avalon_new_sdram_controller"
#define SDRAM_T_AC 5.4
#define SDRAM_T_MRD 3
#define SDRAM_T_RCD 20.0
#define SDRAM_T_RFC 70.0
#define SDRAM_T_RP 20.0
#define SDRAM_T_WR 14.0


/*
 * sdram configuration as viewed by sgdma_rx_m_write
 *
 */

#define SGDMA_RX_M_WRITE_SDRAM_BASE 0x800000
#define SGDMA_RX_M_WRITE_SDRAM_CAS_LATENCY 2
#define SGDMA_RX_M_WRITE_SDRAM_CONTENTS_INFO
#define SGDMA_RX_M_WRITE_SDRAM_INIT_NOP_DELAY 0.0
#define SGDMA_RX_M_WRITE_SDRAM_INIT_REFRESH_COMMANDS 2
#define SGDMA_RX_M_WRITE_SDRAM_IRQ -1
#define SGDMA_RX_M_WRITE_SDRAM_IRQ_INTERRUPT_CONTROLLER_ID -1
#define SGDMA_RX_M_WRITE_SDRAM_IS_INITIALIZED 1
#define SGDMA_RX_M_WRITE_SDRAM_NAME "/dev/sdram"
#define SGDMA_RX_M_WRITE_SDRAM_POWERUP_DELAY 100.0
#define SGDMA_RX_M_WRITE_SDRAM_REFRESH_PERIOD 31.25
#define SGDMA_RX_M_WRITE_SDRAM_REGISTER_DATA_IN 1
#define SGDMA_RX_M_WRITE_SDRAM_SDRAM_ADDR_WIDTH 0x15
#define SGDMA_RX_M_WRITE_SDRAM_SDRAM_BANK_WIDTH 2
#define SGDMA_RX_M_WRITE_SDRAM_SDRAM_COL_WIDTH 8
#define SGDMA_RX_M_WRITE_SDRAM_SDRAM_DATA_WIDTH 32
#define SGDMA_RX_M_WRITE_SDRAM_SDRAM_NUM_BANKS 4
#define SGDMA_RX_M_WRITE_SDRAM_SDRAM_NUM_CHIPSELECTS 1
#define SGDMA_RX_M_WRITE_SDRAM_SDRAM_ROW_WIDTH 11
#define SGDMA_RX_M_WRITE_SDRAM_SHARED_DATA 0
#define SGDMA_RX_M_WRITE_SDRAM_SIM_MODEL_BASE 0
#define SGDMA_RX_M_WRITE_SDRAM_SPAN 8388608
#define SGDMA_RX_M_WRITE_SDRAM_STARVATION_INDICATOR 0
#define SGDMA_RX_M_WRITE_SDRAM_TRISTATE_BRIDGE_SLAVE ""
#define SGDMA_RX_M_WRITE_SDRAM_TYPE "altera_avalon_new_sdram_controller"
#define SGDMA_RX_M_WRITE_SDRAM_T_AC 5.4
#define SGDMA_RX_M_WRITE_SDRAM_T_MRD 3
#define SGDMA_RX_M_WRITE_SDRAM_T_RCD 20.0
#define SGDMA_RX_M_WRITE_SDRAM_T_RFC 70.0
#define SGDMA_RX_M_WRITE_SDRAM_T_RP 20.0
#define SGDMA_RX_M_WRITE_SDRAM_T_WR 14.0


/*
 * sdram configuration as viewed by sgdma_tx_m_read
 *
 */

#define SGDMA_TX_M_READ_SDRAM_BASE 0x800000
#define SGDMA_TX_M_READ_SDRAM_CAS_LATENCY 2
#define SGDMA_TX_M_READ_SDRAM_CONTENTS_INFO
#define SGDMA_TX_M_READ_SDRAM_INIT_NOP_DELAY 0.0
#define SGDMA_TX_M_READ_SDRAM_INIT_REFRESH_COMMANDS 2
#define SGDMA_TX_M_READ_SDRAM_IRQ -1
#define SGDMA_TX_M_READ_SDRAM_IRQ_INTERRUPT_CONTROLLER_ID -1
#define SGDMA_TX_M_READ_SDRAM_IS_INITIALIZED 1
#define SGDMA_TX_M_READ_SDRAM_NAME "/dev/sdram"
#define SGDMA_TX_M_READ_SDRAM_POWERUP_DELAY 100.0
#define SGDMA_TX_M_READ_SDRAM_REFRESH_PERIOD 31.25
#define SGDMA_TX_M_READ_SDRAM_REGISTER_DATA_IN 1
#define SGDMA_TX_M_READ_SDRAM_SDRAM_ADDR_WIDTH 0x15
#define SGDMA_TX_M_READ_SDRAM_SDRAM_BANK_WIDTH 2
#define SGDMA_TX_M_READ_SDRAM_SDRAM_COL_WIDTH 8
#define SGDMA_TX_M_READ_SDRAM_SDRAM_DATA_WIDTH 32
#define SGDMA_TX_M_READ_SDRAM_SDRAM_NUM_BANKS 4
#define SGDMA_TX_M_READ_SDRAM_SDRAM_NUM_CHIPSELECTS 1
#define SGDMA_TX_M_READ_SDRAM_SDRAM_ROW_WIDTH 11
#define SGDMA_TX_M_READ_SDRAM_SHARED_DATA 0
#define SGDMA_TX_M_READ_SDRAM_SIM_MODEL_BASE 0
#define SGDMA_TX_M_READ_SDRAM_SPAN 8388608
#define SGDMA_TX_M_READ_SDRAM_STARVATION_INDICATOR 0
#define SGDMA_TX_M_READ_SDRAM_TRISTATE_BRIDGE_SLAVE ""
#define SGDMA_TX_M_READ_SDRAM_TYPE "altera_avalon_new_sdram_controller"
#define SGDMA_TX_M_READ_SDRAM_T_AC 5.4
#define SGDMA_TX_M_READ_SDRAM_T_MRD 3
#define SGDMA_TX_M_READ_SDRAM_T_RCD 20.0
#define SGDMA_TX_M_READ_SDRAM_T_RFC 70.0
#define SGDMA_TX_M_READ_SDRAM_T_RP 20.0
#define SGDMA_TX_M_READ_SDRAM_T_WR 14.0


/*
 * sgdma_rx configuration
 *
 */

#define ALT_MODULE_CLASS_sgdma_rx altera_avalon_sgdma
#define SGDMA_RX_ADDRESS_WIDTH 32
#define SGDMA_RX_ALWAYS_DO_MAX_BURST 1
#define SGDMA_RX_ATLANTIC_CHANNEL_DATA_WIDTH 4
#define SGDMA_RX_AVALON_MM_BYTE_REORDER_MODE 0
#define SGDMA_RX_BASE 0x11400
#define SGDMA_RX_BURST_DATA_WIDTH 8
#define SGDMA_RX_BURST_TRANSFER 0
#define SGDMA_RX_BYTES_TO_TRANSFER_DATA_WIDTH 16
#define SGDMA_RX_CHAIN_WRITEBACK_DATA_WIDTH 32
#define SGDMA_RX_COMMAND_FIFO_DATA_WIDTH 104
#define SGDMA_RX_CONTROL_DATA_WIDTH 8
#define SGDMA_RX_CONTROL_SLAVE_ADDRESS_WIDTH 0x4
#define SGDMA_RX_CONTROL_SLAVE_DATA_WIDTH 32
#define SGDMA_RX_DESCRIPTOR_READ_BURST 0
#define SGDMA_RX_DESC_DATA_WIDTH 32
#define SGDMA_RX_HAS_READ_BLOCK 0
#define SGDMA_RX_HAS_WRITE_BLOCK 1
#define SGDMA_RX_IN_ERROR_WIDTH 6
#define SGDMA_RX_IRQ 7
#define SGDMA_RX_IRQ_INTERRUPT_CONTROLLER_ID 0
#define SGDMA_RX_NAME "/dev/sgdma_rx"
#define SGDMA_RX_OUT_ERROR_WIDTH 0
#define SGDMA_RX_READ_BLOCK_DATA_WIDTH 32
#define SGDMA_RX_READ_BURSTCOUNT_WIDTH 4
#define SGDMA_RX_SPAN 64
#define SGDMA_RX_STATUS_TOKEN_DATA_WIDTH 24
#define SGDMA_RX_STREAM_DATA_WIDTH 32
#define SGDMA_RX_SYMBOLS_PER_BEAT 4
#define SGDMA_RX_TYPE "altera_avalon_sgdma"
#define SGDMA_RX_UNALIGNED_TRANSFER 0
#define SGDMA_RX_WRITE_BLOCK_DATA_WIDTH 32
#define SGDMA_RX_WRITE_BURSTCOUNT_WIDTH 4


/*
 * sgdma_tx configuration
 *
 */

#define ALT_MODULE_CLASS_sgdma_tx altera_avalon_sgdma
#define SGDMA_TX_ADDRESS_WIDTH 32
#define SGDMA_TX_ALWAYS_DO_MAX_BURST 1
#define SGDMA_TX_ATLANTIC_CHANNEL_DATA_WIDTH 4
#define SGDMA_TX_AVALON_MM_BYTE_REORDER_MODE 0
#define SGDMA_TX_BASE 0x11440
#define SGDMA_TX_BURST_DATA_WIDTH 8
#define SGDMA_TX_BURST_TRANSFER 0
#define SGDMA_TX_BYTES_TO_TRANSFER_DATA_WIDTH 16
#define SGDMA_TX_CHAIN_WRITEBACK_DATA_WIDTH 32
#define SGDMA_TX_COMMAND_FIFO_DATA_WIDTH 104
#define SGDMA_TX_CONTROL_DATA_WIDTH 8
#define SGDMA_TX_CONTROL_SLAVE_ADDRESS_WIDTH 0x4
#define SGDMA_TX_CONTROL_SLAVE_DATA_WIDTH 32
#define SGDMA_TX_DESCRIPTOR_READ_BURST 0
#define SGDMA_TX_DESC_DATA_WIDTH 32
#define SGDMA_TX_HAS_READ_BLOCK 1
#define SGDMA_TX_HAS_WRITE_BLOCK 0
#define SGDMA_TX_IN_ERROR_WIDTH 0
#define SGDMA_TX_IRQ 6
#define SGDMA_TX_IRQ_INTERRUPT_CONTROLLER_ID 0
#define SGDMA_TX_NAME "/dev/sgdma_tx"
#define SGDMA_TX_OUT_ERROR_WIDTH 1
#define SGDMA_TX_READ_BLOCK_DATA_WIDTH 32
#define SGDMA_TX_READ_BURSTCOUNT_WIDTH 4
#define SGDMA_TX_SPAN 64
#define SGDMA_TX_STATUS_TOKEN_DATA_WIDTH 24
#define SGDMA_TX_STREAM_DATA_WIDTH 32
#define SGDMA_TX_SYMBOLS_PER_BEAT 4
#define SGDMA_TX_TYPE "altera_avalon_sgdma"
#define SGDMA_TX_UNALIGNED_TRANSFER 0
#define SGDMA_TX_WRITE_BLOCK_DATA_WIDTH 32
#define SGDMA_TX_WRITE_BURSTCOUNT_WIDTH 4


/*
 * sound_0 configuration
 *
 */

#define ALT_MODULE_CLASS_sound_0 sound
#define SOUND_0_BASE 0x40200
#define SOUND_0_IRQ -1
#define SOUND_0_IRQ_INTERRUPT_CONTROLLER_ID -1
#define SOUND_0_NAME "/dev/sound_0"
#define SOUND_0_SPAN 4
#define SOUND_0_TYPE "sound"


/*
 * spi_0 configuration
 *
 */

#define ALT_MODULE_CLASS_spi_0 altera_avalon_spi
#define SPI_0_BASE 0x100
#define SPI_0_CLOCKMULT 1
#define SPI_0_CLOCKPHASE 0
#define SPI_0_CLOCKPOLARITY 0
#define SPI_0_CLOCKUNITS "Hz"
#define SPI_0_DATABITS 8
#define SPI_0_DATAWIDTH 16
#define SPI_0_DELAYMULT "1.0E-9"
#define SPI_0_DELAYUNITS "ns"
#define SPI_0_EXTRADELAY 0
#define SPI_0_INSERT_SYNC 0
#define SPI_0_IRQ -1
#define SPI_0_IRQ_INTERRUPT_CONTROLLER_ID -1
#define SPI_0_ISMASTER 1
#define SPI_0_LSBFIRST 0
#define SPI_0_NAME "/dev/spi_0"
#define SPI_0_NUMSLAVES 1
#define SPI_0_PREFIX "spi_"
#define SPI_0_SPAN 32
#define SPI_0_SYNC_REG_DEPTH 2
#define SPI_0_TARGETCLOCK 40000000u
#define SPI_0_TARGETSSDELAY "0.0"
#define SPI_0_TYPE "altera_avalon_spi"


/*
 * sys_timer configuration
 *
 */

#define ALT_MODULE_CLASS_sys_timer altera_avalon_timer
#define SYS_TIMER_ALWAYS_RUN 0
#define SYS_TIMER_BASE 0x0
#define SYS_TIMER_COUNTER_SIZE 32
#define SYS_TIMER_FIXED_PERIOD 1
#define SYS_TIMER_FREQ 112500000
#define SYS_TIMER_IRQ 3
#define SYS_TIMER_IRQ_INTERRUPT_CONTROLLER_ID 0
#define SYS_TIMER_LOAD_VALUE 112499
#define SYS_TIMER_MULT 0.001
#define SYS_TIMER_NAME "/dev/sys_timer"
#define SYS_TIMER_PERIOD 1
#define SYS_TIMER_PERIOD_UNITS "ms"
#define SYS_TIMER_RESET_OUTPUT 0
#define SYS_TIMER_SNAPSHOT 1
#define SYS_TIMER_SPAN 32
#define SYS_TIMER_TICKS_PER_SEC 1000
#define SYS_TIMER_TIMEOUT_PULSE_OUTPUT 0
#define SYS_TIMER_TYPE "altera_avalon_timer"


/*
 * ucosii configuration
 *
 */

#define OS_ARG_CHK_EN 1
#define OS_CPU_HOOKS_EN 1
#define OS_DEBUG_EN 1
#define OS_EVENT_NAME_SIZE 32
#define OS_FLAGS_NBITS 16
#define OS_FLAG_ACCEPT_EN 1
#define OS_FLAG_DEL_EN 1
#define OS_FLAG_EN 1
#define OS_FLAG_NAME_SIZE 32
#define OS_FLAG_QUERY_EN 1
#define OS_FLAG_WAIT_CLR_EN 1
#define OS_LOWEST_PRIO 20
#define OS_MAX_EVENTS 60
#define OS_MAX_FLAGS 20
#define OS_MAX_MEM_PART 60
#define OS_MAX_QS 20
#define OS_MAX_TASKS 10
#define OS_MBOX_ACCEPT_EN 1
#define OS_MBOX_DEL_EN 1
#define OS_MBOX_EN 1
#define OS_MBOX_POST_EN 1
#define OS_MBOX_POST_OPT_EN 1
#define OS_MBOX_QUERY_EN 1
#define OS_MEM_EN 1
#define OS_MEM_NAME_SIZE 32
#define OS_MEM_QUERY_EN 1
#define OS_MUTEX_ACCEPT_EN 1
#define OS_MUTEX_DEL_EN 1
#define OS_MUTEX_EN 1
#define OS_MUTEX_QUERY_EN 1
#define OS_Q_ACCEPT_EN 1
#define OS_Q_DEL_EN 1
#define OS_Q_EN 1
#define OS_Q_FLUSH_EN 1
#define OS_Q_POST_EN 1
#define OS_Q_POST_FRONT_EN 1
#define OS_Q_POST_OPT_EN 1
#define OS_Q_QUERY_EN 1
#define OS_SCHED_LOCK_EN 1
#define OS_SEM_ACCEPT_EN 1
#define OS_SEM_DEL_EN 1
#define OS_SEM_EN 1
#define OS_SEM_QUERY_EN 1
#define OS_SEM_SET_EN 1
#define OS_TASK_CHANGE_PRIO_EN 1
#define OS_TASK_CREATE_EN 1
#define OS_TASK_CREATE_EXT_EN 1
#define OS_TASK_DEL_EN 1
#define OS_TASK_IDLE_STK_SIZE 512
#define OS_TASK_NAME_SIZE 32
#define OS_TASK_PROFILE_EN 1
#define OS_TASK_QUERY_EN 1
#define OS_TASK_STAT_EN 1
#define OS_TASK_STAT_STK_CHK_EN 1
#define OS_TASK_STAT_STK_SIZE 512
#define OS_TASK_SUSPEND_EN 1
#define OS_TASK_SW_HOOK_EN 1
#define OS_TASK_TMR_PRIO 0
#define OS_TASK_TMR_STK_SIZE 512
#define OS_THREAD_SAFE_NEWLIB 1
#define OS_TICKS_PER_SEC SYS_TIMER_TICKS_PER_SEC
#define OS_TICK_STEP_EN 1
#define OS_TIME_DLY_HMSM_EN 1
#define OS_TIME_DLY_RESUME_EN 1
#define OS_TIME_GET_SET_EN 1
#define OS_TIME_TICK_HOOK_EN 1
#define OS_TMR_CFG_MAX 16
#define OS_TMR_CFG_NAME_SIZE 16
#define OS_TMR_CFG_TICKS_PER_SEC 10
#define OS_TMR_CFG_WHEEL_SIZE 2
#define OS_TMR_EN 0


/*
 * udp_payload_inserter_0 configuration
 *
 */

#define ALT_MODULE_CLASS_udp_payload_inserter_0 udp_payload_inserter
#define UDP_PAYLOAD_INSERTER_0_BASE 0x4a000
#define UDP_PAYLOAD_INSERTER_0_IRQ -1
#define UDP_PAYLOAD_INSERTER_0_IRQ_INTERRUPT_CONTROLLER_ID -1
#define UDP_PAYLOAD_INSERTER_0_NAME "/dev/udp_payload_inserter_0"
#define UDP_PAYLOAD_INSERTER_0_SPAN 64
#define UDP_PAYLOAD_INSERTER_0_TYPE "udp_payload_inserter"

#endif /* __SYSTEM_H_ */
