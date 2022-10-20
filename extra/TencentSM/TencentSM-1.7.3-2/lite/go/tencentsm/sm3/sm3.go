package sm3

/*
#include "tencentsm/sm.h"

void sm3_update(sm3_ctx_t *ctx, void* data, size_t data_len)
{
	SM3Update(ctx, (unsigned char*)data, data_len);
}

void sm3_final(sm3_ctx_t *ctx, void* dgst)
{
	SM3Final(ctx, (unsigned char*)dgst);
}
*/
import "C"
import (
	"crypto"
	"hash"
	"unsafe"
)

func init() {
	crypto.RegisterHash(crypto.SM3, New)
}

type digest struct {
	ctx C.sm3_ctx_t
}

func New() hash.Hash {
	d := new(digest)
	var ctx C.sm3_ctx_t
	C.SM3Init(&ctx)
	d.ctx = ctx
	return d
}

func NewDigestCtx() *digest {
	d := new(digest)
	var ctx C.sm3_ctx_t
	C.SM3Init(&ctx)
	d.ctx = ctx
	return d
}

func (d *digest) BlockSize() int {
	return SM3_BLOCK_SIZE
}

func (d *digest) Size() int {
	return SM3_DIGEST_SIZE
}

func (d *digest) Reset() {
	var ctx C.sm3_ctx_t
	C.SM3Init(&ctx)
	d.ctx = ctx
}

func (d *digest) Write(p []byte) (int, error) {
	if p == nil {
		return 0, nil
	}
	C.sm3_update(&d.ctx, unsafe.Pointer(&p[0]), C.size_t(len(p)))
	return len(p), nil
}

func (d *digest) Sum(in []byte) []byte {
	dgst := make([]byte, SM3_DIGEST_SIZE)
	_, _ = d.Write(in)
	C.sm3_final(&d.ctx, unsafe.Pointer(&dgst[0]))
	return dgst
}
