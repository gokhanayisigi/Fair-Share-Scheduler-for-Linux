#include <linux/enableFair.h>

int sched_flag = 0;

asmlinkage int sys_enableFair(int flag) {
    if (flag == 0) {  // default scheduler
        sched_flag = 0;
		printk("Default scheduler activated.\n");
        return 0;
    } else if (flag == 1) {  // fair-share scheduler
        sched_flag = 1;
		printk("Fair-Share Scheduler activated.\n");
        return 1;
    } else {
        printk("invalid option\n");
        return -1;
    }
}
