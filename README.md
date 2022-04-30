# Digital Voice Modem Firmware

The DVM firmware provides the embedded microcontroller implementation of a mixed-mode DMR/P25 or dedicated-mode DMR or P25 repeater system. The firmware; is the portion of a complete Over-The-Air modem implementation that connects directly to an appropriate air interface (usually a analog radio) and performs the actual reception and creation of the digitial waveforms.

This project is a direct fork of the MMDVM (https://github.com/g4klx/MMDVM) project.

Please feel free to reach out to us for help, comments or otherwise, on our Discord: https://discord.gg/3pBe8xgrEz

## Building

Please see the various Makefile's included in the project for more information. This project includes a few Makefiles to target different hardware. (All following information assumes familiarity with the standard Linux make system.)

* Makefile.SAM3X8_DUE - This makefile is used for building binaries to flash onto a Arduino Due device.
* Makefile.STM32F4 - This makefile is used for targeting a generic STM32F4 device.
* Makefile.STM32F4_POG - This makefile is used for targeting the STM32F4 device built by RepeaterBuilder (http://www.repeater-builder.com/products/stm32-dvm.html).
* Makefile.STM32F4_EDA - This makefile is used for targeting the "v3" STM32F4 405 or 446 device built by WA0EDA for the MTR2000 and MASTR 3.

All of these firmwares should be compiled on Linux, any other systems YMMV. 

* For the standard Arduino Due device, you should have the Arduino development SDK installed to your home directory under ".arduino15". There are some minor tweaks that must be performed to compile for this platform:
  1. Locate platform.txt. On an Ubuntu OS it should be in:
     /home/$user/.arduino15/packages/arduino/hardware/sam/1.6.8/
  2. Open the file in a text editor and change the line:
```
# Combine gc-sections, archives, and objects
recipe.c.combine.pattern="{compiler.path}{compiler.c.elf.cmd}" -mcpu={build.mcu} -mthumb {compiler.c.elf.flags} "-T{build.variant.path}/{build.ldscript}" "-Wl,-Map,{build.path}/{build.project_name}.map" {compiler.c.elf.extra_flags} -o "{build.path}/{build.project_name}.elf" "-L{build.path}" -Wl,--cref -Wl,--check-sections -Wl,--gc-sections -Wl,--entry=Reset_Handler -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-section-align -Wl,--start-group {compiler.combine.flags} {object_files} "{build.variant.path}/{build.variant_system_lib}" "{build.path}/{archive_file}" -Wl,--end-group -lm -gcc
```

To:
```
recipe.c.combine.pattern="{compiler.path}{compiler.c.elf.cmd}" -mcpu={build.mcu} -mthumb {compiler.c.elf.flags} "-T{build.variant.path}/{build.ldscript}" "-Wl,-Map,{build.path}/{build.project_name}.map" {compiler.c.elf.extra_flags} -o "{build.path}/{build.project_name}.elf" "-L{build.path}" -Wl,--cref -Wl,--check-sections -Wl,--gc-sections -Wl,--entry=Reset_Handler -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-section-align -Wl,--start-group {compiler.combine.flags} {object_files} "{build.variant.path}/{build.variant_system_lib}" "{build.system.path}/CMSIS/CMSIS/Lib/GCC/libarm_cortexM3l_math.a" "{build.path}/{archive_file}" -Wl,--end-group -lm -gcc
```
* For STM32F4 using Ubuntu OS install the standard ARM embedded toolchain (typically arm-gcc-none-eabi).
  - Make sure to clone this repository with the ```--recurse-submodules``` option, otherwise the STM32 platform files will be missing! ```git clone --recurse-submodules https://github.com/DVMProject/dvmfirmware.git```

Use the ```make``` command to build the firmware, choosing the appropriate makefile with the -F switch.

## Firmware installation

Some devices can be used on top on a RPi attached via the GPIO port or standalone and others are connected via USB. Both variants require different handling of compiling and uploading the firmware, examples on flashing devices are mostly not included here because the methods to flash vary from device to device.

## License

This project is licensed under the GPLv2 License - see the [LICENSE.md](LICENSE.md) file for details. Use of this project is intended, for amateur and/or educational use ONLY. Any other use is at the risk of user and all commercial purposes is strictly discouraged.

