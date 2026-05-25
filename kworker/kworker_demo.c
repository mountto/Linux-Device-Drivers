//
//  kworker_demo.c
//
//
 #include <linux/module.h>
 #include <linux/sched.h>

 static void work_queue_function( struct work_struct *data )
 {
   pr_info( "work_queue_function( jiffies: %ld ) %d\n", jiffies, current->pid );
   return;
 }

 static DECLARE_WORK(work, work_queue_function);

 static int __init mod_init(void)
 {
   if( schedule_work( &work )==0 ) {
     pr_info( "schedule_work failed ...\n");
   } else {
     pr_info( "schedule_work successful ...\n");
   }
   return 0;
 }

 static void __exit mod_exit(void)
 {
   pr_info("mod_exit called\n");
   flush_scheduled_work();
 }

 module_init( mod_init );
 module_exit( mod_exit );
 MODULE_LICENSE("GPL");
