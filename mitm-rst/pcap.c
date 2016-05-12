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
 * Use pcap to filter out TCP traffic
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <err.h>
#include <errno.h>
#include <sys/types.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <pcap.h>

#ifndef	ETHERTYPE_8021Q
#define	ETHERTYPE_8021Q	0x8100
#endif

#if 0
typedef uint16_t u_int16_t;
#endif

/* Frame processing */
void pcap_callback(u_char *ptr, const struct pcap_pkthdr *hdr,
    const u_char *data)
{
	int len;
	struct ether_header *e_hdr = (struct ether_header *)data;
	int type;
	int pcap_if_type = *(int *)data;

	if (hdr->caplen != hdr->len)
		errx(1, "Length mismatch");

	len = hdr->len;

	switch (pcap_if_type) {
		case DLT_NULL:
			data += 4;
			len -= 4;
			break;
		case DLT_EN10MB:
			type = ntohs(e_hdr->ether_type);
			data += sizeof (struct ether_header);
			len -= sizeof (struct ether_header);

			/* VLANs */
			if (type == ETHERTYPE_8021Q) {
				type = ntohs(*(u_int16_t *)(data + 2));
				data += 4;
				len += 4;
			}

			if (type != ETHERTYPE_IP)
				return;

			break;
	}

	/* Process the packet here XXX */
}

int main(int argc, char *argv[])
{
	char *ifc_name;
	pcap_t *pd;
	char pcap_err[PCAP_ERRBUF_SIZE];
	bpf_u_int32 localnet, netmask;
	char *filter = NULL;
	unsigned int filter_len = 0;
	int i;
	struct bpf_program fp;
	int promisc = 0;
	int pcap_if_type = DLT_NULL;

	if (argc < 2)
		errx(1, "usage: %s <filter>", argv[0]);

	/* read the filter */
	for (i = 1; i < argc; i++)
		filter_len += strlen(argv[i]) + 1;
	if ((filter = calloc(filter_len, sizeof (char))) == NULL)
		err(1, "malloc");
	for (i = 1; i < argc; i++) {
		strncat(filter, argv[i], filter_len);
		strncat(filter, " ", filter_len);
	}

	if ((ifc_name = pcap_lookupdev(pcap_err)) == NULL)
		err(1, "lookupdev");
	printf("selected device: %s\n", ifc_name);

	if (!(pd = pcap_open_live(ifc_name,
	    5000, !promisc, 1000, pcap_err)))
		errx(1, "open_live: %s", pcap_err);

	if (pcap_lookupnet(ifc_name, &localnet, &netmask, pcap_err) < 0)
		errx(1, "lookupnet: %s", pcap_geterr(pd));

	printf("Compiling filter: '%s'\n", filter);
	if (pcap_compile(pd, &fp, filter, 0, netmask) < 0)
		errx(1, "compile: %s", pcap_geterr(pd));

	printf("Setting filter\n");
	if (pcap_setfilter(pd, &fp) < 0)
		errx(1, "setfilter: %s", pcap_geterr(pd));

	if ((pcap_if_type = pcap_datalink(pd)) == PCAP_ERROR_NOT_ACTIVATED)
		errx(1, "pcap_datalink: %s", pcap_geterr(pd));

	/* revoke privileges here XXX */

	/* enter reading loop */
	if (pcap_loop(pd, -1, pcap_callback, (u_char *)&pcap_if_type) < 0)
		errx(1, "pcap_loop failed: %s", pcap_geterr(pd));

	/* will call pcap_freecode() on the filter. */
	pcap_close(pd);

	return (0);
}
