package sm2

/*
#include "tencentsm/sm.h"
#include "tencentsm/tc.h"
#include "tencentsm/tc_sm2.h"
#include "tencentsm/tc_utils.h"
#include "tencentsm/tc_asn1.h"
#include <string.h>

int genertate_key_pair(
	sm2_ctx_t* ctx,
	void* sk,
	void* pk)
{
	int ret;
	SM2InitCtx(ctx);
	ret = generateKeyPair(ctx, (char*)sk, (char*)pk);
	SM2InitCtxWithPubKey(ctx, (char*)pk);

	return ret;
}

int sm2_encrypt(
	sm2_ctx_t* ctx,
	void* msg, size_t msg_len,
	void* pk, size_t pk_len,
	void* cipher,
	size_t* cipher_len,
	SM2CipherMode mode)
{
	return SM2EncryptWithMode(
		ctx,
		(unsigned char*)msg, msg_len,
		(unsigned char*)pk, pk_len,
		(unsigned char*)cipher,
		cipher_len, mode);
}

int sm2_decrypt(
	sm2_ctx_t* ctx,
	void* cipher,
	size_t cipher_len,
	void* sk,
	size_t sk_len,
	void* msg,
	size_t* msg_len,
	SM2CipherMode mode)
{
	return SM2DecryptWithMode(
		ctx,
		(unsigned char*)cipher,
		cipher_len,
		(unsigned char*)sk,
		sk_len,
		(unsigned char*)msg,
		msg_len, mode);
}

int sm2_sign_with_sm3(
	sm2_ctx_t* ctx,
	void* msg,
	size_t msg_len,
	void* id,
	size_t id_len,
	void* pk,
	size_t pk_len,
	void* sk,
	size_t sk_len,
	void* sig,
	size_t* sig_len,
	SM2SignMode mode)
{
	return SM2SignWithMode(
		ctx,
		(unsigned char*)msg,
		msg_len,
		(unsigned char*)id,
		id_len,
		(unsigned char*)pk,
		pk_len,
		(unsigned char*)sk,
		sk_len,
		(unsigned char*)sig,
		sig_len,
		mode);
}

int sm2_verify_with_sm3(
	sm2_ctx_t* ctx,
	void* msg,
	size_t msg_len,
	void* id,
	size_t id_len,
	oid* sig,
	size_t sig_len,
	void* pk,
	size_t pk_len,
	SM2SignMode mode)
{
	return SM2VerifyWithMode(
		ctx,
		(unsigned char*)msg,
		msg_len,
		(unsigned char*)id,
		id_len,
		(unsigned char*)sig,
		sig_len,
		(unsigned char*)pk,
		pk_len, mode);
}

int sm2_sign(
	sm2_ctx_t* ctx,
	void* dgst,
	size_t dgst_len,
	void* pk,
	size_t pk_len,
	void* sk,
	size_t sk_len,
	void* sig,
	size_t* sig_len,
	SM2SignMode mode)
{
	if (sk_len != 64)
	{
		tc_printf("sm2 sign argument private key length error.  %d %d\n", pk_len, sk_len);
		return -10001;
	}

	tc_ec_group_t* sm2group = (tc_ec_group_t*)ctx->group;

	int pub_i,pri_i = 0;
	int ret;
	tc_bn_t* prik = lock_temp_bn(ctx, &pri_i);
	tc_ec_t* pubk = lock_temp_ec(ctx, &pub_i);
	private_key_set_str((unsigned char*)sk,*prik);
	public_key_set_str((unsigned char*)pk, *pubk);

	sm2_signature sign;
	ret = tc_sm2_sign(ctx, *prik, (unsigned char*)dgst, (unsigned int)dgst_len, &sign);
	if (ret != 0)
	{
		tc_printf("sm2 sign error ! ret = %d!\n", ret);
		goto end;
	}

	switch (mode)
	{
		case SM2SignMode_RS_ASN1:
		{
			unsigned char r_encode[40];
			int r_encode_len = 0;
			tc_asn1_encode_integer(sign.r, TC_ECCref_MAX_LEN, r_encode, &r_encode_len);

			unsigned char s_encode[40];
			int s_encode_len = 0;
			tc_asn1_encode_integer(sign.s, TC_ECCref_MAX_LEN, s_encode, &s_encode_len);

			unsigned char rs_encode[80];
			memcpy(rs_encode, r_encode, r_encode_len);
			memcpy(rs_encode + r_encode_len , s_encode, s_encode_len);

			int encode_len = 0;
			tc_asn1_encode_sequence(rs_encode, r_encode_len + s_encode_len, sig, &encode_len);
			*sig_len = encode_len;
		}
			break;
		case SM2SignMode_RS:
		{
			memcpy(sig, sign.r, TC_ECCref_MAX_LEN);
			memcpy(sig + TC_ECCref_MAX_LEN, sign.s, TC_ECCref_MAX_LEN);
			*sig_len = 2*TC_ECCref_MAX_LEN;
		}
			break;
		default:
			break;
	}

end:
	unlock_temp_ec(ctx, pub_i);
	unlock_temp_bn(ctx, pri_i);
	return ret;
}

int sm2_verify(
	sm2_ctx_t* ctx,
	void* dgst,
	size_t dgst_len,
	void* sig,
	size_t sig_len,
	void* pk,
	size_t pk_len,
	SM2SignMode mode)
{
	int ret;
	if (pk_len != 130)
	{
		tc_printf("sm2 verify argument public length error.  %d \n", ret, pk_len);
		return -10001;
	}

	if (sig_len < 9)
	{
		tc_printf("sm2 verify argument sig length error.\n", ret);
	}

	sm2_signature sign;
	switch (mode)
	{
		case SM2SignMode_RS_ASN1:
		{
			int rs_offset = 0;
			int rs_outlen = 0;

			ret = tc_asn1_decode_object(sig, (int)sig_len, &rs_offset, &rs_outlen);

			if (ret != 0)
			{
				tc_printf("sm2 verify argument sig format error. \n", ret);
				return -11002;
			}
			else
			{
				int r_offset = 0;
				int r_outlen = 0;

				ret = tc_asn1_decode_object(sig + rs_offset, rs_outlen, &r_offset, &r_outlen);

				if (ret != 0 || r_outlen > 33)
				{
					tc_printf("sm2 verify argument sig format error. \n", ret);
					return -11001;
				}

				if (r_outlen == 33)
				{
					memcpy(sign.r, sig + rs_offset + r_offset + 1, 32);
				}
				else if(r_outlen == 32)
				{
					memcpy(sign.r, sig + rs_offset + r_offset, 32);
				}
				else
				{
					int z = 32 - r_outlen;
					memset(sign.r, 0x00, z);
					memcpy(sign.r + z, sig + rs_offset + r_offset, r_outlen);
				}

				int s_offset = 0;
				int s_outlen = 0;

				ret = tc_asn1_decode_object(
					sig + rs_offset + r_offset + r_outlen,
					rs_outlen - r_offset - r_outlen,
					&s_offset,
					&s_outlen);

				if (ret != 0 || s_outlen > 33)
				{
					tc_printf("sm2 verify argument sig format error. \n", ret);
					return -11001;
				}

				if (s_outlen == 33)
				{
					memcpy(sign.s, sig + rs_offset + r_offset + r_outlen + s_offset + 1, 32);
				}
				else if(s_outlen == 32)
				{
					memcpy(sign.s, sig + rs_offset + r_offset + r_outlen + s_offset, 32);
				}
				else
				{
					int z = 32 - s_outlen;
					memset(sign.s, 0x00, z);
					memcpy(sign.s + z, sig + rs_offset + r_offset + r_outlen + s_offset, s_outlen);
				}

				{
					unsigned char plainbuf[TC_ECCref_MAX_LEN*2];
					char plainbufstr[TC_ECCref_MAX_LEN*4+1];
					memset(plainbuf, 0x00, TC_ECCref_MAX_LEN*2);
					memset(plainbufstr, 0x00, TC_ECCref_MAX_LEN*4+1);
					memcpy(plainbuf, sign.r, TC_ECCref_MAX_LEN);
					memcpy(plainbuf+TC_ECCref_MAX_LEN, sign.s, TC_ECCref_MAX_LEN);
					tc_bin2hex((const unsigned char*)plainbuf, TC_ECCref_MAX_LEN * 2, plainbufstr, TC_ECCref_MAX_LEN*4+1);
				}
			}
		}
		break;
	case SM2SignMode_RS:
	{
		memcpy(sign.r, sig, TC_ECCref_MAX_LEN);
		memcpy(sign.s, sig + TC_ECCref_MAX_LEN, TC_ECCref_MAX_LEN);
    }
		break;
	default:
		break;
	}

	tc_ec_group_t* sm2group = (tc_ec_group_t*)ctx->group;

	int pub_i = 0;
	tc_ec_t* pubk = lock_temp_ec(ctx, &pub_i);
	public_key_set_str(pk, *pubk);

	if ((ret = tc_sm2_verify(ctx, *pubk, &sign, dgst, (unsigned int)dgst_len)) != 0)
	{
		tc_printf("sm2 verify error ! ret = %d!\n", ret);
		goto end;
	}

end:
	unlock_temp_ec(ctx, pub_i);
	return ret;
}
*/
import "C"

