/*
Copyright ESIEE (2009) 

m.couprie@esiee.fr

This software is an image processing library whose purpose is to be
used primarily for research and teaching.

This software is governed by the CeCILL  license under French law and
abiding by the rules of distribution of free software. You can  use, 
modify and/ or redistribute the software under the terms of the CeCILL
license as circulated by CEA, CNRS and INRIA at the following URL
"http://www.cecill.info". 

As a counterpart to the access to the source code and  rights to copy,
modify and redistribute granted by the license, users are provided only
with a limited warranty  and the software's author,  the holder of the
economic rights,  and the successive licensors  have only  limited
liability. 

In this respect, the user's attention is drawn to the risks associated
with loading,  using,  modifying and/or developing or reproducing the
software by the user in light of its specific status of free software,
that may mean  that it is complicated to manipulate,  and  that  also
therefore means  that it is reserved for developers  and  experienced
professionals having in-depth computer knowledge. Users are therefore
encouraged to load and test the software's suitability as regards their
requirements in conditions enabling the security of their systems and/or 
data to be ensured and,  more generally, to use and operate it in the 
same conditions as regards security. 

The fact that you are presently reading this means that you have had
knowledge of the CeCILL license and that you accept its terms.
*/
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mccodimage.h>
#include <jccodimage.h>
#include <mcutil.h>
#include <jclderiche.h>
#include <lppm2GA.h>

#define SCALE 10
#define HOMO_SIGMA 1
#define HIST_THRESHOLD 0.85
#define FILTER 0

/*****************************************************************************
 * FUNCTION: invertMatrix
 * DESCRIPTION: Computes the inverse of a matrix. 
 * PARAMETERS:
 *    a: matrix to be inverted and keep the result
 *    n: the dimension of the matrix
 * SIDE EFFECTS: None
 * ENTRY CONDITIONS: None
 * RETURN VALUE: 
 *        <0 : fail
 *        >0 : successful completion 
 * EXIT CONDITIONS: None
 * HISTORY:
 *    Created: 7/22/00 by Ying Zhuge
 *
 *****************************************************************************/
int32_t invertMatrix(a,n)
     int32_t n;
     double a[];
{
  int32_t i,j,k,m;
  double w,g,*b;
  
  b= (double*)malloc(n*sizeof(double));
  if (b == NULL)
    return (-2);
  for (k=0; k<=n-1; k++)
  { 
    w=a[0];
    if (fabs(w)+1.0==1.0)
    { 
      free(b);
      return(-2);
    }
    m=n-k-1;
    for (i=1; i<=n-1; i++)
    { 
      g=a[i*n]; b[i]=g/w;
      if (i<=m) b[i]=-b[i];
      for (j=1; j<=i; j++)
	a[(i-1)*n+j-1]=a[i*n+j]+g*b[j];
    }
    a[n*n-1]=1.0/w;
    for (i=1; i<=n-1; i++)
      a[(n-1)*n+i-1]=b[i];
  }
  for (i=0; i<=n-2; i++)
    for (j=i+1; j<=n-1; j++)
      a[i*n+j]=a[j*n+i];
  free(b);
  return(2);
}

/*****************************************************************************
 * FUNCTION: multiMatrix
 * DESCRIPTION: Compute the multipled matrix of two matrixes 
 * PARAMETERS:
 *    a: source matrix of m*n  
 *    b: source matrix of n*k
 *    c: result matrix
 *    m: rows of matrix a
 *    n: cols of matrix a, rows of matrix b
 *    k: cols of matrix b
 * SIDE EFFECTS: None
 * ENTRY CONDITIONS: None
 * RETURN VALUE: 
 *       
 *         
 * EXIT CONDITIONS: None
 * HISTORY:
 *    Created: 7/24/00 by Ying Zhuge
 *
 *****************************************************************************/

int32_t  multiMatrix(a,b,m,n,k,c)
     int32_t m,n,k;
     double a[],b[],c[];
{
  int32_t i,j,l,u;
  for (i=0; i<=m-1; i++)
    for (j=0; j<=k-1; j++)
    {
      u=i*k+j; c[u]=0.0;
      for (l=0; l<=n-1; l++)
	c[u]=c[u]+a[i*n+l]*b[l*k+j];
    }
  return 0;
}

int32_t dericheDerivateurGA(struct xvimage *image, struct xvimage *ga, double alpha) 
{
  int32_t i,j;

  uint8_t *ima = UCHARDATA(image);
  int32_t rs = image->row_size;
  int32_t cs = image->col_size;
  int32_t N = rs * cs;
  double *Im1;    /* image intermediaire */
  double *Im2;    /* image intermediaire */
  double *Imd;    /* image intermediaire */
  double *buf1;   /* buffer ligne ou colonne */
  double *buf2;   /* buffer ligne ou colonne */
  double k;       /* constante de normalisation pour le lisseur */
  double kp;      /* constante de normalisation pour le derivateur */
  double e_a;     /* stocke exp(-alpha) */
  double e_2a;    /* stocke exp(-2alpha) */
  double a1, a2, a3, a4, a5, a6, a7, a8, b1, b2, b3, b4;
  double lmax;
  uint8_t *GA = UCHARDATA(ga);      /* graphe d'arete est suppose deja allouer */

  if (depth(image) != 1) 
  {
    fprintf(stderr, "lderiche: cette version ne traite pas les images volumiques\n");
    exit(0);
  }

  Im1 = (double *)malloc(N * sizeof(double));
  Im2 = (double *)malloc(N * sizeof(double));
  Imd = (double *)malloc(N * sizeof(double));
  buf1 = (double *)malloc(mcmax(rs, cs) * sizeof(double));
  buf2 = (double *)malloc(mcmax(rs, cs) * sizeof(double));
  if ((Im1==NULL) || (Im2==NULL) || (Imd==NULL) || (buf1==NULL) || (buf2==NULL))
  {   fprintf(stderr,"lderiche() : malloc failed\n");
      return(0);
  }
  for (i = 0; i < N; i++) Imd[i] = (double)ima[i];

  e_a = exp(- alpha);
  e_2a = exp(- 2.0 * alpha);
  k = 1.0 - e_a;
  k = k * k / (1.0 + (2 * alpha * e_a) - e_2a);
  kp = 1.0 - e_a;
  kp = - kp * kp / e_a;
  //  kpp = (1.0 - e_2a) / (2 * alpha * e_a);

  /* valeurs de parametres pour filtre lisseur-derivateur */
  a1 = k;
  a2 = k * e_a * (alpha - 1.0);
  a3 = k * e_a * (alpha + 1.0);
  a4 = - k * e_2a;

  a5 = 0.0;
  a6 = kp * e_a;
  a7 = - kp * e_a;
  a8 = 0.0;

  b1 = b3 = 2 * e_a;
  b2 = b4 = - e_2a;

  derichegen(Imd, rs, cs, buf1, buf2, Im1,
	     a1, a2, a3, a4, a5, a6, a7, a8, b1, b2, b3, b4);
  derichegen(Imd, rs, cs, buf1, buf2, Im2,
	     a5, a6, a7, a8, a1, a2, a3, a4, b1, b2, b3, b4);
  
  for(j = 0; j < cs; j++)
    for(i = 0; i < rs - 1; i++){
      //      lmax = ( mcabs(Im1[j*rs+i]) + mcabs(Im1[j*rs+i+1]) / 2); 
      lmax = mcmin(mcabs(Im1[j*rs+i]), mcabs(Im1[j*rs+i+1])); 
      if (lmax <= 255)  
	GA[j * rs + i] = (uint8_t)lmax; 
      else 
	GA[j * rs + i] = 255; 
    }
  for(j = 0; j < cs-1; j++)
    for(i = 0; i < rs; i++){
      //  lmax = (mcabs(Im2[j*rs+i]) + mcabs(Im2[j*rs+i+1]) / 2); 
      lmax = mcmin(mcabs(Im2[j*rs+i]), mcabs(Im2[j*rs+i+1]));
      if (lmax <= 255)
      GA[N + j * rs + i] =(uint8_t)lmax;
      else 
	GA[N + j * rs + i] = 255;
    }
  free(Im1);
  free(Im2);
  free(Imd);
  free(buf1);
  free(buf2);
  return 1;
}

