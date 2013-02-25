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
#include <jcimage.h>
#include <mccodimage.h>
#include <jccodimage.h>
#include <mclifo.h>
#include <jclabelextrema.h>


int32_t jcfindextrema(struct xvimage *in,         /* GA de depart */
		      struct xvimage *out,           /* resultat: GA tq les aretes appartenant a des extrema sont à 255 */
		      int32_t minimum,                /* booleen, 1: on recherche les minima, 0: on recherche les maxima */
		      int32_t *nlabels               /* resultat: le nombre d'extrema*/
		   )
{
  int32_t i,j,k;                        
  int32_t x,y,w;
  uint8_t *F;
  uint8_t *G;
  uint32_t *LABEL;
  int32_t rs = rowsize(in);
  int32_t cs = colsize(in);
  int32_t N = rs * cs;
  int32_t tailleplateau;
  Lifo * LIFO;
  int32_t label;
  
  F = UCHARDATA(in);
  G = UCHARDATA(out);
  if( (LABEL = (uint32_t *)malloc( sizeof(uint32_t)*2*N)) == NULL)
  {   
    fprintf(stderr, "jcfindxtrema() : echec de malloc\n");
    return(0);
  }

  for (x = 0; x < 2*N; x++) LABEL[x] = -1;
  
  LIFO = CreeLifoVide(2*N);
  if (LIFO == NULL)
  {   
    fprintf(stderr, "jcfindextrema() : CreeLifoVide failed\n");
    return(0);
  }

  *nlabels = 0;
  //printf("taille du GA : %d\n",N);
  /* on explore d'abord les aretes horizontales */

  for(j = 0; j < cs; j++)
    for(i = 0; i < rs -1; i++)
    {
      x = j * rs + i;
      if (LABEL[x] == -1)          /* on trouve un point x non etiquete */
      {
	*nlabels += 1;             /* on cree un numero d'etiquette */
	LABEL[x] = *nlabels;
	LifoPush(LIFO, x);         /* on va parcourir le plateau auquel appartient x */
	tailleplateau = 0;
	while (! LifoVide(LIFO))
	{
	  tailleplateau++;
	  w = LifoPop(LIFO);
	  label = LABEL[w];
	  for (k = 0; k < 6; k++)
	  {
	    y = voisinGA(w, k, rs, N);
	     if (y != -1)
              {
		//	printf("voisin no %d de %d = %d\n",k,w,y);
                if ((label > 0) && 
                    ( (minimum && (F[y] < F[w])) ||
                      (!minimum && (F[y] > F[w]))
		      )
		    )
		{    /* w non dans un minimum (resp. maximum) */
                  label = 0;
                  *nlabels -= 1;
                  LABEL[w] = label;
                  LifoPush(LIFO, w);
                } 
		else
		  if (F[y] == F[w])
		  {
		    if (((label > 0) && (LABEL[y] == -1)) ||
			((label == 0) && (LABEL[y] != 0)))
		    {
		      LABEL[y] = label;
		      LifoPush(LIFO, y);
		    } /* if .. */
		  } /* if F ... */
              } /* if (y != -1) */
	  } /* for k ... */
	  
	} /* while  (! LifoVide(LIFO)) */
      } /* if (LABEL[x] == 0) */
    } /* for (x = 0; x < N; x++) */
  
  /* Puis on explore les aretes verticales */
  for(j = 0; j < cs -1; j++)
    for(i = 0; i < rs; i++)
    {
      x = N + j * rs + i;
      if (LABEL[x] == -1)          /* on trouve un point x non etiquete */
      {
	*nlabels += 1;             /* on cree un numero d'etiquette */
	LABEL[x] = *nlabels;
	LifoPush(LIFO, x);         /* on va parcourir le plateau auquel appartient x */
	tailleplateau = 0;
	while (! LifoVide(LIFO))
	{
	  tailleplateau++;
	  w = LifoPop(LIFO);
	  label = LABEL[w];
	  for (k = 0; k < 6; k++)
	  {
	    y = voisinGA(w, k, rs, N);
	     if (y != -1)
              {
		//printf("horiz voisin no %d de %d = %d\n",k,w,y);
                if ((label > 0) && 
                    ( (minimum && (F[y] < F[w])) ||
                      (!minimum && (F[y] > F[w]))
		      )
		    )
		{    /* w non dans un minimum (resp. maximum) */
                  label = 0;
                  *nlabels -= 1;
                  LABEL[w] = label;
                  LifoPush(LIFO, w);
                } 
		else
		  if (F[y] == F[w])
		  {
		    if (((label > 0) && (LABEL[y] == -1)) ||
			((label == 0) && (LABEL[y] != 0)))
		    {
		      LABEL[y] = label;
		      LifoPush(LIFO, y);
		    } /* if .. */
		  } /* if F ... */
              } /* if (y != -1) */
	  } /* for k ... */
	  
	} /* while  (! LifoVide(LIFO)) */
      } /* if (LABEL[x] == 0) */
    } /* for (x = 0; x < N; x++) */
  
  /* on prepare l'image de sortie */
  
  for(x = 0; x < 2*N; x++) if(LABEL[x] > 0) G[x] = 255; else G[x] = 0;
  
  LifoTermine(LIFO);
  free(LABEL);
  if (tailleplateau != 2 * N) *nlabels += 1; /* pour le niveau 0 */
  return 1;
}

