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
/* 
   Librairie mckhalimsky2d

   Michel Couprie  --  novembre 1998

   Les beta-terminaux (carres) de la grille de khalimsky sont codees par les points
   dont les deux coordonnees sont impaires, les alpha-terminaux (singletons)
   par les points dont les deux coordonnees sont paires.

*/
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <assert.h>
#include <string.h>
#include <mcimage.h>
#include <mccodimage.h>
#include <mcutil.h>
#include <mckhalimsky2d.h>

/*
#define DEBUG
*/

/* en global : la pile des grilles pour l'allocation "maison" */

#define NBGRILLES2D 5
#define GRS2D 3
#define GCS2D 3

#define VAL_NULLE    0
#define VAL_OBJET  255
#define VAL_MARQUE   1

static struct xvimage * PileGrilles2d[NBGRILLES2D];
static int32_t IndexPileGrilles2d;

/* ========================================== */
void InitPileGrilles2d()
/* ========================================== */
{
  int32_t i;
  for (i = 0; i < NBGRILLES2D; i++)
    PileGrilles2d[i] = allocimage(NULL, GRS2D, GCS2D, 1, VFF_TYP_1_BYTE);
  IndexPileGrilles2d = 0;
} /* InitPileGrilles2d() */

/* ========================================== */
void TerminePileGrilles2d()
/* ========================================== */
{
  int32_t i;
  for (i = 0; i < NBGRILLES2D; i++)
    freeimage(PileGrilles2d[i]);
} /* TerminePileGrilles2d() */

/* ========================================== */
struct xvimage * AllocGrille2d()
/* ========================================== */
{
  struct xvimage * g;
  if (IndexPileGrilles2d >= NBGRILLES2D)
  {   
    fprintf(stderr, "AllocGrille2d() : pile pleine\n");
    exit(0);
  }
  g = PileGrilles2d[IndexPileGrilles2d];
  IndexPileGrilles2d++;
  return g;
} /* AllocGrille2d() */

/* ========================================== */
void FreeGrille2d()
/* ========================================== */
{
  if (IndexPileGrilles2d <= 0)
  {   
    fprintf(stderr, "FreeGrille2d() : pile vide\n");
    exit(0);
  }
  IndexPileGrilles2d --;
} /* FreeGrille2d() */

/* ========================================================================== */
/* FONCTIONS POUR LE 2D */
/* ========================================================================== */

/* ==================================== */
struct xvimage * Khalimskize2d(struct xvimage *o)
/* ==================================== */
/*            
   o: image originale
   k: resultat - chaque pixel de o est devenu un beta-terminal de k
*/
{
  index_t ors = rowsize(o);
  index_t ocs = colsize(o);
  uint8_t *O = UCHARDATA(o);
  struct xvimage *k;
  index_t krs, kcs, kN;
  uint8_t *K;
  index_t i, j;  

  assert(datatype(o) == VFF_TYP_1_BYTE);

  krs = 2 * ors + 1;
  kcs = 2 * ocs + 1;
  kN = krs * kcs;
  
  k = allocimage(NULL, krs, kcs, 1, VFF_TYP_1_BYTE);
  if (k == NULL)
  {   fprintf(stderr,"Khalimskize2d() : allocimage failed\n");
      return NULL;
  }
  K = UCHARDATA(k);

  memset(K, VAL_NULLE, kN); /* init a VAL_NULLE */

  for (j = 0; j < ocs; j++)
    for (i = 0; i < ors; i++)
      if (O[j * ors + i])
        K[(2*j+1) * krs + (2*i+1)] = VAL_OBJET;

  return k;
} /* Khalimskize2d() */

/* ==================================== */
struct xvimage * KhalimskizeNDG2d(struct xvimage *o)
/* ==================================== */
/*            
   o: image originale
   b: resultat - chaque pixel de o est devenu un beta-terminal de b
                 et sa valeur de gris a ete transmise. les ndg des
                 autres points sont a 0.
*/
{
#undef F_NAME
#define F_NAME "KhalimskizeNDG2d"
  index_t ors = rowsize(o);
  index_t ocs = colsize(o);
  struct xvimage *b;
  index_t brs, bcs, bN;
  index_t i, j;

  brs = 2 * ors + 1;
  bcs = 2 * ocs + 1;
  bN = brs * bcs;
  
  b = allocimage(NULL, brs, bcs, 1, datatype(o));
  if (b == NULL)
  {   fprintf(stderr,"%s: malloc failed\n", F_NAME);
      return NULL;
  }

  if (datatype(b) == VFF_TYP_1_BYTE)
  {
    uint8_t *O = UCHARDATA(o);
    uint8_t *B = UCHARDATA(b);
    memset(B, 0, bN*sizeof(uint8_t));
    for (j = 0; j < ocs; j++)
      for (i = 0; i < ors; i++)
	B[(2*j+1) * brs + (2*i+1)] = O[j * ors + i];
  }
  else if (datatype(b) == VFF_TYP_4_BYTE)
  {
    int32_t *O = SLONGDATA(o);
    int32_t *B = SLONGDATA(b);
    memset(B, 0, bN*sizeof(uint32_t));
    for (j = 0; j < ocs; j++)
      for (i = 0; i < ors; i++)
	B[(2*j+1) * brs + (2*i+1)] = O[j * ors + i];
  }
  else if (datatype(b) == VFF_TYP_FLOAT)
  {
    float *O = FLOATDATA(o);
    float *B = FLOATDATA(b);
    memset(B, 0, bN*sizeof(float));
    for (j = 0; j < ocs; j++)
      for (i = 0; i < ors; i++)
	B[(2*j+1) * brs + (2*i+1)] = O[j * ors + i];
  }
  else 
  {
    fprintf(stderr, "%s: bad datatype\n", F_NAME);
    exit(0);
  }
  return b;
} /* KhalimskizeNDG2d() */

/* ==================================== */
struct xvimage * KhalimskizeNDG2d_OLD(struct xvimage *o)
/* ==================================== */
/*            
   o: image originale
   k: resultat - chaque pixel de o est devenu un beta-terminal de k
                 et sa valeur de gris a ete transmise. les ndg des
                 autres points sont a 0.
*/
{
  index_t ors = rowsize(o);
  index_t ocs = colsize(o);
  uint8_t *O = UCHARDATA(o);
  struct xvimage *k;
  index_t krs, kcs, kN;
  uint8_t *K;
  index_t i, j;  

  krs = 2 * ors + 1;
  kcs = 2 * ocs + 1;
  kN = krs * kcs;
  
  k = allocimage(NULL, krs, kcs, 1, VFF_TYP_1_BYTE);
  if (k == NULL)
  {   fprintf(stderr,"Khalimskize2d() : allocimage failed\n");
      return NULL;
  }
  K = UCHARDATA(k);

  memset(K, VAL_NULLE, kN); /* init a VAL_NULLE */

  for (j = 0; j < ocs; j++)
    for (i = 0; i < ors; i++)
      K[(2*j+1) * krs + (2*i+1)] = O[j * ors + i];

  return k;
} /* KhalimskizeNDG2d() */

/* ==================================== */
struct xvimage * DeKhalimskize2d(struct xvimage *o)
/* ==================================== */
/*            
   o: image originale dans la grille de Khalimsky
   r: resultat - chaque beta-terminal de k devient un pixel de r
*/
{
  index_t ors = rowsize(o);
  index_t ocs = colsize(o);
  uint8_t *O = UCHARDATA(o);
  struct xvimage *r;
  index_t rrs, rcs, rN;
  uint8_t *R;
  index_t i, j;  

  rrs = ors / 2;
  rcs = ocs / 2;
  rN = rrs * rcs;
  
  r = allocimage(NULL, rrs, rcs, 1, VFF_TYP_1_BYTE);
  if (r == NULL)
  {   fprintf(stderr,"DeKhalimskize2d() : allocimage failed\n");
      return NULL;
  }
  R = UCHARDATA(r);

  memset(R, VAL_NULLE, rN); /* init a VAL_NULLE */

  for (j = 1; j < ocs; j += 2)
    for (i = 1; i < ors; i += 2)
      R[(j/2) * rrs + (i/2)] = O[j * ors + i];
  // version "binaire":
      //      if (O[j * ors + i])
      //        R[(j/2) * rrs + (i/2)] = VAL_OBJET;

  return r;
} /* DeKhalimskize2d() */

/* ==================================== */
void Khalimskize2d_noalloc(struct xvimage *o, struct xvimage * k)
/* ==================================== */
/*            
   o: image originale
   k: resultat - chaque pixel de o est devenu un beta-terminal de k
   \warning pas de vérification de taille
*/
{
  index_t ors = rowsize(o);
  index_t ocs = colsize(o);
  uint8_t *O = UCHARDATA(o);
  index_t krs, kcs, kN;
  uint8_t *K;
  index_t i, j;  

  krs = 2 * ors + 1;
  kcs = 2 * ocs + 1;
  kN = krs * kcs;
  K = UCHARDATA(k);

  memset(K, VAL_NULLE, kN); /* init a VAL_NULLE */

  for (j = 0; j < ocs; j++)
    for (i = 0; i < ors; i++)
      if (O[j * ors + i])
        K[(2*j+1) * krs + (2*i+1)] = VAL_OBJET;
} /* Khalimskize2d_noalloc() */

/* ==================================== */
void KhalimskizeNDG2d_noalloc(struct xvimage *o, struct xvimage * k)
/* ==================================== */
/*            
   o: image originale
   k: resultat - chaque pixel de o est devenu un beta-terminal de k
                 et sa valeur de gris a ete transmise. les ndg des
                 autres points sont a 0.
   \warning pas de vérification de taille
*/
{
  index_t ors = rowsize(o);
  index_t ocs = colsize(o);
  uint8_t *O = UCHARDATA(o);
  index_t krs, kcs, kN;
  uint8_t *K;
  index_t i, j;  

  krs = 2 * ors + 1;
  kcs = 2 * ocs + 1;
  kN = krs * kcs;
  K = UCHARDATA(k);

  memset(K, VAL_NULLE, kN); /* init a VAL_NULLE */

  for (j = 0; j < ocs; j++)
    for (i = 0; i < ors; i++)
      K[(2*j+1) * krs + (2*i+1)] = O[j * ors + i];
} /* KhalimskizeNDG2d_noalloc() */

