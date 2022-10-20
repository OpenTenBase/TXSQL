/* Copyright 2019, Tencent Technology (Shenzhen) Co Ltd
 
 This file is part of the Tencent SM (Lite Version) Library.
 
 The Tencent SM (Lite Version) Library is free software; you can redistribute it and/or modify
 it under the terms of either:
 
 * the GNU Lesser General Public License as published by the Free
 Software Foundation; either version 3 of the License, or (at your
 option) any later version.
 
 or
 
 * the GNU General Public License as published by the Free Software
 Foundation; either version 2 of the License, or (at your option) any
 later version.
 
 or both in parallel, as here.
 
 The Tencent SM (Lite Version) Library is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.
 
 You should have received copies of the GNU General Public License and the
 GNU Lesser General Public License along with the Tencent SM (Lite Version) Library.  If not,
 see https://www.gnu.org/licenses/.  */

#include "include/tc_global.h"
#include "include/tc_sm2.h"
#include "include/tc_sm3.h"
#include "include/tc_kms.h"
#include "include/tc_str.h"
#include "include/tc_kdf.h"
#include "include/tc_utils.h"
#include "include/tc_rand.h"
#include "include/tlv.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include "include/sm_jni.h"

#define KMS_FILE_HEADER ""
#define KMS_FILE_PATH_MAX_LENGTH 4096
#define KMS_PUBKEY_MAX_LENGTH 200
#define KMS_PRIKEY_MAX_LENGTH 100

typedef int kms_bool;

typedef struct key {
    uint8_t *data; // 0x1
} key;

typedef struct key_pair {
    uint8_t *pub_key; //0x1
    uint8_t *pri_key; //0x2
} key_pair;

#define KEY_TYPE         0x1
#define KEY_PAIR_TYPE    0x2

typedef struct key_info {
    uint8_t *des; //0x1
    uint16_t type; //0x2
    key k; //0x3
} key_info;

typedef struct key_pair_info {
    uint8_t *des; //0x1
    uint16_t type; //0x2
    key_pair kp;  //0x3
} key_pair_info;

void tc_kms_secret_salt(unsigned char *secret_salt)
{
    int a = 0x5309fd23;
    int b = 0x902b01b2;
    int c = 0xaff5072b;
    int d = 0x3ef6eacc;
    int e = 0x5476e841;
    int f = 0x902c87c3;
    int g = 0xae5f0df1;
    int h = 0x2bf78b98;
    
    unsigned char c1 = a^b;
    unsigned char c2 = c^d;
    unsigned char c3 = e^f;
    unsigned char c4 = g^h;
    unsigned char c5 = a^h;
    unsigned char c6 = b^g;
    unsigned char c7 = c^f;
    unsigned char c8 = d^e;
    
    unsigned char local_md[32];
    
    sm3_ctx_t md_ctx;
    tcsm_sm3_init_opt(&md_ctx);
    tcsm_sm3_update_opt(&md_ctx, &c1, 1);
    tcsm_sm3_update_opt(&md_ctx, &c2, 1);
    tcsm_sm3_update_opt(&md_ctx, &c3, 1);
    tcsm_sm3_update_opt(&md_ctx, &c4, 1);
    tcsm_sm3_update_opt(&md_ctx, &c5, 1);
    tcsm_sm3_update_opt(&md_ctx, &c6, 1);
    tcsm_sm3_update_opt(&md_ctx, &c7, 1);
    tcsm_sm3_update_opt(&md_ctx, &c8, 1);
    tcsm_sm3_final_opt(&md_ctx, local_md);
    memcpy(secret_salt, local_md, 32);
}

void tc_kms_secret(const char *factors[],int number_of_factors,unsigned char* secret)
{
    unsigned char local_md[32];
    
    sm3_ctx_t md_ctx;
    tcsm_sm3_init_opt(&md_ctx);
    
    for (int i = 0; i < number_of_factors; i++) {
        unsigned char factor_md[32];
        sm3_ctx_t factor_ctx;
        tcsm_sm3_init_opt(&factor_ctx);
        tcsm_sm3_update_opt(&factor_ctx, (const unsigned char*)factors[i], strlen(factors[i]));
        tcsm_sm3_final_opt(&factor_ctx, factor_md);
        tcsm_sm3_update_opt(&md_ctx, factor_md, 32);
    }
    unsigned char secret_salt[32];
    tc_kms_secret_salt(secret_salt);
    tcsm_sm3_update_opt(&md_ctx, secret_salt, 32);
    tcsm_sm3_final_opt(&md_ctx, local_md);
    memcpy(secret, local_md, 32);
}

void tc_kms_master_key(const char *factors[],int number_of_factors,unsigned char* master_key)
{
    unsigned char secret[32];
    tc_kms_secret(factors, number_of_factors, secret);
    tcsm_x9_63_kdf_sm3(secret,32,master_key,16);
}

unsigned char* tc_kms_iv(void)
{
    int a = 0x5309fd53;
    int b = 0x782b01b2;
    int c = 0xaef6072b;
    int d = 0x3ea2eacc;
    int e = 0x6776e841;
    int f = 0x872c834d;
    int g = 0xbe5f0df1;
    int h = 0x2bf93b98;
    
    unsigned char c1 = a^b;
    unsigned char c2 = c^d;
    unsigned char c3 = e^f;
    unsigned char c4 = g^h;
    unsigned char c5 = a^h;
    unsigned char c6 = b^g;
    unsigned char c7 = c^f;
    unsigned char c8 = d^e;
    
    unsigned char local_md[32];
    
    sm3_ctx_t md_ctx;
    tcsm_sm3_init_opt(&md_ctx);
    tcsm_sm3_update_opt(&md_ctx, &c1, 1);
    tcsm_sm3_update_opt(&md_ctx, &c2, 1);
    tcsm_sm3_update_opt(&md_ctx, &c3, 1);
    tcsm_sm3_update_opt(&md_ctx, &c4, 1);
    tcsm_sm3_update_opt(&md_ctx, &c5, 1);
    tcsm_sm3_update_opt(&md_ctx, &c6, 1);
    tcsm_sm3_update_opt(&md_ctx, &c7, 1);
    tcsm_sm3_update_opt(&md_ctx, &c8, 1);
    tcsm_sm3_final_opt(&md_ctx, local_md);
  
    unsigned char* iv = tcsm_tc_malloc(16);
    memcpy(iv, local_md, 16);
    return iv;
}

