#include <asm/types.h>
#include <netinet/ether.h>
#include <netinet/in.h>
#include <net/if.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 8192

struct route_info{
	u_int dstAddr;
	u_int srcAddr;
	u_int gateWay;
	char ifName[IF_NAMESIZE];
};

struct {
	struct nlmsghdr nl;
	struct ifinfomsg lk;
	char buf[BUFSIZE];
} req;

int readNlSock(int sockFd, char *bufPtr, int seqNum, int pId) {
	struct nlmsghdr *nlHdr;
	int readLen = 0, msgLen = 0;
	do {
		/* Receive response from the kernel */
		if((readLen = recv(sockFd, bufPtr, BUFSIZE - msgLen, 0)) < 0)
		{
			perror("SOCK READ: ");
			return -1;
		}
		nlHdr = (struct nlmsghdr *)bufPtr;
		/* Check if the header is valid */
		if((0 == NLMSG_OK(nlHdr, readLen)) || (NLMSG_ERROR == nlHdr->nlmsg_type))
		{
			perror("Error in received packet");
			return -1;
		}
		/* Check if it is the last message */
		if (NLMSG_DONE == nlHdr->nlmsg_type)
		{
			break;
		}
		/* Else move the pointer to buffer appropriately */
		bufPtr += readLen;
		msgLen += readLen;
		/* Check if its a multi part message; return if it is not. */
		if (0 == (nlHdr->nlmsg_flags & NLM_F_MULTI)) {
			break;
		}
	} while((nlHdr->nlmsg_seq != seqNum) || (nlHdr->nlmsg_pid != pId));
	return msgLen;
}

char *ntoa(int addr)
{
	static char buffer[18];
	sprintf(buffer, "%d.%d.%d.%d",
			(addr & 0x000000FF)      ,
			(addr & 0x0000FF00) >>  8,
			(addr & 0x00FF0000) >> 16,
			(addr & 0xFF000000) >> 24);
	return buffer;
}
/* For printing the routes. */
void printRoute(struct route_info *rtInfo)
{
	/* Print Destination address */
	printf("%s\t", rtInfo->dstAddr ? ntoa(rtInfo->dstAddr) : "0.0.0.0");

	/* Print Gateway address */
	printf("%s\t\t", rtInfo->gateWay ? ntoa(rtInfo->gateWay) : "*.*.*.*");

	/* Print Interface Name */
	printf("%s\t", rtInfo->ifName);

	/* Print Source address */
	printf("%s\n", rtInfo->srcAddr ? ntoa(rtInfo->srcAddr) : "*.*.*.*");

	if (0 == rtInfo->dstAddr) {
		printf("\t\t\t^^^^^ here it is!\n");
	}

}

/* For parsing the route info returned */
void parseRoutes(struct nlmsghdr *nlHdr, struct route_info *rtInfo)
{
	struct rtmsg *rtMsg;
	struct rtattr *rtAttr;
	int rtLen;
	char *tempBuf = NULL;

	tempBuf = (char *)malloc(100);
	rtMsg = (struct rtmsg *)NLMSG_DATA(nlHdr);

	/* If the route is not for AF_INET or does not belong to main routing table
	   then return. */
	if((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN))
		return;

	/* get the rtattr field */
	rtAttr = (struct rtattr *)RTM_RTA(rtMsg);
	rtLen = RTM_PAYLOAD(nlHdr);
	for (; RTA_OK(rtAttr,rtLen); rtAttr = RTA_NEXT(rtAttr,rtLen)) {
		switch(rtAttr->rta_type) {
			case RTA_OIF:
				if_indextoname(*(int *)RTA_DATA(rtAttr), rtInfo->ifName);
				//sprintf(rtInfo->ifName, "%d", *(int *)RTA_DATA(rtAttr));
				break;
			case RTA_GATEWAY:
				rtInfo->gateWay = *(u_int *)RTA_DATA(rtAttr);
				break;
			case RTA_PREFSRC:
				rtInfo->srcAddr = *(u_int *)RTA_DATA(rtAttr);
				break;
			case RTA_DST:
				rtInfo->dstAddr = *(u_int *)RTA_DATA(rtAttr);
				break;
		}
	}
	printRoute(rtInfo);
	free(tempBuf);
}

