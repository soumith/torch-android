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
  Common code for files: 
    lattribarea.c
    lattribheight.c
    lattribvol.c
    lsegreconsheight.c
*/

//#define DEBUG
//#define VERBOSE

#include <string.h>

#define FILTERED_OUT 0x01
#define LEAFMIN      0x02
#define LEAFMAX      0x04
#define LEAF         (LEAFMIN|LEAFMAX)
#define LCA1         0x08
#define LCA2         0x10
#define LCA          (LCA1|LCA2)

#define NOT_ANALYZED 2000000000
#define IN_THE_QUEUE 2000000001

/*
   principe de l'encodage des composantes: 
   le niveau est code dans les bits 24 a 31
   il reste donc 24 bits pour coder le numero de la composante, 
         24
   soit 2   = 16 megacomposantes par niveau.
*/

#define ENCODE(y,h) (y|(h<<24))
#define DECODENUM(y) (y&0x00ffffff)
#define DECODENIV(y) (y>>24)

/* 
   macros pour l'acces aux donnees de la structure CompactTree
*/

#define NBCOMP(h) ((h==0)?(cpct->hc[0]):(cpct->hc[h]-cpct->hc[h-1]))
#define INDEXCOMP(h,j) ((h==0)?(j):(cpct->hc[h-1]+j))
#define NBFILS(c) ((c==0)?(cpct->dfils[0]):(cpct->dfils[c]-cpct->dfils[c-1]))
#define INDEXFILS(c,j) ((c==0)?(j):(cpct->dfils[c-1]+j))

/* ============================================================================== */
/* 
  Structure de donnees pour la construction de l'arbre des composantes.

  Les sommets de cet arbre sont les composantes des coupes >=, 
  a l'exception de celles qui sont egales a une composante d'un niveau inferieur.
  Il y a donc moins de N sommets (N = nombre de pixels) et de N-1 arcs.

  Une composante (sommet) est representee par deux donnees : 
  son niveau (sur 8 bits) et son numero dans le niveau (sur 24 bits), 
  le tout encode dans un entier 32 bits.

  L'arbre est represente par
  une liste d'arcs (tableaux tete et queue), fleches de la racine vers les feuilles.
*/
/* ============================================================================== */
typedef struct {
  uint32_t nbmaxarcs;          /* nombre maximum d'arcs */
  uint32_t nbarcs;             /* nombre effectif d'arcs */
  uint32_t racine;             /* racine de l'arbre */
  uint32_t *tete;              /* sommets initiaux des arcs de l'arbre */
  uint32_t *queue;             /* sommets terminaux des arcs de l'arbre */  
#ifdef ATTR_SURF
  int32_t surf_racine;                 /* attribut surface pour la racine */
  int32_t *surf;                       /* attributs des sommets terminaux des arcs */
#endif
#ifdef ATTR_PERIM
  int32_t *perim;
#endif 
#ifdef ATTR_HBORD
  int32_t *hbord;
#endif
#ifdef ATTR_VOL
  int32_t *vol;
#endif
} CompTree;
/* ============================================================================== */

/* ============================================================================== */
/* 
  Structure de donnees compacte pour l'arbre des composantes.

  L'arbre est represente par cinq tableaux:
  - le tableau 'hc' qui au niveau de gris h, associe le nombre de composantes de 
    niveau <= h (histogramme cumule). On a donc:
      nombre de composantes de niveau 0 = hc[0]
      nombre de composantes de niveau h > 0 = hc[h] - hc[h-1]
      index derniere composante de niveau h s'il en existe = hc[h] - 1
  - le tableau 'comp' qui, a l'index i d'une composante, associe le code de la 
    composante (represente dans 32 bits comme dans la structure CompTree)
  - le tableau 'pere' qui, a l'index i d'une composante, associe l'index du pere de i
    (sauf pour la racine qui a pour index 0 et a laquelle est associee 0)
  - le tableau 'dfils' qui, a l'index i d'une composante, associe l'index(+1) 
    du dernier fils de i (sauf pour les feuilles)
  - le tableau 'fils' qui contient les listes de fils
*/
/* ============================================================================== */
#define CPCT_ROOT 0
typedef struct {
  uint32_t nbcomp;  /* nombre de composantes */
  uint32_t *comp;   /* tableau des composantes */
  uint32_t *pere;   /* tableau representant la relation 'pere' */  
  uint32_t *dfils;  /* tableau donnant l'index+1 du dernier fils dans le tableau 'fils' */
  uint32_t *fils;   /* tableau representant, avec le precedent, la relation 'fils' */  
  uint32_t *hc;     /* histogramme cumule des composantes */
#ifdef ATTR_SURF
  int32_t *surf;
#endif
#ifdef ATTR_HEIGHT
  int32_t *height;
#endif
#ifdef ATTR_PERIM
  int32_t *perim; 
#endif
#ifdef ATTR_HBORD
  int32_t *hbord;
#endif
#ifdef ATTR_CONTRAST
  double *contrast;           /* attribut flottant */
#endif
#ifdef ATTR_VOL
  int32_t *vol;
#endif
#ifdef ATTR_DYN
  int32_t *dyn;
#endif
  char *flags;          /* 8 booleens pour des usages divers */
} CompactTree;
/* ============================================================================== */

/* ======================================================================== */
/* ======================================================================== */
/* FONCTIONS POUR LE TRI */
/* ======================================================================== */
/* ======================================================================== */

// cle DOUBLE

/* =============================================================== */
static int32_t d_Partitionner(int32_t *A, double *T, int32_t p, int32_t r)
/* =============================================================== */
/*
  partitionne les elements de A entre l'indice p (compris) et l'indice r (compris)
  en deux groupes : les elements q tq T[A[q]] <= T[A[p]] et les autres.
*/
{
  int32_t t;
  double x = T[A[p]];
  int32_t i = p - 1;
  int32_t j = r + 1;
  while (1)
  {
    do j--; while (T[A[j]] > x);
    do i++; while (T[A[i]] < x);
    if (i < j) { t = A[i]; A[i] = A[j]; A[j] = t; }
    else return j;
  } /* while (1) */   
} /* d_Partitionner() */

#ifdef __GNUC__
static int32_t d_PartitionStochastique(int32_t *A, double *T, int32_t p, int32_t r) __attribute__ ((unused));
#endif
/* =============================================================== */
static int32_t d_PartitionStochastique(int32_t *A, double *T, int32_t p, int32_t r)
/* =============================================================== */
/*
  partitionne les elements de A entre l'indice p (compris) et l'indice r (compris)
  en deux groupes : les elements k tels que T[A[k]] <= T[A[q]] et les autres, 
  avec q tire au hasard dans [p,r].
*/
{
  int32_t t, q;

  q = p + (rand() % (r - p + 1));
  t = A[p];         /* echange A[p] et A[q] */
  A[p] = A[q]; 
  A[q] = t;
  return d_Partitionner(A, T, p, r);
} /* d_PartitionStochastique() */

#ifdef __GNUC__
static void d_TriRapideStochastique (int32_t * A, double *T, int32_t p, int32_t r) __attribute__ ((unused));
#endif
/* =============================================================== */
static void d_TriRapideStochastique (int32_t * A, double *T, int32_t p, int32_t r)
/* =============================================================== */
/* 
  trie les valeurs du tableau A de l'indice p (compris) a l'indice r (compris) 
  par ordre croissant 
*/
{
  int32_t q; 
  if (p < r)
  {
    q = d_PartitionStochastique(A, T, p, r);
    d_TriRapideStochastique (A, T, p, q) ;
    d_TriRapideStochastique (A, T, q+1, r) ;
  }
} /* d_TriRapideStochastique() */

// cle INT

/* =============================================================== */
static int32_t i_Partitionner(int32_t *A, int32_t *T, int32_t p, int32_t r)
/* =============================================================== */
/*
  partitionne les elements de A entre l'indice p (compris) et l'indice r (compris)
  en deux groupes : les elements q tq T[A[q]] <= T[A[p]] et les autres.
*/
{
  int32_t t;
  int32_t x = T[A[p]];
  int32_t i = p - 1;
  int32_t j = r + 1;
  while (1)
  {
    do j--; while (T[A[j]] > x);
    do i++; while (T[A[i]] < x);
    if (i < j) { t = A[i]; A[i] = A[j]; A[j] = t; }
    else return j;
  } /* while (1) */   
} /* i_Partitionner() */

