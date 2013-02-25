#ifndef TH_GENERIC_FILE
#define TH_GENERIC_FILE "generic/lbfgs.c"
#else

/* use storages to allow copying from different types of Tensors on
   the function evaluations side.  The lbfgs operations are hard
   coded to use doubles for now. lbfgsfloatval_t = double */

int THTensor_(copy_evaluate_start)(THTensor *parameters,
                                   const lbfgsfloatval_t *x, 
                                   const int nParameter)
{
  THDoubleStorage *xs = THDoubleStorage_newWithData((double *)x,nParameter);
  THStorage *ps = THTensor_(storage)(parameters);

 
  /* copy given x (xs) -> parameters (ps) */
  THStorage_(copyDouble)(ps,xs);

  /* only want to free the struct part of the storage not the data */
  xs->data = NULL;
  THDoubleStorage_free(xs);
  return 0;
}

int THTensor_(copy_evaluate_end)(lbfgsfloatval_t *g, 
                                 const THTensor * gradParameter,
                                 const int nParameter)
{
  THDoubleStorage *gs = THDoubleStorage_newWithData((double *)g,nParameter);
  THStorage *gps = THTensor_(storage)(gradParameters);
  
  /* copy gradParameters (gps) -> g (gs) */
#ifdef TH_REAL_IS_FLOAT
  THDoubleStorage_copyFloat(gs,gps);
#else
#ifdef TH_REAL_IS_CUDA
  THDoubleStorage_copyCuda(gs,gps);
#else
  THDoubleStorage_copy(gs,gps);
#endif
#endif
  /* only want to free the struct part of the storage not the data */
  gs->data = NULL;
  THDoubleStorage_free(gs);

  return 0;
}


int THTensor_(copy_init)(lbfgsfloatval_t *x, 
                         THTensor *parameters,
                         const int nParameter) 
{
  THDoubleStorage *xs = THDoubleStorage_newWithData((double *)x,nParameter);
  THStorage *ps = THTensor_(storage)(parameters);

  /* copy given parameters (ps) -> x (xs) */
#ifdef TH_REAL_IS_FLOAT
  THDoubleStorage_copyFloat(xs,ps);
#else
#ifdef TH_REAL_IS_CUDA
  THDoubleStorage_copyCuda(xs,ps);
#else 
  THDoubleStorage_copy(xs,ps);
#endif
#endif 
  /* only want to free the struct part of the storage not the data */
  xs->data = NULL;
  THDoubleStorage_free(xs);

  /* done */
  return 0;
}

#endif
