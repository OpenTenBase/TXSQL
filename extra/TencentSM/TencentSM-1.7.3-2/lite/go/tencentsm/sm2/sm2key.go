package sm2

/*
#include "tencentsm/sm.h"

void sm2_init_ctx(sm2_ctx_t* ctx, void* pk)
{
	SM2InitCtxWithPubKey(ctx, (char*)pk);
}
*/
import "C"
import (
	"crypto/elliptic"
	"crypto/x509/pkix"
	"encoding/asn1"
	"encoding/pem"
	"errors"
	"fmt"
	"math/big"
	"reflect"
	"unsafe"
)

type publicKeyInfo struct {
	Raw       asn1.RawContent
	Algorithm pkix.AlgorithmIdentifier
	PublicKey asn1.BitString
}

type pkixPublicKey struct {
	Algo      pkix.AlgorithmIdentifier
	BitString asn1.BitString
}

type pkcs8 struct {
	Version    int
	Algo       pkix.AlgorithmIdentifier
	PrivateKey []byte
}

type sm2PrivateKeyInfo struct {
	Version       int
	PrivateKey    []byte
	NamedCurveOID asn1.ObjectIdentifier `asn1:"optional,explicit,tag:0"`
	PublicKey     asn1.BitString        `asn1:"optional,explicit,tag:1"`
}

// public key conversion
func MarshalPublicKey(key *PublicKey) ([]byte, error) {
	if key == nil {
		return nil, errors.New("input SM2 public key is null")
	}
	var r pkixPublicKey
	var algo pkix.AlgorithmIdentifier

	algo.Algorithm = OidSM2
	algo.Parameters.Class = 0
	algo.Parameters.Tag = 6
	algo.Parameters.IsCompound = false
	algo.Parameters.FullBytes = []byte{6, 8, 42, 129, 28, 207, 85, 1, 130, 45} // asn1.Marshal(asn1.ObjectIdentifier{1, 2, 156, 10197, 1, 301})
	r.Algo = algo
	r.BitString = asn1.BitString{Bytes: elliptic.Marshal(key.Curve, key.X, key.Y)}
	return asn1.Marshal(r)
}

func UnmarshalPublicKey(der []byte) (*PublicKey, error) {
	if der == nil {
		return nil, errors.New("input DER is null")
	}
	var pki publicKeyInfo
	if rest, err := asn1.Unmarshal(der, &pki); err != nil {
		return nil, err
	} else if len(rest) != 0 {
		return nil, errors.New("x509: trailing data after ASN.1 of public-key")
	}
	if !pki.Algorithm.Algorithm.Equal(OidSM2) {
		return nil, fmt.Errorf("fail to unmarshal public key: curve is not SM2P256v1")
	}

	asn1Data := pki.PublicKey.RightAlign()
	x, y := elliptic.Unmarshal(P256Sm2(), asn1Data)
	if x == nil {
		return nil, errors.New("x509: failed to unmarshal elliptic curve point")
	}
	var xByte, yByte [BIG_NUM_SIZE]byte
	xStr := x.Text(16)
	for i := 0; i < BIG_NUM_SIZE-len(xStr); i++ {
		xByte[i] = '0'
	}
	copy(xByte[BIG_NUM_SIZE-len(xStr):], xStr)
	yStr := y.Text(16)
	for i := 0; i < BIG_NUM_SIZE-len(yStr); i++ {
		yByte[i] = '0'
	}
	copy(yByte[BIG_NUM_SIZE-len(yStr):], yStr)
	pkStr := []byte("04" + string(xByte[:]) + string(yByte[:]) + "\x00")

	var ctx C.sm2_ctx_t
	C.sm2_init_ctx(&ctx, unsafe.Pointer(&pkStr[0]))

	pk := PublicKey{
		Curve: P256Sm2(),
		X:     x,
		Y:     y,
		Text:  pkStr,
		ctx:   &ctx,
	}
	return &pk, nil
}

func PublicKeyFromPEM(pkPEM string) (*PublicKey, error) {
	pkBlock, _ := pem.Decode([]byte(pkPEM))
	return UnmarshalPublicKey(pkBlock.Bytes)
}

func PublicKeyToPEM(key *PublicKey) (string, error) {
	der, err := MarshalPublicKey(key)
	if err != nil {
		return "", err
	}
	pemPK := pem.EncodeToMemory(
		&pem.Block{
			Type:  "PUBLIC KEY",
			Bytes: der,
		})
	return string(pemPK), nil
}

// private key conversion
type ecPrivateKey struct {
	Version       int
	PrivateKey    []byte
	NamedCurveOID asn1.ObjectIdentifier `asn1:"optional,explicit,tag:0"`
	PublicKey     asn1.BitString        `asn1:"optional,explicit,tag:1"`
}