int32_t lpgm2ga(struct xvimage *im, struct xvimage *ga, int32_t param, double alpha)
{
  int32_t i,j;                                /* index muet */
  int32_t rs = rowsize(ga);                   /* taille ligne */ 
  int32_t cs = colsize(ga);                   /* taille colone */
  int32_t N = rs * cs;                        /* taille image */
  uint8_t *F = UCHARDATA(im);        /* composante rouge */
  uint8_t *GA = UCHARDATA(ga);      /* graphe d'arete est suppose deja allouer */ 
  
  /* vérifier que les tailles des diférentes images sont cohérentes */
  switch(param)
  {
  case 0:
  for(j = 0; j < cs; j++)
    for(i = 0; i < rs - 1; i++)
    {
      GA[j * rs + i] = (uint8_t) (mcabs( (int32_t)(F[j*rs+i]) - (int32_t)(F[j*rs+i+1]) ));
    }
  for(j = 0; j < cs-1; j++)
    for(i = 0; i < rs; i++)
    {
      GA[N + j * rs + i] =(uint8_t)(mcabs((int32_t)(F[j*rs+i]) - (int32_t)(F[j*rs+i+rs]))) ; 
    }
  break;
  case 1:
  for(j = 0; j < cs; j++)
    for(i = 0; i < rs - 1; i++)
    {
      GA[j * rs + i] = (uint8_t) (mcmax( (int32_t)(F[j*rs+i]), (int32_t)(F[j*rs+i+1]) ));
    }
   for(j = 0; j < cs-1; j++)
    for(i = 0; i < rs; i++)
    {      
      GA[N + j * rs + i] =(uint8_t)(mcmax((int32_t)(F[j*rs+i]),(int32_t)(F[j*rs+i+rs]))) ;
    }
   break;
  case 2: /* Cas du Deriche, ce n'est pas tout a fait la meilleure */
    /* implementation il faudrait proposer un Deriche specifique   */
    /* aux aretes */ 
    dericheDerivateurGA(im, ga, alpha);
    break;
  }
  return 1;  
}

int32_t lpgm2gafloat(struct xvimage *im, struct xvimage *ga, int32_t param, double alpha)
{
  int32_t i,j;                                /* index muet */
  int32_t rs = rowsize(ga);                   /* taille ligne */ 
  int32_t cs = colsize(ga);                   /* taille colone */
  int32_t N = rs * cs;                        /* taille image */
  float *F = FLOATDATA(im);         /* composante rouge */
  float *GA = FLOATDATA(ga);        /* graphe d'arete est suppose deja allouer */ 
  
  /* vérifier que les tailles des diférentes images sont cohérentes */
  switch(param)
  {
  case 0:
  for(j = 0; j < cs; j++)
    for(i = 0; i < rs - 1; i++)
    {
      GA[j * rs + i] = (float) (mcabs( (float)(F[j*rs+i]) - (float)(F[j*rs+i+1]) ));
    }
  for(j = 0; j < cs-1; j++)
    for(i = 0; i < rs; i++){
      GA[N + j * rs + i] =(float)(mcabs((float)(F[j*rs+i]) - (float)(F[j*rs+i+rs]))) ; 
    }
  break;
  case 1:
  for(j = 0; j < cs; j++)
    for(i = 0; i < rs - 1; i++)
    {
      GA[j * rs + i] = (float) (mcmax( (float)(F[j*rs+i]), (float)(F[j*rs+i+1]) ));
      printf("%lf \t",GA[j * rs + i]);
    }
   for(j = 0; j < cs-1; j++)
    for(i = 0; i < rs; i++)
    {      
      GA[N + j * rs + i] =(float)(mcmax((float)(F[j*rs+i]),(float)(F[j*rs+i+rs]))) ;
    }
   break; 
  case 2: 
    printf("Attention pas Deriche mais min !!\n");
    for(j = 0; j < cs; j++)
      for(i = 0; i < rs - 1; i++)
      {
	GA[j * rs + i] = (float) (mcmin( (float)(F[j*rs+i]), (float)(F[j*rs+i+1]) ));
      }
    for(j = 0; j < cs-1; j++)
      for(i = 0; i < rs; i++)
      {      
	GA[N + j * rs + i] =(float)(mcmin((float)(F[j*rs+i]), (float)(F[j*rs+i+rs]))) ;
      }

    /* Cas du Deriche, ce n'est pas tout a fait la meilleure */
    /* implementation il faudrait proposer un Deriche specifique   */
    /* aux aretes */ 
    //  fprintf(stderr,"Deriche float: not yet implemented\n");
    //exit(0);
    break;
  }
  return 1;  
}

double minDouble(double a, double b)
{
  if(a < b) return a;
  else return b;
}

double maxDouble(double a, double b)
{
  if(a < b) return b;
  else return a;
}

#define ARRONDI 1

