#ifndef __NETBSD_NETWORK_H__
#define __NETBSD_NETWORK_H__

#include <stdint.h>
#include "netbsd_compat.h"

/* NetBSD Network Driver Compatibility */

/* Network interface flags */
#define IFF_UP          0x0001
#define IFF_BROADCAST   0x0002
#define IFF_DEBUG       0x0004
#define IFF_LOOPBACK    0x0008
#define IFF_POINTOPOINT 0x0010
#define IFF_RUNNING     0x0040
#define IFF_PROMISC     0x0100
#define IFF_ALLMULTI    0x0200

/* Network interface structure */
struct ifnet {
    const char *if_name;           /* Interface name (e.g., "em0") */
    int if_unit;                   /* Unit number */
    int if_flags;                  /* Flags (IFF_*) */
    int if_metric;                 /* Routing metric */
    uint32_t if_mtu;               /* Maximum transmission unit */
    uint32_t if_hwassist;          /* Hardware offload capabilities */
    uint8_t if_hwaddr[6];          /* Hardware MAC address */
    uint32_t if_ipackets;          /* Packets received */
    uint32_t if_opackets;          /* Packets sent */
    uint32_t if_ierrors;           /* Input errors */
    uint32_t if_oerrors;           /* Output errors */
    void *if_softc;                /* Driver-specific data */
    int (*if_init)(struct ifnet *);     /* Initialization function */
    int (*if_ioctl)(struct ifnet *, int, void *);  /* I/O control */
    int (*if_start)(struct ifnet *);    /* Start transmission */
    int (*if_stop)(struct ifnet *, int); /* Stop device */
};

typedef struct ifnet ifnet_t;

/* Network packet buffer structure */
struct mbuf {
    struct mbuf *m_next;           /* Next packet in chain */
    struct mbuf *m_nextpkt;        /* Next packet in list */
    uint8_t *m_data;               /* Pointer to data */
    uint32_t m_len;                /* Amount of data */
    uint32_t m_flags;              /* Flags */
    uint16_t m_type;               /* Type of data */
};

typedef struct mbuf mbuf_t;

/* mbuf flags */
#define M_PKTHDR    0x0001
#define M_EXT       0x0002
#define M_PROTO1    0x0004

/* mbuf types */
#define MT_DATA     1
#define MT_HEADER   2
#define MT_SOCKET   3

/* Ethernet frame header */
struct ether_header {
    uint8_t ether_dhost[6];        /* Destination MAC */
    uint8_t ether_shost[6];        /* Source MAC */
    uint16_t ether_type;           /* Frame type */
} __attribute__((packed));

/* Ethernet type constants */
#define ETHERTYPE_IP    0x0800
#define ETHERTYPE_ARP   0x0806
#define ETHERTYPE_IPV6  0x86dd

/* Network interface functions */
int if_attach(ifnet_t *ifp);
int if_detach(ifnet_t *ifp);
int if_up(ifnet_t *ifp);
int if_down(ifnet_t *ifp);
int if_addmulti(ifnet_t *ifp, const uint8_t *addr);
int if_delmulti(ifnet_t *ifp, const uint8_t *addr);
void if_input(ifnet_t *ifp, mbuf_t *m);
int if_output(ifnet_t *ifp, mbuf_t *m);
void if_start(ifnet_t *ifp);

/* mbuf allocation and management */
mbuf_t *m_get(uint32_t how, uint16_t type);
mbuf_t *m_gethdr(uint32_t how, uint16_t type);
void m_freem(mbuf_t *m);
int m_append(mbuf_t *m, uint32_t len, void *cp);
mbuf_t *m_prepend(mbuf_t *m, uint32_t len, uint32_t how);

/* Packet transmission/reception */
int if_enqueue(ifnet_t *ifp, mbuf_t *m);
mbuf_t *if_dequeue(ifnet_t *ifp);

#endif /* __NETBSD_NETWORK_H__ */