/* =============================================================== */
static int32_t i_PartitionStochastique(int32_t *A, int32_t *T, int32_t p, int32_t r)
/* =============================================================== */
/*
  partitionne les elements de A entre l'indice p (compris) et l'indice r (compris)
  en deux groupes : les elements k tels que T[A[k]] <= T[A[q]] et les autres, 
  avec q tire au hasard dans [p,r].
*/
{
  int32_t t, q;

  q = p + (rand() % (r - p + 1));
  t = A[p];         /* echange A[p] et A[q] */
  A[p] = A[q]; 
  A[q] = t;
  return i_Partitionner(A, T, p, r);
} /* i_PartitionStochastique() */

#ifdef __GNUC__
static void i_TriRapideStochastique (int32_t * A, int32_t *T, int32_t p, int32_t r) __attribute__ ((unused));
#endif
/* =============================================================== */
static void i_TriRapideStochastique (int32_t * A, int32_t *T, int32_t p, int32_t r)
/* =============================================================== */
/* 
  trie les valeurs du tableau A de l'indice p (compris) a l'indice r (compris) 
  par ordre croissant 
*/
{
  int32_t q; 
  if (p < r)
  {
    q = i_PartitionStochastique(A, T, p, r);
    i_TriRapideStochastique (A, T, p, q) ;
    i_TriRapideStochastique (A, T, q+1, r) ;
  }
} /* i_TriRapideStochastique() */

/* ======================================================================== */
/* ======================================================================== */
/* CONSTRUCTION ET GESTION DE L'ARBRE DES COMPOSANTES */
/* ======================================================================== */
/* ======================================================================== */

/* ==================================== */
static CompTree * InitCompTree(int32_t nbmaxarcs)
/* ==================================== */
{
  CompTree *ct;
  ct = (CompTree *)malloc(sizeof(CompTree));
  if (ct == NULL) return NULL;
  ct->tete = (uint32_t *)malloc(nbmaxarcs * sizeof(int32_t));
  if (ct->tete == NULL) return NULL;
  ct->queue = (uint32_t *)malloc(nbmaxarcs * sizeof(int32_t));
  if (ct->queue == NULL) return NULL;
#ifdef ATTR_SURF
  ct->surf = (int32_t *)malloc(nbmaxarcs * sizeof(int32_t));
  if (ct->surf == NULL) return NULL;
#endif
#ifdef ATTR_PERIM
  ct->perim = (int32_t *)malloc(nbmaxarcs * sizeof(int32_t));
  if (ct->perim == NULL) return NULL;
#endif
#ifdef ATTR_HBORD
  ct->hbord = (int32_t *)malloc(nbmaxarcs * sizeof(int32_t));
  if (ct->hbord == NULL) return NULL;
#endif
#ifdef ATTR_VOL
  ct->vol = (int32_t *)malloc(nbmaxarcs * sizeof(int32_t));
  if (ct->vol == NULL) return NULL;
#endif
  ct->nbmaxarcs = nbmaxarcs;
  ct->nbarcs = 0;
  ct->racine = CPCT_ROOT;
  return ct;
} /* InitCompTree() */

/* ==================================== */
static void TermineCompTree(CompTree *ct)
/* ==================================== */
{
  free(ct->tete);
  free(ct->queue);
#ifdef ATTR_SURF
  free(ct->surf);
#endif
#ifdef ATTR_PERIM
  free(ct->perim);
#endif
#ifdef ATTR_HBORD
  free(ct->hbord);
#endif
#ifdef ATTR_VOL
  free(ct->vol);
#endif
  free(ct);
} /* TermineCompTree() */

/* ==================================== */
static CompactTree * CompTree2CompactTree(CompTree *ct, uint32_t *number_nodes)
/* ==================================== */
/* ATTENTION EFFET DE BORD : DETRUIT LA RELATION number_nodes 
   (number_nodes represente le nombre de composantes par niveau, calcule par flood())
*/
#undef F_NAME
#define F_NAME "CompTree2CompactTree"
{
  CompactTree *cpct;
  uint32_t i, n, h, t, th, tn, q, qh, qn;
  uint32_t nbcomp = ct->nbarcs + 1;
  uint32_t *nfils;

#ifdef VERBOSE
  printf("%s: nbcomp = %d\n", F_NAME, nbcomp);
#endif

  cpct = (CompactTree *)malloc(sizeof(CompactTree));
  if (cpct == NULL) return NULL;
  cpct->nbcomp = nbcomp;
  cpct->comp = (uint32_t *)malloc(nbcomp * sizeof(int32_t));
  if (cpct->comp == NULL) return NULL;
  cpct->pere = (uint32_t *)malloc(nbcomp * sizeof(int32_t));
  if (cpct->pere == NULL) return NULL;
  cpct->dfils = (uint32_t *)malloc(nbcomp * sizeof(int32_t));
  if (cpct->dfils == NULL) return NULL;
  cpct->fils = (uint32_t *)malloc(nbcomp * sizeof(int32_t));
  if (cpct->fils == NULL) return NULL;
  cpct->hc = (uint32_t *)malloc(256 * sizeof(int32_t));
  if (cpct->hc == NULL) return NULL;
#ifdef ATTR_SURF
  cpct->surf = (int32_t *)malloc(nbcomp * sizeof(int32_t));
  if (cpct->surf == NULL) return NULL;
#endif
#ifdef ATTR_HEIGHT
  cpct->height = (int32_t *)malloc(nbcomp * sizeof(int32_t));
  if (cpct->height == NULL) return NULL;
#endif
#ifdef ATTR_PERIM
  cpct->perim = (int32_t *)malloc(nbcomp * sizeof(int32_t));
  if (cpct->perim == NULL) return NULL;
#endif
#ifdef ATTR_HBORD
  cpct->hbord = (int32_t *)malloc(nbcomp * sizeof(int32_t));
  if (cpct->hbord == NULL) return NULL;
#endif
#ifdef ATTR_CONTRAST
  cpct->contrast = (double *)malloc(nbcomp * sizeof(double));
  if (cpct->contrast == NULL) return NULL;
#endif
#ifdef ATTR_VOL
  cpct->vol = (int32_t *)malloc(nbcomp * sizeof(int32_t));
  if (cpct->vol == NULL) return NULL;
#endif
#ifdef ATTR_DYN
  cpct->dyn = (int32_t *)malloc(nbcomp * sizeof(int32_t));
  if (cpct->dyn == NULL) return NULL;
#endif
  cpct->flags = (char *)calloc(nbcomp, sizeof(char));
  if (cpct->flags == NULL) return NULL;

  /* calcule l'histogramme cumule hc */
  n = cpct->hc[0] = number_nodes[0]; 
  for (i = 1; i < 256; i++) { n += number_nodes[i]; cpct->hc[i] = n; }

  /* construit le tableau des composantes comp */
  n = 0; h = 0; while (!number_nodes[h]) h++; /* ATTENTION CODE FRAGILE */
  for (i = 0; i < nbcomp; i++)                /* SUPPOSE CORRECTES LES DONNEES D'ENTREE */
  {                                           /* PAS DE VERIFICATION */
    cpct->comp[i] = ENCODE(n,h); 
    number_nodes[h]--; n++;
    if (!number_nodes[h]) { n = 0; while (!number_nodes[h]) h++; }
  } /* for i */
  
  /* construit la relation pere */
  for (i = 0; i < nbcomp-1; i++)
  {
    t = ct->tete[i]; q = ct->queue[i];
    th = DECODENIV(t); tn = DECODENUM(t);
    qh = DECODENIV(q); qn = DECODENUM(q);
    cpct->pere[INDEXCOMP(qh,qn)] = INDEXCOMP(th,tn);
  }
  q = ct->racine; qh = DECODENIV(q); qn = DECODENUM(q);
  cpct->pere[INDEXCOMP(qh,qn)] = INDEXCOMP(qh,qn);

  /* construit la relation dfils et fils */
  nfils = (uint32_t *)calloc(nbcomp, sizeof(int32_t));
  if (nfils == NULL) return NULL;
  for (i = 1; i < nbcomp; i++) nfils[cpct->pere[i]] += 1;
    /* exception : la racine (0) est fille d'elle-meme, cette relation n'est pas comptee */ 
  cpct->dfils[CPCT_ROOT] = nfils[CPCT_ROOT];
  for (i = 1; i < nbcomp; i++) cpct->dfils[i] = cpct->dfils[i - 1] + nfils[i];
  for (i = 1; i < nbcomp; i++)
  {
    t = cpct->pere[i]; /* i est fils de t */
    nfils[t] -= 1;
    cpct->fils[INDEXFILS(t,nfils[t])] = i;
  }
  free(nfils);

  /* transfere les attributs (cas particulier pour la racine) */
  for (i = 0; i < nbcomp-1; i++)
  {
    q = ct->queue[i]; qh = DECODENIV(q); qn = DECODENUM(q);
#ifdef ATTR_SURF
    cpct->surf[INDEXCOMP(qh,qn)] = ct->surf[i];
#endif
#ifdef ATTR_PERIM
    cpct->perim[INDEXCOMP(qh,qn)] = ct->perim[i];
#endif
#ifdef ATTR_HBORD
    cpct->hbord[INDEXCOMP(qh,qn)] = ct->hbord[i];
#endif
#ifdef ATTR_VOL
    cpct->vol[INDEXCOMP(qh,qn)] = ct->vol[i];
#endif
  }
#ifdef ATTR_SURF
  cpct->surf[CPCT_ROOT] = ct->surf_racine;
#endif
#ifdef ATTR_PERIM
  cpct->perim[CPCT_ROOT] = 0;
#endif
#ifdef ATTR_HBORD
  cpct->hbord[CPCT_ROOT] = 0;
#endif
#ifdef ATTR_VOL
  cpct->vol[CPCT_ROOT] = 0;
#endif  
  return cpct;
} /* CompTree2CompactTree() */

