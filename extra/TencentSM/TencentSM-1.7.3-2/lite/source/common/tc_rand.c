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

#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#ifdef OS_ANDROID
    #include <fcntl.h>
#else
    #include <sys/fcntl.h>
#endif
#include "../include/tc.h"

#ifdef _SM3_BAESD_CSPRNG
    #include <sys/socket.h>
    #include <sys/un.h>
#endif
#include "../include/tc_str.h"
#include "../include/tc_sm3.h"
#include "../include/tc_rand.h"

#ifdef __linux
#include <sys/time.h>
#endif

#define DEVRANDOM "/dev/urandom","/dev/random","/dev/srandom"
#define DEVRANDOM_EGD "/var/run/egd-pool","/dev/egd-pool","/etc/egd-pool","/etc/entropy"
#define DUMMY_SEED "paO8KQJ8VHyuVBNs8DYBSSxQ2y9h3mDH" /* at least MD_DIGEST_LENGTH */
#define _SM2_BIGNUM_BYTES 32
#define _DUMMY_SALT_INT 49826741

extern int tcsm_cs_rand_poll(rand_ctx_t* ctx);
extern int tcsm_cs_rand_query_egd_bytes(const char *path, unsigned char *buf, int bytes,rand_ctx_t* ctx);

void tcsm_cs_rand_init(rand_ctx_t* ctx)
{
  ctx->state_num = 0;
  ctx->state_index = 0;
  ctx->md_count[0] = 0;
  ctx->md_count[1] = 0;
  ctx->entropy = 0;
  ctx->initialized = 0;
}

#ifdef _SM3_BAESD_CSPRNG
void tcsm_cs_rand_add(const void *buf, int num, double add,rand_ctx_t* ctx)
{
    int i, j, k, st_idx;
    long md_c[2];
    unsigned char local_md[MD_DIGEST_LENGTH];
    sm3_ctx_t md_ctx;
    
    if (!num)
        return;
    
    st_idx = (int)ctx->state_index;
    
    md_c[0] = ctx->md_count[0];
    md_c[1] = ctx->md_count[1];
    
    memcpy(local_md, ctx->md, sizeof(ctx->md));
    
    /* state_index <= state_num <= STATE_SIZE */
    ctx->state_index += num;
    if (ctx->state_index >= STATE_SIZE) {
        ctx->state_index %= STATE_SIZE;
        ctx->state_num = STATE_SIZE;
    } else if (ctx->state_num < STATE_SIZE) {
        if (ctx->state_index > ctx->state_num)
            ctx->state_num = ctx->state_index;
    }
    /* state_index <= state_num <= STATE_SIZE */
    
    /*
     * state[st_idx], ..., state[(st_idx + num - 1) % STATE_SIZE] are what we
     * will use now, but other threads may use them as well
     */
    
    ctx->md_count[1] += (num / MD_DIGEST_LENGTH) + (num % MD_DIGEST_LENGTH > 0);
    
    tcsm_sm3_init_opt(&md_ctx);
    
    for (i = 0; i < num; i += MD_DIGEST_LENGTH) {
        j = (num - i);
        j = (j > MD_DIGEST_LENGTH) ? MD_DIGEST_LENGTH : j;
        
        tcsm_sm3_init_opt(&md_ctx);
        tcsm_sm3_update_opt(&md_ctx, local_md, MD_DIGEST_LENGTH);

        k = (st_idx + j) - STATE_SIZE;
        if (k > 0) {
            
            tcsm_sm3_update_opt(&md_ctx, &(ctx->state[st_idx]), j - k);
            tcsm_sm3_update_opt(&md_ctx, &(ctx->state[0]), k);

        } else
            tcsm_sm3_update_opt(&md_ctx, &(ctx->state[st_idx]), j);

        tcsm_sm3_update_opt(&md_ctx, buf, j);
        tcsm_sm3_update_opt(&md_ctx, (unsigned char *)&(md_c[0]), sizeof(md_c));
        tcsm_sm3_final_opt(&md_ctx, local_md);
        md_c[1]++;
        
        buf = (const char *)buf + j;
        
        for (k = 0; k < j; k++) {
            ctx->state[st_idx++] ^= local_md[k];
            if (st_idx >= STATE_SIZE)
                st_idx = 0;
        }
    }

    for (k = 0; k < (int)sizeof(ctx->md); k++) {
        ctx->md[k] ^= local_md[k];
    }
    if (ctx->entropy < ENTROPY_NEEDED) /* stop counting when we have enough */
        ctx->entropy += add;
}

