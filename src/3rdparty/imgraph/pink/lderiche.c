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
/****************************************************************
*
* Routine Name: lderiche - library call for deriche
*
* Purpose:     Filtre lineaire general recursif de Deriche
*
* Input:       Image en niveau de gris
* Output:      Image en niveau de gris
* Written By:  Michel Couprie - janvier 1998
*
* Update avril 2009: lshencastan
* Update decembre 2010: lgradientcd lgaussianfilter
*
****************************************************************/

/* 
le filtre lineaire recursif general de Deriche est defini par :
   x : le signal a filtrer
   y : le signal resultat
   y1, y2, r : resultats intermediaires
   alpha : parametre donnant la "taille" du filtre
   a1..8 : parametres calcules a partir de alpha, differents selon les applications
   b1..4 : parametres calcules a partir de alpha, differents selon les applications

   y1[m,n] = a1 x[m,n] + a2 x[m,n-1] + b1 y1[m,n-1] + b2 y1[m,n-2]
     x[m,-1] = 0
     y1[m,-2] = y1[m,-1] = 0
   y2[m,n] = a3 x[m,n+1] + a4 x[m,n+2] + b1 y2[m,n+1] + b2 y2[m,n+2]
     x[m,N] = x[m,N+1] = 0
     y2[m,N] = y2[m,N+1] = 0
   r[m,n] = y1[m,n] + y2[m,n]

   y1[m,n] = a5 r[m,n] + a6 r[m-1,n] + b3 y1[m-1,n] + b4 y1[m-2,n]
     r[-1,n] = 0
     y1[-2,n] = y1[-1,n] = 0
   y2[m,n] = a7 r[m+1,n] + a8 r[m+2,n] + b3 y2[m+1,n] + b4 y2[m+2,n]
     r[M,n] = r[M+1,n] = 0
     y2[M,n] = y2[M+1,n] = 0
   y[m,n] = y1[m,n] + y2[m,n]
*/

#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mccodimage.h>
#include <mcutil.h>
#include <lderiche.h>
/*
#define DEBUG
*/

#define EPSILON 1E-20
#define DIRMAX  31

/* ==================================== */
void derichegen(double *x,               /* image a traiter */
                int32_t M,                   /* taille ligne */
                int32_t N,                   /* taille colonne */
                double *y1,              /* zone temporaire de la taille d'une colonne */
                double *y2,              /* zone temporaire de la taille d'une ligne */
                double *y,               /* stocke un resultat temporaire, puis le resultat final */ 
                double a1, double a2, double a3, double a4, 
                double a5, double a6, double a7, double a8, 
                double b1, double b2, double b3, double b4)
/* ==================================== */
{
  int32_t n, m;

  for (m = 0; m < M; m++)     /* filtrage vertical sur toutes les colonnes */
  {
    /* filtre causal vertical */
#ifdef BORD_ZERO
    y1[0] = a1 * x[m+M*0];
    y1[1] = a1 * x[m+M*1] + a2 * x[m+M*0] + b1 * y1[0];
#else
    y1[0] = ((a1 + a2) / (1.0 - b1 - b2)) * x[m+M*0];
    y1[1] = a1 * x[m+M*1] + a2 * x[m+M*0] + (b1 + b2) * y1[0];
#endif
    for (n = 2; n < N; n++)
      y1[n] = a1*x[m+M*n] + a2*x[m+M*(n-1)] + b1*y1[n-1] + b2*y1[n-2];

    /* filtre anticausal vertical */
#ifdef BORD_ZERO
    y2[N-1] = 0;
    y2[N-2] = a3 * x[m+M*(N-1)];
#else
    y2[N-1] = ((a3 + a4) / (1.0 - b1 - b2)) * x[m+M*(N-1)];
    y2[N-2] = (a3 + a4) * x[m+M*(N-1)] + (b1 + b2) * y2[N-1];
#endif
    for (n = N-3; n >= 0; n--)
      y2[n] = a3*x[m+M*(n+1)] + a4*x[m+M*(n+2)] + b1*y2[n+1] + b2*y2[n+2];

    for (n = 0; n < N; n++)
      y[m+M*n] = y1[n] + y2[n];
  }

  for (n = 0; n < N; n++)     /* filtrage horizontal sur toutes les lignes */
  {
    /* filtre causal horizontal */
#ifdef BORD_ZERO
    y1[0] = a5 * y[0+M*n];
    y1[1] = a5 * y[1+M*n] + a6 * y[0+M*n] + b3 * y1[0];
#else
    y1[0] = ((a5 + a6) / (1.0 - b3 - b4)) * y[0+M*n];
    y1[1] = a5 * y[1+M*n] + a6 * y[0+M*n] + (b3 + b4) * y1[0];
#endif
    for (m = 2; m < M; m++)
      y1[m] = a5 * y[m+M*n] + a6 * y[m-1+M*n] + b3 * y1[m-1] + b4 * y1[m-2];

    /* filtre anticausal horizontal */
#ifdef BORD_ZERO
    y2[M-1] = 0;
    y2[M-2] = a7 * y[M-1+M*n] + b3 * y2[M-1];
#else
    y2[M-1] = ((a7 + a8) / (1.0 - b3 - b4)) * y[M-1+M*n];
    y2[M-2] = (a7 + a8) * y[M-1+M*n] + (b3 + b4) * y2[M-1];
#endif
    for (m = M-3; m >= 0; m--)
      y2[m] = a7 * y[m+1+M*n] + a8 * y[m+2+M*n] + b3 * y2[m+1] + b4 * y2[m+2];

    for (m = 0; m < M; m++)
      y[m+M*n] = y1[m] + y2[m];
  }

} /* derichegen() */

