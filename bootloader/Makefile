NAME  = bootloader

MCU_DIR=$(subst /bootloader,,$(PWD))

OBJS += bootloader.o
OBJS += signatures.o
OBJS += usb.o

OBJS += $(MCU_DIR)/bwallet-crypto/bignum.small.o
OBJS += $(MCU_DIR)/bwallet-crypto/ecdsa.small.o
OBJS += $(MCU_DIR)/bwallet-crypto/hmac.o
OBJS += $(MCU_DIR)/bwallet-crypto/ripemd160.o
OBJS += $(MCU_DIR)/bwallet-crypto/secp256k1.small.o
OBJS += $(MCU_DIR)/bwallet-crypto/sha2.o

CFLAGS += -DUSE_PRECOMPUTED_IV=0
CFLAGS += -DUSE_PRECOMPUTED_CP=0
CFLAGS += -DUSE_PUBKEY_VALIDATE=0

include $(MCU_DIR)/Makefile.include

align:
	./firmware_align.py $(NAME).bin
