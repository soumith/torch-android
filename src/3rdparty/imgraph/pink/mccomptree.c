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
  Michel Couprie - septembre 2005 : area
*/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcunionfind.h>
#include <mccodimage.h>
#include <mccomptree.h>

#define MAXTREE
#define PARANO
//#define VERBOSE

//#define DEBUG

/* ==================================== */
int32_t * linsortimageup(uint8_t *F, int32_t N)
/* ==================================== */
/*
  Tri par denombrement - cf. Cormen & al., "Introduction a l'algorithmique"
  version pour une image sur 8 bits
  F: l'image
  N: le nombre de pixels
  retourne: un tableau d'indices de pixels (taille N) 
            dans l'ordre croissant des valeurs
*/
#undef F_NAME
#define F_NAME "linsortimageup"
{
  int32_t i, j, k, H[256];
  int32_t *T = (int32_t *)calloc(1,N * sizeof(int32_t));
  if (T == NULL)
  {   fprintf(stderr, "%s() : malloc failed for T\n", F_NAME);
      return NULL;
  }
  for (i = 0; i < 256; i++) H[i] = 0;   // initialise l'histogramme
  for (i = 0; i < N; i++) H[F[i]] += 1; // calcule l'histogramme
  j = H[0]; H[0] = 0;                   // calcule l'histogramme cumule
  for (i = 1; i < 256; i++) { k = H[i]; H[i] = j; j += k; }
  for (i = 0; i < N; i++)               // tri lineaire
  {
    k = F[i]; j = H[k]; T[j] = i; H[k] += 1;
  }
  return T;
} /* linsortimageup() */

/* ==================================== */
int32_t * linsortimagedown(uint8_t *F, int32_t N)
/* ==================================== */
/*
  Tri par denombrement - cf. Cormen & al., "Introduction a l'algorithmique"
  version pour une image sur 8 bits
  F: l'image
  N: le nombre de pixels
  retourne: un tableau d'indices de pixels (taille N) 
            dans l'ordre decroissant des valeurs
*/
#undef F_NAME
#define F_NAME "linsortimagedown"
{
  int32_t i, j, k, H[256];
  int32_t *T = (int32_t *)calloc(1,N * sizeof(int32_t));
  if (T == NULL)
  {   fprintf(stderr, "%s() : malloc failed for T\n", F_NAME);
      return NULL;
  }
  for (i = 0; i < 256; i++) H[i] = 0;   // initialise l'histogramme
  for (i = 0; i < N; i++) H[F[i]] += 1; // calcule l'histogramme
  j = H[255]; H[255] = 0;               // calcule l'histogramme cumule
  for (i = 254; i >= 0; i--) { k = H[i]; H[i] = j; j += k; }
  for (i = 0; i < N; i++)               // tri lineaire
  {
    k = F[i]; j = H[k]; T[j] = i; H[k] += 1;
  }
  return T;
} /* linsortimagedown() */

/* ==================================== */
void addson(ctree *CT, int32_t node, int32_t nodeaux)
/* ==================================== */
// add nodeaux to the lists of sons of node
// operation done in constant time
#undef F_NAME
#define F_NAME "addson"
{
  soncell * newson;
#ifdef DEBUGADDSON
  printf("addson: %d %d\n", node, nodeaux);
#endif
  if (CT->nbsoncells >= CT->nbnodes)
  {
    fprintf(stderr, "%s : fatal error : maximum nb of cells exceeded\n", F_NAME);
    //    mccomptree_ComponentTreePrint(CT);
    exit(1);
  }
#ifdef PARANO
  if (CT->tabnodes[node].nbsons > 0)
    for (newson = CT->tabnodes[node].sonlist; newson != NULL; newson = newson->next)
      if (newson->son == nodeaux)
      {
        fprintf(stderr, "%s : error : son already in list\n", F_NAME);
        return;
      }
#endif
  newson = &(CT->tabsoncells[CT->nbsoncells]);
  CT->nbsoncells += 1;
  newson->son = nodeaux;
  if (CT->tabnodes[node].nbsons == 0)
  {
    newson->next = NULL;
    CT->tabnodes[node].sonlist = CT->tabnodes[node].lastson = newson;
  }
  else
  {
    newson->next = CT->tabnodes[node].sonlist;
    CT->tabnodes[node].sonlist = newson;
  }
  CT->tabnodes[node].nbsons += 1;
  CT->tabnodes[nodeaux].father = node;
} // addson()

