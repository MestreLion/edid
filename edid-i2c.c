/* main() and argument parsing
 *
 * This file is part of edid-i2c, see <https://github.com/MestreLion/edid>
 * Copyright (C) 2023 Rodrigo Silva (MestreLion) <linux@rodrigosilva.com>
 * License: GPLv3 or later, at your choice. See <http://www.gnu.org/licenses/gpl>
 */

#include <errno.h>   // errno, strerror
#include <fcntl.h>   // open, O_RDONLY, O_RDWR
#include <stdarg.h>  // variable arguments: va_*, ...
#include <stdio.h>   // *printf, stderr, stdout, NULL, fwrite
#include <stdlib.h>  // NULL, EXIT_FAILURE, getenv, strtol
#include <string.h>  // strcmp, NULL
#include <stdbool.h> // bool, true, false
#include <unistd.h>  // read

#include <linux/i2c-dev.h>  // I2C_SLAVE
#include <sys/ioctl.h>      // ioctl

#define VERSION  "1.0"

#define I2C_ADDR         0x50  // TODO: get from <linux/i2c.h> or similar
#define EDID_BUS_AUTO    -1
#define EDID_BLOCK_SIZE  256

#define message(...) if (verbose) { fprintf(stderr, __VA_ARGS__); }

static const char * PROGNAME = NULL;  // == argv[0], set by main()
static bool verbose = false;


void usage(FILE* stream) {
	fprintf(stream, "Usage: %s [-h] [-v] [-b BUS] [-a ADDRESS]\n", PROGNAME);
}
void help() {
	usage(stdout);
	printf("Retrieve EDID data from a display via (E-)DDC / I²C.\n");
	printf("  -h|--help              Display this help\n");
	printf("  -v|--verbose           Output informative and debug messages over stderr.\n");
	printf("  -b|--bus      BUS      Use I²C bus BUS, if empty scans all available I²C busses.\n");
	printf("                            Can also be set by EDID_BUS environmental variable.\n");
	printf("  -a|--address  ADDRESS  Use I²C target address ADDRESS, in hexadecimal. [Default: %02xh]\n",
		I2C_ADDR);
	printf("\n");
	printf("%s version %s, licensed under the GPL version 3 or later, at your choice\n",
		PROGNAME, VERSION);
	printf("Copyright (C) 2023 Rodrigo Silva (MestreLion) <linux@rodrigosilva.com>\n");
}

// simple explain_output_error_and_die() using stdlib only
_Noreturn void fatal(const char *fmt, ...) {
	char msg[1000];

	va_list argp;
	va_start(argp, fmt);
	vsnprintf(msg, sizeof(msg), fmt, argp);
	va_end(argp);

	if (errno)
		fprintf(stderr, "%s: %s: %s\n", PROGNAME, msg, strerror(errno));
	else
		fprintf(stderr, "%s: %s\n", PROGNAME, msg);

	usage(stderr);
	exit(EXIT_FAILURE);
}

int parse_int(char *arg, char *name, int base, int range_min, int range_max, int default_if_empty) {
	long num;
	char *end;
	char range[100];
	char *range_fmt = (base == 16 ? "%02xh-%02xh" : "%d-%d");

	if (!*arg)
		return default_if_empty;
	num = strtol(arg, &end, base);
	if (*end)
		fatal("%s is not an integer: %s", name, arg);
	if (num < range_min || num > range_max) {
		snprintf(range, sizeof(range), range_fmt, range_min, range_max);
		fatal("%s out of range [%s]: %s", name, range, arg);
	}
	// No need to check for int limits, as range is always well within it.
	return num;
}

bool edid_i2c(int bus, int address, bool verbose) {
	if (verbose)
		printf("DEBUG mode\n");
	if (bus == EDID_BUS_AUTO) {
		// FIXME: actually loop the busses!
		bus = 8;
	}

	// Adapted from https://www.kernel.org/doc/Documentation/i2c/dev-interface
	// (which is heavily inspired by tools/i2cget.c from i2c-tools package)
	int file;
	char filename[20];
	char edid[EDID_BLOCK_SIZE];

	snprintf(filename, sizeof(filename), "/dev/i2c-%d", bus);
	file = open(filename, O_RDONLY); //, O_RDWR);  // intentionally not fopen()
	if (file < 0)
		fatal("could not open I²C bus %d [%s]", bus, filename);

	if (ioctl(file, I2C_SLAVE, address) < 0)
		fatal("could not set I²C address for display to %02xh", address);

	// Could also use one of i2c_smbus_read_*(), possibly in a loop
	if (read(file, &edid, sizeof(edid)) != sizeof(edid))
		fatal("reading EDID block (%d bytes)", EDID_BLOCK_SIZE);
	fwrite(edid, sizeof(edid), 1, stdout);
	close(file);

	return true;
}


int main(int argc, char* argv[]) {
	char* arg;

	int address = I2C_ADDR;
	int bus = (arg = getenv("EDID_BUS")) ?
		parse_int(arg, "EDID_BUS environment variable", 0, 0, 0xFFFFF, EDID_BUS_AUTO) :
		EDID_BUS_AUTO;

	PROGNAME = argv[0];

	while (--argc) {
		arg = *(++argv);
		if (!strcmp("-h", arg) || !strcmp("--help", arg)) {
			help();
			return 0;
		}
		if (!strcmp("-v", arg) || !strcmp("--verbose", arg)) {
			verbose = true;
			continue;
		}
		if (!strcmp("-b", arg) || !strcmp("--bus", arg)) {
			if (!--argc) fatal("missing argument BUS");
			bus = parse_int(*(++argv), "BUS", 0, 0, 0xFFFFF, EDID_BUS_AUTO);
			continue;
		}
		if (!strcmp("-a", arg) || !strcmp("--address", arg)) {
			if (!--argc) fatal("missing argument ADDRESS");
			address = parse_int(*(++argv), "ADDRESS", 16, 0x03, 0x77, I2C_ADDR);
			continue;
		}
		if (!strcmp("--", arg)) {
			argc--;
			break;
		}
		if (arg[0] == '-')
			fatal("invalid option: %s", arg);
		fatal("invalid argument: %s", arg);
	}
	if (argc)
		fatal("too many arguments: %s", *(++argv));

	message("Bus %d, Address %02xh\n", bus, address);
	if (!edid_i2c(bus, address, verbose))
		exit(EXIT_FAILURE);
}
