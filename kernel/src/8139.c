#include <stddef.h>
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
#define LWIP_IPV6 0
#include <logging.h>
#include <system.h>
 
#include <lwip/sys.h>
#include <lwip/stats.h>
#include <lwip/netif.h>
#include <lwip/tcpip.h>
#include <lwip/snmp.h>
#include <lwip/ethip6.h>
#include <netif/etharp.h>
#include "8139too.h"
#define LWIP_IPV6 0
typedef struct {
	uint32_t base[6];
	uint32_t size[6];
	uint32_t irq;
} pci_info_t;
#define PCI_IGNORE_SUBID	(0)
#define RX_BUF_LEN 	81921
#define TX_BUF_LEN	40961
#define MIN(a, b)	(a) < (b) ? (a) : (b)

static uint8_t rx_buffer[RX_BUF_LEN+16 /* header size */] __attribute__ ((aligned (0x1000)));
static uint8_t tx_buffer[4][TX_BUF_LEN] __attribute__ ((aligned (0x1000)));

/*
 * To set the RTL8139 to accept only the Transmit OK (TOK) and Receive OK (ROK)
 * interrupts, we would have the TOK and ROK bits of the IMR high and leave the
 * rest low. That way when a TOK or ROK IRQ happens, it actually will go through
 * and fire up an IRQ.
 */
#define INT_MASK	(ISR_ROK|ISR_TOK|ISR_RXOVW|ISR_TER|ISR_RER)

// Beside Receive OK (ROK) interrupt, this mask enable all other interrupts
#define INT_MASK_NO_ROK	(ISR_TOK|ISR_RXOVW|ISR_TER|ISR_RER)

typedef struct {
	char *vendor_str;
	char *device_str;
	uint32_t vendor;
	uint32_t device;
} board_t;

static board_t board_tbl[] =
{
	{"RealTek", "RealTek RTL8139", 0x10ec, 0x8139},
	{"RealTek", "RealTek RTL8129 Fast Ethernet", 0x10ec, 0x8129},
	{"RealTek", "RealTek RTL8139B PCI",  0x10ec, 0x8138},
	{"SMC", "SMC1211TX EZCard 10/100 (RealTek RTL8139)", 0x1113, 0x1211},
	{"D-Link", "D-Link DFE-538TX (RTL8139)", 0x1186, 0x1300},
	{"LevelOne", "LevelOne FPC-0106Tx (RTL8139)", 0x018a, 0x0106},
	{"Compaq", "Compaq HNE-300 (RTL8139c)", 0x021b, 0x8139},
	{NULL,},
};

static struct netif* mynetif = NULL;
#define BUILTIN_EXPECT(exp, b) __builtin_expect((exp), (b))
/*
 * @return error code
 * - ERR_OK: packet transferred to hardware
 * - ERR_CONN: no link or link failure
 * - ERR_IF: could not transfer to link (hardware buffer full?)
 */
static err_t rtl8139if_output(struct netif* netif, struct pbuf* p)
{
	rtl1839if_t* rtl8139if = netif->state;
	uint8_t transmitid = rtl8139if->tx_queue % 4;
	uint32_t i;
	struct pbuf *q;

	if (BUILTIN_EXPECT((rtl8139if->tx_queue - rtl8139if->tx_complete) > 3, 0)) {
		debug_print(NOTICE,"rtl8139if_output: too many packets at once\n");
		return ERR_IF;
	}

	if (BUILTIN_EXPECT(p->tot_len > 1792, 0)) {
		debug_print(NOTICE,"rtl8139if_output: packet is longer than 1792 bytes\n");
		return ERR_IF;
	}

	if (rtl8139if->tx_inuse[transmitid] == 1) {
		debug_print(NOTICE,"rtl8139if_output: %i already inuse\n", transmitid);
		return ERR_IF;
	}

	if (inportb(rtl8139if->iobase + MSR) & MSR_LINKB) {
		debug_print(NOTICE,"rtl8139if_output: link failure\n");
		return ERR_CONN;
	}

	rtl8139if->tx_queue++;
	rtl8139if->tx_inuse[transmitid] = 1;

#if ETH_PAD_SIZE
	pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

	/*
	 * q traverses through linked list of pbuf's
	 * This list MUST consist of a single packet ONLY
	 */
	for (q = p, i = 0; q != 0; q = q->next) {
		memcpy(rtl8139if->tx_buffer[transmitid] + i, q->payload, q->len);
		i += q->len;
	}

	// send the packet
	outportl(rtl8139if->iobase + TSD0 + (4 * transmitid), p->tot_len); //|0x3A0000);

#if ETH_PAD_SIZE
	pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

	LINK_STATS_INC(link.xmit);

	return ERR_OK;
}