import (
	"bytes"
	"crypto/elliptic"
	"errors"
	"fmt"
	"math/big"
	"unsafe"
)

type PrivateKey struct {
	PublicKey
	D    *big.Int
	Text []byte
}
type PublicKey struct {
	elliptic.Curve
	X, Y *big.Int
	Text []byte
	ctx  *C.sm2_ctx_t
}

func GenerateKeyPair() (*PrivateKey, *PublicKey, error) {
	var sk [SM2_PRIVATE_KEY_STR_LEN + 1]byte
	var pk [SM2_PUBLIC_KEY_STR_LEN + 1]byte
	var ctx C.sm2_ctx_t

	ret := int(C.genertate_key_pair(&ctx, unsafe.Pointer(&sk[0]), unsafe.Pointer(&pk[0])))
	if ret != 0 {
		return nil, nil, fmt.Errorf("fail to generate SM2 key pair: internal error")
	}
	sk[SM2_PRIVATE_KEY_SIZE] = '\x00'
	pk[SM2_PUBLIC_KEY_SIZE] = '\x00'

	skD := new(big.Int)
	skD, ok := skD.SetString(string(sk[0:SM2_PRIVATE_KEY_SIZE]), 16)
	if !ok {
		return nil, nil, fmt.Errorf("fail to generate SM2 key pair: wrong private key")
	}

	pkX := new(big.Int)
	pkX, ok = pkX.SetString(string(pk[2:(SM2_PUBLIC_KEY_STR_LEN/2)]), 16)
	if !ok {
		return nil, nil, fmt.Errorf("fail to generate SM2 key pair: wrong public key")
	}
	pkY := new(big.Int)
	pkY, ok = pkY.SetString(string(pk[(SM2_PUBLIC_KEY_STR_LEN/2):SM2_PUBLIC_KEY_SIZE]), 16)
	if !ok {
		return nil, nil, fmt.Errorf("fail to generate SM2 key pair: wrong public key")
	}

	pkStruct := PublicKey{
		Curve: P256Sm2(),
		X:     pkX,
		Y:     pkY,
		Text:  pk[0:SM2_PUBLIC_KEY_STR_LEN],
		ctx:   &ctx,
	}

	skStruct := PrivateKey{
		PublicKey: pkStruct,
		D:         skD,
		Text:      sk[0:SM2_PRIVATE_KEY_STR_LEN],
	}

	return &skStruct, &pkStruct, nil
}

