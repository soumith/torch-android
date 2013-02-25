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
#ifdef __cplusplus
extern "C" {
#endif

#ifndef _JCTREESTRUCT_
#define _JCTREESTRUCT_
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


// Manipulation ces graphes binaires
GrapheBasic *initGrapheBasic(int32_t nsom, int32_t nmaxarc);
void termineGrapheBasic(GrapheBasic *g);
uint32_t ajouteGrapheBasicSymArc(GrapheBasic *g, int32_t i, int32_t s);
PBasicCell alloueBasicCell(PBasicCell *plibre);

// Structure pour des graphes a aretes valuees
typedef struct GrapheValue {
  GrapheBasic *g;
  uint8_t *F;
} GrapheValue;

// Manipulations des graphes values
GrapheValue *initGrapheValue(int32_t nsom, int32_t nmaxarc);
void termineGrapheValue(GrapheValue * gv);
// add a weighted edge if necessary, otherwise update the value of the
// edge from i to s
int32_t updateArcValue(GrapheValue *gv, int32_t i, int32_t s, uint8_t val);


// Manipulation des RAGs
extern RAG *initRAG(int32_t nsom, int32_t nmaxarc);
extern void termineRAG(RAG * rag);
extern int32_t updateRAGArc(RAG *rag, int32_t i, int32_t s, uint8_t val);

// Les deux fonctions suivantes vont plutot dans la bibliothèque
// hierarchie
extern void attributNoeud(RAG *rag, struct xvimage *label, struct xvimage *ga, struct xvimage *annexe);

#endif
#ifdef __cplusplus
}
#endif
