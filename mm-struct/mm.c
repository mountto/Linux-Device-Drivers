//-----------------------------------------------------------------------------------
//	mm.c
// tao 1 module file ao mm trong /proc de xem info cua task mm in kernel.
// Murt Meiv.
// Thu 7, ngay 26 thang 6 nam 2021.
// Tested with:
// Linux murt-pc-25062021 5.8.0-59-generic #66~20.04.1-Ubuntu SMP Thu Jun 17 11:14:10 UTC 2021 x86_64 x86_64 x86_64 GNU/Linux
// # cat /proc/mm
//-----------------------------------------------------------------------------------
#include <linux/module.h>  //ham init_module()
#include <linux/proc_fs.h>  //ham create_proc_read_entry()
#include <linux/seq_file.h> //ham seq_printf()
#include <linux/sched.h>  // struct task_struct
#include <linux/mm.h>  // struct mm_struct

#define OK 0;
  static char modname[]="mm";

static int mm_proc_show(struct seq_file *m, void *v) {
  struct task_struct *tsk = current;
  struct mm_struct *mm = tsk->mm;
  unsigned long		stack_size = (mm->stack_vm << PAGE_SHIFT);
  unsigned long		down_to = mm->start_stack - stack_size;

	seq_printf( m, "\nInfo from the Memory Management structure " );
	seq_printf( m, "for task \'%s\' ", tsk->comm );
	seq_printf( m, "(pid=%d) \n", tsk->pid );
	seq_printf( m, "   pgd=%08lX  ", (unsigned long)mm->pgd );
	seq_printf( m, "mmap_base=%08lX  ", (unsigned long)mm->mmap_base );
	seq_printf( m, "map_count=%d  ", mm->map_count );
	seq_printf( m, "mm_users=%d  ", mm->mm_users.counter );
	seq_printf( m, "mm_count=%d  ", mm->mm_count.counter );
	seq_printf( m, "\n" );
	seq_printf( m, "    start_code=%08lX  ", mm->start_code );
	seq_printf( m, " end_code=%08lX\n", mm->end_code );
	seq_printf( m, "    start_data=%08lX  ", mm->start_data );
	seq_printf( m, " end_data=%08lX\n", mm->end_data );
	seq_printf( m, "     start_brk=%08lX  ", mm->start_brk );
	seq_printf( m, "      brk=%08lX\n", mm->brk );
	seq_printf( m, "     arg_start=%08lX  ", mm->arg_start );
	seq_printf( m, "  arg_end=%08lX\n", mm->arg_end );
	seq_printf( m, "     env_start=%08lX  ", mm->env_start );
	seq_printf( m, "  env_end=%08lX\n", mm->env_end );
	seq_printf( m, "   start_stack=%08lX  ", mm->start_stack );
	seq_printf( m, "  down_to=%08lX ", down_to );
	seq_printf( m, " <--- stack grows downward \n" );
	seq_printf( m, "\n" );
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
