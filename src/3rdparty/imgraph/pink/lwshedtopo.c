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
  Ligne de partage des eaux topologique (nouvelle version)

  Ref: CBN04

  Michel Couprie - septembre 2003
  Laurent Najman - mai 2005
  
Updates: 
  MC - cor. bug mem. alloc. - 12 juillet 2005
  MC - cor. bug dans Watershed - 24 septembre 2007
*/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <math.h>
#include <mcimage.h>
#include <mccodimage.h>
#include <mcfahsalembier.h>
#include <mccomptree.h>
#include <mcutil.h>
#include <mcindic.h>
#include <lwshedtopo.h>
#include <assert.h>

static void compressTree(ctree *CT, int32_t *CM, int32_t *newCM, int32_t N);
static void reconsTree(ctree *CT, int32_t *CM, int32_t *newCM, int32_t N, uint8_t *G);

#define EN_FAHS     0 
#define WATERSHED  1
#define MASSIF     2
#define MODIFIE    4
#define LCA1         0x08

//#define VERBOSE
//#define _DEBUG_
//#define PARANO

// If you want the first version (slower), 
// uncomment the following line
//#define OLDVERSION
// If you want the first version of lwshedtopobin (slower, BUT WHICH WORKS), 
// uncomment the following line
//#define OLDVERSIONBIN

// If you want to compute slowly the LCA in the watershed, uncomment the following line
//#define LCASLOW

/* ==================================== */
static int32_t TrouveComposantes(int32_t x, uint8_t *F, int32_t rs, int32_t ps, int32_t N, int32_t connex, 
                             int32_t *CM, int32_t *tabcomp)
/* ==================================== */
// variante sans simplification 
// place la plus haute composante (ou l'une des plus hautes) en premier
{
  int32_t k, y, n = 1, maxval = F[x], first = 1;

  switch (connex)
  {
  case 4:
    for (k = 0; k < 8; k += 2) // parcourt les c-voisins y de x d'un niveau > F[x]
    { y = voisin(x, k, rs, N);
      if ((y != -1) && (F[y] > maxval)) maxval = F[y];
    } /* for (k = 0; k < 8; k += 2) */
    if (maxval == F[x]) return 0;
    for (k = 0; k < 8; k += 2) // parcourt les c-voisins y de x d'un niveau > F[x]

    { y = voisin(x, k, rs, N);
      if ((y != -1) && (F[y] > F[x]))
      {	if (first && (F[y] == maxval)) { tabcomp[0] = CM[y]; first = 0; }
	else                           { tabcomp[n] = CM[y]; n++; }
      }
    } break;
  case 8:
    for (k = 0; k < 8; k += 1) // parcourt les c-voisins y de x d'un niveau > F[x]
    { y = voisin(x, k, rs, N);
      if ((y != -1) && (F[y] > maxval)) maxval = F[y];
    } /* for (k = 0; k < 8; k += 1) */
    if (maxval == F[x]) return 0;
    for (k = 0; k < 8; k += 1) // parcourt les c-voisins y de x d'un niveau > F[x]
    { y = voisin(x, k, rs, N);
      if ((y != -1) && (F[y] > F[x]))
      {	if (first && (F[y] == maxval)) { tabcomp[0] = CM[y]; first = 0; }
	else                           { tabcomp[n] = CM[y]; n++; }
      }
    } break;
  case 6:
    for (k = 0; k <= 10; k += 2) // parcourt les c-voisins y de x d'un niveau > F[x]
    { y = voisin6(x, k, rs, ps, N);
      if ((y != -1) && (F[y] > maxval)) maxval = F[y];
    }
    if (maxval == F[x]) return 0;
    for (k = 0; k <= 10; k += 2) // parcourt les c-voisins y de x d'un niveau > F[x]
    { y = voisin6(x, k, rs, ps, N);
      if ((y != -1) && (F[y] > F[x]))
      {	if (first && (F[y] == maxval)) { tabcomp[0] = CM[y]; first = 0; }
	else                           { tabcomp[n] = CM[y]; n++; }
      }
    } break;
  case 18:
    for (k = 0; k < 18; k += 1) // parcourt les c-voisins y de x d'un niveau > F[x]
    { y = voisin18(x, k, rs, ps, N);
      if ((y != -1) && (F[y] > maxval)) maxval = F[y];
    }
    if (maxval == F[x]) return 0;
    for (k = 0; k < 18; k += 1) // parcourt les c-voisins y de x d'un niveau > F[x]
    { y = voisin18(x, k, rs, ps, N);
      if ((y != -1) && (F[y] > F[x]))
      {	if (first && (F[y] == maxval)) { tabcomp[0] = CM[y]; first = 0; }
	else                           { tabcomp[n] = CM[y]; n++; }
      }
    } break;
  case 26:
    for (k = 0; k < 26; k += 1) // parcourt les c-voisins y de x d'un niveau > F[x]
    { y = voisin26(x, k, rs, ps, N);
      if ((y != -1) && (F[y] > maxval)) maxval = F[y];
    }
    if (maxval == F[x]) return 0;
    for (k = 0; k < 26; k += 1) // parcourt les c-voisins y de x d'un niveau > F[x]
    { y = voisin26(x, k, rs, ps, N);
      if ((y != -1) && (F[y] > F[x]))
      {	if (first && (F[y] == maxval)) { tabcomp[0] = CM[y]; first = 0; }
	else                           { tabcomp[n] = CM[y]; n++; }
      }
    } break;
  } // switch (connex)
  return n;
} // TrouveComposantes() 

