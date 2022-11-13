
# Define out program
SKT_PDE_SRC = XnodeMegaFlash.cpp

# This needs more flash so use mega board
BOARD = megaADK

# Define the program to flash (needs single file)
MASTER_FLASH_PROG = ../xnode-base/build/xnode-base.hex

# Defines extra pairs of boot+image to flash(note optiboot get replaces by optiboot hex)
MASTER_FLASH_PAIR_EXTRA = \
optiboot \
../xnode-satellite/build/xnode-satellite.hex \
optiboot \
../xnode-test-blink/build/xnode-test-blink.hex \
optiboot \
../xnode-satellite/build/xnode-satellite.hex

# Include our master flash build
include ../lib-build/make/Makefile.master-flash
 
