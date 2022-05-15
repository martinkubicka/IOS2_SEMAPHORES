/**
 * @file proj2.c
 * @author Martin Kubička (xkubic45), FIT VUT Brno
 * @brief Simulation of creating molecules H20
 * @date 2022-04-24
 */

#include<stdio.h>
#include<stdlib.h>
#include<ctype.h>
#include<string.h>
#include<unistd.h>
#include<semaphore.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/mman.h>
#include<time.h>
#include <fcntl.h>

/* VARIABLES */
long long NO;
long long NH;
long long TI;
long long TB;

//shared memory variables
int A = 0;
int noM = 1;
int oxygens = 0;
int hydrogens = 0;
int count = 0;
int h_created = 0;

int *ptr_A = 0;
int *ptr_noM = 0;
int *ptr_oxygens = 0;
int *ptr_hydrogens = 0;
int *ptr_count = 0;
int *ptr_h_created = 0;

//semaphore variables
sem_t *s_writing = NULL;
sem_t *s_signal = NULL;
sem_t *s_oxygen = NULL;
sem_t *s_hydrogen = NULL;
sem_t *s_mutex =  NULL;
sem_t *s_barrier = NULL;
sem_t *s_h_signal = NULL;

FILE *file;

/* FUNCTIONS */
/**
 * @brief initializes shared memory and semaphores
 */
void init() {
    //initialiting shared memory
    A = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
    if (A == -1) {
        fprintf(stderr, "Error in creating shared memory!\n");
        exit(1);
    }
    ptr_A = (int *)shmat(A, NULL, 0);
    if (ptr_A == NULL) {
        fprintf(stderr, "Error in creating shared memory!\n");
        exit(1);
    }
    *ptr_A = 0;

    noM = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
    if (noM == -1) {
        fprintf(stderr, "Error in creating shared memory!\n");
        exit(1);
    }
    ptr_noM = (int *)shmat(noM, NULL, 0);
    if (ptr_noM== NULL) {
        fprintf(stderr, "Error in creating shared memory!\n");
        exit(1);
    }
    *ptr_noM = 1;

    oxygens = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
    if (oxygens == -1) {
        fprintf(stderr, "Error in creating shared memory!\n");
        exit(1);
    }
    ptr_oxygens = (int *)shmat(oxygens, NULL, 0);
    if (ptr_oxygens == NULL) {
        fprintf(stderr, "Error in creating shared memory!\n");
        exit(1);
    }
    *ptr_oxygens = 0;

    hydrogens = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
    if (hydrogens == -1) {
        fprintf(stderr, "Error in creating shared memory!\n");
        exit(1);
    }
    ptr_hydrogens = (int *)shmat(hydrogens, NULL, 0);
    if (ptr_hydrogens == NULL) {
        fprintf(stderr, "Error in creating shared memory!\n");
        exit(1);
    }
    *ptr_hydrogens = 0;

    count = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
    if (count == -1) {
        fprintf(stderr, "Error in creating shared memory!\n");
        exit(1);
    }
    ptr_count = (int *)shmat(count, NULL, 0);
    if (ptr_count == NULL) {
        fprintf(stderr, "Error in creating shared memory!\n");
        exit(1);
    }
    *ptr_count = 0;

    h_created = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
    if (h_created == -1) {
        fprintf(stderr, "Error in creating shared memory!\n");
        exit(1);
    }
    ptr_h_created = (int *)shmat(h_created, NULL, 0);
    if (ptr_h_created == NULL) {
        fprintf(stderr, "Error in creating shared memory!\n");
        exit(1);
    }
    *ptr_h_created = 0;

    //initializing semaphores
    s_writing = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    if (s_writing == MAP_FAILED) {
            fprintf(stderr, "Error in creating semaphore!\n");
            exit(1);
    }

    s_signal = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    if (s_signal == MAP_FAILED) {
            fprintf(stderr, "Error in creating semaphore!\n");
            exit(1);
    }

    s_oxygen = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    if (s_oxygen == MAP_FAILED) {
            fprintf(stderr, "Error in creating semaphore!\n");
            exit(1);
    }

    s_hydrogen = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    if (s_hydrogen == MAP_FAILED) {
            fprintf(stderr, "Error in creating semaphore!\n");
            exit(1);
    }

    s_mutex = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    if (s_mutex == MAP_FAILED) {
            fprintf(stderr, "Error in creating semaphore!\n");
            exit(1);
    }

    s_barrier = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    if (s_barrier == MAP_FAILED) {
            fprintf(stderr, "Error in creating semaphore!\n");
            exit(1);
    }

    s_h_signal = mmap(NULL, sizeof(sem_t), PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, 0, 0);
    if (s_h_signal == MAP_FAILED) {
            fprintf(stderr, "Error in creating semaphore!\n");
            exit(1);
    }

    if (sem_init(s_writing, 1, 1) == -1 || sem_init(s_signal, 1, 0) == -1 || sem_init(s_oxygen, 1, 1) == -1 || sem_init(s_hydrogen, 1, 2) == -1 || sem_init(s_mutex, 1, 0) == -1 || sem_init(s_barrier, 1, 0) == -1 || sem_init(s_h_signal, 1, 0) == -1) {
        fprintf(stderr, "Error in creating semaphore!\n");
        exit(1);
    }
}

