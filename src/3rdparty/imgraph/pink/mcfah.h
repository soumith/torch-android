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

#ifndef _MCIMAGE_H
#include <mcimage.h>
#endif

typedef struct FAHELT {
  index_t Point;
  struct FAHELT * Next;
} FahElt;

#ifndef NPRIO
#define NPRIO 512
#endif

typedef struct {
  index_t Max;             /* taille max de la fah (en nombre de points) */
  int32_t Niv;             /* niveau a partir duquel des listes existent */
  index_t Util;            /* nombre de points courant dans la fah */
  index_t Maxutil;         /* nombre de points utilises max (au cours du temps) */
  FahElt *Tete[NPRIO]; /* tableau des tetes de liste */
  FahElt *Queue;       /* queue de liste (la ou l'on preleve) */
  FahElt *QueueUrg;    /* queue de liste d'urgence (la ou l'on preleve) */
  FahElt *TeteUrg;     /* tete de liste d'urgence (la ou l'on insere) */
  FahElt *Libre;       /* pile des cellules libres */
  FahElt Elts[1];      /* tableau des elements physiques */
} Fah;

/* ============== */
/* prototypes     */
/* ============== */

extern Fah * CreeFahVide(
  index_t taillemax
);

extern void FahFlush(
  Fah * L
);

extern int32_t FahVide(
  Fah * L
);

extern int32_t FahVideUrg(
  Fah * L
);

extern int32_t FahVideNiveau(
  Fah * L,
  int32_t niv
);

extern int32_t FahNiveau(
  Fah * L
);

extern index_t FahPop(
  Fah * L
);

extern index_t FahFirst(
  Fah * L
);

extern void FahPush(
  Fah * L,
  index_t Po,
  int32_t Ni
);

extern void FahTermine(
  Fah * L
);

extern void FahPrint(
  Fah * L
);
#ifdef __cplusplus
}
#endif
