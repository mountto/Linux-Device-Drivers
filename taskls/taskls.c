//-----------------------------------------------------------------------------------
//	taskls.c
// tao 1 module file ao taskls trong /proc de xem info cua tasks in kernel.
// Murt Meiv.
// Thu 4, ngay 30 thang 6 nam 2021.
// Tested with:
// Linux murt-pc-25062021 5.8.0-59-generic #66~20.04.1-Ubuntu SMP Thu Jun 17 11:14:10 UTC 2021 x86_64 x86_64 x86_64 GNU/Linux
// # cat /proc/taskls
//-----------------------------------------------------------------------------------
#include <linux/module.h>  //ham init_module()
#include <linux/proc_fs.h>  //ham create_proc_read_entry()
#include <linux/seq_file.h> //ham seq_printf()
#include <linux/sched.h>  // struct task_struct
#include <linux/mm.h>  // struct mm_struct

#define OK 0;
  static char modname[]="taskls";

static int mm_proc_show(struct seq_file *m, void *v) {
  struct task_struct *task;
  int  taskcounts = 0;                      // 'global' so value will be retained
  seq_printf( m,  "#No " );
  seq_printf( m,  "pid "  );
  seq_printf( m,  "state " );
  seq_printf( m,  "comm " );
  seq_printf( m,  "priority " ); //priority
  seq_printf( m,  "static_prio " );
  seq_printf( m,  "normal_prio " );
  seq_printf( m,  "rt_prio " );
  seq_printf( m,  "policy " );//Scheduling strategy
  seq_printf( m,  "nvcsw " );//Context switch
  seq_printf( m,  "nivcsw " );
  seq_printf( m,  "utime " );//operation hours
  seq_printf( m,  "stime " );
  seq_printf( m,  "start_time " );
  seq_printf( m, "\n" );

 for_each_process(task) {
  seq_printf( m,  "#%-3d ", taskcounts++ );
  seq_printf( m,  "%5d ", task->pid );
  seq_printf( m,  "%u ", task->__state );
  seq_printf( m,  "%-15s ", task->comm );
  seq_printf( m,  "%-5d ", task->prio); //priority
  seq_printf( m,  "%-5d ", task->static_prio);
  seq_printf( m,  "%-5d ", task->normal_prio);
  seq_printf( m,  "%-5d ", task->rt_priority);
  seq_printf( m,  "%u ", task->policy);//Scheduling strategy
  seq_printf( m,  "%lu ", task->nvcsw);//Context switch
  seq_printf( m,  "%lu ", task->nivcsw);
  seq_printf( m,  "%llu ", task->utime);//operation hours
  seq_printf( m,  "%llu ", task->stime);
  seq_printf( m,  "%llu ", task->start_time);
  // Older: seq_printf( m, "%llu ", task->real_start_time);
  //seq_puts( m, "\n" );
	seq_printf( m, "\n" );
}
  	return OK;
}

static int mm_proc_open(struct inode *inode, struct  file *file) {
  	return single_open(file, mm_proc_show, NULL);
}

static const struct proc_ops mm_proc_fops = {
  .proc_open = mm_proc_open,
  .proc_read = seq_read,
  .proc_lseek = seq_lseek,
  .proc_release = single_release,
};

static int __init mm_proc_init(void) {
  	proc_create(modname, 0, NULL, &mm_proc_fops);
  	return OK;
}

static void __exit mm_proc_exit(void) {
  	remove_proc_entry(modname, NULL);
}

MODULE_LICENSE("GPL");
module_init(mm_proc_init);
module_exit(mm_proc_exit);
