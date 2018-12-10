#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "types.h"

#define PIT_CHANNEL_0 0x40
#define PIT_COMMAND 0x43
#define PIT_MODE_3 0x36
#define PIT_IRQ 0
#define FREQUENCY 11932
#define FREQ_MASK 0xFF

uint32_t scheduled_processes[3];

uint32_t current_running_idx;

void pit_init(void);

#endif
