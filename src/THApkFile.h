#ifndef TH_APK_FILE_INC
#define TH_APK_FILE_INC

#include "TH/THFile.h"

TH_API THFile *THApkFile_new(const char *name, const char *mode, int isQuiet);
TH_API THFile *THPipeFile_new(const char *name, const char *mode, int isQuiet);

TH_API const char *THApkFile_name(THFile *self);

TH_API int THApkFile_isLittleEndianCPU(void);
TH_API int THApkFile_isBigEndianCPU(void);
TH_API void THApkFile_nativeEndianEncoding(THFile *self);
TH_API void THApkFile_littleEndianEncoding(THFile *self);
TH_API void THApkFile_bigEndianEncoding(THFile *self);
TH_API void THApkFile_longSize(THFile *self, int size);

// must be set by someone on the android side
// AAssetManager * but kept void* to avoid dependency issues
TH_API void THApkFile_setAAssetManager(void *am);

#endif
