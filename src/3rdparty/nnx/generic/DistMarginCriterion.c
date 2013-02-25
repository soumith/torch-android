#ifndef TH_GENERIC_FILE
#define TH_GENERIC_FILE "generic/DistMarginCriterion.c"
#else

static int nn_(DistMarginCriterion_updateOutput)(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor);
  int sizeAverage = luaT_getfieldcheckboolean(L, 1, "sizeAverage");
  real *input_data, *target_data;
  long nframe, dim;
  long t, d, m;
  THTensor *target_;
  THTensor *target;
  real sum;

  THArgCheck((input->nDimension == 1) || (input->nDimension == 2), 2, "vector or matrix expected");

  if(input->nDimension == 1) {
    nframe = 1;
    dim = input->size[0];
    target_ = luaT_checkudata(L, 3, torch_Tensor);
    target = THTensor_(new)();
    THTensor_(set)(target, target_);
    THTensor_(resize2d)(target, 1, dim);
  }
  else {
    nframe = input->size[0];
    dim = input->size[1];
    target_ = luaT_checkudata(L, 3, torch_Tensor);
    THArgCheck((target_->nDimension == 2) && (target_->size[0] == nframe) && (target_->size[1] == dim),
               3, "inconsistent target size");
    target = THTensor_(newContiguous)(target_);
  }

  for(t = 0; t < nframe; t++) {
    for(d = 0; d < dim; d++) {
      real idx = THTensor_(get2d)(target, t, d);
      THArgCheck((idx >= 0) && (idx <= dim), 3, "target out of range");
    }
  }

  input = THTensor_(newContiguous)(input);
  input_data = THTensor_(data)(input);
  target_data = THTensor_(data)(target);

  sum = 0;
  for(t = 0; t < nframe; t++) {
    real input_target = THInf;
    for (m = 0; m < dim; m++) {
      long target_idx = (long)(target_data[m]-1);
      if (target_idx == -1) break;
      if (input_target > input_data[target_idx]) input_target = input_data[target_idx];
    }
    for(d = 0; d < dim; d++) {
      int isatarget = 0;
      for(m = 0; m < dim; m++) {
        long target_idx = (long)(target_data[m]-1);
        if (target_idx == -1) break;
        else if(d == target_idx) {
          isatarget = 1;
          break;
        }
      }
      if (isatarget) continue;

      real z = 1 - input_target + input_data[d];
      if(z > 0) sum += z;
    }
    input_data += dim;
    target_data += dim;
  }

  if(sizeAverage)
    sum /= dim;

  lua_pushnumber(L, sum);
  lua_setfield(L, 1, "output");

  THTensor_(free)(input);
  THTensor_(free)(target);
  lua_pushnumber(L, sum);
  return 1;
}

static int nn_(DistMarginCriterion_updateGradInput)(lua_State *L)
{
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor);
  int sizeAverage = luaT_getfieldcheckboolean(L, 1, "sizeAverage");
  THTensor *gradInput = luaT_getfieldcheckudata(L, 1, "gradInput", torch_Tensor);
  real *input_data;
  real *gradInput_data;
  real *target_data;
  THTensor *target_;
  THTensor *target;
  long nframe, dim;
  long t, d, m;
  real g;

  THArgCheck((input->nDimension == 1) || (input->nDimension == 2), 2, "vector or matrix expected");

  if(input->nDimension == 1) {
    nframe = 1;
    dim = input->size[0];
    target_ = luaT_checkudata(L, 3, torch_Tensor);
    target = THTensor_(new)();
    THTensor_(set)(target, target_);
    THTensor_(resize2d)(target, 1, dim);
  }
  else {
    nframe = input->size[0];
    dim = input->size[1];
    target_ = luaT_checkudata(L, 3, torch_Tensor);
    THArgCheck((target_->nDimension == 2) && (target_->size[0] == nframe) && (target_->size[1] == dim),
               3, "inconsistent target size");
    target = THTensor_(newContiguous)(target_);
  }

  g = (sizeAverage ? 1./((real)dim) : 1.);

  input = THTensor_(newContiguous)(input);
  input_data = THTensor_(data)(input);

  THTensor_(resizeAs)(gradInput, input);
  gradInput_data = THTensor_(data)(gradInput);

  target_data = THTensor_(data)(target);

  for(t = 0; t < nframe; t++) {
    real input_target = THInf;
    int min_idx = -1;
    for (m = 0; m < dim; m++) {
      long target_idx = (long)(target_data[m]-1);
      if (target_idx == -1) break;
      if (input_target > input_data[target_idx]) {
        min_idx = target_idx;
        input_target = input_data[target_idx];
      }
    }
    real gradInput_target = 0;
    for(d = 0; d < dim; d++) {
      int isatarget = 0;
      for(m = 0; m < dim; m++) {
        long target_idx = (long)(target_data[m]-1);
        if (target_idx == -1) break;
        else if(d == target_idx) {
          isatarget = 1;
          break;
        }
      }
      if (isatarget) continue;

      real z = 1 - input_target + input_data[d];
      if(z > 0) {
        gradInput_target -= g;
        gradInput_data[d] = g;
      }
      else
        gradInput_data[d] = 0;
    }
    gradInput_data[min_idx] = gradInput_target;

    input_data += dim;
    gradInput_data += dim;
    target_data += dim;
  }


  THTensor_(free)(input);
  THTensor_(free)(target);
  return 1;
}

static const struct luaL_Reg nn_(DistMarginCriterion__) [] = {
  {"DistMarginCriterion_updateOutput", nn_(DistMarginCriterion_updateOutput)},
  {"DistMarginCriterion_updateGradInput", nn_(DistMarginCriterion_updateGradInput)},
  {NULL, NULL}
};

static void nn_(DistMarginCriterion_init)(lua_State *L)
{
  luaT_pushmetatable(L, torch_Tensor);
  luaT_registeratname(L, nn_(DistMarginCriterion__), "nn");
  lua_pop(L,1);
}

#endif
