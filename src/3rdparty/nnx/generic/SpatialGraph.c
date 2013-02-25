#ifndef TH_GENERIC_FILE
#define TH_GENERIC_FILE "generic/SpatialGraph.c"
#else

#ifdef square
#undef square
#endif
#define square(x) ((x)*(x))

static int nn_(SpatialGraph_updateOutput)(lua_State *L)
{
  // get all params
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor);
  int connex = luaT_getfieldcheckint(L, 1, "connex");
  int dist = luaT_getfieldcheckint(L, 1, "dist");
  int norm = luaT_getfieldcheckint(L, 1, "normalize");
  THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_Tensor);

  // dims
  int iwidth = input->size[2];
  int iheight = input->size[1];
  int ichannels = input->size[0];
  int owidth = iwidth;
  int oheight = iheight;
  int ochannels = connex / 2;

  // norm ?
  double normer = (norm == 1) ? 1/sqrt(ichannels) : 1;

  // zero output
  THTensor_(zero)(output);

  // Euclidean distance
  if (dist == 0) {
    // Sum[ (Xi - Xi+1)^2 ]
    int x,y,k;
    for (k=0; k<ichannels; k++) {
      for (y=0; y<oheight; y++) {
        for (x=0; x<owidth; x++) {
          if (x < owidth-1) {
            double temp = square(THTensor_(get3d)(input, k, y, x) - THTensor_(get3d)(input, k, y, x+1));
            THTensor_(set3d)(output, 0, y, x, temp + THTensor_(get3d)(output, 0, y, x));
          }
          if (y < oheight-1) {
            double temp = square(THTensor_(get3d)(input, k, y, x) - THTensor_(get3d)(input, k, y+1, x));
            THTensor_(set3d)(output, 1, y, x, temp + THTensor_(get3d)(output, 1, y, x));
          }
        }
      }
    }

    // Sqrt[ Sum[ (Xi - Xi+1)^2 ] ]
    for (k=0; k<ochannels; k++) {
      for (y=0; y<oheight; y++) {
        for (x=0; x<owidth; x++) {
          THTensor_(set3d)(output, k, y, x, sqrt(THTensor_(get3d)(output, k, y, x)) * normer);
        }
      }
    }

    // Cosine dissimilarity
  } else {
    // add epsilon to input (to get rid of 0s)
    THTensor *inputb = THTensor_(newWithSize3d)(input->size[0], input->size[1], input->size[2]);
    THTensor_(copy)(inputb, input);
    THTensor_(add)(inputb, inputb, 1e-12);

    // Sum[ (Xi * Xi+1) ]
    int x,y,k;
    for (y=0; y<oheight; y++) {
      for (x=0; x<owidth; x++) {
        double norm_A = 0;
        double norm_B = 0;
        double norm_C = 0;
        for (k=0; k<ichannels; k++) {
          norm_A += square(THTensor_(get3d)(inputb, k, y, x));
          if (x < owidth-1) {
            double temp = THTensor_(get3d)(inputb, k, y, x) * THTensor_(get3d)(inputb, k, y, x+1);
            THTensor_(set3d)(output, 0, y, x, temp + THTensor_(get3d)(output, 0, y, x));
            norm_B += square(THTensor_(get3d)(inputb, k, y, x+1));
          }
          if (y < oheight-1) {
            double temp = THTensor_(get3d)(inputb, k, y, x) * THTensor_(get3d)(inputb, k, y+1, x);
            THTensor_(set3d)(output, 1, y, x, temp + THTensor_(get3d)(output, 1, y, x));
            norm_C += square(THTensor_(get3d)(inputb, k, y+1, x));
          }
        }
        if (x < owidth-1) {
          if (norm) {
            THTensor_(set3d)(output, 0, y, x, 1 - THTensor_(get3d)(output, 0, y, x) / (sqrt(norm_A) * sqrt(norm_B)));
          } else {
            THTensor_(set3d)(output, 0, y, x, ichannels - THTensor_(get3d)(output, 0, y, x));
          }
        }
        if (y < oheight-1) {
          if (norm) {
            THTensor_(set3d)(output, 1, y, x, 1 - THTensor_(get3d)(output, 1, y, x) / (sqrt(norm_A) * sqrt(norm_C)));
          } else {
            THTensor_(set3d)(output, 1, y, x, ichannels - THTensor_(get3d)(output, 1, y, x));
          }
        }
      }
    }

    // Cleanup
    THTensor_(free)(inputb);
  }

  return 1;
}

