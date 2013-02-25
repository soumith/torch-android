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
/* 
   Librairie mccodimage : 

   fonctions pour l'acces aux voisins d'un point et pour la detection de bord

   Michel Couprie 1996

*/
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcimage.h>
#include <mccodimage.h>
#include <mcutil.h>

/*            
		Codage du voisinage en 2D

		3	2	1			
		4	X	0
		5	6	7

		pour le voisinage étendu

	       14      13      12      11      10
	       15       3	2	1	9	
	       16       4	X	0	8
	       17       5	6	7      23
	       18      19      20      21      22

		Traduction index <-> coordonnées (2D)

		i = y*rs + x

		x = i % rs
		y = i / rs;

		Traduction index <-> coordonnées (3D)

		i = z*ps + y*rs + x;

		x = i % rs;
		y = (i % ps) / rs;
		z = i / ps;
*/

/* ==================================== */
int32_t voisin(index_t i, int32_t k, index_t rs, index_t nb)   
/* i : index du point dans l'image */
/* k : direction du voisin */
/* rs : taille d'une rangee */
/* nb : taille de l'image */
/* retourne -1 si le voisin n'existe pas */
/* ==================================== */
#undef F_NAME
#define F_NAME "voisin"
{
  switch(k)
  {
  case EST:        if (i%rs!=rs-1)              return i+1;    else return -1;
  case NORD_EST:   if ((i%rs!=rs-1)&&(i>=rs))   return i+1-rs; else return -1;
  case NORD:       if (i>=rs)                   return i-rs;   else return -1;
  case NORD_OUEST: if ((i>=rs)&&(i%rs!=0))      return i-rs-1; else return -1;
  case OUEST:      if (i%rs!=0)                 return i-1;    else return -1;
  case SUD_OUEST:  if ((i%rs!=0)&&(i<nb-rs))    return i-1+rs; else return -1;
  case SUD:        if (i<nb-rs)                 return i+rs;   else return -1;
  case SUD_EST:    if ((i<nb-rs)&&(i%rs!=rs-1)) return i+rs+1; else return -1;
  default: 
    fprintf(stderr, "%s: bad index value %d\n", F_NAME, k);
    exit(0);
  }
} // voisin()

/* ==================================== */
int32_t voisin2(index_t i, int32_t k, index_t rs, index_t nb)   
/* i : index du point dans l'image */
/* k : index du voisin (24 possibilités - voisinage étendu) */
/* rs : taille d'une rangee */
/* nb : taille de l'image */
/* retourne -1 si le voisin n'existe pas */
/* ==================================== */
#undef F_NAME
#define F_NAME "voisin2"
{
  switch(k)
  {
  case EST:        if (i%rs!=rs-1)              return i+1;    else return -1;
  case NORD_EST:   if ((i%rs!=rs-1)&&(i>=rs))   return i+1-rs; else return -1;
  case NORD:       if (i>=rs)                   return i-rs;   else return -1;
  case NORD_OUEST: if ((i>=rs)&&(i%rs!=0))      return i-rs-1; else return -1;
  case OUEST:      if (i%rs!=0)                 return i-1;    else return -1;
  case SUD_OUEST:  if ((i%rs!=0)&&(i<nb-rs))    return i-1+rs; else return -1;
  case SUD:        if (i<nb-rs)                 return i+rs;   else return -1;
  case SUD_EST:    if ((i<nb-rs)&&(i%rs!=rs-1)) return i+rs+1; else return -1;
  case  8: if (i%rs<rs-2)                return i+2;      else return -1;
  case  9: if ((i%rs<rs-2)&&(i>=rs))     return i+2-rs;   else return -1;
  case 10: if ((i%rs<rs-2)&&(i>=2*rs))   return i+2-2*rs; else return -1;
  case 11: if ((i%rs<rs-1)&&(i>=2*rs))   return i+1-2*rs; else return -1;
  case 12: if (i>=2*rs)                  return i-2*rs;   else return -1;
  case 13: if ((i%rs>0)&&(i>=2*rs))      return i-1-2*rs; else return -1;
  case 14: if ((i%rs>1)&&(i>=2*rs))      return i-2-2*rs; else return -1;
  case 15: if ((i%rs>1)&&(i>=rs))        return i-2-rs;   else return -1;
  case 16: if (i%rs>1)                   return i-2;      else return -1;
  case 17: if ((i%rs>1)&&(i<nb-rs))      return i-2+rs;   else return -1;
  case 18: if ((i%rs>1)&&(i<nb-2*rs))    return i-2+2*rs; else return -1;
  case 19: if ((i%rs>0)&&(i<nb-2*rs))    return i-1+2*rs; else return -1;
  case 20: if (i<nb-2*rs)                return i+2*rs;   else return -1;
  case 21: if ((i%rs<rs-1)&&(i<nb-2*rs)) return i+1+2*rs; else return -1;
  case 22: if ((i%rs<rs-2)&&(i<nb-2*rs)) return i+2+2*rs; else return -1;
  case 23: if ((i%rs<rs-2)&&(i<nb-rs))   return i+2+rs;   else return -1;
  default: 
    fprintf(stderr, "%s: bad index value %d\n", F_NAME, k);
    exit(0);
  }
} // voisin2()

