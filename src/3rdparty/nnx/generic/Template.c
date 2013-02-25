#ifndef TH_GENERIC_FILE
#define TH_GENERIC_FILE "generic/Template.c"
#else

static int nn_(Template_updateOutput)(lua_State *L)
{

}

static int nn_(Template_updateGradInput)(lua_State *L)
{

}

static const struct luaL_Reg nn_(Template__) [] = {
  {"Template_updateOutput", nn_(Template_updateOutput)},
  {"Template_updateGradInput", nn_(Template_updateGradInput)},
  {NULL, NULL}
};

static void nn_(Template_init)(lua_State *L)
{
  luaT_pushmetatable(L, torch_Tensor);
  luaT_registeratname(L, nn_(Template__), "nn");
  lua_pop(L,1);
}

#endif
