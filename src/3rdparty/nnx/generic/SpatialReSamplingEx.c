#ifndef TH_GENERIC_FILE
#define TH_GENERIC_FILE "generic/SpatialReSamplingEx.c"
#else
#include<assert.h>

#ifndef MAX
#define MAX(a,b) ( ((a)>(b)) ? (a) : (b) )
#endif
#ifndef MIN
#define MIN(a,b) ( ((a)<(b)) ? (a) : (b) )
#endif

static int nn_(SpatialReSamplingEx_updateOutput)(lua_State *L)
{
  // get all params
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor);
  THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_Tensor);
  int oheight = luaT_getfieldcheckint(L, 1, "oheightCurrent");
  int owidth = luaT_getfieldcheckint(L, 1, "owidthCurrent");
  int mode = luaT_getfieldcheckint(L, 1, "mode_c");

  // dims
  int iwidth = input->size[2];
  int iheight = input->size[1];
  int channels1 = input->size[0];
  int channels2 = input->size[3];

  // get strides
  long *is = input->stride;
  long *os = output->stride;
  
  // get raw pointers
  real *input_data = THTensor_(data)(input);
  real *output_data = THTensor_(data)(output);

  if (mode == 2) { //bilinear

    // mapping ratios
    float wratio = (float)(iwidth-1) / (owidth-1);
    float hratio = (float)(iheight-1) / (oheight-1);
    
    // resample each plane
    int k1, k2, x, y;
    for (k1 = 0; k1 < channels1; ++k1) {
      for (k2 = 0; k2 < channels2; ++k2) {
      
	// get planes
	real* input_p = input_data + k1*is[0] + k2*is[3];
	real *output_p = output_data + k1*os[0] + k2*os[3];
      
	// for each plane, resample
	for (y = 0; y < oheight; ++y) {
	  for (x = 0; x < owidth; ++x) {
	    
	    // subpixel position:
	    const float ix = wratio*x;
	    const float iy = hratio*y;
	  
	    // 4 nearest neighbors:
	    const int ix_nw = floor(ix);
	    const int iy_nw = floor(iy);
	    const int ix_ne = ix_nw + 1;
	    const int iy_ne = iy_nw;
	    const int ix_sw = ix_nw;
	    const int iy_sw = iy_nw + 1;
	    const int ix_se = ix_nw + 1;
	    const int iy_se = iy_nw + 1;
	  
	    // get surfaces to each neighbor:
	    const float se = (ix-(float)ix_nw)*(iy-(float)iy_nw);
	    const float sw = ((float)ix_ne-ix)*(iy-(float)iy_ne);
	    const float ne = (ix-(float)ix_sw)*((float)iy_sw-iy);
	    const float nw = ((float)ix_se-ix)*((float)iy_se-iy);
	  
	    // weighted sum of neighbors:
	    output_p[y*os[1] + x*os[2]] = input_p[iy_nw*is[1] + ix_nw*is[2]] * nw
	      + input_p[iy_ne*is[1] + MIN(ix_ne,iwidth-1)*is[2]] * ne
	      + input_p[MIN(iy_sw,iheight-1)*is[1] + ix_sw*is[2]] * sw
	      + input_p[MIN(iy_se,iheight-1)*is[1] + MIN(ix_se,iwidth-1)*is[2]] * se;
	  }
	}
      }
    }

  } else { // simple or average
    assert((mode == 0) || (mode == 1));
        
    // resample
    if (oheight >= iheight) { 
      // upsampling (from lua check we know that owidth >= iwidth)
      // upsampling average mode is actually simple mode
      int dH = (oheight+iheight-1)/iheight; //=ceil((float)oheight/(float(iheight)))
      int dW = (owidth+iwidth-1)/iwidth;
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
    } else {
      // downsampling
      
      if (mode == 1) { // average
	
	int dH = iheight/oheight;
	int dW = iwidth/owidth;
	real avg;
	int k1, k2, x, y, i, j;
	for (k1 = 0; k1 < channels1; k1++) {
	  for (k2 = 0; k2 < channels2; k2++) {
	    // get planes
	    real *input_p = input_data + k1*is[0] + k2*is[3];
	    real *output_p = output_data + k1*os[0] + k2*os[3];
	    
	    // for each plane, resample
	    for (y = 0; y < oheight; ++y)
	      for (x = 0; x < owidth; ++x) {
		avg = 0.0;
		for (i = y*dH; i < (y+1)*dH; ++i)
		  for (j = x*dW; j < (x+1)*dW; ++j)
		    avg += input_p[i*is[1]+j*is[2]];
		output_p[y*os[1] + x*os[2]] = avg;
	      }
	  }
	}
	THTensor_(mul)(output, output, 1.0f/(dH*dW));
	
      } else { // simple
	assert(mode == 0);

	int dH = iheight/oheight;
	int dW = iwidth/owidth;
	int k1, k2, x, y;
	for (k1 = 0; k1 < channels1; k1++) {
	  for (k2 = 0; k2 < channels2; k2++) {
	    // get planes
	    real *input_p = input_data + k1*is[0] + k2*is[3];
	    real *output_p = output_data + k1*os[0] + k2*os[3];
	    
	    // for each plane, resample
	    for (y = 0; y < oheight; ++y)
	      for (x = 0; x < owidth; ++x)
		output_p[y*os[1] + x*os[2]] = input_p[y*dH*is[1]+x*dW*is[2]];
	  }
	}
	
      } // mode = average | simple
      
    } // upsampling / downsampling
    
  } // mode = bilinear ?
  return 1;
}

