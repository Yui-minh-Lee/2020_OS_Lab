#include "lib.h"
#include "types.h"

void reader(int *Rcount, sem_t *writeblock, sem_t *mutex)
{
	int id = getpid();
	
	if ((*Rcount) == 0)
	{
		sem_wait(writeblock);
		sleep(128);
	}
	++(*Rcount);
	sleep(128);
	sem_post(mutex);
	sleep(128);
	printf("Reader %d: read, total %d reader\n", id, (*Rcount));
	sleep(128);
	sem_wait(mutex);
	sleep(128);
	--(*Rcount);
	if ((*Rcount) == 0)
	{
		sem_post(writeblock);
		sleep(128);
	}
	sem_post(mutex);
}

void writer(sem_t *writeblock)
{
	int id = getpid();
	sem_wait(writeblock);
	sleep(128);
	printf("Writer %d: write\n", id);
	sleep(128);
	sem_post(writeblock);
}

int main(void)
{
	// TODO in lab4
	printf("reader_writer\n");
	int Rcount = 0;
	sem_t writeblock;
	sem_t mutex;
	sem_init(&writeblock, 1);
	sem_init(&mutex, 1);
	for (int i = 0; i < 3; i++)
	{
		if (fork() == 0)
		{
			writer(&writeblock);
			exit();
		}
	}
	for (int i = 0; i < 2; i++)
	{
		if (fork() == 0)
		{
			reader(&Rcount, &writeblock, &mutex);
			exit();
		}
	}
	reader(&Rcount, &writeblock, &mutex);
	exit();
	return 0;
}