/* ==================================== */
void DeKhalimskize2d_noalloc(struct xvimage *o, struct xvimage * r)
/* ==================================== */
/*            
   o: image originale dans la grille de Khalimsky
   r: resultat - chaque beta-terminal de k devient un pixel de r
   \warning pas de vérification de taille
*/
{
  index_t ors = rowsize(o);
  index_t ocs = colsize(o);
  uint8_t *O = UCHARDATA(o);
  index_t rrs, rcs, rN;
  uint8_t *R;
  index_t i, j;  

  rrs = ors / 2;
  rcs = ocs / 2;
  rN = rrs * rcs;
  R = UCHARDATA(r);

  memset(R, VAL_NULLE, rN); /* init a VAL_NULLE */

  for (j = 1; j < ocs; j += 2)
    for (i = 1; i < ors; i += 2)
      R[(j/2) * rrs + (i/2)] = O[j * ors + i];
  // version "binaire":
      //      if (O[j * ors + i])
      //        R[(j/2) * rrs + (i/2)] = VAL_OBJET;
} /* DeKhalimskize2d_noalloc() */

/* ==================================== */
void Connex8Obj2d(struct xvimage *k)
/* ==================================== */
{
  SatureAlphacarre2d(k);
  EffaceLiensLibres2d(k);
} /* Connex8Obj2d() */

/* ==================================== */
void Connex4Obj2d(struct xvimage *k)
/* ==================================== */
{
  index_t rs = rowsize(k);
  index_t cs = colsize(k);
  uint8_t *K = UCHARDATA(k);
  index_t i, j, x, y, u;
  int32_t n;
  index_t tab[GRS2D*GCS2D];

  for (j = 1; j < cs-1; j++)
    for (i = 1; i < rs-1; i++)
      if (!CARRE(i,j))
      {
        Betacarre2d(rs, cs, i, j, tab, &n);
        for (u = 0; u < n; u++)
        {
          x = tab[u] % rs;
          y = tab[u] / rs;
          if ((CARRE(x,y)) && !K[tab[u]]) goto skip;
	}
        K[j*rs+i] = VAL_OBJET;
skip: ;
      }
} /* Connex4Obj2d() */

/* ==================================== */
void ndgmin2d(struct xvimage *b)
/* ==================================== */
{
#undef F_NAME
#define F_NAME "ndgmin2d"
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t N = rs * cs;
  struct xvimage *bp;
  index_t i, j, u;
  int32_t n;
  index_t tab[GRS2D*GCS2D];

  if (datatype(b) == VFF_TYP_1_BYTE)
  {
    uint8_t *B;
    uint8_t *BP;
    B = UCHARDATA(b);
    bp = copyimage(b);
    BP = UCHARDATA(bp);
    memset(BP, NDG_MAX, N);

    for (j = 1; j < cs; j += 2)
      for (i = 1; i < rs; i += 2)
      {
	BP[j * rs + i] = B[j * rs + i];
	Alphacarre2d(rs, cs, i, j, tab, &n);
	for (u = 0; u < n; u++) BP[tab[u]] = mcmin(BP[tab[u]],B[j * rs + i]);
      }
    memcpy(B, BP, N*sizeof(uint8_t));
    freeimage(bp);
  }
  else if (datatype(b) == VFF_TYP_4_BYTE)
  {
    int32_t *B;
    int32_t *BP;
    B = SLONGDATA(b);
    bp = copyimage(b);
    BP = SLONGDATA(bp);
    for (j = 1; j < N; j += 1) BP[j] = INT32_MAX;

    for (j = 1; j < cs; j += 2)
      for (i = 1; i < rs; i += 2)
      {
	BP[j * rs + i] = B[j * rs + i];
	Alphacarre2d(rs, cs, i, j, tab, &n);
	for (u = 0; u < n; u++) BP[tab[u]] = mcmin(BP[tab[u]],B[j * rs + i]);
      }
    memcpy(B, BP, N*sizeof(uint32_t));
    freeimage(bp);
  }
  else if (datatype(b) == VFF_TYP_FLOAT)
  {
    float *B;
    float *BP;
    B = FLOATDATA(b);
    bp = copyimage(b);
    BP = FLOATDATA(bp);
    for (j = 1; j < N; j += 1) BP[j] = FLT_MAX;

    for (j = 1; j < cs; j += 2)
      for (i = 1; i < rs; i += 2)
      {
	BP[j * rs + i] = B[j * rs + i];
	Alphacarre2d(rs, cs, i, j, tab, &n);
	for (u = 0; u < n; u++) BP[tab[u]] = mcmin(BP[tab[u]],B[j * rs + i]);
      }
    memcpy(B, BP, N*sizeof(float));
    freeimage(bp);
  }
  else 
  {
    fprintf(stderr, "%s: bad datatype\n", F_NAME);
    exit(0);
  }

} /* ndgmin2d() */

/* ==================================== */
void ndgmin2d_OLD(struct xvimage *k)
/* ==================================== */
/*
  Entree: une fonction k de H2 dans [0..255] dont
          seules les valeurs des beta-terminaux (carres) sont significatives.
  Sortie: une fonction kp de H2 dans [0..255].
          Tous les points x non beta-terminaux
          ont recu la valeur min{k[y], y beta-terminal dans betacarre[x]}
*/
{
  index_t rs = rowsize(k);
  index_t cs = colsize(k);
  uint8_t *K = UCHARDATA(k);
  struct xvimage *kp;
  uint8_t *KP;
  index_t i, j, u;
  int32_t n;
  index_t tab[9];

  kp = copyimage(k);
  KP = UCHARDATA(kp);
  memset(KP, NDG_MAX, rs*cs);

  for (j = 1; j < cs; j += 2)
    for (i = 1; i < rs; i += 2)
    {
      KP[j * rs + i] = K[j * rs + i];
      Alphacarre2d(rs, cs, i, j, tab, &n);
      for (u = 0; u < n; u++) KP[tab[u]] = mcmin(KP[tab[u]],K[j * rs + i]);
    }
  memcpy(K, KP, rs*cs);
  freeimage(kp);
} /* ndgmin2d() */

/* ==================================== */
void ndgminbeta2d(struct xvimage *k)
/* ==================================== */
/*
  Entree: une fonction k de H2 dans [0..255] dont
          seules les valeurs des alpha-terminaux (singletons) sont significatives.
  Sortie: une fonction kp de H2 dans [0..255].
          Tous les points x non alpha-terminaux
          ont recu la valeur min{k[y], y alpha-terminal dans alphacarre[x]}
*/
{
  index_t rs = rowsize(k);
  index_t cs = colsize(k);
  uint8_t *K = UCHARDATA(k);
  struct xvimage *kp;
  uint8_t *KP;
  index_t i, j, u;
  int32_t n;
  index_t tab[9];

  kp = copyimage(k);
  KP = UCHARDATA(kp);
  memset(KP, NDG_MAX, rs*cs);

  for (j = 0; j < cs; j += 2)
    for (i = 0; i < rs; i += 2)
    {
      KP[j * rs + i] = K[j * rs + i];
      Betacarre2d(rs, cs, i, j, tab, &n);
      for (u = 0; u < n; u++) KP[tab[u]] = mcmin(KP[tab[u]],K[j * rs + i]);
    }
  memcpy(K, KP, rs*cs);
  freeimage(kp);
} /* ndgminbeta2d() */

/* ==================================== */
void ndgmax2d(struct xvimage *b)
/* ==================================== */
{
#undef F_NAME
#define F_NAME "ndgmax2d"
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t N = rs * cs;
  struct xvimage *bp;
  index_t i, j, u;
  int32_t n;
  index_t tab[GRS2D*GCS2D];

  if (datatype(b) == VFF_TYP_1_BYTE)
  {
    uint8_t *B;
    uint8_t *BP;
    B = UCHARDATA(b);
    bp = copyimage(b);
    BP = UCHARDATA(bp);
    memset(BP, NDG_MIN, N);

    for (j = 1; j < cs; j += 2)
      for (i = 1; i < rs; i += 2)
      {
	BP[j * rs + i] = B[j * rs + i];
	Alphacarre2d(rs, cs, i, j, tab, &n);
	for (u = 0; u < n; u++) BP[tab[u]] = mcmax(BP[tab[u]],B[j * rs + i]);
      }
    memcpy(B, BP, N*sizeof(uint8_t));
    freeimage(bp);
  }
  else if (datatype(b) == VFF_TYP_4_BYTE)
  {
    int32_t *B;
    int32_t *BP;
    B = SLONGDATA(b);
    bp = copyimage(b);
    BP = SLONGDATA(bp);
    for (j = 1; j < N; j += 1) BP[j] = 0;

    for (j = 1; j < cs; j += 2)
      for (i = 1; i < rs; i += 2)
      {
	BP[j * rs + i] = B[j * rs + i];
	Alphacarre2d(rs, cs, i, j, tab, &n);
	for (u = 0; u < n; u++) BP[tab[u]] = mcmax(BP[tab[u]],B[j * rs + i]);
      }
    memcpy(B, BP, N*sizeof(uint32_t));
    freeimage(bp);
  }
  else if (datatype(b) == VFF_TYP_FLOAT)
  {
    float *B;
    float *BP;
    B = FLOATDATA(b);
    bp = copyimage(b);
    BP = FLOATDATA(bp);
    for (j = 1; j < N; j += 1) BP[j] = 0.0;

    for (j = 1; j < cs; j += 2)
      for (i = 1; i < rs; i += 2)
      {
	BP[j * rs + i] = B[j * rs + i];
	Alphacarre2d(rs, cs, i, j, tab, &n);
	for (u = 0; u < n; u++) BP[tab[u]] = mcmax(BP[tab[u]],B[j * rs + i]);
      }
    memcpy(B, BP, N*sizeof(float));
    freeimage(bp);
  }
  else 
  {
    fprintf(stderr, "%s: bad datatype\n", F_NAME);
    exit(0);
  }

} /* ndgmax2d() */