/* ==================================== */
int32_t lderiche(struct xvimage *image, double alpha, int32_t function, double l)
/* ==================================== */
/*
    alpha : parametre (1/taille) du filtre
    function : 0 = module du gradient lisse'
               1 = direction du gradient lisse'
               2 = laplacien (normalise - 127 represente le 0)
               3 = f - l * laplacien(f)
               4 = lisseur
               5 = module du gradient en x
               6 = module du gradient en y
*/
#undef F_NAME
#define F_NAME "lderiche"
{ 
  int32_t i;
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
  double kpp;     /* constante de normalisation pour le laplacien */
  double e_a;     /* stocke exp(-alpha) */
  double e_2a;    /* stocke exp(-2alpha) */
  double a1, a2, a3, a4, a5, a6, a7, a8, b1, b2, b3, b4;
  double t1, t2;
  double lmax, lmin;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    exit(0);
  }

  Im1 = (double *)calloc(1,N * sizeof(double));
  Im2 = (double *)calloc(1,N * sizeof(double));
  Imd = (double *)calloc(1,N * sizeof(double));
  buf1 = (double *)calloc(1,mcmax(rs, cs) * sizeof(double));
  buf2 = (double *)calloc(1,mcmax(rs, cs) * sizeof(double));
  if ((Im1==NULL) || (Im2==NULL) || (Imd==NULL) || (buf1==NULL) || (buf2==NULL))
  {   fprintf(stderr,"%s: malloc failed\n", F_NAME);
      return(0);
  }


  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    uint8_t *ima = UCHARDATA(image);
    for (i = 0; i < N; i++) Imd[i] = (double)ima[i];
  }
  else if (datatype(image) == VFF_TYP_4_BYTE)
  {
    int32_t *ima = SLONGDATA(image);
    for (i = 0; i < N; i++) Imd[i] = (double)ima[i];
  }
  else if (datatype(image) == VFF_TYP_FLOAT)
  {
    float *ima = FLOATDATA(image);
    for (i = 0; i < N; i++) Imd[i] = (double)ima[i];
  }
  else
  {
    fprintf(stderr,"%s: malloc failed\n", F_NAME);
    return(0);
  }

  e_a = exp(- alpha);
  e_2a = exp(- 2.0 * alpha);
  k = 1.0 - e_a;
  k = k * k / (1.0 + (2 * alpha * e_a) - e_2a);
  kp = 1.0 - e_a;
  kp = - kp * kp / e_a;
  kpp = (1.0 - e_2a) / (2 * alpha * e_a);

