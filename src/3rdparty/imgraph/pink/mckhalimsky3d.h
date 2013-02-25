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

#define CUBE3D(i,j,k)  ((i%2)+(j%2)+(k%2)==3)
#define CARRE3D(i,j,k) ((i%2)+(j%2)+(k%2)==2)
#define CARRE3DXY(i,j,k) ((i%2==1)&&(j%2==1)&&(k%2==0))
#define CARRE3DYZ(i,j,k) ((i%2==0)&&(j%2==1)&&(k%2==1))
#define CARRE3DXZ(i,j,k) ((i%2==1)&&(j%2==0)&&(k%2==1))
#define INTER3D(i,j,k) ((i%2)+(j%2)+(k%2)==1)
#define INTER3DX(i,j,k) ((i%2==1)&&(j%2==0)&&(k%2==0))
#define INTER3DY(i,j,k) ((i%2==0)&&(j%2==1)&&(k%2==0))
#define INTER3DZ(i,j,k) ((i%2==0)&&(j%2==0)&&(k%2==1))
#define SINGL3D(i,j,k) ((i%2)+(j%2)+(k%2)==0)
#define CX3D(i,j,k) ((i%2==0)&&(j%2)&&(k%2))
#define CY3D(i,j,k) ((i%2)&&(j%2==0)&&(k%2))
#define CZ3D(i,j,k) ((i%2)&&(j%2)&&(k%2==0))
#define IX3D(i,j,k) ((i%2)&&(j%2==0)&&(k%2==0))
#define IY3D(i,j,k) ((i%2==0)&&(j%2)&&(k%2==0))
#define IZ3D(i,j,k) ((i%2==0)&&(j%2==0)&&(k%2))
#define DIM3D(i,j,k) ((i%2)+(j%2)+(k%2))

#define NDG_CUBE3D  255
#define NDG_CARRE3D 200
#define NDG_CARRE3D 200
#define NDG_CARRE3DXY 201
#define NDG_CARRE3DYZ 202
#define NDG_CARRE3DXZ 203
#define NDG_INTER3D 128
#define NDG_INTER3DX 129
#define NDG_INTER3DY 130
#define NDG_INTER3DZ 131
#define NDG_SINGL3D 60
#define GRS3D 3
#define GCS3D 3
#define GDS3D 3

#define VAL_NULLE    0
#define VAL_OBJET  255
#define VAL_MARQUE   1

extern void InitPileGrilles3d();
extern void TerminePileGrilles3d();
extern struct xvimage * AllocGrille3d();
extern void FreeGrille3d();
extern struct xvimage * Khalimskize3d(struct xvimage *o);
extern struct xvimage * KhalimskizeNDG3d(struct xvimage *o);
extern void ndgmin3d(struct xvimage *b);
extern void ndgmax3d(struct xvimage *b);
extern void ndgmoy3d(struct xvimage *k);
//extern void DeKhalimskize3d(struct xvimage *k, struct xvimage *r);
extern struct xvimage * DeKhalimskize3d(struct xvimage *o);
extern void Dual3d(struct xvimage *bd, struct xvimage *b);
extern void InitKhalimsky3d(struct xvimage *b, uint32_t c);
extern void Connex26Obj3d(struct xvimage *o);
extern void Connex6Obj3d(struct xvimage *o);
extern void Alphacarre3d(index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k, index_t *tab, int32_t *n);
extern void Betacarre3d(index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k, index_t *tab, int32_t *n);
extern void Thetacarre3d(index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k, index_t *tab, int32_t *n);
extern uint32_t XAlphacarre3d(uint8_t *X, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k);
extern uint32_t XBetacarre3d(uint8_t *X, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k);
extern uint32_t XhAlphacarre3d(uint8_t *X, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k, int32_t h);
extern uint32_t XhBetacarre3d(uint8_t *X, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k, int32_t h);
extern void SetXAlphacarre3d(uint32_t b, uint8_t *X, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k);
extern void SetXBetacarre3d(uint32_t b, uint8_t *X, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k);
extern int32_t Precede3d(index_t i, index_t j, index_t k, index_t x, index_t y, index_t z);
extern int32_t NbPred3d(uint8_t *K, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k);
extern int32_t Succede3d(index_t i, index_t j, index_t k, index_t x, index_t y, index_t z);
extern int32_t NbSucc3d(uint8_t *B, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k);
extern int32_t BetaTerminal3d(uint8_t *K, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k);
extern int32_t AlphaTerminal3d(uint8_t *B, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k);
extern int32_t ExactementUnBetaTerminal3d(uint8_t *K, index_t rs, index_t cs, index_t ds);
extern void SatureAlphacarre3d(struct xvimage *k);
extern void AjouteAlphacarre3d(struct xvimage *k);
extern void AjouteBetacarre3d(struct xvimage *k);
extern void MaxAlpha3d(struct xvimage *k);
extern void MaxBeta3d(struct xvimage *k);
extern void ColorieKh3d(struct xvimage *k);
extern void EffaceLiensAlphaLibres3d(struct xvimage *k);
extern void EffaceLiensBetaLibres3d(struct xvimage *k);
extern void EffaceLiensBiLibres3d(struct xvimage *k);
extern void EffaceLiensBiLibresNonMarques3d(struct xvimage *k);
extern void CopieAlphacarre3d(uint8_t *G, uint8_t *K, index_t rs, index_t cs, index_t ds,
                               index_t i, index_t j, index_t k);