/* ==================================== */
void ndgmax2d_OLD(struct xvimage *k)
/* ==================================== */
/*
  Entree: une fonction k de H2 dans [0..255] dont
          seules les valeurs des beta-terminaux (carres) sont significatives.
  Sortie: une fonction kp de H2 dans [0..255].
          Tous les points x non beta-terminaux 
          ont recu la valeur max{k[y], y beta-terminal dans betacarre[x]}
*/
{
  index_t rs = rowsize(k);
  index_t cs = colsize(k);
  uint8_t *K = UCHARDATA(k);
  struct xvimage *kp;
  uint8_t *KP;
  index_t i, j, u;
  int32_t n;
  index_t tab[9];

  kp = copyimage(k);
  KP = UCHARDATA(kp);
  memset(KP, NDG_MIN, rs*cs);

  for (j = 1; j < cs; j += 2)
    for (i = 1; i < rs; i += 2)
    {
      KP[j * rs + i] = K[j * rs + i];
      Alphacarre2d(rs, cs, i, j, tab, &n);
      for (u = 0; u < n; u++) KP[tab[u]] = mcmax(KP[tab[u]],K[j * rs + i]);
    }
  memcpy(K, KP, rs*cs);
  freeimage(kp);
} /* ndgmax2d() */

/* ==================================== */
void ndgmaxbeta2d(struct xvimage *k)
/* ==================================== */
/*
  Entree: une fonction k de H2 dans [0..255] dont
          seules les valeurs des alpha-terminaux (singletons) sont significatives.
  Sortie: une fonction kp de H2 dans [0..255].
          Tous les points x non alpha-terminaux 
          ont recu la valeur max{k[y], y alpha-terminal dans alphacarre[x]}
*/
{
  index_t rs = rowsize(k);
  index_t cs = colsize(k);
  uint8_t *K = UCHARDATA(k);
  struct xvimage *kp;
  uint8_t *KP;
  index_t i, j, u;
  int32_t n;
  index_t tab[9];

  kp = copyimage(k);
  KP = UCHARDATA(kp);
  memset(KP, NDG_MIN, rs*cs);

  for (j = 0; j < cs; j += 2)
    for (i = 0; i < rs; i += 2)
    {
      KP[j * rs + i] = K[j * rs + i];
      Betacarre2d(rs, cs, i, j, tab, &n);
      for (u = 0; u < n; u++) KP[tab[u]] = mcmax(KP[tab[u]],K[j * rs + i]);
    }
  memcpy(K, KP, rs*cs);
  freeimage(kp);
} /* ndgmaxbeta2d() */

/* ==================================== */
void ndgmoy2d(struct xvimage *b)
/* ==================================== */
/*
  Entree: une fonction b de H2 dans [0..255] dont
          seules les valeurs des beta-terminaux (carres) sont significatives.
  Sortie: une fonction bp de H2 dans [0..255].
          Tous les points x non beta-terminaux 
          ont recu la valeur moy{k[y], y beta-terminal dans betacarre[x]}
*/
{
#undef F_NAME
#define F_NAME "ndgmoy2d"
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t i, j, u;
  int32_t n, nb;
  index_t tab[GRS2D*GCS2D];

  if (datatype(b) == VFF_TYP_1_BYTE)
  {
    uint8_t *B;
    uint32_t sum;
    B = UCHARDATA(b);

    for (j = 0; j < cs; j++)
    for (i = 0; i < rs; i++)
      if (!CARRE(i,j))
      {
        nb = sum = 0;
        Betacarre2d(rs, cs, i, j, tab, &n);
        for (u = 0; u < n; u++) 
          if (CARRE((tab[u]%rs),(tab[u]/rs)))
          {
            sum += B[tab[u]];
            nb++;
	  }
        B[j * rs + i] = (uint8_t)(sum/nb);
      }
  }
  else if (datatype(b) == VFF_TYP_4_BYTE)
  {
    int32_t *B;
    int32_t sum;
    B = SLONGDATA(b);

    for (j = 0; j < cs; j++)
    for (i = 0; i < rs; i++)
      if (!CARRE(i,j))
      {
        nb = sum = 0;
        Betacarre2d(rs, cs, i, j, tab, &n);
        for (u = 0; u < n; u++) 
          if (CARRE((tab[u]%rs),(tab[u]/rs)))
          {
            sum += B[tab[u]];
            nb++;
	  }
        B[j * rs + i] = sum / nb;
      }
  }
  else if (datatype(b) == VFF_TYP_FLOAT)
  {
    float *B;
    float sum;
    B = FLOATDATA(b);

    for (j = 0; j < cs; j++)
    for (i = 0; i < rs; i++)
      if (!CARRE(i,j))
      {
        nb = sum = 0;
        Betacarre2d(rs, cs, i, j, tab, &n);
        for (u = 0; u < n; u++) 
          if (CARRE((tab[u]%rs),(tab[u]/rs)))
          {
            sum += B[tab[u]];
            nb++;
	  }
        B[j * rs + i] = sum / nb;
      }
  }
  else 
  {
    fprintf(stderr, "%s: bad datatype\n", F_NAME);
    exit(0);
  }

} /* ndgmoy2d() */

/* ==================================== */
void ndg2grad2d(struct xvimage *k)
/* ==================================== */
/*
  Entree: une fonction k de H2 dans [0..255] dont
          seules les valeurs des beta-terminaux (carres) sont significatives.
  Sortie: une fonction kp de H2 dans [0..255].
          Si x carre, alors kp[x] = 0
          Si x intervalle, alors kp[x] = difference (en val. abs.) des k des carres dans beta(x).
          Si x singleton, alors kp[x] = max des valeurs de kp pour les intervalles dans beta(x).
*/
{
  index_t rs = rowsize(k);
  index_t cs = colsize(k);
  uint8_t *K = UCHARDATA(k);
  struct xvimage *kp;
  uint8_t *KP;
  index_t i, j, u;
  int32_t n;
  index_t tab[9];

  kp = copyimage(k);
  KP = UCHARDATA(kp);
  memset(KP, NDG_MIN, rs*cs);

  for (j = 0; j < cs; j++)
    for (i = 0; i < rs; i++)
    {
      if (CARRE(i,j))
      {
        KP[j * rs + i] = 0;
      }
      else if (INTER(i,j))
      {
        Betacarre2d(rs, cs, i, j, tab, &n);
        if (n == 2)
	{
          KP[j * rs + i] = mcabs(K[tab[0]]-K[tab[1]]);  
        }
        else if (n == 1)
	{
          KP[j * rs + i] = K[tab[0]]; /* hyp. bord a 0 */
        }
        Alphacarre2d(rs, cs, i, j, tab, &n);
        for (u = 0; u < n; u++) KP[tab[u]] = mcmax(KP[tab[u]],KP[j*rs+i]);
      }
    }
  memcpy(K, KP, rs*cs);
  freeimage(kp);

} /* ndg2grad2d() */

/* ==================================== */
void ndg4grad2d(struct xvimage *k)
/* ==================================== */
/*
  Entree: une fonction k de H2 dans [0..255] dont
          seules les valeurs des beta-terminaux (carres) sont significatives.
  Sortie: une fonction kp de H2 dans [0..255].
          Si x carre ou intervalle, alors kp[x] = 0
          Si x singleton, alors kp[x] = difference entre le sup et le inf des k des carres dans beta(x).
*/
{
  index_t rs = rowsize(k);
  index_t cs = colsize(k);
  uint8_t *K = UCHARDATA(k);
  struct xvimage *kp;
  uint8_t *KP;
  index_t i, j;
  int32_t u, ui, uj, n;
  index_t tab[9];
  int32_t sup, inf, tmp;

  kp = copyimage(k);
  KP = UCHARDATA(kp);
  memset(KP, NDG_MIN, rs*cs);

  for (j = 0; j < cs; j++)
    for (i = 0; i < rs; i++)
    {
      if (SINGL(i,j))
      {
        Betacarre2d(rs, cs, i, j, tab, &n);
        sup = NDG_MIN; inf = NDG_MAX;
        for (u = 0; u < n; u++)
	{
          ui = tab[u] % rs;
          uj = tab[u] / rs;
          if (CARRE(ui,uj))
	  {
            tmp = K[tab[u]];
            if (tmp > sup) sup = tmp; 
            if (tmp < inf) inf = tmp;
	  }
	}
        KP[j * rs + i] = sup  - inf;
      }
    }
  memcpy(K, KP, rs*cs);
  freeimage(kp);

} /* ndg4grad2d() */

/* ==================================== */
void Betapoint2d(index_t rs, index_t cs, index_t i, index_t j, index_t *tab, int32_t *n)
/* ==================================== */
/* 
  retourne dans tab la beta-proximite de (i,j).
  *n est le cardinal de cet ensemble
  tab doit etre alloue a l'avance
*/
{
  *n = 0;
  if (i % 2)
  {
    if (j % 2)
    {          /* i impair, j impair : carre */
    }
    else
    {          /* i impair, j pair : intervalle */
      if (j > 0)      tab[(*n)++] = (j-1) * rs + i;      
      if ((j+1) < cs) tab[(*n)++] = (j+1) * rs + i;      
    }
  }
  else
  {
    if (j % 2)
    {          /* i pair, j impair : intervalle */
      if (i > 0)      tab[(*n)++] = j * rs + i - 1;      
      if ((i+1) < rs) tab[(*n)++] = j * rs + i + 1;      
    }
    else
    {          /* i pair, j pair : singleton */
      if (i > 0)      tab[(*n)++] = j * rs + i - 1;      
      if ((i+1) < rs) tab[(*n)++] = j * rs + i + 1;      
      if (j > 0)      tab[(*n)++] = (j-1) * rs + i;      
      if ((j+1) < cs) tab[(*n)++] = (j+1) * rs + i;      
    }
  }
} /* Betapoint2d() */

