# /**
# * anchor.c: To get the MAC address of this station's network interface.
# *
# * @copyright Copyright (c) 2026, Hoàng Hải <leduchoanghai@yahoo.com>
# * @license   MIT, http://www.opensource.org/licenses/mit-license.php
# */

# NOTE: Written and tested for Linux kernel version 5.4.0-113-generic.


#include <linux/module.h>	// for init_module() 
#include <linux/utsname.h>	// for utsname()
#include <linux/pci.h>		// for pci_get_device()
#include <linux/proc_fs.h>	// for create_proc_info_entry() 
#include <asm/io.h>		// for inl(), outl()
#include <linux/seq_file.h> //ham seq_printf()

#define VENDOR_ID	 0x10ec	// Intel Corporation
#define DEVICE_ID 	0x8168	// 82573L ANCHOR
#define HW_ADDRESS	0x5400	// offset to MAC-ADDRESS


char legend[] = "Network controller: Intel Corporation Wireless 3165 (rev 81)";
char modname[] = "anchor";
struct pci_dev	*devp;
//struct pci_dev *pdev;
u8 __iomem *hw_addr;
unsigned char 	mac[8];
unsigned long 	membase, memsize;
//void	*io;
u8 __iomem *io;


static int my_proc_show(struct seq_file *m, void *v)
{
	struct new_utsname	*uts = utsname();
	u32	datum;
	int	i, len = 0;

	seq_printf( m, "\n\n %58s \n\n", legend );
	seq_printf( m, "\n %48s \n", "PCI CONFIGURATION SPACE" );
	for (i = 0; i < 256; i+=4)
		{
		if ( ( i % 32 ) == 0 ) 
			seq_printf( m, "\n %04X: ", i );
		pci_read_config_dword( devp, i, &datum );
		seq_printf( m, "%08X ", datum );
		}
	seq_printf( m, "\n\n" );

	seq_printf( m, " MAC-ADDRESS: " );
	for (i = 0; i < 6; i++)
		{
		seq_printf( m, "%02X", mac[i] );
		if ( i < 5 ) seq_printf( m, ":" );
		}
	seq_printf( m, "   WORKSTATION: %s ", uts->nodename );
	seq_printf( m, "\n\n" );

	return	len;
}

static int my_proc_open(struct inode *inode,struct file *file){
	return single_open(file,my_proc_show,NULL);
}

static const struct file_operations my_proc_op = {
	.owner = THIS_MODULE,
	.open = my_proc_open,
	.read = seq_read,
//	.write = seq_write,
	.llseek = seq_lseek,
	.release = single_release,
};

static int __init my_init( void )
{
   int bar, err;


	// display confirmation message
	printk( "<1>\nInstalling \'%s\' module\n", modname );

	// detect the tigon3 device 
	devp = pci_get_device( VENDOR_ID, DEVICE_ID, NULL );
	if ( !devp ) return -ENODEV;

    printk(KERN_INFO "Device vid: 0x%X pid: 0x%X\n", VENDOR_ID, DEVICE_ID);


	// remap the device's I/O memory
	membase = pci_resource_start( devp, 0 );
	memsize = pci_resource_len( devp, 0 );
	io = ioremap( membase, memsize );
	if ( !io ) return -EBUSY;

	
	// copy device's hardware address to 'mac[]' array
	mac[0] = *(unsigned char*)(io + HW_ADDRESS + 0);
	mac[1] = *(unsigned char*)(io + HW_ADDRESS + 1);
	mac[2] = *(unsigned char*)(io + HW_ADDRESS + 2);
	mac[3] = *(unsigned char*)(io + HW_ADDRESS + 3);
	mac[4] = *(unsigned char*)(io + HW_ADDRESS + 4);
	mac[5] = *(unsigned char*)(io + HW_ADDRESS + 5);


	// create pseudo-file in the '/proc' directoey
	//create_proc_info_entry( modname, 0, NULL, my_get_info );
	proc_create(modname,0,NULL,&my_proc_op);
	return	0;  //SUCCESS
}


static void __exit my_exit(void )
{
	// delete the pseudo-file
	//remove_proc_entry( modname, NULL );
	remove_proc_entry(modname,NULL);
	// unmap the device's I/O memory
	iounmap( io );

	// display confirmation message
	printk( "<1>Removing \'%s\' module\n", modname );
}

module_init( my_init );
module_exit( my_exit );
MODULE_LICENSE("GPL"); 