int main()
{
	struct nlmsghdr *nlMsg;
	struct route_info *rtInfo;
	char dsts[24] = "10.10.0.0";
	char src1[24] = "10.10.1.254";
	char src2[24] = "10.10.2.254";
	int ifcn = 7, pn = 16;
	int sock, len, msgSeq = 0;
	struct nlmsghdr *nlp;
	int nll;
	struct ifinfomsg *link;
	int rtl,i;
	struct rtattr *rtap;
	struct rtnexthop *rtnhp;
	
	/* Create Socket */
	if((sock = socket(PF_NETLINK, SOCK_RAW, NETLINK_ROUTE)) < 0)
		perror("Socket Creation: ");

	/* Initialize the buffer */
	bzero(&req, sizeof(req));

#if 0
	rtl = sizeof(struct rtmsg);

	// add first attrib:
	// set destination IP addr and increment the
	// RTNETLINK buffer size
	rtap = (struct rtattr *) req.buf;
	rtap->rta_type = RTA_DST;
	rtap->rta_len = sizeof(struct rtattr) + 4;
	inet_pton(AF_INET, dsts,
			((char *)rtap) + sizeof(struct rtattr));
	rtl += rtap->rta_len;
	
	//add second attrib multipath
	rtap = (struct rtattr *) (((char *)rtap)
			+ rtap->rta_len);
	rtap->rta_type = RTA_MULTIPATH;
	rtap->rta_len = sizeof(struct rtattr) + (sizeof(struct rtnexthop) + sizeof(struct rtattr) + 4 ) * 2 ;
	rtl += rtap->rta_len;
	
	//add first nexthop
	rtnhp = (struct rtnexthop *) (((char *)rtap) + sizeof(struct rtattr));
	rtnhp->rtnh_len = sizeof(struct rtnexthop) + sizeof(struct rtattr) + 4;
	strcpy(&rtnhp->rtnh_hops,"10");
	rtnhp->rtnh_ifindex = ifcn;
	
	//first nexthop payload
	rtap = RTNH_DATA(rtnhp);
	rtap->rta_type = RTA_GATEWAY;
	rtap->rta_len = sizeof(struct rtattr) + 4;
	inet_pton(AF_INET, "10.10.1.254", RTA_DATA(rtap));
#endif
#if 0
	//add second nexthop
	rtnhp = (struct rtnexthop *) (((char *)rtap) + RTA_SPACE(RTA_PAYLOAD(rtap)));
	rtnhp->rtnh_len = sizeof(struct rtnexthop) + sizeof(struct rtattr) + 4;
	strcpy(&rtnhp->rtnh_hops,"3");
	rtnhp->rtnh_ifindex = ifcn;
	
	//second nexthop payload
	rtap = RTNH_DATA(rtnhp);
	rtap->rta_type = RTA_GATEWAY;
	rtap->rta_len = sizeof(struct rtattr) + 4;
	inet_pton(AF_INET, "10.10.2.254", RTA_DATA(rtap));
#endif
#if 0
	rtap = (struct rtattr *) (((char *)rtap)
			+ rtap->rta_len);
	rtap->rta_type = RTA_METRICS;
	rtap->rta_len = sizeof(struct rtattr) + (sizeof(struct rtnexthop) + sizeof(struct rtattr) + 4 ) * 2 ;
	rtl += rtap->rta_len;

	for (i = 1; i <= RTAX_MAX; i++) {
		

#endif
#if 0
	// add second attrib:
	// set ifc index and increment the size
	rtap = (struct rtattr *) (((char *)rtap)
			+ rtap->rta_len);
	rtap->rta_type = RTA_OIF;
	rtap->rta_len = sizeof(struct rtattr) + 4;
	memcpy(((char *)rtap) + sizeof(struct rtattr),
			&ifcn, 4);
	rtl += rtap->rta_len;
#endif
#if 0
	// add third attrib:
	// set src addr and increment the size
	rtap = (struct rtattr *) (((char *)rtap)
			+ rtap->rta_len);
	rtap->rta_type = RTA_GATEWAY;
	rtap->rta_len = sizeof(struct rtattr) + 4;
	inet_pton(AF_INET, "10.10.10.1",
			((char *)rtap) + sizeof(struct rtattr));
	rtl += rtap->rta_len;
#endif
	// setup the NETLINK header
	rtl = sizeof(struct nlmsghdr) + sizeof(struct ifinfomsg);
	
	req.nl.nlmsg_len = NLMSG_LENGTH(rtl);
	req.nl.nlmsg_flags = NLM_F_REQUEST | NLM_F_CREATE;
	req.nl.nlmsg_type = RTM_SETLINK;

	// setup the service header (struct rtmsg)
	req.lk.ifi_family = AF_UNSPEC;
	req.lk.ifi_index = 7;
	req.lk.ifi_flags &= !IFF_UP;
	req.lk.ifi_change = 0xFFFFFFFF;

	/* Send the request */
	if(send(sock, &req.nl, req.nl.nlmsg_len, 0) < 0){
		printf("Write To Socket Failed...\n");
		return -1;
	}

	close(sock);
	return 0;
}
