#include <types.h>
#include <video.h>
#include <kutils.h>
#include <pci.h>
#include <heapmngr.h>

void check_type_of_pci_method()
{
	
}

u32 write_pci_config( int BusNum, int DevNum, int FncNum, int nOffset, int nSize, u32 nValue )
{
	
}

u16 read_pci_config( u32 BusNum, u32 DevNum, u32 FncNum, u32 Offset, u32 Size )
{
	u32 Value = 0;
		outl( 0x0cf8, 0x80000000 | (BusNum << 16 ) | (DevNum << 11) | (FncNum << 8) | (Offset & ~3));
		if(Size == 1)
				Value = inb( 0x0cfc + (Offset & 3 ));
		if(Size == 2)
				Value = inw( 0x0cfc + (Offset & 2 ));
		if(Size == 4)
				Value = inl(0x0cfc);
		return  Value;
}

u32 read_pci_table( PCI_table * pci_info, int BusNum, int DevNum, int FncNum )
{
	pci_info->nBus = BusNum;
	pci_info->nDevice = DevNum;
	pci_info->nFunction = FncNum;

	pci_info->nVendorID = read_pci_config( BusNum, DevNum, FncNum, PCI_VENDOR_ID, 2 );
	pci_info->nDeviceID = read_pci_config( BusNum, DevNum, FncNum, PCI_DEVICE_ID, 2 );
	pci_info->nCommand = read_pci_config( BusNum, DevNum, FncNum, PCI_COMMAND, 2 );
	pci_info->nStatus = read_pci_config( BusNum, DevNum, FncNum, PCI_STATUS, 2 );
	pci_info->nRevisionID = read_pci_config( BusNum, DevNum, FncNum, PCI_REVISION, 1 );
	pci_info->nClassApi = read_pci_config( BusNum, DevNum, FncNum, PCI_CLASS_API, 1 );
	pci_info->nClassBase = read_pci_config( BusNum, DevNum, FncNum, PCI_CLASS_BASE, 1 );
	pci_info->nClassSub = read_pci_config( BusNum, DevNum, FncNum, PCI_CLASS_SUB, 1 );
	pci_info->u.h0.nBase0 = read_pci_config( BusNum, DevNum, FncNum, PCI_BASE_REGISTERS + 0, 4 );
	pci_info->u.h0.nBase1 = read_pci_config( BusNum, DevNum, FncNum, PCI_BASE_REGISTERS + 4, 4 );
	pci_info->u.h0.nBase2 = read_pci_config( BusNum, DevNum, FncNum, PCI_BASE_REGISTERS + 8, 4 );
	pci_info->u.h0.nBase3 = read_pci_config( BusNum, DevNum, FncNum, PCI_BASE_REGISTERS + 12, 4 );
	pci_info->u.h0.nBase4 = read_pci_config( BusNum, DevNum, FncNum, PCI_BASE_REGISTERS + 16, 4 );
	pci_info->u.h0.nBase5 = read_pci_config( BusNum, DevNum, FncNum, PCI_BASE_REGISTERS + 20, 4 );
	pci_info->u.h0.nCapabilityList = read_pci_config( BusNum, DevNum, FncNum, PCI_CAPABILITY_LIST, 1 );
	pci_info->u.h0.nInterruptLine = read_pci_config( BusNum, DevNum, FncNum, PCI_INTERRUPT_LINE, 1 );
	if( pci_info->u.h0.nInterruptLine >= 16 )
		pci_info->u.h0.nInterruptLine = 0;
}

void scan_pci_bus(int BusNum)
{
	PCI_table *pci_info;
	int nDev, nFnc;
	u32 nVendorID;
	set_term_color(80);
	printk("PCI: Scanning Bus %d\n", BusNum );
	for ( nDev = 0; nDev < 8; nDev++ )
	{
		for ( nFnc = 0; nFnc < 4; nFnc++ )
		{
			nVendorID = read_pci_config( BusNum, nDev, nFnc, PCI_CLASS_BASE, 2 );
			if ( nVendorID && (nVendorID != 0xFFFF) )
			{
				pci_info = malloc_( sizeof( PCI_table ));

				if ( pci_info != NULL )
				{
					set_term_color(make_color(COLOR_BLACK,COLOR_WHITE));	
					read_pci_table( pci_info, BusNum, nDev, nFnc );					
					printk("VendorID: %x DeviceID: %x  at %d:%d:%d Classcode: %x Subclass:%x Status: %x irq: %d\n",
					pci_info->nVendorID, pci_info->nDeviceID, BusNum, nDev, nFnc,pci_info->nClassBase, pci_info->nClassSub,pci_info->nStatus,pci_info->u.h0.nInterruptLine);
									
					/*printk("%x\n", pci_info->u.h0.nBase0);
					printk("%x\n", pci_info->u.h0.nBase1);
					printk("%x\n", pci_info->u.h0.nBase2);
					printk("%x\n", pci_info->u.h0.nBase3);
					printk("%x\n", pci_info->u.h0.nBase4);
					printk("%x\n", pci_info->u.h0.nBase5);*/
				}
				free(pci_info);
			}
		}
	}
	set_term_color(80);

	printk("PCI: Scanning ended!");
	set_term_color(make_color(COLOR_BLACK,COLOR_WHITE));
}

