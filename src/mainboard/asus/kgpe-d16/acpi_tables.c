/*
 * This file is part of the coreboot project.
 *
 * Copyright (C) 2010 Advanced Micro Devices, Inc.
 * Copyright (C) 2015 Timothy Pearson <tpearson@raptorengineeringinc.com>, Raptor Engineering
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <console/console.h>
#include <string.h>
#include <arch/acpi.h>
#include <arch/ioapic.h>
#include <device/pci.h>
#include <device/pci_ids.h>
#include <cpu/x86/msr.h>
#include <cpu/amd/mtrr.h>
#include <cpu/amd/amdfam10_sysconf.h>

#include "mb_sysconf.h"

unsigned long acpi_fill_madt(unsigned long current)
{
	device_t dev;
	u32 dword;
	u32 gsi_base=0;
	uint32_t apicid_sp5100;
	uint32_t apicid_sr5650;
	/* create all subtables for processors */
	current = acpi_create_madt_lapics(current);

	apicid_sp5100 = 0x20;
	apicid_sr5650 = apicid_sp5100 + 1;

	/* Write SB700 IOAPIC, only one */
	current += acpi_create_madt_ioapic((acpi_madt_ioapic_t *) current, apicid_sp5100,
					   IO_APIC_ADDR, gsi_base);
	/* IOAPIC on rs5690 */
	gsi_base += 24;		/* SB700 has 24 IOAPIC entries. */
	dev = dev_find_slot(0, PCI_DEVFN(0, 0));
	if (dev) {
		pci_write_config32(dev, 0xF8, 0x1);
		dword = pci_read_config32(dev, 0xFC) & 0xfffffff0;
		current += acpi_create_madt_ioapic((acpi_madt_ioapic_t *) current, apicid_sr5650,
						   dword, gsi_base);
	}

	/* bus, source, gsirq, flags */
	current += acpi_create_madt_irqoverride((acpi_madt_irqoverride_t *)
						current, 0, 0, 2, 0);
	current += acpi_create_madt_irqoverride((acpi_madt_irqoverride_t *)
						current, 0, 9, 9, 0xF);
	/* 0: mean bus 0--->ISA */
	/* 0: PIC 0 */
	/* 2: APIC 2 */
	/* 5 mean: 0101 --> Edge-triggered, Active high */

	/* create all subtables for processors */
	current += acpi_create_madt_lapic_nmi((acpi_madt_lapic_nmi_t *)current, 0, 5, 1);
	current += acpi_create_madt_lapic_nmi((acpi_madt_lapic_nmi_t *)current, 1, 5, 1);
	/* 1: LINT1 connect to NMI */

	return current;
}