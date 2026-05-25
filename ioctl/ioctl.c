/**
 * @file   ioctl.c
 * @author Murt Meiv
 * @date   Thu 4, ngay 7 thang 7 nam 2021
 * @version 0.1
 * @brief   An introductory character driver to support the second article of my series on
 * Linux loadable kernel module (LKM) development. This module maps to /dev/ioctl and
 * comes with a helper C program that can be run in Linux user space to communicate with
 * this the LKM.
 * @see here for a full description and follow-up descriptions.
 * Tested: Linux murt-pc-25062021 5.8.0-59-generic #66~20.04.1-Ubuntu SMP Thu Jun 17 11:14:10 UTC 2021 x86_64 x86_64 x86_64 GNU/Linux
 * $ cat /dev/ioctl or sudo ./testmyioctl (source code testmyioctl.c)
 */

#include <linux/init.h>           // Macros used to mark up functions e.g. __init __exit
#include <linux/module.h>         // Core header for loading LKMs into the kernel
#include <linux/device.h>         // Header to support the kernel Driver Model
#include <linux/kernel.h>         // Contains types, macros, functions for the kernel
#include <linux/highmem.h>		// kmap() , kunmap()
#include <linux/ioctl.h>
#include <linux/mm.h>		// get_num_physpages()
#include <linux/fs.h>             // Header for the Linux file system support
#include <linux/uaccess.h>          // Required for the copy to user function
#define  DEVICE_NAME "ioctl_dev"    ///< The device will appear at /dev/mychar using this value
#define  CLASS_NAME  "ioctl_classname"        ///< The device class -- this is a character device driver
#define  DEV_MAJOR_NO 123 

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Murt Meiv");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("A simple Linux char driver for the IOctl");  ///< The description -- see modinfo
MODULE_VERSION("0.1");            ///< A version number to inform users

static int    majorNumber;                  ///< Stores the device number -- determined automatically
static char   message[256] = {0};           ///< Memory for the string that is passed from userspace
static short  size_of_message;              ///< Used to remember the size of the string stored
static int    numberOpens = 0;              ///< Counts the number of times the device is opened
static struct class*  mycharClass  = NULL; ///< The device-driver class struct pointer
static struct device* mycharDevice = NULL; ///< The device-driver device struct pointer
#define WR_VALUE _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)

int32_t value_sent_to_user = 9999;

int32_t value = 0;
int minor;
// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
	loff_t dev_lseek( struct file *, loff_t , int );
static long     dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg);
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
   .unlocked_ioctl = dev_ioctl,
   .release = dev_release,
};

//struct vchar_drv {
//	dev_t dev_num;
//} vchar_drv;


static const struct devname
{
   const char *name;
	umode_t mode;
	const struct file_operations *fops;
	fmode_t fmode;
} devname_list[] = {	
   [1] = { "ioctl", 0666, &fops, FMODE_NOWAIT },
   [4] = { "ioctl_test", 0, &fops, 0 },
};

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
   printk(KERN_INFO "%s: Initializing the device LKM\n",DEVICE_NAME);

   // Try to dynamically allocate a major number for the device -- more difficult but worth it
   majorNumber = register_chrdev(DEV_MAJOR_NO, DEVICE_NAME, &fops);
   if (majorNumber<0){
      printk(KERN_ALERT "%s:MyChar failed to register a major number\n",DEVICE_NAME);
      return majorNumber;
   }
   printk(KERN_INFO "%s: registered correctly with major number %d\n", DEVICE_NAME, DEV_MAJOR_NO);

   // Register the device class
   mycharClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(mycharClass)){                // Check for error and clean up if there is
      unregister_chrdev(DEV_MAJOR_NO, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(mycharClass);          // Correct way to return an error on a pointer
   }
   printk(KERN_INFO "%s: device class registered correctly\n", DEVICE_NAME);

	mycharClass->devnode = mem_devnode;

for (minor = 1; minor < ARRAY_SIZE(devname_list); minor++) {
		if (!devname_list[minor].name)
			continue;
   //dev_num = MKDEV(DEV_MAJOR_NO, minor);
   // Register the device driver
   mycharDevice = device_create(mycharClass, NULL, MKDEV(DEV_MAJOR_NO, minor), NULL, devname_list[minor].name);
   if (IS_ERR(mycharDevice)){               // Clean up if there is an error
      class_destroy(mycharClass);           // Repeated code but the alternative is goto statements
      unregister_chrdev(DEV_MAJOR_NO, DEVICE_NAME);
      printk(KERN_ALERT "%s: Failed to create the device\n",DEVICE_NAME);
      return PTR_ERR(mycharDevice);
   }
}
   printk(KERN_INFO "%s: device class created correctly\n",DEVICE_NAME); // Made it! device was initialized

   return 0;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit mychar_exit(void){
   for (minor = 1; minor < ARRAY_SIZE(devname_list); minor++) {
		if (!devname_list[minor].name)
			continue;
      device_destroy(mycharClass, MKDEV(DEV_MAJOR_NO, minor));     // remove the device
   }
   //device_destroy(mycharClass, dev_num);     // remove the device
   //class_unregister(mycharClass);                          // unregister the device class
   class_destroy(mycharClass);                             // remove the device class
   unregister_chrdev(DEV_MAJOR_NO, DEVICE_NAME);             // unregister the major number
   printk(KERN_INFO "%s: Goodbye from the LKM!\n",DEVICE_NAME);
}

/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inodep, struct file *filep){
   numberOpens++;
   printk(KERN_INFO "%s: Device has been opened %d time(s)\n",DEVICE_NAME, numberOpens);
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
   int error_count = 0;
   // copy_to_user has the format ( * to, *from, size) and returns 0 on success
   error_count = copy_to_user(buf, message, size_of_message);

   if (error_count==0){            // if true then have success
      printk(KERN_INFO "%s: Sent %d characters to the user\n",DEVICE_NAME, size_of_message);
      return (size_of_message=0);  // clear the position to the start and return 0
   }
   else {
      printk(KERN_INFO "%s: Failed to send %d characters to the user\n",DEVICE_NAME, error_count);
      return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
   }

}
///////////////////////
loff_t dev_lseek( struct file *file, loff_t offset, int whence )
{
	loff_t	newpos = -1;

	switch( whence )
		{
		case 0: newpos = offset; break;			// SEEK_SET
		case 1: newpos = file->f_pos + offset; break; 	// SEEK_CUR
		case 2: newpos = file->f_inode->i_size + offset; break; 	// SEEK_END
		}

	if (( newpos < 0 )||( newpos > file->f_inode->i_size )) return -EINVAL;
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
   printk(KERN_INFO "%s: Received %zu characters from the user\n",DEVICE_NAME, len);
   return len;
}

/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "%s: Device successfully closed\n",DEVICE_NAME);
   return 0;
}
/*
** This function will be called when we write IOCTL on the Device file
*/
static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
         switch(cmd) {
                case WR_VALUE:
                        if( copy_from_user(&value ,(int32_t*) arg, sizeof(value)) )
                        {
                                pr_err("%s:Data Write : Err!\n",DEVICE_NAME);
                        }
                        pr_info("%s: Value from user = %d\n",DEVICE_NAME, value);
                        break;
                case RD_VALUE:
                        if( copy_to_user((int32_t*) arg, &value_sent_to_user, sizeof(value_sent_to_user)) )
                        {
                                pr_err("%s:Data Read : Err!\n",DEVICE_NAME);
                        }
                        break;
                default:
                        pr_info("%s:Default\n",DEVICE_NAME);
                        break;
        }
        return 0;
}
 
/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(mychar_init);
module_exit(mychar_exit);