int tcsm_cs_rand_bytes(unsigned char *buf, int num, int pseudo,rand_ctx_t* ctx)
{
    static volatile int stirred_pool = 0;
    int i, j, k;
    size_t num_ceil, st_idx, st_num;
    long md_c[2];
    unsigned char local_md[MD_DIGEST_LENGTH];
    sm3_ctx_t md_ctx;

    pid_t curr_pid = getpid();

    int do_stir_pool = 0;
    
    if (num <= 0)
        return 1;
    
    tcsm_sm3_init_opt(&md_ctx);
    /* round upwards to multiple of MD_DIGEST_LENGTH/2 */
    num_ceil =
    (1 + (num - 1) / (MD_DIGEST_LENGTH / 2)) * (MD_DIGEST_LENGTH / 2);
    
    if (!ctx->initialized) {
        tcsm_cs_rand_poll(ctx);
        ctx->initialized = (ctx->entropy >= ENTROPY_NEEDED);
    }
    
    if (!stirred_pool)
        do_stir_pool = 1;
    
    if (!ctx->initialized) {
        /*
         * If the PRNG state is not yet unpredictable, then seeing the PRNG
         * output may help attackers to determine the new state; thus we have
         * to decrease the entropy estimate. Once we've had enough initial
         * seeding we don't bother to adjust the entropy count, though,
         * because we're not ambitious to provide *information-theoretic*
         * randomness. NOTE: This approach fails if the program forks before
         * we have enough entropy. Entropy should be collected in a separate
         * input pool and be transferred to the output pool only when the
         * entropy limit has been reached.
         */
        ctx->entropy -= num;
        if (ctx->entropy < 0)
            ctx->entropy = 0;
    }
    
    if (do_stir_pool) {
        /*
         * In the output function only half of 'md' remains secret, so we
         * better make sure that the required entropy gets 'evenly
         * distributed' through 'state', our randomness pool. The input
         * function (ssleay_rand_add) chains all of 'md', which makes it more
         * suitable for this purpose.
         */
        
        int n = STATE_SIZE;     /* so that the complete pool gets accessed */
        while (n > 0) {
            /*
             * Note that the seed does not matter, it's just that
             * ssleay_rand_add expects to have something to hash.
             */
            tcsm_cs_rand_add(DUMMY_SEED, MD_DIGEST_LENGTH, 0.0,ctx);
            n -= MD_DIGEST_LENGTH;
        }
        if (ctx->initialized)
            stirred_pool = 1;
    }
    
    st_idx = ctx->state_index;
    st_num = ctx->state_num;
    md_c[0] = ctx->md_count[0];
    md_c[1] = ctx->md_count[1];
    memcpy(local_md, ctx->md, sizeof(ctx->md));
    
    ctx->state_index += num_ceil;
    if (ctx->state_index > ctx->state_num)
        ctx->state_index %= ctx->state_num;
    
    /*
     * state[st_idx], ..., state[(st_idx + num_ceil - 1) % st_num] are now
     * ours (but other threads may use them too)
     */
    
    ctx->md_count[0] += 1;
    
    /* before unlocking, we must clear 'crypto_lock_rand' */
    
    while (num > 0) {
        /* num_ceil -= MD_DIGEST_LENGTH/2 */
        j = (num >= MD_DIGEST_LENGTH / 2) ? MD_DIGEST_LENGTH / 2 : num;
        num -= j;
        
        tcsm_sm3_init_opt(&md_ctx);
        if (curr_pid) {         /* just in the first iteration to save time */
            tcsm_sm3_update_opt(&md_ctx, (unsigned char *)&curr_pid, sizeof(curr_pid));
            curr_pid = 0;
        }
        
        tcsm_sm3_update_opt(&md_ctx, local_md, MD_DIGEST_LENGTH);
        
        tcsm_sm3_update_opt(&md_ctx, (unsigned char *)&(md_c[0]), sizeof(md_c));
        tcsm_sm3_update_opt(&md_ctx, buf, j);

        
        k = (int)(st_idx + MD_DIGEST_LENGTH / 2) - (int)st_num;
        if (k > 0) {
            tcsm_sm3_update_opt(&md_ctx, &(ctx->state[st_idx]), MD_DIGEST_LENGTH / 2 - k);
            tcsm_sm3_update_opt(&md_ctx, &(ctx->state[0]), k);
        } else {
            tcsm_sm3_update_opt(&md_ctx, &(ctx->state[st_idx]), MD_DIGEST_LENGTH / 2);
        }
        
        tcsm_sm3_final_opt(&md_ctx, local_md);
        
        for (i = 0; i < MD_DIGEST_LENGTH / 2; i++) {
            /* may compete with other threads */
            ctx->state[st_idx++] ^= local_md[i];
            if (st_idx >= st_num)
                st_idx = 0;
            if (i < j)
                *(buf++) = local_md[i + MD_DIGEST_LENGTH / 2];
        }
    }
    tcsm_sm3_init_opt(&md_ctx);
    tcsm_sm3_update_opt(&md_ctx, (unsigned char *)&(md_c[0]), sizeof(md_c));
    tcsm_sm3_update_opt(&md_ctx, local_md, MD_DIGEST_LENGTH);
    tcsm_sm3_update_opt(&md_ctx, ctx->md, MD_DIGEST_LENGTH);
    tcsm_sm3_final_opt(&md_ctx, ctx->md);
    if (ctx->initialized)
        return (1);
    else if (pseudo)
        return 0;
    else {
        return (0);
    }
    
err:
    return (0);
}