/* ==================================== */
void mergenodes(ctree *CT, int32_t node, int32_t nodeaux)
/* ==================================== */
// add the sons of nodeaux to the lists of sons of node, 
// and mark nodeaux as deleted - operation done in constant time
#undef F_NAME
#define F_NAME "mergenodes"
{
#ifdef PARANO
  soncell * nodeson, * nodeauxson;  
  if ((CT->tabnodes[node].nbsons > 0) && (CT->tabnodes[nodeaux].nbsons > 0))
    for (nodeson = CT->tabnodes[node].sonlist; nodeson != NULL; nodeson = nodeson->next)
      for (nodeauxson = CT->tabnodes[nodeaux].sonlist; nodeauxson != NULL; nodeauxson = nodeauxson->next)
        if (nodeson->son == nodeauxson->son)
        {
          fprintf(stderr, "%s : error : son already in list (%d)\n", F_NAME, nodeson->son);
          return;
        }
#endif
  if (CT->tabnodes[nodeaux].nbsons <= 0)
  {
    CT->tabnodes[nodeaux].nbsons = -1;
    return;
  }
  if (CT->tabnodes[node].nbsons > 0)
  {
    soncell *lson = CT->tabnodes[node].lastson;
    lson->next = CT->tabnodes[nodeaux].sonlist;
    CT->tabnodes[node].lastson = CT->tabnodes[nodeaux].lastson;
    CT->tabnodes[node].nbsons += CT->tabnodes[nodeaux].nbsons;
  }
  else
  {
    CT->tabnodes[node].sonlist = CT->tabnodes[nodeaux].sonlist;
    CT->tabnodes[node].lastson = CT->tabnodes[nodeaux].lastson;
    CT->tabnodes[node].nbsons = CT->tabnodes[nodeaux].nbsons;
  }
  CT->tabnodes[nodeaux].nbsons = -1;
} // mergenodes()

/* ==================================== */
ctree * ComponentTreeAlloc(int32_t N)
/* ==================================== */
#undef F_NAME
#define F_NAME "ComponentTreeAlloc"
{
  ctree *CT;
  CT = (ctree *)calloc(1,sizeof(ctree));
  CT->tabnodes = (ctreenode *)calloc(1,N * sizeof(ctreenode));
  CT->tabsoncells = (soncell *)calloc(1,N * sizeof(soncell));
  CT->flags = (uint8_t *)calloc(N, sizeof(char));
  if ((CT == NULL) || (CT->tabnodes == NULL) || (CT->tabsoncells == NULL) || (CT->flags == NULL))
  { 
    fprintf(stderr, "%s : malloc failed\n", F_NAME);
    return NULL;
  }
  CT->nbnodes = N;
  CT->nbleafs = 0;
  CT->nbsoncells = 0;
  return CT;
} // ComponentTreeAlloc()

/* ==================================== */
void ComponentTreeFree(ctree * CT)
/* ==================================== */
{
  free(CT->tabnodes);
  free(CT->tabsoncells);
  free(CT->flags);
  free(CT);
} // ComponentTreeFree()

/* ==================================== */
void mccomptree_ComponentTreePrint(ctree * CT)
/* ==================================== */
{
  int32_t i;
  soncell *s;
  printf("root = %d ; nbnodes: %d ; nbleafs: %d ; nbsoncells: %d\n", CT->root, CT->nbnodes, CT->nbleafs, CT->nbsoncells);
  for (i = 0; i < CT->nbnodes; i++) if (CT->tabnodes[i].nbsons != -1)
  {
#ifdef ATTRIB_VOL
    printf("node: %d ; level %d ; nbsons: %d ; father: %d ; area: %d ; vol: %d ; ", 
            i, CT->tabnodes[i].data, CT->tabnodes[i].nbsons, CT->tabnodes[i].father, CT->tabnodes[i].area, CT->tabnodes[i].vol);
#else
    printf("node: %d ; level %d ; nbsons: %d ; father: %d ; ", 
            i, CT->tabnodes[i].data, CT->tabnodes[i].nbsons, CT->tabnodes[i].father);
#endif
    if (CT->tabnodes[i].nbsons > 0)
    {
      printf("sons: ");
      for (s = CT->tabnodes[i].sonlist; s != NULL; s = s->next)
        printf("%d  ", s->son);
    }
    printf("\n");
  }
} // mccomptree_ComponentTreePrint()