func (pk *PublicKey) EncryptWithMode(msg []byte, mode int) ([]byte, error) {
	if msg == nil {
		return nil, errors.New("SM2 encrypt: plaintext is null")
	}
	var cipherLen C.size_t
	cipher := make([]byte, len(msg)+SM2_CIPHER_EXTRA_SIZE)
	pkByte := pk.Text
	ret := C.sm2_encrypt(
		pk.ctx,
		unsafe.Pointer(&msg[0]),
		C.size_t(len(msg)),
		unsafe.Pointer(&pkByte[0]),
		SM2_PUBLIC_KEY_SIZE,
		unsafe.Pointer(&cipher[0]),
		&cipherLen,
		C.SM2CipherMode(mode))
	if ret != 0 {
		return nil, errors.New("SM2: fail to encrypt")
	}
	return cipher[0:int(cipherLen)], nil
}

func (sk *PrivateKey) DecryptWithMode(cipher []byte, mode int) ([]byte, error) {
	if cipher == nil {
		return nil, errors.New("SM2 decrypt: ciphertext is null")
	}
	var plainLen C.size_t
	plain := make([]byte, len(cipher))
	skByte := sk.Text
	ret := C.sm2_decrypt(
		sk.ctx,
		unsafe.Pointer(&cipher[0]),
		C.size_t(len(cipher)),
		unsafe.Pointer(&skByte[0]),
		SM2_PRIVATE_KEY_SIZE,
		unsafe.Pointer(&plain[0]),
		&plainLen,
		C.SM2CipherMode(mode))
	if ret != 0 {
		return nil, errors.New("SM2: fail to decrypt")
	}
	return plain[0:int(plainLen)], nil
}

