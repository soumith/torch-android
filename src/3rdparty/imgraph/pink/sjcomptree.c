/*Implementation of "An efficient hierarchical graph based image segmentation"
by Silvio Jamil F. Guimaraes, Jean Cousty, Yukiko Kenmochi and Laurent Najman, submitted to ICIP 2012.

Original Tools from the Pink image processing library:
 authors : J. Cousty - L. Najman and M. Couprie 
Copyright ESIEE (2009) m.couprie@esiee.fr
Modifications: Dec. 2011 by Silvio Jamil F. Guimaraes.
Modifications: Jan 25 2012 by Camille Couprie*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <mcweightgraph.h>
#include <mcunionfind.h>
#include <mcsort.h>
#include <mclifo.h>
#include <jccomptree.h>
#include <sjcomptree.h>

#include <mccodimage.h>
#include <mcimage.h>
#include <mcutil.h>

#define MINSJ(x,y) ((x)>(y)?(y):(x))
#define MAXSJ(x,y) ((x)<(y)?(y):(x))
#define INFINITO 100000

//#define SJVERBOSE

int minKPath_rec(JCctree * CT,  graphe *g,int32_t rootEdge, int32_t leafEdge, int32_t * area, int32_t *k);

/*********************************************************************************************/
void abc_ite_last(JCctree * CT, graphe *g, int32_t root,  int32_t * area, int32_t * k, int last)
/*********************************************************************************************/
// last: boolean indicating if it is the last pass
// Output: fills the array k with the scale value associated with each edge.
{
  JCsoncell *s;
  int k1 = 0;
  int k2 = 0;
  Lifo * lifo = NULL;
  int32_t elem = root;
  int32_t * color = NULL;//root;
  int i = 0;
  
  lifo = CreeLifoVide(CT->nbnodes);
  color = (int32_t*) malloc(CT->nbnodes*sizeof(int32_t));
  
  //initialize all nodes as no-visit
  for (i = 0; i < CT->nbnodes; i++)
    color[i] = 0;
  
  LifoPush(lifo,root);
  
  while (!LifoVide(lifo)){
    elem = LifoPop(lifo);
    if (color[elem] == 1){ //visit the node (compute K)
      if(elem > CT->nbnodes/2){
	/*compute the max of the two min paths from
	   the leaves (vertex of the graph) to edge*/
	s = CT->tabnodes[elem].sonlist;
	k1 = minKPath_rec(CT,g,elem,s->son,area,k);
	k1 = MAXSJ(k1,k[s->son]);
	if (last==1)
	  {
	    s = s->next;
	    k2 = minKPath_rec(CT,g,elem,s->son,area,k);
	    k2 = MAXSJ(k2,k[s->son]);
	    k[elem] = MAXSJ(k1,k2);
	  }
      }
    }else{
      /*Post-traversal order*/
      LifoPush(lifo,elem);
      color[elem] = 1;
      for(s = CT->tabnodes[elem].sonlist; s != NULL; s = s->next) 
	{
	  LifoPush(lifo,s->son);
	}
    }
  }
  LifoTermine(lifo);
  free(color);	
}


/********************************************************************************************************/
int minKPath_rec(JCctree * CT,  graphe *g, int32_t rootEdge, int32_t leafEdge, int32_t * area, int32_t *k)
/********************************************************************************************************/
/*Compute the smallest k between the leaf and the root.*/
{
  int k1 = 0, kaux = INFINITO ;
  int elem = 0;
  int kmin = 0; /*smallest permitted value o K*/
  int kmax = 0; /*the highest K already computed on the path*/
  
  kmin = (CT->tabnodes[rootEdge].data_f - CT->tabnodes[leafEdge].data_f) * area[leafEdge];
  kmax = k[leafEdge];
  
  elem = leafEdge;
  while (elem != rootEdge){

    /*compute the volume*/
    k1 = (CT->tabnodes[rootEdge].data_f - CT->tabnodes[elem].data_f) * area[elem];
    if ((kmin < kmax) || (k1 > k[elem]))
      {
	/*if it is a plateau (the both elem and root are in the same plateau, 
	  so the k must be that already computed*/
	if (k1 == 0) kmin = k[elem]; 
	/*update the kmax*/
	if ((k[elem] > kmax)) kmax = k[elem];
	
	if (k1 > kmax)
	  {
	    if (k1 < k[elem])
	      kaux = k[elem];
	    else if (k1 > k[elem])
	      kaux = k1;
	    if (kaux > k[elem])
	      {
		kmin = MINSJ(kmin,kaux);
		if ((kmin < k[elem])) kaux = kmin;
	      }
	    kmin = MAXSJ(kmin,kaux);
	  }		
	else
	  kmin = MINSJ(kmin,kmax);
	elem = CT->tabnodes[elem].father;
      }else if (k1 < k[elem])
      elem = CT->tabnodes[elem].father; 
    else elem = CT->tabnodes[elem].father;
  }
  if (kmin == 0) kmin = k[leafEdge];
  return kmin;
}

