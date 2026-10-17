#ifndef PIT_H
#define PIT_H

#include <stdint.h>
#include "../io.h"

void pit_count(uint8_t irq, uint64_t count);

#endif //PIT_H