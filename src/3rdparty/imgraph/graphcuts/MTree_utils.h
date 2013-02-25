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

/* ============================================================================== */
/* 
  Structure de donnees pour la construction de l'arbre des composantes.

  Les sommets de cet arbre representent les composantes des coupes de F,  
  a l'exception de celles qui sont egales a une composante d'un niveau inferieur.
  Il y a donc moins de N sommets (N = nombre de pixels) et de N-1 arcs.

  Une composante (sommet) est representee par une structure ctreenode.
  +
  arbre des composantes d'un graphe d'adjacence valué
  +
  arbre de fusion d'un graphe d'adjacence valué 
  +
  un arbre de saillance
*/
/* ============================================================================== */
#ifndef _JCTREESTRUCT_
#define _JCTREESTRUCT_

typedef struct JCsoncell
{
  int32_t son;
  struct JCsoncell *next;
} JCsoncell;



typedef struct
{
  uint8_t k;    // node's k
  uint8_t area;    // node's area
  float data_f;    // node's level
  int32_t edge;    // related edge
  uint32_t data;             // node's level
  int32_t father;            // value -1 indicates the root
  int32_t nbsons;            // value -1 indicates a deleted node
  int32_t max, min;
  JCsoncell *sonlist;
  JCsoncell *lastson;
} JCctreenode;

typedef struct
{
  int32_t nbnodes;
  int32_t nbsoncells;
  int32_t root;
  JCctreenode * tabnodes; 
  JCsoncell * tabsoncells;
  uint8_t *flags;
} JCctree;

typedef struct
{
  JCctree *CT;
  int32_t *mergeEdge;
} mtree;
/*

typedef struct
{
  uint8_t data;             // node's level
  int32_t father;            // value -1 indicates the root
  int32_t nbsons;            // value -1 indicates a deleted node
  int32_t max, min;
  JCsoncell *sonlist;
  JCsoncell *lastson;
  } JCctreenode;

typedef struct
{
  int32_t nbnodes;
  int32_t nbsoncells;
  int32_t root;
  JCctreenode * tabnodes; 
  JCsoncell * tabsoncells;
  uint8_t *flags;
  } JCctree;

typedef struct
{
  JCctree *CT;
  int32_t *mergeEdge;
  } mtree;*/

// Structure de Graphe binaire

typedef struct BasicCell {
  // index du sommet extremite de l'arete
  uint32_t vertex;
  // index de l'arete
  uint32_t edge;
  struct BasicCell * next;
} BasicCell;

typedef BasicCell * PBasicCell;

typedef struct GrapheBasic {
  /* informations globales */
  //!  nombre de sommets 
  int32_t nsom;
  //!  nombre maximum d'arcs
  int32_t nmaxarc;
  //!  nombre d'arcs
  int32_t narc; // Ah les hauts fonctionnaires !!
  /* representation par listes chainees de successeurs (application gamma) */
  //!  tableau des cellules en réserve 
  PBasicCell reserve;
  //!  liste des cellules libres gérée en pile lifo 
  PBasicCell libre;
  //!  tableau des listes de successeurs indexé par les sommets 
  PBasicCell *gamma;     
} GrapheBasic; 


// Graphe d'adjacence (graphes values contenant de l'information sur les sommets)
typedef struct RAG{
  GrapheBasic *g;           // la structure binaire
  uint8_t *F;              // valuation des aretes
  uint8_t *profondeur;     // profondeur des regions
  uint32_t *surface;       //
  uint32_t *altitude;       //
  uint32_t *tete;          // representation du graphe
  uint32_t *queue;         // par liste d'aretes
} RAG;


#ifndef _LISTSTRUCT_
#define _LISTSTRUCT_
typedef struct list
{
  int index;
  struct list *next;
} list ;
#endif

void Insert(list **sl, int index);
void PrintList(list *sl);

#endif

