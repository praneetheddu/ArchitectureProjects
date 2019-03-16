/*
 * student.c
 * Multithreaded OS Simulation for ECE 3056
 *
 * This file contains the CPU scheduler for the simulation.
 */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "os-sim.h"

/** Function prototypes **/
extern void idle(unsigned int cpu_id);
extern void preempt(unsigned int cpu_id);
extern void yield(unsigned int cpu_id);
extern void terminate(unsigned int cpu_id);
extern void wake_up(pcb_t *process);


/*
 * current[] is an array of pointers to the currently running processes.
 * There is one array element corresponding to each CPU in the simulation.
 *
 * current[] should be updated by schedule() each time a process is scheduled
 * on a CPU.  Since the current[] array is accessed by multiple threads, you
 * will need to use a mutex to protect it.  current_mutex has been provided
 * for your use.
 */
static pcb_t **current;
static pthread_mutex_t current_mutex;
static pthread_mutex_t availThread;
static pthread_cond_t nonIdleThread;

pcb_t* begin;
pcb_t* end;


//Args parsed during runtime
int timePart; // time partition
int rr = 0; //round robin identifier
int fifo = 0; //fifo identifier
int LRTF = 0; //LRTF identifier
unsigned int cpu_mode; //

/*
 * schedule() is your CPU scheduler.  It should perform the following tasks:
 *
 *   1. Select and remove a runnable process from your ready queue which 
 *	you will have to implement with a linked list or something of the sort.
 *
 *   2. Set the process state to RUNNING
 *
 *   3. Call context_switch(), to tell the simulator which process to execute
 *      next on the CPU.  If no process is runnable, call context_switch()
 *      with a pointer to NULL to select the idle process.
 *	The current array (see above) is how you access the currently running process indexed by the cpu id. 
 *	See above for full description.
 *	context_switch() is prototyped in os-sim.h. Look there for more information 
 *	about it and its parameters.
 */
static void schedule(unsigned int cpu_id)
{
    pcb_t* tempThread;
    if (begin != NULL) {
        pthread_mutex_lock(&availThread);
        tempThread = begin;
        tempThread -> state = PROCESS_RUNNING; //set the state to running
        if (begin == end) {
            begin = NULL;
            end = NULL;
        } else {
            begin = tempThread->next;
        }
    pthread_mutex_lock(&current_mutex);
    current[cpu_id] = tempThread;
    pthread_mutex_unlock(&current_mutex);
    pthread_mutex_unlock(&availThread);
    context_switch(cpu_id, tempThread, timePart);
    } else {
        context_switch(cpu_id, NULL, timePart);
    }

}


/*
 * idle() is your idle process.  It is called by the simulator when the idle
 * process is scheduled.
 *
 * This function should block until a process is added to your ready queue.
 * It should then call schedule() to select the process to run on the CPU.
 */
extern void idle(unsigned int cpu_id)
{
    pthread_mutex_lock(&current_mutex);
    //wait until the end of the process
    while (end == NULL) {
        pthread_cond_wait(&nonIdleThread , &current_mutex);
    }
    //schedule cpu use after waiting
    pthread_mutex_unlock(&current_mutex);
    schedule(cpu_id);

    /*
     * REMOVE THE LINE BELOW AFTER IMPLEMENTING IDLE()
     *
     * idle() must block when the ready queue is empty, or else the CPU threads
     * will spin in a loop.  Until a ready queue is implemented, we'll put the
     * thread to sleep to keep it from consuming 100% of the CPU time.  Once
     * you implement a proper idle() function using a condition variable,
     * remove the call to mt_safe_usleep() below.
     */
   
}


/*
 * preempt() is the handler called by the simulator when a process is
 * preempted due to its timeslice expiring.
 *
 * This function should place the currently running process back in the
 * ready queue, and call schedule() to select a new runnable process.
 */
extern void preempt(unsigned int cpu_id)
{
    pcb_t* tempThread;
    pthread_mutex_lock(&current_mutex);
    tempThread = current[cpu_id];
    pthread_mutex_unlock(&current_mutex);
    current[cpu_id]->state = PROCESS_READY;
    pthread_mutex_lock(&availThread);
    if (begin != NULL) {
        end->next = tempThread;
        end = tempThread;
    } else {
        begin = tempThread;
        end = tempThread;
    }
    pthread_cond_broadcast(&nonIdleThread);
    pthread_mutex_unlock(&availThread);
    schedule(cpu_id);
}


