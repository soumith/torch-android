#ifndef TH_GENERIC_FILE
#define TH_GENERIC_FILE "generic/SpatialMatching.c"
#else

#define square(x) ((x)*(x))
#define max(x,y) (((x)>(y)) ? (x) : (y))
#define min(x,y) (((x)>(y)) ? (y) : (x))

static int nn_(SpatialMatching_updateOutput)(lua_State *L)
{
  // get all params
  THTensor *input1 = luaT_checkudata(L, 2, torch_Tensor);
  THTensor *input2 = luaT_checkudata(L, 3, torch_Tensor);
  int maxw = luaT_getfieldcheckint(L, 1, "maxw");
  int maxh = luaT_getfieldcheckint(L, 1, "maxh");
  int full_output = luaT_getfieldcheckboolean(L, 1, "full_output");
  THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_Tensor);

  // dims
  int iwidth = input1->size[2];
  int iheight = input1->size[1];
  int ichannels = input1->size[0];

  // make contiguous
  //input1 = THTensor_(newContiguous)(input1);
  //input2 = THTensor_(newContiguous)(input2);
  //output = THTensor_(newContiguous)(output);

  // zero output
  THTensor_(fill)(output, 1e30);

  // get strides
  long *i1s = input1->stride;
  long *i2s = input2->stride;
  long *os  = output->stride;

  // get pointers
  real *input1_p = THTensor_(data)(input1);
  real *input2_p = THTensor_(data)(input2);
  real *output_p = THTensor_(data)(output);

  // compute output
  int x1,y1,x2,y2,k;
  real dist;
  if (full_output) {
    // get halves of window size
    int halfh1 = ceil((real)maxh/2)-1;
    int halfh2 = floor((real)maxh/2)+1;
    int halfw1 = ceil((real)maxw/2)-1;
    int halfw2 = floor((real)maxw/2)+1;

    long dy, dx;
    
    #pragma omp parallel for private(x1,x2,y2,k,dist,dy,dx)
    for (y1 = 0; y1 < iheight; y1++) {
      for (x1 = 0; x1 < iwidth; x1++) {
	for (y2 = max(0,y1-halfh1); y2 < min(iheight,y1+halfh2); y2++) {
	  for (x2 = max(0,(x1-halfw1)); x2 < min(iwidth,x1+halfw2); x2++) {
	    dist = 0;
	    for (k = 0; k < ichannels; k++) {
	      dist += square(input1_p[k*i1s[0] + y1*i1s[1] + x1*i1s[2]] - input2_p[k*i2s[0] + y2*i2s[1] + x2*i2s[2]]);
	    }
	    dy = y2-y1 + halfh1;
	    dx = x2-x1 + halfw1;
	    output_p[dy*os[2] + dx*os[3] + y1*os[0] + x1*os[1]] = dist;
	  }
	}
      }
    }
    /*
    real *input1_p_it_start = input1_p, *input1_p_it_end = input1_p+ichannels*i1s[0];
    real *input1_p_it, *input2_p_it;
    for (y1 = 0; y1 < iheight; y1++) {
      for (x1 = 0; x1 < iwidth; x1++, ++input1_p_it_start, ++input1_p_it_end) {
	for (y2 = max(0,y1-halfh1); y2 < min(iheight,y1+halfh2); y2++) {
	  for (x2 = max(0,(x1-halfw1)); x2 < min(iwidth,x1+halfw2); x2++) {
	    dist = 0;
	    for (input1_p_it = input1_p_it_start, input2_p_it=input2_p+y2*i2s[1]+x2*i2s[2];
		 input1_p_it != input1_p_it_end;
		 input1_p_it += i1s[0], input2_p_it += i2s[0]) {
	      dist += square(*input1_p_it - *input2_p_it);
	    }
	    dy = y2-y1 + halfh1;
	    dx = x2-x1 + halfw1;
	    output_p[dy*os[0] + dx*os[1] + y1*os[2] + x1*os[3]] = dist;
	  }
	}
      }
    }
    */
  } else {
#pragma omp parallel for private(y1,x1,x2,y2,k,dist)
    for (y1 = 0; y1 < iheight; y1++) {
      for (x1 = 0; x1 < iwidth; x1++) {
	for (y2 = y1; y2 < y1+maxh; y2++) {
	  for (x2 = x1; x2 < x1+maxw; x2++) {
	    dist = 0;
	    for (k = 0; k < ichannels; k++) {
	      dist += square(input1_p[k*i1s[0] + y1*i1s[1] + x1*i1s[2]] - input2_p[k*i2s[0] + y2*i2s[1] + x2*i2s[2]]);
	    }
	    output_p[(y2-y1)*os[2] + (x2-x1)*os[3] + y1*os[0] + x1*os[1]] = dist;
	  }
	}
      }
    }
  }
    

  // done
  return 1;
}

