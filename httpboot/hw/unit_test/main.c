#include "unit_test.h"

extern void setup();

int main() {
    int err = 0;
    err += test_tcp();
    return err;
}
