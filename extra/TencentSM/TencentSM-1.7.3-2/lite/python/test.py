from tencentsm import version 
from tencentsm import SM2Init
from tencentsm import SM2Free
from tencentsm import generatePrivateKey 
from tencentsm import generatePublicKey 
from tencentsm import generateKeyPair 
from tencentsm import SM2Encrypt 
from tencentsm import SM2Decrypt 
from tencentsm import SM2Sign 
from tencentsm import SM2Verify
from tencentsm import SM2EncryptWithMode
from tencentsm import SM2DecryptWithMode
from tencentsm import SM2SignWithMode
from tencentsm import SM2VerifyWithMode

from tencentsm import SM3Init 
from tencentsm import SM3Update 
from tencentsm import SM3Final

from tencentsm import SM4_CBC_Encrypt 
from tencentsm import SM4_CBC_Decrypt 
from tencentsm import SM4_ECB_Encrypt
from tencentsm import SM4_ECB_Decrypt 
from tencentsm import generateSM4Key

from enum import Enum, unique

import binascii


result  =  version()
print  "verison:"+result

print "--------------------Test Case For SM2--------------------"

SM2Init()

result = generatePrivateKey()
print "generatePrivateKey:"+result

result = generatePublicKey(result)
print "generatePublicKey:"+result

result = generateKeyPair()
print "generateKeyPair:private key is----"+result[0]
print "generateKeyPair:public key is----"+result[1]


plain_hex = 'd12b0692660000c6e037faeca4245eca8f5c245dec77b377b63334115afa71051d9a1605aa7c89b64d5a0953f00f'
plain_str = str(bytearray.fromhex(plain_hex))

cipher = SM2Encrypt(plain_str,result[1])

cipher_list = [ord(c) for c in cipher]
cipher_hex = binascii.b2a_hex(str(bytearray(cipher_list)))
print "SM2 Encrypt binary data, Cipher is -------"+cipher_hex

decrypt_plain = SM2Decrypt(cipher,result[0])

if plain_str == decrypt_plain:
    print "SM2 Decrypt Success"
else:
    print "SM2 Decrypt Failed"


cipher = SM2Encrypt('hello',result[1])
cipher_list = [ord(c) for c in cipher]
cipher_hex = binascii.b2a_hex(str(bytearray(cipher_list)))
print "SM2 Encrypt char data, Cipher is -------"+cipher_hex

decrypt_plain = SM2Decrypt(cipher,result[0])

if 'hello' == decrypt_plain:
    print "SM2 Decrypt Success"
else:
    print "SM2 Decrypt Failed"

@unique
class SM2CipherMode(Enum):
    SM2CipherMode_C1C3C2_ASN1 = 0
    SM2CipherMode_C1C3C2 = 1
    SM2CipherMode_C1C2C3_ASN1 = 2
    SM2CipherMode_C1C2C3 = 3
    SM2CipherMode_04C1C3C2 = 4
    SM2CipherMode_04C1C2C3 = 5
    
mode = SM2CipherMode.SM2CipherMode_C1C3C2

cipher = SM2EncryptWithMode(plain_str,result[1],mode.value)

cipher_list = [ord(c) for c in cipher]
cipher_hex = binascii.b2a_hex(str(bytearray(cipher_list)))
print "SM2 EncryptWithMode binary data, Cipher is -------"+cipher_hex

decrypt_plain = SM2DecryptWithMode(cipher,result[0],mode.value)

if plain_str == decrypt_plain:
    print "SM2 DecryptWithMode Success"
else:
    print "SM2 DecryptWithMode Failed"

mode = SM2CipherMode.SM2CipherMode_04C1C3C2

cipher = SM2EncryptWithMode('hello',result[1],mode.value)
cipher_list = [ord(c) for c in cipher]
cipher_hex = binascii.b2a_hex(str(bytearray(cipher_list)))
print "SM2 EncryptWithMode char data, Cipher is -------"+cipher_hex

decrypt_plain = SM2DecryptWithMode(cipher,result[0],mode.value)

if 'hello' == decrypt_plain:
    print "SM2 DecryptWithMode Success"
else:
    print "SM2 DecryptWithMode Failed"

signature = SM2Sign('message','1234567812345678',result[1],result[0])

signature_list = [ord(c) for c in signature]
signature_hex = binascii.b2a_hex(str(bytearray(signature_list)))
print "SM2 Sign char message, signature is -------"+signature_hex

verify_code = SM2Verify('message','1234567812345678',signature,result[1])
if 0 == verify_code:
    print "SM2 Verify Success"
else:
    print "SM2 Verify Failed"

@unique
class SM2CipherMode(Enum):
    SM2SignMode_RS_ASN1 = 0
    SM2SignMode_RS = 1

mode = SM2CipherMode.SM2SignMode_RS

signature = SM2SignWithMode('message','1234567812345678',result[1],result[0],mode.value)

signature_list = [ord(c) for c in signature]
signature_hex = binascii.b2a_hex(str(bytearray(signature_list)))
print "SM2 SignWithMode char message, signature is -------"+signature_hex

verify_code = SM2VerifyWithMode('message','1234567812345678',signature,result[1],mode.value)
if 0 == verify_code:
    print "SM2 VerifyWithMode Success"
else:
    print "SM2 VerifyWithMode Failed"

SM2Free()

print "--------------------Test Case For SM3--------------------"

SM3Init()
SM3Update('hello')
digest = SM3Final();

digest_list = [ord(c) for c in digest]
digest_hex = binascii.b2a_hex(str(bytearray(digest_list)))
print "SM3 digest is -------"+digest_hex

print "--------------------Test Case For SM4--------------------"

key = generateSM4Key()
key_list = [ord(c) for c in key]
key_hex = binascii.b2a_hex(str(bytearray(key_list)))
print "key is -------"+key_hex

iv = generateSM4Key()
iv_list = [ord(c) for c in iv]
iv_hex = binascii.b2a_hex(str(bytearray(iv_list)))
print "iv is -------"+iv_hex

cipher = SM4_CBC_Encrypt('hello world!',key,iv)
cipher_list = [ord(c) for c in cipher]
cipher_hex = binascii.b2a_hex(str(bytearray(cipher_list)))
print "SM4 CBC Encrypt char data, Cipher is -------"+cipher_hex

decrypt_plain = SM4_CBC_Decrypt(cipher,key,iv)

if 'hello world!' == decrypt_plain:
    print "SM4 CBC Decrypt Success"
else:
    print "SM4 CBC Decrypt Failed"

cipher = SM4_ECB_Encrypt('hello world!',key)
cipher_list = [ord(c) for c in cipher]
cipher_hex = binascii.b2a_hex(str(bytearray(cipher_list)))
print "SM4 ECB Encrypt char data, Cipher is -------"+cipher_hex

decrypt_plain = SM4_ECB_Decrypt(cipher,key)

if 'hello world!' == decrypt_plain:
    print "SM4 ECB Decrypt Success"
else:
    print "SM4 ECB Decrypt Failed"