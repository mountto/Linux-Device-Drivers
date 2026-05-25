//----------------------------------------------------------------------------------
// 1 module tao 1 file ao tren proc/smpinfo, de xem thong tin MP Floating Pointer, MP Configuration table hearder,...
// Thu 4,ngay 20 thang 10 nam 2021
//	smpinfo.c
// Murt Meiv
// Linux murt-pc-25062021 5.8.0-59-generic #66~20.04.1-Ubuntu SMP Thu Jun 17 11:14:10 UTC 2021 x86_64 x86_64 x86_64 GNU/Linux
//
// cat /proc/smpinfo
//----------------------------------------------------------------------------------
#include <linux/module.h> //ham module_init(), module_exit()
#include <linux/proc_fs.h> //ham proc_create(),remove_proc_entry()
#include <linux/seq_file.h> //ham seq_printf()
#include <asm/io.h>		// for ioremap(), iounmap()
#include <linux/uaccess.h> // for copy_from_user()

//#define IO_APIC_DEFAULT_PHYS_BASE	0xfec00000
//#define APIC_DEFAULT_PHYS_BASE	0xfee00000

#define OK 0;

static char modname[] = "smpinfo";	// name for our '/proc' file
unsigned char *mpfp;		// MP Floating Pointer address
unsigned char *mpct;		// MP Configuration Table address
unsigned short baselen;		// Base Configuration Table length



static int my_proc_show(struct seq_file *m, void *v)
{

	int	i, len = 0;
	
	seq_printf( m, "\n MP Floating Pointer structure\n" );
	for (i = 0; i < 16; i++) 
		seq_printf( m, " %02X", mpfp[i] );
	seq_printf( m, "\n" );


	seq_printf( m, "\n MP Configuration Table Header " );
	for (i = 0; i < 44; i++) 
		{
		if ( ( i % 16 ) == 0 ) seq_printf( m, "\n" );
		seq_printf( m, " %02X", mpct[i] );
		}
	seq_printf( m, "\n" );

	seq_printf( m, "\n Base Configuration Table entries \n" );
	for (i = 44; i < baselen; ) 
		{
		int	j, k = ( mpct[i] ) ? 8 : 20;
		for (j = 0; j < k; j++)
			seq_printf( m, " %02X", mpct[i+j] );
		seq_printf( m, "\n" );
		i += k;
		}
	seq_printf( m, "\n" );

	return	len;

}
static int my_proc_open(struct inode *inode,struct file *file){
	return single_open(file,my_proc_show,NULL);
}
static ssize_t
my_proc_write(struct file *file,
				const char __user * buffer,
				size_t count, loff_t * ppos)
{

	char strbuf[5];
	char str[5] = "";

	if (count > 4)
		count = 4;

	if (copy_from_user(strbuf, buffer, count))
		return -EFAULT;
	strbuf[count] = '\0';
	sscanf(strbuf, "%s", str);
			printk(KERN_ERR "my_proc_write --> %s\n", str);


	return count;
}

static const struct proc_ops my_proc_op = {
	//.owner = THIS_MODULE,
	.proc_open = my_proc_open,
	.proc_read = seq_read,
	.proc_write = my_proc_write,
	.proc_lseek = seq_lseek,
	.proc_release = single_release,
};




static int __init my_proc_init(void)
{
	unsigned long	where;

	printk( "<1>\nInstalling \'%s\' module\n", modname );

	// search for the MP Floating Pointer structure
	for (where = 0xF0000; where < 0x100000; where += 16)
		{
		mpfp = (char*)phys_to_virt( where );
		if ( strncmp( mpfp, "_MP_", 4 ) == 0 ) break;
		}	
	if ( where == 0x100000 ) return -ENODEV;

	printk( "MP Floating Pointer structure found " );
	printk( "at physical-address %08lX \n", where ); 

	// initialize pointer to the MP Configuration Table
	where = (*(unsigned int*)(mpfp+4));
	mpct = (unsigned char*)phys_to_virt( where );
	printk( "MP Configuration Table Header at %08lX \n", where );

	// initialize location and size of the Base Configuration Table
	baselen = *(int*)(mpct+4);
	
	proc_create(modname,0,NULL,&my_proc_op);
	printk( "<1>Created \'%s\' module\n", modname );
	return OK;
}
static void __exit my_proc_exit(void)
{
	remove_proc_entry(modname,NULL);
	printk( "<1>Removing \'%s\' module\n", modname );
}


MODULE_LICENSE("GPL");
module_init(my_proc_init);
module_exit(my_proc_exit);