/**
 * @brief function check if all arguments are digits
 * 
 * @param argv list of arguments
 * @return 1 if all arguments are digits or 0 if not
 */
int check_isdigit(char *argv[]) {
    NO = atoll(argv[1]);
    NH = atoll(argv[2]);
    TI = atoll(argv[3]);
    TB = atoll(argv[4]);
    //going through all arguments except 0
    for (int i = 1; i < 5; i++) {
        //if arguments is empty
        if (!strcmp(argv[i], "")) {
            return 0;
        } else {
            while (*argv[i]) {
                if (!isdigit(*argv[i])) {
                    return 0;
                } else {
                    argv[i]++; //moving to next char of actual argument
                }
            }
        }
    }
    return 1;
}

/**
 * @brief function checking if variables belong to wanted interval
 */
void check_valid_input() {
    if (NO <= 0) {
        fprintf(stderr, "ERROR: Parameter NO must be positive number!\n");
        exit(1);
    }

    if (NH <= 0) {
        fprintf(stderr, "ERROR: Parameter NH must be positive number!\n");
        exit(1);
    }

    if (TI<0 || TI>1000) {
        fprintf(stderr, "ERROR: Paramter TI can from interval 0<=TI<=1000!\n");
        exit(1);
    }
    if (TB<0 || TB>1000) {
        fprintf(stderr, "ERROR: Paramter TB can from interval 0<=TB<=1000!\n");
        exit(1);
    }
}

/**
 * @brief function generating random number from interval 0 to param t
 * 
 * @param t maximum number that can be generated
 * @return returning random generated number
 */
int random_number_oh(int t) {
    srand(time(NULL)*getpid());
    return (rand()%(t+1));
}

/**
 * @brief destructor of shared memory and semaphores
 */
void dtor() {
    fclose(file);

    //freeing shared memory
    if (shmctl(A, IPC_RMID, NULL) == -1 || shmctl(noM, IPC_RMID, NULL) == -1 || shmctl(oxygens, IPC_RMID, NULL) == -1 || shmctl(hydrogens, IPC_RMID, NULL) == -1 || shmctl(count, IPC_RMID, NULL) == -1 || shmctl(h_created, IPC_RMID, NULL) == -1) {
        fprintf(stderr, "Error in freeing shared memory\n");
        exit(1);
    }
    
    if (shmdt(ptr_A) == -1 || shmdt(ptr_noM) == -1 || shmdt(ptr_oxygens) == -1 || shmdt(ptr_hydrogens) == -1 || shmdt(ptr_count) == -1 ||  shmdt(ptr_h_created) == -1) {
        fprintf(stderr, "Error in freeing shared memory\n");
        exit(1);
    }

    //freeing semaphores
    if (sem_destroy(s_writing) == -1 || sem_destroy(s_signal) == -1 || sem_destroy(s_oxygen) == -1 || sem_destroy(s_hydrogen) == -1 || sem_destroy(s_mutex) == -1 || sem_destroy(s_barrier) == -1 || sem_destroy(s_h_signal) == -1) {
        fprintf(stderr, "Error in destroying semaphore!\n");
        exit(1);
    }
}

