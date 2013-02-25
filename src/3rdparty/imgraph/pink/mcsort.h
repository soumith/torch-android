
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

/* authors : J. Cousty - L. Najman and M. Couprie */

#ifdef __cplusplus
    extern "C" {
#endif


/* $Id: mcsort.h,v 1.3 2006/02/28 07:49:12 michel Exp $ */
#include <stdint.h>

extern int32_t Partitionner(int32_t *A, int32_t p, int32_t r);
extern int32_t PartitionStochastique(int32_t *A, int32_t p, int32_t r);
extern void TriRapide (int32_t * A, int32_t p, int32_t r);
extern void TriRapideStochastique (int32_t * A, int32_t p, int32_t r);
extern int32_t SelectionStochastique (int32_t * A, int32_t p, int32_t r, int32_t i);
extern int32_t ElimineDupliques(int32_t *A, int32_t n);
extern void d_TriRapideStochastique(int32_t * A, double *T, int32_t p, int32_t r);

#ifdef __cplusplus
    }
#endif