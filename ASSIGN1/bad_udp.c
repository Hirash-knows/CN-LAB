#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/ethernet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <linux/if_packet.h>
#include <net/if.h>

#define BUF_SIZE 1500

unsigned short checksum(unsigned short *buf, int len) {
    unsigned long sum = 0;
    while (len > 1) {
        sum += *buf++;
        len -= 2;
    }
    if (len) sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

int main() {
    int sock;
    unsigned char buffer[BUF_SIZE];

    /* Open RAW socket at Ethernet layer */
    sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock < 0) { perror("socket"); exit(1); }

    memset(buffer, 0, BUF_SIZE);

    /* -------- Ethernet Header -------- */
    struct ethhdr *eth = (struct ethhdr *)buffer;

    unsigned char src_mac[6] = {0x00,0x00,0x00,0x00,0x00,0x01}; // h1
    unsigned char dst_mac[6] = {0x00,0x00,0x00,0x00,0x00,0x02}; // h2

    memcpy(eth->h_source, src_mac, 6);
    memcpy(eth->h_dest, dst_mac, 6);
    eth->h_proto = htons(ETH_P_IP);

    /* -------- IP Header -------- */
    struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = htons(sizeof(struct iphdr) + sizeof(struct udphdr) + 128);
    ip->id = htons(100);
    ip->frag_off = 0;
    ip->ttl = 64;
    ip->protocol = IPPROTO_UDP;
    ip->saddr = inet_addr("10.0.0.1");
    ip->daddr = inet_addr("10.0.0.2");
    ip->check = checksum((unsigned short *)ip, sizeof(struct iphdr));

    /* -------- UDP Header -------- */
    struct udphdr *udp = (struct udphdr *)(buffer +
        sizeof(struct ethhdr) + sizeof(struct iphdr));

    udp->source = htons(4444);
    udp->dest   = htons(5555);
    udp->len    = htons(sizeof(struct udphdr) + 128);

    /* ❌ DELIBERATELY WRONG CHECKSUM */
    udp->check = htons(0x1234);

    /* -------- Payload -------- */
    char *payload = (char *)(buffer +
        sizeof(struct ethhdr) + sizeof(struct iphdr) + sizeof(struct udphdr));
  
    char user[64];
    getlogin_r(user, sizeof(user));

    time_t now = time(NULL);

    snprintf(payload, 128,
        "RollNo: CSM24027 | User: %s | Time: %s",
        user, ctime(&now));

    /* -------- Send Frame -------- */
    struct sockaddr_ll addr = {0};
    addr.sll_ifindex = if_nametoindex("h1-eth0");
    addr.sll_halen = ETH_ALEN;
    memcpy(addr.sll_addr, dst_mac, 6);

    sendto(sock, buffer,
        sizeof(struct ethhdr) + sizeof(struct iphdr) +
        sizeof(struct udphdr) + 128,
        0, (struct sockaddr *)&addr, sizeof(addr));

    printf("Forged UDP packet sent with INVALID checksum\n");

    close(sock);
    return 0;
}