#ifdef ATTRIB_AREA
/* ==================================== */
int32_t ComputeArea(ctree * CT, int32_t node, int32_t *na1)
/* ==================================== */
/*
   Calcule la surface de chacune des composantes, a partir de 
   l'information stockee dans CT->tabnode[i].area, qui correspond a la 
   difference de surface entre la composante i et ses filles.
   Le resultat est provisoirement stocke dans le tableau na1 (indexe par
   le numero de composante i), pour etre ensuite recopie dans CT.
*/ 
#undef F_NAME
#define F_NAME "ComputeArea"
{
  soncell * s;
  int32_t son;
  na1[node] = CT->tabnodes[node].area;
  if (CT->tabnodes[node].nbsons == 0) return na1[node];
  if (CT->tabnodes[node].nbsons > 0)
  {
    for (s = CT->tabnodes[node].sonlist; s != NULL; s = s->next)
    {
      son = s->son; 
      na1[node] += ComputeArea(CT, son, na1);
    }
    return na1[node];
  }
#ifdef PARANO
  printf("%s : structure arbre pervertie", F_NAME);
  exit(0);
#endif              
} /* ComputeArea() */

#ifdef ATTRIB_VOL
/* ==================================== */
int32_t ComputeVol(ctree * CT, int32_t node, int32_t *na1)
/* ==================================== */
/*
   Calcule la surface de chacune des composantes, a partir de 
   l'information stockee dans CT->tabnode[i].area, qui correspond a la 
   surface de la composante i.
   Le resultat est provisoirement stocke dans le tableau na1 (indexe par
   le numero de composante i), pour etre ensuite recopie dans CT.
*/ 
#undef F_NAME
#define F_NAME "ComputeVol"
{
  soncell * s;
  int32_t son, fth;
  na1[node] = CT->tabnodes[node].area;
  fth = CT->tabnodes[node].father;
  if (fth != -1)
    na1[node] = na1[node] * (CT->tabnodes[node].data - CT->tabnodes[fth].data);
  if (CT->tabnodes[node].nbsons == 0) return na1[node];
  if (CT->tabnodes[node].nbsons > 0)
  {
    for (s = CT->tabnodes[node].sonlist; s != NULL; s = s->next)
    {
      son = s->son; 
      na1[node] += ComputeVol(CT, son, na1);
    }
    return na1[node];
  }
#ifdef PARANO
  printf("%s : structure arbre pervertie", F_NAME);
  exit(0);
#endif              
} /* ComputeVol() */
#endif
#endif

/* ==================================== */
int32_t ComponentTree( uint8_t *F, int32_t rs, int32_t N, int32_t connex, // inputs
                    ctree **CompTree, // output
                    int32_t **CompMap     // output
                  )
