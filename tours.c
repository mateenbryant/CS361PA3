/*
    PA3 - Threads
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include "wrappers.h"
#include <stdbool.h>


#define MAXTOURISTS 50

volatile int trips_made = 0;
sem_t *arrival_semaphore;

sem_t tweet_sem;

void tweet(const char* msg) {
    sem_wait(&tweet_sem);
    printf("%s\n", msg);
    sem_post(&tweet_sem);
}

void *tourist_thread(int tripsToGo) {

    bool seated = false;

    // tweet(“Tourist < j >: Arrived” );
    // Notify the Driver ;

    tweet("Tourist %d: Arrived", pthread_self());

    printf("Tourist %ld: Arrived to Harrisonburg.\n", pthread_self());
    if (tripsToGo - trips_made == 0) {
        printf("Tourist %ld: Packing my luggage\n", pthread_self());
        //wait for driver to say business is closed
        printf("Tourist %ld: Leaving Town\n", pthread_self());
    }
    int time = (random() % (4000 - 1500 + 1)) + 1500;
    printf("Tourist %ld: Round # %d. Going to shop for %d milliseconds\n", pthread_self(), trips_made + 1 ,time);
    

}

void *driver_thread(int tourists) {

    printf("\nIndy\t: Started My Day Waiting for ALL %d tourists to arrive in town\n\n", tourists);

    Sem_wait(arrival_semaphore);

    printf("\nIndy\t: Starting Tour #%d. Announcing %d vacant seat\n", trips_made + 1, /*seats remaining*/);
    printf("Indy\t: Taking a nap till touristst get on board\n");

    printf("Indy : Business is now closed for today. I did %d tours today", trips_made);
    
}

int main(int argc, char *argv[]) {
    // Suggested shared values
    // semaphore AvailSeats , arrived , canStart , seatBelt , busMoved , songFinished , groupGotOff , tourFinished ;
    int onBoard , inTown ; // Number of tourists: on board the bus , inTown
    int shopping ; // count of tourists: still on the street, vs. those who started a tour on board the bus
    int tickets ; // Remaining tickets to be sold for today. Initially = numTourists x trips‐per‐tourist

    
    if (argc != 4) {
        printf("Usage: ./tours S N T\n");
        exit(1);
    }

    // S = number of seats on the bus
    // N = number of tourists
    // T = number of trips per tourist
    int s = atoi(argv[1]);
    int n = atoi(argv[2]);
    if (n > MAXTOURISTS) {
        printf("Error: n must be less than or equal to %d\n", MAXTOURISTS);
        exit(1);
    }
    int t = atoi(argv[3]);

    tickets = n * t;

    pthread_t   tid;
    
    printf("*********************************************************************\n");
    printf("OPERATOR: Bus has %d seats. We expect %d tourists today. Each will make %d tours\n", s, n, t);
    printf("********************************************************************\n");


    //Create semaphores

    // arrival_semaphore = Sem_open("arrival_semaphoreSerjFaiq", O_CREAT | O_EXCL, 0666, s);

    // Create threads
    for(int i = 0; i < n + 1; i++) 
    {
        if (i = 0) {
            // Indiana thread
            pthread_create(&tid, NULL, driver_thread, (void *) i );
        } else {
            // Tourist threads
            pthread_create(&tid, NULL, tourist_thread, (void *) i );
        }
    }

    
    printf("OPERATOR Terminated\n");
}


