#include <errno.h>
#include <logging.h>
 typedef  unsigned int uint32_t;
 typedef  unsigned short uint16_t;
//typedef  char  int8_t;
typedef unsigned char uint8_t;
#pragma once

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

typedef void (*pci_func_t)(uint32_t device, uint16_t vendor_id, uint16_t device_id, void * extra);

static inline int pci_extract_bus(uint32_t device) {
	return (uint8_t)((device >> 16));
}
static inline int pci_extract_slot(uint32_t device) {
	return (uint8_t)((device >> 8));
}
static inline int pci_extract_func(uint32_t device) {
	return (uint8_t)(device);
}

static inline uint32_t pci_get_addr(uint32_t device, int field) {
	return 0x80000000 | (pci_extract_bus(device) << 16) | (pci_extract_slot(device) << 11) | (pci_extract_func(device) << 8) | ((field) & 0xFC);
}

static inline uint32_t pci_box_device(int bus, int slot, int func) {
	return (uint32_t)((bus << 16) | (slot << 8) | func);
}

uint32_t pci_read_field(uint32_t device, int field, int size);
void pci_write_field(uint32_t device, int field, int size, uint32_t value);
uint16_t pci_find_type(uint32_t dev);
const char * pci_vendor_lookup(unsigned short vendor_id);
const char * pci_device_lookup(unsigned short vendor_id, unsigned short device_id);
void pci_scan_hit(pci_func_t f, uint32_t dev, void * extra);
void pci_scan_func(pci_func_t f, int type, int bus, int slot, int func, void * extra);
void pci_scan_slot(pci_func_t f, int type, int bus, int slot, void * extra);
void pci_scan_bus(pci_func_t f, int type, int bus, void * extra);
void pci_scan(pci_func_t f, int type, void * extra);
 typedef  unsigned int uint32_t;
//typedef  char  int8_t;
#ifdef WITH_PCI_IDS
#include "pcihdr.h"
#endif

typedef struct {
	uint32_t base[6];
	uint32_t size[6];
	uint32_t irq;
} pci_info_t;

#define PCI_IGNORE_SUBID	(0)
/*
 * PCI configuration registers
 */
#define	PCI_CFID	0x00	/* Configuration ID */
#define	PCI_CFCS	0x04	/* Configurtion Command/Status */
#define	PCI_CFRV	0x08	/* Configuration Revision */
#define	PCI_CFLT	0x0c	/* Configuration Latency Timer */
#define	PCI_CBIO	0x10	/* Configuration Base IO Address */
#define PCI_CSID	0x2C	/* Configuration Subsystem Id & Subsystem Vendor Id */
#define	PCI_CFIT	0x3c	/* Configuration Interrupt */
#define	PCI_CFDA	0x40	/* Configuration Driver Area */

#define PHYS_IO_MEM_START	0
#define	PCI_MEM			0
#define	PCI_INTA		0
#define PCI_NSLOTS		22
#define PCI_NBUS		0

#define	PCI_CONF_ADDR_REG	0xcf8
#define	PCI_CONF_FRWD_REG	0xcf8
#define	PCI_CONF_DATA_REG	0xcfc

#define PCI_IO_CONF_START	0xc000

#define MAX_BUS			16
#define MAX_SLOTS		32

static uint32_t mechanism = 0;
static uint32_t adapters[MAX_BUS][MAX_SLOTS] = {[0 ... MAX_BUS-1][0 ... MAX_SLOTS-1] = -1};

static void pci_conf_write(uint32_t bus, uint32_t slot, uint32_t off, uint32_t val)
{
	if (mechanism == 1) {
		outportl(PCI_CONF_FRWD_REG, bus);
		outportl(PCI_CONF_ADDR_REG, 0xf0);
		outportl(PCI_IO_CONF_START | (slot << 8) | off, val);
	} else {
		outportl(PCI_CONF_ADDR_REG,
		      (0x80000000 | (bus << 16) | (slot << 11) | off));
		outportl(PCI_CONF_DATA_REG, val);
	}
}

static uint32_t pci_conf_read(uint32_t bus, uint32_t slot, uint32_t off)
{
	uint32_t data = -1;

	outportl(PCI_CONF_ADDR_REG,
	      (0x80000000 | (bus << 16) | (slot << 11) | off));
	data = inportl(PCI_CONF_DATA_REG);

	if ((data == 0xffffffff) && (slot < 0x10)) {
		outportl(PCI_CONF_FRWD_REG, bus);
		outportl(PCI_CONF_ADDR_REG, 0xf0);
		data = inportl(PCI_IO_CONF_START | (slot << 8) | off);
		if (data == 0xffffffff)
			return data;
		if (!mechanism)
			mechanism = 1;
	} else if (!mechanism)
		mechanism = 2;

	return data;
}