/********************************************************************/
int computeK(JCctree * CT, graphe *g, int32_t * area, int32_t *k){
/********************************************************************/
  int i;
  if (k[CT->root] == -1)
    {
      for (i = 0; i <= CT->nbnodes/2;i++)
	k[i] = 0;
      
      abc_ite_last(CT,g,CT->root,area,k,1);
      
      for (i = 0; i < CT->nbnodes;i++)
	if (k[i] == -1) 	
	  abc_ite_last(CT,g,i,area,k,0);
    }
  return 1;
}

 
/* ======================================================================== */
void SJAreaMST(JCctree * CT, int32_t * area)
/* ======================================================================== */
/*Computes a Maximum Spaning Tree storing in the array "area" 
the areas of nodes in the Component Tree.*/
{
  int i;
  int32_t root = CT->root;
  JCsoncell *s;
  Lifo * lifo = NULL;
  int32_t elem = root;
  int32_t soma = 0;
  int32_t * color = NULL;//root;
  
  for (i = 0; i <= CT->nbnodes/2;i++)
    area[i] = 1;    
  
  lifo = CreeLifoVide(CT->nbnodes);
  color = (int32_t*) malloc(CT->nbnodes*sizeof(int32_t));
  //initialize all nodes as no-visit
  for (i = 0; i < CT->nbnodes; i++)
    color[i] = 0;
  
  LifoPush(lifo,root);
  
  while (!LifoVide(lifo))
    {
      elem = LifoPop(lifo);
      soma = 0;
      
      if (color[elem] == 1){ //visit the node (compute area)
	if(elem > CT->nbnodes/2){
	  for(s = CT->tabnodes[elem].sonlist; s != NULL; s = s->next){
	    soma = soma + area[s->son];
	  }
	  area[elem] = soma;
	}
      }else{
	LifoPush(lifo,elem);
	color[elem] = 1;
	for(s = CT->tabnodes[elem].sonlist; s != NULL; s = s->next) 
	  LifoPush(lifo,s->son);
      }
    }
  LifoTermine(lifo);
  free(color);
}

/* ==================================== */
JCctree * sjcomponentTreeAlloc(int32_t N)
/* ==================================== */
{
  JCctree *CT;	
  CT = (JCctree *)malloc(sizeof(JCctree)); 
  CT->tabnodes = (JCctreenode *)malloc(N * sizeof(JCctreenode));
  CT->tabsoncells = (JCsoncell *)malloc(2*N * sizeof(JCsoncell));
  CT->flags = NULL;
  if ((CT == NULL) || (CT->tabnodes == NULL) || (CT->tabsoncells == NULL))
    { fprintf(stderr, "componentTreeAlloc : malloc failed\n"); return NULL; }
  CT->nbnodes = N;
  CT->nbsoncells = 0;
  return CT;
} 

/* ==================================== */
void sjcomponentTreeFree(JCctree * CT)
/* ==================================== */
{
  free(CT->tabnodes);
  free(CT->tabsoncells);
  if(CT->flags != NULL)free(CT->flags);
  free(CT);
} 