func (pk *PublicKey) Encrypt(msg []byte) ([]byte, error) {
	return pk.EncryptWithMode(msg, SM2_CIPHER_MODE_C1C3C2_ASN1)
}

func (sk *PrivateKey) Decrypt(cipher []byte) ([]byte, error) {
	return sk.DecryptWithMode(cipher, SM2_CIPHER_MODE_C1C3C2_ASN1)
}

func (sk *PrivateKey) SignWithSM3WithMode(msg, id []byte, mode int) ([]byte, error) {
	if msg == nil || id == nil {
		return nil, errors.New("SM2 sign: message is null")
	}
	var sigLen C.size_t
	sig := make([]byte, SM2_SIGNATURE_MAX_SIZE)
	skByte := sk.Text
	pkByte := sk.PublicKey.Text
	ret := C.sm2_sign_with_sm3(
		sk.ctx,
		unsafe.Pointer(&msg[0]),
		C.size_t(len(msg)),
		unsafe.Pointer(&id[0]),
		C.size_t(len(id)),
		unsafe.Pointer(&pkByte[0]),
		SM2_PUBLIC_KEY_SIZE,
		unsafe.Pointer(&skByte[0]),
		SM2_PRIVATE_KEY_SIZE,
		unsafe.Pointer(&sig[0]),
		&sigLen,
		C.SM2SignMode(mode))
	if ret != 0 {
		return nil, errors.New("SM2: fail to sign message")
	}
	return sig[0:int(sigLen)], nil
}

func (pk *PublicKey) VerifyWithSM3WithMode(msg, id, sig []byte, mode int) bool {
	if msg == nil || id == nil || sig == nil {
		return false
	}
	pkByte := pk.Text
	ret := C.sm2_verify_with_sm3(
		pk.ctx,
		unsafe.Pointer(&msg[0]),
		C.size_t(len(msg)),
		unsafe.Pointer(&id[0]),
		C.size_t(len(id)),
		unsafe.Pointer(&sig[0]),
		C.size_t(len(sig)),
		unsafe.Pointer(&pkByte[0]),
		SM2_PUBLIC_KEY_SIZE,
		C.SM2SignMode(mode))
	if ret != 0 {
		return false
	}
	return true
}

func (sk *PrivateKey) SignWithSM3(msg, id []byte) ([]byte, error) {
	return sk.SignWithSM3WithMode(msg, id, SM2_SIGNATURE_MODE_RS_ASN1)
}

func (pk *PublicKey) VerifyWithSM3(msg, id, sig []byte) bool {
	return pk.VerifyWithSM3WithMode(msg, id, sig, SM2_SIGNATURE_MODE_RS_ASN1)
}