/* ==================================== */
void Alphapoint2d(index_t rs, index_t cs, index_t i, index_t j, index_t *tab, int32_t *n)
/* ==================================== */
/* 
  retourne dans tab l'alpha-proximite de (i,j).
  *n est le cardinal de cet ensemble
  tab doit etre alloue a l'avance
*/
{
  *n = 0;
  if (i % 2)
  {
    if (j % 2)
    {          /* i impair, j impair : carre */
      if (i > 0)      tab[(*n)++] = j * rs + i - 1;
      if ((i+1) < rs) tab[(*n)++] = j * rs + i + 1;
      if (j > 0)      tab[(*n)++] = (j-1) * rs + i;      
      if ((j+1) < cs) tab[(*n)++] = (j+1) * rs + i;      
    }
    else
    {          /* i impair, j pair : intervalle */
      if (i > 0)      tab[(*n)++] = j * rs + i - 1;
      if ((i+1) < rs) tab[(*n)++] = j * rs + i + 1;
    }
  }
  else
  {
    if (j % 2)
    {          /* i pair, j impair : intervalle */
      if (j > 0)      tab[(*n)++] = (j-1) * rs + i;      
      if ((j+1) < cs) tab[(*n)++] = (j+1) * rs + i;      
    }
    else
    {          /* i pair, j pair : singleton */
    }
  }
} /* Alphapoint2d() */

/* ==================================== */
void Betacarre2d(index_t rs, index_t cs, index_t i, index_t j, index_t *tab, int32_t *n)
/* ==================================== */
/* 
  retourne dans tab la beta-adherence de (i,j) privee de (i,j) DANS LA GRILLE.
  *n est le cardinal de cet ensemble
  tab doit etre alloue a l'avance
*/
{
  *n = 0;
  if (i % 2)
  {
    if (j % 2)
    {          /* i impair, j impair : carre */
    }
    else
    {          /* i impair, j pair : intervalle */
      if (j > 0)      tab[(*n)++] = (j-1) * rs + i;      
      if ((j+1) < cs) tab[(*n)++] = (j+1) * rs + i;      
    }
  }
  else
  {
    if (j % 2)
    {          /* i pair, j impair : intervalle */
      if (i > 0)      tab[(*n)++] = j * rs + i - 1;      
      if ((i+1) < rs) tab[(*n)++] = j * rs + i + 1;      
    }
    else
    {          /* i pair, j pair : singleton */
      if (i > 0)      tab[(*n)++] = j * rs + i - 1;      
      if ((i+1) < rs) tab[(*n)++] = j * rs + i + 1;      
      if (j > 0)      tab[(*n)++] = (j-1) * rs + i;      
      if ((j+1) < cs) tab[(*n)++] = (j+1) * rs + i;      
      if ((i > 0) && (j > 0))           tab[(*n)++] = (j-1) * rs + i - 1;      
      if (((i+1) < rs) && (j > 0))      tab[(*n)++] = (j-1) * rs + i + 1;      
      if ((i > 0) && ((j+1) < cs))      tab[(*n)++] = (j+1) * rs + i - 1;      
      if (((i+1) < rs) && ((j+1) < cs)) tab[(*n)++] = (j+1) * rs + i + 1;      
    }
  }
} /* Betacarre2d() */

/* ==================================== */
void Alphacarre2d(index_t rs, index_t cs, index_t i, index_t j, index_t *tab, int32_t *n)
/* ==================================== */
/* 
  retourne dans tab l'alpha-adherence de (i,j) privee de (i,j) DANS LA GRILLE.
  *n est le cardinal de cet ensemble
  tab doit etre alloue a l'avance
*/
{
  *n = 0;
  if (i % 2)
  {
    if (j % 2)
    {          /* i impair, j impair : carre */
      if (i > 0)      tab[(*n)++] = j * rs + i - 1;      
      if ((i+1) < rs) tab[(*n)++] = j * rs + i + 1;      
      if (j > 0)      tab[(*n)++] = (j-1) * rs + i;      
      if ((j+1) < cs) tab[(*n)++] = (j+1) * rs + i;      
      if ((i > 0) && (j > 0))           tab[(*n)++] = (j-1) * rs + i - 1;      
      if (((i+1) < rs) && (j > 0))      tab[(*n)++] = (j-1) * rs + i + 1;      
      if ((i > 0) && ((j+1) < cs))      tab[(*n)++] = (j+1) * rs + i - 1;      
      if (((i+1) < rs) && ((j+1) < cs)) tab[(*n)++] = (j+1) * rs + i + 1;      
    }
    else
    {          /* i impair, j pair : intervalle */
      if (i > 0)      tab[(*n)++] = j * rs + i - 1;      
      if ((i+1) < rs) tab[(*n)++] = j * rs + i + 1;      
    }
  }
  else
  {
    if (j % 2)
    {          /* i pair, j impair : intervalle */
      if (j > 0)      tab[(*n)++] = (j-1) * rs + i;      
      if ((j+1) < cs) tab[(*n)++] = (j+1) * rs + i;      
    }
    else
    {          /* i pair, j pair : singleton */
    }
  }
} /* Alphacarre2d() */

/* ==================================== */
void Thetacarre2d(index_t rs, index_t cs, index_t i, index_t j, index_t *tab, int32_t *n)
/* ==================================== */
/* 
  retourne dans tab la Theta-adherence de (i,j) privee de (i,j) DANS LA GRILLE.
  *n est le cardinal de cet ensemble
  tab doit etre alloue a l'avance
*/
{
  *n = 0;
  if (i % 2) /* calcule betacarre */
  {
    if (j % 2)
    {          /* i impair, j impair : carre */
    }
    else
    {          /* i impair, j pair : intervalle */
      if (j > 0)      tab[(*n)++] = (j-1) * rs + i;      
      if ((j+1) < cs) tab[(*n)++] = (j+1) * rs + i;      
    }
  }
  else
  {
    if (j % 2)
    {          /* i pair, j impair : intervalle */
      if (i > 0)      tab[(*n)++] = j * rs + i - 1;      
      if ((i+1) < rs) tab[(*n)++] = j * rs + i + 1;      
    }
    else
    {          /* i pair, j pair : singleton */
      if (i > 0)      tab[(*n)++] = j * rs + i - 1;      
      if ((i+1) < rs) tab[(*n)++] = j * rs + i + 1;      
      if (j > 0)      tab[(*n)++] = (j-1) * rs + i;      
      if ((j+1) < cs) tab[(*n)++] = (j+1) * rs + i;      
      if ((i > 0) && (j > 0))           tab[(*n)++] = (j-1) * rs + i - 1;      
      if (((i+1) < rs) && (j > 0))      tab[(*n)++] = (j-1) * rs + i + 1;      
      if ((i > 0) && ((j+1) < cs))      tab[(*n)++] = (j+1) * rs + i - 1;      
      if (((i+1) < rs) && ((j+1) < cs)) tab[(*n)++] = (j+1) * rs + i + 1;      
    }
  }

  if (i % 2) /* calcule alphacarre */
  {
    if (j % 2)
    {          /* i impair, j impair : carre */
      if (i > 0)      tab[(*n)++] = j * rs + i - 1;      
      if ((i+1) < rs) tab[(*n)++] = j * rs + i + 1;      
      if (j > 0)      tab[(*n)++] = (j-1) * rs + i;      
      if ((j+1) < cs) tab[(*n)++] = (j+1) * rs + i;      
      if ((i > 0) && (j > 0))           tab[(*n)++] = (j-1) * rs + i - 1;      
      if (((i+1) < rs) && (j > 0))      tab[(*n)++] = (j-1) * rs + i + 1;      
      if ((i > 0) && ((j+1) < cs))      tab[(*n)++] = (j+1) * rs + i - 1;      
      if (((i+1) < rs) && ((j+1) < cs)) tab[(*n)++] = (j+1) * rs + i + 1;      
    }
    else
    {          /* i impair, j pair : intervalle */
      if (i > 0)      tab[(*n)++] = j * rs + i - 1;      
      if ((i+1) < rs) tab[(*n)++] = j * rs + i + 1;      
    }
  }
  else
  {
    if (j % 2)
    {          /* i pair, j impair : intervalle */
      if (j > 0)      tab[(*n)++] = (j-1) * rs + i;      
      if ((j+1) < cs) tab[(*n)++] = (j+1) * rs + i;      
    }
    else
    {          /* i pair, j pair : singleton */
    }
  }
} /* Thetacarre2d() */

/* ==================================== */
int32_t CardBetapoint2d(uint8_t *K, index_t rs, index_t cs, index_t i, index_t j)
/* ==================================== */
{
  int32_t n = 0;
  if (i % 2)
  {
    if (j % 2)
    {          /* i impair, j impair : carre */
    }
    else
    {          /* i impair, j pair : intervalle */
      if ((j > 0) && K[(j-1) * rs + i]) n++;      
      if (((j+1) < cs) && K[(j+1) * rs + i]) n++;      
    }
  }
  else
  {
    if (j % 2)
    {          /* i pair, j impair : intervalle */
      if ((i > 0) && K[j * rs + i - 1]) n++;      
      if (((i+1) < rs) && K[j * rs + i + 1]) n++;      
    }
    else
    {          /* i pair, j pair : singleton */
      if ((i > 0) && K[j * rs + i - 1]) n++;
      if (((i+1) < rs) && K[j * rs + i + 1]) n++;      
      if ((j > 0) && K[(j-1) * rs + i]) n++;      
      if (((j+1) < cs) && K[(j+1) * rs + i]) n++;      

      if ((i > 0) && (j > 0) && !K[j*rs+i-1] && !K[(j-1)*rs+i] && K[(j-1)*rs+i-1]) n++;
      if (((i+1) < rs) && (j > 0) && !K[j*rs+i+1] && !K[(j-1)*rs+i] && K[(j-1)*rs+i+1]) n++;
      if ((i > 0) && ((j+1) < cs) && !K[j*rs+i-1] && !K[(j+1)*rs+i] && K[(j+1)*rs+i-1]) n++;
      if (((i+1) < rs) && ((j+1) < cs) && !K[j*rs+i+1] && !K[(j+1)*rs+i] && K[(j+1)*rs+i+1]) n++;
    }
  }  
  return n;
} /* CardBetapoint() */