static int nn_(SpatialReSamplingEx_updateGradInput)(lua_State *L)
{
  // get all params
  int iheight = luaT_getfieldcheckint(L, 1, "iheight");
  int iwidth = luaT_getfieldcheckint(L, 1, "iwidth");
  THTensor *gradOutput = luaT_checkudata(L, 2, torch_Tensor);
  THTensor *gradInput = luaT_getfieldcheckudata(L, 1, "gradInput", torch_Tensor);
  int mode = luaT_getfieldcheckint(L, 1, "mode_c");

  // dims
  int owidth = gradOutput->size[2];
  int oheight = gradOutput->size[1];
  int channels1 = gradOutput->size[0];
  int channels2 = gradOutput->size[3];

  // zero gradInput
  THTensor_(zero)(gradInput);

  // get strides
  long *gis = gradInput->stride;
  long *gos = gradOutput->stride;

  // get raw pointers
  real *gradInput_data = THTensor_(data)(gradInput);
  real *gradOutput_data = THTensor_(data)(gradOutput);
  
  if (mode == 2) { //bilinear
    
    // mapping ratios
    float wratio = (float)(iwidth-1) / (owidth-1);
    float hratio = (float)(iheight-1) / (oheight-1);

    // compute gradients for each plane
    int k1, k2, x, y;
    for (k1 = 0; k1 < channels1; ++k1) {
      for (k2 = 0; k2 < channels2; ++k2) {
	
	// get planes
	real *gradInput_p = gradInput_data + k1*gis[0] + k2*gis[3];
	real *gradOutput_p = gradOutput_data + k1*gos[0] + k2*gos[3];
	
	// for each plane, resample
	for (y = 0; y < oheight; ++y) {
	  for (x = 0; x < owidth; ++x) {
	    
	    // subpixel position:
	    const float ix = wratio*x;
	    const float iy = hratio*y;

	    // 4 nearest neighbors:
	    const int ix_nw = floor(ix);
	    const int iy_nw = floor(iy);
	    const int ix_ne = ix_nw + 1;
	    const int iy_ne = iy_nw;
	    const int ix_sw = ix_nw;
	    const int iy_sw = iy_nw + 1;
	    const int ix_se = ix_nw + 1;
	    const int iy_se = iy_nw + 1;

	    // get surfaces to each neighbor:
	    const float se = (ix-(float)ix_nw)*(iy-(float)iy_nw);
	    const float sw = ((float)ix_ne-ix)*(iy-(float)iy_ne);
	    const float ne = (ix-(float)ix_sw)*((float)iy_sw-iy);
	    const float nw = ((float)ix_se-ix)*((float)iy_se-iy);

	    // output gradient
	    const double ograd = gradOutput_p[y*gos[1] + x*gos[2]];

	    // accumulate gradient
	    gradInput_p[iy_nw*gis[1] + ix_nw*gis[2]] += nw * ograd;
	    gradInput_p[iy_ne*gis[1] + MIN(ix_ne,iwidth-1)*gis[2]] += ne * ograd;
	    gradInput_p[MIN(iy_sw,iheight-1)*gis[1] + ix_sw*gis[2]] += sw * ograd;
	    gradInput_p[MIN(iy_se,iheight-1)*gis[1] + MIN(ix_se,iwidth-1)*gis[2]] += se*ograd;
	  }
	}
      }
    }

  } else { // simple or average
    assert((mode == 0) || (mode == 1));
  
    // compute gradients
    if (oheight >= iheight) { 
      // upsampling (from lua check we know that owidth >= iwidth)
      // upsampling average mode is actually simple mode
      int dH = (oheight+iheight-1)/iheight; //=ceil((float)oheight/(float(iheight)))
      int dW = (owidth+iwidth-1)/iwidth;
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
    } else {
      if (mode == 1) { // average
	// downsampling
	int dH = iheight/oheight;
	int dW = iwidth/owidth;
	int k1, k2, x, y, i, j;
	for (k1 = 0; k1 < channels1; k1++) {
	  for (k2 = 0; k2 < channels2; k2++) {
	    // get planes
	    real *gradInput_p = gradInput_data + k1*gis[0] + k2*gis[3];
	    real *gradOutput_p = gradOutput_data + k1*gos[0] + k2*gos[3];
	    
	    // for each plane, resample
	    for (y = 0; y < oheight; ++y)
	      for (x = 0; x < owidth; ++x)
		for (i = y*dH; i < (y+1)*dH; ++i)
		  for (j = x*dW; j < (x+1)*dW; ++j)
		    gradInput_p[i*gis[1]+j*gis[2]] += gradOutput_p[y*gos[1]+x*gos[2]];
	  }
	}
	THTensor_(mul)(gradInput, gradInput, ((real)1.0)/(dH*dW));
      } else { // simple

	assert(mode == 0);
	int dH = iheight/oheight;
	int dW = iwidth/owidth;
	int k1, k2, x, y;
	for (k1 = 0; k1 < channels1; k1++) {
	  for (k2 = 0; k2 < channels2; k2++) {
	    // get planes
	    real *gradInput_p = gradInput_data + k1*gis[0] + k2*gis[3];
	    real *gradOutput_p = gradOutput_data + k1*gos[0] + k2*gos[3];
	    
	    // for each plane, resample
	    for (y = 0; y < oheight; ++y)
	      for (x = 0; x < owidth; ++x)
		gradInput_p[y*dH*gis[1] + x*dW*gis[2]] = gradOutput_p[y*gos[1]+x*gos[2]];
	  }
	}

      } // mode = simple | average
      
    } // upsampling / downsampling
    
  } // mode = bilinear ?
  return 1;
}

static const struct luaL_Reg nn_(SpatialReSamplingEx__) [] = {
  {"SpatialReSamplingEx_updateOutput", nn_(SpatialReSamplingEx_updateOutput)},
  {"SpatialReSamplingEx_updateGradInput", nn_(SpatialReSamplingEx_updateGradInput)},
  {NULL, NULL}
};

static void nn_(SpatialReSamplingEx_init)(lua_State *L)
{
  luaT_pushmetatable(L, torch_Tensor);
  luaT_registeratname(L, nn_(SpatialReSamplingEx__), "nn");
  lua_pop(L,1);
}

#endif