#ifdef DEBUG
printf("alpha = %g , e_a = %g , e_2a = %g , k = %g\n", alpha, e_a, e_2a, k);
#endif

  switch (function)
  {
    case 0: /* module du gradient lisse' */

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

      if (datatype(image) == VFF_TYP_1_BYTE)
      {
	uint8_t *ima = UCHARDATA(image);
	for (i = 0; i < N; i++)      
	{
	  t1 = Im1[i]; t2 = Im2[i];
	  t2 = sqrt((t1 * t1) + (t2 * t2));
	  if (t2 <= 255.0)
	    ima[i] = (uint8_t)t2;
	  else
	    ima[i] = 255;
	}
      }
      else if (datatype(image) == VFF_TYP_4_BYTE)
      {
	int32_t *ima = SLONGDATA(image);
	for (i = 0; i < N; i++)      
	{
	  t1 = Im1[i]; t2 = Im2[i];
	  ima[i] = (int32_t)sqrt((t1 * t1) + (t2 * t2));
	}
      }
      else if (datatype(image) == VFF_TYP_FLOAT)
      {
	float *ima = FLOATDATA(image);
	for (i = 0; i < N; i++)      
	{
	  t1 = Im1[i]; t2 = Im2[i];
	  ima[i] = (float)sqrt((t1 * t1) + (t2 * t2));
	}
      }
      break;

    case 1:  /* direction du gradient lisse' */

      /* valeurs de parametres pour filtre lisseur-derivateur */
      a1 = k;
      a2 = k * e_a * (alpha - 1.0);
      a3 = k * e_a * (alpha + 1.0);
      a4 = - k * e_2a;

      a5 = 0.0;
      a6 = kp * e_a;
      a7 = - kp * e_a;
      a8 = 0;

      b1 = b3 = 2 * e_a;
      b2 = b4 = - e_2a;

      derichegen(Imd, rs, cs, buf1, buf2, Im1,
                 a1, a2, a3, a4, a5, a6, a7, a8, b1, b2, b3, b4);
      derichegen(Imd, rs, cs, buf1, buf2, Im2,
                 a5, a6, a7, a8, a1, a2, a3, a4, b1, b2, b3, b4);

      if (datatype(image) == VFF_TYP_1_BYTE)
      {
	uint8_t *ima = UCHARDATA(image);
	for (i = 0; i < N; i++)      
	{
	  t1 = Im1[i]; t2 = Im2[i];
	  if (mcabs(t1) >= EPSILON)
	    ima[i] = (uint8_t)((DIRMAX * (atan(t2/t1) + M_PI_2)) / M_PI);
	  else
	    ima[i] = DIRMAX;
	}
      }
      else if (datatype(image) == VFF_TYP_4_BYTE)
      {
	int32_t *ima = SLONGDATA(image);
	for (i = 0; i < N; i++)      
	{
	  t1 = Im1[i]; t2 = Im2[i];
	  if (mcabs(t1) >= EPSILON)
	    ima[i] = (int32_t)((DIRMAX * (atan(t2/t1) + M_PI_2)) / M_PI);
	  else
	    ima[i] = DIRMAX;
	}
      }
      else if (datatype(image) == VFF_TYP_FLOAT)
      {
	float *ima = FLOATDATA(image);
	for (i = 0; i < N; i++)      
	{
	  t1 = Im1[i]; t2 = Im2[i];
	  if (mcabs(t1) >= EPSILON)
	    ima[i] = (float)((DIRMAX * (atan(t2/t1) + M_PI_2)) / M_PI);
	  else
	    ima[i] = DIRMAX;
	}
      }
      break;

    case 2:  /* laplacien lisse' */

      /* valeurs de parametres pour filtre lisseur-laplacien */
      a1 = k;
      a2 = k * e_a * (alpha - 1.0);
      a3 = k * e_a * (alpha + 1.0);
      a4 = - k * e_2a;
      a5 = 1.0;
      a6 = - e_a * (kpp * alpha + 1.0);
      a7 = e_a * (1.0 - kpp * alpha);
      a8 = - e_2a;
      b1 = b3 = 2 * e_a;
      b2 = b4 = - e_2a;

      derichegen(Imd, rs, cs, buf1, buf2, Im1,
                 a1, a2, a3, a4, a5, a6, a7, a8, b1, b2, b3, b4);
      derichegen(Imd, rs, cs, buf1, buf2, Im2,
                 a5, a6, a7, a8, a1, a2, a3, a4, b1, b2, b3, b4);

      if (datatype(image) == VFF_TYP_1_BYTE)
      {
	uint8_t *ima = UCHARDATA(image);
	lmin = lmax = 0.0;
	for (i = 0; i < N; i++)
	  {
	    Im1[i] = t2 = - (Im1[i] + Im2[i]);
	    if (t2 > lmax) lmax = t2;
	    if (t2 < lmin) lmin = t2;
	  }        
	lmax = mcmax(lmax, -lmin);
	for (i = 0; i < N; i++)      
	  ima[i] = 127 + (uint8_t)(Im1[i] * 128.0 / lmax);
      }
      else 
      {
        fprintf(stderr, "%s: mode 2 not yet implemented for long and float\n", F_NAME);
        return 0;
      }
      break;

    case 3:  /* f - l * laplacien(f) */

      /* valeurs de parametres pour filtre lisseur-laplacien */
      a1 = k;
      a2 = k * e_a * (alpha - 1.0);
      a3 = k * e_a * (alpha + 1.0);
      a4 = - k * e_2a;

      a5 = 1.0;
      a6 = - e_a * (kpp * alpha + 1.0);
      a7 = e_a * (1.0 - kpp * alpha);
      a8 = - e_2a;

      b1 = b3 = 2 * e_a;
      b2 = b4 = - e_2a;

      derichegen(Imd, rs, cs, buf1, buf2, Im1,
                 a1, a2, a3, a4, a5, a6, a7, a8, b1, b2, b3, b4);
      derichegen(Imd, rs, cs, buf1, buf2, Im2,
                 a5, a6, a7, a8, a1, a2, a3, a4, b1, b2, b3, b4);

      if (datatype(image) == VFF_TYP_1_BYTE)
      {
	uint8_t *ima = UCHARDATA(image);

	for (i = 0; i < N; i++)      
	  {
	    t1 = -(Im1[i] + Im2[i]);
	    t2 = (double)(ima[i]) - l * t1;
	    if (t2 < 0.0) t2 = 0.0;
	    if (t2 > 255.0) t2 = 255.0;
	    ima[i] = (uint8_t)floor(t2);
	  }
      }
      else 
      {
        fprintf(stderr, "%s: mode 3 not yet implemented for long and float\n", F_NAME);
        return 0;
      }

      break;

    case 4:  /* lisseur */

      a5 = a1 = k;
      a6 = a2 = k * e_a * (alpha - 1.0);
      a7 = a3 = k * e_a * (alpha + 1.0);
      a8 = a4 = - k * e_2a;

      b1 = b3 = 2 * e_a;
      b2 = b4 = - e_2a;

      derichegen(Imd, rs, cs, buf1, buf2, Im1,
                 a1, a2, a3, a4, a5, a6, a7, a8, b1, b2, b3, b4);

      if (datatype(image) == VFF_TYP_1_BYTE)
      {
	uint8_t *ima = UCHARDATA(image);
	for (i = 0; i < N; i++)      
	{
	  t1 =  Im1[i];
	  if (t1 < 0.0) t1 = 0.0;
	  if (t1 > 255.0) t1 = 255.0;
	  ima[i] = (uint8_t)floor(t1);
	}
      }
      else if (datatype(image) == VFF_TYP_4_BYTE)
      {
	int32_t *ima = SLONGDATA(image);
	for (i = 0; i < N; i++)      
	  ima[i] = (int32_t)floor(Im1[i]);
      }
      else if (datatype(image) == VFF_TYP_FLOAT)
      {
	float *ima = FLOATDATA(image);
	for (i = 0; i < N; i++)      
	  ima[i] = (float)Im1[i];
      }
      break;

    case 5: /* module du gradient en x */

      /* valeurs de parametres pour filtre derivateur */
      a1 = 1;
      a2 = 0;
      a3 = 0;
      a4 = 0;

      a5 = 0.0;
      a6 = kp * e_a;
      a7 = - kp * e_a;
      a8 = 0.0;

      b1 = b2 = 0;
      b3 = 2 * e_a;
      b4 = - e_2a;

      derichegen(Imd, rs, cs, buf1, buf2, Im1,
                 a1, a2, a3, a4, a5, a6, a7, a8, b1, b2, b3, b4);


      if (datatype(image) == VFF_TYP_1_BYTE)
      {
	uint8_t *ima = UCHARDATA(image);
	for (i = 0; i < N; i++)
	  {
	    t1 = Im1[i];
	    t2 = mcabs(t1);
	    if (t2 <= 255.0)
	      ima[i] = (uint8_t)t2;
	    else
	      ima[i] = 255;
	  }
      }
      else 
      {
        fprintf(stderr, "%s: mode 2 not yet implemented for long and float\n", F_NAME);
        return 0;
      }

      break;

    case 6: /* module du gradient en y */

      /* valeurs de parametres pour filtre derivateur */
      a5 = 1;
      a6 = 0;
      a7 = 0;
      a8 = 0;

      a1 = 0.0;
      a2 = kp * e_a;
      a3 = - kp * e_a;
      a4 = 0.0;

      b3 = b4 = 0;
      b1 = 2 * e_a;
      b2 = - e_2a;

      derichegen(Imd, rs, cs, buf1, buf2, Im1,
                 a1, a2, a3, a4, a5, a6, a7, a8, b1, b2, b3, b4);


      if (datatype(image) == VFF_TYP_1_BYTE)
      {
	uint8_t *ima = UCHARDATA(image);
	for (i = 0; i < N; i++)
	  {
	    t1 = Im1[i];
	    t2 = mcabs(t1);
	    if (t2 <= 255.0)
	      ima[i] = (uint8_t)t2;
	    else
	      ima[i] = 255;
	  }
      }
      else 
      {
        fprintf(stderr, "%s: mode 2 not yet implemented for long and float\n", F_NAME);
        return 0;
      }
      break;

      default: 
        fprintf(stderr, "%s: fonction %d inexistante ; utiliser : \n", F_NAME, function);
        fprintf(stderr, "  0 : module du gradient lisse'\n");
        fprintf(stderr, "  1 : direction du gradient lisse'\n");
        fprintf(stderr, "  2 : laplacien lisse'\n");
        fprintf(stderr, "  3 : f - l * laplacien(f)\n");
        fprintf(stderr, "  4 : lisseur\n");
        return 0;
  } /* switch (function) */


  free(Im1);
  free(Im2);
  free(Imd);
  free(buf1);
  free(buf2);
  return 1;
} // lderiche()

