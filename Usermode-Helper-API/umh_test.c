//----------------------------------------------------------------------------------
//1 module để chạy một process ở usermode từ kernel mode
//CN,ngay 27 thang 6 nam 2021
//	umh_test.c
//Murt Meiv
// Linux murt-asus 6.2.6 #1 SMP PREEMPT_DYNAMIC Wed Mar 15 10:11:44 EDT 2023 x86_64 GNU/Linux
//
//cat /proc/umh_test
//----------------------------------------------------------------------------------
#include <linux/module.h> //ham module_init(), module_exit()
#include <linux/proc_fs.h> //ham proc_create(),remove_proc_entry()
#include <linux/seq_file.h> //ham seq_printf()
#include <linux/kmod.h>
#include <linux/kernel.h>

#define OK 0;

static char modname[] = "umh_test";

//    struct subprocess_info *sub_info;


int alter_uid_gid(uid_t uid, gid_t gid, struct cred *new)
{
        new->uid = new->euid = new->suid = new->fsuid = KUIDT_INIT(uid);
        new->gid = new->egid = new->sgid = new->fsgid = KGIDT_INIT(gid);
        return 0;
}

static int init_func(struct subprocess_info *info, struct cred *new)
{
        printk(KERN_INFO "%s:current->pid= [%d]\n",modname, current->pid);
        alter_uid_gid(1001, 1001, new);// on my system user: teon:x:1001:1001:Teo Nho,2020,+,,not:/home/teon:/bin/bash
        return 0;
}

static int user_process_fork(void *data)
{
    struct subprocess_info *sub_info;
    int ret = 0;
    char *path = (char *)data;
    char *argv[] = {path,"message from kernel device driver", NULL};
    //char* argv[] = {"/home/teon/test", "message from kernel device driver", NULL};
    static char *envp[] = {
    "HOME=/",
    "TERM=linux",
    "PATH=/sbin:/bin:/usr/sbin:/usr/bin", NULL };


    sub_info = call_usermodehelper_setup(argv[0], argv, envp, GFP_ATOMIC,
            init_func, NULL, NULL);
    if (sub_info == NULL) return -ENOMEM;

    ret = call_usermodehelper_exec(sub_info, UMH_KILLABLE);
    pr_info(KERN_INFO "%s:   %s: ret %d\n",modname, __func__, ret);
    //do_exit(0);

    return ret;
}





static int my_proc_init(void)
{
	//proc_create(modname,0,NULL,&my_proc_op);
    user_process_fork("/home/teon/test");
   printk(KERN_INFO "%s: Initializing the LKM\n",modname);

	return OK;
}
static void my_proc_exit(void)
{
        //call_usermodehelper_freeinfo(sub_info);
        //umh_complete(sub_info);
        
       printk(KERN_INFO "%s: It Over the LKM\n",modname);

	//remove_proc_entry(modname,NULL);
}


MODULE_LICENSE("GPL");
module_init(my_proc_init);
module_exit(my_proc_exit);
