package sm

/*
#cgo CFLAGS: -g -O2 -I${SRCDIR}/include/
#cgo windows LDFLAGS: ${SRCDIR}/lib/windows/libTencentSM.dll ${SRCDIR}/lib/windows/libgmp.a
#cgo darwin LDFLAGS: ${SRCDIR}/lib/darwin/libTencentSM.a ${SRCDIR}/lib/darwin/libgmp.a
#cgo linux LDFLAGS: ${SRCDIR}/lib/linux/libTencentSM.a ${SRCDIR}/lib/linux/libgmp.a

#include "sm.h"
*/
import "C"
import (
	"unsafe"
)

/**
 *@brief SM2上下文结构体的大小
 */
func SM2CtxSize() int {
	return int(C.SM2CtxSize())
}

/**
 *@brief 使用SM2获取公私钥或加解密之前，必须调用SM2InitCtx或者SM2InitCtxWithPubKey函数
 *@param ctx  函数出参 - 上下文
 */
func SM2InitCtx(ctx *SM2_ctx_t) {
	if ctx == nil {
		panic("invalid parameter")
	}
	C.SM2InitCtx(&ctx.Context)
}

/**
 * @brief 使用SM2获取公私钥或加解密之前，必须调用SM2InitCtx或者SM2InitCtxWithPubKey函数.如果使用固定公钥加密，可调用SM2InitCtxWithPubKey，将获得较大性能提升
 * @param ctx  函数出参 - 上下文
 * @param pubkey  函数入参 - 公钥
 */
func SM2InitCtxWithPubKey(ctx *SM2_ctx_t, pubkey []byte) {
	if ctx == nil || pubkey == nil {
		panic("invalid parameter")
	}
	if len(pubkey) < 130 {
		panic("memory len is too small")
	}
	C.SM2InitCtxWithPubKey(&ctx.Context, (*C.char)(unsafe.Pointer(&pubkey[0])))
}

/**
 *@brief 使用完SM2算法后，必须调用free函数释放
 *@param ctx  函数入参 - 上下文
 */
func SM2FreeCtx(ctx *SM2_ctx_t) {
	if ctx == nil {
		panic("invalid parameter")
	}
	C.SM2FreeCtx(&ctx.Context)
}

/**
 *@brief 生成私钥
 *@param ctx  函数入参 - 上下文
 *@param out  函数出参 - 私钥，私钥实际上为256bit的大整数，这里输出的为256bit二进制内容Hex后的ASCII编码的可见字符串，长度为64字节，为保证字符串的结束符0，out至少需分配65字节空间。
 *@return  0表示成功，其他值为错误码
 */
func GeneratePrivateKey(ctx *SM2_ctx_t, out []byte) int {
	if ctx == nil || out == nil {
		panic("invalid parameter")
	}
	return int(C.generatePrivateKey(&ctx.Context, (*C.char)(unsafe.Pointer(&out[0]))))
}

/**
 *@brief根据私钥生成对应公钥，
 *@param ctx 函数入参 - 上下文
 *@param privateKey 函数入参 - 私钥，私钥实际上为256bit的大整数，这里输出的为256bit二进制内容Hex后的ASCII编码的可见字符串，长度为64字节
 *@param outPubKey 函数出参 - 公钥，公钥格式为04 | X | Y，其中X和Y为256bit大整数，这里输出的为04 | X | Y的二进制内容Hex后的ASCII编码
 *                 的可见字符串，长度为130字节，为保证字符串的结束符0，outPubKey至少需分配131字节空间。
 *@return  0表示成功，其他值为错误码
 */
func GeneratePublicKey(ctx *SM2_ctx_t, privateKey []byte, outPubKey []byte) int {
	if ctx == nil || privateKey == nil || outPubKey == nil {
		panic("invalid parameter")
	}
	return int(C.generatePublicKey(&ctx.Context, (*C.char)(unsafe.Pointer(&privateKey[0])),
		(*C.char)(unsafe.Pointer(&outPubKey[0]))))
}

/**
 *@brief生成公私钥对
 *@param ctx 函数入参 - 上下文
 *@param outPriKey 函数出参 - 私钥，私钥实际上为256bit的大整数，这里输出的为256bit二进制内容Hex后的ASCII编码的可见字符串，长度为64字节，为保证字符串的结束符0，
 *outPubKey至少需分配65字节空间。
 *@param outPubKey 函数出参 - 公钥，公钥格式为04 | X | Y，其中X和Y为256bit大整数，这里输出的为04 | X | Y的二进制内容Hex后的ASCII编码的可见字符串，
 *长度为130字节，为保证字符串的结束符0，outPubKey至少需分配131字节空间。
 *@return  0表示成功，其他值为错误码
 */
