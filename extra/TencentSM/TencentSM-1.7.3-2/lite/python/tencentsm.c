#include <Python.h>
#include "sm.h"

static sm2_ctx_t sm2_ctx;
static sm3_ctx_t sm3_ctx;
static PyObject *smError;

#define SM2_ERROR "sm2.error"
#define SM4_ERROR "sm4.error"

static PyObject * _version(PyObject *self, PyObject *args)
{
    const char *v = version();
    return PyUnicode_FromString(v);  
}

static PyObject * _SM2Init(PyObject *self, PyObject *args)
{
    SM2InitCtx(&sm2_ctx);
    return PyLong_FromLong(0);
}


static PyObject * _SM2Free(PyObject *self, PyObject *args)
{
    SM2FreeCtx(&sm2_ctx);
    return PyLong_FromLong(0);
}

static PyObject * _generatePrivateKey(PyObject *self, PyObject *args)
{
    char private_key[65] = {0};
    int ret = generatePrivateKey(&sm2_ctx,private_key);
    if(ret != 0)
    {
        char errMsg[1024] = {0};
        smError = PyErr_NewException(SM2_ERROR, NULL, NULL);
        snprintf(errMsg,1024,"sm2 generate private key error with code:%d",ret);
        PyErr_SetString(smError, errMsg);
        return NULL;
    }

    return PyUnicode_FromString(private_key);
}

static PyObject * _generatePublicKey(PyObject *self, PyObject *args)
{
    char *private_key;
 
    if (!PyArg_ParseTuple(args, "s", &private_key))
        return NULL;


    char public_key[131] = {0};
    int ret = generatePublicKey(&sm2_ctx,private_key,public_key);
    if(ret != 0)
    {
        char errMsg[1024] = {0};
        smError = PyErr_NewException(SM2_ERROR, NULL, NULL);
        snprintf(errMsg,1024,"sm2 generate public key error with code:%d",ret);
        PyErr_SetString(smError, errMsg);
        return NULL;
    }

    return PyUnicode_FromString(public_key);
}

static PyObject * _generateKeyPair(PyObject *self, PyObject *args)
{
    char private_key[65] = {0};
    char public_key[131] = {0};
    int ret = generateKeyPair(&sm2_ctx,private_key,public_key);
    if(ret != 0)
    {
        char errMsg[1024] = {0};
        smError = PyErr_NewException(SM2_ERROR, NULL, NULL);
        snprintf(errMsg,1024,"sm2 generate key pair error with code:%d",ret);
        PyErr_SetString(smError, errMsg);
        return NULL;
    }

    return Py_BuildValue("(s,s)", private_key,public_key);
}

static PyObject * _SM2Encrypt(PyObject *self, PyObject *args)
{
    char *in;
    int inlen;
    int publen;
    char *public_key;

    Py_buffer  buffer;

 
    if (!PyArg_ParseTuple(args, "s#s", &in,&inlen,&public_key))
        return NULL;

    size_t cipherlen =  inlen + 200;
    unsigned char *cipher = (unsigned char *)malloc(cipherlen);
    int ret = SM2Encrypt(&sm2_ctx,(const unsigned char*)in,(size_t)inlen,public_key,strlen(public_key),cipher,&cipherlen);
    if(ret != 0)
    {
        char errMsg[1024] = {0};
        smError = PyErr_NewException(SM2_ERROR, NULL, NULL);
        snprintf(errMsg,1024,"sm2 encrypt error with code:%d",ret);
        PyErr_SetString(smError, errMsg);
        free(cipher);
        return NULL;
    }
#ifdef PYTHON3
    PyObject* pyObj = Py_BuildValue("y#", cipher,cipherlen);
#else
    PyObject* pyObj = Py_BuildValue("s#", cipher,cipherlen);
#endif
    free(cipher);
    return pyObj;
}

