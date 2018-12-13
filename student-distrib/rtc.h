/* rtc.h - Defines used in interactions with the RTC chip
 * vim:ts=4 noexpandtab
 */
#ifndef _RTC_H
#define _RTC_H

#include "types.h"
#include "terminal.h"

/* define Ports and Register index */
#define RTC_PORT 0x70
#define CMOS_PORT 0x71
#define REGISTER_A 0x0A
#define REGISTER_B 0x0B
#define REGISTER_C 0x0C
#define DISABLE_NMI 0x80
#define RTC_IRQ 8
#define MAX_FREQ 1024


// tick and counter used for virtual_RTC
volatile uint32_t rtc_num_tick_virtual[TERMINAL_NUMBER];
volatile int32_t rtc_interrupt_occur_virtual[TERMINAL_NUMBER];
volatile uint32_t rtc_clap_counter_virtual[TERMINAL_NUMBER];
// real rtc
volatile uint32_t rtc_num_tick;
volatile int32_t  rtc_interrupt_occur;

/* Externally-visible functions */

/* Initialize the RTC chip (enable raising the interrupts and enable the corresponding IRQ busline)  */
void rtc_init(void);

/* called when a RTC interrupt is raised */
extern void rtc_interrupt_handler(void);

int32_t rtc_open(const uint8_t* filename);

int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

int32_t rtc_close(int32_t fd);

int32_t rtc_set_freq(int32_t freq);

void rtc_reset();

#endif
