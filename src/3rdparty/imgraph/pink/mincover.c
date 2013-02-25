/*
  MinCover algorithm:
  returns a topological cover (a collection of non-disjoint sets) in
  which weights are minimal
  author: Clement Farabet
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
#include <MTree_utils.h>
#include <MSF_utils.h>

#define false 0
#define true 1

// a helper to insert an index into a list
static void insert(list **sl, int index) {
  list *tmp = NULL;
  list *csl = *sl;
  list *elem = (list*) malloc(sizeof(list));
  if(!elem) exit(EXIT_FAILURE);
  elem->index = index;
  while(csl)
    {
      tmp = csl;
      csl = csl->next;
    }
  elem->next = csl;
  if(tmp) tmp->next = elem;
  else *sl = elem;
}

// recursive helper
static void getmins(JCctree *CT, float *weights, long idx, float *min, long *argmin) {
  if (CT->tabnodes[idx].nbsons > 0) {
    JCsoncell *s;
    for (s = CT->tabnodes[idx].sonlist; s != NULL; s = s->next) {
      long sidx = s->son;
      if (weights[sidx] < weights[idx]) {
	min[sidx] = weights[sidx];
	argmin[sidx] = sidx;
      } else {
	min[sidx] = min[idx];
	argmin[sidx] = argmin[idx];
      }
      getmins(CT, weights, sidx, min, argmin);
    }
  }
}

// returns a list of nodes correspondig to the min cover
list * Min_Cover(MergeTree * MT) {
  // vars
  long i;
  mtree *T= MT->tree;
  float *weights = MT->weights;
  JCctree *CT = T->CT;
  int root = CT->root;

  // nb nodes
  long nbnodes = CT->nbnodes;

  // alloc two arrays, to store min/argmin
  float *min = (float *)malloc(sizeof(float)*nbnodes);
  long *argmin = (long *)malloc(sizeof(long)*nbnodes);

  // recursion - init
  min[root] = weights[root];
  argmin[root] = root;

  // recursion
  getmins(CT, weights, root, min, argmin);

  // generate the cover
  char *done = (char *)calloc(sizeof(char),nbnodes);
  list *cut = NULL;
  for (i = 0; i < nbnodes; i++) {
    if (CT->tabnodes[i].nbsons == 0) { // only consider leaves
      long mini = argmin[i]; // for this leave, this is the index of the min node
      if (!done[mini]) {
	insert(&cut, mini); // store the min node ...
	done[mini] = 1; // ... only once
      }
    }
  }

  // cleanup
  free(min);
  free(argmin);
  free(done);

  // return cut
  return cut;
}
