#pragma once

#include "egos.h"
#include "syscall.h"

/* MLFQ Constants */
#define MLFQ_NLEVELS          5
#define MLFQ_RESET_PERIOD     10000000       
#define MLFQ_LEVEL_RUNTIME(x) ((x) + 1) * 100000 

enum proc_status {
    PROC_UNUSED,
    PROC_LOADING,
    PROC_READY,
    PROC_RUNNING,
    PROC_RUNNABLE,
    PROC_PENDING_SYSCALL
};

#define MAX_NPROCESS        16
#define SAVED_REGISTER_NUM  32
#define SAVED_REGISTER_SIZE SAVED_REGISTER_NUM * 4
#define SAVED_REGISTER_ADDR (void*)(EGOS_STACK_TOP - SAVED_REGISTER_SIZE)

struct process {
    int pid;
    struct syscall syscall;
    enum proc_status status;
    uint mepc, saved_registers[SAVED_REGISTER_NUM];
    /* Student's code goes here (Preemptive Scheduler | System Call). */

    /* Add new fields for lifecycle statistics, MLFQ or process sleep. */
    
    /* Lifecycle data for the process */
    ulonglong turnaround_time;     /* Time between creation and termination */
    ulonglong response_time;       /* Time between creation and first schedule */
    ulonglong cpu_time;            /* Accumulated time process actually ran on CPU */
    ulonglong num_timer_interrupts; /* Number of timer interrupts encountered */
    
    /* Information necessary for MLFQ scheduling */
    uint queue_level;              /* Current MLFQ queue level (0-4) */
    ulonglong remaining_runtime;   /* Remaining runtime on current level */
    
    /* Timestamps for lifecycle tracking */
    ulonglong creation_time;       /* When process was created */
    ulonglong first_schedule_time; /* When process was first scheduled */
    ulonglong last_schedule_time;  /* Last time process was scheduled */

    /* Student's code ends here. */
};

ulonglong mtime_get();

int proc_alloc();
void proc_free(int);
void proc_set_ready(int);
void proc_set_running(int);
void proc_set_runnable(int);
void proc_set_pending(int);

void mlfq_reset_level();
void mlfq_update_level(struct process* p, ulonglong runtime);
void proc_sleep(int pid, uint usec);
void proc_coresinfo();

extern uint core_to_proc_idx[NCORES];