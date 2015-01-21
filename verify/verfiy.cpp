
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <cerrno>
#include <err.h>
#include <sysexits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/sha.h>

#include "verfiy.hpp"

static char *tohex(const uint8_t *bin, size_t l)
{
	char *buf = (char *)malloc(l * 2 + 1);
	static char digits[] = "0123456789abcdef";
	size_t i;
	for (i = 0; i < l; i++) {
		buf[i*2  ] = digits[(bin[i] >> 4) & 0xF];
		buf[i*2+1] = digits[bin[i] & 0xF];
	}
	buf[l * 2] = 0;
	return buf;
}

void *verfiy_malloc(size_t size)
{
	void *ptr = malloc(size);
	if (ptr == NULL)
		errx(EX_SOFTWARE, "Cannot allocate memory of size %d bytes", (int)size);
	return (ptr);
}

void sha256_Raw(const uint8_t *data, size_t len, uint8_t digest[SHA256_DIGEST_LENGTH]) 
{
	SHA256_CTX  context;
	SHA256_Init(&context);
	SHA256_Update(&context, data, len);
	SHA256_Final(digest, &context);
}

static inline void read_file(struct verfiy_file *file, const char *file_path)
{
	int fd;
	off_t offset;
	
	if(access(file_path, R_OK))
		err(EX_IOERR, NULL);
	fd = open(file_path, O_RDONLY);
	offset = lseek(fd, 0, SEEK_END);
	if ((int)offset < 0 || (int)offset != offset)
		err(EX_IOERR, "File size is too big");

	if (lseek(fd, 0, SEEK_SET) != 0)
		err(EX_IOERR, "Could not seek to beginning");	

	if(file->boot->flag) {
		file->boot->size = offset;
		file->boot->file = (unsigned char *)verfiy_malloc(file->boot->size);
		if (read(fd, file->boot->file, file->boot->size) != file->boot->size) {
			err(EX_IOERR, "Could not read %d bytes from %s", file->boot->size, file_path);
		}
		file->boot->flag = false;
	}

	if(file->firmware->flag) {
		file->firmware->size = offset;
		file->firmware->file = (unsigned char *)verfiy_malloc(file->firmware->size);
		if (read(fd, file->firmware->file, file->firmware->size) != file->firmware->size) {
			err(EX_IOERR, "Could not read %d bytes from %s", file->firmware->size, file_path);
		}
		file->firmware->flag = false;
	}
		
	close(fd);

}

void bootloader_hash(struct verfiy_file *boot_file, const char *boot_path)
{
	boot_file->boot->flag = true;
	read_file(boot_file, boot_path);
	if(boot_file->boot->size < BOOT_ALIGN_SIZE) {
		boot_file->boot->file_align = (unsigned char *)verfiy_malloc(BOOT_ALIGN_SIZE); 
		memset(boot_file->boot->file_align, 0xff, BOOT_ALIGN_SIZE);
		memcpy(boot_file->boot->file_align, boot_file->boot->file, boot_file->boot->size);
		sha256_Raw(boot_file->boot->file_align, BOOT_ALIGN_SIZE, boot_file->boot->hash);
		sha256_Raw(boot_file->boot->hash, 32, boot_file->boot->hash);
		free(boot_file->boot->file);
		free(boot_file->boot->file_align);
	} else {
		sha256_Raw(boot_file->boot->file, boot_file->boot->size, boot_file->boot->hash);
		sha256_Raw(boot_file->boot->hash, 32, boot_file->boot->hash);	
		free(boot_file->boot->file);
	}
	printf("bootloader hash : %s\n", tohex(boot_file->boot->hash, 32));	
}

void firmware_hash(struct verfiy_file *firmware_file, const char *firmware_path)
{
	firmware_file->firmware->flag = true;
	read_file(firmware_file, firmware_path);
	sha256_Raw((const unsigned char *)firmware_file->firmware->file, firmware_file->firmware->size, firmware_file->firmware->hash);
	free(firmware_file->firmware->file);
	printf("firmware hash : %s\n", tohex(firmware_file->firmware->hash, 32));
}