int32_t lpgm2gaDouble(struct xvimage *im, struct xvimage *ga, int32_t param, double alpha)
{
  int32_t i,j;                                /* index muet */
  int32_t rs = rowsize(ga);                   /* taille ligne */ 
  int32_t cs = colsize(ga);                   /* taille colone */
  int32_t N = rs * cs;                        /* taille image */
  double *F = DOUBLEDATA(im);         /* composante rouge */
  double *GA = DOUBLEDATA(ga);        /* graphe d'arete est suppose deja allouer */ 
  double f,g;  
  /* vérifier que les tailles des diférentes images sont cohérentes */
  switch(param)
  {
  case 0:
  for(j = 0; j < cs; j++)
    for(i = 0; i < rs - 1; i++)
    {
#ifdef ARRONDI_LPE 
      f = (double)(round(F[j*rs+i]*ARRONDI)) / ARRONDI;
      g = (double)(round(F[j*rs+i+1]*ARRONDI)) / ARRONDI; 
#else
      f =  ARRONDI * F[j*rs+i];
      g =  ARRONDI * F[j*rs+i+1];
#endif 
      GA[j * rs + i] = (mcabs( f - g ));
    }
  for(j = 0; j < cs-1; j++)
    for(i = 0; i < rs; i++){
#ifdef ARRONDI_LPE 
      f = (double)(round(F[j*rs+i]*ARRONDI)) / ARRONDI;
      g = (double)(round(F[j*rs+i+rs]*ARRONDI)) / ARRONDI; 
#else
      f =  ARRONDI * F[j*rs+i];
      g =  ARRONDI * F[j*rs+i+rs];
#endif 
      GA[N + j * rs + i] =(mcabs(f - g)) ; 
    }
  break;
  case 1:
  for(j = 0; j < cs; j++)
    for(i = 0; i < rs - 1; i++)
    {
#ifdef ARRONDI_LPE 
      f = (double)(round(F[j*rs+i]*ARRONDI)) / ARRONDI;
      g = (double)(round(F[j*rs+i+1]*ARRONDI)) / ARRONDI; 
#else
      f =  ARRONDI * F[j*rs+i];
      g =  ARRONDI * F[j*rs+i+1];
#endif 
      GA[j * rs + i] =  (maxDouble(f,g ));
    }
   for(j = 0; j < cs-1; j++)
    for(i = 0; i < rs; i++)
    { 
#ifdef ARRONDI_LPE 
      f = (double)(round(F[j*rs+i]*ARRONDI)) / ARRONDI;
      g = (double)(round(F[j*rs+i+rs]*ARRONDI)) / ARRONDI; 
#else
      f =  ARRONDI * F[j*rs+i];
      g =  ARRONDI * F[j*rs+i+rs];
#endif 
      GA[N + j * rs + i] =(maxDouble(f,g)) ;
    }
   break; 
  case 2: 
    printf("Attention pas Deriche mais min !!\n");
    for(j = 0; j < cs; j++)
      for(i = 0; i < rs - 1; i++)
      {
#ifdef ARRONDI_LPE 
      f = (double)(round(F[j*rs+i]*ARRONDI)) / ARRONDI;
      g = (double)(round(F[j*rs+i+1]*ARRONDI)) / ARRONDI; 
#else
      f = ARRONDI * F[j*rs+i];
      g = ARRONDI * F[j*rs+i+1];
#endif
	GA[j * rs + i] =(minDouble( f,g ));
      }
    for(j = 0; j < cs-1; j++)
      for(i = 0; i < rs; i++)
      {
#ifdef ARRONDI_LPE 
	f = (double)(round(F[j*rs+i]*ARRONDI)) / ARRONDI;
	g = (double)(round(F[j*rs+i+rs]*ARRONDI)) / ARRONDI; 
#else
	f = ARRONDI * F[j*rs+i];
	g = ARRONDI * F[j*rs+i+rs];
#endif
	GA[N + j * rs + i] =(minDouble(f,g)) ;
      }
    break;
  }
  return 1;  
}
 
int32_t lpgm2ga3d(struct xvimage *im, struct xvimage *ga, int32_t param)
{
  int32_t i,j,k;                                /* index muet */
  int32_t rs = rowsize(ga);                   /* taille ligne */ 
  int32_t cs = colsize(ga);                   /* taille colone */
  int32_t ps = rs * cs;                       /* taille d'un plan */
  int32_t ds = depth(ga);                     /* taille plan */
  int32_t N = ps*ds;                          /* taille image */
  uint8_t *F = UCHARDATA(im);       /* composante rouge */
  uint8_t *GA = UCHARDATA(ga);      /* graphe d'arete est suppose deja allouer */
  
  /* vérifier que les tailles des diférentes images sont cohérentes */
  switch(param)
  {
  case 0:
    for(k = 0; k < ds; k++)
      for(j = 0; j < cs; j++)
	for(i = 0; i < rs - 1; i++)
	{
	  GA[k*ps + j*rs + i] = (uint8_t)(mcabs( (int32_t)(F[k*ps+j*rs+i]) - (int32_t)(F[k*ps+j*rs+i+1]) ));
	  // printf("GA[%d] = %d \n",k*ps + j*rs + i,GA[k*ps + j*rs + i]); 
	}
    for(k =0; k < ds; k++)
      for(j = 0; j < cs-1; j++)
	for(i = 0; i < rs; i++)
	{
	  GA[N + k*ps + j * rs + i] =(uint8_t)(mcabs((int32_t)(F[k*ps + j*rs+i]) - (int32_t)(F[k*ps + j*rs+i+rs]))) ;
	  // printf("GA[%d] = %d \n",N+ k*ps + j*rs + i,GA[N+k*ps + j*rs + i]);
	}
    for(k =0; k < ds-1; k++)
      for(j = 0; j < cs; j++)
	for(i = 0; i < rs; i++)
	{
	  GA[2*N + k*ps + j * rs + i] =(uint8_t)(mcabs((int32_t)(F[k*ps + j*rs+i]) - (int32_t)(F[k*ps+j*rs+i+ps]))) ;
	  // printf("GA[%d] = %d \n",2*N+ k*ps + j*rs + i,GA[2*N+k*ps + j*rs + i]);
	}
    break;
  case 1:
    for(k = 0; k < ds; k++)
      for(j = 0; j < cs; j++)
	for(i = 0; i < rs - 1; i++)
	{
	  GA[k*ps + j*rs + i] = (uint8_t)(mcmax( (int32_t)(F[k*ps+j*rs+i]), (int32_t)(F[k*ps+j*rs+i+1]) ));
	  // printf("GA[%d] = %d \n",k*ps + j*rs + i,GA[k*ps + j*rs + i]); 
	}
    for(k =0; k < ds; k++)
      for(j = 0; j < cs-1; j++)
	for(i = 0; i < rs; i++)
	{
	  GA[N + k*ps + j * rs + i] =(uint8_t)(mcmax((int32_t)(F[k*ps + j*rs+i]) , (int32_t)(F[k*ps + j*rs+i+rs]))) ;
	  // printf("GA[%d] = %d \n",N+ k*ps + j*rs + i,GA[N+k*ps + j*rs + i]);
	}
    for(k =0; k < ds-1; k++)
      for(j = 0; j < cs; j++)
	for(i = 0; i < rs; i++)
	{
	  GA[2*N + k*ps + j * rs + i] =(uint8_t)(mcmax((int32_t)(F[k*ps + j*rs+i]) , (int32_t)(F[k*ps+j*rs+i+ps]))) ;
	  // printf("GA[%d] = %d \n",2*N+ k*ps + j*rs + i,GA[2*N+k*ps + j*rs + i]);
	}
  }
  return 1;  
}