static void rtl_rx_inthandler(struct netif* netif)
{
	rtl1839if_t* rtl8139if = netif->state;
	uint16_t header;
	uint16_t length, i;
	uint8_t cmd;
	struct pbuf *p = NULL;
	struct pbuf* q;

	cmd = inportb(rtl8139if->iobase + CR);
	while(!(cmd & CR_BUFE)) {
		header = *((uint16_t*) (rtl8139if->rx_buffer+rtl8139if->rx_pos));
		rtl8139if->rx_pos = (rtl8139if->rx_pos + 2) % RX_BUF_LEN;

		if (header & ISR_ROK) {
			length = *((uint16_t*) (rtl8139if->rx_buffer+rtl8139if->rx_pos)) - 4; // copy packet (but not the CRC)
			rtl8139if->rx_pos = (rtl8139if->rx_pos + 2) % RX_BUF_LEN;
#if ETH_PAD_SIZE
			length += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

			p = pbuf_alloc(PBUF_RAW, length, PBUF_POOL);
			if (p) {
#if ETH_PAD_SIZE
				pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif
				for (q=p; q!=NULL; q=q->next) {
					i = MIN(q->len, RX_BUF_LEN - rtl8139if->rx_pos);
					memcpy((uint8_t*) q->payload, rtl8139if->rx_buffer + rtl8139if->rx_pos, i);
					if (i < q->len) // wrap around to end of RxBuffer
						memcpy((uint8_t*) q->payload + i, rtl8139if->rx_buffer, q->len - i);
					rtl8139if->rx_pos = (rtl8139if->rx_pos + q->len) % RX_BUF_LEN;
				}
#if ETH_PAD_SIZE
				pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif
				LINK_STATS_INC(link.recv);

				// forward packet to LwIP
				netif->input(p, netif);
			} else {
				debug_print(NOTICE,"rtl8139if_rx_inthandler: not enough memory!\n");
				rtl8139if->rx_pos += (rtl8139if->rx_pos + length) % RX_BUF_LEN;
				LINK_STATS_INC(link.memerr);
				LINK_STATS_INC(link.drop);
			}

			// packets are dword aligned
			rtl8139if->rx_pos = ((rtl8139if->rx_pos + 4 + 3) & ~0x3) % RX_BUF_LEN;
			outportw(rtl8139if->iobase + CAPR, rtl8139if->rx_pos - 0x10);
		} else {
			debug_print(NOTICE,"rtl8139if_rx_inthandler: invalid header 0x%x, rx_pos %d\n", (uint32_t) header, rtl8139if->rx_pos);
			LINK_STATS_INC(link.drop);
			break;
		}

		cmd = inportb(rtl8139if->iobase + CR);
	}

	rtl8139if->polling = 0;
	// enable all known interrupts
	outportw(rtl8139if->iobase + IMR, INT_MASK);
}

