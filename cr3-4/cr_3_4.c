//----------------------------------------------------------------------------------
//1 module tao 1 file ao tren proc/cr34, de xem info CR3 va CR4
//CN,ngay 27 thang 6 nam 2021
//	cr_3_4.c
//Murt Meiv
//Linux murt-pc-25062021 5.8.0-59-generic #66~20.04.1-Ubuntu SMP Thu Jun 17 11:14:10 UTC 2021 x86_64 x86_64 x86_64 GNU/Linux
//
//cat /proc/cr34
//----------------------------------------------------------------------------------
#include <linux/module.h> //ham module_init(), module_exit()
#include <linux/proc_fs.h> //ham proc_create(),remove_proc_entry()
#include <linux/seq_file.h> //ham seq_printf()

#define OK 0;

static char modname[] = "cr34";

static int my_proc_show(struct seq_file *m, void *v){
  uint64_t _cr3,_cr4;
	asm("mov %%cr3,%%rax \n mov %%rax,%0" : "=m" (_cr3) :: "ax" );
	asm("mov %%cr4,%%rax \n mov %%rax,%0" : "=m" (_cr4) :: "ax" );
	seq_printf(m,"	CR3 = %#018LX ",_cr3);
	seq_printf(m,"	CR4 = %#018Lx ",_cr4);
	seq_printf(m,"	PAE = %#018Lx ",((_cr4 >> 5)&1));
	seq_printf(m,"	PSE = %#018LX \n",((_cr4 >> 4)&1));
	return OK;
}
static int my_proc_open(struct inode *inode,struct file *file){
	return single_open(file,my_proc_show,NULL);
}

static const struct proc_ops my_proc_op = {
	.proc_open = my_proc_open,
	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};

static int my_proc_init(void)
{
	proc_create(modname,0,NULL,&my_proc_op);
	return OK;
}
static void my_proc_exit(void)
{
	remove_proc_entry(modname,NULL);
}


MODULE_LICENSE("GPL");
module_init(my_proc_init);
module_exit(my_proc_exit);