int32_t lpgm2ga4d(struct xvimage4D *im, struct GA4d * ga, int32_t param)
{
  int32_t i,j,k,l;                            /* index muet */
  int32_t rs = rowsize(ga);                   /* taille ligne */ 
  int32_t cs = colsize(ga);                   /* taille colone */
  int32_t ps = rs * cs;                       /* taille d'un plan */
  int32_t ds = depth(ga);                     /* nbre de plans */
  int32_t vs = ps*ds;                         /* taille volume */
  int32_t ss = seqsizeGA(ga);
  int32_t N = ss * vs;
  uint8_t **F;                      /* composante rouge */
  uint8_t *GA = UCHARDATA(ga);      /* graphe d'arete est suppose deja allouer */
  
  // printf("La taile de la sequence %d\n",ss);
  F = (uint8_t **)malloc(sizeof(char *) * ss);
  for(j = 0; j < ss; j++)
    F[j] = UCHARDATA(im->frame[j]);
  /* vérifier que les tailles des diférentes images sont cohérentes */
  memset(GA,0,N*4);
  switch(param)
  {
  case 0:

    for(l = 0; l < ss; l++)
      for(k = 0; k < ds; k++)
	for(j = 0; j < cs; j++)
	  for(i = 0; i < rs - 1; i++){
	    GA[l*vs + k*ps + j*rs + i] = (uint8_t)(mcabs( (int32_t)(F[l][k*ps+j*rs+i]) - (int32_t)(F[l][k*ps+j*rs+i+1]) ));
	    //    printf("x-> F[(%d,%d,%d,%d)] = %d\n", i, j, k, l, GA[l*vs + k*ps + j*rs + i]);
	  }
    for(l = 0; l < ss; l++)
      for(k =0; k < ds; k++)
	for(j = 0; j < cs-1; j++)
	  for(i = 0; i < rs; i++)
	  {
	    GA[N+ l*vs + k*ps + j * rs + i] =
	      (uint8_t)(mcabs((int32_t)(F[l][k*ps + j*rs+i]) - (int32_t)(F[l][k*ps + j*rs+i+rs]))) ;
	    // printf("y-> F[(%d,%d,%d,%d)] = %d\n", i, j, k, l, GA[N+ l*vs + k*ps + j*rs + i]);
	  }
    for(l = 0; l < ss; l++)
      for(k =0; k < ds-1; k++)
	for(j = 0; j < cs; j++)
	  for(i = 0; i < rs; i++) {
	    GA[2*N + l*vs + k*ps + j * rs + i] =(uint8_t)(mcabs((int32_t)(F[l][k*ps + j*rs+i]) - 
								    (int32_t)(F[l][k*ps + j*rs +i + ps]))) ;
    	    //printf("->z F[(%d,%d,%d,%d)] = %d\n", i, j, k, l, GA[l*vs + k*ps + j*rs + i]);
	  }

    for(l = 0; l < ss-1; l++)
      for(k =0; k < ds; k++)
	for(j = 0; j < cs; j++)
	  for(i = 0; i < rs; i++){
	    GA[3*N + l*vs + k*ps + j * rs + i] =(uint8_t)(mcabs((int32_t)(F[l][k*ps + j*rs+i]) - 
								    (int32_t)(F[l+1][k*ps+j*rs+i]))) ;
	    // printf("->t F[(%d,%d,%d,%d)] = %d\n", i, j, k, l, GA[3*N+ l*vs + k*ps + j*rs + i]);
	  }
    break; 
  case 1:
      for(l = 0; l < ss; l++)
      for(k = 0; k < ds; k++)
	for(j = 0; j < cs; j++)
	  for(i = 0; i < rs - 1; i++)
	    GA[l*vs + k*ps + j*rs + i] = (uint8_t)(mcmax((int32_t)(F[l][k*ps+j*rs+i]),
							   (int32_t)(F[l][k*ps+j*rs+i+1]) ));
    
    for(l = 0; l < ss; l++)
      for(k =0; k < ds; k++)
	for(j = 0; j < cs-1; j++)
	  for(i = 0; i < rs; i++)
	    GA[N+ l*vs + k*ps + j * rs + i] =(uint8_t)(mcmax((int32_t)(F[l][k*ps + j*rs+i]),
								 (int32_t)(F[l][k*ps + j*rs+i+rs]))) ;
	    
    for(l = 0; l < ss; l++)
      for(k =0; k < ds-1; k++)
	for(j = 0; j < cs; j++)
	  for(i = 0; i < rs; i++) 
	    GA[2*N + l*vs + k*ps + j * rs + i] =(uint8_t)(mcmax((int32_t)(F[l][k*ps + j*rs+i]), 
								    (int32_t)(F[l][k*ps + j*rs+i+ps]))) ;
    
    for(l = 0; l < ss-1; l++)
      for(k =0; k < ds; k++)
	for(j = 0; j < cs; j++)
	  for(i = 0; i < rs; i++)
	    GA[3*N + l*vs + k*ps + j * rs + i] =(uint8_t)(mcmax((int32_t)(F[l][k*ps + j*rs+i]), 
								    (int32_t)(F[l+1][k*ps+j*rs+i]))) ;
    break;
  }
  return 1;  
}