int tc_kms_file_path(const char *factors[],int number_of_factors,const char * dir_path,char * kms_file_path,int kms_file_path_length)
{
    if( access( dir_path, F_OK ) != -1 ) {
        unsigned char local_md[32];
        sm3_ctx_t md_ctx;
        tcsm_sm3_init_opt(&md_ctx);
        
        for (int i = 0; i < number_of_factors; i++) {
            unsigned char factor_md[32];
            sm3_ctx_t factor_ctx;
            tcsm_sm3_init_opt(&factor_ctx);
            tcsm_sm3_update_opt(&factor_ctx, (const unsigned char*)factors[i], strlen(factors[i]));
            tcsm_sm3_final_opt(&factor_ctx, factor_md);
            tcsm_sm3_update_opt(&md_ctx, factor_md, 32);
        }
        tcsm_sm3_final_opt(&md_ctx, local_md);
        
        char kms_file_name[65] = {0};
        tcsm_bin2hex(local_md, 32, kms_file_name, 65);
        
        char file_path[KMS_FILE_PATH_MAX_LENGTH] = {0};
        int dirpathlen = (int)strlen(dir_path);
        strncpy(file_path, dir_path, dirpathlen);
        
        if (dir_path[dirpathlen - 1] != '/') {
            strcat(file_path, "/");
        }
        char * full_file_path = strcat(file_path, kms_file_name);
        memset(kms_file_path, 0, kms_file_path_length);
        memcpy(kms_file_path, full_file_path, strlen(full_file_path));
        return KMS_STATUS_SUCCESS;
    }else{
        return KMS_STATUS_PATH_NOT_EXIST;
    }
}

int tc_remove_kms_file(const char *factors[],int number_of_factors,const char * dir_path)
{
    char full_file_path[KMS_FILE_PATH_MAX_LENGTH] = {0};
    int ret = tc_kms_file_path(factors, number_of_factors, dir_path, full_file_path, KMS_FILE_PATH_MAX_LENGTH);
    if (ret == KMS_STATUS_SUCCESS) {
        return remove(full_file_path);
    }else{
        return ret;
    }
}

int tc_read_kms_file(const char *factors[],int number_of_factors,const char * full_file_path, tlv_box_t **tlv_box)
{
    int ret = 0;
    FILE* fp = fopen(full_file_path, "rb+");
    LOGV("tc_read_kms_file fopen=%d", fp); 
    if (fp == NULL) {
        ret = KMS_STATUS_UNKNOWN;
        goto err;
    }
    
    size_t file_size = tcsm_get_file_size(fp);
    unsigned char* content = tcsm_tc_malloc(file_size);
    
    if (content != NULL) {
        size_t read_size = fread(content, 1, file_size, fp);
        if (read_size != file_size) {
            tcsm_tc_free(content);
            ret = KMS_STATUS_UNKNOWN;
            goto err;
        }else{
            if (read_size == 0) {
                tcsm_tc_free(content);
                ret = KMS_STATUS_SUCCESS;
                goto err;
            }else{
                unsigned char master_key[16] = {0};
                tc_kms_master_key(factors, number_of_factors, master_key);
                
                unsigned char* iv = tc_kms_iv();
                
                char* plain = tcsm_tc_malloc(read_size);
                size_t plain_length = 0;
                
                SM4_CBC_Decrypt((const unsigned char*)content, (size_t)read_size, (unsigned char*)plain, &plain_length, (const unsigned char*)master_key, (const unsigned char*)iv);
                
                tcsm_tc_free(iv);
                tcsm_tc_free(content);

                int err = 0;
                tlv_box_t *box = tlv_parse((const uint8_t *)plain, plain_length, &err);
                
                tcsm_tc_free(plain);
                
                if (box == NULL) {
                    ret = KMS_STATUS_UNKNOWN;
                    goto err;
                }else{
                    *tlv_box = box;
                    ret = KMS_STATUS_SUCCESS;
                    goto ret;
                }
            }
        }
    }else{
        ret = KMS_STATUS_UNKNOWN;
        goto err;
    }
    
err:
    *tlv_box = NULL;
ret:
    fclose(fp);
    return ret;
}

int tc_read_objects(const char *factors[],int number_of_factors,const char * dir_path, tlv_box_t **tlv_box)
{
    char file_path[KMS_FILE_PATH_MAX_LENGTH] = {0};
    int ret = tc_kms_file_path(factors, number_of_factors, dir_path, file_path, KMS_FILE_PATH_MAX_LENGTH);
    if (ret == KMS_STATUS_SUCCESS) {
        if ( access( file_path, F_OK ) != -1 ) {
            return tc_read_kms_file(factors, number_of_factors, file_path, tlv_box);
        }
    }
err:
    *tlv_box = NULL;
    return KMS_STATUS_UNKNOWN;
}

int tc_write_kms_file(const char *factors[],int number_of_factors,const char * full_file_path,tlv_box_t *box)
{
    int ret = 0;
    FILE* fp = fopen(full_file_path, "wb");
    if (fp == NULL) {
        ret = KMS_STATUS_UNKNOWN;
        return ret;
    }
    
    uint8_t *buf = NULL;
    size_t size = 0;
    tlv_serialized_box(box, &buf, &size);
    
    unsigned char master_key[16] = {0};
    tc_kms_master_key(factors, number_of_factors, master_key);
    
    unsigned char* iv = tc_kms_iv();
    char* cipher = tcsm_tc_malloc(size + 16);
    size_t cipher_length = 0;
    
    SM4_CBC_Encrypt((const unsigned char*)buf, size, (unsigned char*)cipher, &cipher_length, (const unsigned char*)master_key, (unsigned char*)iv);
    tcsm_tc_free(iv);
    
    size_t w_size = fwrite(cipher, 1, cipher_length, fp);
    tcsm_tc_free(cipher);
    if (w_size != cipher_length) {
        ret = KMS_STATUS_UNKNOWN;
    }

    fclose(fp);
    return ret;
}

