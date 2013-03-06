#ifdef __ANDROID__
#include "THGeneral.h"
#include "THApkFile.h"
#include "THFilePrivate.h"

#include <stdio.h>
#include <errno.h>
#include <android/asset_manager.h>

static int apk_read(void* cookie, char* buf, int size) {
  return AAsset_read((AAsset*)cookie, buf, size);
}

static int apk_write(void* cookie, const char* buf, int size) {
  return EACCES; // can't provide write access to the apk
}

static fpos_t apk_seek(void* cookie, fpos_t offset, int whence) {
  return AAsset_seek((AAsset*)cookie, offset, whence);
}

static int apk_close(void* cookie) {
  AAsset_close((AAsset*)cookie);
  return 0;
}

void *THApkFile_AAssetManager = NULL;
void THApkFile_setAAssetManager(void *am) {
  THApkFile_AAssetManager = am;
}
				

FILE* apk_fopen(const char* fname, const char* mode) {
  if(mode[0] == 'w' || THApkFile_AAssetManager == NULL ) return NULL;
  AAsset* asset = AAssetManager_open((AAssetManager*)THApkFile_AAssetManager,
				     fname, 0);
  if(!asset) return NULL;
  return funopen(asset, apk_read, apk_write, apk_seek, apk_close);
}

typedef struct THApkFile__
{
    THFile file;
  
    FILE *handle;
    char *name;
    int isNativeEncoding;

} THApkFile;

static int THApkFile_isOpened(THFile *self)
{
  THApkFile *dfself = (THApkFile*)self;
  return (dfself->handle != NULL);
}

const char *THApkFile_name(THFile *self)
{
  THApkFile *dfself = (THApkFile*)self;
  return dfself->name;
}

#define READ_WRITE_METHODS(TYPE, TYPEC, ASCII_READ_ELEM, ASCII_WRITE_ELEM) \
  static long THApkFile_read##TYPEC(THFile *self, TYPE *data, long n)  \
  {                                                                     \
    THApkFile *dfself = (THApkFile*)(self);                           \
    long nread = 0L;                                                    \
                                                                        \
    THArgCheck(dfself->handle != NULL, 1, "attempt to use a closed file"); \
    THArgCheck(dfself->file.isReadable, 1, "attempt to read in a write-only file"); \
                                                                        \
    if(dfself->file.isBinary)                                           \
    {                                                                   \
      nread = fread(data, sizeof(TYPE), n, dfself->handle);           \
      if(!dfself->isNativeEncoding && (sizeof(TYPE) > 1) && (nread > 0)) \
        THApkFile_reverseMemory(data, data, sizeof(TYPE), nread);      \
    }                                                                   \
    else                                                                \
    {                                                                   \
      long i;                                                           \
      for(i = 0; i < n; i++)                                            \
      {                                                                 \
        ASCII_READ_ELEM; /* increment here result and break if wrong */ \
      }                                                                 \
      if(dfself->file.isAutoSpacing && (n > 0))                         \
      {                                                                 \
        int c = fgetc(dfself->handle);                                  \
        if( (c != '\n') && (c != EOF) )                                 \
          ungetc(c, dfself->handle);                                    \
      }                                                                 \
    }                                                                   \
                                                                        \
    if(nread != n)                                                      \
    {                                                                   \
      dfself->file.hasError = 1; /* shouldn't we put hasError to 0 all the time ? */ \
      if(!dfself->file.isQuiet)                                         \
        THError("read error: read %d blocks instead of %d", nread, n);  \
    }                                                                   \
                                                                        \
    return nread;                                                       \
  }                                                                     \
                                                                        \
  static long THApkFile_write##TYPEC(THFile *self, TYPE *data, long n) \
  {                                                                     \
    THApkFile *dfself = (THApkFile*)(self);                           \
    long nwrite = 0L;                                                   \
                                                                        \
    THArgCheck(dfself->handle != NULL, 1, "attempt to use a closed file"); \
    THArgCheck(dfself->file.isWritable, 1, "attempt to write in a read-only file"); \
                                                                        \
    if(dfself->file.isBinary)                                           \
    {                                                                   \
      if(dfself->isNativeEncoding)                                      \
      {                                                                 \
        nwrite = fwrite(data, sizeof(TYPE), n, dfself->handle);         \
      }                                                                 \
      else                                                              \
      {                                                                 \
        if(sizeof(TYPE) > 1)                                            \
        {                                                               \
          char *buffer = THAlloc(sizeof(TYPE)*n);                       \
          THApkFile_reverseMemory(buffer, data, sizeof(TYPE), n);      \
          nwrite = fwrite(buffer, sizeof(TYPE), n, dfself->handle);     \
          THFree(buffer);                                               \
        }                                                               \
        else                                                            \
          nwrite = fwrite(data, sizeof(TYPE), n, dfself->handle);       \
      }                                                                 \
    }                                                                   \
    else                                                                \
    {                                                                   \
      long i;                                                           \
      for(i = 0; i < n; i++)                                            \
      {                                                                 \
        ASCII_WRITE_ELEM;                                               \
        if( dfself->file.isAutoSpacing && (i < n-1) )                   \
          fprintf(dfself->handle, " ");                                 \
      }                                                                 \
      if(dfself->file.isAutoSpacing && (n > 0))                         \
        fprintf(dfself->handle, "\n");                                  \
    }                                                                   \
                                                                        \
    if(nwrite != n)                                                     \
    {                                                                   \
      dfself->file.hasError = 1;                                        \
      if(!dfself->file.isQuiet)                                         \
        THError("write error: wrote %d blocks instead of %d", nwrite, n); \
    }                                                                   \
                                                                        \
    return nwrite;                                                      \
}

