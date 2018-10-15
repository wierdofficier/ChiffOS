#ifndef RTL8139_H
#define RTL8139_H
#include <types.h>
typedef unsigned char u8_t; 

typedef short s16_t;
typedef unsigned short u16_t;
typedef int s32_t;
typedef unsigned int u32_t;
typedef long long s64_t;
typedef unsigned long long u64_t;

extern void install_RTL8139(void);
extern  int rtl8139_init( void );
bool transferDataToTxBuffer(void* data, u32_t length);
extern void install_network(void);
// RTL8139C register definitions
#define RTL8139_IDR0                0x00        // Mac address
#define RTL8139_MAR0                0x08        // Multicast filter
#define RTL8139_TXSTATUS0           0x10        // Transmit status (4 32bit regs)
#define RTL8139_TXADDR0             0x20        // Tx descriptors (also 4 32bit)
#define RTL8139_RXBUF               0x30        // Receive buffer start address
#define RTL8139_RXEARLYCNT          0x34        // Early Rx byte count
#define RTL8139_RXEARLYSTATUS       0x36        // Early Rx status
#define RTL8139_CHIPCMD             0x37        // Command register
#define RTL8139_RXBUFTAIL           0x38        // Current address of packet read (queue tail)
#define RTL8139_RXBUFHEAD           0x3A        // Current buffer address (queue head)
#define RTL8139_INTRMASK            0x3C        // Interrupt mask
#define RTL8139_INTRSTATUS          0x3E        // Interrupt status
#define RTL8139_TXCONFIG            0x40        // Tx config
#define RTL8139_RXCONFIG            0x44        // Rx config
#define RTL8139_TIMER               0x48        // A general purpose counter
#define RTL8139_RXMISSED            0x4C        // 24 bits valid, write clears
#define RTL8139_CFG9346             0x50        // 93C46 command register
#define RTL8139_CONFIG0             0x51        // Configuration reg 0
#define RTL8139_CONFIG1             0x52        // Configuration reg 1
#define RTL8139_TIMERINT            0x54        // Timer interrupt register (32 bits)
#define RTL8139_MEDIASTATUS         0x58        // Media status register
#define RTL8139_CONFIG3             0x59        // Config register 3
#define RTL8139_CONFIG4             0x5A        // Config register 4
#define RTL8139_MULTIINTR           0x5C        // Multiple interrupt select
#define RTL8139_MII_TSAD            0x60        // Transmit status of all descriptors (16 bits)
#define RTL8139_MII_BMCR            0x62        // Basic Mode Control Register (16 bits)
#define RTL8139_MII_BMSR            0x64        // Basic Mode Status Register (16 bits)
#define RTL8139_AS_ADVERT           0x66        // Auto-negotiation advertisement reg (16 bits)
#define RTL8139_AS_LPAR             0x68        // Auto-negotiation link partner reg (16 bits)
#define RTL8139_AS_EXPANSION        0x6A        // Auto-negotiation expansion reg (16 bits)

// RTL8193C command bits
#define RTL8139_CMD_RESET           0x10
#define RTL8139_CMD_RX_ENABLE       0x08
#define RTL8139_CMD_TX_ENABLE       0x04
#define RTL8139_CMD_RX_BUF_EMPTY    0x01

// RTL8139C interrupt status bits
#define RTL8139_INT_PCIERR          0x8000        // PCI Bus error
#define RTL8139_INT_TIMEOUT         0x4000        // Set when TCTR reaches TimerInt value
#define RTL8139_INT_CABLE           0x2000        // Set when Cable Length Change
#define RTL8139_INT_RXFIFO_OVERFLOW 0x0040        // Rx FIFO overflow
#define RTL8139_INT_RXFIFO_UNDERRUN 0x0020        // Packet underrun / link change
#define RTL8139_INT_RXBUF_OVERFLOW  0x0010        // Rx BUFFER overflow
#define RTL8139_INT_TX_ERR          0x0008
#define RTL8139_INT_TX_OK           0x0004
#define RTL8139_INT_RX_ERR          0x0002
#define RTL8139_INT_RX_OK           0x0001

