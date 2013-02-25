
#ifndef _MERGETREESTRUCT_
#define _MERGETREESTRUCT_
typedef struct {
  mtree *tree;
  RAG *rag;
  struct xvimage *labels;
  int32_t *altitudes;
  float *weights;
  int cs;
  int rs;
} MergeTree;
#endif


#ifndef _LISTSTRUCT_
#define _LISTSTRUCT_
typedef struct list
{
  int index;
  struct list *next;
} list ;
#endif

//#include<MTree_utils.h>

int element_link( int x,int y, uint32_t *Rnk, uint32_t *Fth);
int element_find(int x, uint32_t *Fth );
void TriRapideStochastique_dec (float * A, uint32_t *I, long p, long r);
long PartitionStochastique_dec (float *A, uint32_t * I, long p, long r);
int nb_neighbors(int x, JCctree *CT, int nb_leafs);
int neighbor(int x, int k, JCctree *CT, int nb_leafs, int * SeededNodes);

list * MSF_Kruskal(MergeTree * MT);
list * MSF_Prim(MergeTree * MT);

list * Min_Cover(MergeTree * MT);