static void THApkFile_synchronize(THFile *self)
{
  THApkFile *dfself = (THApkFile*)(self);
  THArgCheck(dfself->handle != NULL, 1, "attempt to use a closed file");
  fflush(dfself->handle);
}

static void THApkFile_seek(THFile *self, long position)
{
  THApkFile *dfself = (THApkFile*)(self);

  THArgCheck(dfself->handle != NULL, 1, "attempt to use a closed file");
  THArgCheck(position >= 0, 2, "position must be positive");

  if(fseek(dfself->handle, position, SEEK_SET) < 0)
  {
    dfself->file.hasError = 1;
    if(!dfself->file.isQuiet)
      THError("unable to seek at position %d", position);
  }
}

static void THApkFile_seekEnd(THFile *self)
{
  THApkFile *dfself = (THApkFile*)(self);

  THArgCheck(dfself->handle != NULL, 1, "attempt to use a closed file");

  if(fseek(dfself->handle, 0L, SEEK_END) < 0)
  {
    dfself->file.hasError = 1;
    if(!dfself->file.isQuiet)
      THError("unable to seek at end of file");
  }
}

static long THApkFile_position(THFile *self)
{
  THApkFile *dfself = (THApkFile*)(self);
  THArgCheck(dfself->handle != NULL, 1, "attempt to use a closed file");
  return ftell(dfself->handle);
}

static void THApkFile_close(THFile *self)
{
  THApkFile *dfself = (THApkFile*)(self);
  THArgCheck(dfself->handle != NULL, 1, "attempt to use a closed file");
  fclose(dfself->handle);
  dfself->handle = NULL;
}

/* Little and Big Endian */

static void THApkFile_reverseMemory(void *dst, const void *src, long blockSize, long numBlocks)
{
  if(blockSize != 1)
  {
    long halfBlockSize = blockSize/2;
    char *charSrc = (char*)src;
    char *charDst = (char*)dst;
    long b, i;
    for(b = 0; b < numBlocks; b++)
    {
      for(i = 0; i < halfBlockSize; i++)
      {
        char z = charSrc[i];
        charDst[i] = charSrc[blockSize-1-i];
        charDst[blockSize-1-i] = z;
      }
      charSrc += blockSize;
      charDst += blockSize;
    }
  }
}

int THApkFile_isLittleEndianCPU(void)
{
  int x = 7;
  char *ptr = (char *)&x;

  if(ptr[0] == 0)
    return 0;
  else
    return 1;
}