// RTL8139C transmit status bits
#define RTL8139_TX_CARRIER_LOST     0x80000000    // Carrier sense lost
#define RTL8139_TX_ABORTED          0x40000000    // Transmission aborted
#define RTL8139_TX_OUT_OF_WINDOW    0x20000000    // Out of window collision
#define RTL8139_TX_STATUS_OK        0x00008000    // Status ok: a good packet was transmitted
#define RTL8139_TX_UNDERRUN         0x00004000    // Transmit FIFO underrun
#define RTL8139_TX_HOST_OWNS        0x00002000    // Set to 1 when DMA operation is completed
#define RTL8139_TX_SIZE_MASK        0x00001FFF    // Descriptor size mask

// RTL8139C receive status bits
#define RTL8139_RX_MULTICAST        0x00008000    // Multicast packet
#define RTL8139_RX_PAM              0x00004000    // Physical address matched
#define RTL8139_RX_BROADCAST        0x00002000    // Broadcast address matched
#define RTL8139_RX_BAD_SYMBOL       0x00000020    // Invalid symbol in 100TX packet
#define RTL8139_RX_RUNT             0x00000010    // Packet size is <64 bytes
#define RTL8139_RX_TOO_LONG         0x00000008    // Packet size is >4K bytes
#define RTL8139_RX_CRC_ERR          0x00000004    // CRC error
#define RTL8139_RX_FRAME_ALIGN      0x00000002    // Frame alignment error
#define RTL8139_RX_STATUS_OK        0x00000001    // Status ok: a good packet was received


#define RTL8139_TSD0     0x10      /* Tx Status of Descriptors */
#define RTL8139_ISR     0x3e      /* Intrpt Status Reg */
#define RTL8139_CAPR    0x38
#define RTL8139_CR      0x37
#define RTL8139_MTU      1500
// Configuration definitions
#define RTL8139_NETWORK_BUFFER_SIZE 0x18048        // 8 KiB
#define SYNFLAG 0x002
#define ACKFLAG 0x010
#define PSHACKFLAG 0x018
#define ETHER_IP 0x8
#define TCPtype 0x6
#define FINACK 0x011

#define PKT_RX          0x0001
#define RX_ERR          0x0002
#define TX_OK           0x0004

/* The macros below were copied straight from the linux 8139too.c driver. */
enum ClearBitMasks {
        MultiIntrClear = 0xF000,
        ChipCmdClear = 0xE2,
        Config1Clear = (1<<7)|(1<<6)|(1<<3)|(1<<2)|(1<<1),
};

enum ChipCmdBits {
        CmdReset = 0x10,
        CmdRxEnb = 0x08,
        CmdTxEnb = 0x04,
        RxBufEmpty = 0x01,
};

/* Interrupt register bits */
enum IntrStatusBits {
        PCIErr = 0x8000,
        PCSTimeout = 0x4000,
        RxFIFOOver = 0x40,
        RxUnderrun = 0x20,
        RxOverflow = 0x10,
        TxErr = 0x08,
        TxOK = 0x04,
        RxErr = 0x02,
        RxOK = 0x01,

        RxAckBits = RxFIFOOver | RxOverflow | RxOK,
};

enum TxStatusBits {
        TxHostOwns = 0x2000,
        TxUnderrun = 0x4000,
        TxStatOK = 0x8000,
        TxOutOfWindow = 0x20000000,
        TxAborted = 0x40000000,
        TxCarrierLost = 0x80000000,
};
enum RxStatusBits {
        RxMulticast = 0x8000,
        RxPhysical = 0x4000,
        RxBroadcast = 0x2000,
        RxBadSymbol = 0x0020,
        RxRunt = 0x0010,
        RxTooLong = 0x0008,
        RxCRCErr = 0x0004,
        RxBadAlign = 0x0002,
        RxStatusOK = 0x0001,
};

/* Bits in RxConfig. */
enum rx_mode_bits {
        AcceptErr = 0x20,
        AcceptRunt = 0x10,
        AcceptBroadcast = 0x08,
        AcceptMulticast = 0x04,
        AcceptMyPhys = 0x02,
        AcceptAllPhys = 0x01,
};

/* Bits in TxConfig. */
enum tx_config_bits {

        /* Interframe Gap Time. Only TxIFG96 doesn't violate IEEE 802.3 */
        TxIFGShift = 24,
        TxIFG84 = (0 << TxIFGShift),    /* 8.4us / 840ns (10 / 100Mbps) */
        TxIFG88 = (1 << TxIFGShift),    /* 8.8us / 880ns (10 / 100Mbps) */
        TxIFG92 = (2 << TxIFGShift),    /* 9.2us / 920ns (10 / 100Mbps) */
        TxIFG96 = (3 << TxIFGShift),    /* 9.6us / 960ns (10 / 100Mbps) */

