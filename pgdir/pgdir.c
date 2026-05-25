//-----------------------------------------------------------------------------------
//	pgdir.c
// tao 1 module file ao pgdir trong /proc de xem info cua thuoc tinh pgdir entry in kernel.
// Murt Meiv.
// Thu 4, ngay 30 thang 6 nam 2021.
// Tested with:
// Linux murt-pc-25062021 5.8.0-59-generic #66~20.04.1-Ubuntu SMP Thu Jun 17 11:14:10 UTC 2021 x86_64 x86_64 x86_64 GNU/Linux
// # cat /proc/pgdir
//-----------------------------------------------------------------------------------
#include <linux/module.h>  //ham init_module()
#include <linux/proc_fs.h>  //ham create_proc_read_entry()
#include <linux/seq_file.h> //ham seq_printf()
#include <linux/sched.h>  // struct task_struct
#include <linux/mm.h>  // struct mm_struct
#include <asm/io.h>  // for phys_to_virt()

#define OK 0;
  static char modname[]="pgdir";

static int mm_proc_show(struct seq_file *m, void *v) {
  unsigned long ptdb, *pgdir;
	int		i, k, attr;

	// inline assembly-language is used here to access register CR3
	asm(" mov %%cr3, %%rax \n mov %%rax, %0 " : "=m" (ptdb) :: "ax" );
	pgdir = phys_to_virt( ptdb );

	// title for the visualization
	seq_printf( m, "\n\n    A visualization of the CPU's" );
	seq_printf( m, " Virtual Address Space under Linux \n" );

	// loop through page-directory entries (from highest to lowest)
	for (i = 0; i < 1024; i++)
		{
		k = 1023 - i;
		attr = (pgdir[ k ] & 7) | '0';
		if ( attr == '0' ) attr = '-';
		if ( ( i % 64 ) == 0 ) seq_printf( m, "\n   " );
		seq_printf( m, "%c", attr );
		if ( ( i % 64 ) == 63 ) 
			seq_printf( m, "  %08x ", k << 22 );

		}

	// display explanatory legend (plus some extra information)
	seq_printf( m, "\n\n\n    Legend:  " );
	seq_printf( m, "'-'=unmapped  " ); 	
	seq_printf( m, "'3'=supervisor  " ); 	
	seq_printf( m, "'7'=user  " ); 
	seq_printf( m, "         (CR3=%018lx) \n\n", ptdb );

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