static inline uint32_t pci_subid(uint32_t bus, uint32_t slot)
{
	return pci_conf_read(bus, slot, PCI_CSID);
}

static inline uint32_t pci_what_irq(uint32_t bus, uint32_t slot)
{
	return pci_conf_read(bus, slot, PCI_CFIT) & 0xFF;
}

static inline uint32_t pci_what_iobase(uint32_t bus, uint32_t slot, uint32_t nr)
{
	return pci_conf_read(bus, slot, PCI_CBIO + nr*4) & 0xFFFFFFFC;
}

static inline void pci_bus_master(uint32_t bus, uint32_t slot)
{
	// set the device to a bus master

	uint32_t cmd = pci_conf_read(bus, slot, PCI_CFCS) | 0x4;
	pci_conf_write(bus, slot, PCI_CFCS, cmd);
}

static inline uint32_t pci_what_size(uint32_t bus, uint32_t slot, uint32_t nr)
{
	uint32_t tmp, ret;

	// backup the original value
	tmp = pci_conf_read(bus, slot, PCI_CBIO + nr*4);

	// determine size
	pci_conf_write(bus, slot, PCI_CBIO + nr*4, 0xFFFFFFFF);
	ret = ~pci_conf_read(bus, slot, PCI_CBIO + nr*4) + 1;

	// restore original value
	pci_conf_write(bus, slot, PCI_CBIO + nr*4, tmp);

	return ret;
}

int pci_init(void)
{
	uint32_t slot, bus;

	for (bus = 0; bus < MAX_BUS; bus++)
		for (slot = 0; slot < MAX_SLOTS; slot++)
			adapters[bus][slot] = pci_conf_read(bus, slot, PCI_CFID);

	return 0;
}

int pci_get_device_info(uint32_t vendor_id, uint32_t device_id, uint32_t subsystem_id, pci_info_t* info, int8_t bus_master)
{
	uint32_t slot, bus, i;

	if (!info)
		return -EINVAL;

	//if (!mechanism && !is_uhyve()) //ändring
		pci_init();

	for (bus = 0; bus < MAX_BUS; bus++) {
		for (slot = 0; slot < MAX_SLOTS; slot++) {
			if (adapters[bus][slot] != -1) {
				if (((adapters[bus][slot] & 0xffff) == vendor_id) &&
				   (((adapters[bus][slot] & 0xffff0000) >> 16) == device_id) &&
				   (((pci_subid(bus, slot) >> 16) & subsystem_id) == subsystem_id)) {
					for(i=0; i<6; i++) {
						info->base[i] = pci_what_iobase(bus, slot, i);
						info->size[i] = (info->base[i]) ? pci_what_size(bus, slot, i) : 0;
					}
					info->irq = pci_what_irq(bus, slot);
					if (bus_master)
						pci_bus_master(bus, slot);
					return 0;
				}
			}
		}
	}

	return -EINVAL;
}

int print_pci_adapters(void)
{
	uint32_t slot, bus;
	uint32_t counter = 0;
#ifdef WITH_PCI_IDS
	uint32_t i;
#endif

	if (!mechanism)
		pci_init();

	for (bus = 0; bus < MAX_BUS; bus++) {
                for (slot = 0; slot < MAX_SLOTS; slot++) {

		if (adapters[bus][slot] != -1) {
				counter++;
				debug_print(NOTICE,"%d) Vendor ID: 0x%x  Device Id: 0x%x\n",
					counter, adapters[bus][slot] & 0xffff,
					(adapters[bus][slot] & 0xffff0000) >> 16);

#ifdef WITH_PCI_IDS
				for (i=0; i<PCI_VENTABLE_LEN; i++) {
					if ((adapters[bus][slot] & 0xffff) ==
					    (uint32_t)PciVenTable[i].VenId)
						debug_print(NOTICE,"\tVendor is %s\n",
							PciVenTable[i].VenShort);
				}

				for (i=0; i<PCI_DEVTABLE_LEN; i++) {
					if ((adapters[bus][slot] & 0xffff) ==
					    (uint32_t)PciDevTable[i].VenId) {
						if (((adapters[bus][slot] & 0xffff0000) >> 16) ==
						    PciDevTable[i].DevId) {
							debug_print(NOTICE,"\tChip: %s ChipDesc: %s\n",
							     PciDevTable[i].Chip,
							     PciDevTable[i].ChipDesc);
						}
					}
				}
#endif
			}
		}
	}

	return 0;
}
 


 

 
#include <pci_list.h>


