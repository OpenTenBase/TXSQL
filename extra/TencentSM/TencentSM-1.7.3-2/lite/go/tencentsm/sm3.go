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
 *@brief SM3上下文结构体的大小
 */
func SM3CtxSize() int {
	return int(C.SM3CtxSize())
}

func SM3Init(ctx *SM3_ctx_t) {
	if ctx == nil {
		panic("invalid parameter")
	}
	C.SM3Init(&ctx.Context)
}

func SM3Update(ctx *SM3_ctx_t, data []byte, datalen int) {
	if ctx == nil || data == nil || datalen <= 0 {
		panic("invalid parameter")
	}
	C.SM3Update(&ctx.Context, (*C.uchar)(unsafe.Pointer(&data[0])), (C.size_t)(datalen))
}

func SM3Final(ctx *SM3_ctx_t, digest []byte) {
	if ctx == nil || digest == nil {
		panic("invalid parameter")
	}
	C.SM3Final(&ctx.Context, (*C.uchar)(unsafe.Pointer(&digest[0])))
}

/**
 *@brief SM3 hash算法， 内部依次调用了init update和final三个接口
 */
func SM3(data []byte, datalen int, digest []byte) {
	if data == nil || digest == nil || datalen <= 0 {
		panic("invalid parameter")
	}
	C.SM3((*C.uchar)(unsafe.Pointer(&data[0])), (C.size_t)(datalen), (*C.uchar)(unsafe.Pointer(&digest[0])))
}

/**
 * @brief 基于sm3算法计算HMAC值 ctx init
 * @param key HMAC用的秘钥
 * @param key_len 秘钥长度
 * @return 0 -- OK
 */
func SM3HMACInit(key []byte, keyLen int) *HmacSm3Ctx {
	if key == nil || keyLen <= 0 {
		panic("invalid parameter")
	}
	var ret HmacSm3Ctx
	cRet := C.SM3_HMAC_Init((*C.uchar)(unsafe.Pointer(&key[0])), (C.size_t)(keyLen))
	ret.Context = cRet
	return &ret
}

/**
 * @brief 基于sm3算法计算HMAC值 update数据
 * @param ctx hmac上下文结构指针
 * @param data 做HMAC计算的数据
 * @param data_len 数据长度
 * @return 0 -- OK
 */
func SM3HmacUpdate(ctx *HmacSm3Ctx, data []byte, dataLen int) int {
	if data == nil || ctx == nil || dataLen <= 0 {
		panic("invalid parameter")
	}
	return int(C.SM3_HMAC_Update(ctx.Context, (*C.uchar)(unsafe.Pointer(&data[0])), (C.size_t)(dataLen)))
}

/**
 * @brief 基于sm3算法计算HMAC值 最终计算HMAC值
 * @param ctx hmac上下文结构指针
 * @param mac 输出的HMAC字节码
 * @return 0 -- OK
 */
func SM3HmacFinal(ctx *HmacSm3Ctx, mac []byte, macLen int) int {
	if mac == nil || len(mac) != macLen || macLen != SM3_HMAC_SIZE {
		panic("invalid parameter")
	}
	return int(C.SM3_HMAC_Final(ctx.Context, (*C.uchar)(unsafe.Pointer(&mac[0]))))
}

/**
 * @brief 基于sm3算法计算HMAC值
 * @param data 做HMAC计算的数据
 * @param data_len 数据长度
 * @param key HMAC用的秘钥
 * @param key_len 秘钥长度
 * @param mac 输出的HMAC字节码
 * @return 0 -- OK
 */
func SM3_HMAC(ctx *HmacSm3Ctx, data []byte, dataLen int, key []byte, keyLen int, mac []byte, macLen int) int {
	if data == nil || key == nil || mac == nil || len(mac) != macLen || macLen != SM3_HMAC_SIZE {
		panic("invalid parameter")
	}
	return int(C.SM3_HMAC((*C.uchar)(unsafe.Pointer(&data[0])), (C.size_t)(dataLen),
		(*C.uchar)(unsafe.Pointer(&key[0])), (C.size_t)(keyLen), (*C.uchar)(unsafe.Pointer(&mac[0]))))
}