static PyObject * _SM2EncryptWithMode(PyObject *self, PyObject *args)
{
    char *in;
    int inlen;
    char *public_key;
    int mode;
 
    if (!PyArg_ParseTuple(args, "s#si", &in,&inlen,&public_key,&mode))
        return NULL;

    size_t cipherlen =  inlen + 220;
    unsigned char *cipher = (unsigned char *)malloc(cipherlen);
    int ret = SM2EncryptWithMode(&sm2_ctx,(const unsigned char*)in,(size_t)inlen,public_key,strlen(public_key),cipher,&cipherlen,(SM2CipherMode)mode);
    if(ret != 0)
    {
        char errMsg[1024] = {0};
        smError = PyErr_NewException(SM2_ERROR, NULL, NULL);
        snprintf(errMsg,1024,"sm2 encrypt with mode error with code:%d",ret);
        PyErr_SetString(smError, errMsg);
        free(cipher);
        return NULL;
    }

#ifdef PYTHON3
    PyObject* pyObj = Py_BuildValue("y#", cipher,cipherlen);
#else
    PyObject* pyObj = Py_BuildValue("s#", cipher,cipherlen);
#endif
    free(cipher);
    return pyObj;
}

static PyObject * _SM2Decrypt(PyObject *self, PyObject *args)
{
    char *in;
    int inlen;
    char *private_key;
 
    if (!PyArg_ParseTuple(args, "s#s", &in,&inlen,&private_key))
        return NULL;

    size_t plainlen =  inlen + 200;
    unsigned char *plain = (unsigned char *)malloc(plainlen);
    int ret = SM2Decrypt(&sm2_ctx,(const unsigned char*)in,(size_t)inlen,private_key,strlen(private_key),plain,&plainlen);
    if(ret != 0)
    {
        char errMsg[1024] = {0};
        smError = PyErr_NewException(SM2_ERROR, NULL, NULL);
        snprintf(errMsg,1024,"sm2 decrypt error with code:%d",ret);
        PyErr_SetString(smError, errMsg);
        free(plain);
        return NULL;
    }
    
#ifdef PYTHON3
    PyObject* pyObj = Py_BuildValue("y#", plain,plainlen);
#else
    PyObject* pyObj = Py_BuildValue("s#", plain,plainlen);
#endif
    free(plain);
    return pyObj;
}

static PyObject * _SM2DecryptWithMode(PyObject *self, PyObject *args)
{
    char *in;
    int inlen;
    char *private_key;
    int mode;
 
    if (!PyArg_ParseTuple(args, "s#si", &in,&inlen,&private_key,&mode))
        return NULL;

    size_t plainlen =  inlen + 220;
    unsigned char *plain = (unsigned char *)malloc(plainlen);
    int ret = SM2DecryptWithMode(&sm2_ctx,(const unsigned char*)in,(size_t)inlen,private_key,strlen(private_key),plain,&plainlen,(SM2CipherMode)mode);
    if(ret != 0)
    {
        char errMsg[1024] = {0};
        smError = PyErr_NewException(SM2_ERROR, NULL, NULL);
        snprintf(errMsg,1024,"sm2 decrypt with mode error with code:%d",ret);
        PyErr_SetString(smError, errMsg);
        free(plain);
        return NULL;
    }

#ifdef PYTHON3
    PyObject* pyObj = Py_BuildValue("y#", plain,plainlen);
#else
    PyObject* pyObj = Py_BuildValue("s#", plain,plainlen);
#endif
    free(plain);
    return pyObj;
}

static PyObject * _SM2Sign(PyObject *self, PyObject *args)
{
    char *msg,*id,*public_key,*private_key;
    int msglen,idlen;
 
    if (!PyArg_ParseTuple(args, "s#s#ss", &msg,&msglen,&id,&idlen,&public_key,&private_key))
        return NULL;

    size_t pubkeylen = strlen(public_key);
    size_t prikeylen = strlen(private_key);

    size_t siglen =  200;
    unsigned char *sig = (unsigned char *)malloc(siglen);

    int ret = SM2Sign(&sm2_ctx,(const unsigned char*)msg,(size_t)msglen,id,(size_t)idlen,(const char*)public_key,pubkeylen,(const char*)private_key,prikeylen,sig,&siglen);
    if(ret != 0)
    {
        char errMsg[1024] = {0};
        smError = PyErr_NewException(SM2_ERROR, NULL, NULL);
        snprintf(errMsg,1024,"sm2 sign error with code:%d",ret);
        PyErr_SetString(smError, errMsg);
        free(sig);
        return NULL;
    }

#ifdef PYTHON3
    PyObject* pyObj = Py_BuildValue("y#", sig,siglen);
#else
    PyObject* pyObj = Py_BuildValue("s#", sig,siglen);
#endif
    free(sig);
    return pyObj;
}