#ifdef __GNUC__
static void ReInitFlags(CompactTree * cpct) __attribute__ ((unused));
#endif
/* ==================================== */
static void ReInitFlags(CompactTree * cpct)
/* ==================================== */
{
  uint32_t nbcomp = cpct->nbcomp;
  memset(cpct->flags,0,nbcomp);
} /* ReInitFlags() */

#ifdef ATTR_SURF
/* ==================================== */
static int32_t surfrec(CompactTree * cpct, uint32_t som, int32_t *na1)
/* ==================================== */
/*
   Calcule la surface de chacune des composantes, a partir de 
   l'information stockee dans cpct->surf[i], qui correspond a la 
   difference de surface entre la composante i et ses filles.
   Le resultat est provisoirement stocke dans le tableau na1 (indexe par
   le numero de composante i), pour etre ensuite recopie dans cpct->surf[i].
*/ 
{
  int32_t i, n, j;
  n = NBFILS(som);
  if (n == 0) return na1[som] = cpct->surf[som];
  na1[som] = cpct->surf[som];
  for (i = 0; i < n; i++) 
  {
    j = INDEXFILS(som, i);
    j = cpct->fils[j];
    na1[som] += surfrec(cpct, j, na1);
  }
  return na1[som];
} /* surfrec() */
#endif

#ifdef ATTR_VOL
/* ==================================== */
static int32_t volrec(CompactTree * cpct, uint32_t som, int32_t *na1)
/* ==================================== */
/*
   Attention: pour utiliser cette fonction, il faut avoir 
   prealablement calcule la surface des composantes (cf. surfrec())
   et avoir stocke cette information dans cpct->surf[].

   Calcule le volume de chacune des composantes, a partir de 
   l'information stockee dans cpct->surf[i], qui correspond a la 
   surface de la composante.
   Le resultat est provisoirement stocke dans le tableau na1 (indexe par
   le numero de composante i), pour etre ensuite recopie dans cpct->vol[i].
*/ 
{
  int32_t i, n, j, nb_coupes_eq;
  n = NBFILS(som);
  if (n == 0) return na1[som] = cpct->surf[som];
  na1[som] = cpct->surf[som];
  for (i = 0; i < n; i++) 
  {
    j = INDEXFILS(som, i);
    j = cpct->fils[j];
    nb_coupes_eq = DECODENIV(cpct->comp[j]) - DECODENIV(cpct->comp[som]) - 1;
    na1[som] += volrec(cpct, j, na1) + (cpct->surf[j] * nb_coupes_eq);
  }
  return na1[som];
} /* volrec() */

#ifdef OLD
/* ==================================== */
static int32_t volrec_old(CompactTree * cpct, uint32_t som, int32_t *na1)
/* ==================================== */
{
  int32_t i, n, j;
  n = NBFILS(som);
  if (n == 0) return na1[som] = cpct->surf[som];
  na1[som] = cpct->surf[som];
  for (i = 0; i < n; i++) 
  {
    j = INDEXFILS(som, i);
    j = cpct->fils[j];
    na1[som] += volrec_old(cpct, j, na1);
  }
  return na1[som];
} /* volrec_old() */
#endif
#endif

#ifdef ATTR_HEIGHT
/* ==================================== */
static int32_t heightrec(CompactTree * cpct, uint32_t som, int32_t *na1)
/* ==================================== */
/* retourne le niveau max des descendants de som (ou le niveau de som pour une feuille) */ 
{
  int32_t i, n, j, h;
  n = NBFILS(som);
  if (n == 0) return na1[som] = DECODENIV(cpct->comp[som]);
  na1[som] = 0;
  for (i = 0; i < n; i++) 
  {
    j = INDEXFILS(som, i);
    j = cpct->fils[j];
    h = heightrec(cpct, j, na1);
    na1[som] = mcmax(na1[som], h);
  }
  return na1[som];
} /* heightrec() */
#endif

#ifdef ATTR_PERIM
/* ==================================== */
static int32_t perimrec(CompactTree * cpct, uint32_t som, int32_t *nperim)
/* ==================================== */
{
  int32_t i, n, j;
  n = NBFILS(som);
  if (n == 0) return nperim[som] = cpct->perim[som];
  nperim[som] = cpct->perim[som];
  for (i = 0; i < n; i++) 
  {
    j = INDEXFILS(som, i);
    j = cpct->fils[j];
    nperim[som] += perimrec(cpct, j, nperim);
  }
  return nperim[som];
} /* perimrec() */
#endif

#ifdef ATTR_HBORD
/* ==================================== */
static int32_t hbordrec(CompactTree * cpct, uint32_t som, int32_t *nhbord)
/* ==================================== */
{
  int32_t i, n, j;
  n = NBFILS(som);
  if (n == 0) return nhbord[som] = cpct->hbord[som];
  nhbord[som] = cpct->hbord[som];
  for (i = 0; i < n; i++) 
  {
    j = INDEXFILS(som, i);
    j = cpct->fils[j];
    nhbord[som] += hbordrec(cpct, j, nhbord);
  }
  return nhbord[som];
} /* hbordrec() */
#endif

#ifdef __GNUC__
static void CalculeAttributs(CompactTree * cpct) __attribute__ ((unused));
#endif
/* ==================================== */
static void CalculeAttributs(CompactTree * cpct)
/* ==================================== */
{
  int32_t *na1, *na2;
  uint32_t nbcomp = cpct->nbcomp;

  na1 = (int32_t *)malloc(nbcomp * sizeof(int32_t));
  na2 = (int32_t *)malloc(nbcomp * sizeof(int32_t));
#ifdef ATTR_SURF
  (void)surfrec(cpct, 0, na1);
  {
    int32_t i;
    for (i = 0; i < nbcomp; i++) cpct->surf[i] = na1[i];
  }
#endif
#ifdef ATTR_VOL
  if (cpct->surf == NULL)
  {
    fprintf(stderr, "CalculeAttributs: VOL ne peut etre calcule dans SURF\n");
    exit(0);
  }
  (void)volrec(cpct, 0, na1);
  {
    int32_t i;
    for (i = 0; i < nbcomp; i++) cpct->vol[i] = na1[i];
  }
#endif
#ifdef ATTR_HEIGHT

  (void)heightrec(cpct, 0, na1);
  /* pour la mesure de la hauteur, il faut rajouter la difference de niveau avec le pere */
  {
    int32_t i;
    for (i = 1; i < nbcomp; i++) cpct->height[i] = na1[i] 
	     /* - DECODENIV(cpct->comp[i]) + DECODENIV(cpct->comp[i]) */ /* inutile */
                                               - DECODENIV(cpct->comp[cpct->pere[i]]) - 1;
  }
  cpct->height[0] = NDG_MAX - NDG_MIN;
#endif
#ifdef ATTR_PERIM
  (void)perimrec(cpct, 0, na1);
  {
    int32_t i;
    for (i = 0; i < nbcomp; i++) cpct->perim[i] = na1[i]; 
  }
#endif
#ifdef ATTR_HBORD
  (void)hbordrec(cpct, 0, na2);
  {
    int32_t i;
    for (i = 0; i < nbcomp; i++) cpct->hbord[i] = na2[i];  
  }
#endif
#ifdef ATTR_CONTRAST
  {
    int32_t i;
    for (i = 0; i < nbcomp; i++) cpct->contrast[i] = ((double)(na2[i]))/na1[i];  
  }
#endif
  free(na1);
  free(na2);
} /* CalculeAttributs() */

