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
#include <mccodimage.h>
#include <mcimage.h>
#include <mcfah.h>
#include <mcindic.h>
#include <llpemeyer.h>
#include <mckhalimsky2d.h>
//#include <llabelextrema.h>
#include <lsaliency.h>
#include <mclistechainee.h>
#include <mcfifo.h>

//#define _DEBUG_SALIENCY_

typedef struct _basinT {
  int32_t altitude;
  int32_t label;
  int32_t dynamics;
  int32_t surface;
  int32_t volume;
  TypListechainee* neighbors;
  int32_t flag;
  int32_t father;
  //struct _basinT *father;
} basinT;

/*
int32_t compareBasin(const basinT *c1, const basinT* c2)
{
  return (int32_t)(c1->dynamics - c2->dynamics);
}
*/


int32_t findsaillence(basinT *basins, int32_t l1, int32_t l2)
{
  int32_t ll1, ll2, c1, c2;
  int32_t sail;
  
  ll1 = l1;
  ll2 = l2;
  while (ll1 != -1) {
    basins[ll1].flag++;
    ll1 = basins[ll1].father;
  }
  while (ll2 != -1) {
    basins[ll2].flag++;
    ll2 = basins[ll2].father;
  }

  //cherche le premier flag a 2
  ll1 = l1;
  while (basins[ll1].flag != 2)
    ll1 = basins[ll1].father;
  c1 = l1;
  c2 = l2;
  if (c1 != ll1) {
    while (basins[c1].father != ll1)
      c1 = basins[c1].father;
  }
  if (c2 != ll1) {
    while (basins[c2].father != ll1)
      c2 = basins[c2].father;
  }

  sail = 0;
  if (c1 != ll1)
    sail = basins[c1].dynamics;
  if (c2 != ll1)
    sail = (sail > basins[c2].dynamics)? sail : basins[c2].dynamics;

  /*
  if ( ((l1 == 24) && (l2 == 16)) || ((l2 == 24) && (l1 == 16)) ) {
    printf("Saillence (%d, %d) : ", l1, l2);
    printf("father l1 = %d, l2 = %d -> %d - ",c1,c2,ll1);
    printf("sail = %d -", sail);
    printf("\n");
  }
  if ( ((l1 == 14) && (l2 == 16)) || ((l2 == 14) && (l1 == 16)) ) {
    printf("Saillence (%d, %d) : ", l1, l2);
    printf("father l1 = %d, l2 = %d -> %d - ",c1,c2,ll1);
    printf("sail = %d -", sail);
    printf("\n");
  }
  */
  // netoyage et re-preparation pour pouvoir refaire
  while (l1 != -1) {
    basins[l1].flag=0;
    l1 = basins[l1].father;
  }
  while (l2 != -1) {
    basins[l2].flag=0;
    l2 = basins[l2].father;
  }

  return sail;
}

/*
int32_t findsaillence(basinT *c1, basinT *c2)
{
  basinT *b1 = c1, *b2 = c2;
  int32_t sail;

  printf("Saillence : %d - %d --", c1->label, c2->label);
  while (b1 != NULL) {
    b1->flag++;
    b1 = b1->father;
  }
  while (b2 != NULL) {
    b2->flag++;
    b2 = b2->father;
  }
  //cherche le premier flag a 2
  if (c1->dynamics < c2->dynamics)
    b1 = c1;
  else
    b1 = c2;
  while (b1->father->flag != 2) {
    b1 = b1->father;
  }
  printf("lca = %d (%d)\n", b1->label, b1->dynamics);
  sail = b1->dynamics;

  // netoyage et re-preparation pour pouvoir refaire
  b1 = c1; b2 = c2;
  while (b1 != NULL) {
    b1->flag=0;
    b1 = b1->father;
  }
  while (b2 != NULL) {
    b2->flag=0;
    b2 = b2->father;
  }

  return sail;
}
*/

