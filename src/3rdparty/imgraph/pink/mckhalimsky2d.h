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

#define CARRE(i,j) ((i%2)+(j%2)==2)
#define INTER(i,j) ((i%2)+(j%2)==1)
#define INTERH(i,j) ((i%2)&&(!(j%2)))
#define INTERV(i,j) ((!(i%2))&&(j%2))
#define SINGL(i,j) ((i%2)+(j%2)==0)
#define DIM2D(i,j) ((i%2)+(j%2))
#define NDG_CARRE 255
#define NDG_INTER 200
#define NDG_SINGL 128
#define GRS2D 3
#define GCS2D 3

#define VAL_NULLE    0
#define VAL_OBJET  255
#define VAL_MARQUE   1

extern void InitPileGrilles2d();
extern void TerminePileGrilles2d();
extern struct xvimage * Khalimskize2d(struct xvimage *o);
extern struct xvimage * KhalimskizeNDG2d(struct xvimage *o);
extern struct xvimage * DeKhalimskize2d(struct xvimage *o);
extern void Khalimskize2d_noalloc(struct xvimage *o, struct xvimage *k);
extern void KhalimskizeNDG2d_noalloc(struct xvimage *o, struct xvimage *k);
extern void DeKhalimskize2d_noalloc(struct xvimage *o, struct xvimage *r);
extern void ndgmin2d(struct xvimage *k);
extern void ndgminbeta2d(struct xvimage *k);
extern void ndgmax2d(struct xvimage *k);
extern void ndgmaxbeta2d(struct xvimage *k);
extern void ndgmoy2d(struct xvimage *k);
extern void ndg2grad2d(struct xvimage *k);
extern void ndg4grad2d(struct xvimage *k);
extern void Connex8Obj2d(struct xvimage *o);
extern void Connex4Obj2d(struct xvimage *o);
extern void Betapoint2d(index_t rs, index_t cs, index_t i, index_t j, index_t *tab, int32_t *n);
extern void Alphapoint2d(index_t rs, index_t cs, index_t i, index_t j, index_t *tab, int32_t *n);
extern void Betacarre2d(index_t rs, index_t cs, index_t i, index_t j, index_t *tab, int32_t *n);
extern void Alphacarre2d(index_t rs, index_t cs, index_t i, index_t j, index_t *tab, int32_t *n);
extern void Thetacarre2d(index_t rs, index_t cs, index_t i, index_t j, index_t *tab, int32_t *n);
extern int32_t CardBetapoint2d(uint8_t *K, index_t rs, index_t cs, index_t i, index_t j);
extern int32_t CardThetaCarre2d(struct xvimage *k, index_t i, index_t j, uint8_t val);
extern int32_t BetaTerminal2d(uint8_t *K, index_t rs, index_t cs, index_t i, index_t j);
extern int32_t ExactementUnBetaTerminal2d(uint8_t *K, index_t rs, index_t cs);
extern void SatureAlphacarre2d(struct xvimage *k);
extern void AjouteAlphacarre2d(struct xvimage *k);
extern void AjouteBetacarre2d(struct xvimage *k);
extern void MaxAlpha2d(struct xvimage *k);
extern void MaxBeta2d(struct xvimage *k);
extern void ColorieKh2d(struct xvimage *k);
extern void EffaceLiensLibres2d(struct xvimage *k);
extern void CopieAlphacarre2d(uint8_t *G,uint8_t *K,index_t rs,index_t cs,index_t i,index_t j);
extern index_t EffaceBetaTerminauxSimples2d(struct xvimage *k);
extern int32_t EnsembleSimple2d(struct xvimage *k);
extern int32_t Ensemble2Contractile2d(struct xvimage *b);
extern void Htkern2d(struct xvimage *b, int32_t n);
extern int32_t AlphaSimple2d(struct xvimage *b, index_t i, index_t j);
extern int32_t BetaSimple2d(struct xvimage *b, index_t i, index_t j);
extern int32_t Alpha2Simple2d(struct xvimage *b, index_t i, index_t j);
extern int32_t Beta2Simple2d(struct xvimage *b, index_t i, index_t j);
extern index_t EulerKh2d(struct xvimage *b);
extern int32_t FaceLibre2d(struct xvimage *b, index_t i, index_t j);
extern int32_t PaireLibre2d(struct xvimage *b, index_t i, index_t j);
extern int32_t Collapse2d(struct xvimage *b, index_t i, index_t j);
#ifdef __cplusplus
}
#endif