/* ==================================== */
void sjcomponentTreePrint(JCctree *CT, int *Alt, int32_t root, int32_t *Att1)
/* ==================================== */
{
  JCsoncell *s; 
  char c;
  if(root <= CT->nbnodes/2) c = 'S'; else c = 'C';
  printf("---------------------\n%c%d\nAltitude: %d, nbsons: %d, area: %d\n", c, root, Alt[root],CT->tabnodes[root].nbsons,CT->tabnodes[root].area  );
  if( Att1 != NULL )
    printf("Attribut 1: %d\n",Att1[root]);
  printf("Fils: ");
  for(s = CT->tabnodes[root].sonlist; s != NULL; s = s->next){
    if(s->son <= CT->nbnodes/2) c = 'S'; else c = 'C';
    printf("%c%d (altitude %d) ",c,s->son, Alt[s->son]);
  }
  printf("\n");
  for(s = CT->tabnodes[root].sonlist; s != NULL; s = s->next)
    sjcomponentTreePrint(CT, Alt, s->son, Att1);
}


/*****************************************************/
void sjcalculReversePointer(JCctree *CT, int32_t root)  
/*****************************************************/
/*Useful to construct father relationship for the component tree*/
{
  JCsoncell *s;
  Lifo * lifo = NULL;
  int32_t elem = root;
  
  lifo = CreeLifoVide(CT->nbnodes);
  LifoPush(lifo,root);
  
  while (!LifoVide(lifo)){
    elem = LifoPop(lifo);
    for(s = CT->tabnodes[elem].sonlist; s != NULL; s = s->next) 
      {
	LifoPush(lifo,s->son);
	CT->tabnodes[s->son].father = elem;
      }
  }
  LifoTermine(lifo);
}

/****************************************************************/
int32_t saliencyTree(/* INPUTS */
		     graphe *g, 
		     /* OUTPUTS */
		     JCctree * ST, /* Component tree */
		     int32_t * mergeEdge
		     )
/****************************************************************/
/* The algorithm is an original adaptation/improvment to edge-weighted
   graph of the component tree algorithm of Najman and Couprie published in IEEE IP
   in 2006 */
{
  int32_t i,x1,x2,n1,n2,z,k, nbsoncellsloc;

  int32_t *clefs; 
  int32_t *STmap;
  JCsoncell * newsoncell1;
  JCsoncell * newsoncell2;
  Tarjan *T;
  int32_t taille = g->nbsom;
  int32_t narcs = g->ind;
 
  double *STaltitude;  /* weights associated to the nodes of the component tree */
  STaltitude = ( double * )calloc(2*g->nbsom, sizeof(double));

  if((STmap = (int32_t *)malloc(sizeof(int32_t) * taille)) == NULL){
    fprintf(stderr, "jcSalliancyTree: erreur de malloc\n"); }
   
  if((clefs = (int32_t*)malloc(sizeof(int32_t) * narcs)) == NULL){
    fprintf(stderr,"jcSaliencyTree: erreur de malloc\n"); exit(0); }
  
  for(i = 0; i < narcs; i++)
    clefs[i] = i; 
  //Sort edges in increasing weight order 
  d_TriRapideStochastique(clefs, g->weight, 0, narcs-1);
  // if( (ST = sjcomponentTreeAlloc((2*taille))) == NULL)
  // {fprintf(stderr, "jcSalliancyTree: erreur de ComponentTreeAlloc\n"); exit(0);}
  for(i = 0; i < ST->nbnodes; i++)
    ST->tabnodes[i].edge = -1;
    
  ST->nbnodes = taille;
  ST->nbsoncells = 0;
  T = CreeTarjan(taille);
  for(i = 0; i < taille; i++)
  {
    STmap[i] = i;
    ST->tabnodes[i].nbsons = 0;
    ST->tabnodes[i].sonlist = NULL;
    ST->tabnodes[i].data_f = 0;
    ST->tabnodes[i].data = 0;
    ST->tabnodes[i].area = 1;
    ST->tabnodes[i].edge = -1;
    ST->tabnodes[i].k = 0;
    ST->tabnodes[i].father = -1;
    TarjanMakeSet(T,i);
  }

  for(i = 0; i < narcs; i++){ 
    // for each edge of the MST taken in increasing order of altitude
    n1 = TarjanFind(T, g->tete[clefs[i]]);  n2 = TarjanFind(T,g->queue[clefs[i]]);
    if(n1 != n2) {
      /* If the two vertices do not belong to a same connected
	 component at a lowest level */
      // Which component of ST n1 and n2 belongs to ?
      x1 = STmap[n1]; x2 = STmap[n2];    
      // Create a new component
      z = ST->nbnodes; ST->nbnodes++; 
      nbsoncellsloc = ST->nbsoncells;
      ST->tabnodes[z].nbsons = 2;
      mergeEdge[z] = clefs[i];
      // the altitude of the new component is the altitude of the edge
      // under consideration
      STaltitude[z] = getweight(g, clefs[i]);
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
      //SILVIO
      ST->tabnodes[z].data_f = STaltitude[z];
      if (clefs[i] <0) 			
	clefs[i] = clefs[i];
      ST->tabnodes[z].edge = clefs[i];
      ST->tabnodes[z].k = g->weight[i];
    }//if(...)
  }//for(...)


  for(i = 0; i < taille; i++)
    {
      STaltitude[i] = 0; /* altitudes of the leaves is 0 */
      if (ST->tabnodes[i].edge < -1)
	STaltitude[i] = 0;
    }
  
  /* Construct father relationship */
  ST->tabnodes[ST->nbnodes-1].father = -1;
  ST->root = ST->nbnodes - 1;
  sjcalculReversePointer(ST, ST->root); 

  for(i = 0; i < ST->nbnodes; i++)
    {
      if (ST->tabnodes[z].edge < -1)
	STaltitude[i] = 0;
    }

  //(*SaliencyTree) = ST;

  #ifdef SJVERBOSE 
  sjcomponentTreePrint(ST,STaltitude, ST->root, NULL); 
 printf("*************************************************\n");
#endif
  /* liberation de la memoire */
  free(STmap); 
  free(clefs);
  TarjanTermine(T);

  sjcalculReversePointer(ST, ST->root);
  return 1;
}



