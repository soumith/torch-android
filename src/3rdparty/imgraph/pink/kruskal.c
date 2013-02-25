/*
  Kruskal algorithm for Maximum Spanning Forest (MSF) computation
  implemented to compute an MSF cut in a tree (hierarchy)
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
#include <MTree_utils.h>
#include <MSF_utils.h>


#define false 0
#define true 1
void Insert2(list **sl, int index);

/*=====================================================================================*/
list * MSF_Kruskal(MergeTree * MT)
/*=====================================================================================*/
/*Segment a tree into two components.
  Returns a list of nodes correspunding to the Max Spanning Forest cut,
  computed using Kruskal's algorithm */
{
  int i, j, k, x, y, e1, e2;
  int nb_markers; int nb_leafs;
  long N, M;

 // -------- Gathering usefull input graph (MT) informations -----------

  float val=1; //weight parameter for leafs.
  mtree * T= MT->tree;
  float * W = MT->weights;
  //  mergeTreePrint(T);
  JCctree *CT = T->CT;
  int root_node = CT->root;

  //nb nodes
  M = CT->nbnodes;

  // nb_edges
  nb_leafs = 0;
  for (i = 0; i < M; i++)
    if (CT->tabnodes[i].nbsons == 0)
      nb_leafs++;

  nb_markers = nb_leafs+1;
  N=M+nb_markers;
  M=N-1;
  //printf("Nb nodes:%d Nb edges: %d Nb leafs :%d \n", N, M, nb_leafs);

  // indexes of edges : son's nodes indexes
  //Memory allocation of temporary arrays for Krukal's algorithm
  Lifo * LIFO;
  LIFO = CreeLifoVide(M);
  if (LIFO == NULL) { fprintf(stderr, "kruskal : CreeLifoVide failed\n"); exit(0); }

  int * Mrk = (int*)calloc(N ,sizeof(int));
  if (Mrk == NULL) { fprintf(stderr, "kruskal : malloc failed\n"); exit(0); }

  uint32_t * SeededNodes = (uint32_t*)malloc(nb_markers*sizeof(uint32_t));
  if (SeededNodes == NULL) { fprintf(stderr, "kruskal : malloc failed\n"); exit(0); }

  // markers
  SeededNodes[0]= M;
  j=1;
  for (i = 0; i < CT->nbnodes; i++)
    if (CT->tabnodes[i].nbsons == 0)
      {
        SeededNodes[j]= i+CT->nbnodes;
        Mrk[SeededNodes[j]] = 1;
        j++;
      }
  Mrk[M] = 1;

  uint32_t * Rnk = (uint32_t*)calloc(N, sizeof(uint32_t));
  if (Rnk == NULL) { fprintf(stderr, "kruskal : malloc failed\n"); exit(0); }
  uint32_t * Fth = (uint32_t*)malloc(N*sizeof(uint32_t));
  if (Fth == NULL) { fprintf(stderr, "kruskal : malloc failed\n"); exit(0); }
  for(k=0;k<N;k++) { Fth[k]=k; }

  // Es : E sorted by decreasing weights
  uint32_t * Es = (uint32_t*)malloc(M*sizeof(uint32_t));
  if (Es == NULL) { fprintf(stderr, "kruskal : malloc failed\n"); exit(0); }
  for(k=0;k<M;k++) Es[k]=k;

  float * sorted_weights = (float *)malloc(M*sizeof(float));
  for(k=0;k<CT->nbnodes;k++)
    sorted_weights[k]=W[k];

  for(k=0;k<nb_leafs;k++)
    sorted_weights[CT->nbnodes+k]=val;

  TriRapideStochastique_dec(sorted_weights,Es, 0, M-1);
  free(sorted_weights);


  long nb_arete = 0;
  int e_max, root;
  long cpt_aretes = 0;

  // ----------- beginning of main loop of Kruskal's algorithm ----------------
  while (nb_arete < N-nb_markers)
    {
      e_max=Es[cpt_aretes];
      cpt_aretes=cpt_aretes+1;
      e1= e_max;  // e1 = Edges[0][e_max];
      if (e_max<CT->nbnodes) e2= CT->tabnodes[e_max].father;
      else if(e_max!=M) e2= e_max-CT->nbnodes;
      else e2=root_node;
      if (e2==-1)e2=M;  //e2 = Edges[1][e_max];
      //printf("(%d %d)\n", e1,e2);
      x = element_find(e1, Fth );
      y = element_find(e2, Fth );
      if ((x != y) && (!(Mrk[x]>=1 && Mrk[y]>=1)))
        {
          root = element_link( x,y, Rnk, Fth);
          //printf("link\n");
          nb_arete=nb_arete+1;
          if ( Mrk[x]>=1) Mrk[root]= Mrk[x];
          else if ( Mrk[y]>=1) Mrk[root]= Mrk[y];
        }
    }

  //building the labeling for each individual markers in map
  // (find the root vertex of each tree)
  int * Map2 = (int *)malloc(N*sizeof(int));
  int * Map = (int *)malloc(N*sizeof(int));
  for (i=0; i<N; i++)
    Map2[i] = element_find(i, Fth);

  // Compute the binary labeling in Map
  for (i = 1; i < nb_markers; i++)
    Map[SeededNodes[i]] = 1;
  Map[M]=0;


  // ---------Loop to assign the proper label to each tree--------
  for (i=0;i<N;i++) Mrk[i] = false;
  for (i=0;i<nb_markers; i++)
    {
      LifoPush(LIFO, SeededNodes[i]);
      while (!LifoVide(LIFO))
        {
          x = LifoPop(LIFO);
          Mrk[x]=true;
          j= nb_neighbors(x, CT, nb_leafs);
          for (k=0;k<j;k++)
            {
              y = neighbor(x, k, CT, nb_leafs, SeededNodes);
              if (y==-1)y=M;
              if (Map2[y]==Map2[SeededNodes[i]]  && Mrk[y]==false)
                {
                  LifoPush(LIFO, y);
                  if (i==0) Map[y]= 0;
                  else  Map[y]= 1;
                  Mrk[y]=true;
                }
            }
        }
      LifoFlush(LIFO);
    }
  for (i = 1; i < nb_markers; i++)
    Map[SeededNodes[i]] = 1;
  Map[M]=0;

  for (i=0; i<N; i++) {
    //fprintf(stderr,"Map[%d]=%d \n",i,Map[i]);
  }

  // Process the tree to find the cut
  list * cut = NULL;
  for (i = 0; i < CT->nbnodes; i++)
    {
      // nodes having a different value than their father are in the cut
      if ((CT->tabnodes[i].father != -1) && (Map[CT->tabnodes[i].father] != Map[i]))
        Insert2(&cut, i);
      // leafs having the same label as the root are in the cut
      if ((CT->tabnodes[i].nbsons == 0) && (Map[i]==0))
        Insert2(&cut, i);
    }

 
  if (cut == NULL)  Insert2(&cut, root_node);
 //PrintList(cut); 
 LifoTermine(LIFO);
  free(Mrk);
  free(SeededNodes);
  free(Rnk);
  free(Fth);
  free(Es);
  free(Map);
  free(Map2);
  return cut;
}



/*================================================*/
void Insert2(list **sl, int index)
/*================================================*/
{
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


/*================================================*/
void PrintList2(list *sl)
/*================================================*/
{
  fprintf(stderr, "Nodes of the cut:\n");
  while(sl)
    {
      printf("%d\n",sl->index);
      sl = sl->next;
    }
}