int16_t (**sphere_points)[3];
/* Construit un graphe d'arete 2D 4-connexe a partir d'une image rgb */
/* Chaque arete a pour valeur l'inverse de la composante homogénéité */
/* de l'afinite définie ds :                                         */
/* "Vectorial Scale based fuzzy connected image segmentation:        */
/* theory, algorithms and applications" - Ying Zhuge, Jayaram K.     */
/* Udupa and Punam K. Saha                                           */
int32_t laffinitynetwork(struct xvimage *r, struct xvimage *v, struct xvimage *b, struct xvimage *ga)
{
  int32_t tti1,tti2,i,j,k,l,xx,yy,x,y;
  int32_t feature_thr[3];
  int32_t homo_sigma;
  int32_t ***ppptti1;
  int32_t **histogram;
  int32_t diff_value_max[3];
  int32_t hist_sum[3];
  int32_t pow_value[3];
  double anisotropy_row, anisotropy_col,tt1,tt2, mask_total;
  double homogeneity_cov[3][3];
  double matrixA[1][3], matrixB[1][3],matrixC[3][1],result;
  float  **homogeneity_map, *scale_map;
  // int16_t (**sphere_points)[3];
  int32_t *sphere_no_points;
  uint8_t *image[3];                /*tableau des 3 bandes */
  int32_t largest_density_value;
  int32_t count;
  double feature_mean[3];                 /* moyenne des 3 bandes */
  int32_t rs = rowsize(ga);                   /* taille ligne */ 
  int32_t cs = colsize(ga);                   /* taille colone */
  int32_t N = rs * cs;                        /* taille image */
  uint8_t *x_affinity = UCHARDATA(ga);
  uint8_t *y_affinity = &x_affinity[N];
  uint8_t *scale_image;
  image[0] = UCHARDATA(r);
  image[1] = UCHARDATA(v);
  image[2] = UCHARDATA(b);
 
  
  // calcul de la moyenne sur chaque bande et de la valeur max
#ifdef DEBUG
  printf(" Calcul moyenne et max des bandes \n");
#endif 
  largest_density_value = 0;
  for (i = 0; i < 3; i++)
  {
    feature_mean[i] = 0;
    for(j = 0;j < N; j++)
    {
      feature_mean[i] += image[i][j];
      if(image[i][j]>largest_density_value)
	largest_density_value = image[i][j];
    }
    feature_mean[i] = feature_mean[i]/N;
  }
  printf("les features mean r %f v %f, b %f et largest density value : %d : \n", feature_mean[0], feature_mean[1] , feature_mean[2], largest_density_value);
  // definition of the masks
  /***********************************************/
  tti1 = 2 * (SCALE + 5);
  ppptti1 = (int32_t ***) malloc(tti1 * sizeof(int32_t **));
  
  if (ppptti1 == NULL)
  { printf("laffinitynetwork: erreur de malloc\n"); exit(0);}

  ppptti1[0] = (int32_t **) malloc(tti1 * tti1 * sizeof(int32_t *));

  if (ppptti1[0] == NULL)
  {printf("laffinitynetwork: erreur de malloc\n"); exit(0);}
  
  for (i = 0; i < tti1; i++)
    ppptti1[i] = ppptti1[0] + i * tti1;
  ppptti1[0][0] = (int32_t *) malloc(tti1 * tti1 * tti1 * sizeof(int32_t));
  
  if (ppptti1[0][0] == NULL)
  {   printf("laffinitynetwork: erreur de malloc\n");  exit(0);}
  for (i = 0; i < tti1; i++)
    for (j = 0; j < tti1; j++)
      ppptti1[i][j] = ppptti1[0][0] + (i * tti1 + j) * tti1;
  
  for (i = 0; i < tti1; i++)
    for (j = 0; j < tti1; j++)
      for (k = 0; k < tti1; k++)
	ppptti1[i][j][k] = 0;

  sphere_no_points = (int32_t *) malloc((SCALE + 1) * sizeof(int32_t));
  if (sphere_no_points == NULL)
  { printf("laffinitynetwork: erreur de malloc\n");  exit(0);}
  
  sphere_points = (void *) malloc((SCALE + 1) * sizeof(void *));
  if (sphere_points == NULL) 
  { printf("laffinitynetwork: erreur de malloc\n");  exit(0);}
  tti1 = SCALE + 5; 
  anisotropy_row = 1;
  anisotropy_col = 1;

  /* Calcul de masques correspondants à des cercles de rayon k autour centre en (0,0) */
  /* pour un rayon croissant des disques */
  for (k = 0; k <= SCALE; k++)
  {
    /*		  printf("Computing circle  %d, ", k);
		  fflush(stdout);
    */		  
    sphere_no_points[k] = 0;
    for (j = -k - 2; j <= k + 2; j++)
      for (l = -k - 2; l <= k + 2; l++)
	if (ppptti1[tti1][tti1 + j][tti1 + l] == 0)
	{
	  tt1 = sqrt(pow(((double) j) * anisotropy_row, 2.0)
		     + pow(((double) l) * anisotropy_col, 2.0));
	  if (tt1 <= ((double) k) + 0.5)
	  {
	    sphere_no_points[k] = sphere_no_points[k] + 1;
	    ppptti1[tti1][tti1 + j][tti1 + l] = 2;
	  }
	  //	  printf("sphere no point de %ds = %d \n", k, sphere_no_points[k]);
	}
    
    sphere_points[k] = (void *) malloc(3 * sphere_no_points[k] * sizeof(int32_t));
    
    if (sphere_points[k] == NULL)
    { printf("laffinitynetwork: erreur de malloc\n");  exit(0);}
    
    tti2 = 0;
    for (j = -k - 2; j <= k + 2; j++)
      for (l = -k - 2; l <= k + 2; l++)
	if (ppptti1[tti1][tti1 + j][tti1 + l] == 2)
	{
	  ppptti1[tti1][tti1 + j][tti1 + l] = 1;
	  sphere_points[k][tti2][0] = 0;
	  sphere_points[k][tti2][1] = j;
	  sphere_points[k][tti2][2] = l;
	  tti2 = tti2 + 1;
	  //	  printf("sphere point de %ds = %d \n", k, sphere_points[k][tti2-1][2]);
	}
  }
  
  // printf("\n");
  fflush(stdout);
  free(ppptti1[0][0]);
  free(ppptti1[0]);
  free(ppptti1);
  mask_total = 0.0;
  
  /**********************************************************************************/
  /*----to compute the histogram for each feature and the threshold for true edge---*/
  histogram = (int32_t **)malloc(3*sizeof(int32_t *));
  histogram[0] = (int32_t *)malloc(3*(largest_density_value+1)*sizeof(int32_t));
  if(histogram == NULL || histogram[0] == NULL)
  { printf("laffinitynetwork: erreur de malloc\n");  exit(0);}
  for(i = 0;i<3;i++)
    histogram[i] = histogram[0]+i*(largest_density_value+1);
  for (i=0;i<3;i++)
  {
    diff_value_max[i] = 0;
    for(j=0;j<=largest_density_value;j++)
      histogram[i][j] = 0;
  }
  // parcourt des aretes horizontales
  for (j=0;j<cs;j++)
    for (k=0;k<rs-1;k++)
    {
      xx = k+1;
      yy = j;
      for (l = 0; l < 3; l++)
      {	 
	tti1 = mcabs(image[l][j*rs+k] - image[l][yy*rs+xx]);		
	if(tti1>diff_value_max[l])
	  diff_value_max[l] = tti1;
	histogram[l][tti1]++;
      }
    }
  // parcourt des aretes verticales
  for (j=0;j<cs-1;j++)
    for (k=0;k<rs;k++)
    {
      xx = k;
      yy = j+1;
      for (l = 0; l < 3; l++)
      {	
	tti1 = mcabs(image[l][j*rs+k] - image[l][yy*rs+xx]);
	if(tti1>diff_value_max[l])
	  diff_value_max[l] = tti1;
	histogram[l][tti1]++;
      }
    }
  for(i = 0; i < largest_density_value; i++)
  {
    printf("dif = % d histo r %d histo v %d histo b %d \n",i, histogram[0][i],  histogram[1][i],  histogram[2][i] );
  }
  
  // computation of the threshold for each feature. First intensity level such that 90/100 of the pixels have an intensity 
  // inferior to that threshold (for each band)
  
  for (i=0;i<3;i++)
  {
    hist_sum[i] = 0;
    for(j=0;j<largest_density_value;j++)
      hist_sum[i] = hist_sum[i] + histogram[i][j];
  }
  for(i = 0;i<3;i++)
  {
    for(j=0;j<largest_density_value;j++)
    {
      tti1 = 0;
      feature_thr[i] = (double)j;
      for(k=0;k<=j;k++)
	tti1 = tti1+histogram[i][k];
      if (((double)tti1 /(double) hist_sum[i])>=HIST_THRESHOLD)
	break;
    }
  }
  printf("seuil r = %d v %d b %d\n", feature_thr[0], feature_thr[1], feature_thr[2]);
  printf("Histogram threshold computation is done \n");
  homo_sigma = mcmax( feature_thr[0] ,mcmax(feature_thr[1] ,feature_thr[2] ));
  homo_sigma = 49;//homo_sigma * homo_sigma;
  /*******************************************************************************************/
  /*******************************************************************************************/
  /*------------To computer the homogeneity covariance matrix--------------------------------*/
  //computation of the covariance matrix between the homogeneity of the different features en 3D a ramener en 2D
  for (x = 0;x < 3; x++)
    for(y = x;y < 3; y++)
    {
      homogeneity_cov[x][y] = 0;
      tt1 = tt2 = 0;
      count = 0.0001;   // pr eviter les divisions par 0 ...
      // pour les aretes horizontales
	for (j = 0;j< cs; j++)
	  for (k = 0;k < rs-1; k++)
	  {
	    yy = j; xx = k+1;
	    // Pourquoi ce test ??
	    //	    if( (image[0][j*rs+k]>feature_mean[0]) 
	    //	&& (image[0][yy*rs+xx] >feature_mean[0]))
	    {
	      tt1 =(double) (image[x][j*rs+k] - image[x][yy*rs+xx]);
	      tt2 =(double) (image[y][j*rs+k] - image[y][yy*rs+xx]);
	    }	    
	    if((fabs(tt1) <= feature_thr[x]) && (fabs(tt2) <= feature_thr[y]))
	    {
	      homogeneity_cov[x][y] += tt1*tt2;
	      count = count+1;
	    }
	  }	
	// pour les aretes verticales
	for (j = 0;j< cs-1; j++)
	  for (k = 0;k < rs; k++)
	  {
	    yy = j+1; xx = k;
	    // if(image[0][j*rs+k]>feature_mean[0] 
	    //	 &&image[0][yy*rs+xx] >feature_mean[0])
	    {
	      tt1 =(double) (image[x][j*rs+k] - image[x][yy*rs+xx]);
	      tt2 =(double) (image[y][j*rs+k] - image[y][yy*rs+xx]);
	    }	      
	    if((fabs(tt1) <= feature_thr[x]) && (fabs(tt2) <= feature_thr[y]))
	    {
	      homogeneity_cov[x][y] += tt1*tt2;
	      count=count+1;
	    }
	  }
	homogeneity_cov[x][y] = homogeneity_cov[x][y]/(double)count;
    }
  // la matrice de covariance est symetrique
  for (x = 0;x < 3; x++)
    for(y = 0;y < x; y++)
      homogeneity_cov[x][y] = homogeneity_cov[y][x];
  // la matrice doit etre inversible
  for (x = 0;x < 3; x++)
    for(y = 0;y < 3; y++)
    {
      homogeneity_cov[x][y] = homo_sigma*homogeneity_cov[x][y];
      if(homogeneity_cov[x][x] == 0) homogeneity_cov[x][x] = 1;
      printf("covariance matrix de %d %d = %f \n", x,y, homogeneity_cov[x][y]  );
    }
  printf("Homogeneity covariance matrix computation is done \n");
  
  i = invertMatrix(homogeneity_cov, 3);
  if(i<0)
  { printf("laffinitynetwork: matrice de covariance non inversible !\n"); exit(0);}
  printf("Matrice inversee\n");
  feature_thr[0] = 255;
  feature_thr[1] = 255;
  feature_thr[2] = 255;
  
  /********************************************************************************************/
  homogeneity_map = (float **)malloc(sizeof(float *));
  tti2 = 1;
  for(i = 0;i<3;i++)
    tti2 = tti2 * (feature_thr[i]+1);
  homogeneity_map[0] = (float*)malloc(tti2*sizeof(float));
  if((homogeneity_map == NULL)||(homogeneity_map[0] == NULL))
  { printf("laffinitynetwork: homogeneitymap erreur de malloc \n"); exit(0);}
  memset(homogeneity_map[0],0,tti2*sizeof(float));
  scale_map = (float *)malloc(tti2*sizeof(float));
  if(scale_map == NULL)
  { printf("laffinitynetwork: scale map erreur de malloc\n"); exit(0);}
  memset(scale_map,0,tti2*sizeof(float));
  for(i = 0;i<3;i++)
  {
    pow_value[i] = 0; tti1 = 1;
    for(j=0;j<=i-1;j++)
      tti1 = tti1*(feature_thr[j]+1); 
    pow_value[i] = tti1;
  }
  
  //---------------------- Multi_variate Gussain Model-----------------------
  // computation of the homogeneity based affinity. To each vector x corresponding to the difference between two adjacent 
  // pixels it associates the value of the homogeneity based affinity
  // calcul d'une lookup table pour l'expression  exp(-1/2(x^t \sigma x))
  
      
  tti1 = 1;
  for(i = 0;i<3;i++)
    tti1 = tti1*(feature_thr[i]+1);
  for(i = 0;i<tti1;i++)
  {
    k = i;
    for(j=0;j<3;j++)
    {
      matrixA[0][j] =  (k % (feature_thr[j]+1));
      matrixC[j][0] =  matrixA[0][j];
      k = k/(feature_thr[j]+1);
    }
    multiMatrix(matrixA,homogeneity_cov,1,3,3,matrixB);
    multiMatrix(matrixB,matrixC,1,3,1,&result);	  
    tti2 = 0;
    for(j=0;j<3;j++)
      tti2 = tti2 + matrixA[0][j]*pow_value[j];
    homogeneity_map[0][tti2] = (float) exp(-0.5*result);
    if(  (homogeneity_map[0][tti2] < 0) || (homogeneity_map[0][tti2] > 1))
      printf("Valeur non valide pour homogeneity map !!! %f \n",homogeneity_map[0][tti2]);
    //  printf("homogeneity_map %f \n", homogeneity_map[0][tti2]);
    // scale_map[tti2] = (float)exp(-0.5*result/9);
    scale_map[tti2] = (float)exp(-0.5*result);
    // printf("scale_map %f \n", scale_map[tti2]);
  }
  
  printf("Homogeneity_map and scale_map computation is done \n");
    
  printf("les feature thr %d %d %d\n", feature_thr[0], feature_thr[1], feature_thr[2]);
  compute_scale(image, &scale_image, scale_map, sphere_no_points, /*sphere_points,*/N,rs,cs,feature_mean,feature_thr, pow_value );
  // printf("Re scale image de 2 vaut %d \n", scale_image[2]);
  
  //printf("Scale computation is done \n");
  //  time(&t2);
  //  printf("scale computation last:%f seconds\n",difftime(t2,t1)); 

  // based on scale, computing filtering image. when it is done, free memory occupied by data array (pas necessaire ds le cas du veritable scale base case ?...)
  /*
    filter_image8 = (uint8_t **)malloc(3*sizeof(uint8_t*));
    filter_image8[0] = (uint8_t *)malloc(3*N*sizeof(char));
  if(filter_image8 ==NULL||filter_image8[0]==NULL)
  { printf("laffinitynetwork: erreur de malloc\n"); exit(0);}
  for (i = 0;i< FEATURES;i++)
    filter_image8[i] =  filter_image8[0] + i*volume_size;
  compute_filter();
  */
  // printf("av calcule material \n");
  // compute_material();  
  // compute_homogeneity(); 
  /* le filtrage n'a servi a rien*/
  //filter_image8 = image;
  compute_homogeneitysb(image, feature_mean, x_affinity, y_affinity, scale_image, sphere_no_points,/* sphere_points,*/ feature_thr, homogeneity_map, N, rs ,cs, pow_value);
  //compute_material();
  // liberer la memoire !!!
  
  for(k = 0; k < SCALE;k++)
    free(sphere_points[k]);
  free(sphere_no_points);
  free(sphere_points);
  free(histogram[0]);
  free(histogram);
  
  free(homogeneity_map[0]);
  free(homogeneity_map);
  free(scale_image);
  printf("av calcu;le de affinite\n");
  return 1;
}

