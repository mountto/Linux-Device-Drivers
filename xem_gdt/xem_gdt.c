//-----------------------------------------------------------------------------------
//	xem_gdt.c
// tao 1 module file ao gdt trong /proc de xem info cua thuoc tinh gdt entry in kernel.
// Murt Meiv.
// Thu 4, ngay 30 thang 6 nam 2021.
// Tested with:
// Linux murt-pc-25062021 5.8.0-59-generic #66~20.04.1-Ubuntu SMP Thu Jun 17 11:14:10 UTC 2021 x86_64 x86_64 x86_64 GNU/Linux
// # cat /proc/xemgdt
//-----------------------------------------------------------------------------------
#include <linux/module.h>  //ham init_module()
#include <linux/proc_fs.h>  //ham create_proc_read_entry()
#include <linux/seq_file.h> //ham seq_printf()
#include <linux/sched.h>  // struct task_struct
#include <linux/mm.h>  // struct mm_struct
#include <asm/io.h>  // for phys_to_virt()

#define OK 0;
  	static char modname[]="xemgdt";
unsigned short 	gdtr[3];

//struct desc_ptr *dtrr;

unsigned long	gdt_virt_address; 
unsigned long	gdt_phys_address;

static int mm_proc_show(struct seq_file *m, void *v) {

	// use inline assembly language to get GDTR register-image
//	asm(" sgdt gdtr ");
//	asm volatile("sgdt %0":"=m" (*dtrr));

	asm("sgdt %0" : "=m"(gdtr));
//	seq_printf( m," dtr->address= %#018lx ;",dtrr->address);
	// extract GDT virtual-address from GDTR register-image
	gdt_virt_address = *(unsigned long*)(gdtr+1);
	seq_printf( m," gdtr= %#018lx ;",*(unsigned long*)(gdtr));

	// compute GDT physical-address using subtraction
	gdt_phys_address = gdt_virt_address - __START_KERNEL_map;

	// extract GDT segment-limit and compute descriptor count
	//int	n_elts = (1 + gdtr[0])/8;

	// report the GDT virtual and physical memory-addresses
	seq_printf( m," __PAGE_OFFSET= %#018lx ;",__PAGE_OFFSET);
	seq_printf( m," __START_KERNEL_map= %#018lx ;",__START_KERNEL_map);
	seq_printf( m,"\n               " );
	seq_printf( m,"gdt_virt_address= %#018lx ", gdt_virt_address );
	seq_printf( m, "gdt_phys_address= %#08lx ", gdt_phys_address );
	seq_printf( m, "\n" );
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
