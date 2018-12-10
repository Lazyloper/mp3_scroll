/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xFF; /* IRQs 0-7  */
uint8_t slave_mask = 0xFF;  /* IRQs 8-15 */

/*   i8529_init
 *   Inputs: none
 *   Return Value: none
 *   Function: initialize both master and slave pics
 */
void i8259_init(void) {

    unsigned long flags;

    cli_and_save(flags);

    outb(0xff, PIC1_DATA); // 0xff -- mask all of 8259A-1
    outb(0xff, PIC2_DATA); // 0xff -- mask all of 8259A-2

    /* initialize the master pic */
    outb(ICW1, PIC1_COMMAND);
    outb(ICW2_MASTER, PIC1_DATA);
    outb(ICW3_MASTER, PIC1_DATA);
    outb(ICW4, PIC1_DATA); // master expects normal EOI

    /* initialize the slave pic */
    outb(ICW1, PIC2_COMMAND);
    outb(ICW2_SLAVE, PIC2_DATA);
    outb(ICW3_SLAVE, PIC2_DATA);
    outb(ICW4, PIC2_DATA);

    /* open the slave irq busline */
    enable_irq(SLAVE_IRQ); // 2 -- the irq_num of slave on the master

    sti();
    restore_flags(flags);
}

/*   enable_irq
 *   Inputs: irq_num -- the number of IRQ busline to be enabled
 *   Return Value: none
 *   Function: Enable (unmask) the specified IRQ
 */
void enable_irq(uint32_t irq_num) {
    uint16_t port;


    if (irq_num < IRQ_TOTAL) { /* 8 -- the number of irqs on one pic */

        /* if the irq is on master pic */
        port = PIC1_DATA;

        /* update the mask*/
        master_mask = master_mask & ~(1 << irq_num);

        /* output to the data port */
        outb(master_mask, port);
    } else {
        /* if the irq is on slave pic */
        port = PIC2_DATA;

        /* adapt the irq number to the range 0 ~ 8 */
        irq_num -= IRQ_TOTAL;

        /* update the mask */
        slave_mask = slave_mask & ~(1 << irq_num);

        /* output to the data port */
        outb(slave_mask, port);
    }
}

/*   disable_irq
 *   Inputs: irq_num -- the number of IRQ busline to be disabled
 *   Return Value: none
 *   Function: Disable (mask) the specified IRQ
 */
void disable_irq(uint32_t irq_num) {
    uint16_t port;

    if (irq_num < IRQ_TOTAL) {
        /* if the irq is on master pic */
        port = PIC1_DATA;

        /* update the mask */
        master_mask = master_mask | (1 << irq_num);

        /* output to the data port */
        outb(master_mask, port);

    } else {
        /* if the irq is on slave pic */
        port = PIC2_DATA;

        /* adapt the irq number to the range 0 ~ 8 */
        irq_num -= IRQ_TOTAL;

        /* update the mask */
        slave_mask = slave_mask | (1 << irq_num);

        /* output to the data port */
        outb(master_mask, port);
    }
}

/*   send_eoi
 *   Inputs: irq_num -- the number of IRQ busline to be sent eoi
 *   Return Value: none
 *   Function: Send end-of-interrupt signal for the specified IRQ
 */
void send_eoi(uint32_t irq_num) {

    if(irq_num >= IRQ_TOTAL) {
        /* if the irq is on slave pic */
        outb(EOI | (irq_num - IRQ_TOTAL), PIC2_COMMAND);
        outb(EOI | SLAVE_IRQ, PIC1_COMMAND);
    }

    /* if the irq is on master pic */
    outb(EOI | irq_num, PIC1_COMMAND);
}
