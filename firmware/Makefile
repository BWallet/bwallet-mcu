APPVER = 1.3.0

NAME  = bwallet

OBJS += usb.o
OBJS += messages.o
OBJS += storage.o
OBJS += bwallet.o
OBJS += pinmatrix.o
OBJS += fsm.o
OBJS += coins.o
OBJS += transaction.o
OBJS += protect.o
OBJS += layout2.o
OBJS += recovery.o
OBJS += reset.o
OBJS += signing.o
OBJS += crypto.o

OBJS += debug.o

OBJS += ../bwallet-crypto/bignum.o
OBJS += ../bwallet-crypto/ecdsa.o
OBJS += ../bwallet-crypto/secp256k1.o
OBJS += ../bwallet-crypto/hmac.o
OBJS += ../bwallet-crypto/bip32.o
OBJS += ../bwallet-crypto/bip39.o
OBJS += ../bwallet-crypto/pbkdf2.o
OBJS += ../bwallet-crypto/base58.o

OBJS += ../bwallet-crypto/ripemd160.o
OBJS += ../bwallet-crypto/sha2.o

OBJS += ../bwallet-crypto/aescrypt.o
OBJS += ../bwallet-crypto/aeskey.o
OBJS += ../bwallet-crypto/aestab.o
OBJS += ../bwallet-crypto/aes_modes.o

OBJS += ../bwallet-qrenc/qr_encode.o

#OBJS += protob/pb_common.o
OBJS += protob/pb_decode.o
OBJS += protob/pb_encode.o
OBJS += protob/messages.pb.o
OBJS += protob/storage.pb.o
OBJS += protob/types.pb.o

include ../Makefile.include

CFLAGS += -Wno-sequence-point
CFLAGS += -Iprotob -DPB_FIELD_16BIT=1
CFLAGS += -DQR_MAX_VERSION=0
CFLAGS += -DUSE_PUBKEY_VALIDATE=0
CFLAGS += -DDEBUG_LINK=0
CFLAGS += -DDEBUG_LOG=0
CFLAGS += -DSCM_REVISION='"$(shell git rev-parse HEAD | sed 's:\(..\):\\x\1:g')"'