func (sk *PrivateKey) SignWithMode(dgst []byte, mode int) ([]byte, error) {
	if dgst == nil || len(dgst) != SM3_DIGEST_SIZE {
		return nil, errors.New("SM2 sign: message is invalid")
	}
	var sigLen C.size_t
	sig := make([]byte, SM2_SIGNATURE_MAX_SIZE)
	skByte := sk.Text
	pkByte := sk.PublicKey.Text
	ret := C.sm2_sign(
		sk.ctx,
		unsafe.Pointer(&dgst[0]),
		SM3_DIGEST_SIZE,
		unsafe.Pointer(&pkByte[0]),
		SM2_PUBLIC_KEY_SIZE,
		unsafe.Pointer(&skByte[0]),
		SM2_PRIVATE_KEY_SIZE,
		unsafe.Pointer(&sig[0]),
		&sigLen,
		C.SM2SignMode(mode))
	if ret != 0 {
		return nil, errors.New("SM2: fail to sign message")
	}
	return sig[0:int(sigLen)], nil
}

func (pk *PublicKey) VerifyWithMode(dgst, sig []byte, mode int) bool {
	if dgst == nil || sig == nil || len(dgst) != SM3_DIGEST_SIZE {
		return false
	}
	pkByte := pk.Text
	ret := C.sm2_verify(
		pk.ctx,
		unsafe.Pointer(&dgst[0]),
		SM3_DIGEST_SIZE,
		unsafe.Pointer(&sig[0]),
		C.size_t(len(sig)),
		unsafe.Pointer(&pkByte[0]),
		SM2_PUBLIC_KEY_SIZE,
		C.SM2SignMode(mode))
	if ret != 0 {
		return false
	}
	return true
}

func (sk *PrivateKey) Sign(dgst []byte) ([]byte, error) {
	return sk.SignWithMode(dgst, SM2_SIGNATURE_MODE_RS_ASN1)
}

func (pk *PublicKey) Verify(dgst, sig []byte) bool {
	return pk.VerifyWithMode(dgst, sig, SM2_SIGNATURE_MODE_RS_ASN1)
}

//CFCA证书若签名为31位，会补0，go本身是不补，长度写31
//兼容 去掉补0，长度改为31
func GetSignatureFromCFCA(signature []byte) []byte {
	dataLength := len(signature)
	dataIndex := 2 //当前下标，初始值为循环数据开始的位置

	//格式为 类型(1)+总长度(1)+[类型(1)+长度(1)+数据]
	//数据字节数为长度对应的大小，一般为32
	var signBuffer bytes.Buffer
	signBuffer.Write(signature[0:dataIndex])
	currentCount := signature[1]  //结构体总长度，用于减去补0后，总长度同样需要减
	currentDataCount := byte('0') //循环中有效数据实际长度
	dataCount := 0                //用于循环中记录每个数据的长度
	zeroCount := 0                //用于循环中记录出现的补0的个数
	for dataIndex+2 < dataLength {
		signBuffer.WriteByte(signature[dataIndex])
		dataCount = int(signature[dataIndex+1])
		if dataIndex+dataCount+2 > dataLength {
			signBuffer.Write(signature[dataIndex+1:])
			break
		}
		//只对长度为32字节的处理，如33字节表示正数但最高位为0需补符号，属于正常
		if 0 == signature[dataIndex+2] && 0 == signature[dataIndex+3]&0x80 {
			currentDataCount = signature[dataIndex+1] - 1
			zeroCount = 1
			//判断是否补多个0
			for {
				if 0 == signature[dataIndex+2+zeroCount] && 0 == signature[dataIndex+3+zeroCount]&0x80 {
					currentDataCount -= 1
					zeroCount += 1
				} else {
					break
				}
			}
			signBuffer.WriteByte(currentDataCount)
			signBuffer.Write(signature[dataIndex+2+zeroCount : dataIndex+2+dataCount])
			currentCount -= signature[dataIndex+1] - currentDataCount
		} else {
			signBuffer.Write(signature[dataIndex+1 : dataIndex+dataCount+2])
		}

		dataIndex += dataCount + 2
	}

	signature = signBuffer.Bytes()

	if 0 < signature[1]-currentCount {
		signature[1] = currentCount
	}

	return signature
}
