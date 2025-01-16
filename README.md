1\. **INTRODUCTION**

Process scheduling is a critical function in operating systems, determining how processes are assigned to run on available CPUs. The default Linux scheduler (SCHED_OTHER) uses a time-sharing approach, ensuring fairness by dynamically adjusting process priorities. This project explores the implementation of an alternative scheduling algorithm, the Fair Share Scheduler, which proportionally allocates CPU time among users or groups, ensuring that each entity receives its designated share of processing resources.

**Background and Motivation**

The default Linux scheduler is designed to balance fairness and efficiency by dynamically adjusting process priorities based on their behavior and resource requirements. While this approach works well for general-purpose computing, it may not always provide the level of fairness required in specific scenarios, such as multi-user environments or workloads requiring more precise resource distribution. The Fair Share Scheduler, an alternative approach, aims to address this by allocating CPU time proportionally among users or groups, offering enhanced control over resource distribution and improved fairness in certain use cases.

**Problem Statement**

The primary objective of this project is to implement Fair Share Scheduler into Linux 2.4.27 Kernel. The default scheduler operates on a time-sharing basis, dynamically adjusting process priorities to ensure fairness. In contrast, the Fair Share Scheduler allocates CPU time based on user or group shares, aiming to provide proportional resource allocation in multi-user or controlled usage environments. This study evaluates the effectiveness of the Fair Share Scheduler in delivering equitable CPU distribution and analyzes its performance relative to the default scheduler.

**Project Environment**

The project environment involved using a virtual machine (VM) to create a controlled setting for developing and testing the schedulers. The VM setup ensured consistency and reproducibility of the tests. Various tools and utilities were employed to facilitate the development and testing process, including:

• Kernel Development: Modifying the Linux kernel to implement the Fair Share Scheduler.

• Testing Tools: Using the top command to monitor and collect CPU utilization and resource allocation data.

• Scripting: Writing shell scripts to automate the testing process and gather data efficiently.

2\. **DESIGN and IMPLEMENTATION**

**2.1. Default Linux Scheduler**

The default Linux scheduler operates on a time-sharing basis, assigning CPU time slices to processes based on dynamic priority calculations. The task_struct in the kernel maintains process-specific information, including state, counter, and nice value. The scheduler uses a multilevel queue structure, preempting lower-priority tasks when higher-priority ones become runnable. The core function, schedule(), iterates over runnable tasks, selects the next task based on the highest priority, and recalculates counters when necessary to maintain fairness.

In the creation process managed by the do_fork function found in fork.c, various values are dynamically assigned. Initially, processes are created with a "nice" value of 0, indicating medium priority. Consequently, their priority is calculated as 20 − nice 20−nice, starting with 20. The state member variable indicates whether a task is runnable, which is crucial for the scheduler when iterating through the linked list of tasks.

A key variable to consider is the counter, calculated using the formula: ( 20 − nice ) / 4 + 1 + counter / 2 (20−nice)/4+1+counter/2. This counter value is essential in the default scheduler because it dictates the remaining time in an epoch. Understanding the main scheduler mechanism is crucial to grasp the significance of these task structure members.

The scheduler's main role is to manage CPU time among various tasks, ensuring fair and efficient execution. It evaluates the state and counter of each task as it iterates over the task list. The counter reflects the remaining time for each task within its time slice, influencing the scheduler's decision on which task to run next. Tasks with higher counter values are prioritized, ensuring they get their fair share of CPU time before lower-priority tasks. To sum up, the do_fork function initializes processes with a balanced priority, while the state and counter variables play pivotal roles in the scheduling process. The careful calculation and management of these variables ensure the system runs smoothly, balancing the load and maintaining efficient CPU usage.

The scheduler function begins by setting the next variable to the result of the idle_task function, which is a placeholder task that runs when there are no runnable tasks. It also initializes the variable c to -1000, representing the minimum value the goodness function can return. The function then iterates over the tasks in the runqueue linked list. During each iteration, it checks if the task p is schedulable. If it is, the function calculates the task's weight using the formula (20 - nice) + counter, where the goodness function is applied. After calculating the weight, it compares this weight with the current value of c. If the weight is greater, it updates the values of c and next.

This process ensures that the task with the maximum counter-weight is selected to run. However, a special case must be considered: if all tasks have a counter-weight of 0 (as the counter decreases each time a task runs). The default scheduler addresses this by checking if c remains -1000. If it does, the scheduler reassigns the counter values of the tasks in a secondary conditional block. This approach guarantees fair preemptive scheduling. For instance, if there are four processes with only CPU bursts (since IO bursts reduce the nice value), the CPU utilization for each process would be approximately 25% because their initial counter values are equal.

**2.2. Fair Share Scheduler**

The Fair Share Scheduler introduces a share-based approach, where each user or group is assigned a certain percentage (or 'share') of the CPU. The scheduler then distributes CPU time proportionally to these shares, ensuring that entities with higher shares receive more CPU time. This mechanism ensures CPU allocation aligns with assigned resource proportions and promotes fairness in multi-user or group-based environments.

Implementation Details:

● Scheduling Logic: Implemented in sched.c with global sched_flag to switch between Default and Fair-Share schedulers.

● System Calls: Added sys_enableFair to switch scheduling policies.