/* Cette fonction indique a quel bord appartient le point */
/* ==================================== */
int32_t bord(index_t i, index_t rs, index_t nb)
/* ==================================== */
{
	/* valeurs renvoyees :
		4 3 2
		5 0 1
		6 7 8
	*/

 	if (i==0) return (4);
 	if (i==(nb-rs)) return (6);
 	if (i==rs-1) return (2);
 	if (i==nb-1) return (8);
 	if (i%rs==0) return (5);
	if (i%rs==rs-1) return (1);
	if (i<rs) return (3);
	if (i>nb-rs) return (7);
	return (0);
}

/*            
		Codage d'une image en 3D niveaux de gris

                L'image 3d est sockee sous forme de plans (images planes verticales)
                Le premier plan constitue l'avant
                Chaque image a pour dimensions rs (taille rangee) et cs (taille colonne)
                L'image est constituee de d plans, chacun de taille n = rs*cs pixels.

*/

/* Cette fonction indique a quel bord appartient le point */
/* ==================================== */
int32_t bord3d(index_t i, index_t rs, index_t ps, index_t nb)
/* ==================================== */
{
  if (i%rs == rs-1)     return 1;
  if (i%rs == 0)        return 1;
  if ((i%ps) < rs)      return 1;
  if ((i%ps) >= ps-rs)  return 1;
  if (i < ps)           return 1;
  if (i >= nb-ps)        return 1;
  return (0);
}

/* ==================================== */
int32_t voisin6(index_t i, int32_t k, index_t rs, index_t ps, index_t N)
/* i : index du point dans l'image */
/* k : direction du voisin */
/* rs : taille d'une rangee */
/* ps : taille d'un plan */
/* N : taille de l'image 3D */
/* retourne -1 si le voisin n'existe pas */
/* ==================================== */
#undef F_NAME
#define F_NAME "voisin6"
{
  switch(k)
  {
  case EST:        if (i%rs!=rs-1) return i+1; else return -1;
  case NORD:       if ((i%ps)>=rs) return i-rs; else return -1;
  case OUEST:      if (i%rs!=0) return i-1; else return -1;
  case SUD:        if ((i%ps)<ps-rs) return i+rs; else return -1;
  case DEVANT:     if (i>=ps) return i-ps; else return -1;
  case DERRIERE:   if (i<N-ps) return i+ps; else return -1;
  default: 
    fprintf(stderr, "%s: bad index value %d\n", F_NAME, k);
    exit(0);
  }
}

