package main

import (
	"fmt"
	sm "tencentsm"
)

func main() {
	appid := []byte("com.tencent.tgmssl")
	token := []byte("3045022100EE5BED87BFF036541300866DDC5445D9BD43950BEFCFCF6C1C22AD91F004446B02202CE61B084289C00225F9D595F054DE1D5849E8F4CB6F38902421F6017D054068")
	//ret := sm.InitTencentSM(appid, token)
	var prikey [65]byte
	var ctx sm.SM2_ctx_t
	sm.SM2InitCtx(&ctx)
	var ret = sm.GeneratePrivateKey(&ctx, prikey[:])
	fmt.Println(prikey)
	fmt.Println(ret)
}
