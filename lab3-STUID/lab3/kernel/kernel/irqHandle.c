#include "x86.h"
#include "device.h"

extern TSS tss;
extern ProcessTable pcb[MAX_PCB_NUM];
extern int current;

extern int displayRow;
extern int displayCol;

extern uint32_t keyBuffer[MAX_KEYBUFFER_SIZE];
extern int bufferHead;
extern int bufferTail;

void syscallHandle(struct TrapFrame *tf);
void syscallWrite(struct TrapFrame *tf);
void syscallPrint(struct TrapFrame *tf);
void syscallFork(struct TrapFrame *tf);
void syscallExec(struct TrapFrame *tf);
void syscallSleep(struct TrapFrame *tf);
void syscallExit(struct TrapFrame *tf);

void GProtectFaultHandle(struct TrapFrame *tf);

void timerHandle(struct TrapFrame *tf);
void keyboardHandle(struct TrapFrame *tf);

void irqHandle(struct TrapFrame *tf)
{ // pointer tf = esp
	/*
	 * 中断处理程序
	 */
	/* Reassign segment register */
	asm volatile("movw %%ax, %%ds" ::"a"(KSEL(SEG_KDATA)));

	uint32_t tmpStackTop = pcb[current].stackTop;
	pcb[current].prevStackTop = pcb[current].stackTop;
	pcb[current].stackTop = (uint32_t)tf;

	switch (tf->irq)
	{
	case -1:
		break;
	case 0xd:
		GProtectFaultHandle(tf); // return
		break;
	case 0x20:
		timerHandle(tf); // return or iret
		break;
	case 0x21:
		keyboardHandle(tf); // return
		break;
	case 0x80:
		syscallHandle(tf); // return
		break;
	default:
		assert(0);
	}

	pcb[current].stackTop = tmpStackTop;
}

void syscallHandle(struct TrapFrame *tf)
{
	switch (tf->eax)
	{ // syscall number
	case 0:
		syscallWrite(tf);
		break; // for SYS_WRITE
	case 1:
		syscallFork(tf);
		break; // for SYS_FORK
	case 2:
		syscallExec(tf);
		break; // for SYS_EXEC
	case 3:
		syscallSleep(tf);
		break; // for SYS_SLEEP
	case 4:
		syscallExit(tf);
		break; // for SYS_EXIT
	default:
		break;
	}
}

void timerHandle(struct TrapFrame *tf)
{
	// TODO in lab3 done
	uint32_t tmpStackTop;
	int pid;
	for (int i = 1; i < MAX_PCB_NUM; i++)
	{
		pid = (i + current) % MAX_PCB_NUM;
		if (pcb[pid].state == STATE_BLOCKED && pcb[pid].sleepTime > 0)
		{
			--pcb[pid].sleepTime;
			if (pcb[pid].sleepTime == 0)
			{
				pcb[pid].state = STATE_RUNNABLE;
			}
		}
	}

	if (pcb[current].state == STATE_RUNNING && pcb[current].timeCount < MAX_TIME_COUNT)
	{
		++pcb[current].timeCount;
	}
	else
	{
		if (pcb[current].state == STATE_RUNNING)
		{
			pcb[current].state = STATE_RUNNABLE;
			pcb[current].timeCount = 0;
		}

		for (int i = 1; i < MAX_PCB_NUM; i++)
		{
			pid = (i + current) % MAX_PCB_NUM;
			if (pid != 0 && pcb[pid].state == STATE_RUNNABLE)
			{
				break;
			}
		}
		if (pcb[pid].state != STATE_RUNNABLE)
		{
			pid = 0;
		}
		current = pid;

		pcb[current].state = STATE_RUNNING;
		pcb[current].timeCount = 1;

		tmpStackTop = pcb[current].stackTop;
		pcb[current].stackTop = pcb[current].prevStackTop;
		tss.esp0 = (uint32_t) & (pcb[current].stackTop);
		asm volatile("movl %0, %%esp" ::"m"(tmpStackTop)); // switch kernel stack
		asm volatile("popl %gs");
		asm volatile("popl %fs");
		asm volatile("popl %es");
		asm volatile("popl %ds");
		asm volatile("popal");
		asm volatile("addl $8, %esp");
		asm volatile("iret");
	}

	return;
}

void keyboardHandle(struct TrapFrame *tf)
{
	uint32_t keyCode = getKeyCode();
	if (keyCode == 0)
		return;
	putChar(getChar(keyCode));
	keyBuffer[bufferTail] = keyCode;
	bufferTail = (bufferTail + 1) % MAX_KEYBUFFER_SIZE;
	return;
}

void syscallWrite(struct TrapFrame *tf)
{
	switch (tf->ecx)
	{ // file descriptor
	case 0:
		syscallPrint(tf);
		break; // for STD_OUT
	default:
		break;
	}
}