func GenerateKeyPair(ctx *SM2_ctx_t, outPriKey []byte, outPubKey []byte) int {
	if ctx == nil || outPriKey == nil || outPubKey == nil {
		panic("invalid parameter")
	}
	return int(C.generateKeyPair(&ctx.Context, (*C.char)(unsafe.Pointer(&outPriKey[0])),
		(*C.char)(unsafe.Pointer(&outPubKey[0]))))
}

/**
 *@briefSM2非对称加解密算法，加密
 *@param ctx 函数入参 - 上下文
 *@param in  函数入参 - 待加密消息
 *@param inlen  函数入参 - 消息长度(字节单位)
 *@param strPubKey  函数入参 - 公钥
 *@param pubkeyLen  函数入参 - 公钥长度
 *@param out  函数出参 - 密文
 *@param outlen  函数出参 - 密文长度
 *@return  0表示成功，其他值为错误码
 */
func SM2Encrypt(ctx *SM2_ctx_t, in []byte, inlen int, strPubKey []byte, pubkeyLen int, out []byte, outlen *int) int {
	if ctx == nil || in == nil || inlen <= 0 || strPubKey == nil || pubkeyLen <= 0 || out == nil || outlen == nil {
		panic("invalid parameter")
	}
	return int(C.SM2Encrypt(&ctx.Context, (*C.uchar)(unsafe.Pointer(&in[0])), (C.size_t)(inlen),
		(*C.char)(unsafe.Pointer(&strPubKey[0])), (C.size_t)(pubkeyLen),
		(*C.uchar)(unsafe.Pointer(&out[0])), (*C.size_t)(unsafe.Pointer(outlen))))
}

/**
 *@briefSM2非对称加解密算法，解密
 *@param ctx  函数入参 - 上下文
 *@param in  函数入参 - 待解密密文
 *@param inlen  函数入参 - 密文长度(字节单位)
 *@param strPriKey  函数入参 - 私钥
 *@param prikeyLen  函数入参 - 私钥长度
 *@param out  函数出参 - 明文
 *@param outlen  函数出参 - 明文长度
 *@return  0表示成功，其他值为错误码
 */
func SM2Decrypt(ctx *SM2_ctx_t, in []byte, inlen int, strPriKey []byte, prikeyLen int, out []byte, outlen *int) int {
	if ctx == nil || in == nil || inlen <= 0 || strPriKey == nil || prikeyLen <= 0 || out == nil || outlen == nil {
		panic("invalid parameter")
	}
	return int(C.SM2Decrypt(&ctx.Context, (*C.uchar)(unsafe.Pointer(&in[0])), (C.size_t)(inlen),
		(*C.char)(unsafe.Pointer(&strPriKey[0])), (C.size_t)(prikeyLen),
		(*C.uchar)(unsafe.Pointer(&out[0])), (*C.size_t)(unsafe.Pointer(outlen))))
}

/**
 *@briefSM2签名验签算法，签名
 *@param ctx 函数入参 - 上下文
 *@param msg 函数入参 - 待签名消息
 *@param msglen 函数入参 - 待签名消息长度
 *@param id 函数入参 - 用户ID(作用是加入到签名hash中，对于传入值无特殊要求)
 *@param idlen 函数入参 - 用户ID长度
 *@param strPubKey 函数入参 - 公钥(作用是加入到签名hash中)
 *@param pubkeyLen 函数入参 - 公钥长度
 *@param strPriKey 函数入参 - 私钥
 *@param prikeyLen 函数入参 - 私钥长度
 *@param sig 函数出参 - 签名结果
 *@param siglen 函数出参 - 签名结果长度
 */
func SM2Sign(ctx *SM2_ctx_t, msg []byte, msglen int, id []byte, idlen int, strPubKey []byte,
	pubkeyLen int, strPriKey []byte, prikeyLen int, sig []byte, siglen *int) int {
	if ctx == nil || msg == nil || msglen <= 0 || id == nil || idlen <= 0 || strPubKey == nil ||
		pubkeyLen <= 0 || strPriKey == nil || prikeyLen <= 0 || sig == nil || siglen == nil {
		panic("invalid parameter")
	}
	return int(C.SM2Sign(&ctx.Context,
		(*C.uchar)(unsafe.Pointer(&msg[0])), (C.size_t)(msglen),
		(*C.char)(unsafe.Pointer(&id[0])), (C.size_t)(idlen),
		(*C.char)(unsafe.Pointer(&strPubKey[0])), (C.size_t)(pubkeyLen),
		(*C.char)(unsafe.Pointer(&strPriKey[0])), (C.size_t)(prikeyLen),
		(*C.uchar)(unsafe.Pointer(&sig[0])), (*C.size_t)(unsafe.Pointer(siglen))))
}