/* ==================================== */
int32_t voisin26(index_t i, int32_t k, index_t rs, index_t ps, index_t N)
/* i : index du point dans l'image */
/* k : direction du voisin */
/* rs : taille d'une rangee */
/* ps : taille d'un plan */
/* N : taille de l'image 3D */
/* retourne -1 si le voisin n'existe pas */
/* ==================================== */
#undef F_NAME
#define F_NAME "voisin26"
{
  switch(k)
  {
  /* les 9 premiers (0 a 8) sont les 9 pixels du plan "ARRIERE" (+ps) */
  case 0:  if ((i<N-ps)&&(i%rs!=rs-1)) return ps+i+1; else return -1;                 /*  1  0  1 */
  case 1:  if ((i<N-ps)&&(i%rs!=rs-1)&&(i%ps>=rs)) return ps+i+1-rs; else return -1;  /*  1 -1  1 */
  case 2:  if ((i<N-ps)&&(i%ps>=rs)) return ps+i-rs; else return -1;                  /*  0 -1  1 */
  case 3:  if ((i<N-ps)&&(i%ps>=rs)&&(i%rs!=0)) return ps+i-rs-1;  else return -1;    /* -1 -1  1 */
  case 4:  if ((i<N-ps)&&(i%rs!=0)) return ps+i-1; else return -1;                    /* -1  0  1 */
  case 5:  if ((i<N-ps)&&(i%rs!=0)&&(i%ps<ps-rs)) return ps+i-1+rs; else return -1;   /* -1  1  1 */
  case 6:  if ((i<N-ps)&&(i%ps<ps-rs)) return ps+i+rs; else return -1;                /*  0  1  1 */
  case 7:  if ((i<N-ps)&&(i%ps<ps-rs)&&(i%rs!=rs-1)) return ps+i+rs+1; else return -1;/*  1  1  1 */
  case 8:  if ((i<N-ps)) return ps+i; else return -1;                                 /*  0  0  1 */
  /* les 8 suivants (9 a 16) sont les 8 pixels du plan "COURANT" () */
  case 9:  if ((i%rs!=rs-1)) return i+1; else return -1;
  case 10: if ((i%rs!=rs-1)&&(i%ps>=rs)) return i+1-rs; else return -1;
  case 11: if ((i%ps>=rs)) return i-rs; else return -1;
  case 12: if ((i%ps>=rs)&&(i%rs!=0)) return i-rs-1;  else return -1;
  case 13: if ((i%rs!=0)) return i-1; else return -1;
  case 14: if ((i%rs!=0)&&(i%ps<ps-rs)) return i-1+rs; else return -1;
  case 15: if ((i%ps<ps-rs)) return i+rs; else return -1;
  case 16: if ((i%ps<ps-rs)&&(i%rs!=rs-1)) return i+rs+1; else return -1;
  /* les 9 derniers (17 a 25) sont les 9 pixels du plan "AVANT" (-ps) */
  case 17: if ((i>=ps)&&(i%rs!=rs-1)) return -ps+i+1; else return -1;
  case 18: if ((i>=ps)&&(i%rs!=rs-1)&&(i%ps>=rs)) return -ps+i+1-rs; else return -1;
  case 19: if ((i>=ps)&&(i%ps>=rs)) return -ps+i-rs; else return -1;
  case 20: if ((i>=ps)&&(i%ps>=rs)&&(i%rs!=0)) return -ps+i-rs-1;  else return -1;
  case 21: if ((i>=ps)&&(i%rs!=0)) return -ps+i-1; else return -1;
  case 22: if ((i>=ps)&&(i%rs!=0)&&(i%ps<ps-rs)) return -ps+i-1+rs; else return -1;
  case 23: if ((i>=ps)&&(i%ps<ps-rs)) return -ps+i+rs; else return -1;
  case 24: if ((i>=ps)&&(i%ps<ps-rs)&&(i%rs!=rs-1)) return -ps+i+rs+1; else return -1;
  case 25: if ((i>=ps)) return -ps+i; else return -1;
  default: 
    fprintf(stderr, "%s: bad index value %d\n", F_NAME, k);
    exit(0);
  }
}

