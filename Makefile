#!/usr/bin/make

default:
	@echo Use the appropriate platform specific Makefile: Makefile.SAM3X8_DUE, Makefile.STM32F4, Makefile.STM32F4_POG, Makefile.STM32F4_EDA.

clean:
	$(MAKE) -f Makefile.SAM3X8_DUE clean
	$(MAKE) -f Makefile.STM32F4 clean
	$(MAKE) -f Makefile.STM32F4_POG clean
	$(MAKE) -f Makefile.STM32F4_EDA clean


.FORCE:
