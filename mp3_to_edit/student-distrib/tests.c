#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "isr.h"
#include "terminal.h"
#include "keyboard.h"
#include "filesyshelper.h"
#include "rtc.h"

#define PASS 1
#define FAIL 0
#define VIDEO_MEM_ADDR_START	0xB8000
#define VIDEO_MEM_ADDR_END		0xB8FFF
#define KERNAL_MEM_ADDR_START 0x400000
#define KERNAL_MEM_ADDR_END	  0x7FFFFF

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 *
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) &&
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// add more tests here

/*
 * test exception DIVIDE_ERROR
 * 	trigger a DIVIDE_ERROR
 * 	the system should stop
 */
	int DIVIDE_ERROR_test(){
		int result = PASS;
		test_DIVIDE_ERROR();
		return result;
	}


/*
 * test exception OVERFLOW
 * 	trigger a DIVIDE_ERROR
 * 	the system should stop
 */
	int OVERFLOW_test(){
		int result = PASS;
		test_OVERFLOW();
		return result;
	}
/*
 * test exception OVERFLOW
 * 	trigger a DIVIDE_ERROR
 * 	the system should stop
*/
	int STACK_SEGMENT_FAULT_test(){
		int result = PASS;
		test_STACK_SEGMENT_FAULT();
		return result;
	}

/*
 * test boundary at VIDEO_MEM
 * 	-return PASS on valid dereference
 *	-throw PAGE FAULT exception on invalid
 * SHOULD RETURN PASS
 */
int page_test1() {
	uint8_t* page_ptr = (uint8_t*)(VIDEO_MEM_ADDR_START);
	// uint32_t* page_ptr = &file_sys_addr;
	uint32_t a;
	a = *page_ptr;
	// printf(" addr=%x\n", file_sys_addr);
	return PASS;
}
/*
 * test boundary at VIDEO_MEM
 * 	-return PASS on valid dereference
 *	-throw PAGE FAULT exception on invalid
 * SHOULD RETURN PASS
 */
int page_test2() {
	uint8_t* page_ptr = (uint8_t*)(VIDEO_MEM_ADDR_END);
	uint8_t a;
	a = *page_ptr;
	return PASS;
}
/*
 * test boundary at VIDEO_MEM
 * 	-return PASS on valid dereference
 *	-throw PAGE FAULT exception on invalid
 * SHOULD RETURN PASS
 */
int page_test3() {
	uint8_t* page_ptr = (uint8_t*)(KERNAL_MEM_ADDR_START);
	uint8_t a;
	a = *page_ptr;
	return PASS;
}
/*
 * test boundary at VIDEO_MEM
 * 	-return PASS on valid dereference
 *	-throw PAGE FAULT exception on invalid
 * SHOULD RETURN PASS
 */
int page_test4() {
	uint8_t* page_ptr = (uint8_t*)(KERNAL_MEM_ADDR_END);
	uint8_t a;
	a = *page_ptr;
	return PASS;
}
/*
 * test boundary at VIDEO_MEM
 * 	-return PASS on valid dereference
 *	-throw PAGE FAULT exception on invalid
 * SHOULD INVOKE EXCEPTION
 */
int page_test5() {
	uint8_t* page_ptr = (uint8_t*)(VIDEO_MEM_ADDR_START - 1);
	uint8_t a;
	a = *page_ptr;
	return PASS;
}
/*
 * test boundary at VIDEO_MEM
 * 	-return PASS on valid dereference
 *	-throw PAGE FAULT exception on invalid
 * SHOULD INVOKE EXCEPTION
 */
int page_test6() {
	uint8_t* page_ptr = (uint8_t*)(VIDEO_MEM_ADDR_END + 1);
	uint8_t a;
	a = *page_ptr;
	return PASS;
}
/*
 * test boundary at VIDEO_MEM
 * 	-return PASS on valid dereference
 *	-throw PAGE FAULT exception on invalid
 * SHOULD INVOKE EXCEPTION
 */
int page_test7() {
	uint8_t* page_ptr = (uint8_t*)(KERNAL_MEM_ADDR_END + 1);
	uint8_t a;
	a = *page_ptr;
	return PASS;
}

//-------------- Checkpoint 2 tests ------------------/

/*
 * test file system read file
 * read the file frame1.txt and print it
 * should return PASS
 */
int read_file_test1(){
	clear();
	uint8_t fname[33] = "frame1.txt";					//allocate a buf(33) to store its name
	file_open(fname);
	int8_t buf[6000];													//allocate enough(6000) space to store the date
	int32_t i, count;
	count = file_read(1, buf, 6000);					//6000 is the size of the buf
	printf("Bytes read: %d\n", count);
	for (i = 0; i < count; i++) {
			if (buf[i] != '\0') {
				putc(buf[i]);
			}
	}
	file_close(0);
	return PASS;
}

/*
 * test file system read file
 * read the file verylargetextwithverylongname.txt and print it
 * should return PASS
 */
int read_file_test2(){
	clear();
	uint8_t fname[33] = "verylargetextwithverylongname.txt";  //allocate a buf(33) to store its name
	file_open(fname);
	int8_t buf[6000];																					//allocate enough(6000) space to store the data
	int32_t i, count, fd;
	fd = 0;
	count = file_read(fd, buf, 6000);													//6000 is the size of the buf
	printf("Bytes read: %d\n", count);
	for (i = 0; i < count; i++) {
			if (buf[i] != '\0') {
				putc(buf[i]);
			}
	}
	file_close(0);
	return PASS;
}

/*
 * test file system read file
 * read the file cat and print it
 * should return PASS
 */