/* ==================================== */
int32_t voisin18(index_t i, int32_t k, index_t rs, index_t ps, index_t N)
/* i : index du point dans l'image */
/* k : direction du voisin */
/* rs : taille d'une rangee */
/* ps : taille d'un plan */
/* N : taille de l'image 3D */
/* retourne -1 si le voisin n'existe pas */
/* ==================================== */
#undef F_NAME
#define F_NAME "voisin18"
{
  switch(k)
  {
  /* les 5 premiers (0 a 4) sont les 5 pixels du plan "ARRIERE" (+ps) */
  case 0:  if ((i<N-ps)&&(i%rs!=rs-1)) return ps+i+1; else return -1;
  case 1:  if ((i<N-ps)&&(i%ps>=rs)) return ps+i-rs; else return -1;
  case 2:  if ((i<N-ps)&&(i%rs!=0)) return ps+i-1; else return -1;
  case 3:  if ((i<N-ps)&&(i%ps<ps-rs)) return ps+i+rs; else return -1;
  case 4:  if ((i<N-ps)) return ps+i; else return -1;
  /* les 8 suivants (5 a 12) sont les 8 pixels du plan "COURANT" () */
  case 5:  if ((i%rs!=rs-1)) return i+1; else return -1;
  case 6:  if ((i%rs!=rs-1)&&(i%ps>=rs)) return i+1-rs; else return -1;
  case 7:  if ((i%ps>=rs)) return i-rs; else return -1;
  case 8:  if ((i%ps>=rs)&&(i%rs!=0)) return i-rs-1;  else return -1;
  case 9:  if ((i%rs!=0)) return i-1; else return -1;
  case 10: if ((i%rs!=0)&&(i%ps<ps-rs)) return i-1+rs; else return -1;
  case 11: if ((i%ps<ps-rs)) return i+rs; else return -1;
  case 12: if ((i%ps<ps-rs)&&(i%rs!=rs-1)) return i+rs+1; else return -1;
  /* les 5 derniers (13 a 17) sont les 5 pixels du plan "AVANT" (-ps) */
  case 13: if ((i>=ps)&&(i%rs!=rs-1)) return -ps+i+1; else return -1;
  case 14: if ((i>=ps)&&(i%ps>=rs)) return -ps+i-rs; else return -1;
  case 15: if ((i>=ps)&&(i%rs!=0)) return -ps+i-1; else return -1;
  case 16: if ((i>=ps)&&(i%ps<ps-rs)) return -ps+i+rs; else return -1;
  case 17: if ((i>=ps)) return -ps+i; else return -1;
  default: 
    fprintf(stderr, "%s: bad index value %d\n", F_NAME, k);
    exit(0);
  }
}

/* ==================================== */
int32_t voisins4(index_t i, index_t j, index_t rs)   
/* i, j : index des deux points dans l'image */
/* rs : taille d'une rangee */
/* retourne 1 si les points i et j sont 4-voisins */
/* ==================================== */
{
  index_t xi = i % rs;
  index_t xj = j % rs;
  index_t yi = i / rs;
  index_t yj = j / rs;
  if (mcabs(xi-xj) + mcabs(yi-yj) != 1) return 0;
  return 1;
} // voisins4()

/* ==================================== */
int32_t voisins8(index_t i, index_t j, index_t rs)   
/* i, j : index des deux points dans l'image */
/* rs : taille d'une rangee */
/* retourne 1 si les points i et j sont 8-voisins */
/* ==================================== */
{
  index_t xi = i % rs;
  index_t xj = j % rs;
  index_t yi = i / rs;
  index_t yj = j / rs;
  if (mcabs(xi-xj) > 1) return 0;
  if (mcabs(yi-yj) > 1) return 0;
  return 1;
} // voisins8()

/* ==================================== */
int32_t voisins6(index_t i, index_t j, index_t rs, index_t ps)   
/* i, j : index des deux points dans l'image */
/* rs : taille d'une rangee */
/* ps : taille d'un plan */
/* retourne 1 si les points i et j sont 6-voisins (en 3D) */
/* ==================================== */
{
  index_t xi = i % rs;
  index_t xj = j % rs;
  index_t yi = (i%ps) / rs;
  index_t yj = (j%ps) / rs;
  index_t zi = i / ps;
  index_t zj = j / ps;
  if (mcabs(xi-xj) + mcabs(yi-yj) + mcabs(zi-zj) != 1) return 0;
  return 1;
} // voisins6()