#ifdef ATTR_HEIGHT
/* ==================================== */
static int32_t FiltreHeightRec(CompactTree * cpct, int32_t som, int32_t h)
/* ==================================== */
/*
  Filtre les sommets de l'arbre selon un critere de hauteur : 

     height(som) >= h

  Un sommet 'som' ne satisfaisant pas le critere est marque FILTERED_OUT.
  La fonction traite recursivement les fils, et retourne le nombre NNM de sommets non marques dans
    la descendance (inclus le sommet lui-meme).
  Un sommet non filtre et dont le NNM de la descendance vaut 0 est marque LEAFMIN. 
*/
{
  int32_t i, n, j, NNM = 0;
  n = NBFILS(som);
  if (cpct->height[som] < h) cpct->flags[som] |= FILTERED_OUT;
  for (i = 0; i < n; i++) 
  {
    j = INDEXFILS(som, i);
    j = cpct->fils[j];
    NNM += FiltreHeightRec(cpct, j, h);
  }
  if (cpct->height[som] >= h)           /* sommet non filtre */
  {
    if (NNM == 0) cpct->flags[som] |= LEAFMIN;
    NNM++;
  }
  return NNM;
} /* FiltreHeightRec() */
#endif

#ifdef ATTR_SURF

#ifdef __GNUC__
static int32_t FiltreSurfRec(CompactTree * cpct, int32_t som, int32_t h) __attribute__ ((unused));
#endif
/* ==================================== */
static int32_t FiltreSurfRec(CompactTree * cpct, int32_t som, int32_t h)
/* ==================================== */
/*
  Filtre les sommets de l'arbre selon un critere de surface : 

     surf(som) >= h

  Un sommet 'som' ne satisfaisant pas le critere est marque FILTERED_OUT.
  La fonction traite recursivement les fils, et retourne le nombre NNM de sommets non marques dans
    la descendance (inclus le sommet lui-meme).
  Un sommet non filtre et dont le NNM de la descendance vaut 0 est marque LEAFMIN. 
*/
{
  int32_t i, n, j, NNM = 0;
  n = NBFILS(som);
  if (cpct->surf[som] < h) cpct->flags[som] |= FILTERED_OUT;
  for (i = 0; i < n; i++) 
  {
    j = INDEXFILS(som, i);
    j = cpct->fils[j];
    NNM += FiltreSurfRec(cpct, j, h);
  }
  if (cpct->surf[som] >= h)           /* sommet non filtre */
  {
    if (NNM == 0) cpct->flags[som] |= LEAFMIN;
    NNM++;
  }
  return NNM;
} /* FiltreSurfRec() */
#endif

#ifdef ATTR_VOL
/* ==================================== */
static int32_t FiltreVolRec(CompactTree * cpct, int32_t som, int32_t h)
/* ==================================== */
/*
  Filtre les sommets de l'arbre selon un critere de volume : 

     vol(som) >= h

  Un sommet 'som' ne satisfaisant pas le critere est marque FILTERED_OUT.
  La fonction traite recursivement les fils, et retourne le nombre NNM de sommets non marques dans
    la descendance (inclus le sommet lui-meme).
  Un sommet non filtre et dont le NNM de la descendance vaut 0 est marque LEAFMIN. 
*/
{
  int32_t i, n, j, NNM = 0;
  n = NBFILS(som);
  if (cpct->vol[som] < h) cpct->flags[som] |= FILTERED_OUT;
  for (i = 0; i < n; i++) 
  {
    j = INDEXFILS(som, i);
    j = cpct->fils[j];
    NNM += FiltreVolRec(cpct, j, h);
  }
  if (cpct->vol[som] >= h)           /* sommet non filtre */
  {
    if (NNM == 0) cpct->flags[som] |= LEAFMIN;
    NNM++;
  }
  return NNM;
} /* FiltreVolRec() */
#endif


#ifdef __GNUC__
static int32_t MaximiseSegmentation(CompactTree * cpct, int32_t som) __attribute__ ((unused));
#endif
/* ==================================== */
static int32_t MaximiseSegmentation(CompactTree * cpct, int32_t som)
/* ==================================== */
/*
  
*/
{
  int32_t i, n, j, f, nf, NF = 0;
  if (cpct->flags[som] & FILTERED_OUT) return 0;
  n = NBFILS(som);
  if (n == 0) return 1;
  for (i = 0; i < n; i++) 
  {
    j = INDEXFILS(som, i);
    j = cpct->fils[j];
    if ((nf = MaximiseSegmentation(cpct, j))) { f = j; NF += nf; }
  }
  if (NF == 0) return 1;
  if (NF == 1)
  {
    cpct->flags[f] |= FILTERED_OUT;
    cpct->flags[som] |= LEAFMAX;
    return 1;
  }
  return 1 + NF;
} /*  MaximiseSegmentation() */

#ifdef ATTR_CONTRAST
/* ==================================== */
static void Reconstruction(CompactTree * cpct, int32_t som)
/* ==================================== */
/*
  Recherche a partir de la racine, les sommets marques LEAF.
  A partir de chacun de ces sommets :
    - remonte en suivant les etiquettes LEAFMAX jusqu'a trouver un sommet marque LEAFMIN,
    - stocke dans 'branche' le chemin (liste de sommets), dans 'contrast' l'attribut contrast associe a chaque sommet,
      et dans 'index' les index des sommets (initialises a 0,1,2,3,...)
    - trie le tableau index sur la cle contraste,
    - selectionne un sommet M maximum (si plusieurs, ...),
    - a partir de M, on "redescend" en demarquant les sommets
*/
{
  int32_t i, n, m, j, k, M;
  double contrast[256];
  int32_t branche[256];
  int32_t index[256];

  if (!(cpct->flags[som] & LEAF)) /* remonte l'arbre pour trouver une LEAF */ 
  {
    n = NBFILS(som);
    for (i = 0; i < n; i++) 
    {
      j = INDEXFILS(som, i);
      j = cpct->fils[j];
      Reconstruction(cpct, j);
    }
  }
  else /* on a trouve une LEAF */
  {
    m = 0;  /* m indexe le tableau contrast */
    k = som;
    while (!(cpct->flags[k] & LEAFMIN))
    {
      contrast[m] = cpct->contrast[k];
      branche[m] = k;
      m++;
      n = NBFILS(k); /* on va chercher le fils qui est marque LEAF */
      for (i = 0; i < n; i++) 
      {
        j = INDEXFILS(k, i);
        j = cpct->fils[j];
        if (cpct->flags[j] & LEAF) break;
      }
      k = j;
#ifdef PARANO
      if (i >= n) fprintf(stderr, "Reconstruction : ERREUR INATTENDUE\n");
#endif
    }
    contrast[m] = cpct->contrast[k];
    branche[m] = k;
    m++;    

#ifdef DEBUGRECONS
    printf("Reconstruction Sommet %d\n", som);
    for (i = 0; i < m; i++) printf("%d %d %g\n", i, branche[i], contrast[i]);
#endif

    /* trie le tableau index sur la cle contraste */
    for (i = 0; i < m; i++) index[i] = i;
    d_TriRapideStochastique (index, contrast, 0, m-1);

#ifdef DEBUGRECONS
    printf("Apres tri : \n");
    for (i = 0; i < m; i++) printf("%d %d %g\n", index[i], branche[index[i]], contrast[index[i]]);
#endif

    /* selectionne un sommet M maximum (SI PLUSIEURS, ... CHOIX ARBITRAIRE), */
    M = branche[index[m-1]];

    /* a partir de M, on "redescend" en demarquant les sommets */
    cpct->flags[M] &= ~FILTERED_OUT;
    cpct->flags[M] |= LEAFMAX;
    k = cpct->pere[M];
    while (cpct->flags[k] & LEAFMAX)
    {
      cpct->flags[k] &= ~LEAFMAX;
      cpct->flags[k] &= ~FILTERED_OUT;
      k = cpct->pere[k];
    }
  } /* else (on a trouve une LEAF) */
} /*  Reconstruction() */
#endif