static void rtl_tx_inthandler(struct netif* netif)
{
	rtl1839if_t* rtl8139if = netif->state;
	uint32_t checks = rtl8139if->tx_queue - rtl8139if->tx_complete;
	uint32_t txstatus;
	uint8_t tmp8;

	while(checks > 0)
	{
		tmp8 = rtl8139if->tx_complete % 4;
		txstatus = inportl(rtl8139if->iobase + TSD0 + tmp8 * 4);

		if (!(txstatus & (TSD_TOK|TSD_TUN|TSD_TABT)))
			return;

		if (txstatus & (TSD_TABT | TSD_OWC)) {
			debug_print(NOTICE,"rtl8139_tx_inthandler: major error\n");
			continue;
		}

		if (txstatus & TSD_TUN) {
			debug_print(NOTICE,"rtl8139_tx_inthandler: transmit underrun\n");
		}

		if (txstatus & TSD_TOK) {
			rtl8139if->tx_inuse[tmp8] = 0;
			rtl8139if->tx_complete++;
			checks--;
		}
	}
}

/* this function is called in the context of the tcpip thread or the irq handler (by using NO_SYS) */
  void rtl8139if_poll(void* ctx)
{
debug_print(NOTICE,"[rtl8139if_poll]\n");
	rtl_rx_inthandler(mynetif);
}

  static int rtl8139if_handler(struct regs *r)
{
debug_print(NOTICE,"[rtl8139if_handler]: GOT A PACKET!\n");
 
	rtl1839if_t* rtl8139if = mynetif->state;
	uint16_t isr_contents;

	// disable all interrupts
	outportw(rtl8139if->iobase + IMR, 0x00);

	while (1) {
		isr_contents = inportw(rtl8139if->iobase + ISR);
		if (isr_contents == 0)
			break;

		if ((isr_contents & ISR_ROK) && !rtl8139if->polling) {
#if NO_SYS
			rtl8139if_poll(NULL);
#else
			if (tcpip_callback_with_block(rtl8139if_poll, NULL, 0) == ERR_OK) {
				rtl8139if->polling = 1;
			} else {
				debug_print(NOTICE,"rtl8139if_handler: unable to send a poll request to the tcpip thread\n");
			}
#endif
 		}

		if (isr_contents & ISR_TOK)
			rtl_tx_inthandler(mynetif);

		if (isr_contents & ISR_RER) {
			debug_print(NOTICE,"rtl8139if_handler: RX error detected!\n");
		}

		if (isr_contents & ISR_TER) {
			debug_print(NOTICE,"rtl8139if_handler: TX error detected!\n");
		}

		if (isr_contents & ISR_RXOVW) {
			debug_print(NOTICE,"rtl8139if_handler: RX overflow detected!\n");
		}

		outportw(rtl8139if->iobase + ISR, isr_contents & (ISR_RXOVW|ISR_TER|ISR_RER|ISR_TOK|ISR_ROK));
	}

	if (rtl8139if->polling) // now, the tcpip thread will check for incoming messages
		outportw(rtl8139if->iobase + IMR, INT_MASK_NO_ROK);
	else
		outportw(rtl8139if->iobase + IMR, INT_MASK); // enable interrupts
}
int wait_loop0 = 10000;
int wait_loop1 = 6000;

// for microprocessor without timer, if it has a timer refer to vendor documentation and use it instead.
void
udelay( int seconds )
{   // this function needs to be finetuned for the specific microprocessor
    int i, j, k;
    for(i = 0; i < seconds; i++)
    {
        for(j = 0; j < wait_loop0; j++)
        {
            for(k = 0; k < wait_loop1; k++)
            {   // waste function, volatile makes sure it is not being optimized out by compiler
                int volatile t = 120 * j * i + k;
                t = t + 5;
            }
        }
    }
}
#define PCI_INTERRUPT_LINE 0x3c
static void find_rtl(uint32_t device, uint16_t vendorid, uint16_t deviceid, void * extra) {
	if ((vendorid == 0x10ec) && (deviceid == 0x8139)) {
		*((uint32_t *)extra) = device;
	}
}
#define PCI_VENDOR_ID            0x00 // 2
#define PCI_DEVICE_ID            0x02 // 2
#define PCI_COMMAND              0x04 // 2
#define PCI_STATUS               0x06 // 2
#define PCI_REVISION_ID          0x08 // 1