void syscallPrint(struct TrapFrame *tf)
{
	int sel = tf->ds; //TODO segment selector for user data, need further modification
	char *str = (char *)tf->edx;
	int size = tf->ebx;
	int i = 0;
	int pos = 0;
	char character = 0;
	uint16_t data = 0;
	asm volatile("movw %0, %%es" ::"m"(sel));
	for (i = 0; i < size; i++)
	{
		asm volatile("movb %%es:(%1), %0"
					 : "=r"(character)
					 : "r"(str + i));
		if (character == '\n')
		{
			displayRow++;
			displayCol = 0;
			if (displayRow == 25)
			{
				displayRow = 24;
				displayCol = 0;
				scrollScreen();
			}
		}
		else
		{
			data = character | (0x0c << 8);
			pos = (80 * displayRow + displayCol) * 2;
			asm volatile("movw %0, (%1)" ::"r"(data), "r"(pos + 0xb8000));
			displayCol++;
			if (displayCol == 80)
			{
				displayRow++;
				displayCol = 0;
				if (displayRow == 25)
				{
					displayRow = 24;
					displayCol = 0;
					scrollScreen();
				}
			}
		}
	}

	updateCursor(displayRow, displayCol);
	//TODO take care of return value
}

void syscallFork(struct TrapFrame *tf)
{
	// TODO in lab3 done
	int pid,j;
	for (pid = 0; pid < MAX_PCB_NUM; pid++)
	{
		if (pcb[pid].state == STATE_DEAD)
		{
			break;
		}
	}
	if (pid != MAX_PCB_NUM)
	{
		enableInterrupt();
		for (j = 0; j < 0x100000; j++) {
			*(uint8_t *)(j + (pid+1)*0x100000) = *(uint8_t *)(j + (current+1)*0x100000);
			//asm volatile("int $0x20"); //XXX Testing irqTimer during syscall
		}
		/*XXX disable interrupt
		 */
		disableInterrupt();
		
		pcb[pid].stackTop = (uint32_t) & (pcb[pid].stackTop) - sizeof(struct TrapFrame);
		pcb[pid].state = STATE_RUNNABLE;
		pcb[pid].timeCount = pcb[current].timeCount;
		pcb[pid].sleepTime = pcb[current].sleepTime;
		pcb[pid].pid = pid;

		pcb[pid].regs.cs = USEL(2 * pid + 1);
		pcb[pid].regs.ds = USEL(2 * pid + 2);
		pcb[pid].regs.ss = USEL(2 * pid + 2);
		pcb[pid].regs.gs = pcb[current].regs.gs;
		pcb[pid].regs.fs = pcb[current].regs.fs;
		pcb[pid].regs.es = pcb[current].regs.es;
		pcb[pid].regs.edi = pcb[current].regs.edi;
		pcb[pid].regs.esi = pcb[current].regs.esi;
		pcb[pid].regs.ebp = pcb[current].regs.ebp;
		pcb[pid].regs.xxx = pcb[current].regs.xxx;
		pcb[pid].regs.ebx = pcb[current].regs.ebx;
		pcb[pid].regs.edx = pcb[current].regs.edx;
		pcb[pid].regs.ecx = pcb[current].regs.ecx;
		pcb[pid].regs.eax = pcb[current].regs.eax;
		pcb[pid].regs.irq = pcb[current].regs.irq;
		pcb[pid].regs.error = pcb[current].regs.error;
		pcb[pid].regs.eip = pcb[current].regs.eip;
		pcb[pid].regs.eflags = pcb[current].regs.eflags;
		pcb[pid].regs.esp = pcb[current].regs.esp;

		// reture value
		pcb[pid].regs.eax = 0;
		pcb[current].regs.eax = pid;
	}
	else
	{
		pcb[current].regs.eax = -1;
	}
	return;
}

void syscallExec(struct TrapFrame *tf)
{
	// TODO in lab3 done
	// hint: ret = loadElf(tmp, (current + 1) * 0x100000, &entry);
	char filename[100];
	uint32_t entry;

	int sel = tf->ds;
	char *str = (char *)tf->ecx;
	// int size = tf->ebx;
	char character = 0;
	asm volatile("movw %0, %%es" ::"m"(sel));
	for (int i = 0; i < 100; i++)
	{
		asm volatile("movb %%es:(%1), %0"
					 : "=r"(character)
					 : "r"(str + i));
		filename[i] = character;
		if (character == 0)
			break;
	}
	int res = loadElf(filename, (current + 1) * 0x100000, &entry);

	if (res == 0)
	{
		pcb[current].regs.eip = entry;
	}
	else
	{
		pcb[current].regs.eax = -1;
	}

	return;
}

void syscallSleep(struct TrapFrame *tf)
{
	// TODO in lab3 done
	if (tf->ecx > 0)
	{
		pcb[current].state = STATE_BLOCKED;
		pcb[current].sleepTime = tf->ecx;
		asm volatile("int $0x20");
	}
	return;
}

void syscallExit(struct TrapFrame *tf)
{
	// TODO in lab3 done
	pcb[current].state = STATE_DEAD;
	asm volatile("int $0x20");
	return;
}

void GProtectFaultHandle(struct TrapFrame *tf)
{
	assert(0);
	return;
}
