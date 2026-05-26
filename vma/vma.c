//-------------------------------------------------------------------
//	vma.c
//
//	This module creates a pseudo-file (named '/proc/vma') which
//	allows a user-task to see information about its memory-map.
//	(The various Virtual Memory Areas which are associated with 
//	a task are maintained by the kernel within a linked-list of
//	'vm_area_struct' objects in the task's 'mm_struct' object.)
//
//	
//	 LE DUC HOANG HAI
//	revised on: Sun 01 Jan 2023 01:19:39 PM EST
//				Linux murt-asus 6.1.0 #4 SMP PREEMPT_DYNAMIC Wed Dec 28 03:21:26 EST 2022 x86_64 GNU/Linux
//
//-------------------------------------------------------------------

#include <linux/module.h>	// for init_module() 
#include <linux/proc_fs.h>	// for create_proc_read_entry() 
#include <linux/mm.h>		// for 'struct vm_area_struct'
#include <linux/sched.h>	// for 'struct task_struct'
#include <linux/seq_file.h> //ham seq_printf()

#define OK 0;

char modname[] = "vma";
static int vma_proc_show(struct seq_file *m, void *v) 
{
	struct task_struct	*tsk = current;
	struct vm_area_struct *vma;
	struct mm_struct *mm = tsk->mm; 
			MA_STATE(mas, &mm->mm_mt, 0, 0);
			VMA_ITERATOR(vmi, mm, 0);

	unsigned long 		ptdb;
	int			i = 0, len = 0;
	struct file *file;
	//vm_flags_t flags = vma->vm_flags;
	const char *name = NULL;

	unsigned long ino = 0;
	unsigned long long pgoff = 0;
	dev_t dev = 0;

	// display title
	seq_printf( m, "\n\nList of the Virtual Memory Areas " );
	seq_printf( m, "for task \'%s\' ", tsk->comm );
	seq_printf( m, "(pid=%d)\n", tsk->pid );

	// loop to traverse the list of the task's vm_area_structs
	vma = mas_find(&mas, ULONG_MAX);

	mmap_read_lock(mm);
	for_each_vma(vmi, vma) {

		char	ch;	
		seq_printf( m, "\n%3d ", ++i );
		seq_printf( m, " vm_start=%08lX ", vma->vm_start );
		seq_printf( m, " vm_end=%08lX  ", vma->vm_end );

		ch = ( vma->vm_flags & VM_READ ) ? 'r' : '-';
		seq_printf( m, "%c", ch );
		ch = ( vma->vm_flags & VM_WRITE ) ? 'w' : '-';
		seq_printf( m, "%c", ch );
		ch = ( vma->vm_flags & VM_EXEC ) ? 'x' : '-';
		seq_printf( m, "%c", ch );
		ch = ( vma->vm_flags & VM_SHARED ) ? 's' : 'p';
		seq_printf( m, "%c", ch );
		
            file = vma->vm_file;
    if (file) {
		struct inode *inode = file_inode(vma->vm_file);
		dev = inode->i_sb->s_dev;
		ino = inode->i_ino;
		pgoff = ((loff_t)vma->vm_pgoff) << PAGE_SHIFT;
	}
        	seq_printf(m, " %08llx ", pgoff);
	seq_printf(m, " %02x ", MAJOR(dev));
	seq_printf(m, ":%02x", MINOR(dev));
	seq_printf(m, " %ld ", ino);
	seq_putc(m, ' ');
	if (file) {
		seq_pad(m, ' ');
		seq_file_path(m, file, "\n");
		goto done;
	}

	if (vma->vm_ops && vma->vm_ops->name) {
		name = vma->vm_ops->name(vma);
		if (name)
			goto done;
	}



done:
	if (name) {
		seq_pad(m, ' ');
		seq_puts(m, name);
	}
	seq_putc(m, '\n');


		}
	seq_printf( m, "\n" );


	// display additional information about tsk->mm
    	asm(" mov %%cr3, %%rax \n mov %%rax, %0 " : "=m" (ptdb) :: "ax" );

	seq_printf( m, "\nCR3=%08lX ", ptdb );
	seq_printf( m, " mm->pgd=%p ", tsk->mm->pgd );
	seq_printf( m, " mm->map_count=%d ", tsk->mm->map_count );
	seq_printf( m, "\n\n" );
	mmap_read_unlock(mm);

	return	len;
}




static int vma_proc_open(struct inode *inode, struct  file *file) {
  	return single_open(file, vma_proc_show, NULL);
}

static const struct proc_ops vma_proc_fops = {
  .proc_open = vma_proc_open,
  .proc_read = seq_read,
  .proc_lseek = seq_lseek,
  .proc_release = single_release,
};

static int __init vma_proc_init(void) {
  	proc_create(modname, 0, NULL, &vma_proc_fops);
    	printk( "<1>\nInstalling \'%s\' module\n", modname );

  	return OK;
}

static void __exit vma_proc_exit(void) {
  	remove_proc_entry(modname, NULL);
    	printk( "<1>Removing \'%s\' module\n", modname );

}

MODULE_LICENSE("GPL");
module_init(vma_proc_init);
module_exit(vma_proc_exit);


