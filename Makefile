
# Defines the part type that this project uses.
#

# VERBOSE
#DEBUG=1

PART=TM4C1294NCPDT

#
# The base directory for TivaWare.
#
ROOT=.
APP=app

#
# Include the common make definitions.
#
include ${ROOT}/makedefs

#
# Where to find source files that do not live in this directory.
#
VPATH=${ROOT}/third_party/lwip-1.4.1/apps
VPATH+=${ROOT}/third_party/FreeRTOS/Source/portable/GCC/ARM_CM4F
VPATH+=${ROOT}/third_party/FreeRTOS/Source/portable/MemMang/
VPATH+=${ROOT}/third_party/FreeRTOS/Source
VPATH+=${APP}/c
VPATH+=${ROOT}/utils
VPATH+=${ROOT}/drivers

#
# Where to find header files that do not live in the source directory.
#
IPATH=.
IPATH+=${ROOT}
IPATH+=${ROOT}/third_party/FreeRTOS/Source/portable/GCC/ARM_CM4F
IPATH+=${ROOT}/third_party/FreeRTOS
IPATH+=${ROOT}/third_party/FreeRTOS/Source/include
IPATH+=${APP}/h
IPATH+=${ROOT}/third_party/lwip-1.4.1/apps
IPATH+=${ROOT}/third_party/lwip-1.4.1/ports/tiva-tm4c129/include
IPATH+=${ROOT}/third_party/lwip-1.4.1/src/include
IPATH+=${ROOT}/third_party/lwip-1.4.1/src/include/ipv4
IPATH+=${ROOT}/third_party

#
# The default rule, which causes the Sample Ethernet I/O Control Application using lwIP to be built.
#
all: ${COMPILER}/out.axf
#
# The rule to clean out all the build products.
#
clean:
	@rm -rf ${COMPILER} ${wildcard *~}
	@mkdir -p ${COMPILER}

dow: all
	tools/lm4flash_64b ${COMPILER}/out.bin

#
# The rule to create the target directory.
#
${COMPILER}:
	@mkdir -p ${COMPILER}

#
# Rules for building the Sample Ethernet I/O Control Application using lwIP.
#
${COMPILER}/out.axf: ${COMPILER}/usr_flash.o
${COMPILER}/out.axf: ${COMPILER}/main.o
${COMPILER}/out.axf: ${COMPILER}/spi_phisical.o
${COMPILER}/out.axf: ${COMPILER}/events.o
${COMPILER}/out.axf: ${COMPILER}/schedule.o
${COMPILER}/out.axf: ${COMPILER}/leds_session.o
${COMPILER}/out.axf: ${COMPILER}/state_machine.o
${COMPILER}/out.axf: ${COMPILER}/gcode.o
${COMPILER}/out.axf: ${COMPILER}/telnet.o
${COMPILER}/out.axf: ${COMPILER}/clk.o
${COMPILER}/out.axf: ${COMPILER}/wdog.o
${COMPILER}/out.axf: ${COMPILER}/commands.o
${COMPILER}/out.axf: ${COMPILER}/esp8266.o
${COMPILER}/out.axf: ${COMPILER}/lwiplib.o
${COMPILER}/out.axf: ${COMPILER}/startup_${COMPILER}.o
${COMPILER}/out.axf: ${COMPILER}/cmdline.o
${COMPILER}/out.axf: ${COMPILER}/uartstdio.o
${COMPILER}/out.axf: ${COMPILER}/ustdlib.o
${COMPILER}/out.axf: ${COMPILER}/flash_pb.o
${COMPILER}/out.axf: ${COMPILER}/ringbuf.o

${COMPILER}/out.axf: ${COMPILER}/timers.o
${COMPILER}/out.axf: ${COMPILER}/heap_2.o
${COMPILER}/out.axf: ${COMPILER}/port.o
${COMPILER}/out.axf: ${COMPILER}/list.o
${COMPILER}/out.axf: ${COMPILER}/queue.o
${COMPILER}/out.axf: ${COMPILER}/tasks.o


${COMPILER}/out.axf: ${ROOT}/driverlib/${COMPILER}/libdriver.a
${COMPILER}/out.axf: ${APP}/ld/out.ld
SCATTERgcc_out=${APP}/ld/out.ld
ENTRY_out=ResetISR
CFLAGSgcc=-DTARGET_IS_TM4C129_RA0



LDFLAGS=-nostartfiles -gc-sections -Map=gcc/map.map --cref
#
# Include the automatically generated dependency files.
#
#ifneq (${MAKECMDGOALS},clean)
#-include ${wildcard ${COMPILER}/*.d} __dummy__
#endif