/*
 * yield() is the handler called by the simulator when a process yields the
 * CPU to perform an I/O request.
 *
 * It should mark the process as WAITING, then call schedule() to select
 * a new process for the CPU.
 */
extern void yield(unsigned int cpu_id)
{
    pcb_t* tempThread;
    pthread_mutex_lock(&current_mutex);
    tempThread = current[cpu_id];
    current[cpu_id]->state = PROCESS_WAITING;
    pthread_mutex_unlock(&current_mutex);
    schedule(cpu_id);
}


/*
 * terminate() is the handler called by the simulator when a process completes.
 * It should mark the process as terminated, then call schedule() to select
 * a new process for the CPU.
 */
extern void terminate(unsigned int cpu_id)
{
    pcb_t* tempThread;
    pthread_mutex_lock(&current_mutex);
    tempThread = current[cpu_id];
    current[cpu_id]->state = PROCESS_TERMINATED; //set the state to terminated
    pthread_mutex_unlock(&current_mutex);
    schedule(cpu_id); //might be redudandant
}


/*
 * wake_up() is the handler called by the simulator when a process's I/O
 * request completes.  It should perform the following tasks:
 *
 *   1. Mark the process as READY, and insert it into the ready queue.
 *
 *   2. If the scheduling algorithm is LRTF, wake_up() may need
 *      to preempt the CPU with lower remaining time left to allow it to
 *      execute the process which just woke up with higher reimaing time.
 * 	However, if any CPU is currently running idle,
* 	or all of the CPUs are running processes
 *      with a higher remaining time left than the one which just woke up, wake_up()
 *      should not preempt any CPUs.
 *	To preempt a process, use force_preempt(). Look in os-sim.h for 
 * 	its prototype and the parameters it takes in.
 */
extern void wake_up(pcb_t *process)
{
    pthread_mutex_lock(&availThread);
    process->state = PROCESS_READY;
    if (begin == NULL) {
        begin = process;
        end = process;
    } else {
        end->next = process;
        end = process;
    }
    pthread_cond_signal(&nonIdleThread);
    pthread_mutex_unlock(&availThread);
    unsigned int i;
    pcb_t* high;
    high = process;
    unsigned int ind = 0;
    if (LRTF == 1) {
        pthread_mutex_lock(&current_mutex);
        for (i = 0; i < cpu_mode; i++) {
            if (current[i] == NULL) {
                high = current[i];
                break;
            }
            if (current[i]->time_remaining < high->time_remaining) {
                high = current[i];
                ind= i;
            }
        }
        pthread_mutex_unlock(&current_mutex);
        if (high != NULL && high != process) {
            force_preempt(ind);
        }
    }
}

/*
 * main() simply parses command line arguments, then calls start_simulator().
 * You will need to modify it to support the -l and -r command-line parameters.
 */
int main(int argc, char *argv[])
{
    unsigned int cpu_count;

    /* Parse command-line arguments */
    if (argc != 2)
    {
        fprintf(stderr, "ECE 3056 OS Sim -- Multithreaded OS Simulator\n"
            "Usage: ./os-sim <# CPUs> [ -l | -r <time slice> ]\n"
            "    Default : FIFO Scheduler\n"
	    "         -l : Longest Remaining Time First Scheduler\n"
            "         -r : Round-Robin Scheduler\n\n");
    }
    cpu_count = strtoul(argv[1], NULL, 0);
    cpu_mode = cpu_count;
    
    /* FIX ME - Add support for -l and -r parameters*/
    if(!argv[2]){
            fifo = 1;
            timePart = -1;
        }else if(strcmp(argv[2],"-r")==0){
            rr = 1;
            timePart = strtoul(argv[3], NULL, 0);
        }else if(strcmp(argv[2],"-l")==0){
            LRTF = 1;
            timePart = -1;
        }else{

        }
    /* Allocate the current[] array and its mutex */
    current = malloc(sizeof(pcb_t*) * cpu_count);
    assert(current != NULL);
    pthread_mutex_init(&current_mutex, NULL);
    pthread_mutex_init(&availThread, NULL);
    pthread_cond_init(&nonIdleThread, NULL);

    /* Start the simulator in the library */
    start_simulator(cpu_count);

    return 0;
}

