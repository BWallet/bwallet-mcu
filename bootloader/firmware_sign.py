#!/usr/bin/python
import argparse
import hashlib
import struct
import binascii
import ecdsa

SLOTS = 3

pubkeys = {
    1: '04b5bedc4854ac22af4f8895ed7aa435ecaced4f3eab41735d5cfbed5277db35c08cd01f2aeae7bb81af6756c7c19c9b9ca0c5a63c06e3134cb795a0c2929012e0',
    2: '04dcb85c8b2e1c16d935f1f8c5ee241f314b3c6476774599bcd13913ac9c29e57a836b766859626bc98492c678d428b75cc05891fb9c8d8b3fe5f3fa9de15b6a21',
    3: '04428c72f1b37b2ec9bdd5553e94443d3f2629d7702b5b8f63e7ab77a786073ec8184921fd2d2c60051e667689f13019cf4ac91e267d98857e0517f3b76fecb9d5',
    4: '0490f1ea6d4b2a06a1edcbc8145c2b0c23f729a2a43b392b7cd6ac2619a756780205243cd85b8909b74bb21955a58087ba666c9b4f8451d5eaa1bc7f66460f5793',
    5: '04aeb9f3af6129e44a83b897648f91307fc64ae5d9a85feebf442eba34a691e48197c8600762f7789a01744fc9f87c05c78943f15cb7eebae791734994a000a35b',
}

INDEXES_START = len('BDXW') + struct.calcsize('<I')
SIG_START = INDEXES_START + SLOTS + 1 + 52

def parse_args():
    parser = argparse.ArgumentParser(description='Commandline tool for signing Trezor firmware.')
    parser.add_argument('-f', '--file', dest='path', help="Firmware file to modify")
    parser.add_argument('-s', '--sign', dest='sign', action='store_true', help="Add signature to firmware slot")
    parser.add_argument('-p', '--pem', dest='pem', action='store_true', help="Use PEM instead of SECEXP")
    parser.add_argument('-g', '--generate', dest='generate', action='store_true', help='Generate new ECDSA keypair')

    return parser.parse_args()

def prepare(data):
    # Takes raw OR signed firmware and clean out metadata structure
    # This produces 'clean' data for signing

    meta = 'BDXW'  # magic
    if data[:4] == 'BDXW':
        meta += data[4:4 + struct.calcsize('<I')]
    else:
        meta += struct.pack('<I', len(data))  # length of the code
    meta += '\x00' * SLOTS  # signature index #1-#3
    meta += '\x01'       # flags
    meta += '\x00' * 52  # reserved
    meta += '\x00' * 64 * SLOTS  # signature #1-#3
#    print "meta : ", meta

    if data[:4] == 'BDXW':
        # Replace existing header
        out = meta + data[len(meta):]
    else:
        # create data from meta + code
        out = meta + data

    return out

def check_signatures(data):
    # Analyses given firmware and prints out
    # status of included signatures

    indexes = [ ord(x) for x in data[INDEXES_START:INDEXES_START + SLOTS] ]

    to_sign = prepare(data)[256:] # without meta
    fingerprint = hashlib.sha256(to_sign).hexdigest()

    print "Firmware fingerprint:", fingerprint

    used = []
    for x in range(SLOTS):
        signature = data[SIG_START + 64 * x:SIG_START + 64 * x + 64]

        if indexes[x] == 0:
            print "Slot #%d" % (x + 1), 'is empty'
        else:
            pk = pubkeys[indexes[x]]
            verify = ecdsa.VerifyingKey.from_string(binascii.unhexlify(pk)[1:],
                        curve=ecdsa.curves.SECP256k1, hashfunc=hashlib.sha256)

            try:
                verify.verify(signature, to_sign, hashfunc=hashlib.sha256)

                if indexes[x] in used:
                    print "Slot #%d signature: DUPLICATE" % (x + 1), binascii.hexlify(signature)
                else:
                    used.append(indexes[x])
                    print "Slot #%d signature: VALID" % (x + 1), binascii.hexlify(signature)

            except:
                print "Slot #%d signature: INVALID" % (x + 1), binascii.hexlify(signature)


def modify(data, slot, index, signature):
    # Replace signature in data

    # Put index to data
    data = data[:INDEXES_START + slot - 1 ] + chr(index) + data[INDEXES_START + slot:]

    # Put signature to data
    data = data[:SIG_START + 64 * (slot - 1) ] + signature + data[SIG_START + 64 * slot:]

    return data

def sign(data, is_pem):
    # Ask for index and private key and signs the firmware

    slot = int(raw_input('Enter signature slot (1-%d): ' % SLOTS))
    if slot < 1 or slot > SLOTS:
        raise Exception("Invalid slot")

    if is_pem:
        print "Paste ECDSA private key in PEM format and press Enter:"
        print "(blank private key removes the signature on given index)"
        pem_key = ''
        while True:
            key = raw_input()
            pem_key += key + "\n"
            if key == '':
                break
        if pem_key.strip() == '':
            # Blank key,let's remove existing signature from slot
            return modify(data, slot, 0, '\x00' * 64)
        key = ecdsa.SigningKey.from_pem(pem_key)
    else:
        print "Paste SECEXP (in hex) and press Enter:"
        print "(blank private key removes the signature on given index)"
        secexp = raw_input()
        if secexp.strip() == '':
            # Blank key,let's remove existing signature from slot
            return modify(data, slot, 0, '\x00' * 64)
        key = ecdsa.SigningKey.from_secret_exponent(secexp = int(secexp, 16), curve=ecdsa.curves.SECP256k1, hashfunc=hashlib.sha256)
        print "key:"
        print key

    to_sign = prepare(data)[256:] # without meta

    # Locate proper index of current signing key
    pubkey = '04' + binascii.hexlify(key.get_verifying_key().to_string())
    index = None
    for i, pk in pubkeys.iteritems():
        if pk == pubkey:
            index = i
            break

    if index == None:
        raise Exception("Unable to find private key index. Unknown private key?")

    signature = key.sign_deterministic(to_sign, hashfunc=hashlib.sha256)

    print "signature:"
    print binascii.hexlify(signature)

    return modify(data, slot, index, signature)

def main(args):
    if args.generate:
        key = ecdsa.SigningKey.generate(
            curve=ecdsa.curves.SECP256k1,
            hashfunc=hashlib.sha256)

        print "PRIVATE KEY (SECEXP):"
        print binascii.hexlify(key.to_string())
        print

        print "PRIVATE KEY (PEM):"
        print key.to_pem()

        print "PUBLIC KEY:"
        print '04' + binascii.hexlify(key.get_verifying_key().to_string())
        return

    if not args.path:
        raise Exception("-f/--file is required")

    data = open(args.path, 'rb').read()
    assert len(data) % 4 == 0

    if data[:4] != 'BDXW':
        print "Metadata has been added..."
        data = prepare(data)

    if data[:4] != 'BDXW':
        raise Exception("Firmware header expected")

    print "Firmware size %d bytes" % len(data)

    check_signatures(data)

    if args.sign:
        data = sign(data, args.pem)
        check_signatures(data)

    fp = open(args.path, 'w')
    fp.write(data)
    fp.close()

if __name__ == '__main__':
    args = parse_args()
    main(args)
