/*=============================================================================
Copyright (C) 2016 Kristina Brooks
All rights reserved.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

FILE DESCRIPTION
VideoCoreIV first stage bootloader.

=============================================================================*/

#include <common.h>
#include <hardware.h>
#include <cpu.h>

uint32_t g_CPUID;

void uart_putc(unsigned int ch)
{
	while(1) {
		if (mmio_read32(AUX_MU_LSR_REG) & 0x20)
			break;
	}
	mmio_write32(AUX_MU_IO_REG, ch);
}

void uart_init(void) {
	unsigned int ra = GP_FSEL1;
	ra &= ~(7 << 12);
	ra |= 2 << 12;
	GP_FSEL1 = ra;

	GP_PUD = 0;

	udelay(150);
	GP_PUDCLK0 = (1 << 14) | (1 << 15);
	udelay(150);
	GP_PUDCLK0 = 0;

	mmio_write32(AUX_ENABLES, 1);
	mmio_write32(AUX_MU_IER_REG, 0);
	mmio_write32(AUX_MU_CNTL_REG, 0);
	mmio_write32(AUX_MU_LCR_REG, 3);
	mmio_write32(AUX_MU_MCR_REG, 0);
	mmio_write32(AUX_MU_IER_REG, 0);
	mmio_write32(AUX_MU_IIR_REG, 0xC6);

	mmio_write32(AUX_MU_BAUD_REG, 270);

	mmio_write32(AUX_MU_LCR_REG, 3);
	mmio_write32(AUX_MU_CNTL_REG, 3);
}

void led_init(void) {
	unsigned int ra;

	ra = GP_FSEL1;
	ra &= ~(7 << 18);
	ra |= 1 << 18;

	GP_FSEL1 = ra;
}

/*
  #define CM_PLLC_DIGRST_BITS                                9:9
  #define CM_PLLC_DIGRST_SET                                 0x00000200
  #define CM_PLLC_ANARST_BITS                                8:8
  #define CM_PLLC_ANARST_SET                                 0x00000100
  #define CM_PLLC_HOLDPER_BITS                               7:7
  #define CM_PLLC_HOLDPER_SET                                0x00000080
  #define CM_PLLC_LOADPER_BITS                               6:6
  #define CM_PLLC_LOADPER_SET                                0x00000040
  #define CM_PLLC_HOLDCORE2_BITS                             5:5
  #define CM_PLLC_HOLDCORE2_SET                              0x00000020
  #define CM_PLLC_LOADCORE2_BITS                             4:4
  #define CM_PLLC_LOADCORE2_SET                              0x00000010
  #define CM_PLLC_HOLDCORE1_BITS                             3:3
  #define CM_PLLC_HOLDCORE1_SET                              0x00000008
  #define CM_PLLC_LOADCORE1_BITS                             2:2
  #define CM_PLLC_LOADCORE1_SET                              0x00000004
  #define CM_PLLC_HOLDCORE0_BITS                             1:1
  #define CM_PLLC_HOLDCORE0_SET                              0x00000002
  #define CM_PLLC_LOADCORE0_BITS                             0:0
  #define CM_PLLC_LOADCORE0_SET                              0x00000001
*/

