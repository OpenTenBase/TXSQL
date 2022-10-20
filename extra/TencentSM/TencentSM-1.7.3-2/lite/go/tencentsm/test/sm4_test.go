package test

import (
	"fmt"
	sm "tencentsm"
	"testing"

	. "github.com/smartystreets/goconvey/convey"
)

func TestSM4CBC(t *testing.T) {
	Convey("CBC", t, func() {
		var key [16]byte
		sm.GenerateSM4Key(key[:])
		fmt.Printf("gen key%x\n", key)
		So(key, ShouldNotBeNil)
		data := []byte("hello world")
		var out [24]byte
		var plainText [24]byte
		outlen := len(out)
		plainLen := len(plainText)
		iv := []byte("1234567890123456")
		sm.SM4_CBC_Encrypt(data[:], len(data), out[:], &outlen, key[:], iv[:])
		fmt.Printf("encrypte%x\n", out[:])
		sm.SM4_CBC_Decrypt(out[:], outlen, plainText[:], &plainLen, key[:], iv[:])
		fmt.Printf("decrypte len%d\n", plainLen)
		//plainText1 := plainText[0 : plainLen*2]
		fmt.Printf("decrypte:%s", plainText)
		data[0] = 'i'
		fmt.Printf("before clear %p\n", &out)
		//out = make([]byte, 24)
		//data = data[:0:24]
		fmt.Printf("data %s\n", data)
		fmt.Printf("clear out%x\n", out[:])
		var out1 [24]byte
		var outlen1 = len(out1)
		data1 := []byte("1234567890123456")
		sm.SM4_CBC_Encrypt_NoPadding(data1[:], len(data1), out1[:], &outlen1, key[:], iv[:])
		fmt.Printf("nopadding encrypte%x\n", out1[:])
		var plainText1 [24]byte
		var plainLen1 = len(plainText1)
		sm.SM4_CBC_Decrypt_NoPadding(out1[:], outlen1, plainText1[:], &plainLen1, key[:], iv[:])
		fmt.Printf("decrypte len%d\n", plainLen1)
		//plainText1 := plainText[0 : plainLen*2]
		fmt.Printf("decrypte:%s", plainText1)
	})
}

func TestSM4ECB(t *testing.T) {
	Convey("CBC", t, func() {
		var key [16]byte
		sm.GenerateSM4Key(key[:])
		fmt.Printf("gen key%x\n", key)
		So(key, ShouldNotBeNil)
		data := []byte("hello world")
		var out [24]byte
		var plainText [24]byte
		outlen := len(out)
		plainLen := len(plainText)
		sm.SM4_ECB_Encrypt(data[:], len(data), out[:], &outlen, key[:])
		fmt.Printf("encrypte%x\n", out[:])
		sm.SM4_ECB_Decrypt(out[:], outlen, plainText[:], &plainLen, key[:])
		fmt.Printf("decrypte len%d\n", plainLen)
		//plainText1 := plainText[0 : plainLen*2]
		fmt.Printf("decrypte:%s", plainText)
		data[0] = 'i'
		fmt.Printf("before clear %p\n", &out)
		//out = make([]byte, 24)
		//data = data[:0:24]
		fmt.Printf("data %s\n", data)
		fmt.Printf("clear out%x\n", out[:])
		var out1 [24]byte
		var outlen1 = len(out1)
		data1 := []byte("1234567890123456")
		sm.SM4_ECB_Encrypt_NoPadding(data1[:], len(data1), out1[:], &outlen1, key[:])
		fmt.Printf("nopadding encrypte%x\n", out1[:])
		var plainText1 [24]byte
		var plainLen1 = len(plainText1)
		sm.SM4_ECB_Decrypt_NoPadding(out1[:], outlen1, plainText1[:], &plainLen1, key[:])
		fmt.Printf("decrypte len%d\n", plainLen1)
		//plainText1 := plainText[0 : plainLen*2]
		fmt.Printf("decrypte:%s", plainText1)
	})
}

