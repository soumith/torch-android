#ifndef TH_GENERIC_FILE
#define TH_GENERIC_FILE "generic/SpatialClassNLLCriterion.c"
#else

static int nn_(SpatialClassNLLCriterion_updateOutput)(lua_State *L)
{
  THTensor *input  = luaT_checkudata(L, 2, torch_(Tensor_id));
  THTensor *target  = luaT_checkudata(L, 3, torch_(Tensor_id));
  THTensor *output = luaT_getfieldcheckudata(L, 1, "fullOutput", torch_(Tensor_id));

  int height = target->size[0];
  int width = target->size[1];
  int x,y;
  for (y=0; y<height; y++) {
    for (x=0; x<width; x++) {
      THTensor_(set2d)(output, y, x, -THTensor_(get3d)(input, THTensor_(get2d)(target, y, x)-1, y, x) );
    }
  }

  return 1;
}

static int nn_(SpatialClassNLLCriterion_updateGradInput)(lua_State *L)
{
  THTensor *input  = luaT_checkudata(L, 2, torch_(Tensor_id));
  THTensor *target  = luaT_checkudata(L, 3, torch_(Tensor_id));
  THTensor *gradInput  = luaT_checkudata(L, 4, torch_(Tensor_id));

  int height = target->size[0];
  int width = target->size[1];
  int x,y;
  for (y=0; y<height; y++) {
    for (x=0; x<width; x++) {
      THTensor_(set3d)(gradInput, THTensor_(get2d)(target, y, x)-1, y, x, -1);
    }
  }

  return 1;
}

static const struct luaL_Reg nn_(SpatialClassNLLCriterion__) [] = {
  {"SpatialClassNLLCriterion_updateOutput", nn_(SpatialClassNLLCriterion_updateOutput)},
  {"SpatialClassNLLCriterion_updateGradInput", nn_(SpatialClassNLLCriterion_updateGradInput)},
  {NULL, NULL}
};

static void nn_(SpatialClassNLLCriterion_init)(lua_State *L)
{
  luaT_pushmetaclass(L, torch_(Tensor_id));
  luaT_registeratname(L, nn_(SpatialClassNLLCriterion__), "nn");
  lua_pop(L,1);
}

#endif
