#include "common.h"

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/pin_map.h"

#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

#define LED_R GPIO_PIN_1
#define LED_B GPIO_PIN_2
#define LED_G GPIO_PIN_3
#define ALL_LEDS (LED_R|LED_B|LED_G)

int main(void) {
    SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));

    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, ALL_LEDS);

    while (true) {
        GPIOPinWrite(GPIO_PORTF_BASE, ALL_LEDS, LED_R);
        SysCtlDelay(0xF00000);
        GPIOPinWrite(GPIO_PORTF_BASE, ALL_LEDS, LED_B);
        SysCtlDelay(0xF00000);
    }

    return 0;
}