/* ==================================== */
int32_t lshencastan(struct xvimage *image, double beta)
/* ==================================== */
/*
    beta : parametre (1/taille) du filtre
*/
{ 
  int32_t i;
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
  double e_a;     /* stocke exp(-beta) */
  double a1, a2, a3, a4, a5, a6, a7, a8, b1, b2, b3, b4;
  double t1, t2;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "lderiche: cette version ne traite pas les images volumiques\n");
    exit(0);
  }

  Im1 = (double *)calloc(1,N * sizeof(double));
  Im2 = (double *)calloc(1,N * sizeof(double));
  Imd = (double *)calloc(1,N * sizeof(double));
  buf1 = (double *)calloc(1,mcmax(rs, cs) * sizeof(double));
  buf2 = (double *)calloc(1,mcmax(rs, cs) * sizeof(double));
  if ((Im1==NULL) || (Im2==NULL) || (Imd==NULL) || (buf1==NULL) || (buf2==NULL))
  {   printf("lderiche() : malloc failed\n");
      return(0);
  }

  for (i = 0; i < N; i++) Imd[i] = (double)ima[i];

  e_a = exp(- beta);
  k = 1.0 - e_a;
  k = - k * k / e_a;
  kp = 1.0 - e_a;

#ifdef DEBUG
printf("beta = %g , e_a = %g , e_2a = %g , k = %g\n", beta, e_a, e_2a, k);
#endif

  /* valeurs de parametres pour filtre derivateur */
  a1 = 0;
  a2 = k * e_a;
  a3 = - k * e_a;
  a4 = 0;
  a5 = 1;
  a6 = 0;
  a7 = 0;
  a8 = 0;
  b1 = e_a;
  b2 = 0;
  b3 = 0;
  b4 = 0;

  derichegen(Imd, rs, cs, buf1, buf2, Im1,
             a1, a2, a3, a4, a5, a6, a7, a8, b1, b2, b3, b4);
  derichegen(Imd, rs, cs, buf1, buf2, Im2,
             a5, a6, a7, a8, a1, a2, a3, a4, b3, b4, b1, b2);

  for (i = 0; i < N; i++)
  {
    t1 = Im1[i];
    t2 = Im2[i];
    t2 = sqrt(((t1 * t1) + (t2 * t2)) / 2.0);
    if (t2 <= 255.0)
      ima[i] = (uint8_t)t2;
    else
      ima[i] = 255;
  }

  free(Im1);
  free(Im2);
  free(Imd);
  free(buf1);
  free(buf2);

  return 1;
} // lshencastan()


/* ==================================== */
void deriche3dgen(double *f,               /* image a traiter */
                int32_t rs,                  /* taille ligne */
                int32_t cs,                  /* taille colonne */
                int32_t ds,                  /* nombre plans */
                double *g1,              /* zone temporaire de taille mcmax(rs,cs,ds) */
                double *g2,              /* zone temporaire de taille mcmax(rs,cs,ds) */
                double *g,               /* stocke un resultat temporaire, puis le resultat final */ 
                double a1, double a2, double a3, double a4, double b1, double b2,   /* param. dir. z */
                double a5, double a6, double a7, double a8, double b3, double b4,   /* param. dir. y */
                double a9, double a10, double a11, double a12, double b5, double b6 /* param. dir. x */
               )
