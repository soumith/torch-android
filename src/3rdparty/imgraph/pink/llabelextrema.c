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
#include <mccodimage.h>
#include <mclifo.h>
#include <llabelextrema.h>

//#define DEBUG

/****************************************************************
*
* Routine Name: llabelextrema - library call for labelextrema
*
* Purpose: etiquetage des extrema d'une image 2d (en 4 ou 8 connexite ou 0/1 en perfect fusion) 
*          ou 3d (en 6, 18 ou 26 connexite) par des labels differents
*
* Input:
* Output:
* Written By: Michel Couprie
* Update: april 1998 - image d'entiers longs
* Update: jan 2005 - llabeldil
* 
*
* Remarques:
*   l'efficacite peut etre notablement amelioree en sortant les tests (switch, etc)
*   des boucles - a faire
*
****************************************************************/

/* ==================================== */
int32_t llabelextrema(
        struct xvimage *img,     /* image de depart */
        int32_t connex,          /* 4, 8 (2d) ou 6, 18, 26 (3d) 0/1 pour biconnecte */
        int32_t minimum,         /* booleen */
        struct xvimage *lab,     /* resultat: image de labels */
        int32_t *nlabels)        /* resultat: nombre d'extrema traites + 1 (0 = non extremum) */
/* ==================================== */
#undef F_NAME
#define F_NAME "llabelextrema"
{
  int32_t k, w, x, y;
  uint8_t *F;
  int32_t *FL;
  int32_t *LABEL =  SLONGDATA(lab);
  int32_t rs = rowsize(img);
  int32_t cs = colsize(img);
  int32_t d = depth(img);
  int32_t n = rs * cs;          /* taille plan */
  int32_t N = n * d;            /* taille image */
  int32_t tailleplateau;
  Lifo * LIFO;
  int32_t label;

  if (datatype(lab) != VFF_TYP_4_BYTE) 
  {
    fprintf(stderr, "%s: le resultat doit etre de type VFF_TYP_4_BYTE\n", F_NAME);
    return 0;
  }

  if ((rowsize(lab) != rs) || (colsize(lab) != cs) || (depth(lab) != d))
  {
    fprintf(stderr, "%s: tailles images incompatibles\n", F_NAME);
    return 0;
  }

  /* le LABEL initialement est mis a -1 */
  for (x = 0; x < N; x++) LABEL[x] = -1;

  LIFO = CreeLifoVide(N);
  if (LIFO == NULL)
  {   fprintf(stderr, "%s : CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  *nlabels = 0;

if (datatype(img) == VFF_TYP_1_BYTE) 
{
  F = UCHARDATA(img);
  for (x = 0; x < N; x++)
  {
    if (LABEL[x] == -1)          /* on trouve un point x non etiquete */
    {
      *nlabels += 1;             /* on cree un numero d'etiquette */
      LABEL[x] = *nlabels;
      LifoPush(LIFO, x);         /* on va parcourir le plateau auquel appartient x */
      tailleplateau = 0;
#ifdef DEBUG
      printf("(%d,%d): init label %d\n", x%rs, x/rs, *nlabels);
#endif
      while (! LifoVide(LIFO))
      {
	tailleplateau++;
        w = LifoPop(LIFO);
        label = LABEL[w];
#ifdef DEBUG
	printf("Pop(%d,%d): label %d\n", w%rs, w/rs, label);
#endif
        switch (connex)
        {
	  case 4:
            for (k = 0; k < 8; k += 2)
            {
              y = voisin(w, k, rs, N);
              if (y != -1)
              {
                if ((label > 0) && 
                    ( (minimum && (F[y] < F[w])) ||
                      (!minimum && (F[y] > F[w]))
                    )
                   )
                {   /* w non dans un minimum (resp. maximum) */
		  tailleplateau = 0;
                  label = 0;
                  *nlabels -= 1;
                  LABEL[w] = label;
		  LifoFlush(LIFO);
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
            break;
	case 0:  /* voisinage biconnecte */
	   for (k = 0; k < 6; k ++)
            {
              y = voisin6b(w, k, rs, N, 0);
              if (y != -1)
		{
		  if ((label > 0) && 
		      ( (minimum && (F[y] < F[w])) ||
			(!minimum && (F[y] > F[w]))
			)
                   )
		    {   /* w non dans un minimum (resp. maximum) */
		      tailleplateau = 0;
		      label = 0;
		      *nlabels -= 1;
		      LABEL[w] = label;
		      LifoFlush(LIFO);
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
	   break;
	case 1:  /* voisinage biconnecte */
	   for (k = 0; k < 6; k ++)
            {
              y = voisin6b(w, k, rs, N, 1);
              if (y != -1)
		{
		  if ((label > 0) && 
		      ( (minimum && (F[y] < F[w])) ||
			(!minimum && (F[y] > F[w]))
			)
                   )
		    {   /* w non dans un minimum (resp. maximum) */
		      tailleplateau = 0;
		      label = 0;
		      *nlabels -= 1;
		      LABEL[w] = label;
		      LifoFlush(LIFO);		      
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
	   break;
	case 14:  /* voisinage biconnecte */
	   for (k = 0; k < 14; k ++)
            {
              y = voisin14b(w, k, rs, N, 1);
              if (y != -1)
		{
		  if ((label > 0) && 
		      ( (minimum && (F[y] < F[w])) ||
			(!minimum && (F[y] > F[w]))
			)
                   )
		    {   /* w non dans un minimum (resp. maximum) */
		      tailleplateau = 0;
		      label = 0;
		      *nlabels -= 1;
		      LABEL[w] = label;
		      LifoFlush(LIFO);
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
	   break;
	  case 6:
            for (k = 0; k <= 10; k += 2) /* parcourt les 6 voisins */
            {
              y = voisin6(w, k, rs, n, N);
              if (y != -1)
              {
                if ((label > 0) && 
                    ( (minimum && (F[y] < F[w])) ||
                      (!minimum && (F[y] > F[w]))
                    )
                   )
                {   /* w non dans un minimum (resp. maximum) */
		  tailleplateau = 0;
                  label = 0;
                  *nlabels -= 1;
                  LABEL[w] = label;
		  LifoFlush(LIFO);
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
            break;

	  case 8:
            for (k = 0; k < 8; k += 1)
            {
              y = voisin(w, k, rs, N);
              if (y != -1)
              {
                if ((label > 0) && 
                    ( (minimum && (F[y] < F[w])) ||
                      (!minimum && (F[y] > F[w]))
                    )
                   )
                {   /* w non dans un minimum (resp. maximum) */
		  tailleplateau = 0;
                  label = 0;
                  *nlabels -= 1;
                  LABEL[w] = label;
		  LifoFlush(LIFO);
                  LifoPush(LIFO, w);
#ifdef DEBUG
		  printf("Push(%d,%d), non extremum: label %d\n", w%rs, w/rs, label);
#endif
                } 
                else
                if (F[y] == F[w])
                {
                  if (((label > 0) && (LABEL[y] == -1)) ||
                      ((label == 0) && (LABEL[y] != 0)))
                  {
                    LABEL[y] = label;
                    LifoPush(LIFO, y);
#ifdef DEBUG
		    printf("Push(%d,%d): label %d\n", y%rs, y/rs, label);
#endif
                  } /* if .. */
                } /* if F ... */
              } /* if (y != -1) */
            } /* for k ... */
            break;

	  case 18:
            for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
            {
              y = voisin18(w, k, rs, n, N);
              if (y != -1)
              {
                if ((label > 0) && 
                    ( (minimum && (F[y] < F[w])) ||
                      (!minimum && (F[y] > F[w]))
                    )
                   )
                {   /* w non dans un minimum (resp. maximum) */
		  tailleplateau = 0;
                  label = 0;
                  *nlabels -= 1;
                  LABEL[w] = label;
		  LifoFlush(LIFO);
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
            break;

	  case 26:
            for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
            {
              y = voisin26(w, k, rs, n, N);
              if (y != -1)
              {
                if ((label > 0) && 
                    ( (minimum && (F[y] < F[w])) ||
                      (!minimum && (F[y] > F[w]))
                    )
                   )
                {   /* w non dans un minimum (resp. maximum) */
		  tailleplateau = 0;
                  label = 0;
                  *nlabels -= 1;
                  LABEL[w] = label;
		  LifoFlush(LIFO);
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
            break;

	} /* switch (connex) */
      } /* while (! LifoVide(LIFO)) */
    } /* if (LABEL[x] == 0) */
  } /* for (x = 0; x < N; x++) */
} /* if (datatype(img) == VFF_TYP_1_BYTE) */
else if (datatype(img) == VFF_TYP_4_BYTE) 
{
  FL = SLONGDATA(img);
  for (x = 0; x < N; x++)
  {
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
        switch (connex)
        {
	  case 4:
            for (k = 0; k < 8; k += 2)
            {
              y = voisin(w, k, rs, N);
              if (y != -1)
              {
                if ((label > 0) && 
                    ( (minimum && (FL[y] < FL[w])) ||
                      (!minimum && (FL[y] > FL[w]))
                    )
                   )
                {   /* w non dans un minimum (resp. maximum) */
		  tailleplateau = 0;
                  label = 0;
                  *nlabels -= 1;
                  LABEL[w] = label;
		  LifoFlush(LIFO);
                  LifoPush(LIFO, w);
                } 
                else
                if (FL[y] == FL[w])
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
            break;
	case 0: 	  
	  for (k = 0; k < 6; k ++)
            {
              y = voisin6b(w, k, rs, N, 0);
              if (y != -1)
		{
		  if ((label > 0) && 
		      ( (minimum && (FL[y] < FL[w])) ||
			(!minimum && (FL[y] > FL[w]))
			)
		      )
		    {   /* w non dans un minimum (resp. maximum) */
		      tailleplateau = 0;
		      label = 0;
		      *nlabels -= 1;
		      LABEL[w] = label;
		      LifoFlush(LIFO);
		      LifoPush(LIFO, w);
		    } 
		  else
		    if (FL[y] == FL[w])
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
	  break;
	case 1: 	  
	  for (k = 0; k < 6; k ++)
            {
              y = voisin6b(w, k, rs, N, 1);
              if (y != -1)
		{
		  if ((label > 0) && 
		      ( (minimum && (FL[y] < FL[w])) ||
			(!minimum && (FL[y] > FL[w]))
			)
		      )
		    {   /* w non dans un minimum (resp. maximum) */
		      tailleplateau = 0;
		      label = 0;
		      *nlabels -= 1;
		      LABEL[w] = label;
		      LifoFlush(LIFO);
		      LifoPush(LIFO, w);
		    } 
		  else
		    if (FL[y] == FL[w])
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
	  break;
	case 14:  /* voisinage biconnecte 3D */
	   for (k = 0; k < 14; k ++)
            {
              y = voisin14b(w, k, rs, N, 1);
              if (y != -1)
		{
		  if ((label > 0) && 
		      ( (minimum && (F[y] < F[w])) ||
			(!minimum && (F[y] > F[w]))
			)
                   )
		    {   /* w non dans un minimum (resp. maximum) */
		      tailleplateau = 0;
		      label = 0;
		      *nlabels -= 1;
		      LABEL[w] = label;
		      LifoFlush(LIFO);
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
	   break;
	case 6:
	  for (k = 0; k <= 10; k += 2) /* parcourt les 6 voisins */
            {
              y = voisin6(w, k, rs, n, N);
              if (y != -1)
              {
                if ((label > 0) && 
                    ( (minimum && (FL[y] < FL[w])) ||
                      (!minimum && (FL[y] > FL[w]))
                    )
                   )
                {   /* w non dans un minimum (resp. maximum) */
		  tailleplateau = 0;
                  label = 0;
                  *nlabels -= 1;
                  LABEL[w] = label;
		  LifoFlush(LIFO);
                  LifoPush(LIFO, w);
                } 
                else
                if (FL[y] == FL[w])
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
            break;

	  case 8:
            for (k = 0; k < 8; k += 1)
            {
              y = voisin(w, k, rs, N);
              if (y != -1)
              {
                if ((label > 0) && 
                    ( (minimum && (FL[y] < FL[w])) ||
                      (!minimum && (FL[y] > FL[w]))
                    )
                   )
                {   /* w non dans un minimum (resp. maximum) */
		  tailleplateau = 0;
                  label = 0;
                  *nlabels -= 1;
                  LABEL[w] = label;
		  LifoFlush(LIFO);
                  LifoPush(LIFO, w);
                } 
                else
                if (FL[y] == FL[w])
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
            break;

	  case 18:
            for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
            {
              y = voisin18(w, k, rs, n, N);
              if (y != -1)
              {
                if ((label > 0) && 
                    ( (minimum && (FL[y] < FL[w])) ||
                      (!minimum && (FL[y] > FL[w]))
                    )
                   )
                {   /* w non dans un minimum (resp. maximum) */
		  tailleplateau = 0;
                  label = 0;
                  *nlabels -= 1;
                  LABEL[w] = label;
		  LifoFlush(LIFO);
                  LifoPush(LIFO, w);
                } 
                else
                if (FL[y] == FL[w])
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
            break;

	  case 26:
            for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
            {
              y = voisin26(w, k, rs, n, N);
              if (y != -1)
              {
                if ((label > 0) && 
                    ( (minimum && (FL[y] < FL[w])) ||
                      (!minimum && (FL[y] > FL[w]))
                    )
                   )
                {   /* w non dans un minimum (resp. maximum) */
		  tailleplateau = 0;
                  label = 0;
                  *nlabels -= 1;
                  LABEL[w] = label;
		  LifoFlush(LIFO);
                  LifoPush(LIFO, w);
                } 
                else
                if (FL[y] == FL[w])
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
            break;

	} /* switch (connex) */
      } /* while (! LifoVide(LIFO)) */
    } /* if (LABEL[x] == 0) */
  } /* for (x = 0; x < N; x++) */
} /* if (datatype(img) == VFF_TYP_4_BYTE) */
else
{
  fprintf(stderr, "%s: bad data type\n", F_NAME);
  return 0;
}

  LifoTermine(LIFO);
//  if (tailleplateau != N) 
     *nlabels += 1; /* pour le niveau 0 */

#ifdef DEBUG
      printf("%s: end: nlabels = %d\n", F_NAME, *nlabels);
#endif

  return(1);
} /* llabelextrema() */

/* ==================================== */
int32_t llabeldil(struct xvimage *f, 
	      struct xvimage *m, 
	      int32_t xc, int32_t yc, 
	      struct xvimage *lab, /* resultat: image de labels */
	      int32_t *nlabels)        /* resultat: nombre de composantes + 1 */
/* labels the connected components defined by the reflexive and symmetric structuring
   element m.
   m : masque representant l'element structurant
   xc, yc : coordonnees de l'origine de l'element structurant 
   lab : 
*/
/* ==================================== */
#undef F_NAME
#define F_NAME "llabeldil"
{
  int32_t x, y, v, w;           /* index muet de pixel */
  register int32_t i, j;           /* index muet */
  register int32_t k, l, c;        /* index muet */
  int32_t rs = rowsize(f);         /* taille ligne */
  int32_t cs = colsize(f);         /* taille colonne */
  int32_t N = rs * cs;             /* taille image */
  int32_t rsm = rowsize(m);        /* taille ligne masque */
  int32_t csm = colsize(m);        /* taille colonne masque */
  int32_t Nm = rsm * csm;
  uint8_t *M = UCHARDATA(m);
  uint8_t *F = UCHARDATA(f);
  int32_t *LABEL = SLONGDATA(lab);
  int32_t label;
  int32_t nptb;                    /* nombre de points de l'e.s. */
  int32_t *tab_es_x;               /* liste des coord. x des points de l'e.s. */
  int32_t *tab_es_y;               /* liste des coord. y des points de l'e.s. */
  Lifo * LIFO;

  if (depth(f) != 1) 
  {
    fprintf(stderr, "%s: not for 3d images\n", F_NAME);
    return 0;
  }

  if (!M[yc * rsm + xc]) /* l'element structurant N'est PAS reflexif */
  {
    fprintf(stderr, "%s: l'element structurant doit etre reflexif\n", F_NAME);
    return 0;
  }

  if (datatype(lab) != VFF_TYP_4_BYTE) 
  {
    fprintf(stderr, "%s: le resultat doit etre de type VFF_TYP_4_BYTE\n", F_NAME);
    return 0;
  }

  if ((rowsize(lab) != rs) || (colsize(lab) != cs) || (depth(lab) != 1))
  {
    fprintf(stderr, "%s: tailles images incompatibles\n", F_NAME);
    return 0;
  }

#define NOLABEL 0
  /* le LABEL initialement est mis a NOLABEL */
  for (x = 0; x < N; x++) LABEL[x] = NOLABEL;

  LIFO = CreeLifoVide(N);
  if (LIFO == NULL)
  {   fprintf(stderr, "%s : CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  label = NOLABEL;

  nptb = 0;
  for (i = 0; i < Nm; i += 1)
    if (M[i])
      nptb += 1;

  tab_es_x = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  tab_es_y = (int32_t *)calloc(1,nptb * sizeof(int32_t));
  if ((tab_es_x == NULL) || (tab_es_y == NULL))
  {  
     fprintf(stderr,"%s : malloc failed for tab_es\n", F_NAME);
     return(0);
  }

  k = 0;
  for (j = 0; j < csm; j += 1)
    for (i = 0; i < rsm; i += 1)
      if (M[j * rsm + i])
      {
         tab_es_x[k] = i;
         tab_es_y[k] = j;
         k += 1;
      }

  for (y = 0; y < cs; y++)
  for (x = 0; x < rs; x++)
  {
    w = y * rs + x;
    if ((F[w]) && (LABEL[w] == NOLABEL))  /* on trouve un point w non etiquete */
    {
      label += 1;       /* on cree un numero d'etiquette */
      LABEL[w] = label;
      LifoPush(LIFO, w);   /* on va parcourir la composante a laquelle appartient w */
      while (! LifoVide(LIFO))
      {
        w = LifoPop(LIFO);
	x = w % rs;
	y = w / rs;
        for (c = 0; c < nptb ; c += 1)
        {
          l = y + tab_es_y[c] - yc;
          k = x + tab_es_x[c] - xc; 
	  v = l*rs + k;
          if ((l >= 0) && (l < cs) && (k >= 0) && (k < rs) &&
	      (F[v]) && (LABEL[v] == NOLABEL))
	  {
	    LABEL[v] = label;
	    LifoPush(LIFO, v);
	  }
        }
      }
    }
  }

  *nlabels = label;
  LifoTermine(LIFO);
  free(tab_es_x);
  free(tab_es_y);
  return 1;
} /* llabeldil() */

/* ==================================== */
int32_t llabelbin(struct xvimage *f, 
	      int32_t connex,
	      struct xvimage *lab, /* resultat: image de labels */
	      int32_t *nlabels)        /* resultat: nombre de composantes + 1 */
/* labels the connected components according to adjacency relation connex.
   lab : 
*/
/* ==================================== */
#undef F_NAME
#define F_NAME "llabelbin"
{
  int32_t x, v, w;           /* index muet de pixel */
  register int32_t k;        /* index muet */
  int32_t rs = rowsize(f);         /* taille ligne */
  int32_t cs = colsize(f);         /* taille colonne */
  int32_t ds = depth(f);           /* nb plans */
  int32_t ps = rs * cs;            /* taille plan */
  int32_t N = ps * ds;             /* taille image */
  uint8_t *F = UCHARDATA(f);
  int32_t *LABEL = SLONGDATA(lab);
  int32_t label;
  Lifo * LIFO;

  if (datatype(lab) != VFF_TYP_4_BYTE) 
  {
    fprintf(stderr, "%s: le resultat doit etre de type VFF_TYP_4_BYTE\n", F_NAME);
    return 0;
  }

  if ((rowsize(lab) != rs) || (colsize(lab) != cs) || (depth(lab) != ds))
  {
    fprintf(stderr, "%s: tailles images incompatibles\n", F_NAME);
    return 0;
  }

#undef NOLABEL
#define NOLABEL 0
  /* le LABEL initialement est mis a NOLABEL */
  for (x = 0; x < N; x++) LABEL[x] = NOLABEL;

  LIFO = CreeLifoVide(N);
  if (LIFO == NULL)
  {   fprintf(stderr, "%s : CreeLifoVide failed\n", F_NAME);
      return(0);
  }

  label = NOLABEL;

  for (w = 0; w < N; w++)
  {
    if ((F[w]) && (LABEL[w] == NOLABEL))  /* on trouve un point w non etiquete */
    {
      label += 1;       /* on cree un numero d'etiquette */
      LABEL[w] = label;
      LifoPush(LIFO, w);   /* on va parcourir la composante a laquelle appartient w */
      while (! LifoVide(LIFO))
      {
        x = LifoPop(LIFO);
	switch (connex)
	{
	case 4: 
	  for (k = 0; k < 8; k += 2)
	  {
	    v = voisin(x, k, rs, N);
	    if ((v != -1)  && (F[v]) && (LABEL[v] == NOLABEL))
	    {
	      LABEL[v] = label;
	      LifoPush(LIFO, v);
	    }
	  }
	  break;
	case 8: 
	  for (k = 0; k < 8; k += 1)
	  {
	    v = voisin(x, k, rs, N);
	    if ((v != -1)  && (F[v]) && (LABEL[v] == NOLABEL))
	    {
	      LABEL[v] = label;
	      LifoPush(LIFO, v);
	    }
	  }
	  break;
	case 6: 
	  for (k = 0; k <= 10; k += 2) /* parcourt les 6 voisins */
	  {
            v = voisin6(x, k, rs, ps, N);
	    if ((v != -1)  && (F[v]) && (LABEL[v] == NOLABEL))
	    {
	      LABEL[v] = label;
	      LifoPush(LIFO, v);
	    }
	  }
	  break;
	case 18:
	  for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
	  {
            v = voisin18(x, k, rs, ps, N);
	    if ((v != -1)  && (F[v]) && (LABEL[v] == NOLABEL))
	    {
	      LABEL[v] = label;
	      LifoPush(LIFO, v);
	    }
	  }
	  break;
	case 26:
	  for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
	  {
            v = voisin26(x, k, rs, ps, N);
	    if ((v != -1)  && (F[v]) && (LABEL[v] == NOLABEL))
	    {
	      LABEL[v] = label;
	      LifoPush(LIFO, v);
	    }
	  }
	  break;
	default:
	  fprintf(stderr, "%s : %d : bad connexity\n", F_NAME, connex);
	  return(0);
	} // switch (connex)
      }
    }
  }

  *nlabels = label+1;
  LifoTermine(LIFO);
  return 1;
} /* llabelbin() */
