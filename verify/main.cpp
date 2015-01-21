
#include <iostream>
#include <cstdio>
#include <getopt.h>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <sysexits.h>

#include "verfiy.hpp"

using namespace std;

static void help(void)
{
	fprintf(stderr, "Usage: verfiy [options] ...\n"
			"  -h --help\t\t\tPrint this help message\n"
			"  -v --version\t\t\tPrint the version number\n");
	fprintf(stderr, "  -b --boot \t\t\tverfiy bootloader hash vaule. eg : --boot [bootloader.bin]\n"
			"  -f --firmware \t\tverfiy firmware hash vaule, eg : --firmware [bwallet-x.x.x.bin]\n");
	exit(EX_USAGE);

}

static void print_version(void)
{
	printf("verfiy 0.0.1\n\n");
	printf("Copyright 2015 kevin for Bidingxing.ltd\n"
			"This program is Bidingxing.ltd to verfiy bootloader and firmware hash vaule\n\n\n");
}



static struct option opts[] = { 
    { "help", 0, 0, 'h'  },
    { "version", 0, 0, 'v'  },
    { "boot", 1, 0, 'b'  },
    { "firmware", 1, 0, 'f'  },
    { 0, 0, 0, 0  } 
};


int main(int argc, char *argv[])
{
	int  c, options_index = 0;
	enum mode mode = MODE_NONE;
	struct verfiy_file file;
	setvbuf(stdout, NULL, _IONBF, 0);
	
	if(argc == 1) {
		help();
	}

	file.boot = (struct file *)verfiy_malloc(sizeof(struct file));	
	file.firmware = (struct file *)verfiy_malloc(sizeof(struct file));	

	file.boot->flag = false;
	file.firmware->flag = false;

	while(1) {
		c = getopt_long(argc , argv , "hvb:f:", opts, &options_index);
		if(c == -1)
			break;

		switch (c) {
			case 'h' :
				help();
				break;
			case 'v' :
				mode = MODE_VERSION;
				print_version();
				break;
			case 'b' :
				mode = MODE_BOOT;
				bootloader_hash(&file, optarg);
				break;
			case 'f' :
				mode = MODE_FIRMWARE;
				firmware_hash(&file, optarg);
				break;
			default :
				help();
				break;
		}
	}
	if(mode == MODE_NONE)
		help();
	
	free(file.boot);
	free(file.firmware);
	return 0;
}