static int nn_(SpatialGraph_updateGradInput)(lua_State *L)
{
  // get all params
  THTensor *input = luaT_checkudata(L, 2, torch_Tensor);
  THTensor *gradInput = luaT_getfieldcheckudata(L, 1, "gradInput", torch_Tensor);
  THTensor *output = luaT_getfieldcheckudata(L, 1, "output", torch_Tensor);
  THTensor *gradOutput = luaT_checkudata(L, 3, torch_Tensor);
  //int connex = luaT_getfieldcheckint(L, 1, "connex");
  int dist = luaT_getfieldcheckint(L, 1, "dist");
  int norm = luaT_getfieldcheckint(L, 1, "normalize");

  // dims
  //int iwidth = input->size[2];
  //int iheight = input->size[1];
  int ichannels = input->size[0];
  int owidth = gradOutput->size[2];
  int oheight = gradOutput->size[1];
  //int ochannels = gradOutput->size[0];

  // norm ?
  double normer = (norm == 1) ? 1/sqrt(ichannels)/sqrt(ichannels) : 1;

  // resize gradInput
  THTensor_(zero)(gradInput);

  // compute derivatives, and backpropagate output error to input
  if (dist == 0) {
    int x,y,k;
    for (k=0; k<ichannels; k++) {
      for (y=0; y<oheight; y++) {
        for (x=0; x<owidth; x++) {
          if (x < owidth-1) {
            double partial_d = THTensor_(get3d)(input, k, y, x) - THTensor_(get3d)(input, k, y, x+1);
            if (partial_d != 0) partial_d /= THTensor_(get3d)(output, 0, y, x);
            partial_d *= THTensor_(get3d)(gradOutput, 0, y, x) * normer;
            THTensor_(set3d)(gradInput, k, y, x, partial_d + THTensor_(get3d)(gradInput, k, y, x));
            THTensor_(set3d)(gradInput, k, y, x+1, -partial_d + THTensor_(get3d)(gradInput, k, y, x+1));
          }
          if (y < oheight-1) {
            double partial_d = THTensor_(get3d)(input, k, y, x) - THTensor_(get3d)(input, k, y+1, x);
            if (partial_d != 0) partial_d /= THTensor_(get3d)(output, 1, y, x);
            partial_d *= THTensor_(get3d)(gradOutput, 1, y, x) * normer;
            THTensor_(set3d)(gradInput, k, y, x, partial_d + THTensor_(get3d)(gradInput, k, y, x));
            THTensor_(set3d)(gradInput, k, y+1, x, -partial_d + THTensor_(get3d)(gradInput, k, y+1, x));
          }
        }
      }
    }

    // Cosine
  } else {
    int x,y,k;
    for (y=0; y<oheight; y++) {
      for (x=0; x<owidth; x++) {
        double sum_A = 0;
        double sum_B = 0;
        double sum_C = 0;
        double sum_AB = 0;
        double sum_AC = 0;

        if (norm) {
          for (k=0; k<ichannels; k++) {
            sum_A += square(THTensor_(get3d)(input, k, y, x));
            if (x < owidth-1) {
              sum_B += square(THTensor_(get3d)(input, k, y, x+1));
              sum_AB += THTensor_(get3d)(input, k, y, x) * THTensor_(get3d)(input, k, y, x+1);
            }
            if (y < oheight-1) {
              sum_C += square(THTensor_(get3d)(input, k, y+1, x));
              sum_AC += THTensor_(get3d)(input, k, y, x) * THTensor_(get3d)(input, k, y+1, x);
            }
          }
        }

        double term1, term2, term3, partial_d;
        double epsi = 1e-12;
        if (x < owidth-1) {
          if (norm) {
            term1 = 1 / ( pow(sum_A, 1/2) * pow(sum_B, 1/2) + epsi );
            term2 = sum_AB / ( pow(sum_A, 3/2) * pow(sum_B, 1/2) + epsi );
            term3 = sum_AB / ( pow(sum_B, 3/2) * pow(sum_A, 1/2) + epsi );
          }
          for (k=0; k<ichannels; k++) {
            if (norm) {
              partial_d = term2 * THTensor_(get3d)(input, k, y, x)
                - term1 * THTensor_(get3d)(input, k, y, x+1);
            } else {
              partial_d = -THTensor_(get3d)(input, k, y, x+1);
            }
            partial_d *= THTensor_(get3d)(gradOutput, 0, y, x);
            THTensor_(set3d)(gradInput, k, y, x, partial_d + THTensor_(get3d)(gradInput, k, y, x));

            if (norm) {
              partial_d = term3 * THTensor_(get3d)(input, k, y, x+1)
                - term1 * THTensor_(get3d)(input, k, y, x);
            } else {
              partial_d = -THTensor_(get3d)(input, k, y, x);
            }
            partial_d *= THTensor_(get3d)(gradOutput, 0, y, x);
            THTensor_(set3d)(gradInput, k, y, x+1, partial_d + THTensor_(get3d)(gradInput, k, y, x+1));
          }
        }
        if (y < oheight-1) {
          if (norm) {
            term1 = 1 / ( pow(sum_A, 1/2) * pow(sum_C, 1/2) + epsi );
            term2 = sum_AC / ( pow(sum_A, 3/2) * pow(sum_C, 1/2) + epsi );
            term3 = sum_AC / ( pow(sum_C, 3/2) * pow(sum_A, 1/2) + epsi );
          }
          for (k=0; k<ichannels; k++) {
            if (norm) {
              partial_d = term2 * THTensor_(get3d)(input, k, y, x)
                - term1 * THTensor_(get3d)(input, k, y+1, x);
            } else {
              partial_d = -THTensor_(get3d)(input, k, y+1, x);
            }
            partial_d *= THTensor_(get3d)(gradOutput, 1, y, x);
            THTensor_(set3d)(gradInput, k, y, x, partial_d + THTensor_(get3d)(gradInput, k, y, x));

            if (norm) {
              partial_d = term3 * THTensor_(get3d)(input, k, y+1, x)
                - term1 * THTensor_(get3d)(input, k, y, x);
            } else {
              partial_d = -THTensor_(get3d)(input, k, y, x);
            }
            partial_d *= THTensor_(get3d)(gradOutput, 1, y, x);
            THTensor_(set3d)(gradInput, k, y+1, x, partial_d + THTensor_(get3d)(gradInput, k, y+1, x));
          }
        }
      }
    }
  }

  return 1;
}

static const struct luaL_Reg nn_(SpatialGraph__) [] = {
  {"SpatialGraph_updateOutput", nn_(SpatialGraph_updateOutput)},
  {"SpatialGraph_updateGradInput", nn_(SpatialGraph_updateGradInput)},
  {NULL, NULL}
};

static void nn_(SpatialGraph_init)(lua_State *L)
{
  luaT_pushmetatable(L, torch_Tensor);
  luaT_registeratname(L, nn_(SpatialGraph__), "nn");
  lua_pop(L,1);
}

#endif
