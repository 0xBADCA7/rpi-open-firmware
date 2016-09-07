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
Second stage bootloader.

=============================================================================*/

#include <drivers/fatfs/ff.h>
#include <chainloader.h>
#include <drivers/mailbox.hpp>
#include <libfdt.h>
#include <memory_map.h>

#define logf(fmt, ...) printf("[LDR:%s]: " fmt, __FUNCTION__, ##__VA_ARGS__);

FATFS g_BootVolumeFs;

#define ROOT_VOLUME_PREFIX "0:"
#define KERNEL_LOAD_ADDRESS 0x2000000

extern "C" {
	void boot_linux(int zero, int machineID, void* dtb, void* kernel);
}

static_assert((MEM_USABLE_START+0x800000) < KERNEL_LOAD_ADDRESS,
	"memory layout would not allow for kernel to be loaded at KERNEL_LOAD_ADDRESS, please check memory_map.h");

struct LoaderImpl {
	inline bool file_exists(const char* path) {
		return f_stat(path, NULL) == FR_OK;
	}

	bool read_file(const char* path, uint8_t*& dest, size_t& size, bool should_alloc = true) {
		/* ensure file exists first */
		if(!file_exists(path))
			return false;

		/* read entire file into buffer */
		FIL fp; 
		f_open(&fp, path, FA_READ);

		unsigned int len = f_size(&fp);

		if(should_alloc) {
			uint8_t* buffer = new uint8_t[len];
			dest = buffer;
		}

		logf("%s: reading %d bytes to 0x%X (allocated=%d) ...\n", path, len, (unsigned int)dest, should_alloc);

		size = len;

		f_read(&fp, dest, len, &len);
		f_close(&fp);

		return true;
	}

	inline bool read_file(const char* path, uint8_t*& dest, bool should_alloc = true) {
		size_t size;
		return read_file(path, dest, size, should_alloc);
	}

	void* load_fdt(const char* filename, uint8_t* cmdline) {
		/* read device tree blob */
		uint8_t* fdt = reinterpret_cast<uint8_t*>(MEM_USABLE_START);
		size_t sz;

		if(!read_file(filename, fdt, sz, false)) {
			panic("error reading fdt");
		}

                void* v_fdt = reinterpret_cast<void*>(fdt);

                int res;

		if ((res = fdt_check_header(v_fdt)) != 0) {
			panic("fdt blob invalid, fdt_check_header returned %d", res);
		}

                /* pass in command line args */
                fdt_setprop(v_fdt, 0, "chosen/cmdline", cmdline, strlen((char*) cmdline));

		logf("valid fdt loaded at 0x%X\n", (unsigned int)fdt);

		return reinterpret_cast<void*>(fdt);
	}

	LoaderImpl() {	
		logf("Mounting boot partitiion ...\n");
		FRESULT r = f_mount(&g_BootVolumeFs, ROOT_VOLUME_PREFIX, 1);
		if (r != FR_OK) {
			panic("failed to mount boot partition, error: %d", (int)r);
		}
		logf("Boot partition mounted!\n");

		size_t sz;

		/* dump cmdline.txt for test */
		uint8_t* cmdline;

		if(!read_file("cmdline.txt", cmdline, sz)) {
			panic("error reading cmdline");
		}

		/* nul terminate it */
		cmdline[sz - 1] = 0;
		logf("kernel cmdline: %s\n", cmdline);
		
		/* load flat device tree */
		void* fdt = load_fdt("rpi.dtb", cmdline);
		if (!fdt) {
			panic("fdt pointer is null");
		}

		/* after load_fdt is done with it, we don't need it anymore */
		delete[] cmdline;

		/* read the kernel -- necessarily at fixed address */
		uint8_t* zImage = reinterpret_cast<uint8_t*>(KERNEL_LOAD_ADDRESS);

		if(!read_file("zImage", zImage, false)) {
			panic("error reading zImage");
		}

		logf("zImage loaded at 0x%X\n", (unsigned int)zImage);
		logf("Jumping to the Linux kernel...\n");
		
		/* this should never return */
		boot_linux(0, ~0, fdt, zImage);
	}
};

static LoaderImpl STATIC_APP g_Loader {};
