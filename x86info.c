/*
 *  (C) 2001-2011 Dave Jones.
 *
 *  Licensed under the terms of the GNU GPL License version 2.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "x86info.h"
#include "mptable.h"

#include "Intel/Intel.h"

int show_apic=0;
int show_bench=0;
int show_bios=0;
int show_machine_check=0;
int show_bugs=0;
int show_cacheinfo=0;
int show_connector=0;
int show_eblcr=0;
int show_msr=0;
int show_microcode=0;
int show_mtrr=0;
int show_pm=0;
int show_registers=0;
int show_urls=0;

static int show_mptable=0;
static int show_flags=0;
static int show_MHz=0;
static int show_addr_sizes=0;

unsigned int all_cpus = 0;

int debug = 0;
int verbose = 0;
int used_UP = 0;
int user_is_root = 1;
static int need_root = 0;

unsigned int nrCPUs=1;
static unsigned int nrSMPCPUs;

struct cpudata *firstcpu;

static void usage (char *programname)
{
	printf("Usage: %s [<switches>]\n\
-a,   --all\n\
      --all-cpus\n\
      --addr\n\
      --apic\n\
      --bench\n\
      --bios\n\
      --bugs\n\
-c,   --cache\n\
      --connector\n\
      --debug\n\
-f,   --flags\n\
-mce, --show-machine-check\n\
-mhz, --mhz\n\
      --microcode\n\
-mp,  --mptable\n\
-m,   --msr\n\
      --mult\n\
      --mtrr\n\
      --pm\n\
-r,   --registers\n\
-u,   --urls\n\
-v,   --verbose\n\
\n", programname);
	exit (0);
}

static void parse_command_line (int argc, char **argv)
{
	char **argp, *arg;

	for (argp = argv+1; argp <= argv + argc && (arg = *argp); argp++) {
		if ((!strcmp(arg, "-a") || !strcmp(arg, "--all"))) {

			if (user_is_root == 1) {
				show_apic = 1;
				show_bios = 1;
				show_msr = 1;
				show_microcode = 1;
				need_root = 1;
				show_mptable =1;
				show_mtrr = 1;
				show_machine_check = 1;
				show_eblcr =1;
			}
			show_addr_sizes = 1;
			show_bugs = 1;
			show_cacheinfo = 1;
			show_connector = 1;
			show_flags = 1;
			show_MHz = 1;
			show_pm = 1;
			show_registers = 1;
			show_urls = 1;
		}

		if (!strcmp(arg, "--all-cpus"))
			all_cpus = 1;

		if (!strcmp(arg, "--addr"))
			show_addr_sizes = 1;

		if (!strcmp(arg, "--apic")) {
			need_root = 1;
			show_apic = 1;
		}

		if (!strcmp(arg, "--bench"))
			show_bench = 1;

		if (!strcmp(arg, "--bios")) {
			need_root = 1;
			show_bios = 1;
		}

		if (!strcmp(arg, "--bugs"))
			show_bugs = 1;

		if ((!strcmp(arg, "-c") || !strcmp(arg, "--cache")))
			show_cacheinfo = 1;

		if (!strcmp(arg, "--connector"))
			show_connector = 1;

		if (!strcmp(arg, "--debug"))
			debug = 1;

		if ((!strcmp(arg, "-f") || !strcmp(arg, "--flags")))
			show_flags = 1;

		if ((!strcmp(arg, "-m") || !strcmp(arg, "--msr"))) {
			need_root = 1;
			show_msr = 1;
		}

		if (!strcmp(arg, "--microcode")) {
			need_root = 1;
			show_microcode = 1;
		}

		if ((!strcmp(arg, "-mhz") || !strcmp(arg, "--mhz")))
			show_MHz = 1;

		if ((!strcmp(arg, "-mp") || !strcmp(arg, "--mptable"))) {
			need_root = 1;
			show_mptable = 1;
		}

		if (!strcmp(arg, "--mtrr")) {
			need_root = 1;
			show_mtrr = 1;
		}

		if (!strcmp(arg, "--mult")) {
			need_root = 1;
			show_eblcr = 1;
		}

		if (!strcmp(arg, "--pm"))
			show_pm = 1;

		if ((!strcmp(arg, "-r") || !strcmp(arg, "--registers")))
			show_registers = 1;

		if ((!strcmp(arg, "-mce") || !strcmp(arg, "--show-machine-check"))) {
			need_root = 1;
			show_machine_check = 1;
		}

		if ((!strcmp(arg, "-u") || !strcmp(arg, "--urls")))
			show_urls = 1;

		if ((!strcmp(arg, "-v") || !strcmp(arg, "--verbose")))
			verbose = 1;

		if ((!strcmp(arg, "?") || !strcmp(arg, "--help")))
			usage(argv[0]);
	}
}


static void separator(void)
{
	int j;

	for (j = 0; j < 74; j++)
		printf("-");
	printf("\n");
}

static void display_MHz(struct cpudata *cpu)
{
	if (cpu->MHz < 1000)
		printf("%uMHz", cpu->MHz);
	else {
		int a = (cpu->MHz / 1000);
		int b = ((cpu->MHz % 1000)/100);
		int c = (a*1000)+(b*100);

		printf("%u.%u%uGHz", a, b, (cpu->MHz - c)/10);
	}
}

static void display_address_sizes(struct cpudata *cpu)
{
	printf("Address sizes : %u bits physical, %u bits virtual\n",cpu->phyaddr_bits,cpu->viraddr_bits);
}


static void display_detailed_info(struct cpudata *cpu)
{
	bind_cpu(cpu);	/* FIXME: Eventually remove once 'gather' has all the per-cpu stuff */
	show_info(cpu);

	if (show_registers) {
		dumpregs(cpu->number, 0, cpu->cpuid_level);
		if (cpu->maxei >=0x80000000)
			dumpregs (cpu->number, 0x80000000, cpu->maxei);

		if (cpu->maxei2 >=0xC0000000)
			dumpregs (cpu->number, 0xC0000000, cpu->maxei2);
	}

	if (show_cacheinfo == 1) {
		switch (cpu->vendor) {
		case VENDOR_INTEL:
			decode_Intel_caches(cpu, 1);
			break;
		case VENDOR_AMD:
			decode_AMD_cacheinfo(cpu);
			break;
		default:
			break;
		}
	}

	if (show_flags == 1)
		display_features(cpu);

	if (show_connector)
		decode_connector(cpu->connector);

	if (show_urls) {
		if (cpu->info_url != NULL)
			printf("Info URL: %s\n", cpu->info_url);
		if (cpu->datasheet_url != NULL)
			printf("Datasheet: %s\n", cpu->datasheet_url);
		if (cpu->errata_url != NULL)
			printf("Errata: %s\n", cpu->errata_url);
	}

	/* Info that requires root access (eg, reading MSRs etc) */
	if (user_is_root) {
		if (show_mtrr)
			dump_mtrrs(cpu);

		if (show_apic)
			dump_apics(cpu);
	}

	if (show_addr_sizes)
		display_address_sizes(cpu);

	if (show_MHz) {
		display_MHz(cpu);
		printf(" processor (estimate).\n\n");
	}

	if (show_bench)
		show_benchmarks(cpu);
}