void switch_vpu_to_pllc() {
	A2W_XOSC_CTRL |= A2W_PASSWORD | A2W_XOSC_CTRL_PLLCEN_SET;

	A2W_PLLC_FRAC = A2W_PASSWORD | 87380;
	A2W_PLLC_CTRL = A2W_PASSWORD | 52 | 0x1000;
	
	A2W_PLLC_ANA3 = A2W_PASSWORD | 0x100;
	A2W_PLLC_ANA2 = A2W_PASSWORD | 0x0;
	A2W_PLLC_ANA1 = A2W_PASSWORD | 0x144000;
	A2W_PLLC_ANA0 = A2W_PASSWORD | 0x0;

	CM_PLLC = CM_PASSWORD | CM_PLLC_DIGRST_SET;

	/* hold all */
	CM_PLLC = CM_PASSWORD | CM_PLLC_DIGRST_SET |
		CM_PLLC_HOLDPER_SET | CM_PLLC_HOLDCORE2_SET |
		CM_PLLC_HOLDCORE1_SET | CM_PLLC_HOLDCORE0_SET;

	A2W_PLLC_DIG3 = A2W_PASSWORD | 0x0;
	A2W_PLLC_DIG2 = A2W_PASSWORD | 0x400000;
	A2W_PLLC_DIG1 = A2W_PASSWORD | 0x5;
	A2W_PLLC_DIG0 = A2W_PASSWORD | 52 | 0x555000;

	A2W_PLLC_CTRL = A2W_PASSWORD | 52 | 0x1000 | A2W_PLLC_CTRL_PRSTN_SET;

	A2W_PLLC_DIG3 = A2W_PASSWORD | 0x42;
	A2W_PLLC_DIG2 = A2W_PASSWORD | 0x500401;
	A2W_PLLC_DIG1 = A2W_PASSWORD | 0x4005;
	A2W_PLLC_DIG0 = A2W_PASSWORD | 52 | 0x555000;

	A2W_PLLC_CORE0 = A2W_PASSWORD | 2;

	CM_PLLC = CM_PASSWORD | CM_PLLC_DIGRST_SET |
		CM_PLLC_HOLDPER_SET | CM_PLLC_HOLDCORE2_SET |
		CM_PLLC_HOLDCORE1_SET | CM_PLLC_HOLDCORE0_SET | CM_PLLC_LOADCORE0_SET;

	CM_PLLC = CM_PASSWORD | CM_PLLC_DIGRST_SET |
		CM_PLLC_HOLDPER_SET | CM_PLLC_HOLDCORE2_SET |
		CM_PLLC_HOLDCORE1_SET | CM_PLLC_HOLDCORE0_SET;

	CM_PLLC = CM_PASSWORD | CM_PLLC_DIGRST_SET |
		CM_PLLC_HOLDPER_SET | CM_PLLC_HOLDCORE2_SET |
		CM_PLLC_HOLDCORE1_SET;

	CM_VPUCTL = CM_PASSWORD | CM_VPUCTL_FRAC_SET | CM_SRC_OSC | CM_VPUCTL_GATE_SET;
	CM_VPUDIV = CM_PASSWORD | (4 << 12);
	CM_VPUCTL = CM_PASSWORD | CM_SRC_PLLC_CORE0 | CM_VPUCTL_GATE_SET;
	CM_VPUCTL = CM_PASSWORD | CM_SRC_PLLC_CORE0 | CM_VPUCTL_GATE_SET | 0x10; /* ENAB */

	CM_TIMERDIV = CM_PASSWORD | (19 << 12) | 819;
	CM_TIMERCTL = CM_PASSWORD | CM_SRC_OSC | 0x10;
}

extern void sdram_init();
extern void arm_init();
extern void monitor_start();

void print_crap() {
	printf("TB_BOOT_OPT = 0x%X\n", TB_BOOT_OPT);
}

int _main(unsigned int cpuid, unsigned int load_address) {
	switch_vpu_to_pllc();

	led_init();
	uart_init();

	printf(
		"=========================================================\n"
		"::\n"
		":: kFW for bcm2708, Copyright 2016, Kristina Brooks. \n"
		"::\n"
		":: BUILDATE  : %s %s \n"
		":: BUILDSTYLE: %s \n"
		"::\n"
		"=========================================================\n",
		__DATE__, __TIME__,
		"OPENSOURCE"
	);

	printf("CPUID    = 0x%X\n", cpuid);
	printf("LoadAddr = 0x%X\n", load_address);

	print_crap();

	g_CPUID = cpuid;

	/* bring up SDRAM */
	sdram_init();
	printf("SDRAM initialization completed successfully!\n");

        printf("Breakpoint mode test:\n");
        {
          unsigned status;
          __asm__ __volatile__ ("mov %0,sr" : "=r" (status));
          printf("status reg (before bkpt): %x\n", status);
          __asm__ __volatile__ ("nop\n\tnop\n\tbkpt\n\tnop\n\tnop");
          __asm__ __volatile__ ("mov %0,sr" : "=r" (status));
          printf("status reg (after bkpt): %x\n", status);
        }

        /* Just stop here.  */
        hang_cpu();

	/* bring up ARM */
	arm_init();

	/* start vpu monitor */
	monitor_start();

	panic("main exiting!");
}