#ifdef __GNUC__
static int32_t LowComAncSlow(ctree * CT, int32_t c1, int32_t c2) __attribute__ ((unused));
#endif
/* ==================================== */
static int32_t LowComAncSlow(
  ctree * CT,
  int32_t c1,
  int32_t c2)
/* Retourne le plus proche commun ancetre des cellules c1,c2
   Utilise le champ "flags". 
*/
/* ==================================== */
#undef F_NAME
#define F_NAME "LowComAncSlow"
{
  int32_t x, lca = -1;

  x = c1; do
  {
    CT->flags[x] |= LCA1;     /* marque LCA1 tous les ancetres de x */
    x = CT->tabnodes[x].father;
  } while (x != -1);
  
  x = c2; do
  {                           /* remonte les ancetres de x */
    if (CT->flags[x] & LCA1) { lca = x; break; }
    x = CT->tabnodes[x].father;
  } while (x != -1);

  x = c1; do
  {                           /* derniere remontee: demarque */
    CT->flags[x] &= ~LCA1;
    x = CT->tabnodes[x].father;
  }  while (x != -1);
#ifdef PARANO
  if (lca == -1)
  {
    fprintf(stderr, "%s: lca not found\n", F_NAME);
    exit(0);
  }
#endif
  return lca;
} // LowComAncSlow()

/////////////////////////////////////////
// lca : Nearest (Lowest) Common Ancestor
// 
// From: The LCA Problem Revisited 
// M.A. Bender - M. Farach-Colton
//

// Depth-first preprocessing
int32_t LCApreprocessDepthFirst(ctree *CT, int32_t node, int32_t depth, int32_t *nbr, int32_t *rep, int32_t *Euler, int32_t *Represent, int32_t *Depth, int32_t *Number)
{
  int32_t son;
  soncell *sc;

  if (CT->tabnodes[node].nbsons > -1) {
    (*nbr)++;
    Euler[*nbr] = node;
    Number[node] = *nbr;
    Depth[node] = depth;
    Represent[*nbr] = node;
    //Represent[(*rep)++] = *nbr;
    (*rep)++;
    for (sc = CT->tabnodes[node].sonlist; sc != NULL; sc = sc->next)    {
      son = sc->son;
      LCApreprocessDepthFirst(CT, son, depth+1, nbr, rep, Euler, Represent, Depth, Number);
      Euler[++(*nbr)] = node;
    }
  }
  return *nbr;
}

int32_t ** LCApreprocess(ctree *CT,   int32_t *Euler, int32_t *Depth, int32_t *Represent, int32_t *Number, int32_t *nbR, int32_t *lognR)
#undef F_NAME
#define F_NAME "LCApreprocess"
{
  //O(n.log(n)) preprocessing
  int32_t nbr, rep, nbNodes;
  int32_t nbRepresent;
  int32_t logn;
  int32_t i,j,k1,k2;
  int32_t *minim;
  int32_t **Minim;

  nbr = -1; // Initialization number of euler nodes
  rep = 0;


  nbr = LCApreprocessDepthFirst(CT, CT->root, 0, &nbr, &rep, Euler, Represent, Depth, Number);
#ifdef _DEBUG_
  ComponentTreePrint(CT);
#endif
  nbNodes = rep;

  // Check that the number of nodes in the tree was correct
#ifdef _DEBUG_
  printf("rep = %d, nbr = %d, nbnodes = %d, 2*nbnodes = %d\n", rep, nbr, nbNodes, 2*nbNodes);
#endif
  assert((nbr+1) == (2*nbNodes-1));

  nbRepresent = 2*nbNodes-1;
  logn = (int32_t)(ceil(log((double)(nbRepresent))/log(2.0)));
  *nbR = nbRepresent;
  *lognR = logn;

  minim = (int32_t *)calloc(logn*nbRepresent, sizeof(int32_t));
  Minim = (int32_t **)calloc(logn, sizeof(int32_t*));
  if ((minim == NULL) || (Minim == NULL)) {
    fprintf(stderr, "%s : malloc failed\n", F_NAME);
    return NULL;
  }
  Minim[0] = minim;

  for (i=0; i<nbRepresent-1; i++) {
    if (Depth[Euler[i]] < Depth[Euler[i+1]]) {
      Minim[0][i] = i;
    } else {
      Minim[0][i] = i+1;
    }
  }
  Minim[0][nbRepresent-1] = nbRepresent-1;

  for (j=1; j<logn; j++) {
    k1 = 1<<(j-1);
    k2 = k1<<1;
    Minim[j] = &minim[j*nbRepresent];
    for (i=0; i<nbRepresent; i++) {
      if ((i+ k2) >= nbRepresent) {
	Minim[j][i] = nbRepresent-1;
      } else {
	if (Depth[Euler[Minim[j-1][i]]] <= Depth[Euler[Minim[j-1][i+k1]]]) {
	  Minim[j][i] = Minim[j-1][i];
	} else {
	  Minim[j][i] = Minim[j-1][i+k1];
	}
      }
    }
  }
#ifdef _DEBUG_
  for (i=0; i<logn; i++) {
    for (j=0; j<nbRepresent; j++)
      printf("M[%d][%d] = %d - ", i, j, Minim[i][j]);
    printf("\n");
  }
#endif
  return Minim;
}