/* ==================================== */
{
  int32_t x, y, z, ps = rs * cs;

  for (y = 0; y < cs; y++)     /* filtrage dans la direction z (f -> g) */
  for (x = 0; x < rs; x++)
  {
    /* filtre causal en z */
#ifdef BORD_ZERO
    g1[0] = a1 * f[x+rs*y+ps*0];
    g1[1] = a1 * f[x+rs*y+ps*1] + a2 * f[x+rs*y+ps*0] + b1 * g1[0];
#else
    g1[0] = ((a1 + a2) / (1.0 - b1 - b2)) * f[x+rs*y+ps*0];
    g1[1] = a1 * f[x+rs*y+ps*1] + a2 * f[x+rs*y+ps*0] + (b1 + b2) * g1[0];
#endif
    for (z = 2; z < ds; z++)
      g1[z] = a1*f[x+rs*y+ps*z] + a2*f[x+rs*y+ps*(z-1)] + b1*g1[z-1] + b2*g1[z-2];

    /* filtre anticausal en z */
#ifdef BORD_ZERO
    g2[ds-1] = 0;
    g2[ds-2] = a3 * f[x+rs*y+ps*(ds-1)] + b1 * g2[ds-1];
#else
    g2[ds-1] = ((a3 + a4) / (1.0 - b1 - b2)) * f[x+rs*y+ps*(ds-1)];
    g2[ds-2] = (a3 + a4) * f[x+rs*y+ps*(ds-1)] + (b1 + b2) * g2[ds-1];
#endif
    for (z = ds-3; z >= 0; z--)
      g2[z] = a3*f[x+rs*y+ps*(z+1)] + a4*f[x+rs*y+ps*(z+2)] + b1*g2[z+1] + b2*g2[z+2];

    for (z = 0; z < ds; z++)
      g[x+rs*y+ps*z] = g1[z] + g2[z];
  }

  for (z = 0; z < ds; z++)     /* filtrage dans la direction y (g -> g) */
  for (x = 0; x < rs; x++)
  {
    /* filtre causal en y */
#ifdef BORD_ZERO
    g1[0] = a5 * g[x+rs*0+ps*z];
    g1[1] = a5 * g[x+rs*1+ps*z] + a6 * g[x+rs*0+ps*z] + b3 * g1[0];
#else
    g1[0] = ((a5 + a6) / (1.0 - b3 - b4)) * g[x+rs*0+ps*z];
    g1[1] = a5 * g[x+rs*1+ps*z] + a6 * g[x+rs*0+ps*z] + (b3 + b4) * g1[0];
#endif
    for (y = 2; y < cs; y++)
      g1[y] = a5*g[x+rs*y+ps*z] + a6*g[x+rs*(y-1)+ps*z] + b3*g1[y-1] + b4*g1[y-2];

    /* filtre anticausal en y */
#ifdef BORD_ZERO
    g2[cs-1] = 0;
    g2[cs-2] = a7 * g[x+rs*(cs-1)+ps*z] + b3 * g2[cs-1];
#else
    g2[cs-1] = ((a7 + a8) / (1.0 - b3 - b4)) * g[x+rs*(cs-1)+ps*z];
    g2[cs-2] = (a7 + a8) * g[x+rs*(cs-1)+ps*z] + (b3 + b4) * g2[cs-1];
#endif
    for (y = cs-3; y >= 0; y--)
      g2[y] = a7*g[x+rs*(y+1)+ps*z] + a8*g[x+rs*(y+2)+ps*z] + b3*g2[y+1] + b4*g2[y+2];

    for (y = 0; y < cs; y++)
      g[x+rs*y+ps*z] = g1[y] + g2[y];
  }

  for (z = 0; z < ds; z++)     /* filtrage dans la direction x (g -> g) */
  for (y = 0; y < cs; y++)
  {
    /* filtre causal en x */
#ifdef BORD_ZERO
    g1[0] = a9 * g[0+rs*y+ps*z];
    g1[1] = a9 * g[1+rs*y+ps*z] + a10 * g[0+rs*y+ps*z] + b5 * g1[0];
#else
    g1[0] = ((a9 + a10) / (1.0 - b5 - b6)) * g[0+rs*y+ps*z];
    g1[1] = a9 * g[1+rs*y+ps*z] + a10 * g[0+rs*y+ps*z] + (b5 + b6) * g1[0];
#endif
    for (x = 2; x < rs; x++)
      g1[x] = a9 * g[x+rs*y+ps*z] + a10 * g[x-1+rs*y+ps*z] + b5 * g1[x-1] + b6 * g1[x-2];

    /* filtre anticausal en x */
#ifdef BORD_ZERO
    g2[rs-1] = 0;
    g2[rs-2] = a11 * g[rs-1+rs*y+ps*z] + b5 * g2[rs-1];
#else
    g2[rs-1] = ((a11 + a12) / (1.0 - b5 - b6)) * g[rs-1+rs*y+ps*z];
    g2[rs-2] = (a11 + a12) * g[rs-1+rs*y+ps*z] + (b5 + b6) * g2[rs-1];
#endif
    for (x = rs-3; x >= 0; x--)
      g2[x] = a11 * g[x+1+rs*y+ps*z] + a12 * g[x+2+rs*y+ps*z] + b5 * g2[x+1] + b6 * g2[x+2];

    for (x = 0; x < rs; x++)
      g[x+rs*y+ps*z] = g1[x] + g2[x];
  }

} /* deriche3dgen() */

