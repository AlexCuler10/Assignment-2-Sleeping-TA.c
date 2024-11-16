#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>

pthread_t *Students;
pthread_t TA;

int ChairsCount = 0;
int CurrentIndex = 0;


sem_t TA_sleep;
sem_t Chair_available;
sem_t TA_available;
sem_t NextStudent;
pthread_mutex_t chair_mutex;

void* TA_Activity(void* args);
void* Student_Activity(void* threadID);

int main( int argc, char* argv[])
{
    int number_of_students;
    int id;
    srand(time(NULL));

    pthread_mutex_init(&chair_mutex, NULL);
    sem_init(&TA_sleep, 0, 0);
    sem_init(&Chair_available, 0, 3); //3 chairs available initially
    sem_init(&TA_available, 0,0);
    sem_init(&NextStudent, 0, 0);

   if(argc<2)
   {
       printf("Number of Students not specified. Using default (5) students. \n");
       number_of_students = 5;
   }
   else{
       printf("Number of Studnets specified. Creating %d threads. \n", number_of_students);
        number_of_students = atoi(argv[1]);
   }

   Students = (pthread_t*) malloc(sizeof(pthread_t)*number_of_students);


    pthread_create(&TA, NULL, TA_Activity, NULL);

    for(int i =0; i <number_of_students; i++) {
        int* id = (int*)malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&Students[i], NULL, Student_Activity, (void*)id);
        usleep(rand() % 50000);
    }

    for (int i = 0; i < number_of_students; i++) {
        pthread_join(Students[i], NULL);
    }

    pthread_cancel(TA);
    pthread_join(TA,NULL);

    free(Students);
    pthread_mutex_destroy(&chair_mutex);
    sem_destroy(TA_sleep);
    sem_destroy(&Chair_available);
    sem_destroy(&TA_available);
    sem_destroy(&NextStudent);

   
    return 0;
}



void* TA_Activity(void* args)
{
    while(1){//if chairs are empty, break the loop.
        printf("TA is sleeping.\n");
        sem_wait(&TA_sleep); //TA is currently sleeping.
        sem_post(&TA_available);
        if(ChairsCount > 0){//TA gets next student on chair.
            pthread_mutex_lock(&chair_mutex);
            ChairsCount--;
            pthread_mutex_unlock(&chair_mutex);
            int sleep_time = rand() % 6 + 5;
            sleep(sleep_time);//Student is being helped by TA
            sem_post(&NextStudent);
            printf("TA is done helping student. Student will now leave and not return. \n");
        }
    }
    pthread_exit(NULL);

}

void* Student_Activity(void* threadID)
{
    int id =((int *)threadID);
    free(threadID);

    //For sitting in Waiting Room 
    pthread_mutex_lock(&chair_mutex);// lock
    if(ChairsCount < 3) {
        ChairsCount++; //takes the chair
        printf("Student %d takes a chair.\n", id);
        pthread_mutex_unlock(&chair_mutex);
    }
    else{
        printf("Student %d couldn't find a seat and has left. Student will return later.\n", id);
        pthread_mutex_unlock(&chair_mutex);

        sem_wait(&Chair_available);
        pthread_mutex_lock(&chair_mutex);
        ChairsCount++; //takes the chair
        printf("Student %d takes a chair: %d.\n", id, ChairsCount);
        pthread_mutex_unlock(&chair_mutex);
    }

    //For getting help from TA
    printf("Student %d is waiting in chair.\n", id);
    sem_post(&TA_sleep);
    sem_wait(&TA_available);
    printf("Student %d wakes up TA and heads into office.\n", id);
    sem_wait(&NextStudent);
    sem_post(&Chair_available);

    pthread_exit(NULL);
}