//#define MAX_NORM 1

/* Construit un graphe d'arete 2D 4-connexe a partir d'une image rgb */
/* Chaque arete correspond soit au max des differences d'intensite   */
/* entre ses extremites sur  les 3  canaux                           */
/* soit a la norme euclidienne entre les vecteurs couleurs des deux  */
/* pixels extremites                                                 */
int32_t lppm2ga(struct xvimage *r, struct xvimage *v, struct xvimage *b, struct xvimage *ga, int32_t param)
{
  int32_t i,j;                                /* index muet */
  int32_t rs = rowsize(ga);                   /* taille ligne */ 
  int32_t cs = colsize(ga);                   /* taille colone */
  int32_t N = rs * cs;                        /* taille image */
  uint8_t *R = UCHARDATA(r);        /* composante rouge */
  uint8_t *V = UCHARDATA(v);        /* composante verte */
  uint8_t *B = UCHARDATA(b);        /* composante bleue */
  uint8_t *GA = UCHARDATA(ga);      /* graphe d'arete est suppose deja allouer */
  
  /* vérifier que les tailles des diférentes images sont cohérentes */
  switch(param)
  {
  case 0:
  for(j = 0; j < cs; j++)
    for(i = 0; i < rs - 1; i++)
    {
      GA[j * rs + i] = (uint8_t)(mcmax(mcabs( (int32_t)(B[j*rs+i]) - (int32_t)(B[j*rs+i+1]) ),
			   mcmax(mcabs( (int32_t)(R[j*rs+i]) - (int32_t)(R[j*rs+i+1]) ),
			       mcabs( (int32_t)(V[j*rs+i]) - (int32_t)(V[j*rs+i+1])))));
    }
  for(j = 0; j < cs-1; j++)
    for(i = 0; i < rs; i++)
    {
      GA[N + j * rs + i] =(uint8_t)(mcmax(mcabs((int32_t)(B[j*rs+i]) - (int32_t)(B[j*rs+i+rs])),
			       mcmax(mcabs( (int32_t)(R[j*rs+i]) - (int32_t)(R[j*rs+i+rs]) ),
				   mcabs( (int32_t)(V[j*rs+i]) - (int32_t)(V[j*rs+i+rs])))));
    }
  break;
  case 1:
  for(j = 0; j < cs; j++)
    for(i = 0; i < rs - 1; i++)
    {
      GA[j * rs + i] = (uint8_t)( 0.57 * sqrt ( 
					   ( (double)(B[j*rs+i]) - (double)(B[j*rs+i+1])) * ( (double)(B[j*rs+i]) - (double)(B[j*rs+i+1])) +  
					   ( (double)(R[j*rs+i]) - (double)(R[j*rs+i+1])) * ( (double)(R[j*rs+i]) - (double)(R[j*rs+i+1])) +  
					   ( (double)(V[j*rs+i]) - (double)(V[j*rs+i+1])) * ( (double)(V[j*rs+i]) - (double)(V[j*rs+i+1]))
					   ));
      // if (GA[j * rs + i] < 8 )  GA[j * rs + i] = 0;
    }
   for(j = 0; j < cs-1; j++)
    for(i = 0; i < rs; i++)
    {      
      GA[N + j * rs + i] = (uint8_t)( 0.57 * sqrt ( 
					   ( (double)(B[j*rs+i]) - (double)(B[j*rs+i+rs]))* ( (double)(B[j*rs+i]) - (double)(B[j*rs+i+rs])) +  
					   ( (double)(R[j*rs+i]) - (double)(R[j*rs+i+rs]))* ( (double)(R[j*rs+i]) - (double)(R[j*rs+i+rs])) +  
					   ( (double)(V[j*rs+i]) - (double)(V[j*rs+i+rs]))* ( (double)(V[j*rs+rs]) - (double)(V[j*rs+i+rs]))
					   ));
      //  if (GA[N + j * rs + i] < 8)  GA[N + j * rs + i] = 0;
    }
   break;
  case 2:
   laffinitynetwork(r, v, b, ga);
  }
  return 1;
}



