/*
  Prim's algorithm for Maximum Spanning Forest (MSF) computation
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
#include "MSF_utils.h"
#include <mcrbt.h>

#define false 0
#define true 1
#define minimum(X,Y) ((X)<=(Y)?(X):(Y))
#define maximum(X,Y) ((X)>=(Y)?(X):(Y))

void Insert3(list **sl, int index);

/*=====================================================================================*/
list * MSF_Prim(MergeTree * MT)
/*=====================================================================================*/
/*Segment a tree into two components.
  Returns a list of nodes correspunding to the Max Spanning Forest cut,
  computed using Prim's algorithm */
{
  int32_t i, j,u,v, x,y,z, x_1,y_1;
  int nb_markers; int nb_leafs;
  long N, M;

// -------- Gathering usefull input graph (MT) informations -----------

  float val=0; //weight parameter for leafs.
  mtree * T= MT->tree;  // mergeTreePrint(T);
  float * W = MT->weights;
  JCctree *CT = T->CT;
  int root_node = CT->root;

  //M = nb nodes
  M = CT->nbnodes;

  //N = nb_edges
  nb_leafs = 0;
  for (i = 0; i < M; i++)
    if (CT->tabnodes[i].nbsons == 0)
      nb_leafs++;

  nb_markers = nb_leafs+1;
  N=M+nb_markers;
  M=N-1;
  

  //init Prim
  //Creates a Red-Black tree to sort edges
  Rbt *L;
  IndicsInit(M);
  L = mcrbt_CreeRbtVide(M);
  i=0;
  int sizeL = 0;

  // Set for already checked edges
  for(u = 0; u < M; u ++)
    Set(u, false);

  // marked nodes
  uint32_t * SeededNodes = (uint32_t*)malloc(nb_markers*sizeof(uint32_t));
  if (SeededNodes == NULL) { fprintf(stderr, "prim : malloc failed\n"); exit(0);}

  // Resulting node labeling goes into G2
  uint8_t * G2 = (uint8_t*)calloc(N ,sizeof(uint8_t));

  // fill the array SeededNodes with marked index nodes
  // fill the tree L only with the marked edges
  SeededNodes[0]= M;
  j=1;
  for (i = 0; i < CT->nbnodes; i++)
    if (CT->tabnodes[i].nbsons == 0)
      {
        SeededNodes[j]= i+CT->nbnodes;
        G2[SeededNodes[j]] = 2;
        mcrbt_RbtInsert(&L, (TypRbtKey)(val), SeededNodes[j]);
        sizeL++;
        //      fprintf(stderr,"L=%d", sizeL);
        Set(SeededNodes[j], true);
        j++;
      }
  G2[root_node]=1;
  mcrbt_RbtInsert(&L, (TypRbtKey)(1-W[root_node]), root_node);
  sizeL++;
  Set(root_node, true);

  // weights
  float * Weights = (float *)malloc(M*sizeof(float));
  for(j=0;j<CT->nbnodes;j++)
    Weights[j]=W[j];

  for(j=0;j<nb_leafs;j++)
    Weights[CT->nbnodes+j]=val;
  
  // While there exists unprocessed nodes
  while(sizeL != 0)
    {
      //Pick an edge u of min weight in the tree.
      u = RbtPopMin(L); // fprintf(stderr, "pop %d\n", u);
      sizeL--;
      // Find its extreme nodes (x,y)
      x = u; // x = G->Edges[0][u];
      if (u<CT->nbnodes) y= CT->tabnodes[u].father;
      else if(u!=M) y= u-CT->nbnodes;
      else y=root_node;
      if (y==-1)y=M;  //y = G->Edges[1][u];

      // y must correspond to the marked node.
      if(G2[x] > G2[y])
        {z=x; x=y; y=z;}

      // if one node is labeled
      if((minimum(G2[x],G2[y]) == 0) && (maximum(G2[x],G2[y]) > 0))
        {
          // assign the same label to the other one
          G2[x] = G2[y];
          //fprintf(stderr,"Map[%d]=Map[%d]\n",x,y);

          // select neighbors edges to place them in the tree
          j= nb_neighbors(u, CT, nb_leafs);
          //fprintf(stderr,"nb_neigbors= %d \n",j);
          for (i=0;i<j;i++)
            {
              v = neighbor(u, i, CT, nb_leafs, SeededNodes);
              if (v==-1)v=M;
              // fprintf(stderr," %d ",v);

              // if the edge v is not processed yet
              if(!IsSet(v, true))
                {
                  // Find its extreme nodes (x_1,y_1)
                  x_1 = v;
                  if (v<CT->nbnodes) y_1= CT->tabnodes[v].father;
                  else if(v!=M) y_1= v-CT->nbnodes;
                  else y_1 = root_node;
                  if (y_1==-1)y_1=M;
                  //fprintf(stderr," [%d %d] ",x_1, y_1);
                  if((minimum(G2[x_1],G2[y_1]) == 0) && (maximum(G2[x_1],G2[y_1]) > 0))
                    {
                      //fprintf(stderr,"( insert %d) ",v);
                      mcrbt_RbtInsert(&L, (TypRbtKey)(1-Weights[v]), v);
                      sizeL++;
                      Set(v,true);
                    }
                }
            }
          // fprintf(stderr," \n");
        }
      UnSet(u,true);
    }

  /* for (i=0; i<N; i++)
     printf("Map[%d]=%d \n",i,G2[i]-1);*/
    
      // Process the tree to find the cut
      list * cut = NULL;
      for (i = 0; i < CT->nbnodes; i++)
	{
	  // nodes having a different value than their father are in the cut
	  if ((CT->tabnodes[i].father != -1) && (G2[CT->tabnodes[i].father] != G2[i]))
	    Insert3(&cut, i);
	  // leafs having the same label as the root are in the cut
	  if ((CT->tabnodes[i].nbsons == 0) && (G2[i]-1==0))
	    Insert3(&cut, i);
	}
      
      if (cut == NULL)  Insert3(&cut, root_node);

      // PrintList(cut);
      IndicsTermine();
      free(G2);
      mcrbt_RbtTermine(L);
      free(SeededNodes);
      free(Weights);
      return cut;
    
}



/*================================================*/
void Insert3(list **sl, int index)
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
