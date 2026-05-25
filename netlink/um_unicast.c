#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define NETLINK_TESTFAMILY		25
#define PAYLOAD_SIZE		1024

int main()
{

	struct sockaddr_nl src_addr;
	int ret = 0;
	int sock_fd = 0;

	struct nlmsghdr *nlh = NULL;     // The nlmsghdr with payload to send
	struct sockaddr_nl dest_addr;

	// Open Socket:
	sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_TESTFAMILY);
	if (sock_fd < 0)
	{
		printf("socket creation for NETLINK_TESTFAMILY failed...\n");
		return -1;
	}

	// Bind Socket:
	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();
	bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));

	// Construct netlink packet:
	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(PAYLOAD_SIZE));
	memset(nlh, 0, NLMSG_SPACE(PAYLOAD_SIZE));
	nlh->nlmsg_len = NLMSG_SPACE(PAYLOAD_SIZE);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;

	// Edit packet payload:
	strcpy(NLMSG_DATA(nlh), "Hello World!");

	// Configure netlink destination:
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0;		// Linux Kernel PID
	dest_addr.nl_groups = 0;

	// Set iovector and msg (https://man7.org/linux/man-pages/man2/readv.2.html):
	struct iovec iov = { nlh, nlh->nlmsg_len };
	struct msghdr msg = { &dest_addr, sizeof(dest_addr), &iov, 1, NULL, 0, 0 };

	// Send msg to kernel:
	sendmsg(sock_fd, &msg, 0);
	printf("Message sent, payload: %s\n", (char *)NLMSG_DATA(nlh));

	// Clear netlink packet payload:
	memset(nlh, 0, NLMSG_SPACE(PAYLOAD_SIZE));

	// Receive msg from kernel:
	recvmsg(sock_fd, &msg, 0);
	printf("Message received, payload: %s\n", (char *)NLMSG_DATA(nlh));

	return 0;
}