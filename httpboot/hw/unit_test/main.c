#include "unit_test.h"

extern void setup();

int main() {
    setup();
    test_tcp();
    return 0;
}