/**
 * @brief function for creating oxygen
 * 
 * @param idO ID of oxygen 
 */
void oxygen_process(int idO) {
    //start creating oxygen
    sem_wait(s_writing);
        (*ptr_A)++;
        fprintf(file, "%d: O %d: started\n", *ptr_A, idO+1);
        fflush(file);
    sem_post(s_writing);
    
    //simulation of creating oxygen
    usleep(random_number_oh(TI)*1000);

    sem_wait(s_writing);
            //increating number of oxygens in queue
            (*ptr_oxygens)++;
            //if there are created all oxygens and hydrogens
            if (*ptr_oxygens == NO && *ptr_hydrogens == NH) {
                sem_post(s_writing);
                sem_post(s_mutex);
            //or wait for creating all oxygens and hydrogens
            } else {
                sem_post(s_writing);
                sem_wait(s_mutex);
                sem_post(s_mutex);
            }

    //add oxygen to queue
    sem_wait(s_writing);
        (*ptr_A)++;
        fprintf(file, "%d: O %d: going to queue\n", *ptr_A, idO+1);
        fflush(file);
    sem_post(s_writing);

    //wait until no molecule won't be creating
    sem_wait(s_oxygen);
        sem_wait(s_writing);
            //if there are no more hydrogens
            if (*ptr_hydrogens < 2) {
                (*ptr_A)++;
                fprintf(file, "%d: O %d: not enough H\n", *ptr_A, idO+1);
                fflush(file);
                
                sem_post(s_oxygen);

                sem_post(s_writing);
                exit(0);
            }
        sem_post(s_writing);

        //creating molecule
        sem_wait(s_writing);
            (*ptr_A)++;
            fprintf(file, "%d: O %d: creating molecule %d\n", *ptr_A, idO+1, *ptr_noM);
            fflush(file);
        sem_post(s_writing);

        //simulation of creating molecule
        usleep(random_number_oh(TB)*1000);

        //inform 2 hydrogens that oxygen was created
        sem_post(s_signal);
        sem_post(s_signal);
        
        //wait until 2 hydrogens from same molecule will be created
        sem_wait(s_h_signal);
        sem_wait(s_h_signal);
        
        //successfully created molecule
        sem_wait(s_writing);
            (*ptr_A)++;
            (*ptr_oxygens)--; //decreasing available oxygens
            fprintf(file, "%d: O %d: molecule %d created\n", *ptr_A, idO+1, *ptr_noM);
            fflush(file);
        sem_post(s_writing);

        sem_wait(s_writing);
            //if hydrogens in molecule created molecule
            //restarting values
            if (*ptr_count == 2) {
                *ptr_count = 0;
                (*ptr_noM)++;
                //posting semaphores to create another molecule
                sem_post(s_oxygen);
                sem_post(s_hydrogen);
                sem_post(s_hydrogen);
            } else {
                (*ptr_count)++;
            }
        sem_post(s_writing);
    exit(0);
}

/**
 * @brief function for creating hydrogen
 * 
 * @param idH ID of hydrogen
 */
