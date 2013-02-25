/*
  Useful functions for MSF-cut algorithms
  author: Camille Couprie
  21 oct. 2011
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mclifo.h>
#include <mcindic.h>
#include <mcutil.h>
#include <jcgraphes.h>
#include <jccomptree.h>
#include <mtrand64.h>
#include "MSF_utils.h"


/*================================================*/
int nb_neighbors(int x, JCctree *CT, int nb_leafs)
/*================================================*/
// returns the nb of neighbors of a node x in a component tree CT.
{
  int tmp;
  if (x<CT->nbnodes)
    {
      tmp = CT->tabnodes[x].nbsons;
      if (tmp==0) tmp=1;
      return tmp+1;
    }
  else if (x<CT->nbnodes+nb_leafs)
    return 1; //CT->tabnodes[CT->root].nbsons;
  else return 1;
}


/*================================================*/
int neighbor(int x, int k, JCctree *CT, int nb_leafs, int * SeededNodes)
/*================================================*/
{

  JCsoncell *s;int i, tmp;
  if (x<CT->nbnodes)
    {
      tmp = CT->tabnodes[x].nbsons;
      if (tmp==0)
        { if (k==0) return CT->tabnodes[x].father;
          return SeededNodes[x+1];
        }
      else if (k<=tmp)
        {
          if (k==tmp) return CT->tabnodes[x].father;
          s = CT->tabnodes[x].sonlist;
          for (i=0;i<k;i++) s = s->next; // INEFFICACE A REFAIRE
	  //fprintf(stderr," ici ");
          return s->son;
        }
    }
  else if (x<CT->nbnodes+nb_leafs)
    return x-CT->nbnodes;
  else
    {
      return CT->root;
    }
}

/*================================================*/
int element_link( int x,int y, uint32_t *Rnk, uint32_t *Fth)
/*================================================*/
{
  if( Rnk[x] > Rnk[y])
    {
      int t;
      t=x;
      x=y;
      y=t;
    }
  if( Rnk[x] == Rnk[y])
    {
      Rnk[y]=Rnk[y]+1;
    }
  Fth[x] = y;
  return y;
}

/*===============================*/
int element_find(int x, uint32_t *Fth )
/*===============================*/
{
  if (Fth[x] != x)
    Fth[x] = element_find(Fth[x], Fth);
  return Fth[x];
}

/* =============================================================== */
long Partitionner_dec(float *A, uint32_t * I, long p, long r)
/* =============================================================== */
/*
  partitionne les elements de A entre l'indice p (compris) et l'indice r (compris)
  en deux groupes : ceux <= A[p] et les autres.
*/
{
  float t;
  int t1;
  float x = A[p];
  long i = p - 1;
  long j = r + 1;
  while (1)
    {
      do j--; while (A[j] < x);
      do i++; while (A[i] > x);
      if (i < j)
        {
          t = A[i];
          A[i] = A[j];
          A[j] = t;
          t1 = I[i];
          I[i] = I[j];
          I[j] = t1;
        }
      else return j;
    } /* while (1) */
} /* Partitionner() */

/* =============================================================== */
long PartitionStochastique_dec (float *A, uint32_t * I, long p, long r)
/* =============================================================== */
/*
  partitionne les elements de A entre l'indice p (compris) et l'indice r (compris)
  en deux groupes : ceux <= A[q] et les autres, avec q tire au hasard dans [p,r].
*/
{
  float t;
  int t1;
  long q;
/* rand must be 64-bit safe, should be OK now */
   q = p + (genrand64_int64() % (r - p + 1)); 
  // q = p + (rand() % (r - p + 1));
  t = A[p];         /* echange A[p] et A[q] */
  A[p] = A[q];
  A[q] = t;

  t1 = I[p];         /* echange I[p] et I[q] */
  I[p] = I[q];
  I[q] = t1;

  return Partitionner_dec(A, I, p, r);
} /* PartitionStochastique() */

/* =============================================================== */
void TriRapideStochastique_dec (float * A, uint32_t *I, long p, long r)
/* =============================================================== */
/*
  trie les valeurs du tableau A de l'indice p (compris) a l'indice r (compris)
  par ordre decroissant
*/
{
  long q;
  if (p < r)
    {
      q = PartitionStochastique_dec(A, I, p, r);
      TriRapideStochastique_dec (A, I, p, q) ;
      TriRapideStochastique_dec (A, I, q+1, r) ;
    }
} /* TriRapideStochastique() */