/**
 *@briefSM2签名验签算法，验签
 *@param ctx 函数入参 - 上下文
 *@param msg 函数入参 - 待验签内容
 *@param msglen 函数入参 - 待验签内容长度
 *@param id 函数入参 - 用户ID
 *@param idlen 函数入参 - 用户ID长度
 *@param sig 函数入参 - 签名结果
 *@param siglen 函数入参 - 签名结果长度
 *@param strPubKey 函数入参 - 公钥
 *@param pubkeyLen 函数入参 - 公钥长度
 *@return 0表示成功，其他值为错误码
 */
func SM2Verify(ctx *SM2_ctx_t, msg []byte, msglen int, id []byte, idlen int, sig []byte,
	siglen int, strPubKey []byte, pubkeyLen int) int {
	if ctx == nil || msg == nil || msglen <= 0 || id == nil || idlen <= 0 || sig == nil ||
		siglen <= 0 || strPubKey == nil || pubkeyLen <= 0 {
		panic("invalid parameter")
	}
	return int(C.SM2Verify(&ctx.Context,
		(*C.uchar)(unsafe.Pointer(&msg[0])), (C.size_t)(msglen),
		(*C.char)(unsafe.Pointer(&id[0])), (C.size_t)(idlen),
		(*C.uchar)(unsafe.Pointer(&sig[0])), (C.size_t)(siglen),
		(*C.char)(unsafe.Pointer(&strPubKey[0])), (C.size_t)(pubkeyLen)))

}

/**
 *@brief SM2非对称加解密算法，加密的兼容接口
 *@param ctx 函数入参 - 上下文
 *@param in 函数入参 - 待加密消息
 *@param inlen 函数入参 - 消息长度(字节单位)
 *@param strPubKey 函数入参 - 公钥
 *@param pubkeyLen 函数入参 - 公钥长度
 *@param out 函数出参 - 密文
 *@param outlen 函数出参 - 密文长度
 *@param mode 密文输出格式
 *@return  0表示成功，其他值为错误码
 */
func SM2EncryptWithMode(ctx *SM2_ctx_t, in []byte, inlen int, strPubKey []byte,
	pubkeyLen int, out []byte, outlen *int, mode SM2CipherMode) int {
	if ctx == nil || in == nil || inlen <= 0 || strPubKey == nil ||
		pubkeyLen <= 0 || out == nil || outlen == nil {
		panic("invalid parameter")
	}
	return int(C.SM2EncryptWithMode(&ctx.Context,
		(*C.uchar)(unsafe.Pointer(&in[0])), (C.size_t)(inlen),
		(*C.char)(unsafe.Pointer(&strPubKey[0])), (C.size_t)(pubkeyLen),
		(*C.uchar)(unsafe.Pointer(&out[0])), (*C.size_t)(unsafe.Pointer(outlen)),
		ConvertSMCipherModeToC(mode)))
}

/**
 *@brief SM2非对称加解密算法，解密的兼容接口
 *@param ctx 函数入参 - 上下文
 *@param in  函数入参 - 待解密密文
 *@param inlen  函数入参 - 密文长度(字节单位)
 *@param strPriKey  函数入参 - 私钥
 *@param prikeyLen  函数入参 - 私钥长度
 *@param out  函数出参 - 明文
 *@param outlen  函数出参 - 明文长度
 *@param mode  密文格式
 *@return  0表示成功，其他值为错误码
 */
func SM2DecryptWithMode(ctx *SM2_ctx_t, in []byte, inlen int, strPriKey []byte,
	prikeyLen int, out []byte, outlen *int, mode SM2CipherMode) int {
	if ctx == nil || in == nil || inlen <= 0 || strPriKey == nil || prikeyLen <= 0 ||
		out == nil || outlen == nil {
		panic("invalid parameter")
	}
	return int(C.SM2DecryptWithMode(&ctx.Context,
		(*C.uchar)(unsafe.Pointer(&in[0])), (C.size_t)(inlen),
		(*C.char)(unsafe.Pointer(&strPriKey[0])), (C.size_t)(prikeyLen),
		(*C.uchar)(unsafe.Pointer(&out[0])), (*C.size_t)(unsafe.Pointer(outlen)),
		ConvertSMCipherModeToC(mode)))
}

