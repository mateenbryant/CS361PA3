/*
    Assignment  :   PA3 - Threads
    Data        :   April 10, 2023

    Authors     :   Kiavash Seraj & Mateen Faieq

    File Name   :   tours.c
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
volatile int seats = 0;   // Remaining seats on the bus. Initially = bus‐capacity
volatile int shopping = 0; // # of tourists going shopping
volatile int on_bus_count = 0;
sem_t semaphores[MAXTOURISTS];
sem_t *arrival_sem;
sem_t *default_sem;
sem_t *threads_closed;
sem_t *shop_sem;
sem_t *on_bus_sem;
sem_t *off_bus_sem;
sem_t *sing_sem;

typedef struct
{
    int tourist_number;
    int trips_to_go;
} TouristData_t;

typedef struct
{
    int seats;
    int tourist_num;
} DriverData_t;

void *tourist_thread(void *arg)
{
    TouristData_t *args = (TouristData_t *)arg;
    int tourist_number = args->tourist_number;
    int tripsToGo = args->trips_to_go;
    bool seated = false;

    Sem_wait(default_sem); // Lock access to stdout for printing
    // Sem_wait(semaphores + tourist_number);
    printf("Tourist %d: Arrived to Harrisonburg.\n", tourist_number);
    Sem_post(default_sem); // Release access to stdout for printing

    if (tripsToGo > 0)
    {

        while (tripsToGo > 0)
        {

            int time = (random() % (4000 - 1500 + 1)) + 1500;

            printf("Tourist %d: Round # %d. Going to shop for %d milliseconds\n", tourist_number, args->trips_to_go - tripsToGo + 1, time);

            Sem_wait(shop_sem);
                shopping++;
            Sem_post(shop_sem);
            /*0 post*/Sem_post(semaphores + tourist_number); // Tell driver one of the tourists is done


            /*3 wait*/Sem_wait(semaphores);                  // driver is napping
            // Shopping
            usleep(time);

            /*2 wait*/Sem_wait(semaphores + tourist_number); // lock tourists sem
            Sem_wait(shop_sem);

            
            printf("Tourist %d: Back from shopping, waiting for a seat on the bus\n", tourist_number);
            if (seats > 0)
            {   
                printf("Tourist %d: I got on board the bus.. Belt CLICK\n", tourist_number);
                Sem_wait(on_bus_sem);
                on_bus_count++;
                Sem_post(on_bus_sem);
                shopping--;
            }

            Sem_post(shop_sem);
            Sem_post(semaphores); // Back on the bus
            Sem_post(semaphores + tourist_number);

            Sem_wait(semaphores); 
            Sem_wait(semaphores + tourist_number); // Driver is welcoming passengers

            Sem_wait(sing_sem);
            printf("Tourist %d: The Wheels on the Bus go Round and Round!\n", tourist_number);
            Sem_post(sing_sem);

            // printf("5\n");
            Sem_post(semaphores + tourist_number); // Tour is over for tourist
            // printf("6\n");


            Sem_wait(semaphores + tourist_number);
            // Wait for the bus tour to finish
            Sem_wait(off_bus_sem);
            printf("Tourist %d: Got off the bus\n", tourist_number);

            Sem_wait(on_bus_sem);
            on_bus_count--;

            if (on_bus_count == 0) {
                printf("Tourist %d: I am Last to get off. Alerting Driver Bus is now empty\n", tourist_number);
            }


            Sem_post(on_bus_sem);
            Sem_post(off_bus_sem);

            tripsToGo--;
        }
    }
    else
    {

        Sem_post(default_sem);                 // Release access to stdout for printing
        Sem_post(semaphores + tourist_number); // Tell driver one of the tourists is done
    }

    printf("Tourist %d: Packing my luggage\n", tourist_number);
    // wait for driver to say business is closed
    Sem_wait(default_sem);
    printf("Tourist %d: Leaving Town\n", tourist_number);
    Sem_post(default_sem);

    free(args);
    pthread_exit(NULL);
}










void *driver_thread(void *arg)
{
    DriverData_t *args = (DriverData_t *)arg;
    int tourists = args->tourist_num;
    int seats = args->seats;

    printf("\nIndy\t: Started My Day Waiting for ALL %d tourists to arrive in town\n\n", tourists);
    Sem_post(default_sem);

    // Wait for all tourists to arrive in town #1
    for (int i = 1; i < tourists + 1; i++)
    {
        /*1 wait*/Sem_wait(semaphores + i);
        // Sem_post(semaphores + i);
    }

    while (true)
    {
        if (tickets == 0)
        {
            printf("\nIndy\t: Business is now closed for today. I did %d tours today\n", trips_made);

            pthread_exit(NULL);
        }
        else
        {

            // printf("%d\n", tickets);
            printf("\nIndy\t: Starting Tour #%d. Announcing %d vacant seats\n", trips_made + 1, seats);
            printf("Indy\t: Taking a nap till tourists get on board\n\n");

            for (int i = 1; i < tourists + 1; i++)
            { // post after both have boarded

                /*1 post*/Sem_post(semaphores + i);
            }

            for (int i = 0; i < tourists; i++)
            {
                /*3wait*/Sem_wait(semaphores); 
            }

            

            // Wait for all tourists to get on board or for the bus to fill up
            printf("199\n");
            for (int i = 1; i < tourists + 1; i++)
            { // lock after both have boarded
                Sem_wait(semaphores + i);
            }
            // printf("200\n");
            for (int i = 0; i < tourists; i++)
            { // post after both have boarded

                /*1 post*/Sem_post(semaphores);
            }

            if (seats > tourists)
            {
                printf("\nIndy\t: Welcome on board dear %d passenger(s)! Please, fasten your seatbelts\n", tourists);
            }
            else
            {
                printf("\nIndy\t: Welcome on board dear %d passenger(s)! Please, fasten your seatbelts\n", seats);
            }
            printf("Indy\t: Thank you all for fastening your seatbelts.\n");
            // duration = random number between 1500 to 4000
            int duration = (random() % (4000 - 1500 + 1)) + 1500;
            printf("Indy\t: Tour will last for %d milliseconds\n", duration);
            printf("Indy\t: Bus will now move. We All must Sing!\n");
            printf("Indy\t: Bus! Bus! On the street! Who is the fastest driver to beat?\n");
            // switch to tourists to begin singing.
            
            // once all tourists are done singing. the tour will finish
            for (int i = 1; i < tourists + 1; i++)
            {                             // lock after both have boarded
                Sem_post(semaphores + i); // post for tourists to sing
            }

            for (int i = 1; i < tourists + 1; i++)
            { // wait for finish singing
                Sem_wait(semaphores + i);
            }

            printf("Indy\t: Tour %d is over. You may now get off the bus.\n\n", trips_made + 1);
            for (int i = 1; i < tourists + 1; i++)
            {                             
                Sem_post(semaphores + i); // post for tourists to get off
            }

            
        }
        
    }
    free(args);
    pthread_exit(NULL);
    // printf("\nIndy\t: Starting Tour #%d. Announcing %d vacant seat\n", trips_made + 1, /*seats remaining*/);
}