int tc_write_objects(const char *factors[],int number_of_factors,const char * dir_path, tlv_box_t *box)
{
    char file_path[KMS_FILE_PATH_MAX_LENGTH] = {0};
    int ret = tc_kms_file_path(factors, number_of_factors, dir_path, file_path, KMS_FILE_PATH_MAX_LENGTH);
    if (ret == KMS_STATUS_SUCCESS) {
        if ( access( file_path, F_OK ) != -1 ) {
            return tc_write_kms_file(factors, number_of_factors, file_path, box);
        }
    }
err:
    return KMS_STATUS_UNKNOWN;
}

int tc_check_kms_file(const char *factors[],int number_of_factors,const char * dir_path)
{
    char full_file_path[KMS_FILE_PATH_MAX_LENGTH] = {0};
    int ret = tc_kms_file_path(factors, number_of_factors, dir_path, full_file_path, KMS_FILE_PATH_MAX_LENGTH);
    if (ret == KMS_STATUS_SUCCESS) {
        if ( access( full_file_path, F_OK ) != -1 ) {
            tlv_box_t *box;
            int ret = tc_read_kms_file(factors, number_of_factors, full_file_path, &box);
            if (box) {
                tlv_box_destroy(box);
            }
            printf("[TSM][KMS] check kms file with code:%d ...\n",ret);
            return ret;
        }else{
            umask(0000);
            int fd = open(full_file_path, O_CREAT | O_RDWR | O_TRUNC | O_EXCL,S_IRWXU);
            if (fd != -1) {
                write(fd, KMS_FILE_HEADER, strlen(KMS_FILE_HEADER));
                close(fd);
                printf("[TSM][KMS] create kms file success...\n");
                return KMS_STATUS_SUCCESS;
            }else{
                return KMS_STATUS_UNKNOWN;
            }
        }
    }else{
        return ret;
    }
}

int tc_generate_sm4_key_write_kms(const char *factors[],int number_of_factors,const char * dir_path,const char* description,int force_update)
{
    unsigned char sm4key[16];
    void* r_ctx = tcsm_tc_rand_init();
    tcsm_tc_rand_bytes(r_ctx,sm4key,16);
    tcsm_tc_free(r_ctx);
    
    char str_sm4_key[33];
    tcsm_bin2hex(sm4key, 16, str_sm4_key, 33);
    
    return tc_import_key_with_description(factors, number_of_factors, dir_path, description,str_sm4_key, force_update);
}

int tc_generate_sm2_key_pair_write_kms(const char *factors[],int number_of_factors,const char * dir_path,const char* description,int force_update)
{
    sm2_ctx_t ctx;
    SM2InitCtx(&ctx);
    
    char pri[KMS_PRIKEY_MAX_LENGTH];
    char pub[KMS_PUBKEY_MAX_LENGTH];
    generateKeyPair(&ctx, pri, pub);
    
    SM2FreeCtx(&ctx);
    
    return tc_import_key_pair_with_description(factors, number_of_factors, dir_path, description, pub, pri, force_update);
}

static int filter_type(tlv_box_t *box, uint16_t type)
{
    tlv_entry_t  *element = NULL;
    int i=0;
    tlv_array_foreach(element, box) {
        int err = 0;
        tlv_box_t *ele_box = tlv_parse(element->t.data, element->t.length, &err);
        if (!ele_box) {
            continue;
        }   

        uint8_t *value = NULL;
        uint32_t des_len = 0;
        int ret = tlv_get_object_by_type(ele_box, 0x2, &value, &des_len);
        if (ret != 0 && des_len != sizeof(uint16_t)) {
            tlv_box_destroy(ele_box);
            continue;
        }

        if (*((uint16_t*)value) != type) {
            tlv_box_destroy(ele_box);
            continue;
        }

        ++i;
        tlv_box_destroy(ele_box);
    }

    return i;
}

int tc_description_count(const char *factors[],int number_of_factors,const char * dir_path,int* count, uint16_t type)
{
    tlv_box_t *box = NULL;
    int ret = tc_read_objects(factors, number_of_factors, dir_path, &box);
    *count = 0;
    if (ret == KMS_STATUS_SUCCESS) {
        if (box != NULL) {
            int ret = filter_type(box, type);
            if (ret >= 0) {
                *count = ret;
            }
            tlv_box_destroy(box);
        }else{
            *count = 0;
        }
    }else{
        *count = 0;
    }
    return ret;
}

int tc_description(const char *factors[],int number_of_factors,const char * dir_path,char* description[],int description_count, uint16_t type)
{
    tlv_box_t *box = NULL;
    int ret = tc_read_objects(factors, number_of_factors, dir_path, &box);
    if (ret == KMS_STATUS_SUCCESS) {
        if (box != NULL) {
            tlv_entry_t  *element = NULL;
            int i=0;
            tlv_array_foreach(element, box) {
                int err = 0;
                tlv_box_t *ele_box = tlv_parse(element->t.data, element->t.length, &err);
                if (!ele_box) {
                    continue;
                }   

                uint8_t *value = NULL;
                uint32_t des_len = 0;
                int ret = tlv_get_object_by_type(ele_box, 0x2, &value, &des_len);
                if (ret != 0 && des_len != sizeof(uint16_t)) {
                    tlv_box_destroy(ele_box);
                    continue;
                }

                if (*((uint16_t*)value) != type) {
                    tlv_box_destroy(ele_box);
                    continue;
                }

                ret = tlv_get_object_by_type(ele_box, 0x1, &value, &des_len);
                if (ret != 0) {
                    tlv_box_destroy(ele_box);
                    continue;
                }

                strncpy(description[i++], (char *)value, des_len);
                tlv_box_destroy(ele_box);
            }

            tlv_box_destroy(box);
        }else{
            *description = NULL;
        }
    }else{
        *description = NULL;
    }
    return ret;
}