void pci_write_field(uint32_t device, int field, int size, uint32_t value) {
	outportl(PCI_ADDRESS_PORT, pci_get_addr(device, field));
	outportl(PCI_VALUE_PORT, value);
}

uint32_t pci_read_field(uint32_t device, int field, int size) {
	outportl(PCI_ADDRESS_PORT, pci_get_addr(device, field));

	if (size == 4) {
		uint32_t t = inportl(PCI_VALUE_PORT);
		return t;
	} else if (size == 2) {
		uint16_t t = inports(PCI_VALUE_PORT + (field & 2));
		return t;
	} else if (size == 1) {
		uint8_t t = inportb(PCI_VALUE_PORT + (field & 3));
		return t;
	}
	return 0xFFFF;
}

uint16_t pci_find_type(uint32_t dev) {
	return (pci_read_field(dev, PCI_CLASS, 1) << 8) | pci_read_field(dev, PCI_SUBCLASS, 1);
}

const char * pci_vendor_lookup(unsigned short vendor_id) {
	for (unsigned int i = 0; i < PCI_VENTABLE_LEN; ++i) {
		if (PciVenTable[i].VenId == vendor_id) {
			return PciVenTable[i].VenFull;
		}
	}
	return "";
}

const char * pci_device_lookup(unsigned short vendor_id, unsigned short device_id) {
	for (unsigned int i = 0; i < PCI_DEVTABLE_LEN; ++i) {
		if (PciDevTable[i].VenId == vendor_id && PciDevTable[i].DevId == device_id) {
			return PciDevTable[i].ChipDesc;
		}
	}
	return "";
}

void pci_scan_hit(pci_func_t f, uint32_t dev, void * extra) {
	int dev_vend = (int)pci_read_field(dev, PCI_VENDOR_ID, 2);
	int dev_dvid = (int)pci_read_field(dev, PCI_DEVICE_ID, 2);

	f(dev, dev_vend, dev_dvid, extra);
}

void pci_scan_func(pci_func_t f, int type, int bus, int slot, int func, void * extra) {
	uint32_t dev = pci_box_device(bus, slot, func);
	if (type == -1 || type == pci_find_type(dev)) {
		pci_scan_hit(f, dev, extra);
	}
	if (pci_find_type(dev) == PCI_TYPE_BRIDGE) {
		pci_scan_bus(f, type, pci_read_field(dev, PCI_SECONDARY_BUS, 1), extra);
	}
}

void pci_scan_slot(pci_func_t f, int type, int bus, int slot, void * extra) {
	uint32_t dev = pci_box_device(bus, slot, 0);
	if (pci_read_field(dev, PCI_VENDOR_ID, 2) == PCI_NONE) {
		return;
	}
	pci_scan_func(f, type, bus, slot, 0, extra);
	if (!pci_read_field(dev, PCI_HEADER_TYPE, 1)) {
		return;
	}
	for (int func = 1; func < 8; func++) {
		uint32_t dev = pci_box_device(bus, slot, func);
		if (pci_read_field(dev, PCI_VENDOR_ID, 2) != PCI_NONE) {
			pci_scan_func(f, type, bus, slot, func, extra);
		}
	}
}

void pci_scan_bus(pci_func_t f, int type, int bus, void * extra) {
	for (int slot = 0; slot < 32; ++slot) {
		pci_scan_slot(f, type, bus, slot, extra);
	}
}

void pci_scan(pci_func_t f, int type, void * extra) {

	if ((pci_read_field(0, PCI_HEADER_TYPE, 1) & 0x80) == 0) {
		pci_scan_bus(f,type,0,extra);
		return;
	}

	for (int func = 0; func < 8; ++func) {
		uint32_t dev = pci_box_device(0, 0, func);
		if (pci_read_field(dev, PCI_VENDOR_ID, 2) != PCI_NONE) {
			pci_scan_bus(f, type, func, extra);
		} else {
			break;
		}
	}
}