/* ==================================== */
int32_t CardThetacarre2d(struct xvimage *k, index_t i, index_t j)
/* ==================================== */
/*  OBSOLETE - utiliser CardThetaCarre2d */
{
  index_t rs = rowsize(k);
  index_t cs = colsize(k);
  uint8_t *K = UCHARDATA(k);
  int32_t u, n;
  index_t tab[GRS2D*GCS2D];
  int32_t card = 0;

  if (K[j * rs + i])
  {
    Alphacarre2d(rs, cs, i, j, tab, &n);
    for (u = 0; u < n; u++) if (K[tab[u]]) card++;
    Betacarre2d(rs, cs, i, j, tab, &n);
    for (u = 0; u < n; u++) if (K[tab[u]]) card++;
  }
  return card;
} /* CardThetacarre2d() */

/* ==================================== */
int32_t CardThetaCarre2d(struct xvimage *k, index_t i, index_t j, uint8_t val)
/* ==================================== */
/* 
   retourne le nombre d'elements dans le thetacarre de (i,j) 
   dont la valeur est egale a val
*/
{
  index_t rs = rowsize(k);
  index_t cs = colsize(k);
  uint8_t *K = UCHARDATA(k);
  int32_t u, n;
  index_t tab[GRS2D*GCS2D];
  int32_t card = 0;

  if (K[j * rs + i])
  {
    Alphacarre2d(rs, cs, i, j, tab, &n);
    for (u = 0; u < n; u++) if (K[tab[u]] == val) card++;
    Betacarre2d(rs, cs, i, j, tab, &n);
    for (u = 0; u < n; u++) if (K[tab[u]] == val) card++;
  }
  return card;
} /* CardThetaCarre2d() */

/* ==================================== */
int32_t BetaTerminal2d(uint8_t *K, index_t rs, index_t cs, index_t i, index_t j)
/* ==================================== */
{
  if (i % 2)
  {
    if (j % 2)
    {          /* i impair, j impair : carre */
      return 1;
    }
    {          /* i impair, j pair : intervalle */
      if ((j > 0) && K[(j-1) * rs + i]) return 0;
      if (((j+1) < cs) && K[(j+1) * rs + i]) return 0;      
    }
  }
  else
  {
    if (j % 2)
    {          /* i pair, j impair : intervalle */
      if ((i > 0) && K[j * rs + i - 1]) return 0;
      if (((i+1) < rs) && K[j * rs + i + 1]) return 0;
    }
    else
    {          /* i pair, j pair : singleton */
      if ((i > 0) && K[j * rs + i - 1]) return 0;
      if (((i+1) < rs) && K[j * rs + i + 1]) return 0;      
      if ((j > 0) && K[(j-1) * rs + i]) return 0;
      if (((j+1) < cs) && K[(j+1) * rs + i]) return 0;

      if ((i > 0) && (j > 0) && K[(j-1)*rs+i-1]) return 0;
      if (((i+1) < rs) && (j > 0) && K[(j-1)*rs+i+1]) return 0;
      if ((i > 0) && ((j+1) < cs) && K[(j+1)*rs+i-1]) return 0;
      if (((i+1) < rs) && ((j+1) < cs) && K[(j+1)*rs+i+1]) return 0;
    }
  }
  return 1;
} /* BetaTerminal2d() */

/* ==================================== */
int32_t ExactementUnBetaTerminal2d(uint8_t *K, index_t rs, index_t cs)
/* ==================================== */
{
  index_t i, N = rs * cs;
  int32_t n = 0;

  for (i = 0; i < N; i++)
    if ((K[i]) && BetaTerminal2d(K, rs, cs, i % rs, i / rs))
      if (++n > 1) return 0;
  if (n < 1) return 0; else return 1;
} /* ExactementUnBetaTerminal2d() */

/* ==================================== */
void SatureAlphacarre2d(struct xvimage *k)
/* ==================================== */
/*
  met à VAL_OBJET les points de l'alphacarre de tous les 2-éléments de k
  (et seulement des 2-éléments !)
 */
{
  index_t rs = rowsize(k);
  index_t cs = colsize(k);
  uint8_t *K = UCHARDATA(k);
  index_t i, j;
  int32_t u, n;
  index_t tab[GRS2D*GCS2D];

  for (j = 1; j < cs; j += 2)
    for (i = 1; i < rs; i += 2)
      if (K[j * rs + i])
      {
        Alphacarre2d(rs, cs, i, j, tab, &n);
        for (u = 0; u < n; u++) K[tab[u]] = VAL_OBJET;
      }
} /* SatureAlphacarre2d() */

/* ==================================== */
void AjouteAlphacarre2d(struct xvimage *k)
/* ==================================== */
/*
  retourne dans k l'ensemble des points de l'alpha de tous les éléments de k
 */
{
  index_t rs = rowsize(k);
  index_t cs = colsize(k);
  uint8_t *K = UCHARDATA(k);
  index_t i, j;
  int32_t u, n;
  index_t tab[GRS2D*GCS2D];

  for (j = 0; j < cs; j += 1)
    for (i = 0; i < rs; i += 1)
      if (K[j * rs + i])
      {
        Alphacarre2d(rs, cs, i, j, tab, &n);
        for (u = 0; u < n; u++) K[tab[u]] = VAL_OBJET;
      }
} /* AjouteAlphacarre2d() */

/* ==================================== */
void AjouteBetacarre2d(struct xvimage *k)
/* ==================================== */
/*
  retourne dans k l'ensemble des points du beta de tous les éléments de k
 */
{
  index_t rs = rowsize(k);
  index_t cs = colsize(k);
  uint8_t *K = UCHARDATA(k);
  index_t i, j;
  int32_t u, n;
  index_t tab[GRS2D*GCS2D];

  for (j = 0; j < cs; j += 1)
    for (i = 0; i < rs; i += 1)
      if (K[j * rs + i])
      {
        Betacarre2d(rs, cs, i, j, tab, &n);
        for (u = 0; u < n; u++) K[tab[u]] = VAL_OBJET;
      }
} /* AjouteBetacarre2d() */

/* ==================================== */
void MaxAlpha2d(struct xvimage *k)
/* ==================================== */
/*
  pour chaque element x, faire K[x] = max{K[y] | y in alpha(x)}
 */
#undef F_NAME
#define F_NAME "MaxAlpha2d"
{
  index_t rs = rowsize(k);
  index_t cs = colsize(k);
  index_t i, j;
  int32_t u, n;
  index_t tab[GRS2D*GCS2D];

  if (datatype(k) == VFF_TYP_1_BYTE)
  {
    uint8_t *K = UCHARDATA(k);
    uint8_t M;
    for (j = 0; j < cs; j += 1)
      for (i = 0; i < rs; i += 1)
      {
	M = K[j * rs + i];
        Alphacarre2d(rs, cs, i, j, tab, &n);
        for (u = 0; u < n; u++) 
	  if (K[tab[u]] > M) M = K[tab[u]];
	K[j * rs + i] = M;
      }
  }
  else if (datatype(k) == VFF_TYP_4_BYTE)
  {
    int32_t *K = SLONGDATA(k);
    int32_t M;
    for (j = 0; j < cs; j += 1)
      for (i = 0; i < rs; i += 1)
      {
	M = K[j * rs + i];
        Alphacarre2d(rs, cs, i, j, tab, &n);
        for (u = 0; u < n; u++) 
	  if (K[tab[u]] > M) M = K[tab[u]];
	K[j * rs + i] = M;
      }
  }
  else if (datatype(k) == VFF_TYP_FLOAT)
  {
    float *K = FLOATDATA(k);
    float M;
    for (j = 0; j < cs; j += 1)
      for (i = 0; i < rs; i += 1)
      {
	M = K[j * rs + i];
        Alphacarre2d(rs, cs, i, j, tab, &n);
        for (u = 0; u < n; u++) 
	  if (K[tab[u]] > M) M = K[tab[u]];
	K[j * rs + i] = M;
      }
  }
  else
  {
    fprintf(stderr, "%s: bad datatype\n", F_NAME);
    exit(0);
  }
} /* MaxAlpha2d() */

/* ==================================== */
void MaxBeta2d(struct xvimage *k)
/* ==================================== */
/*
  pour chaque element x, faire K[x] = max{K[y] | y in beta(x)}
 */
#undef F_NAME
#define F_NAME "MaxBeta2d"
{
  index_t rs = rowsize(k);
  index_t cs = colsize(k);
  index_t i, j;
  int32_t u, n;
  index_t tab[GRS2D*GCS2D];

  if (datatype(k) == VFF_TYP_1_BYTE)
  {
    uint8_t *K = UCHARDATA(k);
    uint8_t M;
    for (j = 0; j < cs; j += 1)
      for (i = 0; i < rs; i += 1)
      {
	M = K[j * rs + i];
        Betacarre2d(rs, cs, i, j, tab, &n);
        for (u = 0; u < n; u++) 
	  if (K[tab[u]] > M) M = K[tab[u]];
	K[j * rs + i] = M;
      }
  }
  else if (datatype(k) == VFF_TYP_4_BYTE)
  {
    int32_t *K = SLONGDATA(k);
    int32_t M;
    for (j = 0; j < cs; j += 1)
      for (i = 0; i < rs; i += 1)
      {
	M = K[j * rs + i];
        Betacarre2d(rs, cs, i, j, tab, &n);
        for (u = 0; u < n; u++) 
	  if (K[tab[u]] > M) M = K[tab[u]];
	K[j * rs + i] = M;
      }
  }
  else if (datatype(k) == VFF_TYP_FLOAT)
  {
    float *K = FLOATDATA(k);
    float M;
    for (j = 0; j < cs; j += 1)
      for (i = 0; i < rs; i += 1)
      {
	M = K[j * rs + i];
        Betacarre2d(rs, cs, i, j, tab, &n);
        for (u = 0; u < n; u++) 
	  if (K[tab[u]] > M) M = K[tab[u]];
	K[j * rs + i] = M;
      }
  }
  else
  {
    fprintf(stderr, "%s: bad datatype\n", F_NAME);
    exit(0);
  }
} /* MaxBeta2d() */

