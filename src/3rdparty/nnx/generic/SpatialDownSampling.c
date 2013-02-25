#ifndef TH_GENERIC_FILE
#define TH_GENERIC_FILE "generic/SpatialDownSampling.c"
#else

static int nn_(SpatialDownSampling_updateOutput)(lua_State *L) {
  // get all params
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor);
  int rW = luaT_getfieldcheckint(L, 1, "rW");
  int rH = luaT_getfieldcheckint(L, 1, "rH");
  THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_Tensor);

  // dims
  int iwidth = input->size[2];
  int iheight = input->size[1];
  int ichannels = input->size[0];
  int owidth = floor(iwidth / rW);
  int oheight = floor(iheight / rH);

  // get strides
  long *is = input->stride;
  long *os = output->stride;

  // get raw pointers
  real *input_data = THTensor_(data)(input);
  real *output_data = THTensor_(data)(output);

  // resample each plane
  real avg;
  real *input_p = input_data, *output_p = output_data;
  int k, x, y, i, j;
  for (k = 0; k < ichannels; ++k, input_p += is[0], output_p += os[0])
    for (y = 0; y < oheight; ++y)
      for (x = 0; x < owidth; ++x) {
	avg = 0.0;
	for (i = y*rH; i < (y+1)*rH; ++i)
	  for (j = x*rW; j < (x+1)*rW; ++j)
	    avg += input_p[i*is[1]+j*is[2]];
        output_p[y*os[1] + x*os[2]] = avg;
      }
  THTensor_(mul)(output, output, 1.0f/(rH*rW));
  return 1;
}

static int nn_(SpatialDownSampling_updateGradInput)(lua_State *L) {
  // get all params
  THTensor *gradOutput = luaT_checkudata(L, 2, torch_Tensor);
  THTensor *gradInput = luaT_getfieldcheckudata(L, 1, "gradInput", torch_Tensor);
  int rW = luaT_getfieldcheckint(L, 1, "rW");
  int rH = luaT_getfieldcheckint(L, 1, "rH");

  // dims
  int owidth = gradOutput->size[2];
  int oheight = gradOutput->size[1];
  int ochannels = gradOutput->size[0];

  // get strides
  long *gis = gradInput->stride;
  long *gos = gradOutput->stride;

  THTensor_(zero)(gradInput);

  // get raw pointers
  real *gradInput_data = THTensor_(data)(gradInput);
  real *gradOutput_data = THTensor_(data)(gradOutput);

  // compute gradients for each plane
  real *gradInput_p = gradInput_data, *gradOutput_p = gradOutput_data;
  int k, x, y, i, j;
  for (k = 0; k < ochannels; ++k, gradInput_p += gis[0], gradOutput_p += gos[0])
    for (y = 0; y < oheight; ++y)
      for (x = 0; x < owidth; ++x)
	for (i = y*rH; i < (y+1)*rH; ++i)
	  for (j = x*rW; j < (x+1)*rW; ++j)
	    gradInput_p[i*gis[1]+j*gis[2]] += gradOutput_p[y*gos[1]+x*gos[2]];
  THTensor_(mul)(gradInput, gradInput, 1.0f/(rH*rW));
  
  return 1;
}

static const struct luaL_Reg nn_(SpatialDownSampling__) [] = {
  {"SpatialDownSampling_updateOutput", nn_(SpatialDownSampling_updateOutput)},
  {"SpatialDownSampling_updateGradInput", nn_(SpatialDownSampling_updateGradInput)},
  {NULL, NULL}
};

static void nn_(SpatialDownSampling_init)(lua_State *L)
{
  luaT_pushmetatable(L, torch_Tensor);
  luaT_registeratname(L, nn_(SpatialDownSampling__), "nn");
  lua_pop(L,1);
}

#endif
