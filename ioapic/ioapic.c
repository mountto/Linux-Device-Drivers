//----------------------------------------------------------------------------------
// 1 module tao 1 file ao tren proc/ioapic, de xem thong tin register thuoc memory-mapped I/O APIC
// Thu 4,ngay 7thang 7 nam 2021
//	ioapic.c
// Murt Meiv
// Linux murt-pc-25062021 5.8.0-59-generic #66~20.04.1-Ubuntu SMP Thu Jun 17 11:14:10 UTC 2021 x86_64 x86_64 x86_64 GNU/Linux
//
// cat /proc/ioapic
//----------------------------------------------------------------------------------
#include <linux/module.h> //ham module_init(), module_exit()
#include <linux/proc_fs.h> //ham proc_create(),remove_proc_entry()
#include <linux/seq_file.h> //ham seq_printf()
#include <asm/io.h>		// for ioremap(), iounmap()

//#define IO_APIC_DEFAULT_PHYS_BASE	0xfec00000
//#define APIC_DEFAULT_PHYS_BASE	0xfee00000

#define OK 0;

static char modname[] = "ioapic";

static int my_proc_show(struct seq_file *m, void *v){
// * ioremap     -   map bus memory into CPU space
// * @offset:    bus address of the memory
// * @size:      size of the resource to map
	void	*io = ioremap( IO_APIC_DEFAULT_PHYS_BASE, PAGE_SIZE );
	void	*to = (void*)((long)io + 0x00);
	void	*fr = (void*)((long)io + 0x10);
	int	i, maxirq, ident, versn;

	iowrite32( 0, to ); ident = ioread32( fr );
	iowrite32( 1, to ); versn = ioread32( fr );
	maxirq = ( versn >> 16 )&0x00FF;


	 seq_printf( m, "\n  I/O APIC       " );
	 seq_printf( m, " Identification: %08X  ", ident );
	 seq_printf( m, "     Version: %08X   \n", versn );
	 seq_printf( m, "\n%25s", " " );
	 seq_printf( m, "%d Redirection-Table entries \n", 1+maxirq );

	for (i = 0; i <= maxirq; i++)
		{
		unsigned int	val_lo, val_hi;
		iowrite32( 0x10 + 2*i, to );
		val_lo = ioread32( fr );
		iowrite32( 0x10+2*i+1, to );
		val_hi = ioread32( fr );
		if ( ( i % 3 ) == 0 )  seq_printf( m, "\n" );
		 seq_printf( m, "  0x%02X: ", i );
		 seq_printf( m, "%08X%08X  ", val_hi, val_lo );
		}
	 seq_printf( m, "\n\n" );

	iounmap( io );
//        seq_printf( m, " hi \n" );

	return	OK;

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