/********************************************************************************/
void concat_lists(JCsoncell **b1,JCsoncell **b2, JCsoncell **b3)
     {
       JCsoncell *z;
       if(b1==NULL)
	 (*b3)=(*b2);
       else
	 {
	   (*b3)=(*b1);
	 }
      
       if(*b2!=NULL)
	 {

	   z=(*b1); 
	   if (z==NULL) (*b3)=(*b2);
	   else {
	   while(z->next!=NULL)
	     z=z->next;

	   z->next=(*b2);

	   }
	 }

     }


void ListDelete(JCsoncell **listP, int value)
{
  JCsoncell *currP, *prevP;
  // For 1st node, indicate there is no previous. 
  prevP = NULL;

  // Visit each node, maintaining a pointer to  the previous node we just visited.
   
  for (currP = *listP;
	currP != NULL;
	prevP = currP, currP = currP->next) {

    if (currP->son == value) 
      {  
	if (prevP == NULL) {
	  // Fix beginning pointer. 
	  *listP = currP->next;
	} else {
	  // Fix previous node's next to skip over the removed node. 
	  prevP->next = currP->next;
	}

      /* Deallocate the node. */
      //free(currP);
      return;
    }
  }
}


/* ====================================================================== */
void FilterHierarchy(graphe *g, JCctree *CT, int32_t * Alt, int SIZE)
// Camille Couprie
/* ===================================================================== */
/*Inputs: 
- a graph g where weights correspond to the hierarchy scale S [Guimaraes et al, ICIP 2012].
- an integer k: threshold for the hierarchy level for the final segmentation.
- an integer SIZE: the minimal area (in nb of pixels) for the smallest region.
Output: 
- an array labeling_nodes that contains the final segmentation.
 */
{
  int32_t i,x,y,root;
  int32_t *flat_Alt; 
  int t1, t2, t3, t4, t5;
 JCsoncell *s,*p,*ss;
 int nb_explored_sons= 0;
int maxheight = 0;
 //(manual thresholds correspunding to values of k1, ...kn)
 for(i = 0; i < CT->nbnodes; i++)
   if(maxheight < Alt[i])maxheight=Alt[i];
 printf("maxheight = %d \n", maxheight);
printf(" Original nbnodes= %d \n", CT->nbnodes);
  flat_Alt = (int32_t*)malloc(sizeof(int32_t) * CT->nbnodes);
t5=500;
t4=300;
  t3=150;
  t2=30;
  t1=10;
  for(i = 0; i < CT->nbnodes; i++)
    {
      flat_Alt[i]=6;
      if(Alt[i]<t1)  flat_Alt[i]=1;
      if((Alt[i]>=t1)&&(Alt[i]<t2))   flat_Alt[i]=2;
      if((Alt[i]>=t2)&&(Alt[i]<t3))   flat_Alt[i]=3;
      if((Alt[i]>=t3)&&(Alt[i]<t4))   flat_Alt[i]=4;
      if((Alt[i]>=t4)&&(Alt[i]<t5))   flat_Alt[i]=5;
      if(i <= CT->nbnodes/2) flat_Alt[i]=0; //leafs
    }


 i = CT->nbnodes-1;
 while(i> CT->nbnodes/2)
  {
    //fprintf(stderr, "Explore node %d\n", i);
    
    for(s = CT->tabnodes[i].sonlist; s != NULL; s = s->next)
      {
	if(( flat_Alt[s->son]== flat_Alt[i] )||((CT->tabnodes[s->son].area <SIZE)&&(s->son > CT->nbnodes/2)))
	  // merge_node:  insert CT->tabnodes[s->son] in CT->tabnodes[i] ;
	  {
	    
	    
	    ListDelete(&CT->tabnodes[i].sonlist,s->son);

	    //fprintf(stderr, "deleted node %d\n", s->son);
	    if (s->son > CT->nbnodes/2)//s not a son node. Then add the sons lists of s to the list of i
	      {
		//	fprintf(stderr, "add the list of sons of %d to node %d \n List:", s->son,i );	
		p=CT->tabnodes[i].sonlist;
		concat_lists(&CT->tabnodes[i].sonlist, &CT->tabnodes[s->son].sonlist, &p);
		CT->tabnodes[i].sonlist = p;
		CT->tabnodes[i].nbsons=0;

		for(ss = CT->tabnodes[i].sonlist; ss != NULL; ss = ss->next)
		  {
		    // fprintf(stderr," %d   ",ss->son); 
		    CT->tabnodes[i].nbsons++;
		    CT->tabnodes[ss->son].father = i ;
		  }
		//fprintf(stderr,"\n");
		CT->tabnodes[s->son].nbsons=-1;//delete the node
	      }
	    else 
	      {	
		CT->tabnodes[s->son].nbsons=-1;
		CT->tabnodes[i].nbsons--;
		//	fprintf(stderr, "CT->tabnodes[%d].nbsons %d\n", i, CT->tabnodes[i].nbsons);
	      }
	      
	  }
	if(s->son <= CT->nbnodes/2) nb_explored_sons++;
      }
    i--;
    while (CT->tabnodes[i].nbsons==-1) i--;
    }
 // fprintf(stderr,"-------------------********************------------ \n");


//(2) update nodes indexes in the tree 
// sjcomponentTreePrint(CT,Alt, CT->root, NULL); 
 int j=0;
 long *index_nodes = (long *)malloc(sizeof(long)*CT->nbnodes);
 long *inv_index_nodes = (long *)malloc(sizeof(long)*CT->nbnodes);
 for (i=0; i<CT->nbnodes; i++) 
   {
     if (CT->tabnodes[i].nbsons !=-1)
       { // the node exists
	 index_nodes[i]=j;
	 inv_index_nodes[j]=i;
	 Alt[j]=flat_Alt[i];
	 CT->tabnodes[j].data= Alt[j];
	 // fprintf(stderr,"index[%d]=%d \n",i,j);
       j++;
     }
   }
 
 // update son lists
 j=0;
 int nb_nodes=0;
 for (i=0; i<CT->nbnodes; i++) 
   {
     if (CT->tabnodes[i].nbsons !=-1)
       { // the node exists
       	
	 nb_nodes++;
	 //	 CT->tabnodes[i].father = index_nodes[CT->tabnodes[i].father];
	 if(i!=j){ //change node's position
	   CT->tabnodes[j].sonlist = CT->tabnodes[i].sonlist;
	   CT->tabnodes[j].nbsons = CT->tabnodes[i].nbsons;
	   for(ss = CT->tabnodes[i].sonlist; ss != NULL; ss = ss->next)
	     ss->son=index_nodes[ss->son];
	 //CT->tabnodes[j].father = CT->tabnodes[i].father;
       }
       j++;
     }
     }
 
 //update fathers 
 for (i=0; i<nb_nodes; i++) 
       {
	 // printf("%d\n",inv_index_nodes[i]);
	 if(CT->tabnodes[inv_index_nodes[i]].father!=-1)
	   CT->tabnodes[i].father = index_nodes[CT->tabnodes[inv_index_nodes[i]].father];
	 else
	   CT->tabnodes[i].father = -1;
      
       }
nb_nodes--;
 // update the tree constants
 CT->root = nb_nodes;
 CT->nbsoncells=nb_nodes;
 CT->nbnodes=nb_nodes+1;
 printf(" Final nbnodes= %d \n", CT->nbnodes);
 // sjcomponentTreePrint(CT,Alt, CT->root, NULL); 
 free(flat_Alt);

}


