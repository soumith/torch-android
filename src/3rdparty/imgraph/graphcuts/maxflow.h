
#include <MTree_utils.h>

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

extern list * Graph_Cuts(MergeTree * );