int tcsm_cs_rand_poll(rand_ctx_t* ctx)
{
    unsigned long l;
    pid_t curr_pid = getpid();
    
#  if defined(DEVRANDOM) || defined(DEVRANDOM_EGD)
    unsigned char tmpbuf[ENTROPY_NEEDED];
    int n = 0;
#  endif
    
#  ifdef DEVRANDOM
    static const char *randomfiles[] = { DEVRANDOM };
    struct stat randomstats[sizeof(randomfiles) / sizeof(randomfiles[0])];
    int fd;
    unsigned int i;
#  endif
    
#  ifdef DEVRANDOM_EGD
    static const char *egdsockets[] = { DEVRANDOM_EGD,NULL };
    const char **egdsocket = NULL;
#  endif
    
#  ifdef DEVRANDOM
    memset(randomstats, 0, sizeof(randomstats));
    /*
     * Use a random entropy pool device. Linux, FreeBSD and OpenBSD have
     * this. Use /dev/urandom if you can as /dev/random may block if it runs
     * out of random entries.
     */
    
    for (i = 0; (i < sizeof(randomfiles) / sizeof(randomfiles[0])) &&
         (n < ENTROPY_NEEDED); i++) {
        if ((fd = open(randomfiles[i], O_RDONLY
#   ifdef O_NONBLOCK
                       | O_NONBLOCK
#   endif
#   ifdef O_BINARY
                       | O_BINARY
#   endif
#   ifdef O_NOCTTY              /* If it happens to be a TTY (god forbid), do
* not make it our controlling tty */
                       | O_NOCTTY
#   endif
                       )) >= 0) {
            int usec = 10 * 1000; /* spend 10ms on each file */
            int r;
            unsigned int j;
            struct stat *st = &randomstats[i];
            
            /*
             * Avoid using same input... Used to be O_NOFOLLOW above, but
             * it's not universally appropriate...
             */
            if (fstat(fd, st) != 0) {
                close(fd);
                continue;
            }
            for (j = 0; j < i; j++) {
                if (randomstats[j].st_ino == st->st_ino &&
                    randomstats[j].st_dev == st->st_dev)
                    break;
            }
            if (j < i) {
                close(fd);
                continue;
            }
            
            do {
                int try_read = 0;
                
                /* use select() */
                fd_set fset;
                struct timeval t;
                
                t.tv_sec = 0;
                t.tv_usec = usec;
                
                if (FD_SETSIZE > 0 && (unsigned)fd >= FD_SETSIZE) {
                    /*
                     * can't use select, so just try to read once anyway
                     */
                    try_read = 1;
                } else {
                    FD_ZERO(&fset);
                    FD_SET(fd, &fset);
                    
                    if (select(fd + 1, &fset, NULL, NULL, &t) >= 0) {
                        usec = t.tv_usec;
                        if (FD_ISSET(fd, &fset))
                            try_read = 1;
                    } else
                        usec = 0;
                }
                
                if (try_read) {
                    r = (int)read(fd, (unsigned char *)tmpbuf + n,
                                  ENTROPY_NEEDED - n);
                    if (r > 0)
                        n += r;
                } else
                    r = -1;
                
                /*
                 * Some Unixen will update t in select(), some won't.  For
                 * those who won't, or if we didn't use select() in the first
                 * place, give up here, otherwise, we will do this once again
                 * for the remaining time.
                 */
                if (usec == 10 * 1000)
                    usec = 0;
            }
            while ((r > 0 ||
                    (errno == EINTR || errno == EAGAIN)) && usec != 0
                   && n < ENTROPY_NEEDED);
            
            close(fd);
        }
    }
#  endif                        /* defined(DEVRANDOM) */
    
#  ifdef DEVRANDOM_EGD
    /*
     * Use an EGD socket to read entropy from an EGD or PRNGD entropy
     * collecting daemon.
     */
    
    for (egdsocket = egdsockets; *egdsocket && n < ENTROPY_NEEDED;
         egdsocket++) {
        int r;
        
        r = tcsm_cs_rand_query_egd_bytes(*egdsocket, (unsigned char *)tmpbuf + n,
                                    ENTROPY_NEEDED - n,ctx);
        if (r > 0)
            n += r;
    }
#  endif                        /* defined(DEVRANDOM_EGD) */
    
#  if defined(DEVRANDOM) || defined(DEVRANDOM_EGD)
    if (n > 0) {
        tcsm_cs_rand_add(tmpbuf, sizeof(tmpbuf), (double)n,ctx);
        memset(tmpbuf, 0, n);
    }
#  endif
    
    /* put in some default random data, we need more than just this */
    l = curr_pid;
    tcsm_cs_rand_add(&l, sizeof(l), 0.0,ctx);
    l = getuid();
    tcsm_cs_rand_add(&l, sizeof(l), 0.0,ctx);
    l = time(NULL);
    tcsm_cs_rand_add(&l, sizeof(l), 0.0,ctx);
    
#  if defined(DEVRANDOM) || defined(DEVRANDOM_EGD)
    return 1;
#  else
    return 0;
#  endif
}