int32_t LowComAncFast(int32_t n1, int32_t n2, int32_t *Euler, int32_t *Number, int32_t *Depth, int32_t **Minim)
#undef F_NAME
#define F_NAME "LowComAncFast"
{
  int32_t ii, jj, kk, k;

  ii = Number[n1];
  jj = Number[n2];
  if (ii == jj)
    return ii;

  if (ii > jj) {
    kk = jj;
    jj = ii;
    ii = kk;
  }

  k = (int32_t)(log((double)(jj - ii))/log(2.));

  if (Depth[Euler[Minim[k][ii]]] < Depth[Euler[Minim[k][jj-(1<<(k))]]]) {
    return Number[Euler[Minim[k][ii]]];
  } else {
    return Number[Euler[Minim[k][jj-(1<<k)]]];
  }
}

/* ==================================== */
static void W_Constructible(int32_t x, uint8_t *F, int32_t rs, int32_t ps, int32_t N, int32_t connex, 
                           ctree *CT, int32_t *CM, int32_t *tabcomp,
			   int32_t *c, int32_t *lcalevel
#ifndef LCASLOW
			   , int32_t *Euler, int32_t *Represent, int32_t *Depth, int32_t *Number, int32_t **Minim
#endif
			   )
    
/* ==================================== */
// Si x est W-construcible, le couple [c, lcalevel] représente la composante (avec son niveau)
// à laquelle x peut être ajouté.
// Sinon la valeur retournée dans c est -1
{
    int32_t c1, k, ncomp = TrouveComposantes(x, F, rs, ps, N, connex, CM, tabcomp);

    if (ncomp > 0)
    {
      if (ncomp == 1) *c = tabcomp[0];
      else 
      {
        *c = tabcomp[0];
        for (k = 1; k < ncomp; k++)
        {
#ifdef LCASLOW
          c1 = LowComAncSlow(CT, *c, tabcomp[k]);
#else
	  c1 = Represent[LowComAncFast(*c, tabcomp[k], Euler, Number, Depth, Minim)];
#endif
          if (c1 != tabcomp[k]) *c = c1;
        }
      }
      *lcalevel = CT->tabnodes[*c].data;
      if (*lcalevel <= F[x]) *c = -1;
    }
    else *c = -1;
} // W_Constructible()

/* ==================================== */
static void Watershed(struct xvimage *image, int32_t connex,
	      Fahs * FAHS, int32_t *CM, ctree * CT)
