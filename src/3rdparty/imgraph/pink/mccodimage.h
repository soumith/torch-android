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

#ifndef _PINK_TYPES_
#define _PINK_TYPES_
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#endif

#ifndef _MCIMAGE_H
#include <mcimage.h>
#endif

#define NDG_MAX 255            /* niveau de gris max */
#define NDG_MIN 0              /* niveau de gris min */

typedef struct {float re; float im;} complex;
typedef struct {double re; double im;} dcomplex;

/* definitions for data storage type,
   uint32_t data_storage_type; */
#define	VFF_TYP_BIT		0	/* pixels are on or off (binary image)*/
                                        /* Note: This is an X11 XBitmap 
					   with bits packed into a byte and
					   padded to a byte */
#define	VFF_TYP_1_BYTE		1	/* pixels are byte (uint8_t) */
#define	VFF_TYP_2_BYTE		2	/* pixels are two byte (int16_t) */
#define	VFF_TYP_4_BYTE		4	/* pixels are four byte (integer) */
#define	VFF_TYP_FLOAT		5	/* pixels are float (single precision)*/
#define VFF_TYP_DOUBLE		6	/* pixels are float (double precision)*/
#define VFF_TYP_COMPLEX		7	/* pixels are complex (single precision)*/
#define VFF_TYP_DCOMPLEX	8	/* pixels are complex (double precision)*/

struct xvimage {
  char *name;                          /* dummy - not used anymore */
  index_t row_size;                    /* Size of a row (number of columns) */
  index_t col_size;                    /* Size of a column (number of rows) */
  index_t depth_size;                  /* Number of planes (for 3d images) */
  index_t time_size;                   /* Number of (2d or 3d) images */
  index_t num_data_bands;	       /* Number of bands per data pixel,
					   or number of bands per image, or
					   dimension of vector data, or
					   number of elements in a vector */
  int32_t data_storage_type;           /* storage type for disk data */
  double xdim, ydim, zdim;             /* voxel dimensions in real world */
  index_t xmin, xmax;                  /* region of interest: x coordinates */
  index_t ymin, ymax;                  /* region of interest: y coordinates */
  index_t zmin, zmax;                  /* region of interest: z coordinates */
  void * image_data;                   /* pointer on raw data */
};

#define SCHARDATA(I)   ((int8_t*)((I)->image_data))
#define UCHARDATA(I)   ((uint8_t*)((I)->image_data))
#define SSHORTDATA(I)  ((int16_t*)((I)->image_data))
#define USHORTDATA(I)  ((uint16_t*)((I)->image_data))
#define SLONGDATA(I)   ((int32_t*)((I)->image_data))
#define ULONGDATA(I)   ((uint32_t*)((I)->image_data))
#define FLOATDATA(I)   ((float*)((I)->image_data))
#define DOUBLEDATA(I)  ((double*)((I)->image_data))
#define COMPLEXDATA(I) ((complex*)((I)->image_data))
#define DCOMPLEXDATA(I)((dcomplex*)((I)->image_data))
#define colsize(I)     ((I)->col_size)
#define rowsize(I)     ((I)->row_size)
#define depth(I)       ((I)->depth_size)
#define tsize(I)       ((I)->time_size)
#define nbands(I)      ((I)->num_data_bands)
#define datatype(I)    ((I)->data_storage_type)
#define pixel(I,x,y)   (((uint8_t*)((I)->image_data))[(y)*(I)->row_size+(x)])
#define voxel(I,x,y,z) (((uint8_t*)((I)->image_data))[((z)*(I)->col_size+(y))*(I)->row_size+(x)])
#define lpixel(I,x,y)   (((uint32_t*)((I)->image_data))[(y)*(I)->row_size+(x)])
#define lvoxel(I,x,y,z) (((uint32_t*)((I)->image_data))[((z)*(I)->col_size+(y))*(I)->row_size+(x)])

/*            
		Codage du voisinage


		3	2	1			
		4	X	0
		5	6	7
*/
#define EST 0
#define NORD 2
#define OUEST 4
#define SUD 6
#define NORD_EST 1
#define NORD_OUEST 3
#define SUD_OUEST 5
#define SUD_EST 7
#define DEVANT 8
#define DERRIERE 10

#define nonbord(p,rs,N) ((p%rs!=rs-1)&&(p>=rs)&&(p%rs!=0)&&(p<N-rs))
#define nonbord3d(p,rs,ps,N) ((p>=ps)&&(p<N-ps)&&(p%ps>=rs)&&(p%ps<ps-rs)&&(p%rs!=0)&&(p%rs!=rs-1))

/* ============== */
/* prototypes     */
/* ============== */

extern int32_t voisin(index_t i, int32_t k, index_t rs, index_t nb);
extern int32_t voisin2(index_t i, int32_t k, index_t rs, index_t nb);
extern int32_t voisin6(index_t i, int32_t k, index_t rs, index_t n, index_t nb);
extern int32_t bord(index_t i, index_t rs, index_t nb);
extern int32_t bord3d(index_t i, index_t rs, index_t ps, index_t nb);
extern int32_t voisin26(index_t i, int32_t k, index_t rs, index_t n, index_t nb);
extern int32_t voisin18(index_t i, int32_t k, index_t rs, index_t n, index_t nb);
extern int32_t voisins4(index_t i, index_t j, index_t rs);
extern int32_t voisins8(index_t i, index_t j, index_t rs);   
extern int32_t voisins6(index_t i, index_t j, index_t rs, index_t ps);   
extern int32_t voisins18(index_t i, index_t j, index_t rs, index_t ps);   
extern int32_t voisins26(index_t i, index_t j, index_t rs, index_t ps);   
extern int32_t voisin5(index_t i, int32_t k, index_t rs, index_t nb);
extern int32_t voisin6b(index_t i, int32_t k, index_t rs, index_t nb, index_t par);
extern int32_t voisinNESO(index_t i, int32_t k, index_t rs, index_t nb);
extern int32_t voisinNOSE(index_t i, int32_t k, index_t rs, index_t nb);
extern int32_t voisin14b(index_t i, int32_t k, index_t rs, index_t ps, index_t N);
extern int32_t voisinONAV(index_t i, int32_t k, index_t rs, index_t ps, index_t N );
extern int32_t voisinENAR(index_t i, int32_t k, index_t rs, index_t ps, index_t N );
extern int32_t voisinENAV(index_t i, int32_t k, index_t rs, index_t ps, index_t N );
extern int32_t voisinONAR(index_t i, int32_t k, index_t rs, index_t ps, index_t N );
extern uint32_t maskvois26(uint8_t *F, uint32_t bitmask, index_t i, index_t rs, index_t ps, index_t N);
extern int32_t sont4voisins(index_t p, index_t q, index_t rs);
extern int32_t sont8voisins(index_t p, index_t q, index_t rs);
extern int32_t sont6voisins(index_t p, index_t q, index_t rs, index_t ps);
extern int32_t sont18voisins(index_t p, index_t q, index_t rs, index_t ps);
extern int32_t sont26voisins(index_t p, index_t q, index_t rs, index_t ps);
 
#ifdef __cplusplus
}
#endif