int tc_all_key_description_count(const char *factors[],int number_of_factors,const char * dir_path,int* count)
{
    return tc_description_count(factors, number_of_factors, dir_path, count, KEY_TYPE);
}

int tc_all_key_description(const char *factors[],int number_of_factors,const char * dir_path,char* description[],int description_count)
{
    return tc_description(factors, number_of_factors, dir_path, description, description_count, KEY_TYPE);
}

int tc_all_key_pair_description_count(const char *factors[],int number_of_factors,const char * dir_path,int* count)
{
    return tc_description_count(factors, number_of_factors, dir_path, count, KEY_PAIR_TYPE);
}

int tc_all_key_pair_description(const char *factors[],int number_of_factors,const char * dir_path,char* description[],int description_count)
{
    return tc_description(factors, number_of_factors, dir_path, description, description_count, KEY_PAIR_TYPE);
}

static int tlv_key_pair_with_des(tlv_box_t *root_box, const char* description,char *pubkey, char *prikey)
{
    int size = tlv_get_object_size(root_box);
    if (size <= 0) {
        tlv_box_destroy(root_box);
        return KMS_STATUS_UNKNOWN;
    }

    tlv_entry_t  *element = NULL;
    tlv_array_foreach(element, root_box) {
        int err = 0;
        tlv_box_t *ele_box = tlv_parse(element->t.data, element->t.length, &err);
        if (!ele_box) {
            continue;
        }

        uint8_t *value = NULL;
        uint32_t des_len = 0;
        int ret = tlv_get_object_by_type(ele_box, 0x1, &value, &des_len);
        if (ret != 0) {
            tlv_box_destroy(ele_box);
            continue;
        }

        if (strlen(description) != des_len ||
                strncmp(description, (char *)value, des_len) != 0) {
            tlv_box_destroy(ele_box);
            continue;
        }

        ret = tlv_get_object_by_type(ele_box, 0x3, &value, &des_len);
        if (ret != 0) {
            tlv_box_destroy(ele_box);
            continue;
        }

        tlv_box_t *kp_item = tlv_parse(value, des_len, &err);
        tlv_box_destroy(ele_box);
        if (!kp_item) {
            return KMS_STATUS_UNKNOWN;
        }

        uint8_t *pub_key = NULL;
        uint8_t *priv_key = NULL;
        uint32_t pub_len = 0;
        uint32_t priv_len = 0;
        int pub_key_ret = tlv_get_object_by_type(kp_item, 0x1, &pub_key, &pub_len);
        int priv_key_ret = tlv_get_object_by_type(kp_item, 0x2, &priv_key, &priv_len);

        if (priv_key_ret != 0 && pub_key_ret != 0) {
            tlv_box_destroy(kp_item);
            return KMS_STATUS_UNKNOWN;
        }

        if (pub_key_ret == 0) {
            strncpy(pubkey, (char *)pub_key, pub_len);
        }

        if (priv_key_ret == 0) {
            strncpy(prikey, (char *)priv_key, priv_len);
        }

        tlv_box_destroy(kp_item);
        return KMS_STATUS_SUCCESS;
    }

    return KMS_STATUS_UNKNOWN;
}

int tc_key_pair_with_description(const char *factors[],int number_of_factors,const char * dir_path,const char* description,char *pubkey,char *prikey)
{
    tlv_box_t *root_box = NULL;
    int ret = tc_read_objects(factors, number_of_factors, dir_path, &root_box);
    if (ret == KMS_STATUS_SUCCESS) {
        if (root_box != NULL) {
            ret = tlv_key_pair_with_des(root_box, description, pubkey, prikey);
            tlv_box_destroy(root_box);
        }
    }
    return ret;
}

static int tlv_key_with_des(tlv_box_t *root_box, const char* description,char *key)
{
    int size = tlv_get_object_size(root_box);
    if (size <= 0) {
        tlv_box_destroy(root_box);
        return KMS_STATUS_UNKNOWN;
    }

    tlv_entry_t  *element = NULL;

    tlv_array_foreach(element, root_box) {
        int err = 0;
        tlv_box_t *ele_box = tlv_parse(element->t.data, element->t.length, &err);
        if (!ele_box) {
            continue;
        }

        uint8_t *value = NULL;
        uint32_t des_len = 0;
        int ret = tlv_get_object_by_type(ele_box, 0x1, &value, &des_len);
        if (ret != 0) {
            tlv_box_destroy(ele_box);
            continue;
        }

        if (strlen(description) != des_len ||
                strncmp(description, (char *)value, des_len) != 0) {
            tlv_box_destroy(ele_box);
            continue;
        }

        ret = tlv_get_object_by_type(ele_box, 0x3, &value, &des_len);
        if (ret != 0) {
            tlv_box_destroy(ele_box);
            continue;
        }

        tlv_box_t *k_item = tlv_parse(value, des_len, &err);
        tlv_box_destroy(ele_box);
        if (!k_item) {
            return KMS_STATUS_UNKNOWN;
        }

        ret = tlv_get_object_by_type(k_item, 0x1, &value, &des_len);
        if (ret < 0) {
            tlv_box_destroy(k_item);
            return KMS_STATUS_UNKNOWN;
        }

        strncpy(key, (char *)value, des_len);
        tlv_box_destroy(k_item);
        return KMS_STATUS_SUCCESS;
    }

    return KMS_STATUS_UNKNOWN;
}

int tc_key_with_description(const char *factors[],int number_of_factors,const char * dir_path,const char* description,char* key)
{
    tlv_box_t *root_box = NULL;
    int ret = tc_read_objects(factors, number_of_factors, dir_path, &root_box);
    if (ret == KMS_STATUS_SUCCESS) {
        if (root_box != NULL) {
            ret = tlv_key_with_des(root_box, description, key);
            tlv_box_destroy(root_box);
        }
    }
    return ret;
}