int THApkFile_isBigEndianCPU(void)
{
  return(!THApkFile_isLittleEndianCPU());
}

void THApkFile_nativeEndianEncoding(THFile *self)
{
  THApkFile *dfself = (THApkFile*)(self);
  THArgCheck(dfself->handle != NULL, 1, "attempt to use a closed file");
  dfself->isNativeEncoding = 1;
}

void THApkFile_littleEndianEncoding(THFile *self)
{
  THApkFile *dfself = (THApkFile*)(self);
  THArgCheck(dfself->handle != NULL, 1, "attempt to use a closed file");
  dfself->isNativeEncoding = THApkFile_isLittleEndianCPU();
}

void THApkFile_bigEndianEncoding(THFile *self)
{
  THApkFile *dfself = (THApkFile*)(self);
  THArgCheck(dfself->handle != NULL, 1, "attempt to use a closed file");
  dfself->isNativeEncoding = !THApkFile_isLittleEndianCPU();
}

/* End of Little and Big Endian Stuff */

static void THApkFile_free(THFile *self)
{
  THApkFile *dfself = (THApkFile*)(self);
  if(dfself->handle)
    fclose(dfself->handle);
  THFree(dfself->name);
  THFree(dfself);
}

/* READ_WRITE_METHODS(int, Bool, */
/*                    int value = 0; int ret = fscanf(file->handle, "%d", &value); array[i] = (value ? 1 : 0); if(ret <= 0) break; else result++, */
/*                    int value = (array[i] ? 1 : 0); nElemWritten = fprintf(file->handle, "%d", value), */
/*                    true) */

/* Note that we do a trick */
READ_WRITE_METHODS(unsigned char, Byte,
                   nread = fread(data, 1, n, dfself->handle); break,
                   nwrite = fwrite(data, 1, n, dfself->handle); break)

READ_WRITE_METHODS(char, Char,
                   nread = fread(data, 1, n, dfself->handle); break,
                   nwrite = fwrite(data, 1, n, dfself->handle); break)

READ_WRITE_METHODS(short, Short,
                   int ret = fscanf(dfself->handle, "%hd", &data[i]); if(ret <= 0) break; else nread++,
                   int ret = fprintf(dfself->handle, "%hd", data[i]); if(ret <= 0) break; else nwrite++)

READ_WRITE_METHODS(int, Int,
                   int ret = fscanf(dfself->handle, "%d", &data[i]); if(ret <= 0) break; else nread++,
                   int ret = fprintf(dfself->handle, "%d", data[i]); if(ret <= 0) break; else nwrite++)

READ_WRITE_METHODS(long, Long,
                   int ret = fscanf(dfself->handle, "%ld", &data[i]); if(ret <= 0) break; else nread++,
                   int ret = fprintf(dfself->handle, "%ld", data[i]); if(ret <= 0) break; else nwrite++)

READ_WRITE_METHODS(float, Float,
                   int ret = fscanf(dfself->handle, "%g", &data[i]); if(ret <= 0) break; else nread++,
                   int ret = fprintf(dfself->handle, "%g", data[i]); if(ret <= 0) break; else nwrite++)

READ_WRITE_METHODS(double, Double,
                   int ret = fscanf(dfself->handle, "%lg", &data[i]); if(ret <= 0) break; else nread++,
                   int ret = fprintf(dfself->handle, "%lg", data[i]); if(ret <= 0) break; else nwrite++)