/* ====================================================================== */
void LabelImage(JCctree *CT, int k, int SIZE, int32_t *labeling_nodes, int Nb_pixels )
// Camille Couprie
/* ===================================================================== */
/*Inputs: 
- a graph g where weights correspond to the hierarchy scale S [Guimaraes et al, ICIP 2012].
- an integer k: threshold for the hierarchy level for the final segmentation.
- an integer SIZE: the minimal area (in nb of pixels) for the smallest region.
Output: 
- an array labeling_nodes that contains the final segmentation.
 */
{
  int32_t i,x,y,root;
  int32_t *clefs; 
  int32_t *Mrk;
  Tarjan *T;
  int32_t taille = CT->nbnodes; 

  Mrk = (int32_t *)calloc(sizeof(int32_t), taille);
  for (i=0; i<taille; i++) Mrk[i]=1;
 
 
  //(1) Merge nodes according to the threashold S(X,Y) <= k (See equation (4) of [Guimaraes et al, ICIP 2012])
  // Explore nodes in a decreasing order of height level
  int color = 0;
  for(i = CT->nbnodes-1; i >= 0 ; i--) 
    { 
      // for each edge of the MST taken in increasing order of altitude
      //	fprintf(stderr,"i= %d \n", i);
      if(CT->tabnodes[i].data <= k)
	{
	  //  fprintf(stderr,"limite altitude pere %d \n",CT->tabnodes[CT->tabnodes[i].father].data );
	  if (CT->tabnodes[CT->tabnodes[i].father].data >k)
	    {
	      //fprintf(stderr,"on change de couleur \n");
	      color ++;
	      Mrk[i]=color;
	    }
	  else //propagate colors
	    {
	      //fprintf(stderr,"couleur prop \n");
	      Mrk[i]= Mrk[CT->tabnodes[i].father];
	    }
	}

    }
  
  for (i=0; i<Nb_pixels; i++)
    { 
      labeling_nodes[i] = Mrk[i];
      // fprintf(stderr,"%d \n", labeling_nodes[i]);
    }
}