static int nn_(SpatialMatching_updateGradInput)(lua_State *L)
{
  // get all params
  THTensor *input1 = luaT_checkudata(L, 2, torch_Tensor);
  THTensor *input2 = luaT_checkudata(L, 3, torch_Tensor);
  THTensor *gradInput1 = luaT_getfieldcheckudata(L, 1, "gradInput1", torch_Tensor);
  THTensor *gradInput2 = luaT_getfieldcheckudata(L, 1, "gradInput2", torch_Tensor);
  THTensor *gradOutput = luaT_checkudata(L, 4, torch_Tensor);
  int full_output = luaT_getfieldcheckboolean(L, 1, "full_output");
  int maxw = luaT_getfieldcheckint(L, 1, "maxw");
  int maxh = luaT_getfieldcheckint(L, 1, "maxh");

  // dims
  int iwidth = input1->size[2];
  int iheight = input1->size[1];
  int ichannels = input1->size[0];

  // get strides
  long *i1s = input1->stride;
  long *i2s = input2->stride;
  long *gi1s = gradInput1->stride;
  long *gi2s = gradInput2->stride;
  long *gos = gradOutput->stride;
  
  // get pointers
  real *input1_p = THTensor_(data)(input1);
  real *input2_p = THTensor_(data)(input2);
  real *gradInput1_p = THTensor_(data)(gradInput1);
  real *gradInput2_p = THTensor_(data)(gradInput2);
  real *gradOutput_p = THTensor_(data)(gradOutput);
  
  // compute gradients
  int x1, y1, x2, y2, k;
  real partial_d;
  if (full_output) {
    // get halves of window size
    int halfh1 = ceil((real)maxh/2)-1;
    int halfh2 = floor((real)maxh/2)+1;
    int halfw1 = ceil((real)maxw/2)-1;
    int halfw2 = floor((real)maxw/2)+1;

    long dy, dx;
    //#pragma omp parallel for private(x1,x2,y2,k,dy,dx,partial_d) NO! gradInput has +=
    for (y1 = 0; y1 < iheight; y1++) {
      for (x1 = 0; x1 < iwidth; x1++) {
	for (y2 = max(0,y1-halfh1); y2 < min(iheight,y1+halfh2); y2++) {
	  for (x2 = max(0,(x1-halfw1)); x2 < min(iwidth,x1+halfw2); x2++) {
	    dy = y2-y1 + halfh1;
	    dx = x2-x1 + halfw1;
	    for (k=0; k<ichannels; k++) {
	      partial_d = 2*(input1_p[k*i1s[0] + y1*i1s[1] + x1*i1s[2]] - input2_p[k*i2s[0] + y2*i2s[1] + x2*i2s[2]]);
	      partial_d *= gradOutput_p[dy*gos[2] + dx*gos[3] + y1*gos[0] + x1*gos[1]];
	      gradInput1_p[k*gi1s[0] + y1*gi1s[1] + x1*gi1s[2]] += partial_d;
	      gradInput2_p[k*gi2s[0] + y2*gi2s[1] + x2*gi2s[2]] -= partial_d;
	    }
	  }
	}
      }
    }
  } else {
    //#pragma omp parallel for private(x1,x2,y2,k,partial_d)
    for (y1 = 0; y1 < iheight; y1++) {
      for (x1 = 0; x1 < iwidth; x1++) {
	for (y2 = y1; y2 < y1+maxh; y2++) {
	  for (x2 = x1; x2 < x1+maxw; x2++) {
	    for (k = 0; k < ichannels; k++) {
	      partial_d = 2*(input1_p[k*i1s[0] + y1*i1s[1] + x1*i1s[2]] - input2_p[k*i2s[0] + y2*i2s[1] + x2*i2s[2]]);
	      partial_d *= gradOutput_p[(y2-y1)*gos[2]+(x2-x1)*gos[3]+y1*gos[0]+x1*gos[1]];
	      gradInput1_p[k*gi1s[0] + y1*gi1s[1] + x1*gi1s[2]] += partial_d;
	      gradInput2_p[k*gi2s[0] + y2*gi2s[1] + x2*gi2s[2]] -= partial_d;
	      
	    }
	  }
	}
      }
    }
  }

  // done
  return 1;
}

static const struct luaL_Reg nn_(SpatialMatching__) [] = {
  {"SpatialMatching_updateOutput", nn_(SpatialMatching_updateOutput)},
  {"SpatialMatching_updateGradInput", nn_(SpatialMatching_updateGradInput)},
  {NULL, NULL}
};

static void nn_(SpatialMatching_init)(lua_State *L)
{
  luaT_pushmetatable(L, torch_Tensor);
  luaT_registeratname(L, nn_(SpatialMatching__), "nn");
  lua_pop(L,1);
}

#endif