static kms_bool tlv_has_object_item(tlv_box_t *box, int type, const char *description)
{
    tlv_entry_t  *element = NULL;
    tlv_array_foreach(element, box) {
        int err = 0;
        tlv_box_t *ele_box = tlv_parse(element->t.data, element->t.length, &err);
        if (!ele_box) {
            continue;
        }
         
        uint8_t *value = NULL;
        uint32_t des_len = 0;
        int ret = tlv_get_object_by_type(ele_box, type, &value, &des_len);
        if (ret != 0) {
            tlv_box_destroy(ele_box);
            continue;
        }

        uint32_t in_des_len = (uint32_t)strlen(description);
        if (in_des_len != des_len) {
            tlv_box_destroy(ele_box);
            continue;
        }

        if (memcmp(description, value, des_len) == 0) {
            tlv_box_destroy(ele_box);
            return 1;
        }

        tlv_box_destroy(ele_box);
    }

    return 0;
}

static kms_bool tlv_del_object_item(tlv_box_t *root, const char *description)
{
    tlv_entry_t  *element = NULL;
    int i = -1;
    tlv_array_foreach(element, root) {
        ++i;
        int err = 0;
        tlv_box_t *ele_box = tlv_parse(element->t.data, element->t.length, &err);
        if (!ele_box) {
            continue;
        }

        uint8_t *value = NULL;
        uint32_t des_len = 0;
        int ret = tlv_get_object_by_type(ele_box, 0x1, &value, &des_len);
        if (ret != 0) {
            tlv_box_destroy(ele_box);
            continue;
        }

        uint32_t in_des_len = (uint32_t)strlen(description);
        if (in_des_len != des_len) {
            tlv_box_destroy(ele_box);
            continue;
        }

        if (memcmp(description, value, des_len) == 0) {
            tlv_box_destroy(ele_box);
            if (tlv_delete_item_from_array(root, i) != 0) {
                return 0;
            }

            return 1;
        }
    }
 
    return 0;
}

static void tlv_add_key_to_object(tlv_box_t *root, const char *description, const char *key)
{
    tlv_box_t *key_box = tlv_box_create();
    tlv_add_object(key_box, 0x1, (uint8_t *)key,  (uint32_t)strlen(key));

    tlv_box_t *item = tlv_box_create();
    tlv_add_object(item, 0x1, (uint8_t *)description,  (uint32_t)strlen(description));
    uint16_t type = KEY_TYPE;
    tlv_add_object(item, 0x2, (uint8_t *)&type, sizeof(uint16_t));

    uint8_t *bufptr = NULL;
    size_t size = 0;

    tlv_serialized_box(key_box, &bufptr, &size);
    tlv_add_object(item, 0x3, bufptr,  (uint32_t)size);
    
    tlv_add_object_to_array(root, item);
    tlv_box_destroy(key_box);
    tlv_box_destroy(item);
}

static void tlv_add_key_pair_to_object(tlv_box_t *root, const char *description, const char *pubkey, const char *privkey)
{
    tlv_box_t *key_pair = tlv_box_create();
    tlv_add_object(key_pair, 0x1, (uint8_t *)pubkey,  (uint32_t)strlen(pubkey));
    tlv_add_object(key_pair, 0x2, (uint8_t *)privkey,  (uint32_t)strlen(privkey));

    tlv_box_t *item = tlv_box_create();
    tlv_add_object(item, 0x1, (uint8_t *)description,  (uint32_t)strlen(description));
    uint16_t type = KEY_PAIR_TYPE;
    tlv_add_object(item, 0x2, (uint8_t *)&type, sizeof(uint16_t));

    uint8_t *bufptr = NULL;
    size_t size = 0;

    tlv_serialized_box(key_pair, &bufptr, &size);
    tlv_add_object(item, 0x3, bufptr,  (uint32_t)size);
    
    tlv_add_object_to_array(root, item);
    tlv_box_destroy(key_pair);
    tlv_box_destroy(item);
}

int tc_import_key_with_description(const char *factors[],int number_of_factors,const char * dir_path,const char* description,const char* key,int force_update)
{
    tlv_box_t *root_box = NULL;
    int ret = tc_read_objects(factors, number_of_factors, dir_path, &root_box);
    if (ret == KMS_STATUS_SUCCESS) {
        if (root_box == NULL) {
            root_box = tlv_box_create();
        }
        
        kms_bool hasObj = tlv_has_object_item(root_box, 0x1, description);
        if (hasObj) {
            if (force_update) {
                tlv_del_object_item(root_box, description);
                tlv_add_key_to_object(root_box, description, key);
            }else{
                printf("[TSM][KMS] import sm4 key to kms file failed since object existed...\n");
                return KMS_STATUS_DESCRIPTION_EXISTED;
            }
        }else{
            tlv_add_key_to_object(root_box, description, key);
        }
        int w_ret = tc_write_objects(factors, number_of_factors, dir_path, root_box);
        if (w_ret != KMS_STATUS_SUCCESS) {
            tlv_box_destroy(root_box);
            printf("[TSM][KMS] import sm4 key to kms file failed with code:%d...\n",w_ret);
            return KMS_STATUS_UNKNOWN;
        }
        tlv_box_destroy(root_box);
    }
    return ret;
}

