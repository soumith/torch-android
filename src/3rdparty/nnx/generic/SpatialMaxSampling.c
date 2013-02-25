#ifndef TH_GENERIC_FILE
#define TH_GENERIC_FILE "generic/SpatialMaxSampling.c"
#else

#ifndef MAX
#define MAX(a,b) ( ((a)>(b)) ? (a) : (b) )
#endif
#ifndef MIN
#define MIN(a,b) ( ((a)<(b)) ? (a) : (b) )
#endif

static int nn_(SpatialMaxSampling_updateOutput)(lua_State *L)
{
  // get all params
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor);
  int owidth = luaT_getfieldcheckint(L, 1, "owidth");
  int oheight = luaT_getfieldcheckint(L, 1, "oheight");
  THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_Tensor);
  THTensor *indices = luaT_getfieldcheckudata(L, 1, "indices", torch_Tensor);

  // check dims
  luaL_argcheck(L, input->nDimension == 3, 2, "3D tensor expected");

  // dims
  int ichannels = input->size[0];
  int iheight = input->size[1];
  int iwidth = input->size[2];
  int ochannels = ichannels;
  float dW = (float)iwidth/owidth;
  float dH = (float)iheight/oheight;

  // get contiguous input
  input = THTensor_(newContiguous)(input);

  // resize output
  THTensor_(resize3d)(output, ochannels, oheight, owidth);

  // indices will contain i,j locations for each output point
  THTensor_(resize4d)(indices, 2, ochannels, oheight, owidth);

  // get raw pointers
  real *input_data = THTensor_(data)(input);
  real *output_data = THTensor_(data)(output);
  real *indices_data = THTensor_(data)(indices);

  // compute max pooling for each input slice
  long k;
  for (k = 0; k < ochannels; k++) {
    // pointers to slices
    real *input_p = input_data + k*iwidth*iheight;
    real *output_p = output_data + k*owidth*oheight;
    real *indy_p = indices_data + k*owidth*oheight;
    real *indx_p = indices_data + (k+ochannels)*owidth*oheight;

    // loop over output
    int i,j;
    for(i = 0; i < oheight; i++) {
      for(j = 0; j < owidth; j++) {
        // compute nearest offsets
        long ixs = (long)(j*dW);
        long iys = (long)(i*dH);
        long ixe = MAX(ixs+1, (long)((j+1)*dW));
        long iye = MAX(iys+1, (long)((i+1)*dH));

        // local pointers
        real *op = output_p + i*owidth + j;
        real *indxp = indx_p + i*owidth + j;
        real *indyp = indy_p + i*owidth + j;

        // compute local max:
	long maxindex = -1;
	real maxval = -THInf;
	long tcntr = 0;
        int x,y;
        for(y = iys; y < iye; y++) {
          for(x = ixs; x < ixe; x++) {
            real val = *(input_p + y*iwidth + x);
            if (val > maxval) {
              maxval = val;
              maxindex = tcntr;
            }
            tcntr++;
          }
        }

        // set output to local max
        *op = maxval;

        // store location of max (x,y)
        long kW = ixe-ixs;
        *indyp = (int)(maxindex / kW)+1;
        *indxp = (maxindex % kW) +1;
      }
    }
  }

  // cleanup
  THTensor_(free)(input);
  return 1;
}

static int nn_(SpatialMaxSampling_updateGradInput)(lua_State *L)
{
  // get all params
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor);  
  THTensor *gradOutput = luaT_checkudata(L, 3, torch_Tensor);
  THTensor *gradInput = luaT_getfieldcheckudata(L, 1, "gradInput", torch_Tensor);
  THTensor *indices = luaT_getfieldcheckudata(L, 1, "indices", torch_Tensor);
  int owidth = luaT_getfieldcheckint(L, 1, "owidth");
  int oheight = luaT_getfieldcheckint(L, 1, "oheight");

  // sizes
  int ichannels = input->size[0];
  int iheight = input->size[1];
  int iwidth = input->size[2];
  int ochannels = ichannels;
  float dW = (float)iwidth/owidth;
  float dH = (float)iheight/oheight;

  // get contiguous gradOutput
  gradOutput = THTensor_(newContiguous)(gradOutput);

  // resize input
  THTensor_(resizeAs)(gradInput, input);
  THTensor_(zero)(gradInput);

  // get raw pointers
  real *gradInput_data = THTensor_(data)(gradInput);
  real *gradOutput_data = THTensor_(data)(gradOutput);
  real *indices_data = THTensor_(data)(indices);

  // backprop all
  long k;
  for (k = 0; k < ichannels; k++) {
    // pointers to slices
    real *gradOutput_p = gradOutput_data + k*owidth*oheight;
    real *gradInput_p = gradInput_data + k*iwidth*iheight;
    real *indy_p = indices_data + k*owidth*oheight;
    real *indx_p = indices_data + (k+ochannels)*owidth*oheight;

    // calculate max points
    int i,j;
    for(i = 0; i < oheight; i++) {
      for(j = 0; j < owidth; j++) {
        // compute nearest offsets
        long iys = (long)(i*dH);
        long ixs = (long)(j*dW);

        // retrieve position of max
        real *indyp = indy_p + i*owidth + j;
        real *indxp = indx_p + i*owidth + j;
	long maxi = (*indyp) - 1 + iys;
	long maxj = (*indxp) - 1 + ixs;

        // update gradient
        *(gradInput_p + maxi*iwidth + maxj) += *(gradOutput_p + i*owidth + j);
      }
    }
  }

  // cleanup
  THTensor_(free)(gradOutput);

  return 1;
}

static const struct luaL_Reg nn_(SpatialMaxSampling__) [] = {
  {"SpatialMaxSampling_updateOutput", nn_(SpatialMaxSampling_updateOutput)},
  {"SpatialMaxSampling_updateGradInput", nn_(SpatialMaxSampling_updateGradInput)},
  {NULL, NULL}
};

static void nn_(SpatialMaxSampling_init)(lua_State *L)
{
  luaT_pushmetatable(L, torch_Tensor);
  luaT_registeratname(L, nn_(SpatialMaxSampling__), "nn");
  lua_pop(L,1);
}

#endif
