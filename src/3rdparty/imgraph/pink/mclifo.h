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
#ifndef MC_LIFO
#define MC_LIFO

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _MCIMAGE_H
#include <mcimage.h>
#endif

typedef struct {
  index_t Max;          /* taille max de la Lifo */
  index_t Sp;           /* index de pile (pointe la 1ere case libre) */
  index_t Pts[1];
} Lifo;

/* ============== */
/* prototypes     */
/* ============== */
extern Lifo * CreeLifoVide(index_t taillemax);
extern void LifoFlush(Lifo * L);
extern index_t LifoVide(Lifo * L);
extern index_t LifoPop(Lifo * L);
extern index_t LifoHead(Lifo * L);
extern void LifoPush(Lifo * L, index_t V);
extern void LifoPrint(Lifo * L);
extern void LifoPrintLine(Lifo * L);
extern void LifoTermine(Lifo * L);
#ifdef __cplusplus
}
#endif
#endif
