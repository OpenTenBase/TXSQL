package sm2

import (
	"crypto/elliptic"
	"encoding/asn1"
	"math/big"
	"sync"
)

// curve
var (
	OidSM2 = asn1.ObjectIdentifier{1, 2, 840, 10045, 2, 1} // SM2 id
)

var (
	oidNamedCurveP224 = asn1.ObjectIdentifier{1, 3, 132, 0, 33}
	oidNamedCurveP256 = asn1.ObjectIdentifier{1, 2, 840, 10045, 3, 1, 7}
	oidNamedCurveP384 = asn1.ObjectIdentifier{1, 3, 132, 0, 34}
	oidNamedCurveP521 = asn1.ObjectIdentifier{1, 3, 132, 0, 35}

	OidNamedCurveSm2 = asn1.ObjectIdentifier{1, 2, 156, 10197, 1, 301}
)

var initonce sync.Once

type (
	p256Curve struct {
		*elliptic.CurveParams
	}
)

var (
	p256 p256Curve
)

func initSMP256() {
	// See FIPS 186-3, section D.2.3
	p256.CurveParams = &elliptic.CurveParams{Name: "SM2-P-256"}
	var pStr string = "115792089210356248756420345214020892766250353991924191454421193933289684991999"
	p256.P, _ = new(big.Int).SetString(pStr, 10)
	var nStr string = "115792089210356248756420345214020892766061623724957744567843809356293439045923"
	p256.N, _ = new(big.Int).SetString(nStr, 10)
	var bStr string = "18505919022281880113072981827955639221458448578012075254857346196103069175443"
	p256.B, _ = new(big.Int).SetString(bStr, 10)
	var gXStr string = "22963146547237050559479531362550074578802567295341616970375194840604139615431"
	p256.Gx, _ = new(big.Int).SetString(gXStr, 10)
	var gYStr string = "85132369209828568825618990617112496413088388631904505083283536607588877201568"
	p256.Gy, _ = new(big.Int).SetString(gYStr, 10)
	p256.BitSize = 256
}

func P256Sm2() elliptic.Curve {
	initonce.Do(initSMP256)
	return p256
}

func OidFromNamedCurve(curve elliptic.Curve) (asn1.ObjectIdentifier, bool) {
	switch curve {
	case elliptic.P224():
		return oidNamedCurveP224, true
	case elliptic.P256():
		return oidNamedCurveP256, true
	case elliptic.P384():
		return oidNamedCurveP384, true
	case elliptic.P521():
		return oidNamedCurveP521, true
	case P256Sm2():
		return OidNamedCurveSm2, true
	}
	return nil, false
}

func namedCurveFromOID(oid asn1.ObjectIdentifier) elliptic.Curve {
	switch {
	case oid.Equal(oidNamedCurveP224):
		return elliptic.P224()
	case oid.Equal(oidNamedCurveP256):
		return elliptic.P256()
	case oid.Equal(oidNamedCurveP384):
		return elliptic.P384()
	case oid.Equal(oidNamedCurveP521):
		return elliptic.P521()
	case oid.Equal(OidNamedCurveSm2):
		return P256Sm2()
	}
	return nil
}
