/*
 * varbufproto.h
 *
 *  Created on: 2014.4.6
 *      Author: harlylei
 */

#ifndef VARBUFPROTO_H_
#define VARBUFPROTO_H_
#include <arpa/inet.h>

//Namespacde of variable buffer protocol
namespace VarBufNS {
  //variable structure
  struct VarValue {
    int valLen;//length of value
    int valIndex;//offset of value in buffer

    VarValue() {
      valLen = 0 ;
      valIndex = sizeof(int) ;
    }
  };


  struct VarBufMgn {
    //The previous 4 bytes stored the used length of varBuf
    unsigned char varBuf[(65536+3)&(~3)];//64k£¬protocol buffer

    VarBufMgn() {
      *(getUsedIndex()) = 4;
    }

    unsigned int *getUsedIndex() { //Get the pointer where stores buffer length
      return (unsigned int *)varBuf ;
    }

    unsigned int getUsedLen() { //Get the length that have been used in buffer
      return *(getUsedIndex());
    }

    //Allocate len bytes(actually len+1)
    //return 0 means failure
    unsigned int allocBuf(unsigned int len) {
      ++len;
      unsigned int *usedIndex = getUsedIndex();
      if( *usedIndex + len >= sizeof(varBuf) - 100) {
        return 0 ;
      }

      unsigned int ret = *usedIndex;
      *usedIndex += len;

      *getBuf(*usedIndex-1) = 0 ;

      return ret;
    }

    //Allocate space and set value
    char * allocVarValue(VarValue& value,unsigned int len) {
      unsigned int keyIndex = allocBuf(len);
      if( !keyIndex ) {
        return NULL;
      }

      value.valLen = len;
      value.valIndex = keyIndex;

      return getBuf(keyIndex);
    }

    char * assigenVarValue(VarValue& value,const void *key,unsigned int len) {
      char *dstBuf = allocVarValue(value,len);
      if (!dstBuf) {
        return NULL;
      }

      memcpy(dstBuf,key,len);

      return dstBuf;
    }

    char *assigenVarValue(VarValue& value, const VarValue& rft,VarBufMgn *rftMgn) {
      return assigenVarValue(value,
                            (const void*)(rftMgn->getBuf(rft.valIndex)),rft.valLen);
    }

    char * assigenVarValue(VarValue & value,const void * key) {
      return assigenVarValue(value,key,strlen((const char*)key));
    }

    char * getBuf(unsigned int index) {
      return (char*)varBuf+index;
    }
  };

#define CloudBaseConstruct\
    do{\
        varbufIndex = (unsigned int)( (unsigned char*)&m_varBuf - ((unsigned char *)this) );\
        decode();\
    }while(0);

#define CloudBaseFun(type)\
    public:\
    VarBufNS::CloudCommHead & assignBase(VarBufNS::CloudCommHead & base)\
    {\
        VarBufNS::CloudCommHead::operator=(base);\
    \
        return *this;\
    }\
    int computeLen()\
    {\
        int realLen = varbufIndex + getVarBuf()->getUsedLen() ;\
        \
        len  = htonl(realLen - 4 );\
        \
        return realLen;\
    }\
    \
    VarBufNS::VarBufMgn * getVarBuf()\
    {\
        return (VarBufNS::VarBufMgn * )( ((unsigned char *)this ) + varbufIndex );\
    \
    }\
    type * product()\
    {\
        computeLen();\
        type * newobj = (type*)new char[ getLen() ];\
        memcpy(newobj,this,getLen());\
        newobj->decode();\
        return newobj;\
    }


#define CloudCommInitVer 1 //Version number

//Common structure for network communication
//Only first varaible stored with network byte order, others use host byte order
#pragma pack(1)
struct CloudCommHead {
    uint32_t len;
    //Parse according to version and classname
    char classname[20]; 
    uint16_t chksum; //checksum
    uint16_t ver;//0: old protocol
    uint16_t timeout;
    uint16_t reserver;

    // Stores the real offset in protocol_buf. Becuase differnet structure or
    // upgrading to different version of same structure, using this to point
    // to the relative offset between this and protocol_buf
    int32_t varbufIndex;

    uint32_t cmdtype ;  //command type
    char cmdcode[32];       //command
    uint32_t innerseq ;     //inner seq number
    uint32_t backseq;// backup innerseq before genenrating new seq number
    uint32_t  clientip;         //client ip

    //For 32bit machine, we need to add extra 4bytes to get aligned so that it can
    //be compatible with 64bit machine. If the table structure is changed, this element
    //must be verified
    #if __WORDSIZE==32
    uint32_t isnull;
    #endif
    CloudCommHead() {
        memset(this,0,sizeof(*this));

        ver = CloudCommInitVer; 
    }

    CloudCommHead & operator=(const CloudCommHead & lft) {
        //don't set len
        //don't set classname
        chksum = 0 ;
        ver = lft.ver;
        timeout = lft.timeout;
        reserver = lft.reserver;

        //don't set varbufIndex

        cmdtype = lft.cmdtype;
        memcpy(cmdcode,lft.cmdcode,sizeof(cmdcode));
        innerseq = lft.innerseq;
        backseq = lft.backseq;
        clientip = lft.clientip;

        return *this;
    }

    int getLen() const {
        return ntohl(len)+4;
    }

    static unsigned int getMinLen() {
        return (unsigned int)(sizeof(CloudCommHead));
    }

};
#pragma pack()
}

#endif /* VARBUFPROTO_H_ */