/* ==================================== */
void EffaceLiensLibres2d(struct xvimage *k)
/* ==================================== */
{
  index_t rs = rowsize(k);
  index_t cs = colsize(k);
  uint8_t *K = UCHARDATA(k);
  index_t i, j;

  for (j = 0; j < cs; j += 1)
    for (i = 0; i < rs; i += 1)
      if (K[j * rs + i] && INTER(i,j) && (CardBetapoint2d(K, rs, cs, i, j) == 1)) K[j * rs + i] = 0;
  for (j = 0; j < cs; j += 1)
    for (i = 0; i < rs; i += 1)
      if (K[j * rs + i] && SINGL(i,j) && (CardBetapoint2d(K, rs, cs, i, j) == 1)) K[j * rs + i] = 0;
} /* EffaceLiensLibres2d() */

/* ==================================== */
void ColorieKh2d(struct xvimage *k)
/* ==================================== */
{
  index_t rs = rowsize(k);
  index_t cs = colsize(k);
  uint8_t *K = UCHARDATA(k);
  index_t i, j;
  for (j = 0; j < cs; j += 1)
    for (i = 0; i < rs; i += 1)
      if (K[j * rs + i])
      {
        if (CARRE(i,j)) K[j * rs + i] = NDG_CARRE;
        if (INTER(i,j)) K[j * rs + i] = NDG_INTER;
        if (SINGL(i,j)) K[j * rs + i] = NDG_SINGL;
      }
} /* ColorieKh2d() */

/* ========================================================================== */
/* ========================================================================== */
/* Simplicite, amincissement - 1ere definition GB (novembre 1998) */
/* ========================================================================== */
/* ========================================================================== */

/* ==================================== */
void CopieAlphacarre2d(uint8_t *G, uint8_t *K, index_t rs, index_t cs, index_t i, index_t j)
/* ==================================== */
{
  index_t x = i % 2;
  index_t y = j % 2;

  memset(G, VAL_NULLE, GRS2D*GCS2D); /* init a VAL_NULLE */
  if (i % 2)
  {
    if (j % 2)
    {          /* i impair, j impair : carre */
      if ((i > 0) && K[j * rs + i - 1]) G[y*GRS2D+x-1] = VAL_OBJET;
      if (((i+1) < rs) && K[j * rs + i + 1]) G[y*GRS2D+x+1] = VAL_OBJET;      
      if ((j > 0) && K[(j-1) * rs + i]) G[(y-1)*GRS2D+x] = VAL_OBJET;      
      if (((j+1) < cs) && K[(j+1) * rs + i]) G[(y+1)*GRS2D+x] = VAL_OBJET;      
      if (((i > 0) && (j > 0)) && K[(j-1) * rs + i - 1]) G[(y-1)*GRS2D+x-1] = VAL_OBJET;      
      if ((((i+1) < rs) && (j > 0)) && K[(j-1) * rs + i + 1]) G[(y-1)*GRS2D+x+1] = VAL_OBJET;      
      if (((i > 0) && ((j+1) < cs)) && K[(j+1) * rs + i - 1]) G[(y+1)*GRS2D+x-1] = VAL_OBJET;      
      if ((((i+1) < rs) && ((j+1) < cs)) && K[(j+1) * rs + i + 1]) G[(y+1)*GRS2D+x+1] = VAL_OBJET;      
    }
    else
    {          /* i impair, j pair : intervalle */
      if ((i > 0) && K[j * rs + i - 1]) G[y*GRS2D+x-1] = VAL_OBJET;
      if (((i+1) < rs) && K[j * rs + i + 1]) G[y*GRS2D+x+1] = VAL_OBJET;      
    }
  }
  else
  {
    if (j % 2)
    {          /* i pair, j impair : intervalle */
      if ((j > 0) && K[(j-1) * rs + i]) G[(y-1)*GRS2D+x] = VAL_OBJET;      
      if (((j+1) < cs) && K[(j+1) * rs + i]) G[(y+1)*GRS2D+x] = VAL_OBJET;      
    }
    else
    {          /* i pair, j pair : singleton */
    }
  }
} /* CopieAlphacarre2d() */

/* ==================================== */
void CopieBetacarreDual2d(uint8_t *G, uint8_t *K, index_t rs, index_t cs, index_t i, index_t j)
/* ==================================== */
{
  index_t x = (i+1) % 2;
  index_t y = (j+1) % 2;

  memset(G, VAL_NULLE, GRS2D*GCS2D); /* init a VAL_NULLE */
  if ((i+1) % 2)
  {
    if ((j+1) % 2)
    {
      if ((i > 0) && K[j * rs + i - 1]) G[y*GRS2D+x-1] = VAL_OBJET;
      if (((i+1) < rs) && K[j * rs + i + 1]) G[y*GRS2D+x+1] = VAL_OBJET;      
      if ((j > 0) && K[(j-1) * rs + i]) G[(y-1)*GRS2D+x] = VAL_OBJET;      
      if (((j+1) < cs) && K[(j+1) * rs + i]) G[(y+1)*GRS2D+x] = VAL_OBJET;      
      if (((i > 0) && (j > 0)) && K[(j-1) * rs + i - 1]) G[(y-1)*GRS2D+x-1] = VAL_OBJET;      
      if ((((i+1) < rs) && (j > 0)) && K[(j-1) * rs + i + 1]) G[(y-1)*GRS2D+x+1] = VAL_OBJET;      
      if (((i > 0) && ((j+1) < cs)) && K[(j+1) * rs + i - 1]) G[(y+1)*GRS2D+x-1] = VAL_OBJET;      
      if ((((i+1) < rs) && ((j+1) < cs)) && K[(j+1) * rs + i + 1]) G[(y+1)*GRS2D+x+1] = VAL_OBJET;      
    }
    else
    {
      if ((i > 0) && K[j * rs + i - 1]) G[y*GRS2D+x-1] = VAL_OBJET;
      if (((i+1) < rs) && K[j * rs + i + 1]) G[y*GRS2D+x+1] = VAL_OBJET;      
    }
  }
  else
  {
    if ((j+1) % 2)
    {          /* i pair, j impair : intervalle */
      if ((j > 0) && K[(j-1) * rs + i]) G[(y-1)*GRS2D+x] = VAL_OBJET;      
      if (((j+1) < cs) && K[(j+1) * rs + i]) G[(y+1)*GRS2D+x] = VAL_OBJET;      
    }
    else
    {
    }
  }
} /* CopieBetacarreDual2d() */

/* ==================================== */
index_t EffaceBetaTerminauxSimples2d(struct xvimage *k)
/* ==================================== */
{
  index_t rs = rowsize(k);
  index_t cs = colsize(k);
  //KLLL  index_t N = rs * cs;
  uint8_t *K = UCHARDATA(k);
  struct xvimage *g;
  uint8_t *G;  
  index_t i, j;
  int32_t n = 0;

  g = AllocGrille2d();
  G = UCHARDATA(g);  
#ifdef DEBUG
printf("EffaceBetaTerminauxSimples2d : niveau %d\n", IndexPileGrilles2d);
#endif
  for (j = 0; j < cs; j += 1)
    for (i = 0; i < rs; i += 1)
      if ((K[j*rs+i]) && BetaTerminal2d(K, rs, cs, i, j))
      {
        CopieAlphacarre2d(G, K, rs, cs, i, j);
#ifdef DEBUG
printf("EffaceBetaTerminauxSimples2d : teste clique %d,%d\n", i,j);
#endif
        EffaceLiensLibres2d(g);
        if (EnsembleSimple2d(g)) 
        { 
          K[j*rs+i] = 0; n++; 
#ifdef DEBUG
printf("EffaceBetaTerminauxSimples2d : efface clique %d,%d\n", i,j);
#endif
        }
      }
  FreeGrille2d();
  return n;
} /* EffaceBetaTerminauxSimples2d() */

/* ==================================== */
int32_t EnsembleSimple2d(struct xvimage *k)
/* ==================================== */
{
#ifdef DEBUG
printf("EnsembleSimple : \n");
printimage(k);
#endif
  if (ExactementUnBetaTerminal2d(UCHARDATA(k), rowsize(k), colsize(k))) return 1;
  if (EffaceBetaTerminauxSimples2d(k) == 0) return 0;
  EffaceLiensLibres2d(k);
  return EnsembleSimple2d(k);
} /* EnsembleSimple2d() */

/* ========================================================================== */
/* ========================================================================== */
/* Simplicite, amincissement - 2ere definition GB (decembre 1998) */
/* ========================================================================== */
/* ========================================================================== */

/* ==================================== */
int32_t ContientUnSeulElement2d(struct xvimage *b)
/* ==================================== */
{
  index_t N = rowsize(b) * colsize(b);
  uint8_t *B = UCHARDATA(b);
  index_t i;
  int32_t n = 0;
  for (i = 0; i < N; i++) if (B[i]) { n++; if (n > 1) return 0; }
  return (n == 1);
} /* ContientUnSeulElement() */

/* ==================================== */
int32_t Alpha1Simple2d(struct xvimage *b, index_t i, index_t j)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  uint8_t *B = UCHARDATA(b);
  index_t tab[GRS2D*GCS2D];
  int32_t u, n, nn = 0;