#ifdef __GNUC__
static int32_t NbLeafs(CompactTree * cpct, int32_t som) __attribute__ ((unused));
#endif
/* ==================================== */
static int32_t NbLeafs(CompactTree * cpct, int32_t som)
/* ==================================== */
/*
  Recherche a partir du sommet som, les sommets marques LEAF.
  Retourne le nombre de ces sommets.
*/
{
  int32_t i, j, k, n;

  if (!(cpct->flags[som] & LEAF)) /* remonte l'arbre pour trouver une LEAF */ 
  {
    n = NBFILS(som);
    k = 0; 
    for (i = 0; i < n; i++) 
    {
      j = INDEXFILS(som, i);
      j = cpct->fils[j];
      k += NbLeafs(cpct, j);
    }
    return k;
  }
  else /* (on a trouve une LEAF) */
    return 1;
} /* NbLeafs() */

#ifdef __GNUC__
static void RecupereImageFiltree(CompactTree * cpct, uint32_t *STATUS, int32_t rs, int32_t N, uint8_t *ORI) __attribute__ ((unused));
#endif
/* ==================================== */
static void RecupereImageFiltree(CompactTree * cpct,           
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
    ORI[i] = DECODENIV(cpct->comp[comp]);
  }  
} /* RecupereImageFiltree() */

#ifdef __GNUC__
static void RecupereSegmentation(CompactTree * cpct, uint32_t *STATUS, int32_t rs, int32_t N, uint8_t *ORI) __attribute__ ((unused));
#endif
/* ==================================== */
static void RecupereSegmentation(CompactTree * cpct,           
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
    while (cpct->flags[comp] & FILTERED_OUT) 
    {
#ifdef PARANO
      if (comp == cpct->pere[comp])       
        fprintf(stderr, "RecupereSegmentation: la racine a ete eliminee\n");
#endif
      comp = cpct->pere[comp];
    }
    if (cpct->flags[comp] & LEAF) ORI[i] = NDG_MAX; else ORI[i] = NDG_MIN;
  }  
} /* RecupereSegmentation() */

/* ==================================== */
static void TermineCompactTree(CompactTree *cpct)
/* ==================================== */
{
  free(cpct->comp);
  free(cpct->pere);
  free(cpct->dfils);
  free(cpct->fils);
  free(cpct->hc);
#ifdef ATTR_SURF
  free(cpct->surf);
#endif
#ifdef ATTR_HEIGHT
  free(cpct->height);
#endif
#ifdef ATTR_PERIM
  free(cpct->perim);
#endif
#ifdef ATTR_HBORD
  free(cpct->hbord);
#endif
#ifdef ATTR_CONTRAST
  free(cpct->contrast);
#endif
#ifdef ATTR_VOL
  free(cpct->vol);
#endif
#ifdef ATTR_DYN
  free(cpct->dyn);
#endif

  free(cpct->flags);
  free(cpct);
} /* TermineCompactTree() */

#ifdef DEBUG
/* ==================================== */
static void AfficheCompTree(CompTree *ct)
/* ==================================== */
{
  int32_t i;
  printf("===========================\n");
  printf("nombre max arcs = %d\n", ct->nbmaxarcs);
  printf("nombre arcs = %d\n", ct->nbarcs);
  printf("racine = C%d,%d\n", DECODENIV(ct->racine), DECODENUM(ct->racine));
  printf("===========================\n");
  for (i = 0; i < ct->nbarcs; i++)
    printf("C%d,%d --> C%d,%d\n", 
           DECODENIV(ct->tete[i]), DECODENUM(ct->tete[i]), 
           DECODENIV(ct->queue[i]), DECODENUM(ct->queue[i]));
  printf("===========================\n");
} /* AfficheCompTree() */

/* ==================================== */
static void AfficheCompactTree(CompactTree *cpct)
/* ==================================== */
{
  uint32_t i, j, n, f;
  printf("===========================\n");
  printf("nombre composantes = %d\n", cpct->nbcomp);
  printf("===========================\n");
  for (i = 0; i < cpct->nbcomp; i++)
  {
    printf("comp[%d] = C%d,%d ", i, DECODENIV(cpct->comp[i]), DECODENUM(cpct->comp[i]));
#ifdef ATTR_SURF
    printf("surf = %d  ", cpct->surf[i]);
#endif
#ifdef ATTR_HEIGHT
    printf("height = %d  ", cpct->height[i]);
#endif
#ifdef ATTR_VOL
    printf("vol = %d  ", cpct->vol[i]);
#endif
#ifdef ATTR_PERIM
    printf("perim = %d  ", cpct->perim[i]);
#endif
#ifdef ATTR_HBORD
    printf("hbord = %d  ", cpct->hbord[i]);
#endif
#ifdef ATTR_CONTRAST
    printf("contrast = %g ",cpct->contrast[i]);
#endif
#ifdef ATTR_DYN
    printf("dynamique = %d ",cpct->dyn[i]);
#endif
    printf("fils = [ ");
    n = NBFILS(i);
    for (j = 0; j < n; j++)
    {
      f = INDEXFILS(i,j);
      f = cpct->fils[f];
      printf("C%d,%d ", DECODENIV(cpct->comp[f]), DECODENUM(cpct->comp[f]));
    }
    printf("] ");
    if (cpct->flags[i] & FILTERED_OUT) printf(" - OUT");
    if (cpct->flags[i] & LEAFMIN) printf(" - LEAFMIN");
    if (cpct->flags[i] & LEAFMAX) printf(" - LEAFMAX");
    printf("\n");
  }
  printf("===========================\n");
  for (i = 0; i < cpct->nbcomp; i++)
    printf("pere[%d] = %d\n", i, cpct->pere[i]);
  printf("===========================\n");
} /* AfficheCompactTree() */

/* ==================================== */
static void AfficheImaComp(CompactTree * cpct,           
       uint32_t *STATUS,
       int32_t rs, int32_t N, 
       uint8_t *ORI            /* informations sur l'image originale */
)
/* ==================================== */
{
  int32_t i, h;
  uint32_t c;
  for (i = 0; i < N; i++) 
  {
    h = ORI[i];
    c = STATUS[i];
    printf("C%d,%d\t", h, c);
    if ((i % rs) == (rs - 1)) printf("\n");
  }  
} /* AfficheImaComp() */
#endif

#ifdef __GNUC__
static void WriteCompactTree(CompactTree *cpct, char * filename) __attribute__ ((unused));
#endif
/* ==================================== */
static void WriteCompactTree(CompactTree *cpct, char * filename)
/* ==================================== */
{
  uint32_t i;
  FILE * fd = NULL;
  char buf[256];

  fd = fopen(filename,"w");
  if (!fd)
  {
    fprintf(stderr, "WriteCompactTree: cannot open file: %s\n", filename);
    exit(0);
  }

  fprintf(fd, "%d %d\n", cpct->nbcomp, cpct->nbcomp-1);
  fprintf(fd, "noms sommets\n");
  for (i = 0; i < cpct->nbcomp; i++)
  {
    sprintf(buf, "C%d,%d", DECODENIV(cpct->comp[i]), DECODENUM(cpct->comp[i]));
    fprintf(fd, "%d %s\n", i, buf);
  }
  fprintf(fd, "arcs\n");
  for (i = 1; i < cpct->nbcomp; i++)
    fprintf(fd, "%d %d\n", cpct->pere[i], i);
  fclose(fd);
} /* WriteCompactTree() */

#ifdef __GNUC__
static int32_t LeafCount(CompactTree *cpct) __attribute__ ((unused));
#endif
/* ==================================== */
static int32_t LeafCount(CompactTree *cpct)
/* ==================================== */
{
  uint32_t i, f = 0;
  for (i = 0; i < cpct->nbcomp; i++)
    if ((NBFILS(i)) == 0) f++;
  return f;
} /* LeafCount() */

#ifdef __GNUC__
static int32_t LeafMark(CompactTree *cpct) __attribute__ ((unused));
#endif
/* ==================================== */
static int32_t LeafMark(CompactTree *cpct)
/* ==================================== */
{
  uint32_t i, f = 0;
  for (i = 0; i < cpct->nbcomp; i++)
    if ((NBFILS(i)) == 0) { f++; cpct->flags[i] |= LEAF; }
  return f;
} /* LeafMark() */

