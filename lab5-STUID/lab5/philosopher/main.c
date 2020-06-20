#include "lib.h"
#include "types.h"

void philosopher(int i, sem_t forks[])
{
	int id = getpid();
	while (1)
	{
		printf("Philosopher %d: think\n", id);
		// sem_wait(&forks[i]);
		// sem_wait(&forks[(i+1)%5]);
		if (i % 2 == 0)
		{
			sem_wait(&forks[i]);
			sleep(128);
			sem_wait(&forks[(i+1)%5]);
			sleep(128);
		}
		else
		{
			sem_wait(&forks[(i+1)%5]);
			sleep(128);
			sem_wait(&forks[i]);
			sleep(128);
		}
		printf("Philosopher %d: eat\n", id);
		sleep(128);
		sem_post(&forks[i]);
		sleep(128);
		sem_post(&forks[(i+1)%5]);
	}
}

int main(void)
{
	// TODO in lab4 done
	printf("philosopher\n");
	int i = 0;
	sem_t forks[5];
	for (int i = 0; i < 5; i++)
	{
		sem_init(&forks[i], 1);
	}

	for (; i < 4; i++)
	{
		if (fork() == 0)
		{
			philosopher(i, forks);
			exit();
		}
	}
	philosopher(i, forks);
	return 0;
}