static PyObject * _SM2SignWithMode(PyObject *self, PyObject *args)
{
    char *msg,*id,*public_key,*private_key;
    int msglen,idlen;
    int mode;
 
    if (!PyArg_ParseTuple(args, "s#s#ssi", &msg,&msglen,&id,&idlen,&public_key,&private_key,&mode))
        return NULL;

    size_t pubkeylen = strlen(public_key);
    size_t prikeylen = strlen(private_key);

    size_t siglen =  200;
    unsigned char *sig = (unsigned char *)malloc(siglen);

    int ret = SM2SignWithMode(&sm2_ctx,(const unsigned char*)msg,(size_t)msglen,id,(size_t)idlen,(const char*)public_key,pubkeylen,(const char*)private_key,prikeylen,sig,&siglen,(SM2SignMode)mode);
    if(ret != 0)
    {
        char errMsg[1024] = {0};
        smError = PyErr_NewException(SM2_ERROR, NULL, NULL);
        snprintf(errMsg,1024,"sm2 sign with mode error with code:%d",ret);
        PyErr_SetString(smError, errMsg);
        free(sig);
        return NULL;
    }
#ifdef PYTHON3
    PyObject* pyObj = Py_BuildValue("y#", sig,siglen);
#else
    PyObject* pyObj = Py_BuildValue("s#", sig,siglen);
#endif
    free(sig);
    return pyObj;
}

static PyObject * _SM2Verify(PyObject *self, PyObject *args)
{
    char *msg,*id,*sig,*public_key;
    int msglen,idlen,siglen;
 
    if (!PyArg_ParseTuple(args, "s#s#s#s", &msg,&msglen,&id,&idlen,&sig,&siglen,&public_key))
        return NULL;

    size_t pubkeylen = strlen(public_key);

    int ret = SM2Verify(&sm2_ctx,(const unsigned char*)msg,(size_t)msglen,id,(size_t)idlen,(const unsigned char*)sig,(size_t)siglen,(const char*)public_key,pubkeylen);
    if(ret != 0)
    {
        char errMsg[1024] = {0};
        smError = PyErr_NewException(SM2_ERROR, NULL, NULL);
        snprintf(errMsg,1024,"sm2 verify error with code:%d",ret);
        PyErr_SetString(smError, errMsg);
        return NULL;
    }

    PyObject* pyObj = PyLong_FromLong(ret);
    return pyObj;
}

static PyObject * _SM2VerifyWithMode(PyObject *self, PyObject *args)
{
    char *msg,*id,*sig,*public_key;
    int msglen,idlen,siglen;
    int mode;
 
    if (!PyArg_ParseTuple(args, "s#s#s#si", &msg,&msglen,&id,&idlen,&sig,&siglen,&public_key,&mode))
        return NULL;

    size_t pubkeylen = strlen(public_key);

    int ret = SM2VerifyWithMode(&sm2_ctx,(const unsigned char*)msg,(size_t)msglen,id,(size_t)idlen,(const unsigned char*)sig,(size_t)siglen,(const char*)public_key,pubkeylen,(SM2SignMode)mode);
    if(ret != 0)
    {
        char errMsg[1024] = {0};
        smError = PyErr_NewException(SM2_ERROR, NULL, NULL);
        snprintf(errMsg,1024,"sm2 verify with mode error with code:%d",ret);
        PyErr_SetString(smError, errMsg);
        return NULL;
    }

    PyObject* pyObj = PyLong_FromLong(ret);
    return pyObj;
}

static PyObject * _SM3Init(PyObject *self, PyObject *args)
{
    SM3Init(&sm3_ctx);
    return PyLong_FromLong(0);
}

static PyObject * _SM3Update(PyObject *self, PyObject *args)
{
    char *data;
    int datalen;
 
    if (!PyArg_ParseTuple(args, "s#", &data,&datalen))
        return NULL;

    SM3Update(&sm3_ctx,(const unsigned char*)data,(size_t)datalen);

    return PyLong_FromLong(0);
}