/***************************************************/
void ColorSegmentation(graphe *g, int32_t * labels )
/***************************************************/
{
 /*(3) Color the image according to the obtained labeling */
  struct xvimage *image_r, *image_g, *image_b;
  unsigned char *F_r, *F_g, *F_b;
  int rs, cs, N, x, y;
  rs = g->rs;
  cs = g->cs;
  N = rs * cs;
  image_r = allocimage(NULL, rs, cs, 1, VFF_TYP_1_BYTE); 
  image_g = allocimage(NULL, rs, cs, 1, VFF_TYP_1_BYTE); 
  image_b = allocimage(NULL, rs, cs, 1, VFF_TYP_1_BYTE); 

  F_r = UCHARDATA(image_r);
  F_g = UCHARDATA(image_g);
  F_b = UCHARDATA(image_b);
  // build lookup table
  int MaxNbRegions=rs*cs;
  int Tab[MaxNbRegions][3];
  for (x = 0; x <MaxNbRegions ; x++)
    for (y = 0; y < 3; y++)
      Tab[x][y] = (int)( 255.0 * rand() / ( RAND_MAX ) );
      
  for(x = 0; x < N; x++)
    { 
      F_r[x] = Tab[labels[x]][0];
      F_g[x] = Tab[labels[x]][1];
      F_b[x] = Tab[labels[x]][2];
    }

  writergbimage(image_r, image_g, image_b, "result.ppm" );
}

