#ifndef TH_GENERIC_FILE
#define TH_GENERIC_FILE "generic/SpatialSparseCriterion.c"
#else

static int nn_(SpatialSparseCriterion_updateOutput)(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_(Tensor_id));  
  int sizeAverage = luaT_getfieldcheckboolean(L, 1, "sizeAverage");
  THTensor *fullOutput = luaT_getfieldcheckudata(L, 1, "fullOutput", torch_(Tensor_id));

  int k,x,y;
  for (y=0; y<input->size[1]; y++) {
    for (x=0; x<input->size[2]; x++) {
      real sum = 0;
      for (k=0; k<input->size[0]; k++) { 
        sum += fabs(THTensor_(get3d)(input, k, y, x));
      }
      if (sizeAverage) sum /= input->size[0];
      THTensor_(set2d)(fullOutput, y, x, sum);
    }
  }

  return 0;
}

static int nn_(SpatialSparseCriterion_updateGradInput)(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_(Tensor_id));
  THTensor *gradInput = luaT_checkudata(L, 3, torch_(Tensor_id));
  int sizeAverage = luaT_getfieldcheckboolean(L, 1, "sizeAverage");

  real norm = (sizeAverage ? 1./((real)THTensor_(nElement)(input)) : 1.);

  int k,x,y;
  for (y=0; y<input->size[1]; y++) {
    for (x=0; x<input->size[2]; x++) {
      for (k=0; k<input->size[0]; k++) {
        real input_data = THTensor_(get3d)(input, k, y, x);
        THTensor_(set3d)(gradInput, k, y, x, input_data >= 0 ? norm : -norm);
      }
    }
  }

  return 0;
}

static const struct luaL_Reg nn_(SpatialSparseCriterion__) [] = {
  {"SpatialSparseCriterion_updateOutput", nn_(SpatialSparseCriterion_updateOutput)},
  {"SpatialSparseCriterion_updateGradInput", nn_(SpatialSparseCriterion_updateGradInput)},
  {NULL, NULL}
};

static void nn_(SpatialSparseCriterion_init)(lua_State *L)
{
  luaT_pushmetaclass(L, torch_(Tensor_id));
  luaT_registeratname(L, nn_(SpatialSparseCriterion__), "nn");
  lua_pop(L,1);
}

#endif