int tc_import_key_pair_with_description(const char *factors[],int number_of_factors,const char * dir_path,const char* description,const char *pubkey,const char *prikey,int force_update)
{
    tlv_box_t *root_box = NULL;
    int ret = tc_read_objects(factors, number_of_factors, dir_path, &root_box);
    if (ret == KMS_STATUS_SUCCESS) {
        if (root_box == NULL) {
            root_box = tlv_box_create();
        }
        
        kms_bool hasObj = tlv_has_object_item(root_box, 0x1, description);
        if (!hasObj || (hasObj && force_update)) {
            tlv_del_object_item(root_box, description);
            tlv_add_key_pair_to_object(root_box, description, pubkey, prikey);
        }else{
            printf("[TSM][KMS] write sm4 key to kms file failed since object existed...\n");
            return KMS_STATUS_DESCRIPTION_EXISTED;
        }
        int w_ret = tc_write_objects(factors, number_of_factors, dir_path, root_box);
        if (w_ret != KMS_STATUS_SUCCESS) {
            tlv_box_destroy(root_box);
            printf("[TSM][KMS] write sm4 key to kms file failed with code:%d...\n",w_ret);
            return KMS_STATUS_UNKNOWN;
        }
        tlv_box_destroy(root_box);
    }
    return ret;
}

int tc_delete_description(const char *factors[],int number_of_factors,const char * dir_path,const char* description)
{
    tlv_box_t *root_box = NULL;
    int ret = tc_read_objects(factors, number_of_factors, dir_path, &root_box);
    if (ret == KMS_STATUS_SUCCESS) {
        if (root_box == NULL) {
            root_box = tlv_box_create();
        }
        
        kms_bool hasObj = tlv_has_object_item(root_box, 0x1, description);
        if (hasObj) {
            tlv_del_object_item(root_box, description);
        }else{
            tlv_box_destroy(root_box);
            return KMS_STATUS_DESCRIPTION_NOT_EXISTED;
        }
        
        int w_ret = tc_write_objects(factors, number_of_factors, dir_path, root_box);
        if (w_ret != KMS_STATUS_SUCCESS) {
            tlv_box_destroy(root_box);
            printf("[TSM][KMS] write sm4 key to kms file failed with code:%d...\n",w_ret);
            return KMS_STATUS_UNKNOWN;
        }
        tlv_box_destroy(root_box);
    }
    return ret;
}

/******************************************↓↓↓JNI 接口定义↓↓↓***********************************************************/
#ifdef JNI_INTERFACE
jint  JNICALL  Java_com_tenpay_utils_SMKeyManUtils_checkKmsFile(JNIEnv* env, jobject classobj, jobjectArray factors, jstring dir_path) {
    LOGV("Java_com_tenpay_utils_SMKeyManUtils_checkKmsFile 00");
    //dir_path
    const char* path =  (*env)->GetStringUTFChars(env, dir_path, NULL);
    //factors=>char * []
    int size = (*env)->GetArrayLength(env, factors);
    const char* factorstr[size];
    jstring tmp;
    for(int i = 0; i< size; i++) {
        tmp = (*env)->GetObjectArrayElement(env, factors, i);
        factorstr[i] = (*env)->GetStringUTFChars(env, tmp, NULL);
    }

    //call method
    return tc_check_kms_file(factorstr, size, path);
}
jint    Java_com_tenpay_utils_SMKeyManUtils_removeKmsFile(JNIEnv* env, jobject classobj, jobjectArray factors, jstring dir_path) {
    //dir_path
    const char* path =  (*env)->GetStringUTFChars(env, dir_path, NULL);
    //factors=>char * []
    int size = (*env)->GetArrayLength(env, factors);
    const char* factorstr[size];
    jstring tmp;
    for(int i = 0; i< size; i++) {
        tmp = (*env)->GetObjectArrayElement(env, factors, i);
        factorstr[i] = (*env)->GetStringUTFChars(env, tmp, NULL);
    }
    //call method
    return tc_remove_kms_file(factorstr, size, path);
}

jint    Java_com_tenpay_utils_SMKeyManUtils_SM4KeyGenWriteKms(JNIEnv* env, jobject classobj, jobjectArray factors, jstring dir_path, 
                                                                                jstring description,jint force_update) {
    //dir_path
    const char* pathStr =  (*env)->GetStringUTFChars(env, dir_path, NULL);
    //description
    const char* descStr =  (*env)->GetStringUTFChars(env, description, NULL);
    //force_update
    int force_updateI = force_update;
    //factors=>char * []
    int size = (*env)->GetArrayLength(env, factors);
    const char* factorstr[size];
    jstring tmp;
    for(int i = 0; i< size; i++) {
        tmp = (*env)->GetObjectArrayElement(env, factors, i);
        factorstr[i] = (*env)->GetStringUTFChars(env, tmp, NULL);
    }
    //call method
    return tc_generate_sm4_key_write_kms(factorstr, size, pathStr, descStr, force_updateI);
}
jint    Java_com_tenpay_utils_SMKeyManUtils_SM2KeyPairGenWriteKms(JNIEnv* env, jobject classobj, jobjectArray factors, jstring dir_path, 
                                                                                    jstring description,jint force_update) {
    //dir_path
    const char* pathStr =  (*env)->GetStringUTFChars(env, dir_path, NULL);
    //description
    const char* descStr =  (*env)->GetStringUTFChars(env, description, NULL);
    //force_update
    int force_updateI = force_update;
    //factors=>char * []
    int size = (*env)->GetArrayLength(env, factors);
    const char* factorstr[size];
    jstring tmp;
    for(int i = 0; i< size; i++) {
        tmp = (*env)->GetObjectArrayElement(env, factors, i);
        factorstr[i] = (*env)->GetStringUTFChars(env, tmp, NULL);
    }
    //call method
    return tc_generate_sm2_key_pair_write_kms(factorstr, size, pathStr, descStr, force_updateI);
}

/**
 * @return <0 失败   >=0 数量值
 */
jint    Java_com_tenpay_utils_SMKeyManUtils_allKeyDescriptionCount(JNIEnv* env, jobject classobj, jobjectArray factors, jstring dir_path) {
    //dir_path
    const char* pathStr =  (*env)->GetStringUTFChars(env, dir_path, NULL);
    //factors=>char * []
    int size = (*env)->GetArrayLength(env, factors);
    const char* factorstr[size];
    jstring tmp;
    for(int i = 0; i< size; i++) {
        tmp = (*env)->GetObjectArrayElement(env, factors, i);
        factorstr[i] = (*env)->GetStringUTFChars(env, tmp, NULL);
    }
    //call method
    int cout = -1;
    int result = tc_all_key_description_count(factorstr, size, pathStr, &cout);
    if(KMS_STATUS_SUCCESS == result) {
        return cout;
    } else {
        return -1;
    }
}
/**
 * @return null 失败   其它：descriptions string数组
 */