#ifdef DEBUGNEW
printf("Alpha1Simple2d : %d %d", i, j);
printimage(b);
#endif

  if (!B[j*rs+i]) return 0;
  Alphacarre2d(rs, cs, i, j, tab, &n);
  for (u = 0; u < n; u++) if (B[tab[u]]) nn++;

#ifdef DEBUGNEW
printf("Alpha1Simple2d : nn = %d\n\n", n);
#endif

  return (nn == 1);
} /* Alpha1Simple2d() */

/* ==================================== */
int32_t Beta1Simple2d(struct xvimage *b, index_t i, index_t j)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  uint8_t *B = UCHARDATA(b);
  index_t tab[GRS2D*GCS2D];
  int32_t u, n, nn = 0;

#ifdef DEBUGNEW
printf("Beta1Simple2d : %d %d", i, j);
printimage(b);
#endif

  if (!B[j*rs+i]) return 0;
  Betacarre2d(rs, cs, i, j, tab, &n);
  for (u = 0; u < n; u++) if (B[tab[u]]) nn++;

#ifdef DEBUGNEW
printf("Beta1Simple2d : nn = %d\n\n", n);
#endif

  return (nn == 1);
} /* Beta1Simple2d() */

/* ==================================== */
index_t EffaceAlpha1Simples2d(struct xvimage *b)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t N = rs * cs;
  uint8_t *B = UCHARDATA(b);
  index_t i, j, nbsimples = 0;

  for (j = 0; j < cs; j++)
    for (i = 0; i < rs; i++)
      if (B[j*rs+i])
        if (Alpha1Simple2d(b, i, j)) B[j*rs+i] = VAL_MARQUE;
  for (i = 0; i < N; i++) if (B[i] == VAL_MARQUE) { B[i] = VAL_NULLE; nbsimples++; }
  return nbsimples;
} /* EffaceAlpha1Simples2d() */

/* ==================================== */
index_t EffaceBeta1Simples2d(struct xvimage *b)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t N = rs * cs;
  uint8_t *B = UCHARDATA(b);
  index_t i, j, nbsimples = 0;

  for (j = 0; j < cs; j++)
    for (i = 0; i < rs; i++)
      if (B[j*rs+i])
        if (Beta1Simple2d(b, i, j)) B[j*rs+i] = VAL_MARQUE;
  for (i = 0; i < N; i++) if (B[i] == VAL_MARQUE) { B[i] = VAL_NULLE; nbsimples++; }
  return nbsimples;
} /* EffaceBeta1Simples2d() */

/* ==================================== */
int32_t Ensemble1Contractile2d(struct xvimage *b)
/* ==================================== */
{
  int32_t stabilite;

#ifdef DEBUGNEW
printf("Ensemble1Contractile2d : avant effacement :\n");
printimage(b);
#endif

  do
  {
    stabilite = 1;
#ifdef BETAALPHA
    if (EffaceBeta1Simples2d(b)) stabilite = 0;
    if (EffaceAlpha1Simples2d(b)) stabilite = 0;
#else
    if (EffaceAlpha1Simples2d(b)) stabilite = 0;
    if (EffaceBeta1Simples2d(b)) stabilite = 0;
#endif
  } while (!stabilite);

#ifdef DEBUGNEW
printf("Ensemble1Contractile2d : apres effacement :\n");
printimage(b);
#endif

  return ContientUnSeulElement2d(b);
} /* Ensemble1Contractile2d() */

/* ==================================== */
int32_t Alpha2Simple2d(struct xvimage *b, index_t i, index_t j)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  uint8_t *B = UCHARDATA(b);
  struct xvimage *g;
  uint8_t *G;  
  int32_t ret;

#ifdef DEBUGNEW
printf("Alpha2Simple2d : %d %d\n", i, j);
printimage(b);
#endif

  if (!B[j*rs+i]) return 0;
  g = AllocGrille2d();
  G = UCHARDATA(g);
  CopieAlphacarre2d(G, B, rs, cs, i, j);
  ret = Ensemble1Contractile2d(g);

#ifdef DEBUGNEW
printf("Alpha2Simple2d : retourne %d\n\n", ret);
#endif

  FreeGrille2d();
  return ret;
} /* Alpha2Simple2d() */

/* ==================================== */
int32_t Beta2Simple2d(struct xvimage *b, index_t i, index_t j)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  uint8_t *B = UCHARDATA(b);
  struct xvimage *g;
  uint8_t *G;  
  int32_t ret;

#ifdef DEBUGNEW
printf("Beta2Simple2d : %d %d\n", i, j);
printimage(b);
#endif

  if (!B[j*rs+i]) return 0;
  g = AllocGrille2d();
  G = UCHARDATA(g);
  CopieBetacarreDual2d(G, B, rs, cs, i, j);
  ret = Ensemble1Contractile2d(g);

#ifdef DEBUGNEW
printf("Beta2Simple2d : retourne %d\n\n", ret);
#endif

  FreeGrille2d();
  return ret;
} /* Beta2Simple2d() */

/* ==================================== */
index_t EffaceAlpha2Simples2d(struct xvimage *b)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t N = rs * cs;
  uint8_t *B = UCHARDATA(b);
  index_t i, j, nbsimples = 0;

  for (j = 0; j < cs; j++)
    for (i = 0; i < rs; i++)
      if (B[j*rs+i])
        if (Alpha2Simple2d(b, i, j)) B[j*rs+i] = VAL_MARQUE;
  for (i = 0; i < N; i++) if (B[i] == VAL_MARQUE) { B[i] = VAL_NULLE; nbsimples++; }
  return nbsimples;
} /* EffaceAlpha2Simples2d() */

/* ==================================== */
index_t EffaceAlpha2SimplesNonExtr2d(struct xvimage *b)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t N = rs * cs;
  uint8_t *B = UCHARDATA(b);
  index_t i, j, nbsimples = 0;

  for (j = 0; j < cs; j++)
    for (i = 0; i < rs; i++)
      if (B[j*rs+i])
        if (Alpha2Simple2d(b, i, j) && (CardThetacarre2d(b, i, j) != 1))
          B[j*rs+i] = VAL_MARQUE;
  for (i = 0; i < N; i++) if (B[i] == VAL_MARQUE) { B[i] = VAL_NULLE; nbsimples++; }
  return nbsimples;
} /* EffaceAlpha2SimplesNonExtr2d() */

/* ==================================== */
index_t EffaceBeta2Simples2d(struct xvimage *b)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t N = rs * cs;
  uint8_t *B = UCHARDATA(b);
  index_t i, j, nbsimples = 0;

  for (j = 0; j < cs; j++)
    for (i = 0; i < rs; i++)
      if (B[j*rs+i])
        if (Beta2Simple2d(b, i, j)) B[j*rs+i] = VAL_MARQUE;
  for (i = 0; i < N; i++) if (B[i] == VAL_MARQUE) { B[i] = VAL_NULLE; nbsimples++; }
  return nbsimples;
} /* EffaceBeta2Simples2d() */

/* ==================================== */
index_t EffaceBeta2SimplesNonExtr2d(struct xvimage *b)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  index_t N = rs * cs;
  uint8_t *B = UCHARDATA(b);
  index_t i, j, nbsimples = 0;

  for (j = 0; j < cs; j++)
    for (i = 0; i < rs; i++)
      if (B[j*rs+i])
        if (Beta2Simple2d(b, i, j) && (CardThetacarre2d(b, i, j) != 1))
          B[j*rs+i] = VAL_MARQUE;
  for (i = 0; i < N; i++) if (B[i] == VAL_MARQUE) { B[i] = VAL_NULLE; nbsimples++; }
  return nbsimples;
} /* EffaceBeta2SimplesNonExtr2d() */

/* ==================================== */
int32_t Ensemble2Contractile2d(struct xvimage *b)
/* ==================================== */
{
  int32_t stabilite;

#ifdef DEBUGNEW
printf("Ensemble2Contractile2d : avant effacement :\n");
printimage(b);
#endif

  do
  {
    stabilite = 1;
#ifdef BETAALPHA
    if (EffaceBeta2Simples2d(b)) stabilite = 0;
    if (EffaceAlpha2Simples2d(b)) stabilite = 0;
#else
    if (EffaceAlpha2Simples2d(b)) stabilite = 0;
    if (EffaceBeta2Simples2d(b)) stabilite = 0;
#endif
  } while (!stabilite);

#ifdef DEBUGNEW
printf("Ensemble2Contractile2d : apres effacement :\n");
printimage(b);
#endif

  return ContientUnSeulElement2d(b);
} /* Ensemble2Contractile2d() */

/* ==================================== */
void Htkern2d(struct xvimage *b, int32_t n)
/* ==================================== */
/*
   noyau homotopique si n = 0, 
   sinon n etapes d'amincissement.
 */ 
{
  int32_t stabilite;

  if (n == 0)
  do
  {
    stabilite = 1;
#ifdef BETAALPHA
    if (EffaceBeta2SimplesNonExtr2d(b)) stabilite = 0;
    if (EffaceAlpha2SimplesNonExtr2d(b)) stabilite = 0;
#else
    if (EffaceAlpha2SimplesNonExtr2d(b)) stabilite = 0;
    if (EffaceBeta2SimplesNonExtr2d(b)) stabilite = 0;
#endif
  } while (!stabilite);
  else
  do
  {
    stabilite = 1;
#ifdef BETAALPHA
    if (EffaceBeta2SimplesNonExtr2d(b)) stabilite = 0;
    n--; 
    if (n == 0) break;
    if (EffaceAlpha2SimplesNonExtr2d(b)) stabilite = 0;
#else
    if (EffaceAlpha2SimplesNonExtr2d(b)) stabilite = 0;
    n--; 
    if (n == 0) break;
    if (EffaceBeta2SimplesNonExtr2d(b)) stabilite = 0;
#endif
    if (stabilite == 1) break;
  } while (n > 0);

} /* Htkern2d() */