/***************************************************************************************************/
int32_t HierarchicalSegmentation(graphe *g, JCctree *CT, int32_t * results,int32_t * MergeEdges ){
/************************************************************************************************/
/* Computes a min-spanning three based segmentation (segmentmst)
   originates from Felzenszwalb's 2004 paper: "Efficient Graph-Based Image Segmentation",
 and adapted to hierarchical segmentation using the component tree (CT) of the image */

  int32_t i, nbmin;
  
  int32_t * area;
  int maxV = 0;
 
  //(0) Memory allocation

  area= ( int32_t * )calloc(2*g->nbsom, sizeof(int32_t));
 
  //(1) Compute the component tree 
  saliencyTree(g,CT,MergeEdges);

  for (i = 0; i <CT->nbnodes; i++) 
    {
      results [i] = -1;
      area[i] = 0;
    }
  
  //initialize the array results to store the scale values K
  for (i = 0; i <=CT->nbnodes/2; i++)  
    results [i] = 0;
  
  /*(2) Compute a maximum spanning tree */
    SJAreaMST(CT,area);

  /*(3) Compute the scale value K for each edge on the hierarchy */
    computeK(CT,g,area,results);

    #ifdef SJVERBOSE
    printf("\n"); for (i = 0; i <CT->nbnodes; i++)  {
      printf("node %d: [%d - Area (%d)] - %d,%d, edge: %d\n", i,results[i], area[i],
	     CT->tabnodes[i].k,CT->tabnodes[i].father, CT->tabnodes[i].edge); }
    #endif
  
    for (i = 0; i <CT->nbnodes; i++)
      CT->tabnodes[i].area = area[i];
  
  /*PUT THE VALUES ON THE GRAPH*/
  for(i = 0; i < 2*g->nbsom-1; i++)
    {
      if (CT->tabnodes[i].edge != -1)
	{
	  nbmin = CT->tabnodes[i].edge;
	  if (CT->tabnodes[i].k == 0)
	    CT->tabnodes[i].k = CT->tabnodes[CT->tabnodes[i].father].k;
	  g->weight[nbmin] = (double) results[i];
	  if (maxV < results[i])
	    maxV = results[i];
	}
    }
int min_area_size=50;

  //  sjcomponentTreePrint(CT,results, CT->root, NULL); 
  //FilterHierarchy(g, CT, results, min_area_size);


//temporary removed for building simple merge trees
for (i=0; i<CT->nbnodes; i++) 
  CT->tabnodes[i].data= results[i];


  int32_t *labels= ( int32_t * )malloc(g->nbsom* sizeof(int32_t));
  int k=50; 
  LabelImage(CT, k, min_area_size, labels,g->nbsom);
  ColorSegmentation(g, labels );


  free(area);
  // sjcomponentTreeFree(CT);
  
  return nbmin;
}


/***************************************************************************************************/
int32_t MergeTree_compatibleArbelaez(graphe *g, JCctree *CT, int32_t * results,int32_t * MergeEdges ){
/************************************************************************************************/
  int32_t i;
 
  // Compute the component tree and returns the levels in results
  saliencyTree(g,CT,MergeEdges);
  for (i=0; i<CT->nbnodes; i++) 
    results[i]= CT->tabnodes[i].data_f;

  return 0;
}