/* ==================================== */
int32_t lsaliency(
        struct xvimage *image,
        struct xvimage *masque,
        struct xvimage *saliency,
        int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "lsaliency"
{
  struct xvimage *label;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t srs = 2*rs+1;
  int32_t scs = 2*cs+1;
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *MA;                             /* l'image de masque */
  int32_t *S;
  uint32_t *it;
  int32_t * L;
  int32_t nbmin; /* Nb of minima */
  int32_t incr_vois;
  register int32_t i, j, k, x, y;
  basinT *basins;
  //const int32_t WSHED_HMAX  =       512;    // Max grey value
  const int32_t INIT	=	-1;	// initial value of pixels of out 
  const int32_t MASK	=	-2;     // Initial value for each level
  const int32_t INQUEUE	=	-3;     // When a pixel is in the queue
#define WSHED_HMAX 512
  static int32_t hi[WSHED_HMAX + 1], hc[WSHED_HMAX + 1];
  int32_t h, ind;
  Fifo * fifo;

  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
    default: 
      fprintf(stderr, "%s: mauvaise connexite: %d\n", F_NAME, connex);
      return 0;
  } /* switch (connex) */    

  if (datatype(saliency) != VFF_TYP_4_BYTE) 
  {
    fprintf(stderr, "%s: le resultat doit etre de type VFF_TYP_4_BYTE\n", 
	    F_NAME);
    return 0;
  }

  /*
  minima = allocimage(NULL, rs, cs, 1, VFF_TYP_1_BYTE);
  if (minima == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    exit(1);
  }
  */
  /*
  tmp = allocimage(NULL, rs, cs, 1, VFF_TYP_4_BYTE);
  if (tmp == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    exit(1);
  }
  */
  /* min = SLONGDATA(minima); */
  label = allocimage(NULL, rs, cs, 1, VFF_TYP_4_BYTE);
  if (label == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    exit(1);
  }

  /*
  if (! llabelextrema(image, connex, LABMIN, tmp, &nbmin)) {
      fprintf(stderr, "%s: llabelextrema failed\n", F_NAME);
      exit(1);
  }
  printf("found %d minima\n", nbmin);
  */

  L = SLONGDATA(label);
  // sort in pixels by increasing grey level
  for (h = 0; h <= WSHED_HMAX; h++) 
    hi[h] = 0;
  hc[0] = 0;
  // Histogram (truncated)
  for (x = 0; x < N; x++)   {
    if (F[x] <= 0) hi[0]++;
    // Inutile (F est du type char) else if (F[x] >= WSHED_HMAX) hi[WSHED_HMAX]++;
    else
      hi[F[x]]++;
    /*
    int32_t max = 0;
    for (k=0; k<8; k+= incr_vois) {
      y = voisin(x, k, rs, N);
      if (y!=-1)
	max = (max > abs(F[x] - F[y])) ? max : abs (F[x]-F[y]);
    }
    hi[max]++;
    */
    // label initialization
    L[x] = INIT;
  }
  // Cumulative histogram
  for (h = 1; h <= WSHED_HMAX; h++)
    hc[h] = hc[h - 1] + hi[h - 1];

  it = (uint32_t *) calloc(N, sizeof(int32_t));
  if (it == NULL) {
    fprintf(stderr, "%s: calloc failed\n", F_NAME);
    exit(1);
  }
  // The sorting itself
  for (x = 0; x < N; x++)   {
    register int32_t hptin=F[x];
    /*
    register int32_t max = 0;
    for (k=0; k<8; k+= incr_vois) {
      y = voisin(x, k, rs, N);
      if (y!=-1)
	max = (max > abs(F[x] - F[y])) ? max : abs (F[x]-F[y]);
    }
    hptin=max;
    */
    // adjust pointer to array limits
    if(hptin<=0) hptin=0;
    else {
      if(hptin>=WSHED_HMAX) hptin=WSHED_HMAX;
    }
    it[hc[hptin]] = x;
    hc[hptin]++;
  }
	
  for (h = WSHED_HMAX; h > 0; h--)
    hc[h] = hc[h - 1];
  hc[0] = 0;

  // Now the watershed
  nbmin = 0;
  fifo = CreeFifoVide(N);
  basins = (basinT*)calloc(N, sizeof(basinT));

  for (h = 0; h < WSHED_HMAX; h++) {
    // Propagation of level h-1 into level h
    for (ind = hc[h]; ind < hc[h + 1]; ind++) {
      x = it[ind];
      L[x] = MASK;
      for (k=0; k<8; k+=incr_vois) {
	y = voisin(x, k, rs, N);
	if ((y != -1) && (L[y] > 0)) {
	  L[x] = INQUEUE;
	  FifoPush(fifo, x);
	  break;
	}
      }
    }
    while (! FifoVide(fifo)) {
      x = FifoPop(fifo);
      for (k = 0; k < 8; k += incr_vois) {
	y = voisin(x, k, rs, N);
	if ((y != -1)) {
	  if (L[y] > 0) {
	    if (L[x] == INQUEUE) {
	      L[x] = L[y];
	      basins[L[x]-1].surface += 1;
	      basins[L[x]-1].volume += h-basins[L[x]-1].altitude;
	    } else if ((L[x] > 0) && (L[x] != L[y])) {
	      // Contact point
	      int32_t l1 = L[x]-1;
	      int32_t l2 = L[y]-1;
	      //printf("a l1 = %d l2 = %d\n", l1, l2);
	      while (basins[l1].father != -1)
		l1 = basins[l1].father;
	      while (basins[l2].father != -1)
		l2 = basins[l2].father;
	      //printf("p l1 = %d l2 = %d\n", l1, l2);
	      if (l1 != l2) {
		if (basins[l1].altitude > basins[l2].altitude) {
		  basins[l1].dynamics = h - basins[l1].altitude;
#ifdef _DEBUG_SALIENCY_
		  printf("dynamics of %d = %d - %d; %d\n", basins[l1].label, basins[l1].dynamics, h, basins[l1].altitude);
#endif
		  l2 = L[y]-1;
		  while ((basins[l2].dynamics <= basins[l1].dynamics)
			 && (basins[l2].father != -1))
		    l2 = basins[l2].father;
#ifdef _DEBUG_SALIENCY_
		  printf("father of %d = %d\n", l1, l2);
#endif
		  basins[l1].father = l2;
		} else {
		  basins[l2].dynamics = h - basins[l2].altitude;
#ifdef _DEBUG_SALIENCY_
		  printf("dynamics of %d = %d - %d; %d\n", basins[l2].label, basins[l2].dynamics, h, basins[l2].altitude);
#endif
		  l1 = L[x]-1;
		  while ((basins[l1].dynamics <= basins[l2].dynamics) 
			 && (basins[l1].father != -1))
		    l1 = basins[l1].father;
#ifdef _DEBUG_SALIENCY_
		  printf("father of %d = %d\n", l2, l1);
#endif
		  basins[l2].father = l1;
		}
	      }
	    }
	  } else if (L[y] == MASK) {
	    L[y] = INQUEUE;
	    FifoPush(fifo,y);
	  }
	}
      } // for k
    }
    // Are they any new minima ?
    for (ind = hc[h]; ind < hc[h + 1]; ind++) {
      x  = it[ind];
      if (L[x] == MASK) {
	basins[nbmin].volume = 0;
	basins[nbmin].surface = 0;
	basins[nbmin].dynamics = -1;
	basins[nbmin].father = -1;
	basins[nbmin].altitude = h;
	basins[nbmin].label = nbmin+1;
	nbmin++;
	FifoPush(fifo, x);
	L[x] = nbmin;

	while (!FifoVide(fifo)) {
	  x = FifoPop(fifo);
	  basins[nbmin-1].surface += 1;
	  basins[nbmin-1].volume  += 1;
	  for (k=0; k<8; k+=incr_vois) {
	    y = voisin(x, k, rs, N);
	    if ((y!=-1) && (L[y] == MASK)) {
	      FifoPush(fifo, y);
	      L[y] = nbmin;
	    }
	  }
	} // End: while (!FifoVide())
      } // End: if (MASK) 
    }  // End: for (ind = hc[h]; ind < hc[h + 1]; ind++)
    
  }
  FifoTermine(fifo);
#ifdef _DEBUG_SALIENCY_
  writeimage(label, "label.pgm");
  printf("%d minima found\n", nbmin);
#endif
  {
    int32_t xx, yy, max;
  //printf("scs = %d, srs = %d\n", scs, srs);
  S = SLONGDATA(saliency);
  // mettre les barres horizontales et verticales à la saillence
  for (i = 0; i < cs; i++) {
    for (j = 0; j < rs; j++) {
      x = i*rs + j;
      xx = (2*i+1) * srs + (2*j+1);
      if ((!masque || MA[x])) 	{
	for (k = 0; k < 8; k += incr_vois) {
	  y = voisin(x, k, rs, N);
	  yy = voisin(xx, k, srs, 2*N+1);
	  //max = (valmin[L[x]] > valmin[L[y]]) ? valmin[L[x]] : valmin[L[y]];
	  //max = 0;
	  if ((y != -1) && (L[y] != L[x])) {
	    //S[yy] = T[L[y]+L[x]*(nbmin)] - max;
	    S[yy] = findsaillence(basins, L[x]-1, L[y]-1);
	  }
	}
      }
    }
  }
  // les points à la saillence (max des barres)
  for (i = 0; i < scs; i+=2) {
    for (j = 0; j < srs; j+=2) {
      x = i*srs + j;
      max = 0;
      for (k = 0; k < 8; k += 2) {
	y = voisin(x,k, srs, 2*N+1);
	if ((y!=-1) && (max < S[y]))
	  max = S[y];
      }
      S[x] = max;
    }
  }
  }
    
  /*
  M = UCHARDATA(minima);
  T = SLONGDATA(tmp);
  for (i = 0; i < N; i++) 
    if (T[i]) M[i] = 1;
  freeimage(tmp);
  //writeimage(minima, "minima.pgm");

  if (! llpemeyersansligne(image, minima, NULL, masque, connex, label)) {
    //fprintf(stderr, "%s: llpemeyersansligne failed\n", F_NAME);
    fprintf(stderr, "%s: lsaliency failed\n", F_NAME);
    exit(1);
  }
  writeimage(label, "label.pgm");
  */

  /*
  // Building of the neighborhood map 
  tmp = allocimage(NULL, nbmin, nbmin, 1, VFF_TYP_4_BYTE);
  if (tmp == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    exit(1);
  }

  basins = (basinT *) calloc(nbmin, sizeof(basinT));
  if (basins == NULL) {
    fprintf(stderr, "%s: calloc failed\n", F_NAME);
    exit(1);
  }

  for (i=0; i<nbmin; i++) {
    basins[i].father = NULL;
    basins[i].label = i;
    basins[i].neighbors = ListechaineeVide();
  }

  if (masque) MA = UCHARDATA(masque);

  L = SLONGDATA(label);
  T = SLONGDATA(tmp);
  for (x = 0; x < N; x++)   {
    // Get the altitude of the minimum
    if (M[x] != 0) basins[L[x]].altitude = F[x];

    if ((!masque || MA[x])) 	{
      for (k = 0; k < 8; k += incr_vois) {
	y = voisin(x, k, rs, N);
	if ((y != -1) && (L[y] != L[x])) {
	  // Contact between two basins
	  passold = T[L[y]+L[x]*(nbmin)];
	  pass = (F[x] > F[y])? F[x] : F[y];
	  if ((passold==0) || (pass < passold)) {
	    T[L[y]+L[x]*(nbmin)] = pass;
	    T[L[x]+L[y]*(nbmin)] = pass;
	    //printf("%d - %d = %d\n", L[x], L[y], pass);
	  }
	  // Look if the neighborood is already registred
	  l = basins[L[x]].neighbors;
	  for (; l != NULL; l = Suite(l)) {
	    if (Tete(l) == L[y]) 
	      break;
	  }
	  if (l == NULL) {
	    basins[L[x]].neighbors = Cons(L[y], basins[L[x]].neighbors);
	    basins[L[y]].neighbors = Cons(L[x], basins[L[y]].neighbors);
	  }
	}  
      }
    }
  }
  //writeimage(tmp, "pass.pgm");
  basins[0].dynamics = 0;
  basins[0].label = 0;
  basins[0].neighbors = NULL;
  */
  /*
  for (i=1; i<nbmin; i++) {
    basins[0].neighbors = Cons(i, basins[0].neighbors);
  }
  */
  /*
  flag = 0;
  for (i=1; i<nbmin; i++) {
    int32_t min = 512;
    l = basins[i].neighbors;
    for (; l != NULL; l = Suite(l)) {
      min = (T[i*nbmin+Tete(l)] > min)? min : T[i*nbmin+Tete(l)];
    }
    basins[i].dynamics = min - basins[i].altitude;
    if ((flag == 0) && (basins[i].altitude == 0)) {
      flag = 1;
      basins[i].dynamics = 512;
    }
    printf("Basin %d ; dyn = %d\n", i, basins[i].dynamics);
    printf("Voisin de %d: ", i);
    AfficheListechainee(basins[i].neighbors);
    printf("\n");
  }
  qsort(basins, nbmin, sizeof(basinT), compareBasin);
  printf("Après tri\n");
  for (i=0; i<nbmin; i++) {
    printf("Basin[%d] %d ; dyn = %d\n", i, basins[i].label, basins[i].dynamics);    
  }
  orig = (basinT **) calloc(nbmin, sizeof(basinT*));
  for (i=0; i<nbmin; i++) {
    for (j=0; j<nbmin; j++) {
      if (basins[i].label == j)
	orig[j] = &basins[i];
    }
  }

  for (i=1; i<nbmin-1; i++) {
    printf("Basin[%d] %d ; dyn = %d\n", i, basins[i].label, basins[i].dynamics);
    int32_t min = 512;
    int32_t lab = 0;
    l = basins[i].neighbors;
    for (; l != NULL; l = Suite(l)) {
      if (orig[Tete(l)]->father == NULL) // verifie que le basin n'a pas ete merge
	if (orig[Tete(l)]->label != basins[i].label) //verifie que c'est pas lui meme
	  if (min > T[basins[i].label*nbmin+Tete(l)]) {
	    min = T[basins[i].label*nbmin+Tete(l)];
	    lab = Tete(l);
	  }
    }
    printf("Basin %d will merge with %d - alt = %d\n", basins[i].label, lab, min);
    basins[i].father = orig[lab];
    l = basins[i].neighbors;
    for (; l != NULL; l = Suite(l)) {
      // les voisins ont un nouveau voisin!!!
      orig[Tete(l)]->neighbors = Union(orig[Tete(l)]->neighbors, orig[lab]->neighbors);
    }
    orig[lab]->neighbors = Union(orig[lab]->neighbors, basins[i].neighbors);
    printf("Voisin de %d: ", lab);
    AfficheListechainee(orig[lab]->neighbors);
    printf("\n");
    //basins[i] = *orig[lab]; 
  }

  {
    int32_t xx, yy, max;
  //printf("scs = %d, srs = %d\n", scs, srs);
  S = SLONGDATA(saliency);
  // mettre les barres horizontales et verticales à la saillence
  for (i = 0; i < cs; i++) {
    for (j = 0; j < rs; j++) {
      x = i*rs + j;
      xx = (2*i+1) * srs + (2*j+1);
      if ((!masque || MA[x])) 	{
	for (k = 0; k < 8; k += incr_vois) {
	  y = voisin(x, k, rs, N);
	  yy = voisin(xx, k, srs, 2*N+1);
	  //max = (valmin[L[x]] > valmin[L[y]]) ? valmin[L[x]] : valmin[L[y]];
	  //max = 0;
	  if ((y != -1) && (L[y] != L[x])) {
	    //S[yy] = T[L[y]+L[x]*(nbmin)] - max;
	    S[yy] = findsaillence(orig[L[x]], orig[L[y]]);
	  }
	}
      }
    }
  }
  // les points à la saillence (max des barres)
  for (i = 0; i < scs; i+=2) {
    for (j = 0; j < srs; j+=2) {
      x = i*srs + j;
      max = 0;
      for (k = 0; k < 8; k += 2) {
	y = voisin(x,k, srs, 2*N+1);
	if ((y!=-1) && (max < S[y]))
	  max = S[y];
      }
      S[x] = max;
    }
  }
  }

  for (i=0; i<nbmin; i++)
    DetruitListechainee(basins[i].neighbors);
  freeimage(tmp);
  */
  free(basins);
  freeimage(label);
  //freeimage(minima);
  return 1;
}


