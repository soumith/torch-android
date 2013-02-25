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
/* ============================================================================== */
/* 
  Structure de donnees pour la construction de l'arbre des composantes.

  Les sommets de cet arbre representent les composantes des coupes de F,  
  a l'exception de celles qui sont egales a une composante d'un niveau inferieur.
  Il y a donc moins de N sommets (N = nombre de pixels) et de N-1 arcs.

  Une composante (sommet) est representee par une structure ctreenode.
*/
/* ============================================================================== */

#define ATTRIB_AREA
#define ATTRIB_VOL

typedef struct soncell   // cell structure for the lists of sons
{
  int32_t son;               // index of the son in table tabnodes [struct ctree]
  struct soncell *next;  // points to next cell
} soncell;

typedef struct           // structure for one node in the component tree
{
  uint8_t data;    // node's level
  int32_t father;            // index of the father node. value -1 indicates the root
  int32_t nbsons;            // number or sons. value -1 indicates a deleted node
#ifdef ATTRIB_AREA
  int32_t area;              // number of pixels in the component
#endif
#ifdef ATTRIB_VOL
  int32_t vol;               // volume of the component
#endif
  soncell *sonlist;      // list of sons (points to the first son cell)
  soncell *lastson;      // direct pointer to the last son cell
} ctreenode;

typedef struct           // structure for a component tree
{
  int32_t nbnodes;           // total number of nodes
  int32_t nbleafs;           // total number of leafs
  int32_t nbsoncells;        // number of avaliable son cells
  int32_t root;              // index of the root node in table tabnodes
  ctreenode * tabnodes;  // table which contains all the nodes
  soncell * tabsoncells; // table which contains all the son cells
  uint8_t *flags;  // each flag is associated to the node with the same index
} ctree;

/* ==================================== */
/* PROTOTYPES */
/* ==================================== */
#define IMGCHAR
//#define IMGLONG

#ifdef IMGCHAR
#define MAXGREY 256
#else
#define MAXGREY 65536
#endif

extern ctree * ComponentTreeAlloc(int32_t N);
extern void ComponentTreeFree(ctree * CT);
#ifdef IMGCHAR
extern int32_t ComponentTree( uint8_t *F, int32_t rs, int32_t N, int32_t connex, // inputs
#endif
#ifdef IMGLONG
extern int32_t ComponentTree( uint32_t *F, int32_t rs, int32_t N, int32_t connex, // inputs
#endif
                           ctree **CompTree, // output
                           int32_t **CompMap     // output
			 );
#ifdef IMGCHAR
extern int32_t ComponentTree3d( uint8_t *F, int32_t rs, int32_t ps, int32_t N, int32_t connex, // inputs
#endif
#ifdef IMGLONG
extern int32_t ComponentTree3d( uint32_t *F, int32_t rs, int32_t ps, int32_t N, int32_t connex, // inputs
#endif
                           ctree **CompTree, // output
                           int32_t **CompMap     // output
			 );
#ifdef __cplusplus
}
#endif