/* ==================================== */
// 
// inondation a partir des voisins des maxima, suivant les ndg decroissants
// nouvelle (08/03) caracterisation des points destructibles
// nouvelle (09/03) construction de l'arbre des composantes
// Nouvelle (05/05) lca en temps constant + qq optimisation de l'article JMIV (najmanl)
#undef F_NAME
#define F_NAME "Watershed"
{
  uint8_t *F = UCHARDATA(image);
  int32_t rs = rowsize(image);
  int32_t ps = rs * colsize(image);
  int32_t N = ps * depth(image);
  int32_t i, k, x, y;
  int32_t c;                        /* une composante */
  int32_t tabcomp[26];              /* liste de composantes */
  int32_t nbelev;                   /* nombre d'elevations effectuees */
  int32_t lcalevel;                 /* niveau du lca */
  int32_t incr_vois; 
#ifndef LCASLOW
  int32_t logn, nbRepresent;
  int32_t *Euler, *Depth, *Represent, *Number, **Minim;
#endif
  // INITIALISATIONS
  FahsFlush(FAHS); // Re-initialise la FAHS

#ifndef LCASLOW
  Euler = (int32_t *)calloc(2*CT->nbnodes-1, sizeof(int32_t));
  Represent = (int32_t *)calloc(CT->nbnodes, sizeof(int32_t));
  Depth = (int32_t *)calloc(CT->nbnodes, sizeof(int32_t));
  Number = (int32_t *)calloc(CT->nbnodes, sizeof(int32_t));
  if ((Euler == NULL) || (Represent == NULL) 
      || (Depth == NULL) || (Number == NULL)) {
    fprintf(stderr, "%s : malloc failed\n", F_NAME);
    return;
  }
  
  Minim = LCApreprocess(CT, Euler, Depth, Represent, Number, &nbRepresent, &logn);
#ifdef _DEBUG_
  printf("Comparison Slow/Fast lca\n");
    {
      int32_t i,j, anc;
      int32_t nbErrors = 0;
      for (i=0; i<CT->nbnodes; i++)
	for (j=0; j<CT->nbnodes; j++) {
	  if ((CT->tabnodes[i].nbsons > -1) && (CT->tabnodes[j].nbsons > -1)) {
	    anc = Represent[LowComAncFast(i,j,Euler,Number,Depth,Minim)];
	    if (anc != LowComAncSlow(CT,i,j)) {
	      nbErrors++;
	      printf("Error node lca(%d, %d) = %d ou %d ?\n", i, j, anc, LowComAncSlow(CT,i,j));
	    } else {
	      printf("Ok node lca(%d, %d) = %d\n", i, j, anc);
	    }
	  }
	}
      printf("Nb errors = %d\n",nbErrors);
    }
  fflush(stdout);
#endif // _DEBUG_
#endif //ifndef LCASLOW

  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
  } /* switch (connex) */

  // etiquetage des c-maxima (doit pouvoir se faire au vol lors de la construction de l'arbre)
  for (i = 0; i < N; i++)
  {
    c = CM[i];
    if (CT->tabnodes[c].nbsons == 0) {
      Set(i,MASSIF);
    }
  } // for (i = 0; i < N; i++)

  // empile les points
  for (i = 0; i < N; i++)
  {
#ifdef OLDVERSION   
    // empile tous les points
    Set(i,EN_FAHS);
    FahsPush(FAHS, i, NDG_MAX - F[i]);
#else 
    // empile les points voisins d'un minima
    char flag=0;
    switch (connex)
    {
      case 4: 
      case 8:
	for (k = 0; (k < 8) && (flag == 0); k += incr_vois)
        { y = voisin(i, k, rs, N);
	  if ((y != -1) && (IsSet(y,MASSIF)))
          { flag = 1; } 
	} break; 
      case 6: 
	for (k = 0; k <= 10; k += 2)
	{ y = voisin6(i, k, rs, ps, N);
	  if ((y != -1) && (IsSet(y,MASSIF)))
	  { flag = 1; } 
	} break;
      case 18: 
        for (k = 0; k < 18; k += 1)
        { y = voisin18(i, k, rs, ps, N);
	  if ((y != -1) && (IsSet(y,MASSIF)))
          { flag = 1; } 
	} break;
      case 26: 
	for (k = 0; k < 26; k += 1)
        { y = voisin26(i, k, rs, ps, N);
	  if ((y != -1) && (IsSet(y,MASSIF)))
          { flag = 1; } 
	} break;
    } /* switch (connex) */
    if (flag) { 
      Set(i,EN_FAHS); FahsPush(FAHS, i, NDG_MAX - F[i]); 
    }
#endif
  } // for (i = 0; i < N; i++)


  // BOUCLE PRINCIPALE
  nbelev = 0;
  while (!FahsVide(FAHS))
  {
    x = FahsPop(FAHS);
    UnSet(x,EN_FAHS);
    W_Constructible(x, F, rs, ps, N, connex, CT, CM, tabcomp, &c, &lcalevel
#ifndef LCASLOW
		    , Euler, Represent, Depth, Number, Minim
#endif
		    );
    if (c != -1)
    {
        nbelev++;
        F[x] = lcalevel;      // eleve le niveau du point x
        CM[x] = c;            // maj pointeur image -> composantes 
        Set(x,MODIFIE);
        if (CT->tabnodes[c].nbsons == 0) // feuille
        {
          Set(x,MASSIF);
        } // if feuille
        else
        if (CT->tabnodes[c].nbsons > 1) // noeud
        {
        }
#ifdef PARANO
        else
          printf("%s : ERREUR COMPOSANTE BRANCHE!!!\n", F_NAME);
#endif
        // empile les c-voisins de x non marques MASSIF ni EN_FAHS
	switch (connex)
	{
	case 4: 
	case 8:
	  for (k = 0; k < 8; k += incr_vois)
          { y = voisin(x, k, rs, N);
	    if ((y != -1) && (!IsSet(y,MASSIF)) && (!IsSet(y,EN_FAHS)))
            { Set(y,EN_FAHS); FahsPush(FAHS, y, NDG_MAX - F[y]); }
	  } break; 
	case 6: 
	  for (k = 0; k <= 10; k += 2)
          { y = voisin6(x, k, rs, ps, N);
	    if ((y != -1) && (!IsSet(y,MASSIF)) && (!IsSet(y,EN_FAHS)))
            { Set(y,EN_FAHS); FahsPush(FAHS, y, NDG_MAX - F[y]); }
	  } break;
	case 18: 
	  for (k = 0; k < 18; k += 1)
          { y = voisin18(x, k, rs, ps, N);
	    if ((y != -1) && (!IsSet(y,MASSIF)) && (!IsSet(y,EN_FAHS)))
            { Set(y,EN_FAHS); FahsPush(FAHS, y, NDG_MAX - F[y]); }
	  } break;
	case 26: 
	  for (k = 0; k < 26; k += 1)
          { y = voisin26(x, k, rs, ps, N);
	    if ((y != -1) && (!IsSet(y,MASSIF)) && (!IsSet(y,EN_FAHS)))
            { Set(y,EN_FAHS); FahsPush(FAHS, y, NDG_MAX - F[y]); }
	  } break;
	} /* switch (connex) */
    } // if (c != -1)}
  } // while (!FahsVide(FAHS))
