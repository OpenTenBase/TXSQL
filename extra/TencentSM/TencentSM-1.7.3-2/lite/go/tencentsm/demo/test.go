package main

import (
	"fmt"
	"plugin"
	sm "tencentsm"
)

func test() {
	p, err := plugin.Open("../release/libtencentsm_go.so")
	if err != nil {
		panic(err)
	}
	f, err := p.Lookup("Version")
	if err != nil {
		panic(err)
	}
	s := f.(func() string)()
	fmt.Println(s)
	appid := []byte("com.tencent.tgmssl")
	token := []byte("3045022100EE5BED87BFF036541300866DDC5445D9BD43950BEFCFCF6C1C22AD91F004446B02202CE61B084289C00225F9D595F054DE1D5849E8F4CB6F38902421F6017D054068")

	f, err = p.Lookup("InitTencentSM")
	if err != nil {
		panic(err)
	}
	f.(func([]byte, []byte))(appid, token)
	// SM2_ctx_t, err := p.Lookup("SM2_ctx_t")
	// if err != nil {
	// 	panic(err)
	// }
	//sm.InitTencentSM(appid, token)
	var prikey [65]byte
	var ctx sm.SM2_ctx_t
	f, err = p.Lookup("SM2InitCtx")
	if err != nil {
		panic(err)
	}
	f.(func(*sm.SM2_ctx_t))(&ctx)
	//sm.SM2InitCtx(&ctx)
	var pubkey [131]byte
	f, err = p.Lookup("GenerateKeyPair")
	if err != nil {
		panic(err)
	}
	f.(func(*sm.SM2_ctx_t, []byte, []byte))(&ctx, prikey[:], pubkey[:])
	//sm.GenerateKeyPair(&ctx, prikey[:], pubkey[:])
	fmt.Println(pubkey)
	fmt.Println(prikey)
}