/*
 * check to see if all CPUs are the same.
 * returns 1 if all cpus are the same, 0 if something is different
 */
static int check_cpu_similarity()
{
	struct cpudata *cpu;
	unsigned int i;

	/* force to display all cpus if the user requested it. */
	if (all_cpus == 1)
		return 0;

	cpu = firstcpu;
	if (nrCPUs > 1) {
		for (i = 0; i < nrCPUs; i++) {
			cpu = cpu->next;
			if (!cpu)
				return 1;

			if (cpu->efamily != firstcpu->efamily)
				return 0;
			if (cpu->emodel != firstcpu->emodel)
				return 0;
			if (cpu->family != firstcpu->family)
				return 0;
			if (model(cpu) != model(firstcpu))
				return 0;
			if (cpu->stepping != firstcpu->stepping)
				return 0;
		}
	}
	return 1;
}


int main (int argc, char **argv)
{
	struct cpudata *cpu=NULL, *head=NULL, *tmp;
	unsigned int i;
	unsigned int display_one_cpu = 1;

	if (getuid() != 0)
		user_is_root=0;

	parse_command_line(argc, argv);

	printf("x86info v1.28beta.  Dave Jones 2001-2011\n");
	printf("Feedback to <davej@redhat.com>.\n\n");

	if ((HaveCPUID()) == 0) {
		printf("No CPUID instruction available.\n");
		printf("No further information available for this CPU.\n");
		exit(EXIT_SUCCESS);
	}

	if (need_root && !user_is_root) {
		printf("Need to be root to use specified options.\n");
		exit(EXIT_FAILURE);
	}

	nrCPUs = sysconf(_SC_NPROCESSORS_ONLN);
	/*
	 * can't have less than 1 CPU, or more than
	 * 65535 (some arbitrary large number)
	 */
	if ((nrCPUs < 1) || (nrCPUs > 65535)) {
		printf("CPU count is bogus: defaulting to 1 CPU.\n");
		nrCPUs = 1;
	}


	/* First we gather information */
	for (i = 0; i < nrCPUs; i++) {
		cpu = malloc (sizeof (struct cpudata));
		if (!cpu) {
			printf("Out of memory\n");
			exit(EXIT_FAILURE);
		}
		if (!firstcpu)
			firstcpu = cpu;

		memset(cpu, 0, sizeof(struct cpudata));

		if (!head) {
			head = cpu;
		} else {
			cpu->next = head;
			head = cpu;
		}

		cpu->number = i;

		bind_cpu(cpu);

		estimate_MHz(cpu);
		get_cpu_info_basics(cpu);	/* get vendor,family,model,stepping */
		get_feature_flags(cpu);
		identify(cpu);
	}

	display_one_cpu = check_cpu_similarity();

	if (show_mptable && user_is_root)
		display_mptable();

	/* Now we display the info we gathered */
	cpu = firstcpu;

	if (display_one_cpu == 1) {
		if (nrCPUs >= 2)
			printf("Found %d identical CPUs", nrCPUs);

		/* Check mptable if present. This way we get number of CPUs
		   on SMP systems that have booted UP kernels. */
		if (user_is_root) {
			nrSMPCPUs = enumerate_cpus();

			if (nrSMPCPUs > nrCPUs)
				printf(" (but found %ud CPUs in MPTable!)", nrSMPCPUs);
		}
		printf("\n");

		display_detailed_info(cpu);
	} else {
		printf("Found %d CPUs.\n", nrCPUs);
		for (i = 0; i < nrCPUs; i++) {
			printf("CPU #%u:\n", i+1);

			display_detailed_info(cpu);

			if (nrCPUs > 1)
				separator();
		}
	}

	/* For now, we only support topology parsing on Intel. */
	if (cpu->vendor == VENDOR_INTEL)
		display_topology(head);

	printf(" running at an estimated ");
	display_MHz(cpu);
	printf("\n");


	/* Tear down the linked list. */
	cpu = head;
	for (i = 0; i < nrCPUs; i++) {
		if (cpu->info_url)
			free(cpu->info_url);
		if (cpu->datasheet_url)
			free(cpu->datasheet_url);
		if (cpu->errata_url)
			free(cpu->errata_url);
		tmp = cpu->next;
		free(cpu);
		cpu = tmp;
	}

	exit(EXIT_SUCCESS);
}