int tcsm_cs_rand_query_egd_bytes(const char *path, unsigned char *buf, int bytes,rand_ctx_t* ctx)
{
    int ret = 0;
    struct sockaddr_un addr;
    int len, num, numbytes;
    int fd = -1;
    int success;
    unsigned char egdbuf[2], tempbuf[255], *retrievebuf;
    
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    if (strlen(path) >= sizeof(addr.sun_path))
        return (-1);
    tcsm_buf_strlcpy(addr.sun_path, path, sizeof(addr.sun_path));
    len = (int)offsetof(struct sockaddr_un, sun_path) + (int)strlen(path);
    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1)
        return (-1);
    success = 0;
    while (!success) {
        if (connect(fd, (struct sockaddr *)&addr, len) == 0)
            success = 1;
        else {
            switch (errno) {
# ifdef EINTR
                case EINTR:
# endif
# ifdef EAGAIN
                case EAGAIN:
# endif
# ifdef EINPROGRESS
                case EINPROGRESS:
# endif
# ifdef EALREADY
                case EALREADY:
# endif
                    /* No error, try again */
                    break;
# ifdef EISCONN
                case EISCONN:
                    success = 1;
                    break;
# endif
                default:
                    goto err;       /* failure */
            }
        }
    }
    
    while (bytes > 0) {
        egdbuf[0] = 1;
        egdbuf[1] = bytes < 255 ? bytes : 255;
        numbytes = 0;
        while (numbytes != 2) {
            num = (int)write(fd, egdbuf + numbytes, 2 - numbytes);
            if (num >= 0)
                numbytes += num;
            else {
                switch (errno) {
# ifdef EINTR
                    case EINTR:
# endif
# ifdef EAGAIN
                    case EAGAIN:
# endif
                        /* No error, try again */
                        break;
                    default:
                        ret = -1;
                        goto err;   /* failure */
                }
            }
        }
        numbytes = 0;
        while (numbytes != 1) {
            num = (int)read(fd, egdbuf, 1);
            if (num == 0)
                goto err;       /* descriptor closed */
            else if (num > 0)
                numbytes += num;
            else {
                switch (errno) {
# ifdef EINTR
                    case EINTR:
# endif
# ifdef EAGAIN
                    case EAGAIN:
# endif
                        /* No error, try again */
                        break;
                    default:
                        ret = -1;
                        goto err;   /* failure */
                }
            }
        }
        if (egdbuf[0] == 0)
            goto err;
        if (buf)
            retrievebuf = buf + ret;
        else
            retrievebuf = tempbuf;
        numbytes = 0;
        while (numbytes != egdbuf[0]) {
            num = (int)read(fd, retrievebuf + numbytes, egdbuf[0] - numbytes);
            if (num == 0)
                goto err;       /* descriptor closed */
            else if (num > 0)
                numbytes += num;
            else {
                switch (errno) {
# ifdef EINTR
                    case EINTR:
# endif
# ifdef EAGAIN
                    case EAGAIN:
# endif
                        /* No error, try again */
                        break;
                    default:
                        ret = -1;
                        goto err;   /* failure */
                }
            }
        }
        ret += egdbuf[0];
        bytes -= egdbuf[0];
        if (!buf)
            tcsm_cs_rand_add(tempbuf, egdbuf[0], egdbuf[0],ctx);
    }
