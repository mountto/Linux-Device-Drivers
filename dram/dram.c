/**
 * @file   dram.c
 * @author Murt Meiv
 * @date   Thu 3, ngay 6 thang 7 nam 2021
 * @version 0.1
 * @brief   An introductory character driver to support the second article of my series on
 * Linux loadable kernel module (LKM) development. This module maps to /dev/dram and
 * comes with a helper C program that can be run in Linux user space to communicate with
 * this the LKM.
 * @see here for a full description and follow-up descriptions.
 * Tested: Linux murt-pc-25062021 5.8.0-59-generic #66~20.04.1-Ubuntu SMP Thu Jun 17 11:14:10 UTC 2021 x86_64 x86_64 x86_64 GNU/Linux
 * $ cat /dev/dram or ./testmychar (source code testmychar.c)
 */

#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/highmem.h>		// kmap() , kunmap()
//#include <linux/mmzone.h>
#include <linux/mm.h>		// get_num_physpages()
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>          // Required for the copy to user function
#define  DEV_NAME "devram"    ///< The device will appear at /dev/mychar using this value
#define  CLASS_NAME  "dram_classname"        ///< The device class -- this is a character device driver
#define MEM_MAJOR		123
static DEFINE_MUTEX(misc_mtx);

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
loff_t dev_lseek( struct file *, loff_t , int );
/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */

static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .llseek  = dev_lseek,
   .write = dev_write,
   .release = dev_release,
};

static const struct devname
{
   const char *name;
	umode_t mode;
	const struct file_operations *fops;
	fmode_t fmode;
} devname_list[] = {	
   [1] = { "dram", 0666, &fops, FMODE_NOWAIT },
   [4] = { "dramtest", 0, &fops, 0 },
};


MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Murt Meiv");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A simple Linux char driver for the DrAM");  ///< The description -- see modinfo
MODULE_VERSION("0.1");            ///< A version number to inform users

static int    majorNumber;                  ///< Stores the device number -- determined automatically
static char   message[256] = {0};           ///< Memory for the string that is passed from userspace
static short  size_of_message;              ///< Used to remember the size of the string stored
static int    numberOpens = 0;              ///< Counts the number of times the device is opened
static struct class*  mycharClass  = NULL; ///< The device-driver class struct pointer
static struct device* mycharDevice = NULL; ///< The device-driver device struct pointer

unsigned long dram_size;


int minor;

//struct vchar_drv {
//	dev_t dev_num;
//} vchar_drv;

// ha`m chmod 666 cho dev file
static char *mem_devnode(struct device *dev, umode_t *mode)
{
	if (mode && devname_list[MINOR(dev->devt)].mode)
		*mode = devname_list[MINOR(dev->devt)].mode;
	return NULL;
}


