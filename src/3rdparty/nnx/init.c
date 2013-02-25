#include "TH.h"
#include "luaT.h"

#ifdef _OPENMP
#include "omp.h"
#endif

#define torch_(NAME) TH_CONCAT_3(torch_, Real, NAME)
#define torch_Tensor TH_CONCAT_STRING_3(torch., Real, Tensor)
#define nn_(NAME) TH_CONCAT_3(nn_, Real, NAME)

#include "generic/SpatialReSamplingEx.c"
#include "THGenerateFloatTypes.h"

#include "generic/SpatialLinear.c"
#include "THGenerateFloatTypes.h"

#include "generic/SpatialUpSampling.c"
#include "THGenerateFloatTypes.h"

#include "generic/SpatialDownSampling.c"
#include "THGenerateFloatTypes.h"

#include "generic/SpatialReSampling.c"
#include "THGenerateFloatTypes.h"

#include "generic/SpatialMaxSampling.c"
#include "THGenerateFloatTypes.h"

#include "generic/DistMarginCriterion.c"
#include "THGenerateFloatTypes.h"

#include "generic/SpatialGraph.c"
#include "THGenerateFloatTypes.h"

#include "generic/SpatialMatching.c"
#include "THGenerateFloatTypes.h"

#include "generic/SpatialRadialMatching.c"
#include "THGenerateFloatTypes.h"

#include "generic/DataSetLabelMe.c"
#include "THGenerateFloatTypes.h"

DLL_EXPORT int luaopen_libnnx(lua_State *L)
{
  nn_FloatSpatialLinear_init(L);
  nn_FloatSpatialReSamplingEx_init(L);
  nn_FloatSpatialUpSampling_init(L);
  nn_FloatSpatialDownSampling_init(L);
  nn_FloatSpatialReSampling_init(L);
  nn_FloatSpatialMaxSampling_init(L);
  nn_FloatDistMarginCriterion_init(L);
  nn_FloatSpatialGraph_init(L);
  nn_FloatSpatialMatching_init(L);
  nn_FloatSpatialRadialMatching_init(L);
  nn_FloatDataSetLabelMe_init(L);

  nn_DoubleSpatialLinear_init(L);
  nn_DoubleSpatialReSamplingEx_init(L);
  nn_DoubleSpatialUpSampling_init(L);
  nn_DoubleSpatialDownSampling_init(L);
  nn_DoubleSpatialReSampling_init(L);
  nn_DoubleSpatialMaxSampling_init(L);
  nn_DoubleDistMarginCriterion_init(L);
  nn_DoubleSpatialGraph_init(L);
  nn_DoubleSpatialMatching_init(L);
  nn_DoubleSpatialRadialMatching_init(L);
  nn_DoubleDataSetLabelMe_init(L);

  return 1;
}
