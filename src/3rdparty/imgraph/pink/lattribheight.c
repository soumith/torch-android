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
   Operateurs connexes bases sur l'attribut hauteur
   =============================================
   (algorithme de P. Salembier)

   Operateurs : 
      lsegmentheight
      lheightmaxima
      lheightselnb   (selection d'un nombre donne de composantes)

   Michel Couprie - 1999-2002
*/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mclifo.h>
#include <mcfahsalembier.h>
#include <mcutil.h>
#include <mcindic.h>
#include <lattribheight.h>

#define PARANO
/*
#define VERBOSE
#define DEBUG
#define DEBUGRECONS
#define DEBUGFLOOD
*/

#define ATTR_HEIGHT

#include "lattrib.h"

/* ======================================================================== */
/* ======================================================================== */
/* OPERATEURS BASES SUR L'ARBRE DES COMPOSANTES */
/* ======================================================================== */
/* ======================================================================== */

/* ==================================== */
int32_t lsegmentheight(struct xvimage *image, int32_t connex, int32_t param, int32_t maximise)
/* ==================================== */
{
  register int32_t i, k;         /* index muet */
  int32_t rs = rowsize(image);      /* taille ligne */
  int32_t cs = colsize(image);      /* taille colonne */
  int32_t ds = depth(image);        /* nb plans */
  int32_t ps = rs * cs;             /* taille plan */
  int32_t N = ps * ds;              /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  Fahs * FAHS;                    /* la file d'attente hierarchique */
  int32_t incr_vois;                /* 1 pour la 8-connexite,  2 pour la 4-connexite */
  uint32_t *STATUS;         /* etat d'un pixel - doit etre initialise a NOT_ANALYZED */
                                /* en sortie, contient le numero de la composante de niveau h */
                                /* qui contient le pixel (avec h = valeur du pixel) */
  uint32_t *number_nodes;   /* nombre de composantes par niveau */
  uint8_t *node_at_level; /* tableau de booleens */
  CompTree * TREE;              /* resultat : l'arbre des composantes */
  CompactTree * CTREE;          /* resultat : l'arbre des composantes compacte' */

  switch (connex)
    {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
    } /* switch (connex) */

  FAHS = CreeFahsVide(N);

  STATUS = (uint32_t *)malloc(N * sizeof(int32_t));
  if (STATUS == NULL)
  {   fprintf(stderr, "lsegmentheight() : malloc failed for STATUS\n");
      return(0);
  }

  number_nodes = (uint32_t *)calloc(256, sizeof(int32_t));
  if (number_nodes == NULL)
  {   fprintf(stderr, "lsegmentheight() : calloc failed for number_nodes\n");
      return(0);
  }

  node_at_level = (uint8_t *)calloc(256, sizeof(char));
  if (node_at_level == NULL)
  {   fprintf(stderr, "lsegmentheight() : calloc failed for node_at_level\n");
      return(0);
  }
  
  TREE = InitCompTree(N);
  if (TREE == NULL)
  {   fprintf(stderr, "lsegmentheight() : InitCompTree failed\n");
      return(0);
  }

  /* ================================================ */
  /* INITIALISATIONS                                  */
  /* ================================================ */

  for (i = 0; i < N; i++) STATUS[i] = NOT_ANALYZED;
  k = 0;             /* recherche un pixel k de niveau de gris minimal dans l'image */
  for (i = 1; i < N; i++) if (F[i] < F[k]) k = i;
  FahsPush(FAHS, k, F[k]);

#ifdef VERBOSE
  fprintf(stderr, "init terminee\n");
#endif

  /* ================================================ */
  /* APPEL FONCTION RECURSIVE flood                   */
  /* ================================================ */

  if (ds == 1) {
    if ((connex == 4) || (connex == 8))
      (void)flood(F[k], FAHS, STATUS, number_nodes, node_at_level, TREE, incr_vois, rs, N, F); 
    else if ((connex == 0) || (connex == 1))
      (void)floodb(F[k], FAHS, STATUS, number_nodes, node_at_level, TREE, connex, rs, N, F); 
  } else
    (void)flood3d(F[k], FAHS, STATUS, number_nodes, node_at_level, TREE, connex, rs, ps, N, F);

#ifdef VERBOSE
  fprintf(stderr, "flood terminee\n");
#endif
#ifdef DEBUG
  AfficheCompTree(TREE);
#endif

  CTREE = CompTree2CompactTree(TREE, number_nodes);

#ifdef VERBOSE
  fprintf(stderr, "CompTree2CompactTree terminee\n");
#endif

  CalculeAttributs(CTREE);

#ifdef VERBOSE
  fprintf(stderr, "CalculeAttributs terminee\n");
#endif

#ifdef DEBUG
  AfficheCompactTree(CTREE);
#endif

  (void)FiltreHeightRec(CTREE, 0, param);

#ifdef VERBOSE
  fprintf(stderr, "FiltreHeightRec terminee\n");
#endif

  if ( maximise )
    (void)MaximiseSegmentation(CTREE, 0);

#ifdef VERBOSE
  fprintf(stderr, "MaximiseSegmentation terminee\n");
#endif

#ifdef DEBUG
  AfficheCompactTree(CTREE);
  WriteCompactTree(CTREE, "ctree.graph");
#endif

  RecupereSegmentation(CTREE, STATUS, rs, N, F);

#ifdef VERBOSE
  fprintf(stderr, "RecupereSegmentation terminee\n");
#endif

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  FahsTermine(FAHS);
  TermineCompTree(TREE);
  TermineCompactTree(CTREE);
  free(STATUS);
  free(number_nodes);
  free(node_at_level);
  return(1);
} /* lsegmentheight() */

#ifdef __GNUC__
static void ProcessLeafMins(CompactTree * cpct, int32_t som) __attribute__ ((unused));
#endif
/* ==================================== */
static void ProcessLeafMins(CompactTree * cpct, int32_t som)
/* ==================================== */
/*
  Recherche a partir du sommet som, les sommets marques LEAFMIN.
  Demarque en remontant jusqu'au premier noeud.
  Version pour un filtre morphologique (avec RecupereImageFiltreeH).
*/
{
  int32_t i, j, n;

  if (!(cpct->flags[som] & LEAFMIN))
  {
    n = NBFILS(som);           /* remonte l'arbre pour trouver une LEAFMIN */ 
    for (i = 0; i < n; i++) 
    {
      j = INDEXFILS(som, i);
      j = cpct->fils[j];
      ProcessLeafMins(cpct, j);
    }
  }
  else /* (on a trouve une LEAFMIN) */
  {
    cpct->flags[som] &= ~LEAFMIN; /* demarque som */
    while (NBFILS(som) == 1)      /* remonte jusqu'au premier noeud ou feuille */
    {
      som = INDEXFILS(som, 0);
      som = cpct->fils[som];
      cpct->flags[som] &= ~FILTERED_OUT;
    }
    cpct->flags[som] |= LEAFMIN;  /* marque */
  }
} /* ProcessLeafMins() */

/* ==================================== */
static void ProcessLeafMinsOp(CompactTree * cpct, int32_t som)
/* ==================================== */
/*
  Recherche a partir du sommet som, les sommets marques LEAFMIN.
  Demarque en remontant jusqu'a la feuille la plus haute.
  Version pour une ouverture (avec RecupereImageFiltree).
*/
{
  int32_t i, j, n, m, jm, h;

  if (!(cpct->flags[som] & LEAFMIN))
  {
    n = NBFILS(som);           /* remonte l'arbre pour trouver une LEAFMIN */ 
    for (i = 0; i < n; i++) 
    {
      j = INDEXFILS(som, i);
      j = cpct->fils[j];
      ProcessLeafMinsOp(cpct, j);
    }
  }
  else /* (on a trouve une LEAFMIN) */
  {
    cpct->flags[som] &= ~LEAFMIN; /* demarque som */
    while (NBFILS(som) > 0)       /* remonte jusqu'a une (vraie) feuille */
    {
      /* recherche l'indice im du fils de hauteur maxi */
      n = NBFILS(som);
      m = -1;
      for (i = 0; i < n; i++) 
      {
        j = INDEXFILS(som, i);
        j = cpct->fils[j];
        h = DECODENIV(cpct->comp[j]) + cpct->height[j];
        if (h > m) { m = h; jm = j; } 
      }
      /* remonte en demarquant */
      som = jm;
      cpct->flags[som] &= ~FILTERED_OUT;
    }
    cpct->flags[som] |= LEAFMIN;  /* marque LEAFMIN le sommet atteint */
  }
} /* ProcessLeafMinsOp() */

#ifdef __GNUC__
static void RecupereImageFiltreeH(CompactTree * cpct, uint32_t *STATUS, int32_t rs, int32_t N, uint8_t *ORI) __attribute__ ((unused));
#endif
/* ==================================== */
static void RecupereImageFiltreeH(CompactTree * cpct,           
       uint32_t *STATUS,
       int32_t rs, int32_t N, 
       uint8_t *ORI            /* informations sur l'image originale */
)
/* ==================================== */
{
  int32_t i, h;
  uint32_t c, comp;
  for (i = 0; i < N; i++) 
  {
    h = ORI[i];
    c = STATUS[i];
    comp = INDEXCOMP(h,c);
    while (cpct->flags[comp] == FILTERED_OUT) comp = cpct->pere[comp];
    if (cpct->flags[comp] & LEAFMIN)
      ORI[i] = DECODENIV(cpct->comp[comp]) + cpct->height[comp];
    else
      ORI[i] = DECODENIV(cpct->comp[comp]);
  }  
} /* RecupereImageFiltreeH() */

/* ==================================== */
int32_t lheightmaxima_variante(struct xvimage *image, int32_t connex, int32_t param)
/* ==================================== */
{
  register int32_t i, k;         /* index muet */
  int32_t rs = rowsize(image);      /* taille ligne */
  int32_t cs = colsize(image);      /* taille colonne */
  int32_t ds = depth(image);        /* nb plans */
  int32_t ps = rs * cs;             /* taille plan */
  int32_t N = ps * ds;              /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  Fahs * FAHS;                    /* la file d'attente hierarchique */
  int32_t incr_vois;                /* 1 pour la 8-connexite,  2 pour la 4-connexite */
  uint32_t *STATUS;         /* etat d'un pixel - doit etre initialise a NOT_ANALYZED */
                                /* en sortie, contient le numero de la composante de niveau h */
                                /* qui contient le pixel (avec h = valeur du pixel) */
  uint32_t *number_nodes;   /* nombre de composantes par niveau */
  uint8_t *node_at_level; /* tableau de booleens */
  CompTree * TREE;              /* resultat : l'arbre des composantes */
  CompactTree * CTREE;          /* resultat : l'arbre des composantes compacte' */

  switch (connex)
    {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
    } /* switch (connex) */

  FAHS = CreeFahsVide(N);

  STATUS = (uint32_t *)malloc(N * sizeof(int32_t));
  if (STATUS == NULL)
  {   fprintf(stderr, "lheightopening() : malloc failed for STATUS\n");
      return(0);
  }

  number_nodes = (uint32_t *)calloc(256, sizeof(int32_t));
  if (number_nodes == NULL)
  {   fprintf(stderr, "lheightopening() : calloc failed for number_nodes\n");
      return(0);
  }

  node_at_level = (uint8_t *)calloc(256, sizeof(char));
  if (node_at_level == NULL)
  {   fprintf(stderr, "lheightopening() : calloc failed for node_at_level\n");
      return(0);
  }
  
  TREE = InitCompTree(N);
  if (TREE == NULL)
  {   fprintf(stderr, "lheightopening() : InitCompTree failed\n");
      return(0);
  }

  /* ================================================ */
  /* INITIALISATIONS                                  */
  /* ================================================ */

  for (i = 0; i < N; i++) STATUS[i] = NOT_ANALYZED;
  k = 0;             /* recherche un pixel k de niveau de gris minimal dans l'image */
  for (i = 1; i < N; i++) if (F[i] < F[k]) k = i;
  FahsPush(FAHS, k, F[k]);

#ifdef VERBOSE
  fprintf(stderr, "init terminee\n");
#endif

  /* ================================================ */
  /* APPEL FONCTION RECURSIVE flood                   */
  /* ================================================ */
  if (ds == 1) {
    if ((connex == 4) || (connex == 8))
      (void)flood(F[k], FAHS, STATUS, number_nodes, node_at_level, TREE, incr_vois, rs, N, F); 
    else if ((connex == 0) || (connex == 1))
      (void)floodb(F[k], FAHS, STATUS, number_nodes, node_at_level, TREE, connex, rs, N, F); 
  } else
    (void)flood3d(F[k], FAHS, STATUS, number_nodes, node_at_level, TREE, connex, rs, ps, N, F);

#ifdef VERBOSE
fprintf(stderr, "flood terminee\n");
#endif
#ifdef DEBUG
  AfficheCompTree(TREE);
#endif

  CTREE = CompTree2CompactTree(TREE, number_nodes);

#ifdef VERBOSE
  fprintf(stderr, "CompTree2CompactTree terminee\n");
#endif
#ifdef DEBUG
  AfficheCompactTree(CTREE);
#endif

  CalculeAttributs(CTREE);

#ifdef VERBOSE
  fprintf(stderr, "CalculeAttributs terminee\n");
#endif

  (void)FiltreHeightRec(CTREE, 0, param);

#ifdef VERBOSE
  fprintf(stderr, "FiltreHeightRec terminee\n");
#endif

#ifdef DEBUG
  AfficheCompactTree(CTREE);
  WriteCompactTree(CTREE, "ctree.graph");
#endif

  ProcessLeafMinsOp(CTREE, 0);

#ifdef VERBOSE
  fprintf(stderr, "ProcessLeafMinsOp terminee\n");
#endif

  RecupereImageFiltree(CTREE, STATUS, rs, N, F);

#ifdef VERBOSE
  fprintf(stderr, "RecupereImageFiltree terminee\n");
#endif

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  FahsTermine(FAHS);
  TermineCompTree(TREE);
  TermineCompactTree(CTREE);
  free(STATUS);
  free(number_nodes);
  free(node_at_level);
  return(1);
} /* lheightopening() */

/* ==================================== */
void lattribheight_inverse(struct xvimage * image)
/* ==================================== */
{
  int32_t i, N = rowsize(image) * colsize(image) * depth(image);
  uint8_t *pt;
  for (pt = UCHARDATA(image), i = 0; i < N; i++, pt++)
    *pt = NDG_MAX - *pt;
} // inverse


/* ==================================== */
int32_t lheightminima(struct xvimage *image, int32_t connex, int32_t param)
/* ==================================== */
{
  int res;
  lattribheight_inverse(image);
  res = lheightmaxima(image, connex, param);
  lattribheight_inverse(image);

  return res;  
} /* lheightminima */


/* ==================================== */
int32_t lheightmaxima(struct xvimage *image, int32_t connex, int32_t param)
/* ==================================== */
{
  register int32_t i, k;         /* index muet */
  int32_t rs = rowsize(image);      /* taille ligne */
  int32_t cs = colsize(image);      /* taille colonne */
  int32_t ds = depth(image);        /* nb plans */
  int32_t ps = rs * cs;             /* taille plan */
  int32_t N = ps * ds;              /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  Fahs * FAHS;                    /* la file d'attente hierarchique */
  int32_t incr_vois;                /* 1 pour la 8-connexite,  2 pour la 4-connexite */
  uint32_t *STATUS;         /* etat d'un pixel - doit etre initialise a NOT_ANALYZED */
                                /* en sortie, contient le numero de la composante de niveau h */
                                /* qui contient le pixel (avec h = valeur du pixel) */
  uint32_t *number_nodes;   /* nombre de composantes par niveau */
  uint8_t *node_at_level; /* tableau de booleens */
  CompTree * TREE;              /* resultat : l'arbre des composantes */
  CompactTree * CTREE;          /* resultat : l'arbre des composantes compacte' */

  switch (connex)
    {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
} /* switch (connex) */

  FAHS = CreeFahsVide(N);

  STATUS = (uint32_t *)malloc(N * sizeof(int32_t));
  if (STATUS == NULL)
  {   fprintf(stderr, "lheightmaxima() : malloc failed for STATUS\n");
      return(0);
  }

  number_nodes = (uint32_t *)calloc(256, sizeof(int32_t));
  if (number_nodes == NULL)
  {   fprintf(stderr, "lheightmaxima() : calloc failed for number_nodes\n");
      return(0);
  }

  node_at_level = (uint8_t *)calloc(256, sizeof(char));
  if (node_at_level == NULL)
  {   fprintf(stderr, "lheightmaxima() : calloc failed for node_at_level\n");
      return(0);
  }
  
  TREE = InitCompTree(N);
  if (TREE == NULL)
  {   fprintf(stderr, "lheightmaxima() : InitCompTree failed\n");
      return(0);
  }

  /* ================================================ */
  /* INITIALISATIONS                                  */
  /* ================================================ */

  for (i = 0; i < N; i++) STATUS[i] = NOT_ANALYZED;
  k = 0;             /* recherche un pixel k de niveau de gris minimal dans l'image */
  for (i = 1; i < N; i++) if (F[i] < F[k]) k = i;
  FahsPush(FAHS, k, F[k]);

#ifdef VERBOSE
  fprintf(stderr, "init terminee\n");
#endif

  /* ================================================ */
  /* APPEL FONCTION RECURSIVE flood                   */
  /* ================================================ */

  if (ds == 1) {
    if ((connex == 4) || (connex == 8))
      (void)flood(F[k], FAHS, STATUS, number_nodes, node_at_level, TREE, incr_vois, rs, N, F); 
    else
      if((connex == 0) || (connex == 1))
	(void)floodb(F[k], FAHS, STATUS, number_nodes, node_at_level, TREE, connex, rs, N, F); 
  }
    else
      (void)flood3d(F[k], FAHS, STATUS, number_nodes, node_at_level, TREE, connex, rs, ps, N, F);

#ifdef VERBOSE
  fprintf(stderr, "flood terminee\n");
#endif
#ifdef DEBUG
  AfficheCompTree(TREE);
#endif

  CTREE = CompTree2CompactTree(TREE, number_nodes);

#ifdef VERBOSE
  fprintf(stderr, "CompTree2CompactTree terminee\n");
#endif
#ifdef DEBUG
  AfficheCompactTree(CTREE);
#endif

  CalculeAttributs(CTREE);

#ifdef VERBOSE
  fprintf(stderr, "CalculeAttributs terminee\n");
#endif

  (void)FiltreHeightRec(CTREE, 0, param);

#ifdef VERBOSE
  fprintf(stderr, "FiltreHeightRec terminee\n");
#endif

#ifdef DEBUG
  AfficheCompactTree(CTREE);
  WriteCompactTree(CTREE, "ctree.graph");
#endif

  RecupereImageFiltree(CTREE, STATUS, rs, N, F);

#ifdef VERBOSE
  fprintf(stderr, "RecupereImageFiltree terminee\n");
#endif

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  FahsTermine(FAHS);
  TermineCompTree(TREE);
  TermineCompactTree(CTREE);
  free(STATUS);
  free(number_nodes);
  free(node_at_level);
  return(1);
} /* lheightmaxima() */

/* ==================================== */
int32_t lheightselnb(struct xvimage *image, int32_t connex, int32_t param, int32_t mode)
/* ==================================== */
/* 
  param: nombre de composantes a trouver
  mode: parametre obsolete
*/
{
  register int32_t i, k;         /* index muet */
  int32_t rs = rowsize(image);      /* taille ligne */
  int32_t cs = colsize(image);      /* taille colonne */
  int32_t ds = depth(image);        /* nb plans */
  int32_t ps = rs * cs;             /* taille plan */
  int32_t N = ps * ds;              /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  Fahs * FAHS;                    /* la file d'attente hierarchique */
  int32_t incr_vois;                /* 1 pour la 8-connexite,  2 pour la 4-connexite */
  uint32_t *STATUS;         /* etat d'un pixel - doit etre initialise a NOT_ANALYZED */
                                /* en sortie, contient le numero de la composante de niveau h */
                                /* qui contient le pixel (avec h = valeur du pixel) */
  uint32_t *number_nodes;   /* nombre de composantes par niveau */
  uint8_t *node_at_level; /* tableau de booleens */
  CompTree * TREE;              /* resultat : l'arbre des composantes */
  CompactTree * cpct;          /* resultat : l'arbre des composantes compacte' */
  int32_t nbcomp, nbfeuilles;
  int32_t *A;                       /* tableau pour le tri des composantes par hauteurs croissantes */

  switch (connex)
    {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
  } /* switch (connex) */

  FAHS = CreeFahsVide(N);

  STATUS = (uint32_t *)malloc(N * sizeof(int32_t));
  if (STATUS == NULL)
  {   fprintf(stderr, "lheightselnb() : malloc failed for STATUS\n");
      return(0);
  }

  number_nodes = (uint32_t *)calloc(256, sizeof(int32_t));
  if (number_nodes == NULL)
  {   fprintf(stderr, "lheightselnb() : calloc failed for number_nodes\n");
      return(0);
  }

  node_at_level = (uint8_t *)calloc(256, sizeof(char));
  if (node_at_level == NULL)
  {   fprintf(stderr, "lheightselnb() : calloc failed for node_at_level\n");
      return(0);
  }
  
  TREE = InitCompTree(N);
  if (TREE == NULL)
  {   fprintf(stderr, "lheightselnb() : InitCompTree failed\n");
      return(0);
  }

  /* ================================================ */
  /* INITIALISATIONS                                  */
  /* ================================================ */

  for (i = 0; i < N; i++) STATUS[i] = NOT_ANALYZED;
  k = 0;             /* recherche un pixel k de niveau de gris minimal dans l'image */
  for (i = 1; i < N; i++) if (F[i] < F[k]) k = i;
  FahsPush(FAHS, k, F[k]);

#ifdef VERBOSE
  fprintf(stderr, "init terminee\n");
#endif

  /* ================================================ */
  /* APPEL FONCTION RECURSIVE flood                   */
  /* ================================================ */

  if (ds == 1) {
    if ((connex == 4) || (connex == 8))
      (void)flood(F[k], FAHS, STATUS, number_nodes, node_at_level, TREE, incr_vois, rs, N, F); 
    else 
      if ((connex == 0) || (connex == 1))
	(void)floodb(F[k], FAHS, STATUS, number_nodes, node_at_level, TREE, connex, rs, N, F); 
  } else
    (void)flood3d(F[k], FAHS, STATUS, number_nodes, node_at_level, TREE, connex, rs, ps, N, F);

#ifdef VERBOSE
  fprintf(stderr, "flood terminee\n");
#endif
#ifdef DEBUG
  AfficheCompTree(TREE);
#endif

  cpct = CompTree2CompactTree(TREE, number_nodes);

#ifdef VERBOSE
  fprintf(stderr, "CompTree2CompactTree terminee\n");
#endif

  CalculeAttributs(cpct);

#ifdef VERBOSE
  fprintf(stderr, "CalculeAttributs terminee\n");
#endif
#ifdef DEBUG
  AfficheCompactTree(cpct);
#endif

  nbfeuilles = LeafMark(cpct);
  nbcomp = cpct->nbcomp;
  A = (int32_t *)malloc(nbcomp * sizeof(int32_t));
  if (A == NULL)
  {   fprintf(stderr, "lheightselnb() : malloc failed\n");
      return(0);
  }
  for (i = 0; i < nbcomp; i++) A[i] = i;
  i_TriRapideStochastique (A, cpct->height, 0, nbcomp-1);
  i = 0;
  while ((nbfeuilles > param) && (i < nbcomp))
  {
    //printf("i = %d, A[i] = %d, cpct->height[A[i]] = %d, nbf = %d\n", 
    //       i, A[i], cpct->height[A[i]], nbfeuilles);
    if (NbFilsNonFiltres(cpct, A[i]) == 0)
    {
      cpct->flags[A[i]] |= FILTERED_OUT;
      cpct->flags[A[i]] &= ~LEAF;
      k = cpct->pere[A[i]];
      if (NbFilsNonFiltres(cpct, k) != 0) 
        nbfeuilles--;
      else
        cpct->flags[k] |= LEAF;
    }
#ifdef PARANO
    else
      printf("Erreur imprevue : Composante non feuille : %d\n", A[i]);
#endif
    i++;
  } // while ((nbfeuilles > param) && (i < nbcomp))

#ifdef DEBUG
  AfficheCompactTree(cpct);
  WriteCompactTree(cpct, "ctree.graph");
#endif

  RecupereSegmentation(cpct, STATUS, rs, N, F);

#ifdef VERBOSE
  fprintf(stderr, "RecupereSegmentation terminee\n");
#endif

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  FahsTermine(FAHS);
  TermineCompTree(TREE);
  TermineCompactTree(cpct);
  free(STATUS);
  free(number_nodes);
  free(node_at_level);
  free(A);
  return(1);
} /* lheightselnb() */