/* ==================================== */
#undef F_NAME
#define F_NAME "ComponentTree"
{
  ctree *CT; 
  int32_t *CM; 
  int32_t *S; 
  int32_t i, k, p, q, incr_vois, tmp; 
  int32_t numbernodes = N; 
  int32_t *SubtreeRoot;
  int32_t currentSubtree;
  int32_t currentNode;
  int32_t neighbSubtree;
  int32_t neighbNode;
  Tarjan *T1, *T2;
  soncell *sc; int32_t son;

  CT = ComponentTreeAlloc(N);
  if (CT == NULL)
  {   fprintf(stderr, "%s() : ComponentTreeAlloc failed\n", F_NAME);
      return 0;
  }
  CM = (int32_t *)calloc(1,N * sizeof(int32_t));
  if (CM == NULL)
  {   fprintf(stderr, "%s() : malloc failed for CM\n", F_NAME);
      return 0;
  }
  SubtreeRoot = (int32_t *)calloc(1,N * sizeof(int32_t));
  if (SubtreeRoot == NULL)
  {   fprintf(stderr, "%s() : malloc failed for SubtreeRoot\n", F_NAME);
      return 0;
  }
#ifdef MAXTREE
  S = linsortimagedown(F, N);
#else
  S = linsortimageup(F, N);
#endif
  if (S == NULL)
  {   fprintf(stderr, "%s() : linsortimage failed\n", F_NAME);
      return 0;
  }
  T1 = CreeTarjan(N);
  T2 = CreeTarjan(N);
  if ((T1 == NULL) || (T2 == NULL))
  {   fprintf(stderr, "%s() : CreeTarjan failed\n", F_NAME);
      return 0;
  }
  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
  } /* switch (connex) */

  for (i = 0; i < N; i++) 
  {
    TarjanMakeSet(T1, i);
    TarjanMakeSet(T2, i);
    SubtreeRoot[i] = i;
    CT->tabnodes[i].nbsons = 0;
    CT->tabnodes[i].data = F[i];
    CT->tabnodes[i].father = -1;
#ifdef ATTRIB_AREA
    CT->tabnodes[i].area = 1;
#endif
  }

  for (i = 0; i < N; i++) 
  {
    p = S[i];  // pixel courant, dans l'ordre croissant
#ifdef DEBUG
printf("pixel %d ; valeur %d\n", p, F[p]);
#endif
    currentSubtree = TarjanFind(T1, p);
    currentNode = TarjanFind(T2, SubtreeRoot[currentSubtree]);
#ifdef DEBUG
printf("currentSubtree %d ; currentNode %d\n", currentSubtree, currentNode);
#endif
 
    for (k = 0; k < 8; k += incr_vois)
    {
      q = voisin(p, k, rs, N);
#ifdef MAXTREE
      if ((q != -1) && (F[q] >= F[p]))
#else
      if ((q != -1) && (F[q] <= F[p]))
#endif
      {
#ifdef DEBUG
printf("  neighbor %d ; valeur %d\n", q, F[q]);
#endif
        neighbSubtree = TarjanFind(T1, q);
        neighbNode = TarjanFind(T2, SubtreeRoot[neighbSubtree]);
#ifdef DEBUG
printf("  neighbSubtree %d ; neighbNode %d\n", neighbSubtree, neighbNode);
#endif
        if (currentNode != neighbNode)
        {
          if (CT->tabnodes[currentNode].data == CT->tabnodes[neighbNode].data)
          { // merge the nodes
            numbernodes -= 1;
            tmp = TarjanLink(T2, neighbNode, currentNode);
#ifdef PARANO
            if ((tmp != currentNode) && (tmp != neighbNode)) 
              printf("%s : choix inattendu pour le representant", F_NAME);
#endif            
            if (tmp == currentNode) mergenodes(CT, currentNode, neighbNode);
            else                    mergenodes(CT, neighbNode, currentNode);
#ifdef DEBUG
            printf("  mergenodes: %d %d\n", currentNode, neighbNode);
#endif
#ifdef ATTRIB_AREA
            CT->tabnodes[tmp].area = CT->tabnodes[currentNode].area + CT->tabnodes[neighbNode].area;
#endif            
	    currentNode = tmp;
	  }
          else
          { // add neighbNode as a new son of currentNode
            addson(CT, currentNode, neighbNode);
#ifdef DEBUG
printf("  addson: %d %d\n", currentNode, neighbNode);
#endif
	  }
          currentSubtree = TarjanLink(T1, neighbSubtree, currentSubtree);
          SubtreeRoot[currentSubtree] = currentNode;
	} // endif
      } // if ((q != -1) && (F[q] <= F[p]))
    } // for (k = 0; k < 8; k += incr_vois)
  } // for (i = 0; i < N; i++) 

  CT->root = SubtreeRoot[TarjanFind(T1, TarjanFind(T2, 0))];
  for (i = 0; i < N; i++) CM[i] = TarjanFind(T2, i);
  for (i = 0; i < N; i++) // construction de la relation "father"
    if (CT->tabnodes[i].nbsons > 0)
      for (sc = CT->tabnodes[i].sonlist; sc != NULL; sc = sc->next)
      {
        son = sc->son;
        CT->tabnodes[son].father = i;
      }

  k = 0;
  for (i = 0; i < N; i++)
    if (CT->tabnodes[i].nbsons == 0)
      k++;
  CT->nbleafs = k;
#ifdef VERBOSE
  printf("nombre de feuilles = %d\n", CT->nbleafs);
#endif

  TarjanTermine(T1);
  TarjanTermine(T2);

#ifdef ATTRIB_AREA
{
  int32_t *area = (int32_t *)malloc(N * sizeof(int32_t));
  if (area == NULL)
  {   fprintf(stderr, "%s() : malloc failed\n", F_NAME);
      return 0;
  }
  ComputeArea(CT, CT->root, area);
  for (i = 0; i < N; i++)
    CT->tabnodes[i].area = area[i];

#ifdef ATTRIB_VOL
  ComputeVol(CT, CT->root, area);

  for (i = 0; i < N; i++)
    CT->tabnodes[i].vol = area[i];
#endif            

  free(area);
}
#endif            

  *CompTree = CT;
  *CompMap = CM;
  free(S);
  free(SubtreeRoot);
  return 1;
} // ComponentTree()

