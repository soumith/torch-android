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
  Arbre des composantes (nouvelle version)

  Ref: NC04

  Michel Couprie - septembre 2003
  Michel Couprie - aout 2004 : 3D
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcunionfind.h>
#include <mcimage.h>
#include <jcgraphes.h>
#include <jccomptree.h>
#include <assert.h>
#include <mtrand64.h> // 64-bit random number generator

#define LCA1         0x08
#define MAXTREE
//#define VERBOSE

//#define DEBUG
//#define PARANO

/* ==================================== */
void jccomptree_ComponentTreePrint(JCctree * CT)
/* ==================================== */
{
  int32_t i;
  JCsoncell *s;
  printf("root = %d ;  nbnodes: %d ; nbsoncells: %d\n", CT->root, CT->nbnodes, CT->nbsoncells);
  for (i = 0; i < CT->nbnodes; i++) 
  {
    printf("node: %d ; level %d ; nbsons: %d ; father: %d ; max = %d ; min = %d ; ", 
	   i, CT->tabnodes[i].data, CT->tabnodes[i].nbsons, 
	   CT->tabnodes[i].father,
	   CT->tabnodes[i].max, CT->tabnodes[i].min);
    if (CT->tabnodes[i].nbsons > 0)
    {
      printf("sons: ");
      for (s = CT->tabnodes[i].sonlist; s != NULL; s = s->next)
        printf("%d  ", s->son);
    }
    printf("\n");
  }
} // jccomptree_ComponentTreePrint() 

/* ==================================== */
void ComponentTreeDotty(JCctree * CT)
/* ==================================== */
{
  FILE *fp;
  int32_t i;
  JCsoncell *s;
  fp = fopen("CT.dot", "w");
  fprintf(fp, "digraph G {\n");
  fprintf(fp, "size=\"8,6\"; ratio=fill;\n");
  //printf("root = %d ;  nbnodes: %d ; nbsoncells: %d\n", CT->root, CT->nbnodes, CT->nbsoncells);
  for (i = 0; i < CT->nbnodes; i++) 
  {
    /*printf("node: %d ; level %d ; nbsons: %d ; father: %d ; max = %d ; min = %d ; ", 
            i, CT->tabnodes[i].data, CT->tabnodes[i].nbsons, CT->tabnodes[i].father,
	   CT->tabnodes[i].max, CT->tabnodes[i].min);
    */
    fprintf(fp, "n%d [label = \"max = %d, min = %d, level = %d\"]\n", i,  
	   CT->tabnodes[i].max, CT->tabnodes[i].min,
	   CT->tabnodes[i].data);
    if (CT->tabnodes[i].nbsons > 0)
    {
      //printf("sons: ");
      for (s = CT->tabnodes[i].sonlist; s != NULL; s = s->next) {
	fprintf(fp, "n%d -> n%d;\n", s->son, i); 
        //printf("%d  ", s->son);
      }
    }
    //printf("\n");
  }
  fprintf(fp, "}\n");
  fclose(fp);
} // jccomptree_ComponentTreePrint() 

/* ==================================== */
void mergeTreePrint(mtree * MT)
/* ==================================== */
{
  int32_t i;
  JCsoncell *s;
  JCctree *CT = MT->CT;
  printf("root = %d ;  nbnodes: %d ; nbsoncells: %d\n", CT->root, CT->nbnodes, CT->nbsoncells);
  for (i = 0; i < CT->nbnodes; i++) 
  {
    printf("node: %d ; level %d ; nbsons: %d ; father: %d ; mergeEdge %d ", 
            i, CT->tabnodes[i].data, CT->tabnodes[i].nbsons, CT->tabnodes[i].father, MT->mergeEdge[i]);
    if (CT->tabnodes[i].nbsons > 0)
    {
      printf("sons: ");
      for (s = CT->tabnodes[i].sonlist; s != NULL; s = s->next)
        printf("%d  ", s->son);
    }
    printf("\n");
  }
} // mergeTreePrint()


/* LN code */

