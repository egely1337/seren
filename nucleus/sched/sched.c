#include <nucleus/sched/sched.h>


const char* KERNEL_IDLE_TASK = KERNEL_TASK_NAME;

// TODO: yield()
void yield(
void) {

}

// TODO: sched_init()
void sched_init(
void) {

}

//TODO: create_task_from_address
void create_task_from_address(
const char *task_name, 
uint64_t eip,
uint8_t priority) {
    (void)eip;
    (void)priority;
    (void)task_name;
}