func TestSM4GCM(t *testing.T) {
	Convey("GCM", t, func() {
		var key [16]byte
		sm.GenerateSM4Key(key[:])
		fmt.Printf("gen key%x\n", key)
		So(key, ShouldNotBeNil)
		data := []byte("hello world")
		var out [24]byte
		var plainText [24]byte
		outlen := len(out)
		plainLen := len(plainText)
		iv := []byte("1234567890123456")
		var tag [24]byte
		taglen := 24
		add := []byte("666666")
		addlen := len(add)
		ret := sm.SM4_GCM_Encrypt(data[:], len(data), out[:], &outlen, tag[:], &taglen, key[:], iv[:], add[:], addlen)
		fmt.Printf("encrypte%x\n", out[:])
		So(ret, ShouldBeZeroValue)
		ret = sm.SM4_GCM_Decrypt(out[:], outlen, plainText[:], &plainLen, tag[:], taglen, key[:], iv[:], add[:], addlen)
		fmt.Printf("decrypte len%d\n", plainLen)
		So(ret, ShouldBeZeroValue)
		//plainText1 := plainText[0 : plainLen*2]
		fmt.Printf("decrypte:%s", plainText)
		data[0] = 'i'
		fmt.Printf("before clear %p\n", &out)
		//out = make([]byte, 24)
		//data = data[:0:24]
		fmt.Printf("data %s\n", data)
		fmt.Printf("clear out%x\n", out[:])
		var out1 [24]byte
		var outlen1 = len(out1)
		data1 := []byte("1234567890123456")
		sm.SM4_GCM_Encrypt_NoPadding(data1[:], len(data1), out1[:], &outlen1, tag[:], &taglen, key[:], iv[:], add[:], addlen)
		fmt.Printf("nopadding encrypte%x\n", out1[:])
		var plainText1 [24]byte
		var plainLen1 = len(plainText1)
		sm.SM4_GCM_Decrypt_NoPadding(out1[:], outlen1, plainText1[:], &plainLen1, tag[:], taglen, key[:], iv[:], add[:], addlen)
		fmt.Printf("decrypte len%d\n", plainLen1)
		//plainText1 := plainText[0 : plainLen*2]
		fmt.Printf("decrypte:%s", plainText1)
	})
}

func TestCTRSM4(t *testing.T) {
	Convey("CTR SM4", t, func() {
		var key [16]byte
		sm.GenerateSM4Key(key[:])
		fmt.Printf("gen key%x\n", key)
		So(key, ShouldNotBeNil)
		data := []byte("hello world@@@!!!+++")
		var out [24]byte
		var plainText [24]byte
		plainLen := len(data)
		//iv := []byte("1234567890123456")
		// sm.SM4_CTR_Encrypt(data[:], out[:], len(data), key[:], iv[:])
		// fmt.Printf("encrypte%x\n", out[:])
		// sm.SM4_CTR_Decrypt(out[:], plainText[:], plainLen, key[:], iv[:])
		// fmt.Printf("decrypte len%d\n", plainLen)
		//plainText1 := plainText[0 : plainLen*2]
		fmt.Printf("decrypte:%s\n", plainText[:plainLen])
		fmt.Printf("before clear %x\n", out[:plainLen])
	})
}

func TestSM4GCM_NIST_SP800_38D(t *testing.T) {
	Convey("GCM", t, func() {
		var key [16]byte
		sm.GenerateSM4Key(key[:])
		fmt.Printf("gen key%x\n", key)
		So(key, ShouldNotBeNil)
		data := []byte("hello world")
		var out [24]byte
		var plainText [24]byte
		outlen := len(out)
		plainLen := len(plainText)
		iv := []byte("1234567890123456")
		var tag [24]byte
		taglen := 24
		add := []byte("666666")
		addlen := len(add)
		ivlen := len(iv)
		ret := sm.SM4_GCM_Encrypt_NIST_SP800_38D(data[:], len(data), out[:], &outlen, tag[:], &taglen, key[:], iv[:], ivlen, add[:], addlen)
		fmt.Printf("encrypte%x\n", out[:])
		So(ret, ShouldBeZeroValue)
		ret = sm.SM4_GCM_Decrypt_NIST_SP800_38D(out[:], outlen, plainText[:], &plainLen, tag[:], taglen, key[:], iv[:], ivlen, add[:], addlen)
		So(ret, ShouldBeZeroValue)
		fmt.Printf("decrypte len%d\n", plainLen)
		//plainText1 := plainText[0 : plainLen*2]
		fmt.Printf("decrypte:%s", plainText)
		data[0] = 'i'
		fmt.Printf("before clear %p\n", &out)
		//out = make([]byte, 24)
		//data = data[:0:24]
		fmt.Printf("data %s\n", data)
		fmt.Printf("clear out%x\n", out[:])
		var out1 [24]byte
		var outlen1 = len(out1)
		data1 := []byte("1234567890123456")
		ret = sm.SM4_GCM_Encrypt_NoPadding_NIST_SP800_38D(data1[:], len(data1), out1[:], &outlen1, tag[:], &taglen, key[:], iv[:], ivlen, add[:], addlen)
		So(ret, ShouldBeZeroValue)
		fmt.Printf("nopadding encrypte%x\n", out1[:])
		var plainText1 [24]byte
		var plainLen1 = len(plainText1)
		ret = sm.SM4_GCM_Decrypt_NoPadding_NIST_SP800_38D(out1[:], outlen1, plainText1[:], &plainLen1, tag[:], taglen, key[:], iv[:], ivlen, add[:], addlen)
		So(ret, ShouldBeZeroValue)
		fmt.Printf("decrypte len%d\n", plainLen1)
		//plainText1 := plainText[0 : plainLen*2]
		fmt.Printf("decrypte:%s", plainText1)
	})
}