#ifdef __GNUC__
static int32_t NbFilsNonFiltres(CompactTree * cpct, int32_t som) __attribute__ ((unused));
#endif
/* ==================================== */
static int32_t NbFilsNonFiltres(CompactTree * cpct, int32_t som)
/* ==================================== */
/*
  retourne le nombre de fils de 'som' non marques FILTERED_OUT.
*/
{
  int32_t i, n, j, NNM = 0;
  n = NBFILS(som);
  if (n == 0) 
    return 0;
  for (i = 0; i < n; i++) 
  {
    j = INDEXFILS(som, i);
    j = cpct->fils[j];
    if (!(cpct->flags[j] & FILTERED_OUT)) NNM++;
    else if (NbFilsNonFiltres(cpct, j)>=1) NNM++;
  }
  return NNM;
} /* NbFilsNonFiltres() */

/* ==================================== */
static void AjouteArc(CompTree *ct, uint32_t i, uint32_t j 
#ifdef ATTR_SURF
               , int32_t surf
#endif
#ifdef ATTR_PERIM
               , int32_t perim
#endif
#ifdef ATTR_HBORD
               , int32_t hbord
#endif
              )
/* ==================================== */
{
  if (ct->nbarcs >= ct->nbmaxarcs)
  {
    fprintf(stderr, "AjouteArc : structure CompTree saturee (%d arcs)\n", ct->nbarcs);
    exit(0);
  }
  ct->tete[ct->nbarcs] = i;
  ct->queue[ct->nbarcs] = j;
#ifdef ATTR_SURF
  ct->surf[ct->nbarcs] = surf;
#endif
#ifdef ATTR_PERIM
  ct->perim[ct->nbarcs] = perim;
#endif
#ifdef ATTR_HBORD
  ct->hbord[ct->nbarcs] = hbord;
#endif
  ct->nbarcs += 1;
} /* AjouteArc() */

#ifdef ATTR_PERIM
/* ==================================== */
static int32_t contrib_perim(int32_t p, uint8_t *ORI, uint32_t *STATUS, int32_t rs, int32_t N, int32_t incr_vois)
/* ==================================== */
{ /* calcule la contribution du point p au perimetre de la composante */
  int32_t q, k, nv = 0;

  for (k = 0; k < 8; k += incr_vois) /* compte le nombre nv de voisins deja traites */
  {
    q = voisin(p, k, rs, N);
    if ((q != -1) && (ORI[q] >= ORI[p]) && (STATUS[q] < NOT_ANALYZED)) nv++;
  } /* for (k = 0; k < 8; k += incr_vois) */

  return 4 - 2 * nv;
}

/* ==================================== */
static int32_t contrib_perimb(int32_t p, uint8_t *ORI, uint32_t *STATUS, int32_t rs, int32_t N, int32_t connex)
/* ==================================== */
{ /* calcule la contribution du point p au perimetre de la composante */
  int32_t q, k, nv = 0;

  for (k = 0; k < 6; k ++) /* compte le nombre nv de voisins deja traites */
  {
    q = voisin6b(p, k, rs, N, connex);
    if ((q != -1) && (ORI[q] >= ORI[p]) && (STATUS[q] < NOT_ANALYZED)) nv++;
  } /* for (k = 0; k < 8; k += incr_vois) */

  return 4 - 2 * nv;
}
#endif

#ifdef ATTR_HBORD
/* ==================================== */
static int32_t contrib_hbord(int32_t p, uint8_t *ORI, uint32_t *STATUS, int32_t rs, int32_t N, int32_t incr_vois)
/* ==================================== */
{ /* calcule la contribution du point p au hbord de la composante */
  int32_t q, k, h = 0;

  for (k = 0; k < 8; k += incr_vois)
  {
    q = voisin(p, k, rs, N);
    if (q != -1) h += (ORI[p] - ORI[q]);
  } /* for (k = 0; k < 8; k += incr_vois) */

  return h;
}

/* ==================================== */
static int32_t contrib_hbordb(int32_t p, uint8_t *ORI, uint32_t *STATUS, int32_t rs, int32_t N, int32_t connex)
/* ==================================== */
{ /* calcule la contribution du point p au hbord de la composante */
  int32_t q, k, h = 0;

  for (k = 0; k < 6; k ++)
  {
    q = voisin6b(p, k, rs, N, connex);
    if (q != -1) h += (ORI[p] - ORI[q]);
  } /* for (k = 0; k < 8; k += incr_vois) */

  return h;
}
#endif


/* ==================================== */
static int32_t flood(int32_t h,                 /* niveau a inonder */
          Fahs *FAHS, 
          uint32_t *STATUS,         /* etat d'un pixel - doit etre initialise a NOT_ANALYZED */
	                                /* en sortie, contient le numero de la comp. de niveau h */
                                        /* qui contient le pixel */
          uint32_t *number_nodes,   /* nombre de composantes par niveau */
          uint8_t *node_at_level, /* tableau de booleens */
          CompTree * tree,              /* l'arbre en construction */
          int32_t incr_vois,                /* = 1 pour la 8-connexite, 
                                           = 2 pour la 4-connexite */ 
          int32_t rs, int32_t N, 
          uint8_t *ORI            /* informations sur l'image originale */
)
/* ==================================== */
{
  int32_t p, q, k, m, i, j;
#ifdef ATTR_SURF
  int32_t surf = 0;
#endif
#ifdef ATTR_PERIM
  int32_t perim = 0;
#endif
#ifdef ATTR_HBORD
  int32_t hbord = 0;
#endif

#ifdef DEBUGFLOOD
  printf("debut flood niveau %d\n", h);
#endif

  node_at_level[h] = 1; /* CORRECTION BUG: LIGNE AJOUTEE LE 02/08/00 */
  while (!FahsVideNiveau(FAHS, h))               /* first step : propagation */
  {                                            /* ======================== */
    p = FahsPopNiveau(FAHS, h);
    STATUS[p] = number_nodes[h];
#ifdef DEBUGFLOOD
    printf("STATUS[p] %d\n", STATUS[p]);
#endif
    for (k = 0; k < 8; k += incr_vois)
    {
      q = voisin(p, k, rs, N);
      if ((q != -1) && (STATUS[q] == NOT_ANALYZED))
      {
        FahsPush(FAHS, q, ORI[q]);
        STATUS[q] = IN_THE_QUEUE;
        node_at_level[ORI[q]] = 1;
        if (ORI[q] > ORI[p])
	{
          m = ORI[q];
          do 
          {
#ifdef PARANO
            if ((m < 0) || (m > 255))
            {
              fprintf(stderr, "flood: mauvais niveau : %d ; ORI[q] = %d\n", m, ORI[q]);
              exit(0);
            }
#endif
            m = flood(m, FAHS, STATUS, number_nodes, node_at_level, tree, incr_vois, rs, N, ORI); 
          } while ((m != h) && (m >= 0));
	} /* if (ORI[q] > ORI[p]) */
      } /* if ((q != -1) && (STATUS[q] == NOT_ANALYZED)) */
    } /* for (k = 0; k < 8; k += incr_vois) */
#ifdef ATTR_SURF
    surf += 1;
#endif
#ifdef ATTR_PERIM
    perim += contrib_perim(p, ORI, STATUS, rs, N, incr_vois);
#endif
#ifdef ATTR_HBORD
    hbord += contrib_hbord(p, ORI, STATUS, rs, N, incr_vois);
#endif
  } /* while (!FahsVideNiveau(FAHS, h)) */
#ifdef DEBUGFLOOD
  printf("retour flood niveau %d\n", h);
#endif
  number_nodes[h] += 1;

  m = h - 1;                                    /* second step : define the father */
  while ((m >= 0) && (!node_at_level[m])) m--;  /* =============================== */
  i = number_nodes[h] - 1;
  if (m >= 0)
  {
    j = number_nodes[m];
#ifdef DEBUGFLOOD
    printf("AjouteArc (%d %d) (%d %d)\n", m, j, h, i);
#endif
    AjouteArc(tree, ENCODE(j,m), ENCODE(i,h)    /* definit Cm,j comme le pere de Ch,i */
#ifdef ATTR_SURF
              , surf
#endif
#ifdef ATTR_PERIM
              , perim
#endif
#ifdef ATTR_HBORD
              , hbord
#endif
             ); 
  } /* if (m >= 0) */
  else
  {
#ifdef DEBUGFLOOD
    printf("DefinitRacine (%d %d)\n", h, i);
#endif
    tree->racine = ENCODE(i,h);                /* Ch,i est racine */
#ifdef ATTR_SURF
    tree->surf_racine = surf;
#endif

  }
  node_at_level[h] = 0;
#ifdef DEBUGFLOOD
  printf("fin flood niveau %d, retourne %d\n", h, m);
#endif
  return m;
} /* flood() */