        TxLoopBack = (1 << 18) | (1 << 17), /* enable loopback test mode */
        TxCRC = (1 << 16),      /* DISABLE appending CRC to end of Tx packets */
        TxClearAbt = (1 << 0),  /* Clear abort (WO) */
        TxDMAShift = 8,         /* DMA burst value (0-7) is shifted this many bits */
        TxRetryShift = 4,       /* TXRR value (0-15) is shifted this many bits */

        TxVersionMask = 0x7C800000, /* mask out version bits 30-26, 23 */
};

/* Bits in Config1 */
enum Config1Bits {
        Cfg1_PM_Enable = 0x01,
        Cfg1_VPD_Enable = 0x02,
        Cfg1_PIO = 0x04,
        Cfg1_MMIO = 0x08,
        LWAKE = 0x10,           /* not on 8139, 8139A */
        Cfg1_Driver_Load = 0x20,
        Cfg1_LED0 = 0x40,
        Cfg1_LED1 = 0x80,
        SLEEP = (1 << 1),       /* only on 8139, 8139A */
        PWRDN = (1 << 0),       /* only on 8139, 8139A */
};

/* Bits in Config3 */
enum Config3Bits {
        Cfg3_FBtBEn    = (1 << 0), /* 1 = Fast Back to Back */
        Cfg3_FuncRegEn = (1 << 1), /* 1 = enable CardBus Function registers */
        Cfg3_CLKRUN_En = (1 << 2), /* 1 = enable CLKRUN */
        Cfg3_CardB_En  = (1 << 3), /* 1 = enable CardBus registers */
        Cfg3_LinkUp    = (1 << 4), /* 1 = wake up on link up */
        Cfg3_Magic     = (1 << 5), /* 1 = wake up on Magic Packet (tm) */
        Cfg3_PARM_En   = (1 << 6), /* 0 = software can set twister parameters */
        Cfg3_GNTSel    = (1 << 7), /* 1 = delay 1 clock from PCI GNT signal */
};

/* Bits in Config4 */
enum Config4Bits {
        LWPTN = (1 << 2),       /* not on 8139, 8139A */
};

/* Bits in Config5 */
enum Config5Bits {
        Cfg5_PME_STS     = (1 << 0), /* 1 = PCI reset resets PME_Status */
        Cfg5_LANWake     = (1 << 1), /* 1 = enable LANWake signal */
        Cfg5_LDPS        = (1 << 2), /* 0 = save power when link is down */
        Cfg5_FIFOAddrPtr = (1 << 3), /* Realtek internal SRAM testing */
        Cfg5_UWF         = (1 << 4), /* 1 = accept unicast wakeup frame */
        Cfg5_MWF         = (1 << 5), /* 1 = accept multicast wakeup frame */
        Cfg5_BWF         = (1 << 6), /* 1 = accept broadcast wakeup frame */
};

enum RxConfigBits {
        /* rx fifo threshold */
        RxCfgFIFOShift = 13,
        RxCfgFIFONone = (7 << RxCfgFIFOShift),

        /* Max DMA burst */
        RxCfgDMAShift = 8,
        RxCfgDMAUnlimited = (7 << RxCfgDMAShift),

        /* rx ring buffer length */
        RxCfgRcv8K = 0,
        RxCfgRcv16K = (1 << 11),
        RxCfgRcv32K = (1 << 12),
        RxCfgRcv64K = (1 << 11) | (1 << 12),

        /* Disable packet wrap at end of Rx buffer. (not possible with 64k) */
        RxNoWrap = (1 << 7),
};

/* Twister tuning parameters from RealTek.
   Completely undocumented, but required to tune bad links on some boards. */
enum CSCRBits {
        CSCR_LinkOKBit = 0x0400,
        CSCR_LinkChangeBit = 0x0800,
        CSCR_LinkStatusBits = 0x0f000,
        CSCR_LinkDownOffCmd = 0x003c0,
        CSCR_LinkDownCmd = 0x0f3c0,
};

enum Cfg9346Bits {
        Cfg9346_Lock = 0x00,
        Cfg9346_Unlock = 0xC0,
};
extern u32 total_length;
#endif