err:
    if (fd != -1)
        close(fd);
    return (ret);
}
#endif


void* tcsm_tc_rand_init(void)
{
#ifdef _SM3_BAESD_CSPRNG
  void* ctx = tcsm_tc_secure_malloc(sizeof(rand_ctx_t));
  tcsm_cs_rand_init(ctx);
#else
  void* ctx = tcsm_tc_secure_malloc(sizeof(gmp_randstate_t));
  gmp_randstate_t* r_ctx = (gmp_randstate_t*)ctx;
  gmp_randinit_default(*r_ctx);
  unsigned char digest[32] = {0};
  
  unsigned long seed = 0;
  unsigned char entropy[32] = {0};
  int fd = open("/dev/urandom", 0);
  if (fd < 0 || read(fd, entropy, 32) < 0) {
    static int salt = _DUMMY_SALT_INT;
    time_t t;
    seed = time(&t) + (salt++);
  }
  if (fd >= 0) close(fd);
  
  sm3_ctx_t md_ctx;
  tcsm_sm3_init_opt(&md_ctx);
  tcsm_sm3_update_opt(&md_ctx, (const unsigned char*)&seed, sizeof(seed));
  tcsm_sm3_update_opt(&md_ctx, (const unsigned char*)entropy, 32);
  tcsm_sm3_update_opt(&md_ctx, (const unsigned char*)DUMMY_SEED, 32);
  tcsm_sm3_final_opt(&md_ctx, digest);
  
  tc_bn_t bn_seed;
  tcsm_tc_bn_init(bn_seed);
  tcsm_tc_bn_set_bin(bn_seed, (char*)digest, 32);
  
  gmp_randseed(*r_ctx,bn_seed->val);
  tcsm_tc_bn_clear(bn_seed);
#endif
  return ctx;
}

void tcsm_tc_rand_bignum(void* ctx,tc_bn_t op, tc_bn_t n)
{
#ifdef _SM3_BAESD_CSPRNG
  unsigned char *buf = tcsm_tc_secure_malloc(_SM2_BIGNUM_BYTES);
  tcsm_cs_rand_bytes(buf, (int)_SM2_BIGNUM_BYTES, 0, ctx);
  tcsm_tc_bn_set_bin(op, (char*)buf, (int)_SM2_BIGNUM_BYTES);
  tcsm_tc_secure_free(buf);
  tcsm_tc_bn_mod(op, op, n);
#else
  gmp_randstate_t* r_ctx = (gmp_randstate_t*)ctx;
  return mpz_urandomm(op->val, *r_ctx, n->val);
#endif
}

void tcsm_tc_rand_bytes(void* ctx,unsigned char *buf, int num)
{
#ifdef _SM3_BAESD_CSPRNG
  tcsm_cs_rand_bytes(buf, num, 0, ctx);
#else
  gmp_randstate_t* r_ctx = (gmp_randstate_t*)ctx;
  tc_bn_t op; tcsm_tc_bn_init(op);
  mpz_urandomb(op->val, *r_ctx, 8*num);
  unsigned int len = 0;
  tcsm_tc_bn_get_bin(buf, &len, op, num);
  tcsm_tc_bn_clear(op);
#endif
}

void tcsm_tc_rand_clear(void* ctx)
{
#ifdef _SM3_BAESD_CSPRNG
  tcsm_cs_rand_init(ctx);
#else
  gmp_randstate_t* r_ctx = (gmp_randstate_t*)ctx;
  gmp_randclear(*r_ctx);
#endif
}
