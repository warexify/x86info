/*
 *  $Id: features.c,v 1.20 2002/10/30 02:11:08 davej Exp $
 *  This file is part of x86info
 *  (C) 2001 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 *
 *  Feature flag decoding.
 */

#include <stdio.h>
#include "x86info.h"

void decode_feature_flags (struct cpudata *cpu, int flags, int eflags)
{
	const char *generic_cap_flags[] = {
		"fpu", "vme", "de", "pse", "tsc", "msr", "pae", "mce",
		"cx8", "apic", NULL, "sep", "mtrr", "pge", "mca", "cmov",
		"pat", "pse36", "psn", "clflsh", NULL, "dtes", "acpi", "mmx",
		"fxsr", "sse", "sse2", "selfsnoop", "ht", "acc", "ia64", "pbe"
	};
	const char *generic_cap_flags_desc[] = {
		"Onboard FPU",
		"Virtual Mode Extensions",
		"Debugging Extensions",
		"Page Size Extensions",
		"Time Stamp Counter",
		"Model-Specific Registers",
		"Physical Address Extensions",
		"Machine Check Architecture",
		"CMPXCHG8 instruction",
		"Onboard APIC",
		NULL,
		"SYSENTER/SYSEXIT",
		"Memory Type Range Registers",
		"Page Global Enable",
		"Machine Check Architecture",
		"CMOV instruction",
		"Page Attribute Table",
		"36-bit PSEs",
		"Processor serial number",
		"CLFLUSH instruction",
		NULL,
		"Debug Trace Store",
		"ACPI via MSR",
		"MMX support",
		"FXSAVE and FXRESTORE instructions",
		"SSE support",
		"SSE2 support",
		"CPU self snoop",
		"Hyper-Threading",
		"Automatic clock Control",
		"IA-64 processor",
		"Pending Break Enable"
	};
	const char *amd_cap_flags[] = {
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, "syscall", NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, "mp", NULL, NULL, "mmxext", NULL,
		NULL, NULL, NULL, NULL, NULL, "lm", "3dnowext", "3dnow"
	};
	const char *centaur_cap_flags[] = {
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, "mmxext", NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, "3dnowext", "3dnow"
	};
	const char *transmeta_cap_flags[] = {
		"recovery", "longrun", NULL, "lrti", NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL
	};

	int i;

	if (show_flags == 0)
		return;

	printf ("Feature flags:\n");
	for (i = 0; i < 32; i++)
		if (flags & (1 << i)) {
			if (show_verbose_flags) {
				printf ("\t%s\n", generic_cap_flags_desc[i]);
			} else {
				printf (" %s", generic_cap_flags[i]);
			}
		}

	printf ("\n");

	if (eflags == 0) {
		printf ("\n");
		return;
	}

	/* Vendor specific extensions. */
	switch (cpu->vendor) {

		case VENDOR_AMD:
			printf ("Extended feature flags:\n");
			for (i = 0; i < 32; i++) {
				if (eflags & (1 << i) && amd_cap_flags[i])
					printf (" %s", amd_cap_flags[i]);
			}
			break;

		case VENDOR_CENTAUR:
			printf ("Extended feature flags:\n");
			for (i = 0; i < 32; i++) {
				if (eflags & (1 << i) && centaur_cap_flags[i])
					printf (" %s", centaur_cap_flags[i]);
			}
			break;

		case VENDOR_TRANSMETA:
			printf ("Extended feature flags:\n");
			for (i = 0; i < 32; i++) {
				if (eflags & (1 << i) && transmeta_cap_flags[i])
					printf (" %s", transmeta_cap_flags[i]);
			}
			break;

		case VENDOR_CYRIX:
			break;

		case VENDOR_INTEL:
			/* Hyper-Threading Technology */
			if (flags & (1 << 28)) {
				int nr_ht = (eflags >> 16) & 0xFF;
				printf ("Number of logical processors supported "
						"within the physical package: %d\n", nr_ht);
		}
		break;

		default:
			/* Unknown CPU manufacturer or no special handling needed */
			break;
	}

	printf ("\n\n");
}
