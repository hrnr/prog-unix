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

/*
 * Construct ICMP packet with arbitrary type and code.
 *
 * NOTE: this code contains several mistakes. Fix them so that it produces
 *	 valid ICMP packet and then convert the program so that it can send
 *	 both ICMP and ICMPv6 packets.
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

#define	ICMP_ECHO_SIZE (sizeof (struct icmp) + 36)

unsigned short
ipv4_checksum(unsigned short *buf, int nwords) {
	unsigned long sum;

	for (sum = 0; nwords > 0; nwords--)
		sum += *buf++;

	sum = (sum >> 16) + (sum & 0xffff);
	sum += (sum >> 16);

	/* return 1-bit complement */
	return (~sum);
}

void timespec_diff(const struct timespec *start, const struct timespec *stop,
				   struct timespec *result)
{
	if ((stop->tv_nsec - start->tv_nsec) < 0) {
		result->tv_sec = stop->tv_sec - start->tv_sec - 1;
		result->tv_nsec = stop->tv_nsec - start->tv_nsec + 1000000000;
	} else {
		result->tv_sec = stop->tv_sec - start->tv_sec;
		result->tv_nsec = stop->tv_nsec - start->tv_nsec;
	}

	return;
}

int
main(int argc, char *argv[]) {
	int sock, bytes_sent;
	struct sockaddr_in dst;
	struct icmp *icmp_hdr;
	struct timespec send_time, recv_time, roundtrip_time;

	if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0) {
		err(1, "socket");
		exit(-2);
	}

	if (setuid(getuid())) {
		perror("setuid");
		exit(-1);
	}

	/*
	 * NOTE: ICMP_MINLEN is minimum length of ICMP header in bytes
	 *	 to be filled (8 bytes). See RFC 792 for details.
	 * NOTE: on some platforms structs must be aligned, ideally on
	 *	 4-byte boundary. By declaring the array for packet buffer
	 *	 as array of uint32_t (and its size rounded to be divisible
	 *	 by sizeof (uint32_t)) it will be aligned to sizeof (uint32_t)
	 *	 boundary which is 4 bytes.
	 */
	uint32_t buffer[ICMP_ECHO_SIZE / sizeof (uint32_t)];

	if (argc != 4)
		errx(1, "usage: %s <address> <type> <code>", argv[0]);

	/* set the socket to no-delay */
	/*if (fcntl(sock, F_SETFL, O_NDELAY) == -1)
		err(1, "fcntl");*/

	/* Fill ICMP header (1 packet only) */
	bzero((void *)buffer, sizeof (buffer));
	icmp_hdr = (struct icmp *)buffer;
	icmp_hdr->icmp_type = atoi(argv[2]);
	icmp_hdr->icmp_code = atoi(argv[3]);
	icmp_hdr->icmp_id = 1;
	icmp_hdr->icmp_seq = 0;

	/* ICMP data */
	buffer[ICMP_MINLEN] = 0x12345678;

	/* compute checksum */
	icmp_hdr->icmp_cksum = ipv4_checksum((uint16_t *)buffer,
		ICMP_ECHO_SIZE / sizeof (uint16_t));

	/* set destination address */
	(void) memset((void *)&dst, 0, sizeof (struct sockaddr_in));
	dst.sin_family = AF_INET;
	if (inet_pton(dst.sin_family, argv[1], &dst.sin_addr) <= 0)
		err(1, "inet_pton");

	if(clock_gettime(CLOCK_REALTIME, &send_time) != 0)
		err(1, "clock_gettime");

	bytes_sent = sendto(sock, (char *)buffer, ICMP_ECHO_SIZE, 0,
		(struct sockaddr *)&dst, sizeof (struct sockaddr));
	if (bytes_sent !=  ICMP_ECHO_SIZE)
		err(1, "sendto");

	bytes_sent = recvfrom(sock, buffer, ICMP_ECHO_SIZE, 0,
		NULL, NULL);
	if (bytes_sent !=  ICMP_ECHO_SIZE) {
		printf("errno was: %d\n", errno);
		err(1, "recvfrom");
	}

	if(clock_gettime(CLOCK_REALTIME, &recv_time) != 0)
		err(1, "clock_gettime");

	timespec_diff(&send_time, &recv_time, &roundtrip_time);
	float roundtrip_sec = roundtrip_time.tv_sec + roundtrip_time.tv_nsec * 1e-9;
	float roundtrip_msec = roundtrip_sec * 1e3;

	printf("received echo reply id: %d, roudtrip time: %.3f ms\n",
		(int)icmp_hdr->icmp_id, roundtrip_msec);

	return (0);
}
