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
volatile int tickets = 0; // Remaining tickets to be sold for today. Initially = numTourists x trips‐per‐tourist
sem_t *arrival_sem;
sem_t *default_sem;
sem_t *threads_closed;
sem_t *shop_sem;
sem_t *on_bus_sem;
sem_t *off_bus_sem;
sem_t *sing_sem;

typedef struct {
    int tourist_number;
    int trips_to_go;
} TouristData_t;

typedef struct {
    int seats;
    int tourist_num;
} DriverData_t;

void *tourist_thread(void *arg) {
    TouristData_t* args = (TouristData_t*) arg;
    int tourist_number = args->tourist_number;
    int tripsToGo = args->trips_to_go;
    bool seated = false;

    Sem_wait(default_sem); // Lock access to stdout for printing
    printf("Tourist %d: Arrived to Harrisonburg.\n", tourist_number);
    /////////////////////////////
    if (tripsToGo > 0) {
        while (tripsToGo > 0) {
        int time = (random() % (4000 - 1500 + 1)) + 1500;
        // Sem_wait(default_sem); // Lock access to stdout for printing
        printf("Tourist %d: Round # %d. Going to shop for %d milliseconds\n", tourist_number, args->trips_to_go - tripsToGo + 1, time);
        Sem_post(default_sem); // Release access to stdout for printing

        // Shopping
        usleep(time * 1000);
        Sem_wait(shop_sem);
        printf("Tourist %d: Back from shopping, waiting for a seat on the bus\n", tourist_number);
        Sem_post(shop_sem);
        Sem_post(arrival_sem);

        // Get on the bus
        Sem_wait(on_bus_sem);
        printf("Tourist %d: I got on board the bus.. Belt CLICK\n", tourist_number);
        Sem_post(on_bus_sem);

        // // Wait for the bus tour to finish
        // Sem_wait(off_bus_sem);
        // printf("Tourist %d: Got off the bus\n", tourist_number);
        // Sem_post(off_bus_sem);

        tripsToGo--;
        }
    } else {

        Sem_post(default_sem); // Release access to stdout for printing
        Sem_post(arrival_sem); // Signal that the tourist has arrived in town
    }
    /////////////////////////////

    if (tripsToGo == 0) {
        printf("Tourist %d: Packing my luggage\n", tourist_number);
        //wait for driver to say business is closed
        Sem_wait(default_sem);
        printf("Tourist %d: Leaving Town\n", tourist_number);
    } else {

    }
    int time = (random() % (4000 - 1500 + 1)) + 1500;
    
    free(args);
    pthread_exit(NULL);
}

void *driver_thread(void *arg) {
   DriverData_t* args = (DriverData_t*) arg;
    int tourists = args->tourist_num;
    int seats = args->seats;
    printf("\nIndy\t: Started My Day Waiting for ALL %d tourists to arrive in town\n\n", tourists);
    Sem_post(default_sem);

    // Wait for all tourists to arrive in town
    for (int i = 0; i < tourists; i++) {
        Sem_wait(arrival_sem);
    }


    Sem_wait(shop_sem);
    while (true) {
        if (tickets == 0) {
            printf("\nIndy\t: Business is now closed for today. I did %d tours today\n", trips_made);
            Sem_post(shop_sem);
            pthread_exit(NULL);
        } else {
            printf("Indy\t: Starting Tour #%d. Announcing %d vacant seats\n", trips_made + 1, seats);
            printf("Indy\t: Taking a nap till tourists get on board\n");
            Sem_post(shop_sem);
            Sem_wait(on_bus_sem);
        }
    }
    
    

    // printf("\nIndy\t: Starting Tour #%d. Announcing %d vacant seat\n", trips_made + 1, /*seats remaining*/);

    
}

int main(int argc, char *argv[]) {
    // Suggested shared values
    // semaphore AvailSeats , arrived , canStart , seatBelt , busMoved , songFinished , groupGotOff , tourFinished ;
    int onBoard , inTown ; // Number of tourists: on board the bus , inTown
    int shopping ; // count of tourists: still on the street, vs. those who started a tour on board the bus
    
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

    // Sem_unlink("/default_semaphoreSerjFaiq");
    // Sem_unlink("/arrival_semaphoreSerjFaiq");
    // Sem_unlink("/on_bus_semaphoreSerjFaiq");
    // Sem_unlink("/shop_semaphoreSerjFaiq");
    
    
    // return 1;

    //Create semaphores
    arrival_sem = Sem_open("/arrival_semaphoreSerjFaiq", O_CREAT | O_EXCL, 0666, 0);
    default_sem = Sem_open("/default_semaphoreSerjFaiq", O_CREAT | O_EXCL, 0666, 0);
    shop_sem = Sem_open("/shop_semaphoreSerjFaiq", O_CREAT | O_EXCL, 0666, 0);
    on_bus_sem = Sem_open("/on_bus_semaphoreSerjFaiq", O_CREAT | O_EXCL, 0666, -2);
    // off_bus_sem = Sem_open("/off_bus_semaphoreSerjFaiq", O_CREAT | O_EXCL, 0666, 0);
    // sing_sem = Sem_open("/sing_semaphore", O_CREAT | O_EXCL, 0666, 0);
    // printf("hello\n");


    pthread_t threads[n + 1];
    // Create threads
    for(int i = 0; i < n + 1; i++) 
    {
        
        if (i == 0) {
            // Indiana thread
            DriverData_t *driver_data = (DriverData_t *)malloc(sizeof(DriverData_t));
            driver_data->seats = s;
            driver_data->tourist_num = n;
            pthread_create(&threads[i], NULL, driver_thread, (void *) driver_data);
        } else {
            // Tourist threads
            TouristData_t *tourist_data = (TouristData_t *)malloc(sizeof(TouristData_t));
            tourist_data->tourist_number = i;
            tourist_data->trips_to_go = t;
            pthread_create(&threads[i], NULL, tourist_thread, (void *) tourist_data);
        }
    }
    for (int i = 0; i < n + 1; i++) {
        pthread_join(threads[i], NULL);
    }

    Sem_close(arrival_sem);
    Sem_close(default_sem);
    Sem_close(shop_sem);
    Sem_close(on_bus_sem);
    // Sem_close(off_bus_sem);
    // Sem_close(sing_sem);

    Sem_unlink("/arrival_semaphoreSerjFaiq");
    Sem_unlink("/default_semaphoreSerjFaiq");
    Sem_unlink("/shop_semaphoreSerjFaiq");
    Sem_unlink("/on_bus_semaphoreSerjFaiq");
    // Sem_unlink("/off_bus_semaphore");
    // Sem_unlink("/sing_semaphore");

    printf("\nOPERATOR Terminated\n");
}