static PyObject * _SM3Final(PyObject *self, PyObject *args)
{
    unsigned char digest[32] = {0};

    SM3Final(&sm3_ctx,digest);


#ifdef PYTHON3
    return Py_BuildValue("y#", digest,32);
#else
    return Py_BuildValue("s#", digest,32);
#endif
}

static PyObject * _generateSM4Key(PyObject *self, PyObject *args)
{
    unsigned char key[16] = {0};

    generateSM4Key(key);

#ifdef PYTHON3
    return Py_BuildValue("y#", key,16);
#else
    return Py_BuildValue("s#", key,16);
#endif
}

static PyObject * _SM4_CBC_Encrypt(PyObject *self, PyObject *args)
{
    char *in,*key,*iv;
    int inlen,keylen,ivlen;
 
    if (!PyArg_ParseTuple(args, "s#s#s#", &in,&inlen,&key,&keylen,&iv,&ivlen))
        return NULL;
    if ((16 != keylen) || (16 != ivlen))
        return NULL;

    size_t outlen =  inlen + 100;
    unsigned char *out = (unsigned char *)malloc(outlen);

    SM4_CBC_Encrypt((const unsigned char*)in,(size_t)inlen,out,&outlen,(const unsigned char*)key,(const unsigned char*)iv);
    if(outlen == 0)
    {
        char errMsg[1024] = {0};
        smError = PyErr_NewException(SM4_ERROR, NULL, NULL);
        snprintf(errMsg,1024,"sm4 encrypt error,may be argument fault");
        PyErr_SetString(smError, errMsg);
        free(out);
        return NULL;
    }


#ifdef PYTHON3
    PyObject* pyObj = Py_BuildValue("y#", out,outlen);
#else
    PyObject* pyObj = Py_BuildValue("s#", out,outlen);
#endif

    free(out);
    return pyObj;
}

static PyObject * _SM4_CBC_Decrypt(PyObject *self, PyObject *args)
{
    char *in,*key,*iv;
    int inlen,keylen,ivlen;
 
    if (!PyArg_ParseTuple(args, "s#s#s#", &in,&inlen,&key,&keylen,&iv,&ivlen))
        return NULL;
    if ((16 != keylen) || (16 != ivlen))
        return NULL;

    size_t outlen =  inlen + 100;
    unsigned char *out = (unsigned char *)malloc(outlen);

    SM4_CBC_Decrypt((const unsigned char*)in,(size_t)inlen,out,&outlen,(const unsigned char*)key,(const unsigned char*)iv);
    if(outlen == 0)
    {
        char errMsg[1024] = {0};
        smError = PyErr_NewException(SM4_ERROR, NULL, NULL);
        snprintf(errMsg,1024,"sm4 decrypt error,may be argument fault");
        PyErr_SetString(smError, errMsg);
        free(out);
        return NULL;
    }

#ifdef PYTHON3
    PyObject* pyObj = Py_BuildValue("y#", out,outlen);
#else
    PyObject* pyObj = Py_BuildValue("s#", out,outlen);
#endif
    free(out);
    return pyObj;
}

static PyObject * _SM4_ECB_Encrypt(PyObject *self, PyObject *args)
{
    char *in,*key;
    int inlen,keylen;
 
    if (!PyArg_ParseTuple(args, "s#s#", &in,&inlen,&key,&keylen))
        return NULL;
    if (16 != keylen)
        return NULL;

    size_t outlen =  inlen + 100;
    unsigned char *out = (unsigned char *)malloc(outlen);

    SM4_ECB_Encrypt((const unsigned char*)in,(size_t)inlen,out,&outlen,(const unsigned char*)key);
    if(outlen == 0)
    {
        char errMsg[1024] = {0};
        smError = PyErr_NewException(SM4_ERROR, NULL, NULL);
        snprintf(errMsg,1024,"sm4 encrypt error,may be argument fault");
        PyErr_SetString(smError, errMsg);
        free(out);
        return NULL;
    }

#ifdef PYTHON3
    PyObject* pyObj = Py_BuildValue("y#", out,outlen);
#else
    PyObject* pyObj = Py_BuildValue("s#", out,outlen);
#endif
    free(out);
    return pyObj;
}

