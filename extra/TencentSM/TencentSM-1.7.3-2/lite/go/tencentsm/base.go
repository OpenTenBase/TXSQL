package sm

/*
#cgo windows LDFLAGS: ${SRCDIR}/lib/windows/libTencentSM.dll ${SRCDIR}/lib/windows/libgmp.a
#cgo darwin LDFLAGS: ${SRCDIR}/lib/darwin/libTencentSM.a ${SRCDIR}/lib/darwin/libgmp.a
#cgo linux LDFLAGS: ${SRCDIR}/lib/linux/libTencentSM.a ${SRCDIR}/lib/linux/libgmp.a

#cgo CFLAGS: -g -O2 -I${SRCDIR}/include/

#include "sm.h"
*/
import "C"

const SM3_BLOCK_SIZE int = 64

const SM3_DIGEST_LENGTH int = 32
const SM3_HMAC_SIZE int = SM3_DIGEST_LENGTH

//SM2上下文
type SM2_ctx_t struct {
	Context C.sm2_ctx_t
}

//SM3上下文
type SM3_ctx_t struct {
	Context C.sm3_ctx_t
}

//SM3 HMAC上下文
type HmacSm3Ctx struct {
	Context *C.TstHmacSm3Ctx
}

//Sm2签名模式
type SM2SignMode int

const (
	SM2SignMode_RS_ASN1 SM2SignMode = iota
	SM2SignMode_RS
)

func ConvertSMSignModeToC(mode SM2SignMode) C.SM2SignMode {
	var ret C.SM2SignMode
	switch mode {
	case SM2SignMode_RS_ASN1:
		ret = C.SM2SignMode_RS_ASN1
	case SM2SignMode_RS:
		ret = C.SM2SignMode_RS
	default:
		ret = C.SM2SignMode_RS_ASN1
	}
	return ret
}

//sm2加密模式
type SM2CipherMode int

const (
	SM2CipherMode_C1C3C2_ASN1 SM2CipherMode = iota
	SM2CipherMode_C1C3C2
	SM2CipherMode_C1C2C3_ASN1
	SM2CipherMode_C1C2C3
)

func ConvertSMCipherModeToC(mode SM2CipherMode) C.SM2CipherMode {
	var ret C.SM2CipherMode
	switch mode {
	case SM2CipherMode_C1C3C2_ASN1:
		ret = C.SM2CipherMode_C1C3C2_ASN1
	case SM2CipherMode_C1C3C2:
		ret = C.SM2CipherMode_C1C3C2
	case SM2CipherMode_C1C2C3_ASN1:
		ret = C.SM2CipherMode_C1C2C3_ASN1
	case SM2CipherMode_C1C2C3:
		ret = C.SM2CipherMode_C1C2C3
	default:
		ret = C.SM2SignMode_RS_ASN1
	}
	return ret
}

//证书模式
type SM2CSRMode int

const (
	SM2CSRMode_Single SM2CSRMode = iota
	SM2CSRMode_Double
)

/**
 *@brief 获取当前sdk版本
 */
func Version() string {
	return C.GoString(C.version())
}
