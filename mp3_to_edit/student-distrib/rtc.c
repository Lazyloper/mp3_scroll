/* rtc.c - Functions to interact with the RTC chip
 * vim:ts=4 noexpandtab
 */
#include "i8259.h"
#include "rtc.h"
#include "lib.h"
#include "terminal.h"

// #define VIRTUAL_RTC

int32_t  rtc_freqs[10]={1024, 512, 256, 128, 64, 32, 16, 8, 4, 2}; //all the rates rtc can be set
// uint32_t  rtc_clap[3]={terminals[0].freq,terminals[1].freq,terminals[2].freq};


/*   rtc_init
 *   Inputs: none
 *   Return Value: none
 *   Function: Initialize the RTC chip (enable raising the interrupts and enable the corresponding IRQ busline)
 */
void rtc_init(void) {

#ifdef VIRTUAL_RTC

    cli();

    /* set the register B index and disable NMIs */
    outb(DISABLE_NMI + REGISTER_B, RTC_PORT);

    /* read current content in register B */
    uint8_t prev = inb(CMOS_PORT);

    /* set the register B index and disable NMIs */
    outb(DISABLE_NMI + REGISTER_B, RTC_PORT);

    /* set the 6th bit in register B to high */
    outb(prev | 0x40, CMOS_PORT); // 0x40 -- turn on the 6th bit on register B

    /* open the irq busline */
    enable_irq(RTC_IRQ); // 8 is the irq_num of RTC chip

    rtc_num_tick=0;
    rtc_interrupt_occur=0;
    rtc_clap_counter=0;

    //set frequency to 1024
    uint8_t rate= (uint8_t)(6);
    outb(DISABLE_NMI+REGISTER_A, RTC_PORT);
    rate = rate & 0x0F ;
    outb(rate, CMOS_PORT);

    rtc_open((uint8_t*)"rtc");
    printf("rtc init");

    sti();

#else

    cli();

    /* set the register B index and disable NMIs */
    outb(DISABLE_NMI + REGISTER_B, RTC_PORT);

    /* read current content in register B */
    uint8_t prev = inb(CMOS_PORT);

    /* set the register B index and disable NMIs */
    outb(DISABLE_NMI + REGISTER_B, RTC_PORT);

    /* set the 6th bit in register B to high */
    outb(prev | 0x40, CMOS_PORT); // 0x40 -- turn on the 6th bit on register B

    /* open the irq busline */
    enable_irq(RTC_IRQ); // 8 is the irq_num of RTC chip

    rtc_num_tick=0;
    rtc_interrupt_occur=0;
    printf("rtc init");

    sti();

#endif

}

/*   rtc_interrupt_handler
 *   Inputs: none
 *   Return Value: none
 *   Function: called when a RTC interrupt is raised
 */
void rtc_interrupt_handler(void) {

#ifdef VIRTUAL_RTC

    cli();

    /* Acknowlegement */
    send_eoi(RTC_IRQ);

    /* call the test_interrupts defined in lib.c */
    // test_interrupts();

    if(rtc_clap_counter==terminals[current_terminal_id].clap){
        rtc_interrupt_occur=1;
        rtc_clap_counter=0;
    }
    else{
        rtc_clap_counter++;
    }
    rtc_num_tick++;


    // printf("RTC\n");
    // putc('a');

    /* cast all the content of register C so that the next interrupt can be raised */
    outb(REGISTER_C, RTC_PORT);
    inb(CMOS_PORT);

    sti();

#else

    cli();

    /* Acknowlegement */
    send_eoi(RTC_IRQ);

    /* call the test_interrupts defined in lib.c */
    // test_interrupts();

    rtc_interrupt_occur=1;
    rtc_num_tick++;

    // printf("RTC\n");
    // putc('a');

    /* cast all the content of register C so that the next interrupt can be raised */
    outb(REGISTER_C, RTC_PORT);
    inb(CMOS_PORT);

    sti();

#endif

}

/*rtc_open
 *Description: Open the RTC
 *Inputs:      none
 *Outputs:     Return 0-Success
 *                    1-Failure
 *Effects:     Set the frequency of rtc to 2HZ
 */

