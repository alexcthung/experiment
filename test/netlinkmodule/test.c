#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define MAX_PAYLOAD 1024 

int main(int argc,char *argv[])
{
	struct sockaddr_nl src_addr, dest_addr;
	struct nlmsghdr *nlh = NULL;
	struct iovec iov;
	int sock_fd;
	struct msghdr msg;
	
	sock_fd = socket(PF_NETLINK, SOCK_RAW,NETLINK_ROUTE);
	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	//src_addr.nl_pid = getpid();  /* self pid */
	printf("pid = %d\n",src_addr.nl_pid);
	src_addr.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;  /* not in mcast groups */
	bind(sock_fd, (struct sockaddr*)&src_addr,sizeof(src_addr));

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0;   /* For Linux Kernel */
	//dest_addr.nl_groups = 0; /* unicast */

	nlh=(struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	/* Fill the netlink message header */
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = 0;  /* self pid */
	nlh->nlmsg_flags |= NLM_F_ACK;
	/* Fill in the netlink message payload */
	//strcpy(NLMSG_DATA(nlh), "Hello World!");

	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	sendmsg(sock_fd, &msg, 0);
	
	printf("Waiting for message from kernel\n");

 	/* Read message from kernel */
 	recvmsg(sock_fd, &msg, 0);
 	
 	printf("Received message payload: %s\n",NLMSG_DATA(msg.msg_iov->iov_base));
	close(sock_fd);
	return 0;
}