/* ==================================== */
int32_t AlphaSimple2d(struct xvimage *b, index_t i, index_t j)
/* ==================================== */
/* 
   test d'alpha-simplicite specifique pour la 2D, plus rapide qu'Alpha2Simple2d 
   le point (i,j) ne doit pas etre un point de bord (ATTENTION: ca peut planter sinon)
*/
{
  index_t rs = rowsize(b);
  uint8_t *B = UCHARDATA(b);
  int32_t n; /* pour compter les transitions dans l'alpha d'un carre */

  if (SINGL(i,j)) return 0;
  if (INTERH(i,j)) return (B[j*rs + i-1] != B[j*rs + i+1]);
  if (INTERV(i,j)) return (B[(j-1)*rs + i] != B[(j+1)*rs + i]);
  n = 0;
  if (B[(j)  *rs + i+1] != B[(j-1)*rs + i+1]) n++;
  if (B[(j-1)*rs + i+1] != B[(j-1)*rs + i  ]) n++;
  if (B[(j-1)*rs + i  ] != B[(j-1)*rs + i-1]) n++;
  if (B[(j-1)*rs + i-1] != B[(j)  *rs + i-1]) n++;
  if (B[(j)  *rs + i-1] != B[(j+1)*rs + i-1]) n++;
  if (B[(j+1)*rs + i-1] != B[(j+1)*rs + i  ]) n++;
  if (B[(j+1)*rs + i  ] != B[(j+1)*rs + i+1]) n++;
  if (B[(j+1)*rs + i+1] != B[(j)  *rs + i+1]) n++;
  return (n == 2);
} /* AlphaSimple2d() */

/* ==================================== */
int32_t BetaSimple2d(struct xvimage *b, index_t i, index_t j)
/* ==================================== */
/* 
   test de beta-simplicite specifique pour la 2D, plus rapide que Beta2Simple2d 
   le point (i,j) ne doit pas etre un point de bord (ATTENTION: ca peut planter sinon)
*/
{
  index_t rs = rowsize(b);
  uint8_t *B = UCHARDATA(b);
  int32_t n; /* pour compter les transitions dans le beta d'un singleton */

  if (CARRE(i,j)) return 0;
  if (INTERV(i,j)) return (B[j*rs + i-1] != B[j*rs + i+1]);
  if (INTERH(i,j)) return (B[(j-1)*rs + i] != B[(j+1)*rs + i]);
  n = 0;
  if (B[(j)  *rs + i+1] != B[(j-1)*rs + i+1]) n++;
  if (B[(j-1)*rs + i+1] != B[(j-1)*rs + i  ]) n++;
  if (B[(j-1)*rs + i  ] != B[(j-1)*rs + i-1]) n++;
  if (B[(j-1)*rs + i-1] != B[(j)  *rs + i-1]) n++;
  if (B[(j)  *rs + i-1] != B[(j+1)*rs + i-1]) n++;
  if (B[(j+1)*rs + i-1] != B[(j+1)*rs + i  ]) n++;
  if (B[(j+1)*rs + i  ] != B[(j+1)*rs + i+1]) n++;
  if (B[(j+1)*rs + i+1] != B[(j)  *rs + i+1]) n++;
  return (n == 2);
} /* BetaSimple2d() */

/* ==================================== */
index_t EulerKh2d(struct xvimage *b)
/* ==================================== */
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  uint8_t *B = UCHARDATA(b);
  index_t i, j;
  index_t n2=0, n1=0, n0=0;

    for (j = 0; j < cs; j += 1)
      for (i = 0; i < rs; i += 1)
        if (B[j*rs + i])
        {
          if (CARRE(i,j)) n2++;
          else if (INTER(i,j)) n1++;
          else if (SINGL(i,j)) n0++;
        }
  return n0 - n1 + n2;
} /* EulerKh2d() */

/* ========================================================================== */
/* ========================================================================== */
/*                     Collapse                               */
/* ========================================================================== */
/* ========================================================================== */

/* ==================================== */
int32_t FaceLibre2d(struct xvimage *b, index_t i, index_t j)
/* ==================================== */
// Détermine si la face (i,j) est libre dans le complexe b, c'est-a-dire si 
// elle est strictement incluse dans exactement une face de b.
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  uint8_t *B = UCHARDATA(b);
  index_t tab[GCS2D*GRS2D];
  int32_t u, n, nn = 0;

  if (!B[j*rs+i]) return 0;
  Betacarre2d(rs, cs, i, j, tab, &n);
  for (u = 0; u < n; u++) if (B[tab[u]]) nn++;

  return (nn == 1);
} /* FaceLibre2d() */

/* ==================================== */
int32_t PaireLibre2d(struct xvimage *b, index_t i, index_t j)
/* ==================================== */
// Détermine si la face (i,j) est libre dans le complexe b, c'est-a-dire si 
// elle est strictement incluse dans exactement une face de b.
// Si non, retourne -1.
// Si oui, retourne la face contenant (i,j).
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  uint8_t *B = UCHARDATA(b);
  index_t tab[GCS2D*GRS2D];
  int32_t u, uu, n, nn = 0;

  if (!B[j*rs+i]) return -1;
  Betacarre2d(rs, cs, i, j, tab, &n);
  for (u = 0; u < n; u++) if (B[tab[u]]) { nn++; uu = u; }
  if (nn != 1) return -1;
  return tab[uu];
} /* PaireLibre2d() */

/* ==================================== */
int32_t Collapse2d(struct xvimage *b, index_t i, index_t j)
/* ==================================== */
#undef F_NAME
#define F_NAME "Collapse2d"
// Checks whether the face g = (i,j) is a free face for the complex b.
// If it is not, return -1.
// If it is, it forms a free pair with a face f which contains it. 
// These two faces are removed from b, and the face f is returned.
{
  index_t rs = rowsize(b);
  index_t cs = colsize(b);
  uint8_t *B = UCHARDATA(b);
  index_t tab[GCS2D*GRS2D];
  int32_t u, uu, n, nn = 0;

  if (!B[j*rs+i]) return -1;
  Betacarre2d(rs, cs, i, j, tab, &n);
  for (u = 0; u < n; u++) if (B[tab[u]]) { nn++; uu = u; }
  if (nn != 1) return -1;
  B[tab[uu]] = B[j*rs+i] = VAL_NULLE;
  return tab[uu];
} /* Collapse2d() */

/* ========================================================================== */
/* ========================================================================== */
/*                     Impression en Postscript                               */
/* ========================================================================== */
/* ========================================================================== */

/*========================================*/
void move (double x, double y) 
/*========================================*/
{
  int32_t xx, yy;
  xx = (int32_t)x;
  yy = (int32_t)y;
  printf("%d %d moveto ", xx, yy);
} /* move() */

/*========================================*/
void draw (double x, double y)
/*========================================*/
{
  int32_t xx, yy;
  xx = (int32_t)x;
  yy = (int32_t)y;
  printf("%d %d lineto ", xx, yy);
} /* draw() */


/*========================================*/
void line (double xd, double yd, double xf, double yf)
/*========================================*/
{
  printf("newpath ");
  move(xd, yd);
  draw(xf, yf);
  printf("stroke\n");
} /* line() */

/*========================================*/
void drawcircle (double x, double y, double r)
/*========================================*/
{
  int32_t xx, yy, rr;
  xx = (int32_t)x;
  yy = (int32_t)y;
  rr = (int32_t)r;
  printf("newpath ");
  printf("%d %d %d 0 360 arc ", xx, yy, rr);
  printf("stroke\n");
} /* drawcircle() */

/*========================================*/
void drawdisc (double x, double y, double r)
/*========================================*/
{
  int32_t xx, yy, rr;
  xx = (int32_t)x;
  yy = (int32_t)y;
  rr = (int32_t)r;
  printf("newpath ");
  printf("%d %d %d 0 360 arc ", xx, yy, rr);
  printf("fill stroke\n");
} /* drawdisc() */

/* ==================================== */
void DrawPsKh2d(struct xvimage *k, int32_t m, int32_t p, int32_t re, int32_t ri, int32_t rc)
/* ==================================== */
/*
  m : marge (haut et gauche)
  p : pas de la grille
  rx: rayon (e = element, i = intervalle, c = carre)
*/
{
  index_t rs = rowsize(k);
  index_t cs = colsize(k);
  uint8_t *K = UCHARDATA(k);
  index_t i, j;

  printf("%%!PS-Adobe-2.0 EPSF-2.0\n");
  printf("%%%%Creator: cube2ps by MC - 1998\n");
#ifdef MC_64_BITS
  printf("%%%%BoundingBox: 0 0 %lld %lld\n", m+m+p*rs, m+m+p*cs);
#else
  printf("%%%%BoundingBox: 0 0 %d %d\n", m+m+p*rs, m+m+p*cs);
#endif
  /* printf("2 setlinewidth\n"); */

#ifdef TRACELIGNES
  /* lignes horizontales */
  for (Y = 0; Y < cs; Y++) line( m, m+Y*p, m+(rs-1)*p, m+Y*p );

  /* lignes verticales */
  for (X = 0; X < rs; X++) line( m+X*p, m, m+X*p, m+(cs-1)*p );
#endif

  for (j = 0; j < cs; j += 1)
    for (i = 0; i < rs; i += 1)
      if (K[j * rs + i])
      {
        if (CARRE(i,j)) drawdisc(m+i*p, m+(cs-j-1)*p, rc); 
        if (INTER(i,j)) drawdisc(m+i*p, m+(cs-j-1)*p, ri); 
        if (SINGL(i,j)) drawdisc(m+i*p, m+(cs-j-1)*p, re); 
      }
      else
      {
        if (CARRE(i,j)) drawcircle(m+i*p, m+(cs-j-1)*p, rc); 
        if (INTER(i,j)) drawcircle(m+i*p, m+(cs-j-1)*p, ri); 
        if (SINGL(i,j)) drawcircle(m+i*p, m+(cs-j-1)*p, re); 
      }
} /* DrawPsKh2d() */
