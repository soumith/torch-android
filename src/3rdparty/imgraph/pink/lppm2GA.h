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
extern int32_t lpgm2ga(struct xvimage *im, struct xvimage *ga, int32_t param, double alpha);
extern int32_t lpgm2gafloat(struct xvimage *im, struct xvimage *ga, int32_t param, double alpha);
extern int32_t lpgm2gaDouble(struct xvimage *im, struct xvimage *ga, int32_t param, double alpha);
extern int32_t lpgm2gaDouble(struct xvimage *im, struct xvimage *ga, int32_t param, double alpha);
extern int32_t lppm2ga(struct xvimage *r, struct xvimage *v, struct xvimage *b, struct xvimage *ga, int32_t param);
extern int32_t lpgm2ga3d(struct xvimage *im, struct xvimage *ga, int32_t param);
extern int32_t lpgm2ga4d(struct xvimage4D *im, struct GA4d * ga, int32_t param);
extern int32_t compute_scale(uint8_t **image, uint8_t **scale_image, float *scale_map, int32_t *sphere_no_points, /*int16_t ***sphere_points,*/ int32_t N, int32_t rs, int32_t cs, double * feature_mean, int32_t *feature_thr, int32_t * pow_value);
extern void compute_homogeneitysb(uint8_t ** image, double *feature_mean, uint8_t *x_affinity, uint8_t *y_affinity, uint8_t* scale_image, int32_t *sphere_no_points, /*int16_t ***sphere_points,*/ int32_t *feature_thr, float **homogeneity_map, int32_t N, int32_t rs, int32_t cs, int32_t * pow_value);
#ifdef __cplusplus
}
#endif
