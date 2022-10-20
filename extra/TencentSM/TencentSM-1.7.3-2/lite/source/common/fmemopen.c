/**
 * 参考了https://github.com/jalola/android_fmemopen/blob/master/fmemopen.c
 https://github.com/libconfuse/libconfuse/blob/master/src/fmemopen.c
 */
#ifdef __ANDROID__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

typedef struct tFileMem {
  size_t pos;
  size_t size;
  char *buffer;
} T_tFileMem;

static int readfn(void *handler, char *buf, int size) {
    T_tFileMem *mem = handler;
    size_t available = mem->size - mem->pos;
  
    if (available < 0) {
        available = 0;
    }
    if (size > available) {
        size = available;
    }
    memcpy(buf, mem->buffer + mem->pos, sizeof(char) * size);
    mem->pos += size;

    return size;
}

static int writefn(void *handler, const char *buf, int size) {
    T_tFileMem *mem = handler;
    size_t available = mem->size - mem->pos;

    if (available < 0) {
        available = 0;
    }
    if (size > available) {
        size = available;
    }
    memcpy(mem->buffer + mem->pos, buf, sizeof(char) * size);
    mem->pos += size;

    return size;
}

static fpos_t seekfn(void *handler, fpos_t offset, int whence) {
  size_t pos;
  T_tFileMem *mem = handler;

  switch (whence) {
    case SEEK_SET: {
      if (offset >= 0) {
        pos = (size_t)offset;
      } else {
        pos = 0;
      }
      break;
    }
    case SEEK_CUR: {
      if (offset >= 0 || (size_t)(-offset) <= mem->pos) {
        pos = mem->pos + (size_t)offset;
      } else {
        pos = 0;
      }
      break;
    }
    case SEEK_END: 
        pos = mem->size + (size_t)offset; 
        break;
    default: return -1;
  }

  if (pos > mem->size || pos < 0) {
    return -1;
  }

  mem->pos = pos;
  return (fpos_t)pos;
}

static int closefn(void *handler) {
  T_tFileMem *mem = handler;
//   free(mem->buffer); //这个是外部传入的，应该由外部管理
  free(handler);
  return 0;
}

FILE *fmemopen(void *buf, size_t size, const char *mode) {
    // This data is released on fclose.
    T_tFileMem* pstFileMem = (T_tFileMem *) malloc(sizeof(T_tFileMem));

    if(!pstFileMem) {
        return NULL;
    }
    // Zero-out the structure.
    pstFileMem->pos = 0;
    pstFileMem->size = size;
    pstFileMem->buffer = buf;

    return funopen(pstFileMem, readfn, writefn, seekfn, closefn);
}

#elif defined _WIN32
#include <stdio.h>
#include <windows.h>
#include <share.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>

FILE *fmemopen(void *buf, size_t len, const char *type)
{
	int fd;
	FILE *fp;
	char tp[MAX_PATH - 13];
	char fn[MAX_PATH + 1];
	int * pfd = &fd;
	int retner = -1;
	char tfname[] = "MemTF_";
    // fprintf(stdout, "fmemopen in windows");
	if (!GetTempPathA(sizeof(tp), tp))
		return NULL;
	if (!GetTempFileNameA(tp, tfname, 0, fn))
		return NULL;
	retner = _sopen_s(pfd, fn, _O_CREAT | _O_SHORT_LIVED | _O_TEMPORARY | _O_RDWR | _O_BINARY | _O_NOINHERIT, _SH_DENYRW, _S_IREAD | _S_IWRITE);
	if (retner != 0)
		return NULL;
	if (fd == -1)
		return NULL;
	fp = _fdopen(fd, "wb+");
	if (!fp) {
		_close(fd);
		return NULL;
	}
	/*File descriptors passed into _fdopen are owned by the returned FILE * stream.If _fdopen is successful, do not call _close on the file descriptor.Calling fclose on the returned FILE * also closes the file descriptor.*/
	fwrite(buf, len, 1, fp);
	rewind(fp);
	return fp;
}

#endif