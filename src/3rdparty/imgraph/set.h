#ifndef _SET_
#define _SET_

/*
  This file provides a very simple data structure to represent
  a disjoint set. This code is heavily inspired by Pedro
  Felzenszwalb's min-spanning tree code, released under a 
  GNU GPL license (Copyright (C) 2006 Pedro).
*/

typedef struct {
  int pseudorank;
  int parent;
  int surface;
  int id;
} Elt;

typedef struct {
  Elt *elts;
  int nelts;
} Set;

Set * set_new(int nelts) {
  Set *set = (Set *)calloc(1, sizeof(Set));
  set->elts = (Elt *)calloc(nelts, sizeof(Elt));
  set->nelts = nelts;
  int i;
  for (i = 0; i < nelts; i++) {
    set->elts[i].pseudorank = 0;
    set->elts[i].surface = 1;
    set->elts[i].parent = i;
    set->elts[i].id = -1;
  }
  return set;
}

void set_free(Set *set) {
  free(set->elts);
  free(set);
}

int set_find(Set *set, int x) {
  int y = x;
  while (y != set->elts[y].parent)
    y = set->elts[y].parent;
  set->elts[x].parent = y;
  return y;
}

void set_join(Set *set, int x, int y) {
  if (set->elts[x].pseudorank > set->elts[y].pseudorank) {
    set->elts[y].parent = x;
    set->elts[x].surface += set->elts[y].surface;
  } else {
    set->elts[x].parent = y;
    set->elts[y].surface += set->elts[x].surface;
    if (set->elts[x].pseudorank == set->elts[y].pseudorank)
      set->elts[y].pseudorank++;
  }
  set->nelts--;
}

#endif
