/*
*	debugfs_file.c
*	id:	le duc hoang hai
*	link:  https://lore.kernel.org/lkml/20210401225833.566238-11-ira.weiny@intel.com/
*/

#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/entry-common.h>
#include <linux/fs.h>
#include <linux/list.h>
#include <linux/mman.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/percpu-defs.h>
#include <linux/pgtable.h>
#include <linux/pkeys.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>

  struct pks_test_ctx {
 	bool pass;
 	bool pks_cpu_enabled;
 	bool debug;
 	int pkey;
 	char data[64];
 };

	static struct dentry *pks_test_dentry,*test_dir;

 static ssize_t pks_read_file(struct file *file, char __user *user_buf,
 			     size_t count, loff_t *ppos)
 {
 	struct pks_test_ctx *ctx = file->private_data;
 	char buf[32];
 	unsigned int len;
    	 			pr_info("test pks_read_file\n");

 	if (!ctx){ 
		len = sprintf(buf, "not run\n");
		}
 	 	else {	len = sprintf(buf, "%s\n", ctx->pass ? "PASS" : "FAIL");
   	 			pr_info("test pks_read_file\n");
		}
 	return simple_read_from_buffer(user_buf, count, ppos, buf, len);
		
 	
 }


 static ssize_t pks_write_file(struct file *file, const char __user *user_buf,
			      size_t count, loff_t *ppos)
{
 	char buf[2];
 	//struct pks_test_ctx *ctx = file->private_data;
 
 	if (copy_from_user(buf, user_buf, 1))
 		return -EFAULT;
 	buf[1] = '\0';

  	 	pr_info("test pks_write_file\n");
 	return count;

}
 static int pks_release_file(struct inode *inode, struct file *file)
{
 	struct pks_test_ctx *ctx = file->private_data;
 
 	if (!ctx){
 		return 0;
  	 	pr_info("test pks_release_file\n");
 		kfree(ctx);
	}
 	return 0;

}

 static const struct file_operations fops_init_pks = {
 	.read = pks_read_file,
 	.write = pks_write_file,
 	.llseek = default_llseek,
 	.release = pks_release_file,
 };
  
 static int __init pks_test_init(void)
 {

	test_dir = debugfs_create_dir("run_pks_d2", NULL);
	if (!test_dir){
		return -EIO;
	}
 
 		//debugfs_create_file("run_pks", 0600, arch_debugfs_dir,NULL, &fops_init_pks);
 		// $ sudo ls -al /sys/kernel/debug/run_pks_d/run_pks

pks_test_dentry = debugfs_create_file("run_pks2", S_IRUSR | S_IWUSR, test_dir,NULL, &fops_init_pks);
	if (!pks_test_dentry){
		 	 	pr_info("test ko tao duoc run_pks\n");
	}
 	 	pr_info("test pks_test_init\n");
 
 		return 0;
 }

 
 static void __exit pks_test_exit(void)
 {
	debugfs_remove_recursive(test_dir);
 	//debugfs_remove_recursive(test_dir); //Xo'a toa`n bo files trong dir

 	//debugfs_remove(pks_test_dentry); //Xo'a 1 file ca`n remove
 	pr_info("test pks_test_exit\n");
 }


module_init(pks_test_init);
module_exit(pks_test_exit);

MODULE_DESCRIPTION("Test debugfs_create_file ");
MODULE_LICENSE("GPL");



