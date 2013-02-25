#ifndef TH_GENERIC_FILE
#define TH_GENERIC_FILE "generic/SparseCriterion.c"
#else

static int nn_(SparseCriterion_updateOutput)(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_(Tensor_id));  
  int sizeAverage = luaT_getfieldcheckboolean(L, 1, "sizeAverage");
  real sum = 0;

  TH_TENSOR_APPLY(real, input, sum += fabs(*input_data);)

  if(sizeAverage) sum /= THTensor_(nElement)(input);

  lua_pushnumber(L, sum);
  lua_setfield(L, 1, "output");

  lua_pushnumber(L, sum);
  return 1;
}

static int nn_(SparseCriterion_updateGradInput)(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_(Tensor_id));
  int sizeAverage = luaT_getfieldcheckboolean(L, 1, "sizeAverage");
  THTensor *gradInput = luaT_getfieldcheckudata(L, 1, "gradInput", torch_(Tensor_id));
  real norm = (sizeAverage ? 1./((real)THTensor_(nElement)(input)) : 1.);

  THTensor_(resizeAs)(gradInput, input);
  TH_TENSOR_APPLY2(real, gradInput, real, input,
                   *gradInput_data = ( *input_data >= 0 ? norm : -norm);)

  return 1;
}

static const struct luaL_Reg nn_(SparseCriterion__) [] = {
  {"SparseCriterion_updateOutput", nn_(SparseCriterion_updateOutput)},
  {"SparseCriterion_updateGradInput", nn_(SparseCriterion_updateGradInput)},
  {NULL, NULL}
};

static void nn_(SparseCriterion_init)(lua_State *L)
{
  luaT_pushmetaclass(L, torch_(Tensor_id));
  luaT_registeratname(L, nn_(SparseCriterion__), "nn");
  lua_pop(L,1);
}

#endif
