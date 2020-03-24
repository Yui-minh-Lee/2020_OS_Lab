#include "lib.h"
#include "types.h"

//int data = 0;

int uEntry(void)
{

	//asm volatile("cli"); //XXX test for CPL, will cause a #GP
	//asm volatile("int $0xe"); //XXX test for SWInterrupt/exception:fault, current instruction address is pushed into kernel stack, if idt[0xe].dpl>=cpl, o.w., cause #GP
	//asm volatile("int $0x80");//XXX test for SWInterrupt, next instruction address is pushed into kernel stack, if idt[0x80].dpl>=cpl, o.w., cause #GP
	//asm volatile("int 3 ...");//XXX equivalent to int 0x3?
	//asm volatile("into ...");//XXX equivalent to int 0x4?
	//asm volatile("bound ...");//XXX equivalent to int 0x5?
	//printf("1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n");
	//printf("Scroll Screen Test.\n");
	//data = 'H';
	//printf("%c\n", data);

	printf("printf test begin...\n");
	printf("the answer should be:\n");
	printf("#######################################################\n");
	printf("Hello, welcome to OSlab! I'm the body of the game. ");
	printf("Bootblock loads me to the memory position of 0x100000, and Makefile also tells me that I'm at the location of 0x100000.");
	printf("\\%%~!@#/(^&*()_+`1234567890-=...... ");
	printf("Now I will test your printf: ");
	printf("1 + 1 = 2, 123 * 456 = 56088\n0, -1, -2147483648, -1412567295,-32768, 102030\n0, ffffffff, 80000000, abcdef01, ffff8000, 18e8e\n");
	printf("#######################################################\n");
	printf("your answer:\n");
	printf("=======================================================\n");
	printf("%s %s%scome %co%s", "Hello,", "", "wel", 't', " ");
	printf("%c%c%c%c%c! ", 'O', 'S', 'l', 'a', 'b');
	printf("I'm the %s of %s. %s 0x%x, %s 0x%x. ", "body", "the game", "Bootblock loads me to the memory position of", 0x100000, "and Makefile also tells me that I'm at the location of", 0x100000);
	printf("\\%%~!@#/(^&*()_+`1234567890-=...... ");
	printf("Now I will test your printf: ");
	printf("%d + %d = %d, %d * %d = %d\n", 1, 1, 1 + 1, 123, 456, 123 * 456);
	printf("%d, %d, %d, %d, %d, %d\n", 0, 0xffffffff, 0x80000000, 0xabcdef01,
		   -32768, 102030);
	printf("%x, %x, %x, %x, %x, %x\n", 0, 0xffffffff, 0x80000000, 0xabcdef01,
		   -32768, 102030);
	printf("=======================================================\n");
	printf("Test end!!! Good luck!!!\n");
	while (1)
		;
	return 0;
}
