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

#define max(a,b) error_max_function_is_ambigous_use_mcmax_instead
#define min(a,b) error_min_function_is_ambigous_use_mcmin_instead
#define abs(b) error_abs_function_is_ambigous_use_mcabs_instead
#define sign(b) error_sign_function_is_ambigous_use_mcsign_instead
#define odd(b) error_odd_function_is_ambigous_use_mcodd_instead
#define even(b) error_even_function_is_ambigous_use_mceven_instead
#define sqr(b) error_sqr_function_is_ambigous_use_mcsqr_instead
  
#define mcabs(X) ((X)>=0?(X):-(X))
#define mcmax(X,Y) ((X)>=(Y)?(X):(Y))
#define mcmin(X,Y) ((X)<=(Y)?(X):(Y))
#define mcodd(X) ((X)&1)
#define mceven(X) (((X)&1)==0)
#define arrondi(z) (((z)-(double)((int32_t)(z)))<=0.5?((int32_t)(z)):((int32_t)(z+1)))
#define signe(z) (((z)>0.0)?1.0:-1.0)
#define mcsqr(x) ((x)*(x))

#ifndef M_PI
# define M_E		2.7182818284590452354	/* e */
# define M_LOG2E	1.4426950408889634074	/* log_2 e */
# define M_LOG10E	0.43429448190325182765	/* log_10 e */
# define M_LN2		0.69314718055994530942	/* log_e 2 */
# define M_LN10		2.30258509299404568402	/* log_e 10 */
# define M_PI		3.14159265358979323846	/* pi */
# define M_PI_2		1.57079632679489661923	/* pi/2 */
# define M_PI_4		0.78539816339744830962	/* pi/4 */
# define M_1_PI		0.31830988618379067154	/* 1/pi */
# define M_2_PI		0.63661977236758134308	/* 2/pi */
# define M_2_SQRTPI	1.12837916709551257390	/* 2/sqrt(pi) */
# define M_SQRT2	1.41421356237309504880	/* sqrt(2) */
# define M_SQRT1_2	0.70710678118654752440	/* 1/sqrt(2) */
#endif
#ifdef __cplusplus
}
#endif