#ifdef __GNUC__
static int32_t floodb(int32_t h, Fahs *FAHS, uint32_t *STATUS, uint32_t *number_nodes, uint8_t *node_at_level, CompTree * tree, int32_t connex, int32_t rs, int32_t N, uint8_t *ORI) __attribute__ ((unused));
#endif
/* ==================================== */
static int32_t floodb(int32_t h,                 /* niveau a inonder */
          Fahs *FAHS, 
          uint32_t *STATUS,         /* etat d'un pixel - doit etre initialise a NOT_ANALYZED */
	                                /* en sortie, contient le numero de la comp. de niveau h */
                                        /* qui contient le pixel */
          uint32_t *number_nodes,   /* nombre de composantes par niveau */
          uint8_t *node_at_level, /* tableau de booleens */
          CompTree * tree,              /* l'arbre en construction */
          int32_t connex,                   /* = 0 pour première case vide, 
                                           = 1 pour la translation */ 
          int32_t rs, int32_t N, 
          uint8_t *ORI            /* informations sur l'image originale */
)
/* ==================================== */
{
  int32_t p, q, k, m, i, j;
#ifdef ATTR_SURF
  int32_t surf = 0;
#endif
#ifdef ATTR_PERIM
  int32_t perim = 0;
#endif
#ifdef ATTR_HBORD
  int32_t hbord = 0;
#endif

#ifdef DEBUGFLOOD
  printf("debut flood niveau %d\n", h);
#endif

  node_at_level[h] = 1; /* CORRECTION BUG: LIGNE AJOUTEE LE 02/08/00 */
  while (!FahsVideNiveau(FAHS, h))               /* first step : propagation */
  {                                            /* ======================== */
    p = FahsPopNiveau(FAHS, h);
    STATUS[p] = number_nodes[h];
#ifdef DEBUGFLOOD
    printf("STATUS[p] %d\n", STATUS[p]);
#endif
    for (k = 0; k < 6; k ++)
    {
      q = voisin6b(p, k, rs, N, connex);
      if ((q != -1) && (STATUS[q] == NOT_ANALYZED))
      {
        FahsPush(FAHS, q, ORI[q]);
        STATUS[q] = IN_THE_QUEUE;
        node_at_level[ORI[q]] = 1;
        if (ORI[q] > ORI[p])
	{
          m = ORI[q];
          do 
          {
#ifdef PARANO
            if ((m < 0) || (m > 255))
            {
              fprintf(stderr, "flood: mauvais niveau : %d ; ORI[q] = %d\n", m, ORI[q]);
              exit(0);
            }
#endif
            m = floodb(m, FAHS, STATUS, number_nodes, node_at_level, tree, connex, rs, N, ORI); 
          } while ((m != h) && (m >= 0));
	} /* if (ORI[q] > ORI[p]) */
      } /* if ((q != -1) && (STATUS[q] == NOT_ANALYZED)) */
    } /* for (k = 0; k < 6; k++) */
#ifdef ATTR_SURF
    surf += 1;
#endif
#ifdef ATTR_PERIM
    perim += contrib_perimb(p, ORI, STATUS, rs, N, connex);
#endif
#ifdef ATTR_HBORD
    hbord += contrib_hbordb(p, ORI, STATUS, rs, N, connex);
#endif
  } /* while (!FahsVideNiveau(FAHS, h)) */
#ifdef DEBUGFLOOD
  printf("retour flood niveau %d\n", h);
#endif
  number_nodes[h] += 1;

  m = h - 1;                                    /* second step : define the father */
  while ((m >= 0) && (!node_at_level[m])) m--;  /* =============================== */
  i = number_nodes[h] - 1;
  if (m >= 0)
  {
    j = number_nodes[m];
#ifdef DEBUGFLOOD
    printf("AjouteArc (%d %d) (%d %d)\n", m, j, h, i);
#endif
    AjouteArc(tree, ENCODE(j,m), ENCODE(i,h)    /* definit Cm,j comme le pere de Ch,i */
#ifdef ATTR_SURF
              , surf
#endif
#ifdef ATTR_PERIM
              , perim
#endif
#ifdef ATTR_HBORD
              , hbord
#endif
             ); 
  } /* if (m >= 0) */
  else
  {
#ifdef DEBUGFLOOD
    printf("DefinitRacine (%d %d)\n", h, i);
#endif
    tree->racine = ENCODE(i,h);                /* Ch,i est racine */
#ifdef ATTR_SURF
    tree->surf_racine = surf;
#endif

  }
  node_at_level[h] = 0;
#ifdef DEBUGFLOOD
  printf("fin flood niveau %d, retourne %d\n", h, m);
#endif
  return m;
} /* floodb() */

#ifdef __GNUC__
static int32_t flood3d(int32_t h, Fahs *FAHS, uint32_t *STATUS, uint32_t *number_nodes, uint8_t *node_at_level, CompTree * tree, int32_t connex, int32_t rs, int32_t ps, int32_t N, uint8_t *ORI) __attribute__ ((unused));
#endif
/* ==================================== */
static int32_t flood3d(
          int32_t h,                        /* niveau a inonder */
          Fahs *FAHS, 
          uint32_t *STATUS,         /* etat d'un pixel - doit etre initialise a NOT_ANALYZED */
	                                /* en sortie, contient le numero de la comp. de niveau h */
                                        /* qui contient le pixel */
          uint32_t *number_nodes,   /* nombre de composantes par niveau */
          uint8_t *node_at_level, /* tableau de booleens */
          CompTree * tree,              /* l'arbre en construction */
          int32_t connex,
          int32_t rs, int32_t ps, int32_t N, 
          uint8_t *ORI            /* informations sur l'image originale */
)
/* ==================================== */
{
  int32_t p, q, k, m, i, j;
#ifdef ATTR_SURF
  int32_t surf = 0;
#endif
#ifdef ATTR_PERIM
  int32_t perim = 0;
#endif
#ifdef ATTR_HBORD
  int32_t hbord = 0;
#endif

#ifdef DEBUGFLOOD
  printf("debut flood3d niveau %d\n", h);
#endif

  node_at_level[h] = 1; /* CORRECTION BUG: LIGNE AJOUTEE LE 02/08/00 */
  while (!FahsVideNiveau(FAHS, h))               /* first step : propagation */
  {                                            /* ======================== */
    p = FahsPopNiveau(FAHS, h);
    STATUS[p] = number_nodes[h];
#ifdef DEBUGFLOOD
    printf("STATUS[p] %d\n", STATUS[p]);
#endif

    switch (connex)
    {
      case 6:
        for (k = 0; k <= 10; k += 2)   /* parcourt les 6 voisins */
        {
          q = voisin6(p, k, rs, ps, N);
          if ((q != -1) && (STATUS[q] == NOT_ANALYZED))
          {
            FahsPush(FAHS, q, ORI[q]);
            STATUS[q] = IN_THE_QUEUE;
            node_at_level[ORI[q]] = 1;
            if (ORI[q] > ORI[p])
	    {
              m = ORI[q];
              do 
              {
#ifdef PARANO
                if ((m < 0) || (m > 255))
                {
                  fprintf(stderr, "flood3d: mauvais niveau : %d ; ORI[q] = %d\n", m, ORI[q]);
                  exit(0);
                }
#endif
                m = flood3d(m, FAHS, STATUS, number_nodes, node_at_level, tree, connex, rs, ps, N, ORI); 
              } while ((m != h) && (m >= 0));
	    } /* if (ORI[q] > ORI[p]) */
          } /* if ((q != -1) && (STATUS[q] == NOT_ANALYZED)) */
        } /* for (...) */
        break;
      case 18:
        for (k = 0; k < 18; k++)   /* parcourt les 18 voisins */
        {
          q = voisin18(p, k, rs, ps, N);
          if ((q != -1) && (STATUS[q] == NOT_ANALYZED))
          {
            FahsPush(FAHS, q, ORI[q]);
            STATUS[q] = IN_THE_QUEUE;
            node_at_level[ORI[q]] = 1;
            if (ORI[q] > ORI[p])
	    {
              m = ORI[q];
              do 
              {
#ifdef PARANO
                if ((m < 0) || (m > 255))
                {
                  fprintf(stderr, "flood3d: mauvais niveau : %d ; ORI[q] = %d\n", m, ORI[q]);
                  exit(0);
                }
#endif
                m = flood3d(m, FAHS, STATUS, number_nodes, node_at_level, tree, connex, rs, ps, N, ORI); 
              } while ((m != h) && (m >= 0));
	    } /* if (ORI[q] > ORI[p]) */
          } /* if ((q != -1) && (STATUS[q] == NOT_ANALYZED)) */
        } /* for (...) */
        break;
      case 26:
        for (k = 0; k < 26; k++)   /* parcourt les 26 voisins */
        {
          q = voisin26(p, k, rs, ps, N);
          if ((q != -1) && (STATUS[q] == NOT_ANALYZED))
          {
            FahsPush(FAHS, q, ORI[q]);
            STATUS[q] = IN_THE_QUEUE;
            node_at_level[ORI[q]] = 1;
            if (ORI[q] > ORI[p])
	    {
              m = ORI[q];
              do 
              {
#ifdef PARANO
                if ((m < 0) || (m > 255))
                {
                  fprintf(stderr, "flood3d: mauvais niveau : %d ; ORI[q] = %d\n", m, ORI[q]);
                  exit(0);
                }
#endif
                m = flood3d(m, FAHS, STATUS, number_nodes, node_at_level, tree, connex, rs, ps, N, ORI); 
              } while ((m != h) && (m >= 0));
	    } /* if (ORI[q] > ORI[p]) */
          } /* if ((q != -1) && (STATUS[q] == NOT_ANALYZED)) */
        } /* for (...) */
        break;
      default:
        fprintf(stderr, "flood3d: mauvaise connexite: %d\n", connex);
        exit(0);
    } /* switch (connex) */

#ifdef ATTR_SURF
    surf += 1;
#endif
  } /* while (!FahsVideNiveau(FAHS, h)) */
#ifdef DEBUGFLOOD
  printf("retour flood3d niveau %d\n", h);
#endif
  number_nodes[h] += 1;

  m = h - 1;                                    /* second step : define the father */
  while ((m >= 0) && (!node_at_level[m])) m--;  /* =============================== */
  i = number_nodes[h] - 1;
  if (m >= 0)
  {
    j = number_nodes[m];
#ifdef DEBUGFLOOD
    printf("AjouteArc (%d %d) (%d %d)\n", m, j, h, i);
#endif
    AjouteArc(tree, ENCODE(j,m), ENCODE(i,h)    /* definit Cm,j comme le pere de Ch,i */
#ifdef ATTR_SURF
              , surf
#endif
#ifdef ATTR_PERIM
              , perim
#endif
#ifdef ATTR_HBORD
              , hbord
#endif
             ); 
  } /* if (m >= 0) */
  else
  {
#ifdef DEBUGFLOOD
    printf("DefinitRacine (%d %d)\n", h, i);
#endif
    tree->racine = ENCODE(i,h);                /* Ch,i est racine */
#ifdef ATTR_SURF
    tree->surf_racine = surf;
#endif
  }
  node_at_level[h] = 0;
#ifdef DEBUGFLOOD
  printf("fin flood3d niveau %d, retourne %d\n", h, m);
#endif
  return m;
} /* flood3d() */

