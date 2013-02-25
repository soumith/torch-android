#ifndef TH_GENERIC_FILE
#define TH_GENERIC_FILE "generic/SpatialRadialMatching.c"
#else

#define square(x) ((x)*(x))
#define max(x,y) (((x)>(y)) ? (x) : (y))
#define min(x,y) (((x)>(y)) ? (y) : (x))

static int nn_(SpatialRadialMatching_updateOutput)(lua_State *L)
{
  // get all params
  THTensor *input1  = luaT_checkudata(L, 2, torch_Tensor);
  THTensor *input2  = luaT_checkudata(L, 3, torch_Tensor);
  //THLongTensor *mask= luaT_checkudata(L, 4, luaT_checktypename2id(L, "torch.LongTensor"));
  int maxh          = luaT_getfieldcheckint(L, 1, "maxh");
  THTensor *output  = luaT_getfieldcheckudata(L, 1, "output", torch_Tensor);

  // dims
  int iwidth = input1->size[2];
  int iheight = input1->size[1];
  int ichannels = input1->size[0];

  // get strides
  long *i1s = input1->stride;
  long *i2s = input2->stride;
  //long *ms  = mask  ->stride;
  long *os  = output->stride;

  // get pointers
  real *input1_p = THTensor_(data)(input1);
  real *input2_p = THTensor_(data)(input2);
  //long *mask_p   = THLongTensor_data(mask);
  real *output_p = THTensor_(data)(output);

  // compute output
  int x1,y1,y2,k;
  real dist;
#pragma omp parallel for private(y1,x1,y2,k,dist)
  for (y1 = 0; y1 < iheight; y1++) {
    for (x1 = 0; x1 < iwidth; x1++) {
      //if (mask_p[y1*ms[0] + x1*ms[1]]) {
	for (y2 = y1; y2 < y1+maxh; y2++) {
	  dist = 0.0f;
	  for (k = 0; k < ichannels; k++)
	    dist += square(  input1_p[k*i1s[0] + y1*i1s[1] + x1*i1s[2]]
			     - input2_p[k*i2s[0] + y2*i2s[1] + x1*i2s[2]]);
	  output_p[(y2-y1)*os[2] + y1*os[0] + x1*os[1]] = dist;
	}
	//}
    }
  }
  
  // done
  return 0;
}

static int nn_(SpatialRadialMatching_updateGradInput)(lua_State *L)
{
  // get all params
  THTensor*     input1 = luaT_checkudata(L, 2, torch_Tensor);
  THTensor*     input2 = luaT_checkudata(L, 3, torch_Tensor);
  THTensor* gradOutput = luaT_checkudata(L, 4, torch_Tensor);
  //THLongTensor*   mask = luaT_checkudata(L, 5, luaT_checktypename2id(L, "torch.LongTensor"));
  THTensor* gradInput1 = luaT_getfieldcheckudata(L, 1, "gradInput1", torch_Tensor);
  THTensor* gradInput2 = luaT_getfieldcheckudata(L, 1, "gradInput2", torch_Tensor);
  int             maxh = luaT_getfieldcheckint(L, 1, "maxh");

  // dims
  int iwidth    = input1->size[2];
  int iheight   = input1->size[1];
  int ichannels = input1->size[0];

  // get strides
  long* i1s  = input1->stride;
  long* i2s  = input2->stride;
  long* gi1s = gradInput1->stride;
  long* gi2s = gradInput2->stride;
  long* gos  = gradOutput->stride;
  //long* ms   = mask->stride;
  
  // get pointers
  real* input1_p     = THTensor_(data)(input1);
  real* input2_p     = THTensor_(data)(input2);
  real* gradInput1_p = THTensor_(data)(gradInput1);
  real* gradInput2_p = THTensor_(data)(gradInput2);
  real* gradOutput_p = THTensor_(data)(gradOutput);
  //long* mask_p       = THLongTensor_data(mask);
  
  // compute gradients
  int x1, y1, y2, k;
  real partial_d;
  for (y1 = 0; y1 < iheight; y1++) {
    for (x1 = 0; x1 < iwidth; x1++) {
      // if (mask_p[y1*ms[0] + x1*ms[1]]) {
	for (y2 = y1; y2 < y1+maxh; y2++) {
	  for (k = 0; k < ichannels; k++) {
	    partial_d = 2.0f*(  input1_p[k*i1s[0] + y1*i1s[1] + x1*i1s[2]]
				- input2_p[k*i2s[0] + y2*i2s[1] + x1*i2s[2]]);
	    partial_d *= gradOutput_p[(y2-y1)*gos[2]+y1*gos[0]+x1*gos[1]];
	    gradInput1_p[k*gi1s[0] + y1*gi1s[1] + x1*gi1s[2]] += partial_d;
	    gradInput2_p[k*gi2s[0] + y2*gi2s[1] + x1*gi2s[2]] -= partial_d;
	  }
	}
	//}
    }
  }

  // done
  return 0;
}

static const struct luaL_Reg nn_(SpatialRadialMatching__) [] = {
  {"SpatialRadialMatching_updateOutput", nn_(SpatialRadialMatching_updateOutput)},
  {"SpatialRadialMatching_updateGradInput", nn_(SpatialRadialMatching_updateGradInput)},
  {NULL, NULL}
};

static void nn_(SpatialRadialMatching_init)(lua_State *L)
{
  luaT_pushmetatable(L, torch_Tensor);
  luaT_registeratname(L, nn_(SpatialRadialMatching__), "nn");
  lua_pop(L,1);
}

#endif