/* ==================================== */
int32_t LowComAncSlow(
  JCctree * CT,
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
// from lwshedtopo.c

// Depth-first preprocessing
static JCctree *lca_CT;
static int32_t *lca_nbr;
static int32_t *lca_rep;
static int32_t *lca_Euler;
static int32_t *lca_Represent;
static int32_t *lca_Depth;
static int32_t *lca_Number;
int32_t jccomptree_LCApreprocessDepthFirst(int32_t node, int32_t depth)
{
  int32_t son;
  JCsoncell *sc;
  //  printf("jccomptree_LCApreprocessDepthFirst\n");
  if (lca_CT->tabnodes[node].nbsons > -1) {
    (*lca_nbr)++;
    lca_Euler[*lca_nbr] = node;
    lca_Number[node] = *lca_nbr;
    lca_Depth[node] = depth;
    lca_Represent[*lca_nbr] = node;
    (*lca_rep)++;
    for (sc = lca_CT->tabnodes[node].sonlist; sc != NULL; sc = sc->next)    {
      son = sc->son;
      jccomptree_LCApreprocessDepthFirst(son, depth+1);
      lca_Euler[++(*lca_nbr)] = node;
    }
  }
  return *lca_nbr;
}

int32_t ** jccomptree_LCApreprocess(JCctree *CT, int32_t *Euler, int32_t *Depth, int32_t *Represent, int32_t *Number, int32_t *nbR, int32_t *lognR)
{
  //O(n.log(n)) preprocessing
  int32_t nbr, rep, nbNodes;

  nbr = -1; // Initialization number of euler nodes
  rep = 0;
  lca_CT = CT;
  lca_nbr = &nbr;
  lca_rep = &rep;
  lca_Euler = Euler;
  lca_Represent = Represent;
  lca_Depth = Depth;
  lca_Number = Number;
  nbr = jccomptree_LCApreprocessDepthFirst(CT->root, 0);
  nbNodes = rep;

  // Check that the number of nodes in the tree was correct
  assert((nbr+1) == (2*nbNodes-1));

  int32_t nbRepresent = 2*nbNodes-1;
  int32_t logn = (int32_t)(ceil(log((double)(nbRepresent))/log(2.0)));
  *nbR = nbRepresent;
  *lognR = logn;

  int32_t i,j,k1,k2;
  int32_t *minim = (int32_t *)calloc(logn*nbRepresent, sizeof(int32_t));
  int32_t **Minim = (int32_t **)calloc(logn, sizeof(int32_t*));
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
  //printf("preprocessing nlogn OK\n");
  return Minim;
}

int32_t jccomptree_LowComAncFast(int32_t n1, int32_t n2, int32_t *Euler, int32_t *Number, int32_t *Depth, int32_t **Minim)
#undef F_NAME
#define F_NAME "jccomptree_LowComAncFast"
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

/* =============================================================== */
int32_t i_Partitionner(int32_t *A, int32_t *T, int32_t p, int32_t r)
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
int32_t i_PartitionStochastique(int32_t *A, int32_t *T, int32_t p, int32_t r)
/* =============================================================== */
/*
  partitionne les elements de A entre l'indice p (compris) et l'indice r (compris)
  en deux groupes : les elements k tels que T[A[k]] <= T[A[q]] et les autres, 
  avec q tire au hasard dans [p,r].
*/
{
  int32_t t, q;

  q = p + (genrand64_int64() % (r - p + 1)); /* rand must be 64-bit safe, should be OK now */
  t = A[p];         /* echange A[p] et A[q] */
  A[p] = A[q]; 
  A[q] = t;
  return i_Partitionner(A, T, p, r);
} /* i_PartitionStochastique() */
 
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

int32_t* jcLinSortIncreasing(uint8_t *F, int32_t N)
/* Trie par denombrement */
#undef F_NAME
#define F_NAME "jcLinSortIncreasing"
{
  int32_t i, j, k, H[256];
  int32_t *T = (int32_t *)malloc(N * sizeof(int32_t));
  if (T == NULL){   
    fprintf(stderr, "%s() : malloc failed for T\n", F_NAME);
    return NULL;
  }
  for (i = 0; i < 256; i++) H[i] = 0;   // initialise l'histogramme
  for (i = 0; i < N; i++) if(i < N)  H[F[i]] += 1; // calcule l'histogramme
  j = H[0]; H[0] = 0;                   // calcule l'histogramme cumule
  for (i = 1; i < 256; i++) { k = H[i]; H[i] = j; j += k; }
  for (i = 0; i < N; i++)   
    // tri lineaire
  {
    k = F[i]; j = H[k]; T[j] = i; H[k] += 1; 
  }
  return T;
} 

// Manipulation de component tree et merge tree
/* ==================================== */
mtree * mergeTreeAlloc(int32_t N)
/* ==================================== */
#undef F_NAME
#define F_NAME "mergeTreeAlloc"
{
  mtree *MT;
  MT = (mtree *)malloc(sizeof(mtree));
  MT->CT = componentTreeAlloc(N);
  if( (MT->mergeEdge = (int32_t *)malloc(sizeof(int32_t)*N)) == NULL){
    fprintf(stderr,"%s: erreu de malloc pour mergeEdge\n",F_NAME);
    exit(0);
  }
  return MT;
} // ComponentTreeAlloc()

/* ==================================== */
JCctree * componentTreeAlloc(int32_t N)
/* ==================================== */
#undef F_NAME
#define F_NAME "componentTreeAlloc"
{
  JCctree *CT;
  CT = (JCctree *)malloc(sizeof(JCctree));
  CT->tabnodes = (JCctreenode *)malloc(N * sizeof(JCctreenode));
  CT->tabsoncells = (JCsoncell *)malloc(2*N * sizeof(JCsoncell));
  CT->flags = (uint8_t *)calloc(N, sizeof(char));
  memset(CT->flags, 0, N);
  if ((CT == NULL) || (CT->tabnodes == NULL) || (CT->tabsoncells == NULL))
  { 
    fprintf(stderr, "%s : malloc failed\n", F_NAME);
    return NULL;
  }
  CT->nbnodes = N;
  CT->nbsoncells = 0;
  return CT;
} // componentTreeAlloc()


/* ==================================== */
void componentTreeFree(JCctree * CT)
/* ==================================== */
{
  free(CT->tabnodes);
  free(CT->tabsoncells);
  if(CT->flags != NULL)free(CT->flags);
  free(CT);
} // ComponentTreeFree()

/* ==================================== */
void mergeTreeFree(mtree * MT)
/* ==================================== */
{
  componentTreeFree(MT->CT);
  free(MT->mergeEdge);
  free(MT);
} // ComponentTreeFree()

void calculReversePointer(JCctree *CT, int32_t root)  
{
  JCsoncell *s; 
  for(s = CT->tabnodes[root].sonlist; s != NULL; s = s->next) 
  {
    calculReversePointer(CT, s->son);
    CT->tabnodes[s->son].father = root;
  }
}

//Compute the merge tree of a MST represented by a list of edges and
//an array of corresponding values (Valeur). JCctree is a structure to
//store a tree and STaltitude gives the altitudes of the nodes. As
//well as the Component tree and saliency tree algorithm, this is an
//original contribution which allows to compute a MT (hence, a CT) of
//a watershed using only one union-find
int32_t jcSaliencyTree_b (JCctree ** SaliencyTree, int32_t *MST, int32_t *Valeur, RAG *rag, int32_t *STaltitude)
{
  int32_t i,x1,x2,n1,n2,z,k, nbsoncellsloc;
  JCctree *ST;
  int32_t *clefs; 
  int32_t *STmap;
  JCsoncell * newsoncell1;
  JCsoncell * newsoncell2;
  Tarjan *T;
  int32_t taille = rag->g->nsom;


  if((STmap = (int32_t *)malloc(sizeof(int32_t) *taille)) == NULL){
    fprintf(stderr, "jcSalliancyTree: erreur de malloc\n"); 
  }
  if((clefs = (int32_t*)malloc(sizeof(int32_t) * taille-1)) == NULL){
    fprintf(stderr,"jcSaliencyTree: erreur de malloc\n");
    exit(0);
  }
  for(i = 0; i < taille-1; i++)
    clefs[i] = i; 

  i_TriRapideStochastique(clefs, Valeur, 0, taille-2);

  if( (ST = componentTreeAlloc((2*taille))) == NULL){
    fprintf(stderr, "jcSalliancyTree: erreur de ComponentTreeAlloc\n");
    exit(0);
  }
  ST->nbnodes = taille;
  ST->nbsoncells = 0;
  T = CreeTarjan(taille);
  for(i = 0; i < taille; i++)
  { 
    STmap[i] = i;
    ST->tabnodes[i].data = 0;
    ST->tabnodes[i].nbsons = 0;
    ST->tabnodes[i].sonlist = NULL;
    TarjanMakeSet(T,i);
  }  

  /*for(i = 0; i < taille-1; i++){ 
  printf("\n MST %d \n",MST[clefs[i]]);
  
  }*/
  //printf("Nb of nodes %d \n",ST->nbnodes);

  for(i = 0; i < taille-1; i++){ 
    // for each edge of the MST taken in increasing order of altitude
    n1 = TarjanFind(T, rag->tete[MST[clefs[i]]]);  n2 = TarjanFind(T,rag->queue[MST[clefs[i]]]);
    // Remark that n1 != n2 since we consider a the edges of a tree
    
    // Which component of ST n1 and n2 belongs to ?
    x1 = STmap[n1]; x2 = STmap[n2];    
    // Create a new component
    z = ST->nbnodes; ST->nbnodes++; 
    nbsoncellsloc = ST->nbsoncells;
    ST->tabnodes[z].nbsons = 2;
    // the altitude of the new component is the altitude of the edge
    // under consideration
    STaltitude[z] = Valeur[clefs[i]];
    // add x1 and x2 to the lists of sons of the new component
    newsoncell1 = &(ST->tabsoncells[nbsoncellsloc]);  
    newsoncell2 = &(ST->tabsoncells[nbsoncellsloc+1]);
    ST->tabnodes[z].sonlist = newsoncell1;
    newsoncell1->son = x1;
    newsoncell1->next = newsoncell2;
    newsoncell2->son = x2;
    newsoncell2->next = NULL;
    ST->tabnodes[z].lastson = newsoncell2;
    ST->nbsoncells += 2;
    
    // then link n1 and n2
    k = TarjanLink(T, n1, n2);
    STmap[k] = z;
  }

  for(i = 0; i < taille; i++)
    STaltitude[i] = 0;

  /* Construct father relationship */
  ST->tabnodes[ST->nbnodes-1].father = -1;
  ST->root = ST->nbnodes - 1;
  calculReversePointer(ST, ST->root); 
  
  /* liberation de la memoire */
  free(STmap); free(clefs);
  (*SaliencyTree) = ST;
  TarjanTermine(T);
  return ST->nbnodes - 1;
} 

/* l'algorithme est le meme que celui du componnent tree */
int32_t mergeTree(RAG *rag, // inputs
		  mtree **MergeTree
		  )
{
  int32_t i,x1,x2,n1,n2,z,k, nbsoncellsloc;
  mtree *MT;
  int32_t *clefs; 
  int32_t *CTmap;
  JCsoncell * newsoncell1;
  JCsoncell * newsoncell2;
  Tarjan *T;
  int32_t nbarcs = rag->g->narc/2;
  int32_t nbsoms = rag->g->nsom;
  JCctree *CT;
  if( (CTmap = (int32_t *)malloc(sizeof(int32_t) *nbsoms)) == NULL){
    fprintf(stderr, "jcSalliancyTree: erreur de malloc\n"); 
  }
  if( (MT = mergeTreeAlloc((2*nbsoms))) == NULL){
    fprintf(stderr, "jcSalliancyTree: erreur de ComponentTreeAlloc\n");
    exit(0);
  }
  CT = MT->CT;
  CT->nbnodes = nbsoms;
  CT->nbsoncells = 0;
  T = CreeTarjan(CT->nbnodes);
  for(i = 0; i < CT->nbnodes; i++)
  { 
    CTmap[i] = i;
    CT->tabnodes[i].data = rag->profondeur[i];
    CT->tabnodes[i].nbsons = 0;
    CT->tabnodes[i].sonlist = NULL;
    TarjanMakeSet(T,i);
  }  
  
  clefs = jcLinSortIncreasing(rag->F, nbarcs); 

  for(i = 0; i < nbarcs; i++){  
    n1 = TarjanFind(T, rag->tete[clefs[i]]);  n2 = TarjanFind(T, rag->queue[clefs[i]]);
    if(n1 != n2){
      x1 = CTmap[n1]; x2 = CTmap[n2];
      z = CT->nbnodes; CT->nbnodes++; 
      nbsoncellsloc = CT->nbsoncells;
      CT->tabnodes[z].nbsons = 2;
      MT->mergeEdge[z] = clefs[i];
      newsoncell1 = &(CT->tabsoncells[nbsoncellsloc]);  
      newsoncell2 = &(CT->tabsoncells[nbsoncellsloc+1]);
   
      CT->tabnodes[z].data = (rag->F[clefs[i]]);
      CT->tabnodes[z].sonlist = newsoncell1;
      newsoncell1->son = x1;
      newsoncell1->next = newsoncell2;
      newsoncell2->son = x2;
      newsoncell2->next = NULL;
      CT->tabnodes[z].lastson = newsoncell2;
      CT->nbsoncells += 2;	
      k = TarjanLink(T, n1, n2);
      CTmap[k] = z;
    }
  }
  /* Construction de la relation father */
  CT->tabnodes[CT->nbnodes-1].father = -1;
  CT->root = CT->nbnodes - 1;
  // doit etre remplace par la fonction pour les component trees
  calculReversePointer(CT,CT->root); 
  /* liberation de la memoire */
  free(CTmap); free(clefs);
  *MergeTree = MT;
  TarjanTermine(T);
  return CT->nbnodes - 1;
}
