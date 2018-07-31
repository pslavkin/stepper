#include <stdbool.h>
#include <stdint.h>
#include "driverlib/sysctl.h"
#include "driverlib/rom_map.h"

#include "FreeRTOS.h"
 
void Init_Clk(void)
{
   // Make sure the main oscillator is enabled because this is required 
   // the PHY.  The system must have a 25MHz crystal attached to the OSC
   // pins.  The SYSCTL_MOSC_HIGHFREQ parameter is used when the crystal
   // frequency is 10MHz or higher.
   //
   SysCtlMOSCConfigSet(SYSCTL_MOSC_HIGHFREQ);
   //
   // Run from the PLL at 120 MHz.
   //
   MAP_SysCtlClockFreqSet( \
               SYSCTL_XTAL_25MHZ    |  \
               SYSCTL_OSC_MAIN      |  \
               SYSCTL_USE_PLL       |  \
               SYSCTL_CFG_VCO_480,     \
               configCPU_CLOCK_HZ );
}