The modifications ensure that the Fair Share Scheduler operates effectively by first identifying all active users in the system and counting their respective processes. For each scheduling decision, the scheduler calculates user shares by dividing the total available CPU time by the number of active users, ensuring each user receives an equal portion. Then, for each user's processes, their allocated CPU share is divided equally among their running processes. For example, if a user has 33.3% CPU share and runs two processes, each process receives 16.65% of the CPU time.

This approach contrasts with the default scheduler by focusing on user-centric resource allocation rather than treating all processes equally regardless of ownership. The scheduler maintains dynamic adjustment capabilities within sched.c, recalculating shares whenever the process count per user changes or when users enter/leave the system. This ensures consistent fairness in resource distribution while maintaining the system's responsiveness to changing workloads.

Key modifications include updates to the task_struct in sched.h, introducing new member variables to store user-based share information and process group identification. The schedule function in sched.c was adapted to handle both user-level and process-level CPU share calculations. Within sched.c, the implementation first calculates the equal distribution of CPU time among active users (33.3% each for three users), then determines the appropriate share for each process based on how many processes their owner is running. This centralized approach in sched.c maintains all user and process share calculations, ensuring fair distribution of CPU resources across users regardless of their process count. The scheduler dynamically adjusts these calculations when the number of users or processes changes to maintain fair sharing of system resources.

A new system call, enableFair, was added to manage the fair share scheduler. This system call is crucial for setting policies, ensuring smooth operation and management.

The implementation of the Fair Share Scheduler prioritizes fairness by focusing on user-level resource allocation. By dynamically adjusting CPU shares based on active users and their processes, the scheduler ensures equitable distribution of resources. The modifications to sched.c, updates to task_struct, and the addition of the enableFair system call provide a robust foundation for the Fair Share Scheduler, balancing fairness and responsiveness in a dynamic system environment.

**Code Implementation:**

**sched.c:**
```c
if(sched_flag){ //enter fair-share
	    if (unlikely(!c)) {
			int current_usr = 0;
			int usr_process[10] = {0};
			spin_unlock_irq(&runqueue_lock);
			read_lock(&tasklist_lock);
            // count users and processes
			list_for_each(tmp, &runqueue_head) {
				p = list_entry(tmp, struct task_struct, run_list);
				while (p->uid >= 1000) {
					while (p->uid < 1000 + 10) {
						int usr_index = p->uid % 10;
						while (usr_process[usr_index] == 0) {
							current_usr++;
							break; 
						}
						usr_process[usr_index]++;
						break; 
					}
					break;
				}
			}
			int usr_share = 0;
            if (current_usr > 0) {
                usr_share = 100;
            }

// share timeslice per users
            list_for_each(tmp, &runqueue_head) {
                p = list_entry(tmp, struct task_struct, run_list);
                while (p->uid >= 1000) {
                    while (p->uid < 1000 + 10) {
                        int usr_index = p->uid % 10;
                        while (usr_process[usr_index] > 0) {
                            p->counter = usr_share / usr_process[usr_index];
                            break;
                        }
                        break;
                    }
                    break;
                }
                while (p->uid < 1000) {
                    p->counter = (p->counter >> 1) + NICE_TO_TICKS(p->nice);
                    break;
                }
                while (p->uid >= 1000 + 10) {
                    p->counter = (p->counter >> 1) + NICE_TO_TICKS(p->nice);
                    break;
                }
            }
            read_unlock(&tasklist_lock);
            spin_lock_irq(&runqueue_lock);
            goto repeat_schedule;
        } 

```

**System Call Implementation:**

There is a system call added and associated with the Fair Share Scheduler implementation called sys_enableFairShare, which controls the scheduling behavior through a global variable sched_flag defined in sched.c. The initial value of sched_flag is set to 0 during kernel initialization, indicating that the system begins with the default Linux scheduler active. When sys_enableFairShare is called with a non-zero value, it switches sched_flag to 1, activating the Fair-Share scheduling policy.

The schedule function in sched.c uses this flag to determine which scheduling algorithm to execute. When sched_flag is 0, it maintains the default Linux scheduling behavior where each process receives an equal share of CPU time regardless of user ownership. When sched_flag is set to 1, the scheduler switches to Fair-Share mode, activating the user-based share calculations where CPU time is first divided equally among users (33.3% each for three users) and then subdivided among each user's processes.

This dynamic switching capability allows system administrators to toggle between scheduling policies without requiring a system restart. The system call provides flexibility in resource management, enabling administrators to adapt the scheduling policy based on system requirements. For example, they can switch to Fair-Share scheduling during periods of high multi-user activity to ensure equitable resource distribution, then revert to the default scheduler when user-based fairness is less critical.

enableFair.c file:
```c
# include &lt;linux/enableFair.h&gt;

int sched_flag = 0;

asmlinkage int sys_enableFair(int flag) {

  if (flag == 0) { // default scheduler

    sched_flag = 0;

    printk("Default scheduler activated.\\n");

    return 0;

  } else if (flag == 1) { // fair-share scheduler

    sched_flag = 1;

    printk("Fair-Share Scheduler activated.\\n");

    return 1;

  } else {

    printk("invalid option\\n");

    return -1;

  }

}
```