int main(int argc, char *argv[])
{
    // Suggested shared values
    // semaphore AvailSeats , arrived , canStart , seatBelt , busMoved , songFinished , groupGotOff , tourFinished ;
    int onBoard, inTown; // Number of tourists: on board the bus , inTown
    int shopping;        // count of tourists: still on the street, vs. those who started a tour on board the bus

    if (argc == 2) {
        Sem_unlink("/default_semaphoreSerjFaiq");
        Sem_unlink("/arrival_semaphoreSerjFaiq");
        Sem_unlink("/shop_semaphoreSerjFaiq");
        Sem_unlink("/sing_semaphoreSerjFaiq");
        Sem_unlink("/off_bus_semaphoreSerjFaiq");
        Sem_unlink("/on_bus_semaphoreSerjFaiq");

        return 1;
    }

    if (argc != 4)
    {
        printf("Usage: ./tours S N T\n");
        exit(1);
    }

    // S = number of seats on the bus
    // N = number of tourists
    // T = number of trips per tourist
    int s = atoi(argv[1]);
    int n = atoi(argv[2]);
    if (n > MAXTOURISTS)
    {
        printf("Error: n must be less than or equal to %d\n", MAXTOURISTS);
        exit(1);
    }
    int t = atoi(argv[3]);

    tickets = n * t;
    seats = s;

    pthread_t tid;

    printf("*********************************************************************\n");
    printf("OPERATOR: Bus has %d seats. We expect %d tourists today. Each will make %d tours\n", s, n, t);
    printf("********************************************************************\n");

    

    // Create semaphores
    arrival_sem = Sem_open("/arrival_semaphoreSerjFaiq", O_CREAT | O_EXCL, 0666, 0);
    default_sem = Sem_open("/default_semaphoreSerjFaiq", O_CREAT | O_EXCL, 0666, 0);
    shop_sem = Sem_open("/shop_semaphoreSerjFaiq", O_CREAT | O_EXCL, 0666, 1);
    on_bus_sem = Sem_open("/on_bus_semaphoreSerjFaiq", O_CREAT | O_EXCL, 0666, 1);
    off_bus_sem = Sem_open("/off_bus_semaphoreSerjFaiq", O_CREAT | O_EXCL, 0666, 1);
    sing_sem = Sem_open("/sing_semaphoreSerjFaiq", O_CREAT | O_EXCL, 0666, 1);
    // printf("hello\n");

    pthread_t threads[n + 1];
    // Create threads

    for (int i = 0; i < n + 1; i++)
    {
        
        if (i == 0) {
            Sem_init(semaphores + i, 0, 2);
        } else {
            Sem_init(semaphores + i, 0, 0);
        }

        if (i == 0)
        {
            // Indiana thread
            DriverData_t *driver_data = (DriverData_t *)malloc(sizeof(DriverData_t));
            driver_data->seats = s;
            driver_data->tourist_num = n;
            pthread_create(&threads[i], NULL, driver_thread, (void *)driver_data);
        }
        else
        {
            // Tourist threads
            TouristData_t *tourist_data = (TouristData_t *)malloc(sizeof(TouristData_t));
            tourist_data->tourist_number = i;
            tourist_data->trips_to_go = t;
            pthread_create(&threads[i], NULL, tourist_thread, (void *)tourist_data);
        }
    }
    for (int i = 0; i < n + 1; i++)
    {
        pthread_join(threads[i], NULL);
    }

    Sem_close(arrival_sem);
    Sem_close(default_sem);
    Sem_close(shop_sem);
    Sem_close(on_bus_sem);
    Sem_close(off_bus_sem);
    Sem_close(sing_sem);

    Sem_unlink("/arrival_semaphoreSerjFaiq");
    Sem_unlink("/default_semaphoreSerjFaiq");
    Sem_unlink("/shop_semaphoreSerjFaiq");
    Sem_unlink("/on_bus_semaphoreSerjFaiq");
    Sem_unlink("/off_bus_semaphoreSerjFaiq");
    Sem_unlink("/sing_semaphoreSerjFaiq");

    printf("\nOPERATOR Terminated\n");
}
