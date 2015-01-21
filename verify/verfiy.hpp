
#ifndef __VERFIY_H__
#define __VERFIY_H__

#define BOOT_ALIGN_SIZE 32768

#include <stdint.h>

enum mode {
	MODE_NONE,
	MODE_VERSION,
	MODE_BOOT,
	MODE_FIRMWARE,
};

struct file {
	uint8_t *file;
	uint8_t *file_align;
	uint8_t hash[64];
	int	size;
	bool flag;
};

struct verfiy_file {
	struct file *boot;
	struct file *firmware;	
};

void *verfiy_malloc(size_t size);
void bootloader_hash(struct verfiy_file *boot_file, const char *boot_path);
void firmware_hash(struct verfiy_file *firmware_file, const char *firmware_path);


#endif //__VERFIY_H__