const ecPrivKeyVersion = 1

func UnmarshalPrivateKey(der []byte) (*PrivateKey, error) {
	return UnmarshalPrivateKeyWithCurve(nil, der)
}

func UnmarshalPrivateKeyWithCurve(namedCurveOID *asn1.ObjectIdentifier, der []byte) (*PrivateKey, error) {
	if der == nil {
		return nil, errors.New("input DER is null")
	}
	var privKeyPKCS8 pkcs8
	if _, err := asn1.Unmarshal(der, &privKeyPKCS8); err != nil {
		return nil, err
	}
	if !reflect.DeepEqual(privKeyPKCS8.Algo.Algorithm, OidSM2) {
		return nil, errors.New("x509: not sm2 elliptic curve")
	}

	var privKey sm2PrivateKeyInfo
	if _, err := asn1.Unmarshal(privKeyPKCS8.PrivateKey, &privKey); err != nil {
		return nil, errors.New("x509: failed to parse SM2 private key: " + err.Error())
	}
	curve := P256Sm2()
	k := new(big.Int).SetBytes(privKey.PrivateKey)
	curveOrder := curve.Params().N
	if k.Cmp(curveOrder) >= 0 {
		return nil, errors.New("x509: invalid elliptic curve private key value")
	}
	priv := new(PrivateKey)
	priv.Curve = curve
	priv.D = k
	privateKey := make([]byte, (curveOrder.BitLen()+7)/8)
	for len(privKey.PrivateKey) > len(privateKey) {
		if privKey.PrivateKey[0] != 0 {
			return nil, errors.New("x509: invalid private key length")
		}
		privKey.PrivateKey = privKey.PrivateKey[1:]
	}
	copy(privateKey[len(privateKey)-len(privKey.PrivateKey):], privKey.PrivateKey)
	priv.X, priv.Y = curve.ScalarBaseMult(privateKey)
	var xByte, yByte [BIG_NUM_SIZE]byte
	xStr := priv.X.Text(16)
	for i := 0; i < BIG_NUM_SIZE-len(xStr); i++ {
		xByte[i] = '0'
	}
	copy(xByte[BIG_NUM_SIZE-len(xStr):], xStr)
	yStr := priv.Y.Text(16)
	for i := 0; i < BIG_NUM_SIZE-len(yStr); i++ {
		yByte[i] = '0'
	}
	copy(yByte[BIG_NUM_SIZE-len(yStr):], yStr)
	priv.PublicKey.Text = []byte("04" + string(xByte[:]) + string(yByte[:]) + "\x00")

	var ctx C.sm2_ctx_t
	C.sm2_init_ctx(&ctx, unsafe.Pointer(&priv.PublicKey.Text[0]))
	priv.PublicKey.ctx = &ctx

	var dByte [BIG_NUM_SIZE]byte
	dStr := k.Text(16)
	for i := 0; i < BIG_NUM_SIZE-len(dStr); i++ {
		dByte[i] = '0'
	}
	copy(dByte[BIG_NUM_SIZE-len(dStr):], dStr)
	priv.Text = []byte(string(dByte[:]) + "\x00")

	return priv, nil
}

func MarshalPrivateKey(key *PrivateKey) ([]byte, error) {
	if key == nil {
		return nil, errors.New("input SM2 private key is null")
	}

	var r pkcs8
	var priv sm2PrivateKeyInfo
	var algo pkix.AlgorithmIdentifier

	algo.Algorithm = OidSM2
	algo.Parameters.Class = 0
	algo.Parameters.Tag = 6
	algo.Parameters.IsCompound = false
	algo.Parameters.FullBytes = []byte{6, 8, 42, 129, 28, 207, 85, 1, 130, 45} // asn1.Marshal(asn1.ObjectIdentifier{1, 2, 156, 10197, 1, 301})
	priv.Version = 1
	priv.NamedCurveOID = OidNamedCurveSm2
	priv.PublicKey = asn1.BitString{Bytes: elliptic.Marshal(key.Curve, key.X, key.Y)}
	priv.PrivateKey = key.D.Bytes()
	r.Version = 0
	r.Algo = algo
	r.PrivateKey, _ = asn1.Marshal(priv)
	return asn1.Marshal(r)
}

func PrivateKeyFromPEM(skPEM string) (*PrivateKey, error) {
	pemBlock, _ := pem.Decode([]byte(skPEM))
	return UnmarshalPrivateKey(pemBlock.Bytes)
}

func PrivateKeyToPEM(key *PrivateKey) (string, error) {
	der, err := MarshalPrivateKey(key)
	if err != nil {
		return "", err
	}
	skPEM := pem.EncodeToMemory(&pem.Block{
		Type:  "PRIVATE KEY",
		Bytes: der,
	})
	return string(skPEM), nil
}