/**
 *@brief SM2签名验签算法，签名的兼容接口
 *@param ctx 函数入参 - 上下文
 *@param msg 函数入参 - 待签名消息
 *@param msglen 函数入参 - 待签名消息长度
 *@param id 函数入参 - 用户ID(作用是加入到签名hash中，对于传入值无特殊要求)
 *@param idlen 函数入参 - 用户ID长度
 *@param strPubKey 函数入参 - 公钥(作用是加入到签名hash中)
 *@param pubkeyLen 函数入参 - 公钥长度
 *@param strPriKey 函数入参 - 私钥
 *@param prikeyLen 函数入参 - 私钥长度
 *@param sig 函数出参 - 签名结果
 *@param siglen 函数出参 - 签名结果长度
 *@param mode 签名格式
 */
func SM2SignWithMode(ctx *SM2_ctx_t, msg []byte, msglen int, id []byte,
	idlen int, strPubKey []byte, pubkeyLen int, strPriKey []byte,
	prikeyLen int, sig []byte, siglen *int, signMode SM2SignMode) int {
	if ctx == nil || msg == nil || msglen <= 0 || id == nil || idlen <= 0 ||
		strPubKey == nil || pubkeyLen <= 0 || strPriKey == nil ||
		prikeyLen <= 0 || sig == nil || siglen == nil {
		panic("invalid parameter")
	}
	return int(C.SM2SignWithMode(&ctx.Context,
		(*C.uchar)(unsafe.Pointer(&msg[0])), (C.size_t)(msglen),
		(*C.char)(unsafe.Pointer(&id[0])), (C.size_t)(idlen),
		(*C.char)(unsafe.Pointer(&strPubKey[0])), (C.size_t)(pubkeyLen),
		(*C.char)(unsafe.Pointer(&strPriKey[0])), (C.size_t)(prikeyLen),
		(*C.uchar)(unsafe.Pointer(&sig[0])), (*C.size_t)(unsafe.Pointer(siglen)), ConvertSMSignModeToC(signMode)))
}

/**
 *@brief SM2签名验签算法，验签的兼容接口
 *@param ctx 函数入参 - 上下文
 *@param msg 函数入参 - 待验签内容
 *@param msglen 函数入参 - 待验签内容长度
 *@param id 函数入参 - 用户ID
 *@param idlen 函数入参 - 用户ID长度
 *@param sig 函数入参 - 签名结果
 *@param siglen 函数入参 - 签名结果长度
 *@param strPubKey 函数入参 - 公钥
 *@param pubkeyLen 函数入参 - 公钥长度
 *@param mode 签名格式
 *@return 0表示成功，其他值为错误码
 */
func SM2VerifyWithMode(ctx *SM2_ctx_t, msg []byte, msglen int, id []byte, idlen int,
	sig []byte, siglen int, strPubKey []byte, pubkeyLen int, signMode SM2SignMode) int {
	if ctx == nil || msg == nil || msglen <= 0 || id == nil || idlen <= 0 ||
		sig == nil || siglen <= 0 || strPubKey == nil || pubkeyLen <= 0 {
		panic("invalid parameter")
	}
	return int(C.SM2VerifyWithMode(&ctx.Context,
		(*C.uchar)(unsafe.Pointer(&msg[0])), (C.size_t)(msglen),
		(*C.char)(unsafe.Pointer(&id[0])), (C.size_t)(idlen),
		(*C.uchar)(unsafe.Pointer(&sig[0])), (C.size_t)(siglen),
		(*C.char)(unsafe.Pointer(&strPubKey[0])), (C.size_t)(pubkeyLen),
		ConvertSMSignModeToC(signMode)))
}

/**
 *@brief 为SM2增加外部熵源，该接口主要用于输入外部随机熵，一般情况下无需调用该接口，模块内部使用的随机熵通常情况下可保证熵值足够。
 *@param ctx 函数入参 - 上下文
 *@param buf 函数入参 - 熵buf
 *@param buflen 函数入参 - 熵buf长度
 */
// func sm2ReSeed(ctx *SM2_ctx_t, buf []byte, buflen int) int {
// 	if ctx == nil || buf == nil || buflen <= 0 {
// 		panic("invalid parameter")
// 	}
// 	return int(C.SM2ReSeed(&ctx.Context, (*C.uchar)(unsafe.Pointer(&buf[0])), (C.size_t)(buflen)))
// }