jobjectArray    Java_com_tenpay_utils_SMKeyManUtils_allKeyDescription(JNIEnv* env, jobject classobj, jobjectArray factors, jstring dir_path) {
    //dir_path
    const char* pathStr =  (*env)->GetStringUTFChars(env, dir_path, NULL);
    //factors=>char * []
    int size = (*env)->GetArrayLength(env, factors);
    const char* factorstr[size];
    jstring tmp;
    for(int i = 0; i< size; i++) {
        tmp = (*env)->GetObjectArrayElement(env, factors, i);
        factorstr[i] = (*env)->GetStringUTFChars(env, tmp, NULL);
    }
    //call method
    int cout = -1;
    int result = tc_all_key_description_count(factorstr, size, pathStr, &cout);
    if(KMS_STATUS_SUCCESS != result || cout <= 0) {
        return NULL;
    }
    char* descArray[cout];
    jobjectArray resultArray = NULL;
    for(int i = 0; i < cout; i++) {
        descArray[i] = malloc(sizeof(char) * SM_DESC_MAX_LEN);
    }
    result = tc_all_key_description(factorstr, size, pathStr, descArray, cout);
    if(result == KMS_STATUS_SUCCESS) {
        jclass stringarrCls = (*env)->FindClass(env, "java/lang/String");
        resultArray = (*env)->NewObjectArray(env, cout, stringarrCls, NULL);
        for(int i = 0; i < cout; i++) {
            (*env)->SetObjectArrayElement(env, resultArray, i, (*env)->NewStringUTF(env, descArray[i]));
        }
    }
    for(int i = 0; i < cout; i++) {
        free(descArray[i]);
    }
    return resultArray;
}

/**
 * @return <0 失败   >=0 数量值
 */
jint    Java_com_tenpay_utils_SMKeyManUtils_allKeyPairDescriptionCount(JNIEnv* env, jobject classobj, jobjectArray factors, jstring dir_path) {
    //dir_path
    const char* pathStr =  (*env)->GetStringUTFChars(env, dir_path, NULL);
    //factors=>char * []
    int size = (*env)->GetArrayLength(env, factors);
    const char* factorstr[size];
    jstring tmp;
    for(int i = 0; i< size; i++) {
        tmp = (*env)->GetObjectArrayElement(env, factors, i);
        factorstr[i] = (*env)->GetStringUTFChars(env, tmp, NULL);
    }
    //call method
    int cout = -1;
    int result = tc_all_key_pair_description_count(factorstr, size, pathStr, &cout);
    if(KMS_STATUS_SUCCESS == result) {
        return cout;
    } else {
        return -1;
    }
}
/**
 * @return null 失败   其它：descriptions string数组
 */
jobjectArray    Java_com_tenpay_utils_SMKeyManUtils_allKeyPairDescription(JNIEnv* env, jobject classobj, jobjectArray factors, jstring dir_path) {
    //dir_path
    const char* pathStr =  (*env)->GetStringUTFChars(env, dir_path, NULL);
    //factors=>char * []
    int size = (*env)->GetArrayLength(env, factors);
    const char* factorstr[size];
    jstring tmp;
    for(int i = 0; i< size; i++) {
        tmp = (*env)->GetObjectArrayElement(env, factors, i);
        factorstr[i] = (*env)->GetStringUTFChars(env, tmp, NULL);
    }
    //call method
    int cout = 0;
    int result = tc_all_key_pair_description_count(factorstr, size, pathStr, &cout);
    if(KMS_STATUS_SUCCESS != result || cout <= 0) {
        return NULL;
    }
    char* descArray[cout];
    jobjectArray resultArray = NULL;
    for(int i = 0; i < cout; i++) {
        descArray[i] = malloc(sizeof(char) * SM_DESC_MAX_LEN);
    }
    result = tc_all_key_pair_description(factorstr, size, pathStr, descArray, cout);
    if(result == KMS_STATUS_SUCCESS) {
        jclass stringarrCls = (*env)->FindClass(env, "java/lang/String");
        resultArray = (*env)->NewObjectArray(env, cout, stringarrCls, NULL);
        for(int i = 0; i < cout; i++) {
            (*env)->SetObjectArrayElement(env, resultArray, i, (*env)->NewStringUTF(env, descArray[i]));
        }
    }
    for(int i = 0; i < cout; i++) {
        free(descArray[i]);
    }
    return resultArray;
}

/**
 * @return null 失败   其它：string数组--[0]char *prikey, [1]char *pubkey,
 */
jobjectArray    Java_com_tenpay_utils_SMKeyManUtils_keyPairWithDescription(JNIEnv* env, jobject classobj, jobjectArray factors, jstring dir_path, jstring description) {
    //dir_path
    const char* pathStr =  (*env)->GetStringUTFChars(env, dir_path, NULL);
    //description
    const char* descStr =  (*env)->GetStringUTFChars(env, description, NULL);
    //factors=>char * []
    int size = (*env)->GetArrayLength(env, factors);
    const char* factorstr[size];
    jstring tmp;
    for(int i = 0; i< size; i++) {
        tmp = (*env)->GetObjectArrayElement(env, factors, i);
        factorstr[i] = (*env)->GetStringUTFChars(env, tmp, NULL);
    }
    //call method
    int cout = -1;
    char pubkey[KMS_PUBKEY_MAX_LENGTH], prikey[KMS_PRIKEY_MAX_LENGTH];
    memset(pubkey, 0x00, KMS_PUBKEY_MAX_LENGTH);
    memset(prikey, 0x00, KMS_PRIKEY_MAX_LENGTH);
    int result = tc_key_pair_with_description(factorstr, size, pathStr, descStr, pubkey, prikey);
    if(KMS_STATUS_SUCCESS != result) {
        return NULL;
    }
    jobjectArray resArray;
    jclass stringarrCls = (*env)->FindClass(env, "java/lang/String");
    resArray = (*env)->NewObjectArray(env, 2, stringarrCls, NULL);
    if(strlen(prikey) > 0) {
        (*env)->SetObjectArrayElement(env, resArray, 0, (*env)->NewStringUTF(env, prikey));
    }
    if(strlen(pubkey) > 0) {
        (*env)->SetObjectArrayElement(env, resArray, 1, (*env)->NewStringUTF(env, pubkey));
    }
    return resArray;
}
/**
 * @return null 失败   其它：string--char* key
 */
