/**
 * @file   my_sysfc.c
 * @author Murt Meiv
 * @date   Thu 6, ngay 2 thang 7 nam 2021
 * @brief  A kernel module group sysfs .
 * The sysfs entry appears at /sys/mysysfs/myg2021
 * @see Linux murt-pc-25062021 5.8.0-59-generic #66~20.04.1-Ubuntu SMP Thu Jun 17 11:14:10 UTC 2021 x86_64 x86_64 x86_64 GNU/Linux
 * $ cat /sys/mysysfs/myg2021/vfile_1  or # echo 111 > /sys/mysysfs/myg2021/vfile_1
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/kobject.h>    // Using kobjects for the sysfs bindings
#include <linux/time.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Murt Meiv");
MODULE_DESCRIPTION("A simple Linux sysfs LKM for the MySysfs");
MODULE_VERSION("0.1");

static char   my_sysfs_name[] = "mysysfs";      ///< Null terminated default string -- just in case
static char   my_g_name[8] = "myg2021";      ///< Null terminated default string -- just in case
static int    vfile_1 = 1;
static bool   vfile_2 = 2;
static int    vfile_3 = 3;
static struct timespec64 ts_last, ts_diff;  ///< timespecs from linux/time.h (has nano precision)

static ssize_t vfile_1_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
   return sprintf(buf, "%d\n", vfile_1);
}

static ssize_t vfile_1_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
   sscanf(buf, "%du", &vfile_1);
   return count;
}

static ssize_t vfile_2_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
   return sprintf(buf, "%d\n", vfile_2);
}

static ssize_t lastTime_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
   return sprintf(buf, "%.2llu:%.2llu:%.2llu:%.9lu \n",(ts_last.tv_sec/3600)%24,
          (ts_last.tv_sec/60) % 60, ts_last.tv_sec % 60, ts_last.tv_nsec );
}

static ssize_t diffTime_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
   return sprintf(buf, "%llu.%.9lu\n", ts_diff.tv_sec, ts_diff.tv_nsec);
}

static ssize_t vfile_3_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf){
   return sprintf(buf, "%d\n", vfile_3);
}

static ssize_t vfile_3_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count){
   sscanf(buf, "%du", &vfile_3);
   return count;
}
static struct kobj_attribute vfile_1_attr = __ATTR(vfile_1, 0664, vfile_1_show, vfile_1_store);
static struct kobj_attribute vfile_3_attr = __ATTR(vfile_3, 0664, vfile_3_show, vfile_3_store);


/**  The __ATTR_RO macro defines a read-only attribute. There is no need to identify that the
 *  function is called _show, but it must be present. __ATTR_WO can be  used for a write-only
 *  attribute but only in Linux 3.11.x on.
 */
static struct kobj_attribute vfile_2_attr = __ATTR_RO(vfile_2);
static struct kobj_attribute time_attr  = __ATTR_RO(lastTime);
static struct kobj_attribute diff_attr  = __ATTR_RO(diffTime);

static struct attribute *mysysfs_attrs[] = {
      &vfile_1_attr.attr,
      &vfile_2_attr.attr,
      &time_attr.attr,
      &diff_attr.attr,
      &vfile_3_attr.attr,
      NULL,
};

static struct attribute_group attr_group = {
      .name  = my_g_name,
      .attrs = mysysfs_attrs,
};

static struct kobject *mysysfs_kobj;

// @brief The LKM initialization function
static int __init my_sysfs_init(void){
   int result = 0;

   printk(KERN_INFO "mysysfs: Initializing the MySysfs LKM\n");

   // create the kobject sysfs entry at /sys/mysysfs -- probably not an ideal location!
   mysysfs_kobj = kobject_create_and_add(my_sysfs_name, kernel_kobj->parent); // kernel_kobj points to /sys/kernel
   if(!mysysfs_kobj){
      printk(KERN_ALERT "mysysfs: failed to create kobject mapping\n");
      return -ENOMEM;
   }
   // add the attributes to /sys/mysysfs/ -- for example, /sys/mysysfs/myg2012/vfile_1
   result = sysfs_create_group(mysysfs_kobj, &attr_group);
   if(result) {
      printk(KERN_ALERT "mysysfs: failed to create sysfs group\n");
      kobject_put(mysysfs_kobj);                          // clean up -- remove the kobject sysfs entry
      return result;
   }
   ktime_get_ts64(&ts_last);                          // set the last time to be the current time
   ts_diff = timespec64_sub(ts_last, ts_last);          // set the initial time difference to be 0


   return result;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit my_sysfs_exit(void){
   printk(KERN_INFO "mysysfs: count vfile_1 %d times\n", vfile_1);
   kobject_put(mysysfs_kobj);                   // clean up -- remove the kobject sysfs entry
   printk(KERN_INFO "mysysfs: Goodbye from the MySysfs LKM!\n");
}

// This next calls are  mandatory -- they identify the initialization function
// and the cleanup function (as above).
module_init(my_sysfs_init);
module_exit(my_sysfs_exit);