int read_file_test3(){
	clear();
	uint8_t fname[33] = "ls";							 //allocate a buf(33) to store its name
	file_open(fname);
	int8_t buf[6000];												//allocate enough(6000) space to store the data
	int32_t i, count;
	count = file_read(1, buf, 6000);				//6000 is the size of the buf
	printf("Bytes read: %d\n", count);
	for (i = 0; i < count; i++) {
			if (buf[i] != '\0') {
				putc(buf[i]);
			}
	}
	file_close(0);
	return PASS;
}

/*
 * test file system read file
 * read the file cat and print it
 * should return PASS
 */
int read_file_test(uint32_t index){
	clear();
	read_dentry_by_index(index, &file_dentry);							 //allocate a buf(33) to store its name

	int8_t buf[6000];												//allocate enough(6000) space to store the data
	int32_t i, count;
	if (file_dentry.file_type == 1) {
			file_open(file_dentry.file_name);
			dir_read(0, buf, 0);
			dir_close(0);
	} else {
			file_open(file_dentry.file_name);
			file_read(0, buf, 0);
			count = file_read(1, buf, 6000);				//6000 is the size of the buf
			printf("Bytes read: %d\n", count);
			for (i = 0; i < count; i++) {
					if (buf[i] != '\0') {
						putc(buf[i]);
					}
			}
			file_close(0);
	}
	return PASS;
}


/*
 * test file system read directory
 * read the directory "." and print all file with name, type and inode number
 * should return PASS
 */
int read_dir_test(){
	clear();
	uint8_t fname[33] = ".";								 //allocate a buf(33) to store its name
	int8_t buf[33];													//allocate enough(33) space to store the data
	dir_open(fname);
	dir_read(0, buf, 0);
	dir_close(0);
	return PASS;
}

int terminal_write_test1() {
	uint8_t test1[12] = "I am Groot.\n";
	if (terminal_write(0, test1, 13) == 12)
		return PASS;
	else
		return FAIL;
}

int terminal_read_test1() {
	uint8_t test1[MAX_INPUT_LENGTH];
	while (1) {
		printf("input something \n");
		int input_length = terminal_read(0, test1, MAX_INPUT_LENGTH);
		test1[input_length] = '\0';
		printf("input_length: %d, input string: %s\n", input_length, test1);
	}
	return PASS;
}

int terminal_read_test2() {
	uint8_t test2[MAX_INPUT_LENGTH];
	int input_length = terminal_read(0, test2, 3);
	printf("input_length, %d \n", input_length);
	if (input_length > 0) {
		int output_length = terminal_write(0, test2, input_length);
		printf("output_length, %d \n", output_length);
		if (output_length == input_length)
			return PASS;
	}
	return FAIL;
}

/*
 * test rtc open
 * open the rtc and will set the frequency to 2 Hz
 * should return PASS
 */
int rtc_open_test(){
	rtc_open((uint8_t*)(0));
	return PASS;
}

/*
 * test rtc write
 * set the frequency to 512 Hz
 * should return PASS and the frequency will change
 * use print('a') in rtc handler to view the change
 */
int rtc_write_test(){
	rtc_open((uint8_t*)(0));
	int32_t buf[10];			// allocate enough space for buff
	buf[0] = 32;					//set the frequency to 512
	rtc_write(0, buf, 4);	//4 is the length to be written
	return PASS;
}

/*
 * test rtc
 * change the frequency into 16, 512, 128 and 75 and view what will happen
 * should return PASS and the frequency will change
 * use print('a') in rtc handler to view the change
 */
int rtc_test(){
	//rtc_init();
	int ret;
	ret = 32;
	rtc_open((uint8_t*)(0));
	rtc_write(0, &ret, 4);
	while (1){
		putc('a');
		rtc_read(0, &ret, 4);
	}
	// int32_t freqs[4]={16, 512, 128,75};				// the freqs will change to 16, 512 and 128.
	// int32_t i;
	// i=0;
	// while(i<4){
	// 	if(rtc_num_tick>50){										//for each frequency, just call RTC handler for 50 times
	// 		printf("\nthe freqs change to : %d\n", freqs[i]);
	// 		rtc_reset();
	// 		rtc_write(0, &freqs[i], 4);
	// 		i++;
	// 	}
	// }
	return PASS;
}
/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
	clear();
	// TEST_OUTPUT("teminal_write_test1()", teminal_write_test1());
	// TEST_OUTPUT("DIVIDE_ERROR_test()", DIVIDE_ERROR_test());
	// TEST_OUTPUT("OVERFLOW_test", OVERFLOW_test());
	// TEST_OUTPUT("STACK_SEGMENT_FAULT_test", STACK_SEGMENT_FAULT_test());
	// TEST_OUTPUT("page_test1", page_test1());
 	// TEST_OUTPUT("page_test2", page_test2());
	// TEST_OUTPUT("page_test3", page_test3());
	// TEST_OUTPUT("page_test4", page_test4());
	// TEST_OUTPUT("page_test5", page_test5());
	// TEST_OUTPUT("page_test6", page_test6());
	// TEST_OUTPUT("page_test7", page_test7());
	// TEST_OUTPUT("read_file_test1()", read_file_test1());
	// TEST_OUTPUT("read_file_test2()", read_file_test2());
	// TEST_OUTPUT("read_file_test3()", read_file_test3());
	// TEST_OUTPUT("read_file_test()", read_file_test(9));
	// TEST_OUTPUT("read_file_test()", read_file_test(10));
	// TEST_OUTPUT("read_dir_test()", read_dir_test());
	// TEST_OUTPUT("rtc_open_test()", rtc_open_test());
	// TEST_OUTPUT("rtc_write_test()", rtc_write_test());
	// TEST_OUTPUT("rtc_test()", rtc_test());
	// TEST_OUTPUT("terminal_read_test1()", terminal_read_test1());
	// TEST_OUTPUT("terminal_write_test1()", terminal_write_test1());
}