#ifdef __GNUC__
static int32_t LowestCommonAncestor(CompactTree *, int32_t, int32_t *, uint8_t) __attribute__ ((unused));
#endif
/* ==================================== */
static int32_t LowestCommonAncestor(
  CompactTree * cpct,
  int32_t argc,
  int32_t *argv,
  uint8_t d)
/* Retourne le plus proche commun ancetre des cellules de la liste (argc, argv)
   dont le niveau est > d, ou -1 s'il n'existe pas. 
   Utilise le champ "flags". 
   
*/
/* ==================================== */
#undef F_NAME
#define F_NAME "LowestCommonAncestor"
{
  int32_t x, i, lca, NoComAnc;
  if (argc <= 0) return -1;
  if (argc == 1) return argv[0];

  x = argv[0];      /* index de la premiere cellule */
  do
  {
    cpct->flags[x] |= LCA1;     /* marque LCA1 tous les ancetres de x plus hauts que d */
    x = cpct->pere[x];
  } while ((x != CPCT_ROOT) && (DECODENIV(cpct->comp[x]) > d));
  
  for (i = 1; i < argc; i++)   /* boucle pour les autres cellules */
  {
    x = argv[i];
    NoComAnc = 1;              /* pas d'ancetre commun trouve */
    do
    {                          /* remonte les ancetres de x sans depasser d */
      if (cpct->flags[x] & LCA1)  /* on a un ancetre commun */
      {
        NoComAnc = 0; 
      	cpct->flags[x] |= LCA2;   /* on le marque LCA2 */
      	break;                 /* on arrete la remontee (sort du do while) */
      }
      else
        x = cpct->pere[x];     /* on continue la remontee */
    } while ((x != CPCT_ROOT) && (DECODENIV(cpct->comp[x]) > d));
    if (NoComAnc) break;       /* pas d'AC: on sort aussi du for */
  } /* for (i = 1; i < argc; i++) */

  x = argv[0];        /* index de la premiere cellule */
  do
  {                   /* derniere remontee: demarque et repere le lca */
    if (cpct->flags[x] & LCA2) lca = x;
    cpct->flags[x] &= ~LCA;
    x = cpct->pere[x];
  }  while ((x != CPCT_ROOT) && (DECODENIV(cpct->comp[x]) > d));
  
  for (i = 1; i < argc; i++)   /* boucle pour les autres cellules */
  {
    x = argv[i];
    do
    { cpct->flags[x] &= ~LCA;
      x = cpct->pere[x];     /* on continue la remontee */
    } while ((x != CPCT_ROOT) && (DECODENIV(cpct->comp[x]) > d));
  } /* for (i = 1; i < argc; i++) */

#ifdef DEBUGLCA
printf("%s(", F_NAME);
for (i = 0; i < argc; i++) printf(" %d ", argv[i]);
printf(") -> ");
if (NoComAnc) printf("NIL\n"); else printf("%d\n", lca);
#endif

  if (NoComAnc)                /* pas d'ancetre commun */
    return -1;
  else
    return lca;
} /* LowestCommonAncestor() */

#ifdef __GNUC__
static int32_t LowComAnc(CompactTree *, int32_t, int32_t) __attribute__ ((unused));
#endif
/* ==================================== */
static int32_t LowComAnc(
  CompactTree * cpct,
  int32_t c1,
  int32_t c2)
/* Retourne le plus proche commun ancetre des cellules c1,c2
   Utilise le champ "flags". 
*/
/* ==================================== */
#undef F_NAME
#define F_NAME "LowComAnc"
{
  int32_t x, lca = -1;

  x = c1; do
  {
    cpct->flags[x] |= LCA1;     /* marque LCA1 tous les ancetres de x */
    x = cpct->pere[x];
  } while (x != CPCT_ROOT);
  cpct->flags[x] |= LCA1;       /* marque aussi la racine */

  x = c2; do
  {                          /* remonte les ancetres de x */
    if (cpct->flags[x] & LCA1) { lca = x; break; }
    x = cpct->pere[x];
  } while (x != CPCT_ROOT);
  if ((lca == -1) && (cpct->flags[x] & LCA1)) lca = x;

  x = c1; do
  {                   /* derniere remontee: demarque */
    cpct->flags[x] &= ~LCA1;
    x = cpct->pere[x];
  }  while (x != CPCT_ROOT);
  cpct->flags[x] &= ~LCA1;       /* demarque aussi la racine */
#ifdef PARANO
  if (lca == -1)
  {
    fprintf(stderr, "%s: lca not found\n", F_NAME);
    exit(0);
  }
#endif
  return lca;
} /* LowComAnc() */

#ifdef __GNUC__
static int32_t Ancestor(CompactTree * cpct, int32_t c1, int32_t c2) __attribute__ ((unused));
#endif
/* ==================================== */
static int32_t Ancestor(CompactTree * cpct, int32_t c1, int32_t c2)
/* Teste si la composante c1 est ancetre de la composante c2
*/
/* ==================================== */
{
  do
  {
    if (c1 == c2) return 1;
    c2 = cpct->pere[c2];
  } while (c2 != CPCT_ROOT);
  return 0;
} /* Ancestor() */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
