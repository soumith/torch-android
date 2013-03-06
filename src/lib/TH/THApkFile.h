#ifndef TH_APK_FILE_INC
#define TH_APK_FILE_INC

#ifdef __ANDROID__
#include "THFile.h"

THFile *THApkFile_new(const char *name, const char *mode, int isQuiet);
THFile *THPipeFile_new(const char *name, const char *mode, int isQuiet);

const char *THApkFile_name(THFile *self);

int THApkFile_isLittleEndianCPU(void);
int THApkFile_isBigEndianCPU(void);
void THApkFile_nativeEndianEncoding(THFile *self);
void THApkFile_littleEndianEncoding(THFile *self);
void THApkFile_bigEndianEncoding(THFile *self);

// must be set by someone on the android side
// AAssetManager * but kept void* to avoid dependency issues
void THApkFile_setAAssetManager(void *am);

#endif
#endif