int32_t jclabelextrema(struct xvimage *in,         /* GA de depart */
		       uint32_t **label,           /* resultat: carte de labels pour les minima du GA */
		       int32_t minimum,            /* booleen, 1: on recherche les minima, 0: on recherche les maxima */
		   int32_t *nlabels                /* resultat: le nombre d'extrema */
		   )
{
  int32_t i,j,k;                        
  int32_t x,y,w;
  uint8_t *F;
  int32_t rs = rowsize(in);
  int32_t cs = colsize(in);
  int32_t N = rs * cs;
  //  int32_t tailleplateau;
  Lifo * LIFO;
  uint32_t lab;
  
  F = UCHARDATA(in);
  if( ((*label) = (uint32_t *)malloc(sizeof(uint32_t)*2*N)) == NULL)
  {   
    fprintf(stderr, "jclabelextrema() : echec de malloc\n");
    return(0);
  }

  for (x = 0; x < 2*N; x++) (*label)[x] = -1;
  
  LIFO = CreeLifoVide(2*N);
  if (LIFO == NULL)
  {   
    fprintf(stderr, "jclabelextrema() : CreeLifoVide failed\n");
    return(0);
  }

  *nlabels = 0;
  /* on explore d'abord les aretes horizontales */

  for(j = 0; j < cs; j++)
    for(i = 0; i < rs -1; i++)
    {
      x = j * rs + i;
      if ((*label)[x] == -1)          /* on trouve un point x non etiquete */
      {
	*nlabels += 1;             /* on cree un numero d'etiquette */
	(*label)[x] = *nlabels;
	LifoPush(LIFO, x);         /* on va parcourir le plateau auquel appartient x */
	//	tailleplateau = 0;
	while (! LifoVide(LIFO))
	{
	  // tailleplateau++;
	  w = LifoPop(LIFO);
	  lab = (*label)[w];
	  for (k = 0; k < 6; k++)
	  {
	    y = voisinGA(w, k, rs, N);
	     if (y != -1)
              {
                if ((lab > 0) && 
                    ( (minimum && (F[y] < F[w])) ||
                      (!minimum && (F[y] > F[w]))
		      )
		    )
		{    /* w non dans un minimum (resp. maximum) */
                  lab = 0;
                  *nlabels -= 1;
		  (*label)[w] = lab;
                  LifoPush(LIFO, w);
                } 
		else
		  if (F[y] == F[w])
		  {
		    if (((lab > 0) && ((*label)[y] == -1)) ||
			((lab == 0) && ((*label)[y] != 0)))
		    {
		      (*label)[y] = lab;
		      LifoPush(LIFO, y);
		    } /* if .. */
		  } /* if F ... */
              } /* if (y != -1) */
	  } /* for k ... */
	  
	} /* while  (! LifoVide(LIFO)) */
      } /* if (LABEL[x] == 0) */
    } /* for (x = 0; x < N; x++) */
  
  /* Puis on explore les aretes verticales */
  for(j = 0; j < cs -1; j++)
    for(i = 0; i < rs; i++)
    {
      x = N + j * rs + i;
      if ((*label)[x] == -1)          /* on trouve un point x non etiquete */
      {
	*nlabels += 1;             /* on cree un numero d'etiquette */
	(*label)[x] = *nlabels;
	LifoPush(LIFO, x);         /* on va parcourir le plateau auquel appartient x */
	//tailleplateau = 0;
	while (! LifoVide(LIFO))
	{
	  // tailleplateau++;
	  w = LifoPop(LIFO);
	  lab = (*label)[w];
	  for (k = 0; k < 6; k++)
	  {
	    y = voisinGA(w, k, rs, N);
	     if (y != -1)
              {
		//printf("horiz voisin no %d de %d = %d\n",k,w,y);
                if ((lab > 0) && 
                    ( (minimum && (F[y] < F[w])) ||
                      (!minimum && (F[y] > F[w]))
		      )
		    )
		{    /* w non dans un minimum (resp. maximum) */
                  lab = 0;
                  *nlabels -= 1;
		  (*label)[w] = lab;
                  LifoPush(LIFO, w);
                } 
		else
		  if (F[y] == F[w])
		  {
		    if (((lab > 0) && ((*label)[y] == -1)) ||
			((lab == 0) && ((*label)[y] != 0)))
		    {
		      (*label)[y] = lab;
		      LifoPush(LIFO, y);
		    } /* if .. */
		  } /* if F ... */
              } /* if (y != -1) */
	  } /* for k ... */
	  
	} /* while  (! LifoVide(LIFO)) */
      } /* if (LABEL[x] == 0) */
    } /* for (x = 0; x < N; x++) */
  
  /* on prepare l'image de sortie */
    
  LifoTermine(LIFO);
  //if (tailleplateau != 2 * N) 
  *nlabels += 1; /* pour le niveau 0 */
  return 1;
}