#ifdef VERBOSE
    printf("Nombre d'elevations %d\n", nbelev);
#endif

#ifndef LCASLOW
    free(Euler);
    free(Represent);
    free(Depth);
    free(Number);
    free(Minim[0]);
    free(Minim);
#endif //LCASLOW
} // Watershed()

/* ==================================== */
int32_t lwshedtopo_lwshedtopo(struct xvimage *image, int32_t connex)
/* ==================================== */
/*! \fn int32_t lwshedtopo(struct xvimage *image, int32_t connex)
    \param image (entrée/sortie) : une image 2D ndg
    \param connex (entrée) : 4 ou 8 (2D), 6, 18 ou 26 (3D) 
    \return code erreur : 0 si échec, 1 sinon
    \brief ligne de partage des eaux "topologique" (algo MC, GB, LN)
*/
#undef F_NAME
#define F_NAME "lwshedtopo_lwshedtopo"
{
  register int32_t i;      /* index muet */
  int32_t rs = rowsize(image);      /* taille ligne */
  int32_t cs = colsize(image);      /* taille colonne */
  int32_t ds = depth(image);        /* nb plans */
  int32_t ps = rs * cs;             /* taille plan */
  int32_t N = ps * ds;              /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  Fahs * FAHS;                    /* la file d'attente hierarchique */
  int32_t *CM, *newCM;               /* etat d'un pixel */
  ctree * CT;                   /* resultat : l'arbre des composantes */

  FAHS = CreeFahsVide(N);
  if (FAHS == NULL)
  {   fprintf(stderr, "%s() : CreeFahsVide failed\n", F_NAME);
      return 0;
  }
  if ((connex == 4) || (connex == 8))
  {
    if (!ComponentTree(F, rs, N, connex, &CT, &CM))
    {   fprintf(stderr, "%s() : ComponentTree failed\n", F_NAME);
        return 0;
    }
  }
  else if ((connex == 6) || (connex == 18) || (connex == 26))
  {
    if (!ComponentTree3d(F, rs, ps, N, connex, &CT, &CM))
    {   fprintf(stderr, "%s() : ComponentTree failed\n", F_NAME);
        return 0;
    }
  }
  else
  { fprintf(stderr, "%s() : bad value for connex : %d\n", F_NAME, connex);
    return 0;
  }
#ifndef OLDVERSION
  newCM = (int32_t *)calloc(CT->nbnodes, sizeof(int32_t));
  if (newCM == NULL) {
    fprintf(stderr, "%s : malloc failed\n", F_NAME);
    return 0;
  }
  compressTree(CT, CM, newCM, N);
  free(newCM);
#endif

  IndicsInit(N);
  Watershed(image, connex, FAHS, CM, CT);
  for (i=0; i<N; i++)
    F[i] = CT->tabnodes[CM[i]].data; 

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahsTermine(FAHS);
  ComponentTreeFree(CT);
  free(CM);
  return(1);
} /* lwshedtopo_lwshedtopo() */

