//----------------------------------------------------------------------------------
//1 module tao 1 file ao tren proc/listusb, de xem danh sách của USB trên system
//CN,ngay 21 thang 3 nam 2023
//	listusb.c
//Murt Meiv
// Linux murt-asus 6.2.6 #1 SMP PREEMPT_DYNAMIC Wed Mar 15 10:11:44 EDT 2023 x86_64 GNU/Linux
//
//cat /proc/listusb
//----------------------------------------------------------------------------------
#include <linux/module.h> //ham module_init(), module_exit()
#include <linux/proc_fs.h> //ham proc_create(),remove_proc_entry()
#include <linux/seq_file.h> //ham seq_printf()
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>
#include <linux/notifier.h>

#define OK 0;

static char modname[] = "listusb";
struct usb_userdata {
	unsigned long long busnum;
	unsigned long long devnum;
	struct usb_device *d;
};
static int usb_device_find(struct usb_device *d, void *userdata)
{
	struct usb_userdata *user = userdata;
	//char *start;
		user->d = usb_get_dev(d);
		//start += sprintf(start,"bus_nr = %d \n",user->d->bus->busnum);
       printk(KERN_INFO "%s:  \n",modname);
	printk(KERN_INFO "%s:	bus_nr = %d ",modname,user->d->bus->busnum);
	printk(KERN_INFO "%s:	device_nr = %d ",modname,user->d->devnum);
	printk(KERN_INFO "%s:	product = %s ",modname,user->d->product);
	printk(KERN_INFO "%s:	manufacturer = %s \n",modname,user->d->manufacturer);
       printk(KERN_INFO "%s:  \n",modname);

	usb_put_dev(user->d);

	return 0;
}
static int my_proc_show(struct seq_file *m, void *v){


	

	seq_printf(m,"	================== Va`o dmesg ma` xem nha ======== \n");
	//seq_printf(m,"	manufacturer = %s \n",user.d->manufacturer);
 
 	return 0;
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
    	struct usb_userdata user = {};
		int r;

		//usb_register_notify(&mon_nb);

	proc_create(modname,0,NULL,&my_proc_op);
	   printk(KERN_INFO "%s: Initializing the LKM\n",modname);

		r = usb_for_each_dev(&user, usb_device_find);
		printk(KERN_INFO "%s:	r = %d ",modname,r);

	return OK;
}
static void my_proc_exit(void)
{
		//usb_unregister_notify(&mon_nb);

	remove_proc_entry(modname,NULL);
	       printk(KERN_INFO "%s: =============================It Over the LKM=====================\n",modname);

}


MODULE_LICENSE("GPL");
module_init(my_proc_init);
module_exit(my_proc_exit);