/*****************************************************************************
 * FUNCTION: compute_scale
 * DESCRIPTION: Computes the scale values for the entire volume anf store in the 
 *        scale-image array.
 * PARAMETERS: None
 * SIDE EFFECTS: 
 * FUNCTIONS CALEED: None
 * ENTRY CONDITIONS: 1) scale_map array is alloted and 
 *           proper values are assigned
 * RETURN VALUE: None
 * EXIT CONDITIONS: Compute scale values
 * HISTORY:
 *  Created: 02/24/00
 *  Modified:07/25/00 extend to 24 bits color image by Ying Zhuge 
 *
 *****************************************************************************/
int32_t compute_scale(uint8_t **image, uint8_t **scale_image, float *scale_map, int32_t *sphere_no_points, /*int16_t ***sphere_points,*/ int32_t N, int32_t rs, int32_t cs, double * feature_mean, int32_t *feature_thr, int32_t * pow_value)
{
  int32_t i, j, k, x, y, xx, yy, row, col;
  int32_t flag, tti1, edge_flag;
  double count_obj, count_nonobj;
  double tolerance = 15;
  int32_t mean[3],temp[3];
  double mask_f[3];
  double d,c;

  (*scale_image) = (uint8_t *)malloc(N * sizeof(uint8_t));
  printf("les feature thr %d %d %d\n", feature_thr[0], feature_thr[1], feature_thr[2]);
  
  /* pour tout pt de l'image */
  for (row = 0; row < cs; row++)
    for (col = 0; col < rs; col++)  
    {
      {
	flag = 0;
	edge_flag = 0;
	// Attention on n'effectue pas de filtrage, c'est tout l'interet de la méthode  	
	//------------------------------mean filter----------------------
	for(i=0;i<3;i++)
	  mask_f[i] = 0;
	k = 0;
	c = 0;
	for (yy = -FILTER; yy <= FILTER; yy++)
	  for (xx = -FILTER; xx <= FILTER; xx++)
	  {
	    x = xx + col;
	    y = yy + row;
	    d = 1/sqrt(xx*xx + yy*yy );
	    for(i=0;i<3;i++)
	      if (x >= 0 && y >= 0  
		  && x < rs && y < cs)
		mask_f[i] += d * image[i][y*rs+x];
	      else
		mask_f[i] += d * image[i][row*rs+col];
	    c += d;
	  }
	for(i=0;i<3;i++)
	  //	  mean[i] = (int32_t)(mask_f[i] / c);
	  mean[i] = image[i][row*rs +col];
	for (k = 1; k < SCALE && !flag; k++)
	{
	  count_obj = 0;
	  count_nonobj = 0;
	  //printf("echelle de valeur %d\n",k);
	  for (i = 0; i < sphere_no_points[k]; i++) 
	  {
	    x = col + sphere_points[k][i][2];
	    y = row + sphere_points[k][i][1];
	    if (x < 0 || x > rs)
	      x = col;
	    if (y < 0 || y > cs)
	      y = row;
	    tti1 = 0;
	    edge_flag = 0;
	    for(j=0;j<3;j++)
	    {	      
	      temp[j] = mcabs((int32_t)image[j][y*rs + x] - (int32_t)mean[j]/* image[j][row*rs + col]*/ );
	      /* if((row == 137) && (col == 178))
		 printf("x %d y %d, k %d et %d\n ",x,y,k,temp[j]);*/
	      tti1 = tti1+(temp[j])*pow_value[j];
	      if(temp[j] > feature_thr[j])
		edge_flag = 1;
	    }
	    // printf("Avant utilisation de la scale_map\n");
	    if(!edge_flag)
	    {	 
	      /*	      if((row == 137) && (col == 178))
			      printf(" par ici\n");*/
	      count_obj =  count_obj + scale_map[tti1];
	      count_nonobj = count_nonobj + 1.0 - scale_map[tti1];
	    }
	    else
	    {	      
	      count_obj = count_obj ;
	      count_nonobj = count_nonobj + 1.0 ;
	    }
	  }
	  if((row == 137) && (col == 178))
	    printf("count obj %f count_nonobj %f valeur d'homogeneite %f ",count_obj,count_nonobj,  count_nonobj /  (count_nonobj + count_obj) );
	  if (100.0 * count_nonobj >= tolerance * (count_nonobj + count_obj))
	  {
	  
	    (*scale_image)[row*rs+col] = mcmax(1, k-1);
	    flag = 1;
	  }
	}
	if (!flag)
	  (*scale_image)[row*rs + col] = k-1;
	if((row == 137) && (col == 178))
	  printf("scale de %d %d vaut %d \n", row, col, (*scale_image)[row*rs + col]);
	fflush(stdout);
      }
    } 
  if(scale_map)
    free(scale_map);

    //  printf("scale image de 2 %d \n", (*scale_image)[2]);
  printf("\rScale computation is done.     \n"); fflush(stdout);
  return 1;
}