/* ==================================== */
int32_t voisins18(index_t i, index_t j, index_t rs, index_t ps)   
/* i, j : index des deux points dans l'image */
/* rs : taille d'une rangee */
/* ps : taille d'un plan */
/* retourne 1 si les points i et j sont 18-voisins (en 3D) */
/* ==================================== */
{
  index_t xi = i % rs;
  index_t xj = j % rs;
  index_t yi = (i%ps) / rs;
  index_t yj = (j%ps) / rs;
  index_t zi = i / ps;
  index_t zj = j / ps;
  if (mcabs(xi-xj) > 1) return 0;
  if (mcabs(yi-yj) > 1) return 0;
  if (mcabs(zi-zj) > 1) return 0;
  if ((mcabs(xi-xj) == 1) && (mcabs(yi-yj) == 1) && (mcabs(zi-zj) == 1)) return 0;
  return 1;
} // voisins18()

/* ==================================== */
int32_t voisins26(index_t i, index_t j, index_t rs, index_t ps)   
/* i, j : index des deux points dans l'image */
/* rs : taille d'une rangee */
/* ps : taille d'un plan */
/* retourne 1 si les points i et j sont 26-voisins (en 3D) */
/* ==================================== */
{
  index_t xi = i % rs;
  index_t xj = j % rs;
  index_t yi = (i%ps) / rs;
  index_t yj = (j%ps) / rs;
  index_t zi = i / ps;
  index_t zj = j / ps;
  if (mcabs(xi-xj) > 1) return 0;
  if (mcabs(yi-yj) > 1) return 0;
  if (mcabs(zi-zj) > 1) return 0;
  return 1;
} // voisins26()

/* ==================================== */
int32_t voisin5(index_t i, int32_t k, index_t rs, index_t nb)   
/* i : index du point dans l'image */
/* k : direction du voisin */
/* rs : taille d'une rangee */
/* nb : taille de l'image */
/* retourne -1 si le voisin n'existe pas */
/* ==================================== */
/*
     + 2 + 1 +
     3 + + + 0
     + + i + +
     4 + + + 7
     + 5 + 6 +
*/
#undef F_NAME
#define F_NAME "voisin5"
{
  index_t rs2;
  switch(k)
  {
  case 0:              if ((i%rs<rs-2)&&(i>=rs))    return i -rs  +2;  else return -1;
  case 1: rs2 = rs+rs; if ((i%rs<rs-1)&&(i>=rs2))   return i -rs2 +1; else return -1;
  case 2: rs2 = rs+rs; if ((i%rs>0)&&(i>=rs2))      return i -rs2 -1; else return -1;
  case 3:              if ((i%rs>1)&&(i>=rs))       return i -rs  -2;  else return -1;
  case 4:              if ((i%rs>1)&&(i<nb-rs))     return i +rs  -2;  else return -1;
  case 5: rs2 = rs+rs; if ((i%rs>0)&&(i<nb-rs2))    return i +rs2 -1; else return -1;
  case 6: rs2 = rs+rs; if ((i%rs<rs-1)&&(i<nb-rs2)) return i +rs2 +1; else return -1;
  case 7:              if ((i%rs<rs-2)&&(i<nb-rs))  return i +rs  +2;  else return -1;
  default: 
    fprintf(stderr, "%s: bad index value %d\n", F_NAME, k);
    exit(0);
  }
}

/* ==================================== */
/* renvoie l'index du voisin si il      */
/* appartient a gamma b sinon renvoie   */ 
/* -1                                   */
int32_t voisin6b(index_t i, int32_t k, index_t rs, index_t nb, index_t par)   
/* i : index du point dans l'image */
/* k : direction du voisin */
/* rs : taille d'une rangee */
/* nb : taille de l'image */
/* retourne -1 si le voisin n'existe pas */
/* ==================================== */
{
  if (par==0) {
    if( ((i%rs)%2) == ((i/rs)%2) )    
      return voisinNOSE(i, k, rs, nb);
    else
      return voisinNESO(i, k, rs, nb);
  } else {
    if( ((i%rs)%2) == ((i/rs)%2) )    
      return voisinNESO(i, k, rs, nb);
    else
      return voisinNOSE(i, k, rs, nb);
  }
}

/*      1 0 *       */
/*      2 X 5       */
/*      * 3 4       */

