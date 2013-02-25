#ifndef TH_GENERIC_FILE
#define TH_GENERIC_FILE "generic/SpatialMSECriterion.c"
#else

static int nn_(SpatialMSECriterion_updateOutput)(lua_State *L)
{
  THTensor *input  = luaT_checkudata(L, 2, torch_(Tensor_id));
  THTensor *target  = luaT_checkudata(L, 3, torch_(Tensor_id));
  THTensor *output = luaT_getfieldcheckudata(L, 1, "fullOutput", torch_(Tensor_id));
  real z = 0;
  TH_TENSOR_APPLY3(real, output, real, input, real, target,
                   z = (*input_data - *target_data);
                   *output_data =  0.5*z*z;)
    return 1;
}

static int nn_(SpatialMSECriterion_updateGradInput)(lua_State *L)
{
  THTensor *input  = luaT_checkudata(L, 2, torch_(Tensor_id));
  THTensor *target  = luaT_checkudata(L, 3, torch_(Tensor_id));
  THTensor *gradInput  = luaT_checkudata(L, 4, torch_(Tensor_id));
  TH_TENSOR_APPLY3(real, gradInput, real, input, real, target,
                   *gradInput_data = (*input_data - *target_data);)
    return 1;
}

static int nn_(SpatialMSECriterion_retarget)(lua_State *L)
{
  THTensor *new  = luaT_checkudata(L, 1, torch_(Tensor_id));
  THTensor *old  = luaT_checkudata(L, 2, torch_(Tensor_id));
  int height = new->size[1];
  int width = new->size[2];
  int k,x,y;
  for (y=0; y<height; y++) {
    for (x=0; x<width; x++) {
      THTensor_(set3d)(new, THTensor_(get2d)(old, y, x)-1, y, x, 1);
    }
  }
  return 1;
}

static const struct luaL_Reg nn_(SpatialMSECriterion__) [] = {
  {"SpatialMSECriterion_updateOutput", nn_(SpatialMSECriterion_updateOutput)},
  {"SpatialMSECriterion_updateGradInput", nn_(SpatialMSECriterion_updateGradInput)},
  {"SpatialMSECriterion_retarget", nn_(SpatialMSECriterion_retarget)},
  {NULL, NULL}
};

static void nn_(SpatialMSECriterion_init)(lua_State *L)
{
  luaT_pushmetaclass(L, torch_(Tensor_id));
  luaT_registeratname(L, nn_(SpatialMSECriterion__), "nn");
  lua_pop(L,1);
}

#endif
