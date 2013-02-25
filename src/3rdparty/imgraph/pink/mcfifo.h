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

typedef struct {
  index_t Max;          /* taille max de la fifo */
  index_t In;           /* index ou ranger le prochain arrivant */
  index_t Out;          /* index d'ou retirer le prochain sortant */
  index_t Pts[1];
} Fifo;

/* ============== */
/* prototypes     */
/* ============== */

extern Fifo * CreeFifoVide(index_t taillemax);
extern void FifoFlush(Fifo * L);
extern int32_t FifoVide(Fifo * L);
extern index_t FifoPop(Fifo * L);
extern void FifoPush(Fifo * L, index_t V);
extern void FifoPushFirst(Fifo * L, index_t V);
extern index_t FifoTaille(Fifo * L);
extern void FifoPrint(Fifo * L);
extern void FifoTermine(Fifo * L);



#ifdef __cplusplus
}
#endif