/* ==================================== */
int32_t lderiche3d(struct xvimage *image, double alpha, int32_t function, double l)
/* ==================================== */
/*
    alpha : parametre (1/taille) du filtre
    function : 0 = module du gradient lisse'
               1 = direction du gradient lisse' 
               2 = laplacien (normalise - 127 represente le 0)
               3 = f - l * laplacien(f)
               4 = lisseur
*/
#undef F_NAME
#define F_NAME "lderiche3d"
{ 
  int32_t i;
  int32_t rs = rowsize(image);
  int32_t cs = colsize(image);
  int32_t ds = depth(image);
  int32_t ps = rs * cs;
  int32_t N = ps * ds;
  double *Im1;    /* image intermediaire */
  double *Im2;    /* image intermediaire */
  double *Im3;    /* image intermediaire */
  double *Imd;    /* image intermediaire */
  double *buf1;   /* buffer ligne ou colonne */
  double *buf2;   /* buffer ligne ou colonne */
  double k;       /* constante de normalisation pour le lisseur */
  double kp;      /* constante de normalisation pour le derivateur */
  double kpp;     /* constante de normalisation pour le laplacien */
  double e_a;     /* stocke exp(-alpha) */
  double e_2a;    /* stocke exp(-2alpha) */
  double a1, a2, a3, a4, a5, a6, a7, a8;
  double b1, b2, b3, b4;
  double t1, t2, t3;
  double sbuf;

  Im1 = (double *)calloc(1,N * sizeof(double));
  Imd = (double *)calloc(1,N * sizeof(double));
  if ((Im1==NULL) || (Imd==NULL))
  {
    fprintf(stderr,"%s: malloc failed\n", F_NAME);
    return(0);
  }
  if (function == 0)
  {
    Im2 = (double *)calloc(1,N * sizeof(double));
    Im3 = (double *)calloc(1,N * sizeof(double));
    if ((Im2==NULL) || (Im3==NULL))
    {
      fprintf(stderr,"%s: malloc failed\n", F_NAME);
      return(0);
    }
  }
  sbuf = mcmax((mcmax(rs,cs)),ds);
  buf1 = (double *)calloc(1,sbuf * sizeof(double));
  buf2 = (double *)calloc(1,sbuf * sizeof(double));
  if ((buf1==NULL) || (buf2==NULL))
  {   
    fprintf(stderr,"%s: malloc failed\n", F_NAME);
    return(0);
  }

  if (datatype(image) == VFF_TYP_1_BYTE)
  {
    uint8_t *ima = UCHARDATA(image);
    for (i = 0; i < N; i++) Imd[i] = (double)ima[i];
  }
  else if (datatype(image) == VFF_TYP_4_BYTE)
  {
    int32_t *ima = SLONGDATA(image);
    for (i = 0; i < N; i++) Imd[i] = (double)ima[i];
  }
  else if (datatype(image) == VFF_TYP_FLOAT)
  {
    float *ima = FLOATDATA(image);
    for (i = 0; i < N; i++) Imd[i] = (double)ima[i];
  }
  else
  {
    fprintf(stderr,"%s: malloc failed\n", F_NAME);
    return(0);
  }

  e_a = exp(- alpha);
  e_2a = exp(- 2.0 * alpha);
  k = 1.0 - e_a;
  k = k * k / (1.0 + (2 * alpha * e_a) - e_2a);
  kp = 1.0 - e_a;
  kp = - kp * kp / e_a;
  kpp = (1.0 - e_2a) / (2 * alpha * e_a);

#ifdef DEBUG
printf("alpha = %g , e_a = %g , e_2a = %g , k = %g\n", alpha, e_a, e_2a, k);
#endif

  switch (function)
  {
    case 0: /* module du gradient lisse' */

      /* valeurs de parametres pour filtre lisseur-derivateur */
      a1 = k;                         /* lisseur */
      a2 = k * e_a * (alpha - 1.0);
      a3 = k * e_a * (alpha + 1.0);
      a4 = - k * e_2a;

      a5 = 0.0;                       /* derivateur */
      a6 = kp * e_a;
      a7 = - kp * e_a;
      a8 = 0.0;

      b1 = 2 * e_a;
      b2 = - e_2a;

      deriche3dgen(Imd, rs, cs, ds, buf1, buf2, Im1,
                   a5, a6, a7, a8, b1, b2,           /* derive en x */
                   a1, a2, a3, a4, b1, b2,           /* lisse en y */
                   a1, a2, a3, a4, b1, b2);          /* lisse en z */

      deriche3dgen(Imd, rs, cs, ds, buf1, buf2, Im2,
                   a1, a2, a3, a4, b1, b2,           /* lisse en x */
                   a5, a6, a7, a8, b1, b2,           /* derive en y */
                   a1, a2, a3, a4, b1, b2);          /* lisse en z */

      deriche3dgen(Imd, rs, cs, ds, buf1, buf2, Im3,
                   a1, a2, a3, a4, b1, b2,           /* lisse en x */
                   a1, a2, a3, a4, b1, b2,           /* lisse en y */
                   a5, a6, a7, a8, b1, b2);          /* derive en z */


      if (datatype(image) == VFF_TYP_1_BYTE)
      {
	uint8_t *ima = UCHARDATA(image);
	for (i = 0; i < N; i++)      
	{
	  t1 = Im1[i]; t2 = Im2[i]; t3 = Im3[i];
	  t2 = sqrt((t1 * t1) + (t2 * t2) + (t3 * t3));
	  if (t2 <= 255.0)
	    ima[i] = (uint8_t)t2;
	  else
	    ima[i] = 255;
	}
      }
      else if (datatype(image) == VFF_TYP_4_BYTE)
      {
	int32_t *ima = SLONGDATA(image);
	for (i = 0; i < N; i++)      
	{
	  t1 = Im1[i]; t2 = Im2[i]; t3 = Im3[i];
	  ima[i] = (int32_t)sqrt((t1 * t1) + (t2 * t2) + (t3 * t3));
	}
      }
      else if (datatype(image) == VFF_TYP_FLOAT)
      {
	float *ima = FLOATDATA(image);
	for (i = 0; i < N; i++)      
	{
	  t1 = Im1[i]; t2 = Im2[i]; t3 = Im3[i];
	  ima[i] = (float)sqrt((t1 * t1) + (t2 * t2) + (t3 * t3));
	}
      }
      break;

    case 1:  /* direction du gradient lisse' */
        fprintf(stderr, "%s: mode 1 not yet implemented\n", F_NAME);
        return 0;

    case 2:  /* laplacien lisse' */

      /* valeurs de parametres pour filtre lisseur-laplacien */
      a1 = k;
      a2 = k * e_a * (alpha - 1.0);
      a3 = k * e_a * (alpha + 1.0);
      a4 = - k * e_2a;
      a5 = 1.0;
      a6 = - e_a * (kpp * alpha + 1.0);
      a7 = e_a * (1.0 - kpp * alpha);
      a8 = - e_2a;
      b1 = b3 = 2 * e_a;
      b2 = b4 = - e_2a;
      fprintf(stderr, "%s: mode 2 not yet implemented\n", F_NAME);
      /*
      deriche3dgen(Imd, rs, cs, buf1, buf2, Im1,
                 a1, a2, a3, a4, a5, a6, a7, a8, b1, b2, b3, b4);
      deriche3dgen(Imd, rs, cs, buf1, buf2, Im2,
                 a5, a6, a7, a8, a1, a2, a3, a4, b1, b2, b3, b4);


      lmin = lmax = 0.0;
      for (i = 0; i < N; i++)
      {
        Im1[i] = t2 = - (Im1[i] + Im2[i]);
        if (t2 > lmax) lmax = t2;
        if (t2 < lmin) lmin = t2;
      }        
      lmax = mcmax(lmax, -lmin);
      for (i = 0; i < N; i++)      
        ima[i] = 127 + (uint8_t)(Im1[i] * 128.0 / lmax);
      */
      break;

    case 3:  /* f - l * laplacien(f) */

      /* valeurs de parametres pour filtre lisseur-laplacien */
      a1 = k;
      a2 = k * e_a * (alpha - 1.0);
      a3 = k * e_a * (alpha + 1.0);
      a4 = - k * e_2a;

      a5 = 1.0;
      a6 = - e_a * (kpp * alpha + 1.0);
      a7 = e_a * (1.0 - kpp * alpha);
      a8 = - e_2a;

      b1 = b3 = 2 * e_a;
      b2 = b4 = - e_2a;
      fprintf(stderr, "%s: mode 3 not yet implemented\n", F_NAME);
      /*
      deriche3dgen(Imd, rs, cs, buf1, buf2, Im1,
                 a1, a2, a3, a4, a5, a6, a7, a8, b1, b2, b3, b4);
      deriche3dgen(Imd, rs, cs, buf1, buf2, Im2,
                 a5, a6, a7, a8, a1, a2, a3, a4, b1, b2, b3, b4);

      for (i = 0; i < N; i++)      
      {
        t1 = -(Im1[i] + Im2[i]);
        t2 = (double)(ima[i]) - l * t1;
        if (t2 < 0.0) t2 = 0.0;
        if (t2 > 255.0) t2 = 255.0;
        ima[i] = (uint8_t)floor(t2);
      }
      */
      break;

    case 4:  /* lisseur */

      a1 = k;
      a2 = k * e_a * (alpha - 1.0);
      a3 = k * e_a * (alpha + 1.0);
      a4 = - k * e_2a;

      b1 = 2 * e_a;
      b2 = - e_2a;

      deriche3dgen(Imd, rs, cs, ds, buf1, buf2, Im1,
                   a1, a2, a3, a4, b1, b2,           /* lisse en x */
                   a1, a2, a3, a4, b1, b2,           /* lisse en y */
                   a1, a2, a3, a4, b1, b2);          /* lisse en z */

      if (datatype(image) == VFF_TYP_1_BYTE)
      {
	uint8_t *ima = UCHARDATA(image);
	for (i = 0; i < N; i++)      
	{
	  t1 =  Im1[i];
	  if (t1 < 0.0) t1 = 0.0;
	  if (t1 > 255.0) t1 = 255.0;
	  ima[i] = (uint8_t)floor(t1);
	}
      }
      else if (datatype(image) == VFF_TYP_4_BYTE)
      {
	int32_t *ima = SLONGDATA(image);
	for (i = 0; i < N; i++)      
	  ima[i] = (int32_t)floor(Im1[i]);
      }
      else if (datatype(image) == VFF_TYP_FLOAT)
      {
	float *ima = FLOATDATA(image);
	for (i = 0; i < N; i++)      
	  ima[i] = (float)Im1[i];
      }
      break;

      default: 
        fprintf(stderr, "%s: fonction %d inexistante ; utiliser : \n", F_NAME, function);
        fprintf(stderr, "  0 : module du gradient lisse'\n");
        fprintf(stderr, "  1 : direction du gradient lisse'\n");
        fprintf(stderr, "  2 : laplacien lisse'\n");
        fprintf(stderr, "  3 : f - l * laplacien(f)\n");
        fprintf(stderr, "  4 : lisseur\n");
        return 0;
  } /* switch (function) */


  free(Im1);
  free(Imd);
  if (function == 1)
  {
    free(Im2);
    free(Im3);
  }
  free(buf1);
  free(buf2);
  return 1;
}