#define PCI_PROG_IF              0x09 // 1
#define PCI_SUBCLASS             0x0a // 1
#define PCI_CLASS                0x0b // 1
#define PCI_CACHE_LINE_SIZE      0x0c // 1
#define PCI_LATENCY_TIMER        0x0d // 1
#define PCI_HEADER_TYPE          0x0e // 1
#define PCI_BIST                 0x0f // 1
#define PCI_BAR0                 0x10 // 4
#define PCI_BAR1                 0x14 // 4
#define PCI_BAR2                 0x18 // 4
#define PCI_BAR3                 0x1C // 4
#define PCI_BAR4                 0x20 // 4
#define PCI_BAR5                 0x24 // 4

#define PCI_INTERRUPT_LINE       0x3C // 1

#define PCI_SECONDARY_BUS        0x19 // 1

#define PCI_HEADER_TYPE_DEVICE  0
#define PCI_HEADER_TYPE_BRIDGE  1
#define PCI_HEADER_TYPE_CARDBUS 2

#define PCI_TYPE_BRIDGE 0x0604
#define PCI_TYPE_SATA   0x0106

#define PCI_ADDRESS_PORT 0xCF8
#define PCI_VALUE_PORT   0xCFC

#define PCI_NONE 0xFFFF
err_t rtl8139if_init(struct netif* netif)
{

	rtl1839if_t* rtl8139if;
	uint32_t tmp32;
	uint16_t tmp16, speed;
	uint8_t tmp8;
	static uint8_t num = 0;
	pci_info_t pci_info;

	LWIP_ASSERT("netif != NULL", (netif != NULL));

	tmp8 = 0;
	while (board_tbl[tmp8].vendor_str) {
		if (pci_get_device_info(board_tbl[tmp8].vendor, board_tbl[tmp8].device, PCI_IGNORE_SUBID, &pci_info, 1) == 0)
			break;
		tmp8++;
	}

	if (!board_tbl[tmp8].vendor_str)
		return ERR_ARG;

	debug_print(NOTICE,"Found %s %s\n", board_tbl[tmp8].vendor_str, board_tbl[tmp8].device_str);

	rtl8139if =  malloc(sizeof(rtl1839if_t));
	if (!rtl8139if) {
		debug_print(NOTICE,"rtl8139if_init: out of memory\n");
		return ERR_MEM;
	}
	debug_print(NOTICE,"Found %s %s\n", board_tbl[tmp8].vendor_str, board_tbl[tmp8].device_str);
	memset(rtl8139if, 0x00, sizeof(rtl1839if_t));

	rtl8139if->iobase = pci_info.base[0];
	rtl8139if->irq = pci_info.irq;

	/* allocate the receive buffer */
	rtl8139if->rx_buffer = rx_buffer;
	//memset(rtl8139if->rx_buffer, 0x00, RX_BUF_LEN + 16);

	/* allocate the send buffers */
	rtl8139if->tx_buffer[0] = tx_buffer[0];
	//memset(rtl8139if->tx_buffer[0], 0x00, 4*TX_BUF_LEN);
	rtl8139if->tx_buffer[1] = tx_buffer[1];
	rtl8139if->tx_buffer[2] = tx_buffer[2];
	rtl8139if->tx_buffer[3] = tx_buffer[3];

	netif->state = rtl8139if;
	mynetif = netif;
	debug_print(NOTICE,"Found %s %s\n", board_tbl[tmp8].vendor_str, board_tbl[tmp8].device_str);
	tmp32 = inportl(rtl8139if->iobase + TCR);
	if (tmp32 == 0xFFFFFF) {
		debug_print(NOTICE,"rtl8139if_init: ERROR\n");
		 free(rtl8139if);
		memset(netif, 0x00, sizeof(struct netif));
		mynetif = NULL;

		return ERR_ARG;
	}

	debug_print(NOTICE,"Found %s %s\n", board_tbl[tmp8].vendor_str, board_tbl[tmp8].device_str);
	// determine the hardware revision
	//tmp32 = (tmp32 & TCR_HWVERID) >> TCR_HWOFFSET;
	static uint32_t rtl_device_pci = 0x00000000;
	pci_scan(&find_rtl, -1, &rtl_device_pci);
	if (!rtl_device_pci) {
		debug_print(ERROR, "No RTL 8139 found?");	 
	}
	int rtl_irq = pci_read_field(rtl_device_pci, PCI_INTERRUPT_LINE, 1);
	debug_print(NOTICE, "Interrupt Line: %x\n", rtl_irq);

	/*debug_print(NOTICE, "Located an RTL 8139: 0x%x\n", rtl_device_pci);

		uint16_t command_reg = pci_read_field(rtl_device_pci, PCI_COMMAND, 4);
		debug_print(NOTICE, "COMMAND register before: 0x%4x\n", command_reg);
		if (command_reg & (1 << 2)) {
			debug_print(NOTICE, "Bus mastering already enabled.\n");
		} else {
			command_reg |= (1 << 2); /* bit 2 */
			/*debug_print(NOTICE, "COMMAND register after:  0x%4x\n", command_reg);
			pci_write_field(rtl_device_pci, PCI_COMMAND, 4, command_reg);
			command_reg = pci_read_field(rtl_device_pci, PCI_COMMAND, 4);
			debug_print(NOTICE, "COMMAND register after:  0x%4x\n", command_reg);
		}

		rtl_irq = pci_read_field(rtl_device_pci, PCI_INTERRUPT_LINE, 1);
		debug_print(NOTICE, "Interrupt Line: %x\n", rtl_irq);
		irq_install_handler(rtl_irq, rtl8139if_handler);*/



	 install_device(rtl_irq, rtl8139if_handler);

	debug_print(NOTICE,"Found %s %s\n", board_tbl[tmp8].vendor_str, board_tbl[tmp8].device_str);
	/* hardware address length */
	netif->hwaddr_len = ETHARP_HWADDR_LEN;

	debug_print(NOTICE,"rtl8139if_init: Found %s at iobase 0x%x (irq %d)\n", board_tbl[tmp8].device_str,
	    rtl8139if->iobase, rtl8139if->irq);
	// determine the mac address of this card
	LWIP_DEBUGF(NETIF_DEBUG, ("rtl8139if_init: MAC address "));
	for (tmp8=0; tmp8<ETHARP_HWADDR_LEN; tmp8++) {
		netif->hwaddr[tmp8] = inportb(rtl8139if->iobase + IDR0 + tmp8);
		LWIP_DEBUGF(NETIF_DEBUG, ("%02x ", netif->hwaddr[tmp8]));
	}
	LWIP_DEBUGF(NETIF_DEBUG, ("\n"));

	rtl8139if->ethaddr = (struct eth_addr *) netif->hwaddr;

	// Software reset
	outportb(rtl8139if->iobase + CR, CR_RST);

	/*
	 * The RST bit must be checked to make sure that the chip has finished the reset.
	 * If the RST bit is high (1), then the reset is still in operation.
	 */
	//udelay(1);
	tmp16 = 10000;
	while ((inportb(rtl8139if->iobase + CR) & CR_RST) && tmp16 > 0) {
		tmp16--;
	}

	if (!tmp16) {
		// it seems not to work
		debug_print(NOTICE,"RTL8139 reset failed\n");
		 free(rtl8139if);
		memset(netif, 0x00, sizeof(struct netif));
		mynetif = NULL;

		return ERR_ARG;
	}

	// Enable Receive and Transmitter
	outportb(rtl8139if->iobase + CR, CR_TE|CR_RE); // Sets the RE and TE bits high

	// lock config register
	outportb(rtl8139if->iobase + CR9346, CR9346_EEM1 | CR9346_EEM0);

	// clear all of CONFIG1
	outportb(rtl8139if->iobase + CONFIG1, 0);

	// disable driver loaded and lanwake bits, turn driver loaded bit back on
	outportb(rtl8139if->iobase + CONFIG1,
		(inportb(rtl8139if->iobase + CONFIG1) & ~(CONFIG1_DVRLOAD | CONFIG1_LWACT)) | CONFIG1_DVRLOAD);

	// unlock config register
	outportb(rtl8139if->iobase + CR9346, 0);

	/*
	 * configure receive buffer
	 * AB - Accept Broadcast: Accept broadcast packets sent to mac ff:ff:ff:ff:ff:ff
	 * AM - Accept Multicast: Accept multicast packets.
	 * APM - Accept Physical Match: Accept packets send to NIC's MAC address.
	 * AAP - Accept All Packets. Accept all packets (run in promiscuous mode).
	 */
	outportl(rtl8139if->iobase + RCR, RCR_MXDMA2|RCR_MXDMA1|RCR_MXDMA0|RCR_AB|RCR_AM|RCR_APM|RCR_AAP); // The WRAP bit isn't set!

	// set the transmit config register to
	// be the normal interframe gap time
	// set DMA max burst to 64bytes
 	outportl(rtl8139if->iobase + TCR, TCR_IFG|TCR_MXDMA0|TCR_MXDMA1|TCR_MXDMA2);

	// register the receive buffer
	outportl(rtl8139if->iobase + RBSTART, map_to_physical((size_t) rtl8139if->rx_buffer));

	// set each of the transmitter start address descriptors
	outportl(rtl8139if->iobase + TSAD0, map_to_physical((size_t) rtl8139if->tx_buffer[0]));
	outportl(rtl8139if->iobase + TSAD1, map_to_physical((size_t) rtl8139if->tx_buffer[1]));
	outportl(rtl8139if->iobase + TSAD2, map_to_physical((size_t) rtl8139if->tx_buffer[2]));
	outportl(rtl8139if->iobase + TSAD3, map_to_physical((size_t) rtl8139if->tx_buffer[3]));

	// Enable all known interrupts by setting the interrupt mask.
	outportw(rtl8139if->iobase + IMR, INT_MASK);

	outportw(rtl8139if->iobase + BMCR, BMCR_ANE);
	tmp16 = inportw(rtl8139if->iobase + BMCR);
	if (tmp16 & BMCR_SPD1000)
		speed = 1000;
	else if (tmp16 & BMCR_SPD100)
		speed = 100;
	else
		speed = 10;
	// Enable Receive and Transmitter
	outportb(rtl8139if->iobase + CR, CR_TE|CR_RE); // Sets the RE and TE bits high

	debug_print(NOTICE,"RTL8139: CR = 0x%x, ISR =  %d, speed = %d mbps\n",
		inportb(rtl8139if->iobase + CR), inportw(rtl8139if->iobase + ISR), speed);

	/*
	 * Initialize the snmp variables and counters inside the struct netif.
	 * The last argument should be replaced with your link speed, in units
	 * of bits per second.
	 */
	NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, speed);

	/* administrative details */
	netif->name[0] = 'e';
	netif->name[1] = 'n';
	netif->num = num;
	num++;
	/* downward functions */
	netif->output = etharp_output;
	netif->linkoutput = rtl8139if_output;
	/* maximum transfer unit */
	netif->mtu = 1500;
	/* broadcast capability */
	netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_IGMP | NETIF_FLAG_LINK_UP ;
#if LWIP_IPV6
	netif->output_ip6 = ethip6_output;
	netif_create_ip6_linklocal_address(netif, 1);
	netif->ip6_autoconfig_enabled = 1;
#endif

	return ERR_OK;
}

