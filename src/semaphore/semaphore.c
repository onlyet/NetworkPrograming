#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

static sem_t sem_one;
static sem_t sem_two;
static int num;

void* read(void *arg)
{
	int i;
	for (i = 0; i < 5; ++i) {
		sem_wait(&sem_two);
		fputs("input a num:\t", stdout);
		scanf("%d", &num);
		sem_post(&sem_one);
	}
}

void* accu(void *arg)
{
	int i, sum = 0;
	for (i = 0; i < 5; ++i) {
		sem_wait(&sem_one);
		sum += num;
		sem_post(&sem_two);
	}
	printf("sum is %d\n", sum);
	return NULL;
}

int main()
{
	pthread_t pid1, pid2;
	sem_init(&sem_one, 0, 0);
	sem_init(&sem_two, 0, 1);
	pthread_create(&pid1, NULL, read, NULL);
	pthread_create(&pid2, NULL, accu, NULL);

	pthread_join(pid1, NULL);
	pthread_join(pid2, NULL);
	sem_destroy(&sem_one);
	sem_destroy(&sem_two);
	return 0;
}