/* ==================================== */
int32_t ComponentTree3d( uint8_t *F, int32_t rs, int32_t ps, int32_t N, int32_t connex, // inputs
                    ctree **CompTree, // output
                    int32_t **CompMap     // output
                  )
/* ==================================== */
#undef F_NAME
#define F_NAME "ComponentTree3d"
{
  ctree *CT; 
  int32_t *CM; 
  int32_t *S; 
  int32_t i, k, p, q, tmp; 
  int32_t numbernodes = N; 
  int32_t *SubtreeRoot;
  int32_t currentSubtree;
  int32_t currentNode;
  int32_t neighbSubtree;
  int32_t neighbNode;
  Tarjan *T1, *T2;
  soncell *sc; int32_t son;

  CT = ComponentTreeAlloc(N);
  if (CT == NULL)
  {   fprintf(stderr, "%s() : ComponentTreeAlloc failed\n", F_NAME);
      return 0;
  }
  CM = (int32_t *)calloc(1,N * sizeof(int32_t));
  if (CM == NULL)
  {   fprintf(stderr, "%s() : malloc failed for CM\n", F_NAME);
      return 0;
  }
  SubtreeRoot = (int32_t *)calloc(1,N * sizeof(int32_t));
  if (SubtreeRoot == NULL)
  {   fprintf(stderr, "%s() : malloc failed for SubtreeRoot\n", F_NAME);
      return 0;
  }
#ifdef MAXTREE
  S = linsortimagedown(F, N);
#else
  S = linsortimageup(F, N);
#endif
  if (S == NULL)
  {   fprintf(stderr, "%s() : linsortimage failed\n", F_NAME);
      return 0;
  }
  T1 = CreeTarjan(N);
  T2 = CreeTarjan(N);
  if ((T1 == NULL) || (T2 == NULL))
  {   fprintf(stderr, "%s() : CreeTarjan failed\n", F_NAME);
      return 0;
  }

  for (i = 0; i < N; i++) 
  {
    TarjanMakeSet(T1, i);
    TarjanMakeSet(T2, i);
    SubtreeRoot[i] = i;
    CT->tabnodes[i].nbsons = 0;
    CT->tabnodes[i].data = F[i];
    CT->tabnodes[i].father = -1;
#ifdef ATTRIB_AREA
    CT->tabnodes[i].area = 1;
#endif
  }

  if (connex == 6)
  {
    for (i = 0; i < N; i++) 
    {
      p = S[i];  // pixel courant, dans l'ordre croissant
      currentSubtree = TarjanFind(T1, p);
      currentNode = TarjanFind(T2, SubtreeRoot[currentSubtree]);
      for (k = 0; k <= 10; k += 2)
      {
	q = voisin6(p, k, rs, ps, N);
#ifdef MAXTREE
	if ((q != -1) && (F[q] >= F[p]))
#else
	  if ((q != -1) && (F[q] <= F[p]))
#endif
        {
	  neighbSubtree = TarjanFind(T1, q);
	  neighbNode = TarjanFind(T2, SubtreeRoot[neighbSubtree]);
	  if (currentNode != neighbNode)
	  {
	    if (CT->tabnodes[currentNode].data == CT->tabnodes[neighbNode].data)
            { // merge the nodes
	      numbernodes -= 1;
	      tmp = TarjanLink(T2, neighbNode, currentNode);
#ifdef PARANO
	      if ((tmp != currentNode) && (tmp != neighbNode)) 
		printf("%s : choix inattendu pour le representant", F_NAME);
#endif            
	      if (tmp == currentNode) mergenodes(CT, currentNode, neighbNode);
	      else                    mergenodes(CT, neighbNode, currentNode);
#ifdef ATTRIB_AREA
	      CT->tabnodes[tmp].area = CT->tabnodes[currentNode].area + CT->tabnodes[neighbNode].area;
#endif                        
	      currentNode = tmp;
	    }
	    else
            { // add neighbNode as a new son of currentNode
	      addson(CT, currentNode, neighbNode);
	    }
	    currentSubtree = TarjanLink(T1, neighbSubtree, currentSubtree);
	    SubtreeRoot[currentSubtree] = currentNode;
	  } // endif
	} // if ((q != -1) && (F[q] <= F[p]))
      } // for (k = 0; k < 8; k += incr_vois)
    } // for (i = 0; i < N; i++) 
  } // if (connex == 6)
  else if (connex == 18)
  {
    for (i = 0; i < N; i++) 
    {
      p = S[i];  // pixel courant, dans l'ordre croissant
      currentSubtree = TarjanFind(T1, p);
      currentNode = TarjanFind(T2, SubtreeRoot[currentSubtree]);
      for (k = 0; k < 18; k++)
      {
	q = voisin18(p, k, rs, ps, N);
#ifdef MAXTREE
	if ((q != -1) && (F[q] >= F[p]))
#else
	  if ((q != -1) && (F[q] <= F[p]))
#endif
        {
	  neighbSubtree = TarjanFind(T1, q);
	  neighbNode = TarjanFind(T2, SubtreeRoot[neighbSubtree]);
	  if (currentNode != neighbNode)
	  {
	    if (CT->tabnodes[currentNode].data == CT->tabnodes[neighbNode].data)
            { // merge the nodes
	      numbernodes -= 1;
	      tmp = TarjanLink(T2, neighbNode, currentNode);
#ifdef PARANO
	      if ((tmp != currentNode) && (tmp != neighbNode)) 
		printf("%s : choix inattendu pour le representant", F_NAME);
#endif            
	      if (tmp == currentNode) mergenodes(CT, currentNode, neighbNode);
	      else                    mergenodes(CT, neighbNode, currentNode);
#ifdef ATTRIB_AREA
	      CT->tabnodes[tmp].area = CT->tabnodes[currentNode].area + CT->tabnodes[neighbNode].area;
#endif                                    
	      currentNode = tmp;
	    }
	    else
            { // add neighbNode as a new son of currentNode
	      addson(CT, currentNode, neighbNode);
	    }
	    currentSubtree = TarjanLink(T1, neighbSubtree, currentSubtree);
	    SubtreeRoot[currentSubtree] = currentNode;
	  } // endif
	} // if ((q != -1) && (F[q] <= F[p]))
      } // for (k = 0; k < 8; k += incr_vois)
    } // for (i = 0; i < N; i++) 
  } // if (connex == 18)
  else if (connex == 26)
  {
    for (i = 0; i < N; i++) 
    {
      p = S[i];  // pixel courant, dans l'ordre croissant
      currentSubtree = TarjanFind(T1, p);
      currentNode = TarjanFind(T2, SubtreeRoot[currentSubtree]);
      for (k = 0; k < 26; k++)
      {
	q = voisin26(p, k, rs, ps, N);
#ifdef MAXTREE
	if ((q != -1) && (F[q] >= F[p]))
#else
	  if ((q != -1) && (F[q] <= F[p]))
#endif
        {
	  neighbSubtree = TarjanFind(T1, q);
	  neighbNode = TarjanFind(T2, SubtreeRoot[neighbSubtree]);
	  if (currentNode != neighbNode)
	  {
	    if (CT->tabnodes[currentNode].data == CT->tabnodes[neighbNode].data)
            { // merge the nodes
	      numbernodes -= 1;
	      tmp = TarjanLink(T2, neighbNode, currentNode);
#ifdef PARANO
	      if ((tmp != currentNode) && (tmp != neighbNode)) 
		printf("%s : choix inattendu pour le representant", F_NAME);
#endif            
	      if (tmp == currentNode) mergenodes(CT, currentNode, neighbNode);
	      else                    mergenodes(CT, neighbNode, currentNode);
#ifdef ATTRIB_AREA
	      CT->tabnodes[tmp].area = CT->tabnodes[currentNode].area + CT->tabnodes[neighbNode].area;
#endif            
	      currentNode = tmp;
	    }
	    else
            { // add neighbNode as a new son of currentNode
	      addson(CT, currentNode, neighbNode);
	    }
	    currentSubtree = TarjanLink(T1, neighbSubtree, currentSubtree);
	    SubtreeRoot[currentSubtree] = currentNode;
	  } // endif
	} // if ((q != -1) && (F[q] <= F[p]))
      } // for (k = 0; k < 8; k += incr_vois)
    } // for (i = 0; i < N; i++) 
  } // if (connex == 26)
  else
  {   fprintf(stderr, "%s() : bad value for connex : %d\n", F_NAME, connex);
      return 0;
  }

  CT->root = SubtreeRoot[TarjanFind(T1, TarjanFind(T2, 0))];
  for (i = 0; i < N; i++) CM[i] = TarjanFind(T2, i);
  for (i = 0; i < N; i++) // construction de la relation "father"
    if (CT->tabnodes[i].nbsons > 0)
      for (sc = CT->tabnodes[i].sonlist; sc != NULL; sc = sc->next)
      {
        son = sc->son;
        CT->tabnodes[son].father = i;
      }

  k = 0;
  for (i = 0; i < N; i++)
    if (CT->tabnodes[i].nbsons == 0)
      k++;
  CT->nbleafs = k;
#ifdef VERBOSE
  printf("nombre de feuilles = %d\n", CT->nbleafs);
#endif

  TarjanTermine(T1);
  TarjanTermine(T2);

#ifdef ATTRIB_AREA
{
  int32_t *area = (int32_t *)malloc(N * sizeof(int32_t));
  if (area == NULL)
  {   fprintf(stderr, "%s() : malloc failed\n", F_NAME);
      return 0;
  }
  ComputeArea(CT, CT->root, area);
  for (i = 0; i < N; i++)
    CT->tabnodes[i].area = area[i];

#ifdef ATTRIB_VOL
  ComputeVol(CT, CT->root, area);
  for (i = 0; i < N; i++)
    CT->tabnodes[i].vol = area[i];
#endif            

  free(area);
}
#endif            

  *CompTree = CT;
  *CompMap = CM;
  free(S);
  free(SubtreeRoot);
  return 1;
} // ComponentTree3d()

