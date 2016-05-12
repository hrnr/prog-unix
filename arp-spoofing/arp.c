/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * Copyright 2009 Jan Pechanec, Vladimir Kotal.  All rights reserved.
 * Use is subject to license terms.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <err.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <time.h>
#include <errno.h>
#include <linux/if_packet.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <arpa/inet.h>
#include <linux/if_arp.h>


struct arp_ether {
	struct arphdr ar_hdr;
	unsigned char ar_sha[ETH_ALEN]; /* sender hardware address */
	unsigned char ar_sip[4]; /* sender IP address */
	unsigned char ar_tha[ETH_ALEN]; /* target hardware address */
	unsigned char ar_tip[4]; /* target IP address */
} __attribute__ ((packed));

int
main(int argc, char *argv[]) {
	int sock, bytes_sent;
	struct sockaddr_ll dst;
	struct arp_ether payload;
	(void) argc;
	(void) argv;

	if ((sock = socket(AF_PACKET, SOCK_DGRAM, htons(ETH_P_ARP))) < 0) {
		err(1, "socket");
		exit(-2);
	}

	if (setuid(getuid())) {
		perror("setuid");
		exit(-1);
	}

	/* set destination address */
	memset(&dst, 0, sizeof(dst));
	dst.sll_family = AF_PACKET;
	dst.sll_protocol = htons(ETH_P_ARP);
	dst.sll_ifindex = 2;
	dst.sll_pkttype = PACKET_BROADCAST;
	dst.sll_halen = 6 /* size of ethernet addr*/;
	memset(&dst.sll_addr, 0xff, 6);

	/* set ARP payload */
	memset(&payload, 0, sizeof(payload));
	payload.ar_hdr.ar_hrd = htons(ARPHRD_ETHER) /*Ethernet*/;
	payload.ar_hdr.ar_pro = htons(ETH_P_IP);
	payload.ar_hdr.ar_hln = ETH_ALEN;
	payload.ar_hdr.ar_pln = 4 /* IPv4 */;
	payload.ar_hdr.ar_op = htons(ARPOP_REPLY);

	payload.ar_sha[0] = 0xde;
	payload.ar_sha[1] = 0xad;
	payload.ar_sha[2] = 0xba;
	payload.ar_sha[3] = 0xbe;
	payload.ar_sha[4] = 0xad;
	payload.ar_sha[5] = 0xda;

	payload.ar_sip[0] = 10;
	payload.ar_sip[1] = 0;
	payload.ar_sip[2] = 0;
	payload.ar_sip[3] = 1;
	// payload.ar_tha =
	// payload.ar_tip =

	bytes_sent = sendto(sock, &payload, sizeof(payload), 0,
		(struct sockaddr *)&dst, sizeof(dst));
	if (bytes_sent !=  sizeof(payload))
		err(1, "sendto");

	return (0);
}