jstring    Java_com_tenpay_utils_SMKeyManUtils_keyWithDescription(JNIEnv* env, jobject classobj, jobjectArray factors, jstring dir_path, jstring description) {
    //dir_path
    const char* pathStr =  (*env)->GetStringUTFChars(env, dir_path, NULL);
    //description
    const char* descStr =  (*env)->GetStringUTFChars(env, description, NULL);
    //factors=>char * []
    int size = (*env)->GetArrayLength(env, factors);
    const char* factorstr[size];
    jstring tmp;
    for(int i = 0; i< size; i++) {
        tmp = (*env)->GetObjectArrayElement(env, factors, i);
        factorstr[i] = (*env)->GetStringUTFChars(env, tmp, NULL);
    }
    //call method
    int cout = -1;
    char symkey[KMS_SYMKEY_MAX_LENGTH];
    memset(symkey, 0x00, KMS_SYMKEY_MAX_LENGTH);
    int result = tc_key_with_description(factorstr, size, pathStr, descStr, symkey);
    if(KMS_STATUS_SUCCESS != result) {
        return NULL;
    }
    return (*env)->NewStringUTF(env, symkey);
}

jint    Java_com_tenpay_utils_SMKeyManUtils_importKeyWithDescription(JNIEnv* env, jobject classobj, jobjectArray factors, jstring dir_path, jstring description, jstring key, jint force_update) {
    //dir_path
    const char* pathStr =  (*env)->GetStringUTFChars(env, dir_path, NULL);
    //description
    const char* descStr =  (*env)->GetStringUTFChars(env, description, NULL);
    //key
    const char* keyStr =  (*env)->GetStringUTFChars(env, key, NULL);
    //force_update
    int force_updateI = force_update;
    //factors=>char * []
    int size = (*env)->GetArrayLength(env, factors);
    const char* factorstr[size];
    jstring tmp;
    for(int i = 0; i< size; i++) {
        tmp = (*env)->GetObjectArrayElement(env, factors, i);
        factorstr[i] = (*env)->GetStringUTFChars(env, tmp, NULL);
    }
    //call method
    return tc_import_key_with_description(factorstr, size, pathStr, descStr, keyStr, force_updateI);
}
jint    Java_com_tenpay_utils_SMKeyManUtils_importKeyPairWithDescription(JNIEnv* env, jobject classobj, jobjectArray factors, jstring dir_path, 
                                                    jstring description, jstring pubkey, jstring prikey, jint force_update) {
    //dir_path
    const char* pathStr =  (*env)->GetStringUTFChars(env, dir_path, NULL);
    //description
    const char* descStr =  (*env)->GetStringUTFChars(env, description, NULL);
    //pubkey
    const char* pubkeyStr =  NULL;
    if(pubkey != NULL) {
        pubkeyStr =  (*env)->GetStringUTFChars(env, pubkey, NULL);
    }
    //prikey
    const char* prikeyStr =  NULL;
    if(prikey != NULL) {
        prikeyStr =  (*env)->GetStringUTFChars(env, prikey, NULL);
    }
    //force_update
    int force_updateI = force_update;
    //factors=>char * []
    int size = (*env)->GetArrayLength(env, factors);
    const char* factorstr[size];
    jstring tmp;
    for(int i = 0; i< size; i++) {
        tmp = (*env)->GetObjectArrayElement(env, factors, i);
        factorstr[i] = (*env)->GetStringUTFChars(env, tmp, NULL);
    }
    //call method
    return tc_import_key_pair_with_description(factorstr, size, pathStr, descStr, pubkeyStr, prikeyStr, force_updateI);
}

jint    Java_com_tenpay_utils_SMKeyManUtils_delDescription(JNIEnv* env, jobject classobj, jobjectArray factors, jstring dir_path, jstring description) {
   //dir_path
    const char* pathStr =  (*env)->GetStringUTFChars(env, dir_path, NULL);
    //description
    const char* descStr =  (*env)->GetStringUTFChars(env, description, NULL);
    //factors=>char * []
    int size = (*env)->GetArrayLength(env, factors);
    const char* factorstr[size];
    jstring tmp;
    for(int i = 0; i< size; i++) {
        tmp = (*env)->GetObjectArrayElement(env, factors, i);
        factorstr[i] = (*env)->GetStringUTFChars(env, tmp, NULL);
    }
    //call method
    return tc_delete_description(factorstr, size, pathStr, descStr);

}

static const char* CHARS = "ABCDEF";
static const char* DIGITS = "0123456789";
char b2Char(unsigned char byte) {
    if(byte >= 10) {
        return CHARS[byte-10];
    } else {
        return DIGITS[byte];
    }
}
void byte2str(unsigned char *bytes, int len, char* strbuf) {
    unsigned char chhight, chlow;
    int pos = 0;
    for(int i=0; i<len; i++) {
        chhight = *(bytes+i);
        chlow = chhight&0x0F;
        chhight = (chhight&0xF0)>>4;
        strbuf[pos] = b2Char(chhight);
        pos++;
        strbuf[pos] = b2Char(chlow);
        pos++;
    }
}
#endif //JNI_INTERFACE