int32_t voisinNOSE(index_t i, int32_t k, index_t rs, index_t nb)
{
  switch(k)
    {      
    case 0:       if (i>=rs)                   return i-rs;   else return -1;
    case 1:       if ((i>=rs)&&(i%rs!=0))      return i-rs-1; else return -1;
    case 2:       if (i%rs!=0)                 return i-1;    else return -1;   
    case 5:       if (i%rs!=rs-1)              return i+1;    else return -1;
    case 3:       if (i<nb-rs)                 return i+rs;   else return -1;
    case 4:       if ((i<nb-rs)&&(i%rs!=rs-1)) return i+rs+1; else return -1;
    default: return -1;
    }
}   

/*      * 0 5      */
/*      1 X 4      */
/*      2 3 *      */
int32_t voisinNESO(index_t i, int32_t k, index_t rs, index_t nb)
{
  switch(k)
    { 
    case 4:        if (i%rs!=rs-1)              return i+1;    else return -1;
    case 5:   if ((i%rs!=rs-1)&&(i>=rs))   return i+1-rs; else return -1;
    case 0:       if (i>=rs)                   return i-rs;   else return -1;
    case 1:      if (i%rs!=0)                 return i-1;    else return -1;
    case 2:  if ((i%rs!=0)&&(i<nb-rs))    return i-1+rs; else return -1;
    case 3:        if (i<nb-rs)                 return i+rs;   else return -1;   
    default: return -1;
    }
}

/* ==================================== */
/* renvoie l'index du voisin si il      */
/* appartient a gamma b sinon renvoie   */ 
/*                                      */
/* les différents cas sont traites      */
/* directement dans la fonction pour    */
/* eviter de recalculer les coords du   */
/* point courannt                       */

int32_t voisin14b(index_t i, int32_t k, index_t rs, index_t ps, index_t N)
{
  index_t px, py, pz;
  px = (i%rs)%2;
  py = (i/ps)%2;
  pz = ((i%ps)/rs)%2;
  if( (px && py && pz) || (!px && !py && !pz))
    return voisinONAV(i, k, rs, ps, N );
  if( (px && !py && pz) || (!px && py && !pz))
    return voisinENAR( i, k, rs, ps, N  );
  if( (!px && py && pz) || (px && !py && !pz))
    return voisinENAV(i, k, rs, ps, N );
  else
    return voisinONAR( i, k, rs, ps, N );
}

/* Soit l'ordre suivant
   d'OUEST en EST,
   de NORD à SUD,
   d'AVANT vers l'ARRIERE.
   les voisin sont numérotés suivant cet ordre
*/
int32_t voisinONAV(index_t i, int32_t k, index_t rs, index_t ps, index_t N )
#undef F_NAME
#define F_NAME "voisinONAV"
{
  switch(k)
  {
    /* Premiere clique */
  case 0: if ((i/ps != 0) && (i%rs !=0)  && ((i%ps)/rs != 0)) return (i-ps-rs-1); else return -1;
  case 1: if ((i/ps != 0) && ((i%ps)/rs != 0)) return i-ps-rs; else return -1;
  case 2: if ((i/ps != 0) && (i%rs !=0)) return i-ps -1; else return -1;
  case 3: if ( (i/ps != 0) ) return i-ps; else return -1;
  case 4: if ((i%rs !=0)  && ((i%ps)/rs != 0)) return i-rs-1; else return -1;
  case 5: if ((i%ps)/rs != 0) return i-rs; else return -1;
  case 6: if ((i%rs !=0)) return i-1; else return -1;
    /* Deuxième clique */
  case 7: if ((i%rs < rs-1) ) return i+1; else return -1;
  case 8: if (i%ps < ps-rs) return i+rs; else return -1;
  case 9: if ( (i%rs < rs-1)  && (i%ps < ps-rs) ) return i+rs+1; else return -1;
  case 10: if (i < N-ps ) return i+ps; else return-1;
  case 11: if ( (i < N-ps ) && (i%rs < rs-1)) return i+ps+1; else return -1;
  case 12: if ( (i < N-ps) &&  (i%ps < ps-rs)) return i+ps+rs; else return -1;
  case 13: if ((i < N-ps ) && (i%rs < rs-1)  && (i%ps < ps-rs)) return i+ps+rs+1; else return -1;
  default: 
    fprintf(stderr, "%s: bad index value %d\n", F_NAME, k);
    exit(0);
  }
}