void hydrogen_process(int idH) {
    //start creating hydrogen
    sem_wait(s_writing);
        (*ptr_A)++;
        fprintf(file, "%d: H %d: started\n", *ptr_A, idH+1);
        fflush(file);
    sem_post(s_writing);

    //simulate creating hydrogen
    usleep(random_number_oh(TI)*1000);

    sem_wait(s_writing);
            (*ptr_hydrogens)++; //increasing number of hydrogens in queue
            //wait until all hydrogens and oxygens will be created
            if (*ptr_oxygens == NO && *ptr_hydrogens == NH) {
                sem_post(s_writing);
                sem_post(s_mutex);
            } else {
                sem_post(s_writing);
                sem_wait(s_mutex);
                sem_post(s_mutex);
            }

    //add hydrogen to queue
    sem_wait(s_writing);
        (*ptr_A)++;
        fprintf(file, "%d: H %d: going to queue\n", *ptr_A, idH+1);
        fflush(file);
    sem_post(s_writing);

    
    //wait until no molecule won't be creating
    sem_wait(s_hydrogen);
        sem_wait(s_writing);
            //if there are not enough oxygens or hydrogens
            if (*ptr_hydrogens < 2 || *ptr_oxygens < 1) {
                (*ptr_A)++;
                fprintf(file, "%d: H %d: not enough O or H\n", *ptr_A, idH+1);
                fflush(file);

                sem_post(s_hydrogen);

                sem_post(s_writing);
                exit(0);
            }
        sem_post(s_writing);

        //creating molecule
        sem_wait(s_writing);
            (*ptr_A)++;
            fprintf(file, "%d: H %d: creating molecule %d\n", *ptr_A, idH+1, *ptr_noM);
            fflush(file);
            (*ptr_h_created)++;
        sem_post(s_writing);

        //send signal to oxygen that hydrogen was created
        sem_post(s_h_signal);
        //waiting for signal that oxygen was created
        sem_wait(s_signal);

        //wait until hydrogens will be created
        sem_wait(s_writing);
        if (*ptr_h_created >= 2) {
            sem_post(s_barrier);
            sem_post(s_writing);
        } else {
            sem_post(s_writing);
            sem_wait(s_barrier);
        }

        //molecule created
        sem_wait(s_writing);
            (*ptr_A)++;
            (*ptr_hydrogens)--; //decreasing available hydrogens
            (*ptr_h_created) = 0;
            fprintf(file, "%d: H %d: molecule %d created\n", *ptr_A, idH+1, *ptr_noM);
            fflush(file);
        sem_post(s_writing);

        //restaring values
        sem_wait(s_writing);
            //if hydrogens in molecule created molecule
            //restarting values
            if (*ptr_count == 2) {
                *ptr_count = 0;
                (*ptr_noM)++;
                //posting semaphores to create another molecule
                sem_post(s_oxygen);
                sem_post(s_hydrogen);
                sem_post(s_hydrogen);
            } else {
                (*ptr_count)++;
            }
        sem_post(s_writing);
    exit(0);
}

int main(int argc, char *argv[]) {
    //checking if there is right amout of arguments entered by user
    if (argc != 5) {
        fprintf(stderr, "ERROR: Not enough or too much arguments were entered!\n");
        exit(1);
    } else {
        //check if all arguments are digits
        if (!check_isdigit(argv)) {
            fprintf(stderr, "ERROR: Arguments can be only numbers!\n");
            exit(1);
        }
    }

    //check if arguments are from right inverval of numbers
    check_valid_input();

    //opening file
    file = fopen("proj2.out", "w");
    if (file == NULL) {
        fprintf(stderr, "ERROR: Can't open file proj2.out!\n");
        exit(1);
    }
    setbuf(file, NULL);

    //initialization of shared memory and semaphores
    init();

    pid_t p_oxygen;
    //creating oxygens
    for (int idO = 0; idO < NO; idO++) {
        p_oxygen = fork();
        if (p_oxygen == 0) { //child process
            oxygen_process(idO);
            exit(0);
        } else if (p_oxygen < 0) {
            dtor();
            fprintf(stderr, "Error in fork command!\n");
            exit(1);
        }
    }

    pid_t p_hydrogen;
    //creating hydrogens
    for (int idH = 0; idH < NH; idH++) {
        p_hydrogen = fork();
        if (p_hydrogen == 0) { //child process
            hydrogen_process(idH);
            exit(0);
        } else if (p_hydrogen < 0) {
            dtor();
            fprintf(stderr, "Error in fork command!\n");
            exit(1);
        }
    }

    //wait until all processes will be finished
    while (wait(NULL) > 0);
    
    //freeing shared memory and semaphores
    dtor();

    return 0;
}

/*** END OF proj2.c ***/