/* ==================================== */
int32_t lsaliency6b(
        struct xvimage *image,
        struct xvimage *masque,
        struct xvimage *saliency,
        int32_t parite)
/* ==================================== */
#undef F_NAME
#define F_NAME "lsaliency6b"
{
  struct xvimage *label;
  int32_t rs = rowsize(image);     /* taille ligne */
  int32_t cs = colsize(image);     /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *MA;                        /* l'image de masque */
  int32_t *S;
  uint32_t *it;
  int32_t * L;
  int32_t nbmin; /* Nb of minima */
  int32_t incr_vois;
  register int32_t k, x, y;
  basinT *basins;
  //const int32_t WSHED_HMAX  =       512;    // Max grey value
  const int32_t INIT	=	-1;	// initial value of pixels of out 
  const int32_t MASK	=	-2;     // Initial value for each level
  const int32_t INQUEUE	=	-3;     // When a pixel is in the queue
  const int32_t WSHED	=	-4;     // When a pixel is a divide pixel
#define WSHED_HMAX 512
  static int32_t hi[WSHED_HMAX + 1], hc[WSHED_HMAX + 1];
  int32_t h, ind;
  Fifo * fifo;

  incr_vois = 1;

  if (datatype(saliency) != VFF_TYP_4_BYTE) 
  {
    fprintf(stderr, "%s: le resultat doit etre de type VFF_TYP_4_BYTE\n", 
	    F_NAME);
    return 0;
  }

  label = allocimage(NULL, rs, cs, 1, VFF_TYP_4_BYTE);
  if (label == NULL)
  {   
    fprintf(stderr, "%s: allocimage failed\n", F_NAME);
    exit(1);
  }

  L = SLONGDATA(label);
  // sort in pixels by increasing grey level
  for (h = 0; h <= WSHED_HMAX; h++) 
    hi[h] = 0;
  hc[0] = 0;
  // Histogram (truncated)
  for (x = 0; x < N; x++)   {
    if (F[x] <= 0) hi[0]++;
    // Inutile (F est du type char) else if (F[x] >= WSHED_HMAX) hi[WSHED_HMAX]++;
    else
      hi[F[x]]++;
    /*
    int32_t max = 0;
    for (k=0; k<8; k+= incr_vois) {
      y = voisin(x, k, rs, N);
      if (y!=-1)
	max = (max > abs(F[x] - F[y])) ? max : abs (F[x]-F[y]);
    }
    hi[max]++;
    */
    // label initialization
    L[x] = INIT;
  }
  // Cumulative histogram
  for (h = 1; h <= WSHED_HMAX; h++)
    hc[h] = hc[h - 1] + hi[h - 1];

  it = (uint32_t *) calloc(N, sizeof(int32_t));
  if (it == NULL) {
    fprintf(stderr, "%s: calloc failed\n", F_NAME);
    exit(1);
  }
  // The sorting itself
  for (x = 0; x < N; x++)   {
    register int32_t hptin=F[x];
    /*
    register int32_t max = 0;
    for (k=0; k<8; k+= incr_vois) {
      y = voisin(x, k, rs, N);
      if (y!=-1)
	max = (max > abs(F[x] - F[y])) ? max : abs (F[x]-F[y]);
    }
    hptin=max;
    */
    // adjust pointer to array limits
    if(hptin<=0) hptin=0;
    else {
      if(hptin>=WSHED_HMAX) hptin=WSHED_HMAX;
    }
    it[hc[hptin]] = x;
    hc[hptin]++;
  }
	
  for (h = WSHED_HMAX; h > 0; h--)
    hc[h] = hc[h - 1];
  hc[0] = 0;

  // Now the watershed
  nbmin = 0;
  fifo = CreeFifoVide(N);
  basins = (basinT*)calloc(N, sizeof(basinT));

  for (h = 0; h < WSHED_HMAX; h++) {
    // Propagation of level h-1 into level h
    for (ind = hc[h]; ind < hc[h + 1]; ind++) {
      x = it[ind];
      L[x] = MASK;
      for (k=0; k<6; k+=incr_vois) {
	y = voisin6b(x, k, rs, N, parite);
	if ((y != -1) && (L[y] > 0)) {
	  L[x] = INQUEUE;
	  FifoPush(fifo, x);
	  break;
	}
      }
    }
    while (! FifoVide(fifo)) {
      x = FifoPop(fifo);
      for (k = 0; k < 6; k += incr_vois) {
	y = voisin6b(x, k, rs, N, parite);
	if ((y != -1)) {
	  if (L[y] > 0) {
	    if (L[x] == INQUEUE) {
	      L[x] = L[y];
	      basins[L[x]-1].surface += 1;
	      basins[L[x]-1].volume += h-basins[L[x]-1].altitude;
	    } else if ((L[x] > 0) && (L[x] != L[y])) {
	      // Contact point
	      int32_t l1 = L[x]-1;
	      int32_t l2 = L[y]-1;
	      //printf("a l1 = %d l2 = %d\n", l1, l2);
	      while (basins[l1].father != -1)
		l1 = basins[l1].father;
	      while (basins[l2].father != -1)
		l2 = basins[l2].father;
	      //printf("p l1 = %d l2 = %d\n", l1, l2);
	      if (l1 != l2) {
		if (basins[l1].altitude > basins[l2].altitude) {
		  basins[l1].dynamics = h - basins[l1].altitude;
#ifdef _DEBUG_SALIENCY_
		  printf("dynamics of %d = %d - %d; %d\n", basins[l1].label, basins[l1].dynamics, h, basins[l1].altitude);
#endif
		  l2 = L[y]-1;
		  while ((basins[l2].dynamics <= basins[l1].dynamics)
			 && (basins[l2].father != -1))
		    l2 = basins[l2].father;
#ifdef _DEBUG_SALIENCY_
		  printf("father of %d = %d\n", l1, l2);
#endif
		  basins[l1].father = l2;
		} else {
		  basins[l2].dynamics = h - basins[l2].altitude;
#ifdef _DEBUG_SALIENCY_
		  printf("dynamics of %d = %d - %d; %d\n", basins[l2].label, basins[l2].dynamics, h, basins[l2].altitude);
#endif
		  l1 = L[x]-1;
		  while ((basins[l1].dynamics <= basins[l2].dynamics) 
			 && (basins[l1].father != -1))
		    l1 = basins[l1].father;
#ifdef _DEBUG_SALIENCY_
		  printf("father of %d = %d\n", l2, l1);
#endif
		  basins[l2].father = l1;
		}
	      }
	      L[x] = WSHED;
	    }
	  } else if (L[y] == MASK) {
	    L[y] = INQUEUE;
	    FifoPush(fifo,y);
	  }
	}
      } // for k
    }
    // Are they any new minima ?
    for (ind = hc[h]; ind < hc[h + 1]; ind++) {
      x  = it[ind];
      if (L[x] == MASK) {
	basins[nbmin].volume = 0;
	basins[nbmin].surface = 0;
	basins[nbmin].dynamics = -1;
	basins[nbmin].father = -1;
	basins[nbmin].altitude = h;
	basins[nbmin].label = nbmin+1;
	nbmin++;
	FifoPush(fifo, x);
	L[x] = nbmin;

	while (!FifoVide(fifo)) {
	  x = FifoPop(fifo);
	  basins[nbmin-1].surface += 1;
	  basins[nbmin-1].volume  += 1;
	  for (k=0; k<6; k+=incr_vois) {
	    y = voisin6b(x, k, rs, N, parite);
	    if ((y!=-1) && (L[y] == MASK)) {
	      FifoPush(fifo, y);
	      L[y] = nbmin;
	    }
	  }
	} // End: while (!FifoVide())
      } // End: if (MASK) 
    }  // End: for (ind = hc[h]; ind < hc[h + 1]; ind++)
    
  }
  FifoTermine(fifo);
#ifdef _DEBUG_SALIENCY_
  writeimage(label, "label.pgm");
  printf("%d minima found\n", nbmin);
#endif
  S = SLONGDATA(saliency);
  for (x=0; x<N; x++) {
    if (L[x] == WSHED) {
      int32_t c1, c2;
      if ((!masque || MA[x])) 	{
	k = 0;
	while (k < 6) {
	  y = voisin6b(x, k, rs, N, parite);
	  if ((y != -1) && (L[y] != WSHED)) {
	    c1 = L[y];
	    break;
	  }
	  k += incr_vois;
	}
	while (k < 6) {
	  y = voisin6b(x, k, rs, N, parite);
	  if ((y != -1) && (L[y] != WSHED) && (L[y] != c1)) {
	    c2 = L[y];
	    break;
	  }
	  k += incr_vois;
	}
      }
      S[x] = findsaillence(basins, c1-1, c2-1);
    } else
      S[x] = 0;
  }


  free(basins);
  freeimage(label);
  return 1;
}