int32_t  voisinENAR(index_t i, int32_t k, index_t rs, index_t ps, index_t N )
#undef F_NAME
#define F_NAME "voisinENAR"
{
  switch(k)
  {
     /* Premiere clique */
  case 0: if((i%rs !=0) &&  (i/ps != 0)) return i-ps-1; else return -1;
  case 1: if( (i/ps != 0)) return i-ps; else return -1;
  case 2: if ((i%rs !=0) && ( i%ps<ps-rs) && (i/ps != 0)) return i-ps+rs-1; else return -1;
  case 3: if (  ( i%ps<ps-rs) && (i/ps != 0) ) return i-ps+rs; else return -1;
  case 4: if (i%rs !=0) return i-1; else return -1;
  case 5: if ( (i%rs !=0) && (i%ps<ps-rs) ) return i+rs-1; else return -1;
  case 6: if ( (i%ps<ps-rs) ) return i+rs; else return -1;
    /* Deuxième clique */
  case 7: if  ((i%ps)/rs != 0) return i-rs; else return -1;
  case 8: if ( (i%rs < rs-1) && ((i%ps)/rs != 0) ) return i-rs+1; else return -1;
  case 9: if (i%rs < rs-1) return i+1; else return -1;
  case 10: if (  ((i%ps)/rs != 0) && (i < N-ps ) ) return i+ps-rs; else return -1;
  case 11: if ( (i%rs < rs-1) && ((i%ps)/rs != 0) && (i < N-ps ))  return i+ps-rs+1; else return -1;
  case 12: if (i < N-ps ) return i+ps; else return -1;
  case 13: if ((i%rs < rs-1) && (i < N-ps )) return i+ps+1; else return -1;
  default: 
    fprintf(stderr, "%s: bad index value %d\n", F_NAME, k);
    exit(0);
  }
}

int32_t voisinENAV(index_t i, int32_t k, index_t rs, index_t ps, index_t N )
#undef F_NAME
#define F_NAME "voisinENAV"
{
  switch(k)
  {
     /* Premiere clique */
  case 0: if ( (i%rs < rs-1) && (i/ps != 0)) return i-ps-rs; else return -1;
  case 1: if ( (i%rs < rs-1) &&  ((i%ps)/rs != 0) && (i/ps != 0) ) return i-ps-rs+1; else return -1;
  case 2: if (i/ps != 0) return i-ps; else return -1;
  case 3: if ( (i%rs < rs-1) &&  (i/ps != 0) ) return i-ps+1; else return -1;
  case 4: if  ((i%ps)/rs != 0) return i-rs; else return -1;
  case 5: if ( (i%rs < rs-1) &&   ((i%ps)/rs != 0)) return i-rs+1; else return -1;
  case 6: if (i%rs < rs-1) return i+1; else return -1;
    /* Deuxième clique */
  case 7: if (i%rs !=0) return i-1; else return -1;
  case 8: if ( (i%rs !=0) && (i%ps<ps-rs) ) return i+rs-1; else return -1;
  case 9: if ( i%ps<ps-rs ) return i+rs; else return -1;
  case 10: if ( (i%rs !=0) && (i < N-ps ) ) return i+ps-1; else return -1;
  case 11: if (i < N-ps) return i+ps; else return -1; 
  case 12: if ( (i%rs !=0) && (i%ps<ps-rs) && (i < N-ps )) return i+ps+rs-1; else return -1;
  case 13: if ( (i%ps<ps-rs) && (i < N-ps ) ) return i+ps+rs; else return -1;
  default: 
    fprintf(stderr, "%s: bad index value %d\n", F_NAME, k);
    exit(0);
  }
}

