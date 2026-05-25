#include "io.h"
#include <fat.h>

bool ioInitFAT(void) {
    return fatInitDefault();
}