/* ==================================== */
void deriche3dgenb(uint8_t *f,     /* image a traiter */
                int32_t rs,                  /* taille ligne */
                int32_t cs,                  /* taille colonne */
                int32_t ds,                  /* nombre plans */
                double *g1,              /* zone temporaire de taille mcmax(rs,cs,ds) */
                double *g2,              /* zone temporaire de taille mcmax(rs,cs,ds) */
                double *g,               /* stocke un resultat temporaire, puis le resultat final */ 
                double a1, double a2, double a3, double a4, double b1, double b2,   /* param. dir. z */
                double a5, double a6, double a7, double a8, double b3, double b4,   /* param. dir. y */
                double a9, double a10, double a11, double a12, double b5, double b6 /* param. dir. x */
               )
/* ==================================== */
//  traite directement une image d'entiers
{
  int32_t x, y, z, ps = rs * cs;

  for (y = 0; y < cs; y++)     /* filtrage dans la direction z (f -> g) */
  for (x = 0; x < rs; x++)
  {
    /* filtre causal en z */
#ifdef BORD_ZERO
    g1[0] = a1 * (double)(f[x+rs*y+ps*0]);
    g1[1] = a1 * (double)(f[x+rs*y+ps*1]) + a2 * (double)(f[x+rs*y+ps*0]) + b1 * g1[0];
#else
    g1[0] = ((a1 + a2) / (1.0 - b1 - b2)) * (double)(f[x+rs*y+ps*0]);
    g1[1] = a1 * (double)(f[x+rs*y+ps*1]) + a2 * (double)(f[x+rs*y+ps*0]) + (b1 + b2) * g1[0];
#endif
    for (z = 2; z < ds; z++)
      g1[z] = a1*(double)(f[x+rs*y+ps*z]) + a2*(double)(f[x+rs*y+ps*(z-1)]) + b1*g1[z-1] + b2*g1[z-2];

    /* filtre anticausal en z */
#ifdef BORD_ZERO
    g2[ds-1] = 0;
    g2[ds-2] = a3 * (double)(f[x+rs*y+ps*(ds-1)]) + b1 * g2[ds-1];
#else
    g2[ds-1] = ((a3 + a4) / (1.0 - b1 - b2)) * (double)(f[x+rs*y+ps*(ds-1)]);
    g2[ds-2] = (a3 + a4) * (double)(f[x+rs*y+ps*(ds-1)]) + (b1 + b2) * g2[ds-1];
#endif
    for (z = ds-3; z >= 0; z--)
      g2[z] = a3*(double)(f[x+rs*y+ps*(z+1)]) + a4*(double)(f[x+rs*y+ps*(z+2)]) + b1*g2[z+1] + b2*g2[z+2];

    for (z = 0; z < ds; z++)
      g[x+rs*y+ps*z] = g1[z] + g2[z];
  }

  for (z = 0; z < ds; z++)     /* filtrage dans la direction y (g -> g) */
  for (x = 0; x < rs; x++)
  {
    /* filtre causal en y */
#ifdef BORD_ZERO
    g1[0] = a5 * g[x+rs*0+ps*z];
    g1[1] = a5 * g[x+rs*1+ps*z] + a6 * g[x+rs*0+ps*z] + b3 * g1[0];
#else
    g1[0] = ((a5 + a6) / (1.0 - b3 - b4)) * g[x+rs*0+ps*z];
    g1[1] = a5 * g[x+rs*1+ps*z] + a6 * g[x+rs*0+ps*z] + (b3 + b4) * g1[0];
#endif
    for (y = 2; y < cs; y++)
      g1[y] = a5*g[x+rs*y+ps*z] + a6*g[x+rs*(y-1)+ps*z] + b3*g1[y-1] + b4*g1[y-2];

    /* filtre anticausal en y */
#ifdef BORD_ZERO
    g2[cs-1] = 0;
    g2[cs-2] = a7 * g[x+rs*(cs-1)+ps*z] + b3 * g2[cs-1];
#else
    g2[cs-1] = ((a7 + a8) / (1.0 - b3 - b4)) * g[x+rs*(cs-1)+ps*z];
    g2[cs-2] = (a7 + a8) * g[x+rs*(cs-1)+ps*z] + (b3 + b4) * g2[cs-1];
#endif
    for (y = cs-3; y >= 0; y--)
      g2[y] = a7*g[x+rs*(y+1)+ps*z] + a8*g[x+rs*(y+2)+ps*z] + b3*g2[y+1] + b4*g2[y+2];

    for (y = 0; y < cs; y++)
      g[x+rs*y+ps*z] = g1[y] + g2[y];
  }

  for (z = 0; z < ds; z++)     /* filtrage dans la direction x (g -> g) */
  for (y = 0; y < cs; y++)
  {
    /* filtre causal en x */
#ifdef BORD_ZERO
    g1[0] = a9 * g[0+rs*y+ps*z];
    g1[1] = a9 * g[1+rs*y+ps*z] + a10 * g[0+rs*y+ps*z] + b5 * g1[0];
#else
    g1[0] = ((a9 + a10) / (1.0 - b5 - b6)) * g[0+rs*y+ps*z];
    g1[1] = a9 * g[1+rs*y+ps*z] + a10 * g[0+rs*y+ps*z] + (b5 + b6) * g1[0];
#endif
    for (x = 2; x < rs; x++)
      g1[x] = a9 * g[x+rs*y+ps*z] + a10 * g[x-1+rs*y+ps*z] + b5 * g1[x-1] + b6 * g1[x-2];

    /* filtre anticausal en x */
#ifdef BORD_ZERO
    g2[rs-1] = 0;
    g2[rs-2] = a11 * g[rs-1+rs*y+ps*z] + b5 * g2[rs-1];
#else
    g2[rs-1] = ((a11 + a12) / (1.0 - b5 - b6)) * g[rs-1+rs*y+ps*z];
    g2[rs-2] = (a11 + a12) * g[rs-1+rs*y+ps*z] + (b5 + b6) * g2[rs-1];
#endif
    for (x = rs-3; x >= 0; x--)
      g2[x] = a11 * g[x+1+rs*y+ps*z] + a12 * g[x+2+rs*y+ps*z] + b5 * g2[x+1] + b6 * g2[x+2];

    for (x = 0; x < rs; x++)
      g[x+rs*y+ps*z] = g1[x] + g2[x];
  }

} /* deriche3dgenb() */

