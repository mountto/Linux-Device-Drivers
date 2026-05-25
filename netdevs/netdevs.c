//----------------------------------------------------------------------------------
//1 module tao 1 file ao tren proc/netdevs, de xem displays a list of the current 'struct net_device' objects
//CN,ngay 27 thang 8 nam 2021
//	netdevs.c
//Murt Meiv
//Linux murt-pc-25062021 5.8.0-59-generic #66~20.04.1-Ubuntu SMP Thu Jun 17 11:14:10 UTC 2021 x86_64 x86_64 x86_64 GNU/Linux
//
//cat /proc/netdevs
//----------------------------------------------------------------------------------
#include <linux/module.h> //ham module_init(), module_exit()
#include <linux/proc_fs.h> //ham proc_create(),remove_proc_entry()
#include <linux/seq_file.h> //ham seq_printf()
#include <linux/netdevice.h>	// for 'struct net_device'
#include <asm/io.h>		// for virt_to_phys()
#include <linux/rtnetlink.h> // ham rtnl_lock()
#include <linux/rwsem.h>  //ham down_read()
#define OK 0;


static char modname[] = "netdevs";
char legend[] = "List of the kernel's current 'struct net_device' objects";
char header[] = "MemAddress   HardwareAddress  ifindex  Name";


static int my_proc_show(struct seq_file *m, void *v){
	struct net_device	*dev;
	struct net *net;

	int			i, len = 0;
	
	seq_printf( m, "\n %s\n\n %s\n", legend, header );
	
	/* Lock the rtnl to make sure the netdevs does not move under
	 * our feet
	 */
	rtnl_lock();
	down_read(&net_rwsem);
	for_each_net(net)
		for_each_netdev( net , dev ) 
		{
		unsigned long	phys_address = virt_to_phys( dev );
		const unsigned char	*mac = dev->dev_addr;
		seq_printf( m, " 0x%08lX  ", phys_address );
		for (i = 0; i < 6; i++) 
		    seq_printf( m, "%02X%c", mac[i], (i<5)?':':' ' );
		seq_printf( m, " %3d   ", dev->ifindex ); 
		seq_printf( m, "  %s ", dev->name );
		seq_printf( m, "\n" );
		}
	up_read(&net_rwsem);
	rtnl_unlock();

	seq_printf( m, "\n" );
	seq_printf( m, " sizeof( struct net_device )=" );
	seq_printf( m, "%ld bytes\n\n", sizeof( struct net_device ) );
	return	len;
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
	printk(KERN_INFO "%s: device created correctly\n",modname); // Made it! device was initialized
	return OK;
}
static void my_proc_exit(void)
{
	remove_proc_entry(modname,NULL);
	printk(KERN_INFO "%s: device closed, exited correctly\n",modname); 
}


MODULE_LICENSE("GPL");
module_init(my_proc_init);
module_exit(my_proc_exit);