static PyObject * _SM4_ECB_Decrypt(PyObject *self, PyObject *args)
{
    char *in,*key;
    int inlen,keylen;
 
    if (!PyArg_ParseTuple(args, "s#s#", &in,&inlen,&key,&keylen))
        return NULL;
    if (16 != keylen)
        return NULL;

    size_t outlen =  inlen + 100;
    unsigned char *out = (unsigned char *)malloc(outlen);

    SM4_ECB_Decrypt((const unsigned char*)in,(size_t)inlen,out,&outlen,(const unsigned char*)key);
    if(outlen == 0)
    {
        char errMsg[1024] = {0};
        smError = PyErr_NewException(SM4_ERROR, NULL, NULL);
        snprintf(errMsg,1024,"sm4 decrypt error,may be argument fault");
        PyErr_SetString(smError, errMsg);
        free(out);
        return NULL;
    }

#ifdef PYTHON3
    PyObject* pyObj = Py_BuildValue("y#", out,outlen);
#else
    PyObject* pyObj = Py_BuildValue("s#", out,outlen);
#endif
    free(out);
    return pyObj;
}

static PyMethodDef TencentSMMethods[] = {
    {
        "version",
        _version,
        METH_VARARGS,
        ""
    },
    {
        "SM2Init",
        _SM2Init,
        METH_VARARGS,
        ""
    },
    {
        "SM2Free",
        _SM2Free,
        METH_VARARGS,
        ""
    },
    {
        "generatePrivateKey",
        _generatePrivateKey,
        METH_VARARGS,
        ""
    },
    {
        "generatePublicKey",
        _generatePublicKey,
        METH_VARARGS,
        ""
    },
    {
        "generateKeyPair",
        _generateKeyPair,
        METH_VARARGS,
        ""
    },
    {
        "SM2Encrypt",
        _SM2Encrypt,
        METH_VARARGS,
        ""
    },
    {
        "SM2Decrypt",
        _SM2Decrypt,
        METH_VARARGS,
        ""
    },
    {
        "SM2Sign",
        _SM2Sign,
        METH_VARARGS,
        ""
    },
    {
        "SM2Verify",
        _SM2Verify,
        METH_VARARGS,
        ""
    },
    {
        "SM2EncryptWithMode",
        _SM2EncryptWithMode,
        METH_VARARGS,
        ""
    },
    {
        "SM2DecryptWithMode",
        _SM2DecryptWithMode,
        METH_VARARGS,
        ""
    },
    {
        "SM2SignWithMode",
        _SM2SignWithMode,
        METH_VARARGS,
        ""
    },
    {
        "SM2VerifyWithMode",
        _SM2VerifyWithMode,
        METH_VARARGS,
        ""
    },
    {
        "SM3Init",
        _SM3Init,
        METH_VARARGS,
        ""
    },
    {
        "SM3Update",
        _SM3Update,
        METH_VARARGS,
        ""
    },
    {
        "SM3Final",
        _SM3Final,
        METH_VARARGS,
        ""
    },
    {
        "generateSM4Key",
        _generateSM4Key,
        METH_VARARGS,
        ""
    },
    {
        "SM4_CBC_Encrypt",
        _SM4_CBC_Encrypt,
        METH_VARARGS,
        ""
    },
    {
        "SM4_CBC_Decrypt",
        _SM4_CBC_Decrypt,
        METH_VARARGS,
        ""
    },
    {
        "SM4_ECB_Encrypt",
        _SM4_ECB_Encrypt,
        METH_VARARGS,
        ""
    },
    {
        "SM4_ECB_Decrypt",
        _SM4_ECB_Decrypt,
        METH_VARARGS,
        ""
    },
    {NULL, NULL, 0, NULL}
};

#ifdef PYTHON3

static struct PyModuleDef keywdargmodule = {
    PyModuleDef_HEAD_INIT,
    "tencentsm",
    NULL,
    -1,
    TencentSMMethods
};


PyMODINIT_FUNC PyInit_tencentsm(void)
{
    return PyModule_Create(&keywdargmodule);
}
#else
PyMODINIT_FUNC inittencentsm(void) {
    (void) Py_InitModule("tencentsm", TencentSMMethods);
}
#endif