#ifdef TESTSORT
int32_t main() {
  uint8_t F[20] = {25, 10, 1, 1, 0, 0, 1, 3, 5, 3, 2, 3, 4, 5, 7, 10, 11, 6, 3, 3};
  int32_t i;
  int32_t *T = linsortimagedown((uint8_t *)F, 20);
  for (i = 0; i < 20; i++) printf("F[T[%d]] = %d\n", i, F[T[i]]);
  free(T);
  T = linsortimageup((uint8_t *)F, 20);
  for (i = 0; i < 20; i++) printf("F[T[%d]] = %d\n", i, F[T[i]]);
  free(T);
}
#endif

#ifdef TESTCOMPTREE
int32_t main() {
/*
  uint8_t F[15] = {
    110,  90, 100,  10,  40, 
     50,  50,  50,  10,  20, 
     80,  60,  70,  10,  30
  }; 
  int32_t i, rs = 5, cs = 3;
  uint8_t F[60] = {
    110, 110,  90,  90, 100, 100,  10,  10,  40,  40, 
    110, 110,  90,  90, 100, 100,  10,  10,  40,  40, 
     50,  50,  50,  50,  50,  50,  10,  10,  20,  20, 
     50,  50,  50,  50,  50,  50,  10,  10,  20,  20, 
     80,  80,  60,  60,  70,  70,  10,  10,  30,  30,
     80,  80,  60,  60,  70,  70,  10,  10,  30,  30
  }; 
  int32_t i, rs = 10, cs = 6;
  uint8_t F[60] = {
    110, 110,  90,  90, 100, 100,  75,  75, 140, 140, 
    110, 110,  90,  90, 100, 100,  75,  75, 140, 140, 
     60,  60,  50,  50,  50,  50,  50,  50,  85,  85, 
     60,  60,  50,  50,  50,  50,  50,  50,  85,  85, 
     80,  80,  60,  60,  70,  70,  55,  55, 130, 130,
     80,  80,  60,  60,  70,  70,  55,  55, 130, 130
  }; 
  int32_t i, rs = 10, cs = 6;
*/
  uint8_t F[15] = {
    110,  90, 100,
     50,  50,  50,
     40,  20,  50,
     50,  50,  50,
    120,  70,  80
  }; 
  int32_t i, rs = 3, cs = 5;
  ctree *CT;
  int32_t *CM;

  ComponentTree(F, rs, rs*cs, 4, &CT, &CM);
  printf("component tree:\n");
  mccomptree_ComponentTreePrint(CT);
  printf("component mapping:\n");
  for (i = 0; i < rs*cs; i++)
  {
    if (i % rs == 0) printf("\n");
    printf("%3d ", CM[i]);
  } /* for i */
  printf("\n");

  ComponentTreeFree(CT);
  free(CM);
}
#endif
