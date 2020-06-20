#include "lib.h"
#include "types.h"


void producer(sem_t *empty, sem_t *full, sem_t *mutex)
{
	int id=getpid();
	
	for (int i=0;i<2;i++)
	{
		sem_wait(empty);
		sleep(128);
		sem_wait(mutex);
		sleep(128);
		printf("Producer %d: produce\n", id);
		sleep(128);
		sem_post(mutex);
		sleep(128);
		sem_post(full);
	}
	return;
}

void consumer(sem_t *empty, sem_t *full, sem_t *mutex)
{
	for (int i=0;i<10;i++)
	{
		sem_wait(full);
		sleep(128);
		sem_wait(mutex);
		sleep(128);
		printf("Consumer : consume\n");
		sleep(128);
		sem_post(mutex);
		sleep(128);
		sem_post(empty);
	}
	return;
}

int main(void)
{
	// TODO in lab4 done
	printf("bounded_buffer\n");
	sem_t empty;
	sem_t full;
	sem_t mutex;
	sem_init(&empty, 5);
	sem_init(&full, 0);
	sem_init(&mutex, 1);
	for (int i = 0; i < 4; i++)
	{
		if (fork() == 0)
		{
			producer(&empty, &full, &mutex);
			exit();
		}
	}
	consumer(&empty, &full, &mutex);

	sem_destroy(&empty);
	sem_destroy(&full);
	sem_destroy(&mutex);
	return 0;
}
