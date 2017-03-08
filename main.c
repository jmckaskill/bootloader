#include <hw/hw_gpio_v2.h>

static int g_foo = 1;
static int g_foo2;

#define HWREG(base, x) (*((volatile unsigned int *)((char*) base + x)))

int main(void) {
    // boot-button is 2.8
    // leds are 1.21-24
    void *gpio = (void*) 0x4804c000;

    // enable output for pin 21
    //HWREG(gpio, GPIO_OE) = ~(1 << 21);
    HWREG(gpio, GPIO_SETDATAOUT) = 1 << 23;

    for (;;) {
        for (int i = 0; i < 1000000; i++) {

        }
        HWREG(gpio, GPIO_DATAOUT) = ~HWREG(gpio, GPIO_DATAOUT);
    }
    return 0;
}