/* ==================================== */
static void Reconstruction(struct xvimage *g, struct xvimage *f, int32_t *CM, ctree * CT)
/* ==================================== */
#undef F_NAME
#define F_NAME "Reconstruction"
{
  uint8_t *F = UCHARDATA(f);
  uint8_t *G = UCHARDATA(g);
  int32_t rs = rowsize(f);      /* taille ligne */
  int32_t cs = colsize(f);      /* taille colonne */
  int32_t ds = depth(f);        /* nb plans */
  int32_t ps = rs * cs;         /* taille plan */
  int32_t N = ps * ds;          /* taille image */
  int32_t i, c, d;

  for (i = 0; i < N; i++) if (G[i] >= F[i]) CT->flags[CM[i]] = 1; // marque les feuilles

  for (d = 0; d < CT->nbnodes; d++) 
    if (CT->flags[d] == 1)
    {                                               // pour toutes les feuilles marquees 
      c = CT->tabnodes[d].father;
      while ((c != -1) && (CT->flags[c] == 0))
      {
	CT->flags[c] = 1;                               // marque tous les ancetres de c
	c = CT->tabnodes[c].father;
      } 
    }

  for (i = 0; i < N; i++) // AMELIORATION POSSIBLE !!!
  {
    c = CM[i];
    while ((CT->flags[c] == 0) && (CT->tabnodes[c].father != -1))
    {
      c = CT->tabnodes[c].father;
    }
    G[i] = CT->tabnodes[c].data;
  }  

} // Reconstruction()

/* ==================================== */
int32_t lwshedtopo_lreconsdilat(
        struct xvimage *g,
        struct xvimage *f,
        int32_t connex) 
/* reconstruction de g sous f */
/* g : image marqueur */
/* f : image masque */
/* connex : 4 ou 8 */
/* resultat dans g */
/* ==================================== */
{
#undef F_NAME
#define F_NAME "lwshedtopo_lreconsdilat"
  uint8_t *F = UCHARDATA(f);
  int32_t rs = rowsize(f);      /* taille ligne */
  int32_t cs = colsize(f);      /* taille colonne */
  int32_t ds = depth(f);        /* nb plans */
  int32_t N = rs * cs * ds;
  int32_t *CM;     // component mapping
  ctree * CT;  // component tree

  if ((rowsize(g) != rs) || (colsize(g) != cs) || (depth(g) != ds))
  {
    fprintf(stderr, "%s: incompatible sizes\n", F_NAME);
    return 0;
  }

  ComponentTree(F, rs, N, connex, &CT, &CM);
  Reconstruction(g, f, CM, CT);

  ComponentTreeFree(CT);
  free(CM);
  return(1);
} /* lwshedtopo_lreconsdilat() */

/* ==================================== */
int32_t lreconseros(
        struct xvimage *g,
        struct xvimage *f,
        int32_t connex) 
/* reconstruction de g sur f */
/* g : image marqueur */
/* f : image masque */
/* connex : 4 ou 8 */
/* resultat dans g */
/* ==================================== */
{
#undef F_NAME
#define F_NAME "lreconseros"
  int32_t ret, i;
  int32_t rs = rowsize(f);      /* taille ligne */
  int32_t cs = colsize(f);      /* taille colonne */
  int32_t ds = depth(f);        /* nb plans */
  int32_t N = rs * cs * ds;
  uint8_t *F = UCHARDATA(f);
  uint8_t *G = UCHARDATA(g);
  if ((rowsize(g) != rs) || (colsize(g) != cs) || (depth(g) != ds))
  {
    fprintf(stderr, "%s: incompatible sizes\n", F_NAME);
    return 0;
  }
  for (i = 0; i < N; i++) F[i] = NDG_MAX - F[i];
  for (i = 0; i < N; i++) G[i] = NDG_MAX - G[i];
  ret = lwshedtopo_lreconsdilat(g, f, connex);
  for (i = 0; i < N; i++) G[i] = NDG_MAX - G[i];

  return(ret);
} /* lreconseros() */