static long THApkFile_readString(THFile *self, const char *format, char **str_)
{
  THApkFile *dfself = (THApkFile*)(self);
  THArgCheck(dfself->handle != NULL, 1, "attempt to use a closed file");
  THArgCheck(dfself->file.isReadable, 1, "attempt to read in a write-only file");
  THArgCheck((strlen(format) >= 2 ? (format[0] == '*') && (format[1] == 'a' || format[1] == 'l') : 0), 2, "format must be '*a' or '*l'");

/* note: the string won't survive long, as it is copied into lua */
/* so 1024 is not that big... */
#define TBRS_BSZ 1024L

  if(format[1] == 'a')
  {
    char *p = THAlloc(TBRS_BSZ);
    long total = TBRS_BSZ;
    long pos = 0L;
    
    for (;;)
    {
      if(total-pos == 0) /* we need more space! */
      {
        total += TBRS_BSZ;
        p = THRealloc(p, total);
      }
      pos += fread(p+pos, 1, total-pos, dfself->handle);
      if (pos < total) /* eof? */
      {
        if(pos == 0L)
        {
          THFree(p);
          dfself->file.hasError = 1;
          if(!dfself->file.isQuiet)
            THError("read error: read 0 blocks instead of 1");

          *str_ = NULL;
          return 0;
        }
        *str_ = p;
        return pos;
      }
    }    
  }
  else
  {
    char *p = THAlloc(TBRS_BSZ);
    long total = TBRS_BSZ;
    long pos = 0L;
    long size;

    for (;;)
    {
      if(total-pos <= 1) /* we can only write '\0' in there! */
      {
        total += TBRS_BSZ;
        p = THRealloc(p, total);
      }
      if (fgets(p+pos, total-pos, dfself->handle) == NULL) /* eof? */
      {
        if(pos == 0L)
        {
          THFree(p);
          dfself->file.hasError = 1;
          if(!dfself->file.isQuiet)
            THError("read error: read 0 blocks instead of 1");

          *str_ = NULL;
          return 0;
        }
        *str_ = p;
        return pos;
      }
      size = strlen(p+pos);
      if (size == 0L || (p+pos)[size-1] != '\n')
      {
        pos += size;
      }
      else
      {
        pos += size-1L; /* do not include `eol' */
        *str_ = p;
        return pos;
      }
    }
  }

  *str_ = NULL;
  return 0;
}


static long THApkFile_writeString(THFile *self, const char *str, long size)
{
  THApkFile *dfself = (THApkFile*)(self);
  long nwrite;

  THArgCheck(dfself->handle != NULL, 1, "attempt to use a closed file");
  THArgCheck(dfself->file.isWritable, 1, "attempt to write in a read-only file");

  nwrite = fwrite(str, 1, size, dfself->handle);
  if(nwrite != size)
  {
    dfself->file.hasError = 1;
    if(!dfself->file.isQuiet)
      THError("write error: wrote %ld blocks instead of %ld", nwrite, size);
  }

  return nwrite;
}

THFile *THApkFile_new(const char *name, const char *mode, int isQuiet)
{
  static struct THFileVTable vtable = {
    THApkFile_isOpened,

    THApkFile_readByte,
    THApkFile_readChar,
    THApkFile_readShort,
    THApkFile_readInt,
    THApkFile_readLong,
    THApkFile_readFloat,
    THApkFile_readDouble,
    THApkFile_readString,

    THApkFile_writeByte,
    THApkFile_writeChar,
    THApkFile_writeShort,
    THApkFile_writeInt,
    THApkFile_writeLong,
    THApkFile_writeFloat,
    THApkFile_writeDouble,
    THApkFile_writeString,

    THApkFile_synchronize,
    THApkFile_seek,
    THApkFile_seekEnd,
    THApkFile_position,
    THApkFile_close,
    THApkFile_free
  };

  int isReadable = 1;
  int isWritable = 0;
  FILE *handle;
  THApkFile *self;
  handle = apk_fopen(name, "rb");
  if(!handle)
  {
    if(isQuiet)
      return 0;
    else
      THError("cannot open <%s> from apk", name);
  }

  self = THAlloc(sizeof(THApkFile));

  self->handle = handle;
  self->name = THAlloc(strlen(name)+1);
  strcpy(self->name, name);
  self->isNativeEncoding = 1;

  self->file.vtable = &vtable;
  self->file.isQuiet = isQuiet;
  self->file.isReadable = isReadable;
  self->file.isWritable = isWritable;
  self->file.isBinary = 1;
  self->file.isAutoSpacing = 1;
  self->file.hasError = 0;

  return (THFile*)self;
}


#endif //__ANDROID__