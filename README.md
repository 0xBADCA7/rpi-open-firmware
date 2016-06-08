# Minimal Raspberry Pi VPU firmware
This is a small firmware for RPi VPU (VideoCore4) versions 1/2/3 that is capable of initializing UART, VPU PLL (PLLC) and ARM itself. It's intended to be used instead of stock `bootcode.bin` on RPi's SD card. You **need** to have UART to see anything meaningful as far as output goes.

This has been tested on RPi1 Model B (Hynix PoP DDR), RPi 2 Model B and RPi 3 Model B (both Elpida DDR). 

If you want to contact me because you're interested in contributing, you can message `kristina` on Freenode, but I would suggest talking in `#raspberrypi-internals` instead.

All Broadcom headers are licensed under 3-Clause BSD License while the rest of this is under GPLv2+. See `LICENSE` for more information.

## Building

You need Julian Brown's VC4 toolchain to build this (https://github.com/puppeh/vc4-toolchain) as well as a arm-none-eabi-toolchain. You can tweak the paths to it in CROSS_COMPILE in `Makefile` (for VC4) and for ARM in `arm_chainloader/Makefile`. After you've done it, run `buildall.sh` and you should have a blob in `build/bootcode.bin`. 

## Technical Details
The firmware is split into two parts, a VC4 part and and ARM part. The VC4 part initializes PLLC and moves VPU over to it, and then brings up UART. It then performs SDRAM initialization, making SDRAM available at `0xC0000000` (uncached alias). The ARM loader will do ARM initialization and then copy the ARM bootloader that's embedded in it to the alias. It will then map it to `0x0` in ARM's memory space and start ARM. The code under `arm_chainloader` is what will run on the ARM. 

The current iteration of ARM chainloader (which I've still yet to commit) can actually initialize the eMMC controller and access the boot partition with a FAT driver. From here, it would be trivial to chainloaded the next stage bootloader *but* you still face the issues of missing a substantial portion of the drivers that were previously provided by the firmware.

The current makefiles in the ARM part of it aim at **RPi1** (ie. ARMv6) but they can be changed to ARMv7 if you want to build it for a newer model. I tested it on all RPi models and it works without any issues as far as I can tell (ARM can access peripherals and memory just fine as AXI supervisor). However it cannot access any secure peripherals (OTP/SDRAM/etc) since they appear to be on a separate bus accessible only to VC4.

**Beware:** This doesn't handle SMP at the moment so if you run this on RPi2 and above, all ARM cores will start executing the ARM bootloader code at the start which could cause problems. I will fix this soon.

## Issues/Fixes
 * PLL rate on ARM is slow, it's a bit annoying. Need to tweak the PLL rate later.
 * All SDRAM configurations (including 1GB work), however, I'm not certain whether they will be reliable or not . ARM detection works for all of the RPi models I got to test it on which was two RPi2s, 1 RPi1 and 1 RPi3.
 * SD card support kind of works now, can parse MBR, parse the FAT boot partition but it still needs to pull off a next stage bootloader of it (possibly u-boot/LittleKernel or maybe even just start Linux straight from bootcode.bin).
 * The VPU still needs some sort of a firmware since ARM can't access all registers by itself, at the moment I'm trying to figure out how the VC4 interrupt controller works and how I can wake the sleeping VPU from ARM and get it to do things via a mailbox.

## Does/Will it boot Linux?

Eventually maybe. Since `start.elf` is responsible for clock and power management (all registers in the `cpr` block), these drivers will have to be rewritten on ARM (or even on the open source VC4) to have any meaningful peripherals working properly (for example HDMI/DMA/Pixel Valve/Whatever). You can boot a very minimal version of Linux without the firmware and get it to work with UART and some USB devices, but you can expect half of the things to be broken (most importantly, video and DMA).

## Thanks To
 * **Herman Hermitage** for his VC4 documentation and for helping me with working out suitable ARM PLL configurations.
 * **Julian Brown** for reviewing my code and for his awesome VC4 toolchain.
 * **David Given** for his initial LLVM project which I used as the base for my LLVM toolchain before moving to GCC.
 * **phire** for reviewing my code.
 * **Broadcom** for their header release.
 * Various other people not mentioned here.