static void reconsTree(ctree *CT, int32_t *CM, int32_t *newCM, int32_t N, uint8_t *G)
{
  int32_t d, c, e, i;

#ifdef _DEBUG_
  printf("Reconstruction - Marquage\n");
#endif

  for (d = 0; d < CT->nbnodes; d++) CT->flags[d] = 0; // utile ???
  for (i = 0; i < N; i++) if (G[i]) CT->flags[CM[i]] = 1; // marque les feuilles

  for (d = 0; d < CT->nbnodes; d++) 
    if ((CT->tabnodes[d].nbsons == 0) && (CT->flags[d] == 1))
    {                                               // pour toutes les feuilles marquees 
      c = CT->tabnodes[d].father;
      while ((c != -1) && (CT->flags[c] == 0))
      {
	CT->flags[c] = 1;                               // marque tous les ancetres de c
	c = CT->tabnodes[c].father;
      } 
    }

#ifdef _DEBUG_
  printf("Reconstruction - Elimination arbre\n");
#endif
  // elimine les feuilles non marquees
  for (d = 0; d < CT->nbnodes; d++) {
    if ((CT->tabnodes[d].nbsons == 0) && (CT->flags[d] == 0)) {
      // pour toutes les feuilles non marquees 
      c = CT->tabnodes[d].father;                   // récupère le père marqué
      while (CT->flags[c] == 0)
      {
	c = CT->tabnodes[c].father;
      }
#ifdef PARANO
      if (c < 0) {
	printf("ERROR : Father is lower than 0\n");
      }
#endif
      e=d;
      while ((e != c) && (CT->tabnodes[e].nbsons != -2)) {
	if (CT->tabnodes[e].father == c) { // remove the son which is elevated to the father
	  soncell *sc, *prev = NULL;
	  for (sc = CT->tabnodes[c].sonlist; sc != NULL; sc = sc->next)    {
		if (sc->son == e) {
		  CT->tabnodes[c].nbsons--;
		  if (prev) {
		    prev->next = sc->next;
		  } else {
		    CT->tabnodes[c].sonlist = sc->next;
		  }
		  break;
		}
		prev = sc;
	      }
	}
	// Save the number of the node to which e has been moved
	newCM[e] = c;
	CT->tabnodes[e].nbsons = -2;
	e = CT->tabnodes[e].father; 
      }
    } // if ((CT->tabnodes[d].nbsons == 0) && (CT->flags[d] == 0)) 
  } //  for (d = 0; d < CT->nbnodes; d++) 

  // change the component mapping BUT NOT THE GRAY VALUE of the original image
  // That allows to place the watershed line on the original contour
  // while having done the reconstruction on the component tree only.

  // LNA 16Septembre 2005
  // Well apparently we have to reconstruct the original image
  // It is done that later in the main procedure
  // I do not fully understand why it is the case

  for (i=0; i<N; i++) { 
    if (CT->tabnodes[CM[i]].nbsons == -2) {
      CM[i] = newCM[CM[i]];
      // However, we can build the reconstructed image.
      G[i] = CT->tabnodes[CM[i]].data; 
    } 
  }
}

static void compressTree(ctree *CT, int32_t *CM, int32_t *newCM, int32_t N) 
{
  // Compress the component tree
  // suppress all nodes that have only one son
  int32_t i, d, c, e, f;

  //ComponentTreePrint(CT);
  for (d = 0; d < CT->nbnodes; d++) CT->flags[d] = 0; // utile !!
  for (d = 0; d < CT->nbnodes; d++) {
    if (CT->tabnodes[d].nbsons == 0) { // C'est une feuille
      c = CT->tabnodes[d].father;
      //while ((CT->tabnodes[c].father != -1) && (CT->flags[c]==0)) {
      while ((c!=-1) && (CT->flags[c]==0)) {
	if ((CT->tabnodes[c].nbsons == 1) && (CT->tabnodes[c].father != -1)) {
	  soncell *sc = NULL;
	  f = CT->tabnodes[c].father;
	  e = CT->tabnodes[c].sonlist->son;
	  for (sc = CT->tabnodes[f].sonlist; sc != NULL; sc = sc->next)    
	  {
    	    if (sc->son == c) 
	    {
	      sc->son = e;
	      break;
	    }
	  }
#ifdef PARANO
	  if (sc != NULL)
	    if (sc->son != e) printf("Compress Erreur %d != %d\n", sc->son, e);
#endif
	  CT->tabnodes[e].father = f;
	  CT->tabnodes[c].nbsons = -3;
	  newCM[c] = e;
	  if (CT->tabnodes[c].father == -1)
	    CT->root = e;
	}
	CT->flags[c] = 1;
	c = CT->tabnodes[c].father;
      }
    }
  }

  // Change the component mapping BUT NOT THE GRAY VALUE
  // That allows to place the watershed line on the original contour
  // while having done the reconstruction on the component tree only.
  for (i=0; i<N; i++) { 
    if (CT->tabnodes[CM[i]].nbsons == -3) {
      CM[i] = newCM[CM[i]];
    } 
  }
}