void compute_homogeneitysb(uint8_t ** image, double *feature_mean, uint8_t *x_affinity, uint8_t *y_affinity, uint8_t* scale_image, int32_t *sphere_no_points, /*int16_t ***sphere_points,*/ int32_t *feature_thr, float **homogeneity_map, int32_t N, int32_t rs, int32_t cs, int32_t * pow_value)
{

  int32_t i, j, k, tti1, xx, yy, x1, y1, x, y, iscale;
  double tt1, tt2, inv_k, count;
  int32_t col, row, col1, row1;
  int32_t temp[3];
  double weight[SCALE][SCALE];  
  int32_t edge_flag;
  double val;
  
  for(i = 0;i<SCALE;i++)
    for(j = 0;j<SCALE;j++)
      weight[i][j] = 0;

  for(i = 1;i <= SCALE;i++)
  {
    tt1 = (double)i*0.5;
    tt2 = -0.5 / pow(tt1, 2.0);      
    for(j = 0;j<i;j++)
    {
      inv_k = exp(tt2 * pow((double)j, 2.0));
      weight[i-1][j] = inv_k;
    }
  } 
  // computation du poids d'un point en fonction de se distance au centre de la sphere
  //  printf("on a calcule la fction weight\n");
  // pour les aretes horizontales
  for (row = 0; row < cs; row++)
    for (col = 0; col < rs - 1; col++)
    {
      //      if( (image[0][row*rs+col] < feature_mean[0])
      //	  && (image[0][row*rs+col+1] < feature_mean[0]))
      // {
      //	x_affinity[row*rs+col] = 0;
      // }
      //else
      {
	col1 = col + 1;
	row1 = row;
	// on considere le minimum entre la taille de la boule homogene centre en (x,j) et en (x+1,j)
	if( (row == 32) && (col == 319))
	  printf("On va utiliser les valeurs suivantes pr les echelles: scale %d = %d et scale %d = %d \n",
		 row*rs+col,
		 scale_image[row*rs+col],      row1*rs+col1,
		 scale_image[row1*rs+col1]
		 );
	iscale = mcmin(scale_image[row*rs+col],scale_image[row1*rs+col1]);
	count = 0.0;
//printf("iscale %d ",iscale);
	val = 0.0;
	count = 0.0;
	tti1 = 0;
	edge_flag = 0;
	// pour les tailles de rayon croissante
	for(k = 0;k < iscale;k++)
	{
	  tt1 = weight[iscale-1][k];	    
	  for(i = 0; i < sphere_no_points[k];i++)
	  {
	    xx = sphere_points[k][i][2];
	    yy = sphere_points[k][i][1];	      
	    // (x,y) ds B correspond à (x1,y1) ds B1, B coule centre (col,row) et B1 centre en (col1,row1)
	    x = col + xx; y = row + yy; x1 = col1 + xx; y1 = row1 + yy;
	    tti1 = 0;	      
	    if( x >= 0 && x1>=0 && y>=0 && y1>=0 && 
		x<rs && x1<rs && y<cs && y1<cs)
	    {		
	      for(j=0;j<3;j++)
	      {
		temp[j] = (int32_t) mcabs(image[j][y*rs + x] -
				    image[j][y1*rs + x1]);
		// Interet de ce edge flag a discuter ....
		if (temp[j] > feature_thr[j])
		{
		  edge_flag = 1;
		  if( (row == 32) && (col == 319))
		    printf("j %d temp de j %d \n", j,temp[j]);
		}
		tti1 = tti1 + temp[j]*pow_value[j];
	      }		
	      if(edge_flag)
		tt2 = 0;
	      else			       
		tt2 = homogeneity_map[0][tti1];
	      count = count + tt1;
	      val = val + tt2*tt1;			     
	    }
	  }
	}
	if( (row == 32) && (col == 319))
	  printf("valeur d'afinite : val %f et count %f \n", val,count);
	if(count != 0.0)
	  x_affinity[row*rs+col] = (uint8_t) (255 - ((255 * val)/count) );
	else
	{
	  x_affinity[row*rs+col] = 0;
	  printf("ppp");
	}
      }
    }
  
  // la meme chose pour les aretes verticales
//  printf("feature threshold %d %d\n",feature_thr[0],feature_thr[1]);
  for (row = 0; row < cs-1; row++)
    for (col = 0; col < rs; col++)
    {
      {
	col1 = col; row1 = row+1;
	tti1 = 0; edge_flag = 0; val = 0.0; count = 0.0;
	iscale = mcmin(scale_image[row*rs+col],scale_image[row1*rs+col1]);
	for(k = 0;k < iscale;k++)
	{
	  tt1 = weight[iscale-1][k];	  
	  for(i = 0; i < sphere_no_points[k];i++)
	  {
	    xx = sphere_points[k][i][2];
	    yy = sphere_points[k][i][1];	       
	    x = col + xx; y = row + yy; x1 = col1 + xx; y1 = row1 + yy;
	    tti1 = 0;
	    
	    if( x >= 0 && x1>=0 && y>=0 && y1>=0
		&& x<rs && x1<rs && y<cs && y1<cs)
	    {
	      for(j=0;j<3;j++)
	      {
		temp[j] = (int32_t) mcabs(image[j][y*rs+x] -
				    image[j][y1*rs+x1]);
		if (temp[j] > feature_thr[j])
		  edge_flag = 1;
		tti1 = tti1 + temp[j]*pow_value[j];
	      }		
	      if(edge_flag)
		tt2 = 0;
	      else			       
		tt2 = homogeneity_map[0][tti1];
	      count = count + tt1;
	      val = val + tt2*tt1 ;	      
	    }
	  }
	}  
	if(count != 0.0)
	  y_affinity[row*rs+col]= (uint8_t) (255 - ((255 * val)/count) );
	else
	  y_affinity[row*rs+col]=0;
      }
    }  
// printf("je passe ici ?\n");
  printf("\rHomogeneity computation is done.     \n"); fflush(stdout);
}



