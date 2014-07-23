#!/bin/bash
# A short script to compile the three events called by read_dmesg.sh
# Note: compiled events should be removed before these files are transferred to BBB

gcc eventA.c ../../utilities/spi/spi.c -o eventA
gcc eventB.c ../../utilities/spi/spi.c -o eventB
gcc eventAB.c ../../utilities/spi/spi.c -o eventAB