/* ==================================== */
int32_t lwshedtopobin(struct xvimage *image, struct xvimage *marqueur, int32_t connex)
/* ==================================== */
/*! \fn int32_t lwshedtopobin(struct xvimage *image, struct xvimage *marqueur, int32_t connex)
    \param image (entrée/sortie) : une image ndg
    \param marqueur (entrée/sortie) : une image binaire
    \param connex (entrée) : 4 ou 8 (2D), 6, 18 ou 26 (3D)
    \return code erreur : 0 si échec, 1 sinon
    \brief ligne de partage des eaux "topologique" binaire (algo MC, GB, LN)
*/
#undef F_NAME
#define F_NAME "lwshedtopobin"
{
  register int32_t i, x;      /* index muet */
  int32_t rs = rowsize(image);      /* taille ligne */
  int32_t cs = colsize(image);      /* taille colonne */
  int32_t ds = depth(image);        /* nb plans */
  int32_t ps = rs * cs;             /* taille plan */
  int32_t N = ps * ds;              /* taille image */
  uint8_t *F = UCHARDATA(image);
  uint8_t *G = UCHARDATA(marqueur);
  Fahs * FAHS;                    /* la file d'attente hierarchique */
  int32_t *CM, *newCM;              /* etat d'un pixel */
  ctree * CT;                   /* resultat : l'arbre des composantes */

  if ((datatype(image) != VFF_TYP_1_BYTE) || (datatype(marqueur) != VFF_TYP_1_BYTE))
  {
    fprintf(stderr, "%s: image and marker must be both byte\n", F_NAME);
    return 0;
  }

  if ((rowsize(marqueur) != rs) || (colsize(marqueur) != cs) || (depth(marqueur) != ds))
  {
    fprintf(stderr, "%s: incompatible sizes\n", F_NAME);
    return 0;
  }

  FAHS = CreeFahsVide(N);
  if (FAHS == NULL)
  {   fprintf(stderr, "%s() : CreeFahsVide failed\n", F_NAME);
      return 0;
  }

  IndicsInit(N);
  // imposition des maxima
  for (i = 0; i < N; i++) if (G[i]) F[i] = G[i] = NDG_MAX; 

#ifdef _DEBUG_
  printf("Component tree\n");
#endif
  if ((connex == 4) || (connex == 8))
  {
    if (!ComponentTree(F, rs, N, connex, &CT, &CM))
    {   fprintf(stderr, "%s() : ComponentTree failed\n", F_NAME);
        return 0;
    }
  }
  else if ((connex == 6) || (connex == 18) || (connex == 26))
  {
    if (!ComponentTree3d(F, rs, ps, N, connex, &CT, &CM))
    {   fprintf(stderr, "%s() : ComponentTree failed\n", F_NAME);
        return 0;
    }
  }
  else
  { fprintf(stderr, "%s() : bad value for connex : %d\n", F_NAME, connex);
    return 0;
  }

#ifdef OLDVERSIONBIN
#ifdef _DEBUG_
  printf("Reconstruction \n");
#endif
  Reconstruction(marqueur, image, CM, CT);
  ComponentTreeFree(CT); // AMELIORATION POSSIBLE: modification du CT et reutilisation pour la suite
  free(CM);

#ifdef _DEBUG_
  printf("Component tree \n");
#endif
  if ((connex == 4) || (connex == 8))
  {
    if (!ComponentTree(G, rs, N, connex, &CT, &CM))
    {   fprintf(stderr, "%s() : ComponentTree failed\n", F_NAME);
        return 0;
    }
  }
  else
  {
    if (!ComponentTree3d(G, rs, ps, N, connex, &CT, &CM))
    {   fprintf(stderr, "%s() : ComponentTree failed\n", F_NAME);
        return 0;
    }
  }

  Watershed(marqueur, connex, FAHS, CM, CT);
#endif // #ifdef OLDVERSIONBIN

#ifndef OLDVERSIONBIN
#ifdef _DEBUG_
  printf("Reconstruction - par arbre\n");
#endif
  newCM = (int32_t *)calloc(CT->nbnodes, sizeof(int32_t));
  if (newCM == NULL) {
    fprintf(stderr, "%s : malloc failed\n", F_NAME);
    return 0;
  }
  reconsTree(CT, CM, newCM, N, G);
  //writeimage(marqueur, "marqueur");

  compressTree(CT, CM, newCM, N);

  // Reconstruction de l'image
  for (x=0; x<N; x++) {
      F[x] = CT->tabnodes[CM[x]].data;    
  }
  //writeimage(image, "test");

  /* Not useful
  for (d = 0; d < CT->nbnodes; d++) {
    if ((CT->tabnodes[d].nbsons == -2) || (CT->tabnodes[d].nbsons == -3)){
      CT->tabnodes[d].nbsons = -1;
    }
  }
  */

  Watershed(image, connex, FAHS, CM, CT);
#endif // ifndef OLDVERSIONBIN

#ifdef _DEBUG_
  printf("Watershed\n");
#endif
 
#ifdef _DEBUG_
  printf("Binarisation\n");
#endif
  for (i = 0; i < N; i++) 
    if (CT->tabnodes[CM[i]].nbsons == 0) // maximum
      F[i] = NDG_MAX;
    else
      F[i] = NDG_MIN;

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahsTermine(FAHS);
  ComponentTreeFree(CT);
  free(CM);
#ifndef OLDVERSIONBIN
  free(newCM);
#endif
  return(1);
} /* lwshedtopobin() */