/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init mychar_init(void){
   printk(KERN_INFO "%s: Initializing the MyChar LKM\n",DEV_NAME);

   // Try to dynamically allocate a major number for the device -- more difficult but worth it
   majorNumber = register_chrdev(MEM_MAJOR, DEV_NAME, &fops);
   if (majorNumber<0){
      printk(KERN_ALERT "%s:MyChar failed to register a major number\n",DEV_NAME);
      return majorNumber;
   }
   printk(KERN_INFO "%s: registered correctly with major number %d\n", DEV_NAME, MEM_MAJOR);

   // Register the device class
   mycharClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(mycharClass)){                // Check for error and clean up if there is
      unregister_chrdev(MEM_MAJOR, DEV_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(mycharClass);          // Correct way to return an error on a pointer
   }
   printk(KERN_INFO "%s: device class registered correctly\n", DEV_NAME);

	mycharClass->devnode = mem_devnode;
            mutex_lock(&misc_mtx);

for (minor = 1; minor < ARRAY_SIZE(devname_list); minor++) {
		if (!devname_list[minor].name)
			continue;

		mycharDevice = device_create(mycharClass, NULL, MKDEV(MEM_MAJOR, minor),
			      NULL, devname_list[minor].name);
            mutex_unlock(&misc_mtx);

      if (IS_ERR(mycharDevice)){               // Clean up if there is an error
         class_destroy(mycharClass);           // Repeated code but the alternative is goto statements
         unregister_chrdev(MEM_MAJOR, DEV_NAME);
         printk(KERN_ALERT "%s: Failed to create the device\n",DEV_NAME);
         return PTR_ERR(mycharDevice);
      }
}

/*
   dev_num = MKDEV(majorNumber, 0);
   // Register the device driver
   mycharDevice = device_create(mycharClass, NULL, dev_num, NULL, devname_list[1].name);
   if (IS_ERR(mycharDevice)){               // Clean up if there is an error
      class_destroy(mycharClass);           // Repeated code but the alternative is goto statements
      unregister_chrdev(majorNumber, devname_list[1].name);
      printk(KERN_ALERT "%s: Failed to create the device\n",devname_list[1].name);
      return PTR_ERR(mycharDevice);
   }
*/
   printk(KERN_INFO "%s: device class created correctly\n",DEV_NAME); // Made it! device was initialized

	dram_size = get_num_physpages() << PAGE_SHIFT;
//	int mbytes = get_num_physpages() >> (20-PAGE_SHIFT);
	printk(KERN_INFO "%s: <1>  ramtop=%08lX (%lu MB)\n",DEV_NAME, dram_size, dram_size >> 20 );

   return 0;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit mychar_exit(void){

         mutex_lock(&misc_mtx);
   for (minor = 1; minor < ARRAY_SIZE(devname_list); minor++) {
		if (!devname_list[minor].name)
			continue;
      device_destroy(mycharClass, MKDEV(MEM_MAJOR, minor));     // remove the device
   }
         mutex_unlock(&misc_mtx);

      //class_unregister(mycharClass);                          // unregister the device class
      class_destroy(mycharClass);                             // remove the device class
      unregister_chrdev(MEM_MAJOR, DEV_NAME);             // unregister the major number
      printk(KERN_INFO "%s: Goodbye from the LKM!\n",DEV_NAME);

}

/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep){
   numberOpens++;
   printk(KERN_INFO "%s: Device has been opened %d time(s)\n",DEV_NAME, numberOpens);
   return 0;
}

/** @brief This function is called whenever device is being read from user space i.e. data is
 *  being sent from the device to the user. In this case is uses the copy_to_user() function to
 *  send the buffer string to the user and captures any errors.
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 *  @param buffer The pointer to the buffer to which this function writes the data
 *  @param len The length of the b
 *  @param offset The offset if required
 */
static ssize_t dev_read( struct file *file, char *buf, size_t count, loff_t *pos ){

	struct page	*pp;
	void		*from;
	int		page_number, page_indent, more;
	
	// we cannot read beyond the end-of-file
	if ( *pos >= dram_size ) return 0;

	// determine which physical page to temporarily map
	// and how far into that page to begin reading from 
	page_number = *pos / PAGE_SIZE;
	page_indent = *pos % PAGE_SIZE;
	
	// map the designated physical page into kernel space
	pp = &vmemmap[ page_number ];
	from = kmap( pp ) + page_indent;
	
	// cannot reliably read beyond the end of this mapped page
	if ( page_indent + count > PAGE_SIZE ) count = PAGE_SIZE - page_indent;

	// now transfer count bytes from mapped page to user-supplied buffer 	
	more = copy_to_user( buf, from, count );
	
	// ok now to discard the temporary page mapping
	kunmap( pp );
	
	// an error occurred if less than count bytes got copied
	if ( more ) return -EFAULT;
	
	// otherwise advance file-pointer and report number of bytes read
	*pos += count;
	return	count;

}
///////////////////////
loff_t dev_lseek( struct file *file, loff_t offset, int whence )
{
	loff_t	newpos = -1;

	switch( whence )
		{
		case 0: newpos = offset; break;			// SEEK_SET
		case 1: newpos = file->f_pos + offset; break; 	// SEEK_CUR
		case 2: newpos = dram_size + offset; break; 	// SEEK_END
		}

	if (( newpos < 0 )||( newpos > dram_size )) return -EINVAL;
	file->f_pos = newpos;
	return	newpos;
}

/** @brief This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using the sprintf() function along with the length of the string.
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
   sprintf(message, "%s(%zu letters)", buffer, len);   // appending received string with its length
   size_of_message = strlen(message);                 // store the length of the stored message
   printk(KERN_INFO "%s: Received %zu characters from the user\n",DEV_NAME, len);
   return len;
}

/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "%s: Device successfully closed\n",DEV_NAME);
   return 0;
}

/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(mychar_init);
module_exit(mychar_exit);
