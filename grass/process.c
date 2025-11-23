#include "process.h"

extern struct process proc_set[MAX_NPROCESS + 1];

static void proc_set_status(int pid, enum proc_status status) {
    for (uint i = 1; i <= MAX_NPROCESS; i++)
        if (proc_set[i].pid == pid)
            proc_set[i].status = status;
}

void proc_set_ready(int pid) { proc_set_status(pid, PROC_READY); }
void proc_set_running(int pid) { proc_set_status(pid, PROC_RUNNING); }
void proc_set_runnable(int pid) { proc_set_status(pid, PROC_RUNNABLE); }
void proc_set_pending(int pid) { proc_set_status(pid, PROC_PENDING_SYSCALL); }

int proc_alloc() {
    static uint curr_pid = 0;
    for (uint i = 1; i <= MAX_NPROCESS; i++)
        if (proc_set[i].status == PROC_UNUSED) {
            proc_set[i].pid    = ++curr_pid;
            proc_set[i].status = PROC_LOADING;
            /* Student's code goes here (Preemptive Scheduler | System Call). */

            /* Initialization of lifecycle statistics, MLFQ or process sleep. */
            ulonglong current_time = mtime_get();
            proc_set[i].creation_time = current_time;
            proc_set[i].first_schedule_time = 0;
            proc_set[i].last_schedule_time = 0;
            proc_set[i].turnaround_time = 0;
            proc_set[i].response_time = 0;
            proc_set[i].cpu_time = 0;
            proc_set[i].num_timer_interrupts = 0;
            
            /* MLFQ initialization - all processes start at level 0 (Rule 3) */
            proc_set[i].queue_level = 0;
            proc_set[i].remaining_runtime = MLFQ_LEVEL_RUNTIME(0);

            /* Student's code ends here. */
            return curr_pid;
        }

    FATAL("proc_alloc: reach the limit of %d processes", MAX_NPROCESS);
}
void proc_free(int pid) {
    /* Student's code goes here (Preemptive Scheduler). */

    
    if (pid != GPID_ALL) {
        for (uint i = 1; i <= MAX_NPROCESS; i++) {
            if (proc_set[i].pid == pid && proc_set[i].status != PROC_UNUSED) {
                ulonglong current_time = mtime_get();
                
                /* Add any remaining CPU time if process is currently running */
                if (proc_set[i].last_schedule_time > 0 && proc_set[i].last_schedule_time < current_time) {
                    ulonglong remaining_runtime = current_time - proc_set[i].last_schedule_time;
                    proc_set[i].cpu_time += remaining_runtime;
                }
                
                proc_set[i].turnaround_time = current_time - proc_set[i].creation_time;
                
                INFO("Process %d terminated:", pid);
                INFO("  Turnaround time: %d us", (int)proc_set[i].turnaround_time);
                INFO("  Response time: %d us", (int)proc_set[i].response_time);
                INFO("  CPU time: %d us", (int)proc_set[i].cpu_time);
                INFO("  Timer interrupts: %d", (int)proc_set[i].num_timer_interrupts);
                
                earth->mmu_free(pid);
                proc_set[i].status = PROC_UNUSED;
                break;
            }
        }
    } else {
        /* Free all user processes. */
        INFO("Terminating all user processes:");
        ulonglong current_time = mtime_get();
        
        for (uint i = 1; i <= MAX_NPROCESS; i++) {
            if (proc_set[i].pid >= GPID_USER_START &&
                proc_set[i].status != PROC_UNUSED) {
                
                /* Add any remaining CPU time if process is currently running */
                if (proc_set[i].last_schedule_time > 0 && proc_set[i].last_schedule_time < current_time) {
                    ulonglong remaining_runtime = current_time - proc_set[i].last_schedule_time;
                    proc_set[i].cpu_time += remaining_runtime;
                }
                
                proc_set[i].turnaround_time = current_time - proc_set[i].creation_time;
                
                INFO("Process %d:", proc_set[i].pid);
                INFO("  Turnaround time: %d us", (int)proc_set[i].turnaround_time);
                INFO("  Response time: %d us", (int)proc_set[i].response_time);
                INFO("  CPU time: %d us", (int)proc_set[i].cpu_time);
                INFO("  Timer interrupts: %d", (int)proc_set[i].num_timer_interrupts);
                
                earth->mmu_free(proc_set[i].pid);
                proc_set[i].status = PROC_UNUSED;
            }
        }
    }
    /* Student's code ends here. */
}

void mlfq_update_level(struct process* p, ulonglong runtime) {
    /* Student's code goes here (Preemptive Scheduler). */

    
    
    /* Deduct the runtime from remaining runtime on current level */
    if (p->remaining_runtime > runtime) {
        p->remaining_runtime -= runtime;
    } else {
        p->remaining_runtime = 0;
    }
    
    /* Rule 4: If a process uses up its time allotment at level i, 
     * move it to level i+1 */
    if (p->remaining_runtime == 0 && p->queue_level < (MLFQ_NLEVELS - 1)) {
        p->queue_level++;
        p->remaining_runtime = MLFQ_LEVEL_RUNTIME(p->queue_level);
    } else if (p->remaining_runtime == 0 && p->queue_level == (MLFQ_NLEVELS - 1)) {
        /* At lowest priority level, reset remaining runtime */
        p->remaining_runtime = MLFQ_LEVEL_RUNTIME(p->queue_level);
    }

    /* Student's code ends here. */
}

void mlfq_reset_level() {
    /* Student's code goes here (Preemptive Scheduler). */
    
    /* Rule 5: Reset the level of GPID_SHELL if there is pending keyboard input */
    if (!earth->tty_input_empty()) {
        for (uint i = 1; i <= MAX_NPROCESS; i++) {
            if (proc_set[i].pid == GPID_SHELL && proc_set[i].status != PROC_UNUSED) {
                proc_set[i].queue_level = 0;
                proc_set[i].remaining_runtime = MLFQ_LEVEL_RUNTIME(0);
                break;
            }
        }
    }

    static ulonglong MLFQ_last_reset_time = 0;
    /* Reset the level of all processes every MLFQ_RESET_PERIOD microseconds. */
    ulonglong current_time = mtime_get();
    
    if (MLFQ_last_reset_time == 0) {
        MLFQ_last_reset_time = current_time;
    }
    
    /* Rule 5: Every 10 seconds, move all processes to level 0 */
    if (current_time - MLFQ_last_reset_time >= MLFQ_RESET_PERIOD) {
        for (uint i = 1; i <= MAX_NPROCESS; i++) {
            if (proc_set[i].status != PROC_UNUSED) {
                proc_set[i].queue_level = 0;
                proc_set[i].remaining_runtime = MLFQ_LEVEL_RUNTIME(0);
            }
        }
        MLFQ_last_reset_time = current_time;
    }

    /* Student's code ends here. */
}

void proc_sleep(int pid, uint usec) {
    /* Student's code goes here (System Call & Protection). */

    /* Update the sleep-related fields in the struct process for process pid. */

    /* Student's code ends here. */
}

void proc_coresinfo() {
    /* Student's code goes here (Multicore & Locks). */

    /* Print out the pid of the process running on each CPU core. */

    /* Student's code ends here. */
}