extern void CopieThetacarre3d(uint8_t *G, uint8_t *B, 
                        index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k);
extern void CopieBetacarreDual3d(uint8_t *G, uint8_t *B, 
                           index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k);
extern void MarqueThetacarre3d(uint8_t *B, 
                        index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k);
extern int32_t CardThetacarre3d(uint8_t *B, 
                        index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k);
extern int32_t CardThetaCarre3d(uint8_t *B, 
                        index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k,
			uint8_t v);
extern index_t EffaceBetaTerminauxSimples3d(struct xvimage *k);
extern int32_t EnsembleSimple3d(struct xvimage *k);
extern int32_t BetaTerminalSimple3d(struct xvimage *b, index_t i, index_t j, index_t k);
extern int32_t Connexe3d(struct xvimage *b);
extern int32_t Connexe3dVal(struct xvimage *b, uint8_t Val);
extern int32_t Prop0CycleBetaTerminal3d(uint8_t *B, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k);
extern int32_t Prop0CycleAlphaTerminal3d(uint8_t *B, index_t rs, index_t cs, index_t ds, index_t i, index_t j, index_t k);
extern int32_t CourbeSimple3d(struct xvimage *b);
extern int32_t Arbre3d(struct xvimage *b);
extern int32_t Alpha3Simple3d(struct xvimage *b, index_t i, index_t j, index_t k);
extern int32_t Beta3Simple3d(struct xvimage *b, index_t i, index_t j, index_t k);
extern int32_t NbCompConnexe3d(struct xvimage *b);
extern int32_t T3d(struct xvimage *b, index_t i, index_t j, index_t k, struct xvimage *g);
extern int32_t Tbar3d(struct xvimage *b, index_t i, index_t j, index_t k, struct xvimage *g);
extern int32_t Surfend3d(struct xvimage *b, index_t i, index_t j, index_t k);
extern index_t EulerKh3d(struct xvimage *b);
extern index_t EulerKh3dVal(struct xvimage *b, uint8_t Val);
extern int32_t FaceLibre3d(struct xvimage *b, index_t i, index_t j, index_t k);
extern int32_t PaireLibre3d(struct xvimage *b, index_t i, index_t j, index_t k);

extern int32_t Collapse3d(struct xvimage *b, index_t i, index_t j, index_t k);
extern int32_t simple_26_att(uint8_t *img, index_t p, index_t rs, index_t ps, index_t N);
extern int32_t pairesimple_26_x(uint8_t *img, index_t p, index_t rs, index_t ps, index_t N);
extern int32_t pairesimple_26_y(uint8_t *img, index_t p, index_t rs, index_t ps, index_t N);
extern int32_t pairesimple_26_z(uint8_t *img, index_t p, index_t rs, index_t ps, index_t N);
#ifdef __cplusplus
}
#endif