int32_t rtc_open(const uint8_t* filename){

#ifdef VIRTUAL_RTC

    uint8_t rightrate= (uint8_t)(6);
    outb(DISABLE_NMI+REGISTER_A, RTC_PORT);
    rightrate = rightrate & 0x0F ;
    uint8_t readrate=inb(CMOS_PORT);
    if(readrate!=rightrate){
        outb(rightrate,CMOS_PORT);
    }

    rtc_reset();

    //reset frequency to 2
    int32_t a=rtc_set_freq(2);

    if(a==0){       //Success
        return 0;
    }
    return -1;      //Failure

#else

    rtc_reset();
    int32_t a=rtc_set_freq(2);     //Set the frequency to 2HZ(default).
    if(a==0){       //Success
        return 0;
    }
    return -1;      //Failure

#endif

}

/*rtc_read
 *Description: Read frequency of RTC
 *Inputs:      buf- whatever
 *             nbytes- whatever
 *Outputs:     Return 0-Success
 *                    1-Failure
 *Effects:     Read after interrupt occur and reset the flag
 */

int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){

#ifdef VIRTUAL_RTC

    while(!rtc_interrupt_occur){
        //KEEP SPIN AND WAIT
    }

    // if(cur_num_tick==rtc_num_tick){
    //     //something is wrong
    //     // printf("ERROR::Cannot figure out whether there is interrupt.");
    //     return -1;      //Theoritically this does not happen.
    // }
    // putc('a');

    rtc_interrupt_occur=0;

    return 0;

#else

    while(!rtc_interrupt_occur){
        //KEEP SPIN AND WAIT
    }

    // if(cur_num_tick==rtc_num_tick){
    //     //something is wrong
    //     // printf("ERROR::Cannot figure out whether there is interrupt.");
    //     return -1;      //Theoritically this does not happen.
    // }
    // putc('a');

    rtc_interrupt_occur=0;

    return 0;

#endif

}

/*rtc_write
 *Description: Set RTC frequency
 *Inputs:      buf- hold the value of frequency to be set
 *             nbytes- need to be 4 to work
 *Outputs:     none
 *Effects:     Set RTC frequency to the value in buf
 */

int32_t rtc_write(int32_t fd ,const void* buf, int32_t nbytes){

#ifdef VIRTUAL_RTC
    if(nbytes!=4 || buf==NULL){
        return -1;
    }
    terminals[current_terminal_id].freq=(*(int32_t*)buf);
    return rtc_set_freq(*(int32_t*)buf);

#else

    if(nbytes!=4 || buf==NULL){     //if nbytes is not four or buf is a NULL Pointer
        return -1;                  //Failure
    }

    return rtc_set_freq(*(int32_t*)buf);

#endif

}

/*rtc_close
 *Description: close rtc
 *Inputs:      fd-file descriptor (no use)
 *Outputs:     none
 *Effects:     Set RTC frequency to the value in buf
 */

int32_t rtc_close(int32_t fd){
    return 0;
}

/*rtc_set_freq
 *Description: Helper function of rtc_write and rtc_open
 *Inputs:      freq- frequency to be set
 *Outputs:     Return 0- Success
                      1- Failure
 *O
 */

int32_t rtc_set_freq(int32_t freq){

#ifdef VIRTUAL_RTC

    //check freq to set
    int index = -1;
    int i;
    for (i=0; i< 10; i++){  //check if in the array, if inside, locate
        if (freq == rtc_freqs[i]){
            index=i;
        }
    }
    if(index==-1){      //if not inside print error
        printf("Wrong frequency input!!!!!!\n");
        return -1;
    }

    terminals[current_terminal_id].clap= MAX_FREQ/freq-1;
    return 0;

#else

    //check freq to set
    int index = -1;
    int i;
    for (i=0; i< 10; i++){  //check if in the array, if inside, locate
        if (freq == rtc_freqs[i]){
            index=i;
        }
    }
    if(index==-1){      //if not inside print error
        printf("Wrong frequency input!!!!!!\n");
        return -1;
    }

    uint8_t rate= (uint8_t)(index+6);           //rate is freq index + 6
    outb(DISABLE_NMI+REGISTER_A, RTC_PORT);
    rate = rate & 0x0F ;                        //set the first four digits to 1
    outb(rate, CMOS_PORT);
    return 0;   //Success

#endif

}

/*rtc_reset
 *Description: reset the tick number
 */

void rtc_reset(){

#ifdef VIRTUAL_RTC

    rtc_num_tick=0;
    rtc_clap_counter=0;
    return;

#else

  //printf("rtc reset\n");
  rtc_num_tick=0;   //reset tick
  return;

#endif

}
