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
/* ============== */
/* prototypes for jcimage.c    */
/* ============== */

// ! jcimage s'ajoute mcimage!
// Les deux deux fichiers doivent etre utilisés conjointement

extern struct xvimage4D *allocimage4D(int32_t ss);
extern struct xvimage *allocGAimage(char * name, int32_t rs, int32_t cs, int32_t d, int32_t t);
extern void writerawGAimage(struct xvimage * image, char *filename);
extern struct xvimage * readGAimage(char *filename);
extern struct xvimage4D *readimage4D(char *prefix, int32_t first, int32_t last);     
extern void freeimage4D(struct xvimage4D  *im);
extern struct GA4d * readGA4d(char *filename);
extern struct GA4d * allocGA4d(char * name, int32_t rs, int32_t cs, int32_t d, int32_t ss);
extern void writeGA4d(struct GA4d * image, char *filename);
extern void freeGA4d(struct GA4d  *im);
void writeimage4D(struct xvimage4D * image, char *prefix, int32_t first, int32_t last);
#ifdef __cplusplus
}
#endif