int32_t voisinONAR(index_t i, int32_t k, index_t rs, index_t ps, index_t N )
#undef F_NAME
#define F_NAME "voisinONAR"
{
  switch(k)
  {
     /* Premiere clique */
  case 0: if (i/ps != 0) return i-ps; else return -1;
  case 1: if ( (i%rs < rs-1) && (i/ps != 0) ) return i-ps+1; else return -1;
  case 2: if ( (i%ps<ps-rs) && (i/ps != 0) ) return i-ps+rs; else return -1;    
  case 3: if ( (i%rs < rs-1) && (i%ps<ps-rs) && (i/ps != 0) ) return i-ps+rs+1; else return -1;
  case 4: if (i%rs < rs-1) return i+1; else return -1;
  case 5: if (i%ps<ps-rs) return i+rs; else return -1;
  case 6: if  ( (i%rs < rs-1) && (i%ps<ps-rs)) return i+rs+1; else return -1;
    /* Deuxième clique */
  case 7: if ((i%rs !=0) &&  ((i%ps)/rs != 0)) return i-rs-1; else return -1;
  case 8: if ((i%ps)/rs != 0) return i-rs;else return -1;
  case 9: if (i%rs !=0) return i-1; else return -1;
  case 10: if ((i%rs !=0) &&  ((i%ps)/rs != 0) &&   (i < N-ps )) return i+ps-rs-1; else return -1;
  case 11: if (((i%ps)/rs != 0) &&   (i < N-ps )) return i+ps-rs; else return -1;
  case 12: if ((i%rs !=0) &&  (i < N-ps )) return i+ps-1; else return -1;
  case 13: if (i < N-ps ) return i+ps; else return -1;
  default: 
    fprintf(stderr, "%s: bad index value %d\n", F_NAME, k);
    exit(0);
  }
}

/* ==================================== */
uint32_t maskvois26(uint8_t *F, uint32_t bitmask, index_t i, index_t rs, index_t ps, index_t N)
/* F : pointeur de base de l'image */
/* bitmask : masque du bit à tester */
/* i : index du point dans l'image */
/* rs : taille d'une rangee */
/* ps : taille d'un plan */
/* N : taille de l'image 3D */
/* ==================================== */
{
  uint32_t mask = 0;
  index_t v;
  int32_t k;
  for (k = 0; k < 26; k++)
  {
    v = voisin26(i, k, rs, ps, N);
    if ((v != -1) && (F[v] & bitmask))
      mask = mask | (1<<k);
  }
  return mask;
}

/* ==================================== */
int32_t sont4voisins(index_t p, index_t q, index_t rs)
/* ==================================== */
{
  index_t xp = p % rs, xq = q % rs, yp = p / rs, yq = q / rs;
  return ((mcabs(xp-xq) + mcabs(yp-yq)) == 1);
}

/* ==================================== */
int32_t sont8voisins(index_t p, index_t q, index_t rs)
/* ==================================== */
{
  index_t xp = p % rs, xq = q % rs, yp = p / rs, yq = q / rs;
  index_t ax = mcabs(xp-xq);
  index_t ay = mcabs(yp-yq);
  return (mcmax(ax,ay) == 1);
}

/* ==================================== */
int32_t sont6voisins(index_t p, index_t q, index_t rs, index_t ps)
/* ==================================== */
{
  index_t xp = p % rs, xq = q % rs; 
  index_t yp = (p%ps) / rs, yq = (q%ps) / rs; 
  index_t zp = p / ps, zq = q / ps;
  return ((mcabs(xp-xq) + mcabs(yp-yq) + mcabs(zp-zq)) == 1);
}

/* ==================================== */
int32_t sont26voisins(index_t p, index_t q, index_t rs, index_t ps)
/* ==================================== */
{
  index_t xp = p % rs, xq = q % rs; 
  index_t yp = (p%ps) / rs, yq = (q%ps) / rs; 
  index_t zp = p / ps, zq = q / ps;
  index_t ax = mcabs(xp-xq);
  index_t ay = mcabs(yp-yq);
  index_t axy = mcmax(ax,ay);
  index_t az = mcabs(zp-zq);
  return (mcmax(axy,az) == 1);
}

/* ==================================== */
int32_t sont18voisins(index_t p, index_t q, index_t rs, index_t ps)
/* ==================================== */
{
  index_t xp = p % rs, xq = q % rs; 
  index_t yp = (p%ps) / rs, yq = (q%ps) / rs; 
  index_t zp = p / ps, zq = q / ps;
  index_t ax = mcabs(xp-xq);
  index_t ay = mcabs(yp-yq);
  index_t axy = mcmax(ax,ay);
  index_t az = mcabs(zp-zq);
  return ((mcmax(axy,az) == 1) && ((ax + ay + az) <= 2));
}
