//
// netlink_test.c
// Linux murt-asus 6.2.6 #1 SMP PREEMPT_DYNAMIC Wed Mar 15 10:11:44 EDT 2023 x86_64 GNU/Linux
// $ss -a
// Netid    State     Recv-Q    Send-Q                                        Local Address:Port                         Peer Address:Port    Process    
// nl       UNCONN    0         0                                                      rtnl:1837105698                               *                   
// nl       UNCONN    0         0                                                      genl:1254097408                               *                   
// nl       UNCONN    0         0                                                        25:kernel                                   *                   
// p_dgr    UNCONN    0         0                                                       [0]:*                                        *                   

// https://insujang.github.io/2019-02-07/implementing-a-new-custom-netlink-family-protocol/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netlink.h>
#include <net/netlink.h>
#include <net/net_namespace.h>

#define NETLINK_TESTFAMILY 25
static char modname[] = "netlink_test";

struct sock *socket;

static void test_nl_receive_message(struct sk_buff *skb) {
  struct nlmsghdr *nlh = (struct nlmsghdr *) skb->data;

  printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

  //struct nlmsghdr *nlh = (struct nlmsghdr *) skb->data;
  printk(KERN_INFO "Received message: %s\n", (char*) nlmsg_data(nlh));
}

static int __init test_init(void) {
  struct netlink_kernel_cfg config = {
    .input = test_nl_receive_message,
  };

  socket = netlink_kernel_create(&init_net, NETLINK_TESTFAMILY, &config);
  if (socket == NULL) {
    return -1;
  }
	   printk(KERN_INFO "%s: Initializing the LKM\n",modname);

  return 0;
}

static void __exit test_exit(void) {
  if (socket) {
    netlink_kernel_release(socket);
    printk(KERN_INFO "%s: =============================It Over the LKM=====================\n",modname);

  }
}

MODULE_LICENSE("GPL");
module_init(test_init);
module_exit(test_exit);