/* ==================================== */
int32_t llisseurrec3d(struct xvimage *image, double alpha)
/* ==================================== */
/*
    alpha : parametre (1/taille) du filtre
*/
{ 
  int32_t i;
  uint8_t *ima = UCHARDATA(image);
  int32_t rs = rowsize(image);
  int32_t cs = colsize(image);
  int32_t ds = depth(image);
  int32_t ps = rs * cs;
  int32_t N = ps * ds;
  double *Im1;    /* image intermediaire */
  double *buf1;   /* buffer ligne ou colonne */
  double *buf2;   /* buffer ligne ou colonne */
  double k;       /* constante de normalisation pour le lisseur */
  double kp;      /* constante de normalisation pour le derivateur */
  double kpp;     /* constante de normalisation pour le laplacien */
  double e_a;     /* stocke exp(-alpha) */
  double e_2a;    /* stocke exp(-2alpha) */
  double a1, a2, a3, a4;
  double b1, b2;
  double t1;

  Im1 = (double *)calloc(1,N * sizeof(double));
  if (Im1==NULL)
  {   fprintf(stderr,"lderiche3d() : malloc failed\n");
      return(0);
  }
  buf1 = (double *)calloc(1,mcmax(mcmax(rs,cs),ds) * sizeof(double));
  buf2 = (double *)calloc(1,mcmax(mcmax(rs,cs),ds) * sizeof(double));
  if ((buf1==NULL) || (buf2==NULL))
  {   fprintf(stderr,"lderiche3d() : malloc failed\n");
      return(0);
  }

  e_a = exp(- alpha);
  e_2a = exp(- 2.0 * alpha);
  k = 1.0 - e_a;
  k = k * k / (1.0 + (2 * alpha * e_a) - e_2a);
  kp = 1.0 - e_a;
  kp = - kp * kp / e_a;
  kpp = (1.0 - e_2a) / (2 * alpha * e_a);

#ifdef DEBUG
printf("alpha = %g , e_a = %g , e_2a = %g , k = %g\n", alpha, e_a, e_2a, k);
#endif

  a1 = k;
  a2 = k * e_a * (alpha - 1.0);
  a3 = k * e_a * (alpha + 1.0);
  a4 = - k * e_2a;
  b1 = 2 * e_a;
  b2 = - e_2a;

  deriche3dgenb(ima, rs, cs, ds, buf1, buf2, Im1,
                a1, a2, a3, a4, b1, b2,           /* lisse en x */
                a1, a2, a3, a4, b1, b2,           /* lisse en y */
                a1, a2, a3, a4, b1, b2);          /* lisse en z */

  for (i = 0; i < N; i++)      
  {
    t1 =  Im1[i];
    if (t1 < 0.0) t1 = 0.0;
    if (t1 > 255.0) t1 = 255.0;
    ima[i] = (uint8_t)floor(t1);
  }

  free(Im1);
  free(buf1);
  free(buf2);
  return 1;
} /* llisseurrec3d() */

/* ==================================== */
int32_t lgradientcd(struct xvimage *image, double alpha)
/* ==================================== */
#undef F_NAME
#define F_NAME "lgradientcd"
{ 
  double dummy;
  if (depth(image) == 1)
    return lderiche(image, alpha, 0, dummy);
  else
    return lderiche3d(image, alpha, 0, dummy);
} // lgradientcd()

/* ==================================== */
int32_t lgaussianfilter(struct xvimage *image, double alpha)
/* ==================================== */
#undef F_NAME
#define F_NAME "lgaussianfilter"
{ 
  double dummy;
  if (depth(image) == 1)
    return lderiche(image, alpha, 4, dummy);
  else
    return lderiche3d(image, alpha, 4, dummy);
} // lgaussianfilter()
