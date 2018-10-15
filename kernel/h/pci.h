#ifndef PCI_H
#define PCI_H
#include <types.h>
	
#define PCI_INTERRUPT_LINE		0x3c	
#define PCI_STPCI				0x09
#define PCI_INFINIBAND			0x0a	
#define PCI_BRIDGE_OTHER		0x80
#define PCI_VENDOR_ID			0x00		
#define PCI_DEVICE_ID			0x02		
#define PCI_COMMAND				0x04		
#define PCI_STATUS				0x06		
#define PCI_REVISION			0x08		
#define PCI_CLASS_API			0x09		
#define PCI_CLASS_SUB			0x0a	
#define PCI_CLASS_BASE			0x0b					
#define PCI_BIST				0x0f		
#define PCI_BASE_REGISTERS		0x10		
#define PCI_STATUS				0x06
#define	PCI_CAPABILITY_LIST		0x34	

typedef struct
{
    int		nBus;
    int		nDevice;
    int		nFunction;
	
    u16	nVendorID;
    u16	nDeviceID;
    u16	nCommand;
    u16	nStatus;
    u8	nRevisionID;
	u8 	nClassApi;
	u8	nClassBase;
	u8	nClassSub;
    u8	nCacheLineSize;
    u8	nLatencyTimer;
    u8	nHeaderType;
    u8	nSelfTestResult;
    u32	nAGPMode;

    union
    {
	struct
	{
	    u32	nBase0;
	    u32	nBase1;
	    u32	nBase2;
	    u32	nBase3;
	    u32	nBase4;
	    u32	nBase5;
	    u32	nCISPointer;
	    u16	nSubSysVendorID;
	    u16	nSubSysID;
	    u32	nExpROMAddr;
	    u8	nCapabilityList;
	    u8	nInterruptLine;
	    u8	nInterruptPin;
	    u8	nMinDMATime;
	    u8	nMaxDMALatency;
	    u8	nAGPStatus;
	    u8	nAGPCommand;
	} h0;
    } u;
    int nHandle;
} PCI_table;

void scan_pci_bus( int BusNum);
u16 read_pci_config( u32 BusNum, u32 DevNum, u32 FncNum, u32 Offset, u32 Size );
u32 write_pci_config( int nBusNum, int nDevNum, int nFncNum, int nOffset, int nSize, u32 nValue );
u32 read_pci_table( PCI_table * pci_info, int nBusNum, int nDevNum, int nFncNum );



#endif
