#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>

pthread_t *Students;
pthread_t TA;

int ChairsCount = 0;
int CurrentIndex = 0;
#define WR 3

sem_t TA_sleep;
sem_t Chair_available;
sem_t NextStudent;
pthread_mutex_t chair_mutex;

void* TA_Activity(void* args);
void* Student_Activity(void* threadID);

int main( int argc, char* argv[])
{
    int number_of_students;
    

    pthread_mutex_init(&chair_mutex, NULL);
    sem_init(&TA_sleep, 0, 0);
    sem_init(&Chair_available, 0, WR); //3 chairs available initially
    sem_init(&NextStudent, 0, 0);

   if(argc<2)
   {
       printf("Number of Students not specified. Using default (5) students. \n");
       number_of_students = 5;
   }
   else{
       number_of_students = atoi(argv[1]);
       printf("Number of Studnets specified. Creating %d threads. \n", number_of_students);
        
   }

   Students = (pthread_t*) malloc(sizeof(pthread_t)*number_of_students);


    pthread_create(&TA, NULL, TA_Activity, NULL);

    for(int i =0; i <number_of_students; i++) {
        int* id = (int*)malloc(sizeof(int));
        *id = i;
        pthread_create(&Students[i], NULL, Student_Activity, (void*)id);
        usleep(rand() % 500000);
    }

    for (int i = 0; i < number_of_students; i++) {
        pthread_join(Students[i], NULL);
    }

    printf("All students have been helped. TA is done for the day.\n");

    pthread_cancel(TA);
    pthread_join(TA,NULL);

    free(Students);
    pthread_mutex_destroy(&chair_mutex);
    sem_destroy(&TA_sleep);
    sem_destroy(&Chair_available);
    sem_destroy(&NextStudent);

   
    return 0;
}



void* TA_Activity(void* args)
{
    while(1){//if chairs are empty, break the loop.
        printf("TA is sleeping.\n");
        sem_wait(&TA_sleep); //TA is currently sleeping.
        
        pthread_mutex_lock(&chair_mutex);
        if(ChairsCount > 0){//TA gets next student on chair.
            
            ChairsCount--;
            printf("TA is helping a student.\n");
            pthread_mutex_unlock(&chair_mutex);

            sleep(1);
            printf("TA finished helping a Student.\n");

            sem_post(&NextStudent);
        }
        else{
            pthread_mutex_unlock(&chair_mutex);
        }
    }
    pthread_exit(NULL);

}

void* Student_Activity(void* threadID)
{
    int id = *((int *)threadID);
    free(threadID);

    //For sitting in Waiting Room 
    while(1) {  // Loop to retry if no seat available
        // Sit in Waiting Room
        printf("Student %d needs help.\n", id);

        pthread_mutex_lock(&chair_mutex);
        if(ChairsCount < WR){
            int chair_index = (CurrentIndex + ChairsCount) % WR;
            ChairsCount++;
            printf("Student %d is waiting on chair %d.\n", id, chair_index);
            pthread_mutex_unlock(&chair_mutex);

            sem_post(&TA_sleep);
            sem_wait(&NextStudent);
            printf("Student %d is getting help from the TA.\n", id);

            printf("Student %d leaves after getting help.\n", id);
            
            break;

        }
        else{
            printf("Student %d will return later as no chairs are available.\n", id);
            pthread_mutex_unlock(&chair_mutex);
            usleep(rand() % 1000000);
        }
    }
    pthread_exit(NULL);
}

