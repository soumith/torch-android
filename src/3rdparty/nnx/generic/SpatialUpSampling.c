#ifndef TH_GENERIC_FILE
#define TH_GENERIC_FILE "generic/SpatialUpSampling.c"
#else

static int nn_(SpatialUpSampling_updateOutput)(lua_State *L)
{
  // get all params
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor);
  int dW = luaT_getfieldcheckint(L, 1, "dW");
  int dH = luaT_getfieldcheckint(L, 1, "dH");
  THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_Tensor);

  // dims
  int iwidth = input->size[2];
  int iheight = input->size[1];
  int owidth = iwidth * dW;
  int oheight = iheight * dH;
  int channels1 = input->size[0];
  int channels2 = input->size[3];

  // get strides
  long *is = input->stride;
  long *os = output->stride;

  // get raw pointers
  real *input_data = THTensor_(data)(input);
  real *output_data = THTensor_(data)(output);

  // resample each plane
  int k1, k2, x, y;
  for (k1 = 0; k1 < channels1; k1++) {
    for (k2 = 0; k2 < channels2; k2++) {
      // get planes
      real *input_p = input_data + k1*is[0] + k2*is[3];
      real *output_p = output_data + k1*os[0] + k2*os[3];
      
      // for each plane, resample
      for (y=0; y<oheight; y++) {
	for (x=0; x<owidth; x++) {
	  // input positions (floored)
	  int ix = x/dW;
	  int iy = y/dH;
	  
	  // set output
	  output_p[y*os[1] + x*os[2]] = input_p[iy*is[1] + ix*is[2]];
	}
      }
    }
  }
  return 1;
}

static int nn_(SpatialUpSampling_updateGradInput)(lua_State *L)
{
  // get all params
  //THTensor *input = luaT_checkudata(L, 2, torch_Tensor);
  THTensor *gradOutput = luaT_checkudata(L, 3, torch_Tensor);
  THTensor *gradInput = luaT_getfieldcheckudata(L, 1, "gradInput", torch_Tensor);
  int dW = luaT_getfieldcheckint(L, 1, "dW");
  int dH = luaT_getfieldcheckint(L, 1, "dH");

  // dims
  int owidth = gradOutput->size[2];
  int oheight = gradOutput->size[1];
  int channels1 = gradOutput->size[0];
  int channels2 = gradOutput->size[3];

  // resize gradInput
  THTensor_(zero)(gradInput);

  // get strides
  long *gis = gradInput->stride;
  long *gos = gradOutput->stride;


  // get raw pointers
  real *gradInput_data = THTensor_(data)(gradInput);
  real *gradOutput_data = THTensor_(data)(gradOutput);

  // compute gradients for each plane
  int k1, k2, x, y;
  for (k1 = 0; k1 < channels1; k1++) {
    for (k2 = 0; k2 < channels2; k2++) {
      // get planes
      real *gradInput_p = gradInput_data + k1*gis[0] + k2*gis[3];
      real *gradOutput_p = gradOutput_data + k1*gos[0] + k2*gos[3];
      
      // for each plane, resample
      for (y=0; y<oheight; y++) {
	for (x=0; x<owidth; x++) {
	  // input positions (floored)
	  int ix = x/dW;
	  int iy = y/dH;
	  
	  // accumulate gradient
	  gradInput_p[iy*gis[1] + ix*gis[2]] += gradOutput_p[y*gos[1] + x*gos[2]];
	}
      }
    }
  }
  return 1;
}

static const struct luaL_Reg nn_(SpatialUpSampling__) [] = {
  {"SpatialUpSampling_updateOutput", nn_(SpatialUpSampling_updateOutput)},
  {"SpatialUpSampling_updateGradInput", nn_(SpatialUpSampling_updateGradInput)},
  {NULL, NULL}
};

static void nn_(SpatialUpSampling_init)(lua_State *L)
{
  luaT_pushmetatable(L, torch_Tensor);
  luaT_registeratname(L, nn_(SpatialUpSampling__), "nn");
  lua_pop(L,1);
}

#endif
