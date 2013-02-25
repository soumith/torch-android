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
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include <mccodimage.h>
#include <jccodimage.h>
#include <mcutil.h>

#define VFF_TYP_GABYTE          12      /* graphe d'arete code sur 1 octet */

/*
  Codage du voisinage d'un graphe d'arete
               NB : Seul la 4-connexite est disponible
  Pour une arete horizontale                 

                 _|_|_
                  | |

                0     5    
             1     X     4  
                2     3
 Pour une arete verticale
                  
                 _|_
                 _|_
                  |

                  1
               2     0              
                  X
               3     5
                  4 
*/


int32_t voisinGA(int32_t i, int32_t k, int32_t rs, int32_t nb)
{
  if(i < nb)                  /* i est horizontale */
    switch(k)
    {
    case 0: if(i/rs > 0) return nb+i-rs; else return -1;
    case 1: if(i%rs > 0) return i-1; else return -1;
    case 2: if(i < nb-rs) return nb+i; else return -1;
    case 3: if((i < nb-rs) && (i%rs < rs-1)) return nb+i+1; else return -1;
    case 4: if(i%rs < rs-2) return i+1; else return -1;
    case 5: if((i/rs > 0) &&  (i%rs < rs-1)) return nb+i-rs+1; else return -1;
    }
  else                       /* sinon i est verticale */
    switch(k)
    {
    case 0: if(i%rs < rs-1) return i-nb; else return -1;
    case 1: if( ((i-nb)/rs)>0) return i-rs; else return -1;
    case 2: if(i%rs > 0) return i-nb-1; else return -1;
    case 3: if( ((i-nb) < nb-rs) && (i%rs > 0)) return i-nb-1+rs; else return -1; 
    case 4: if((i-nb) < nb-2*rs) return i+rs;else return -1;
    case 5: if( ((i-nb) < nb-rs) && (i%rs < rs-1)) return i-nb+rs; else return -1;
    }
  assert(1); exit(1);
}

/* ==================================== */
int32_t incidente(int32_t i, int32_t k, int32_t rs, int32_t nb)   
/* ds un ga 4-connexe, retourne les index des aretes incidente à i*/
/* i : index du point dans l'image */
/* k : direction du voisin */
/* rs : taille d'une rangee */
/* nb : taille de l'image */
/* retourne -1 si le voisin n'existe pas */
/* ==================================== */
{
  switch(k)
  {
    /* EST */
  case 0:        if (i%rs!=rs-1)              return i;     else return -1;
    /* NORD */
  case 1:        if (i>=rs)                   return nb+i-rs;  else return -1;
    /*OUEST */
  case 2:        if (i%rs!=0)                 return i-1;     else return -1;
    /* SUD */
  case 3:        if (i<nb-rs)                 return nb+i;     else return -1;
  default: return -1;
  }
}

/* ==================================== */
int32_t incidente3d(int32_t i, int32_t k, int32_t rs, int32_t nb, int32_t ps)   
/* ds un ga 4-connexe, retourne les index des aretes incidente à i*/
/* i : index du point dans l'image */
/* k : direction du voisin */
/* rs : taille d'une rangee */
/* nb : taille de l'image */
/* retourne -1 si le voisin n'existe pas */
/* ==================================== */
{
  switch(k)
  {
    /* EST */
  case 0:        if (i%rs!=rs-1) return i; else return -1;
    /* NORD */
  case 1:        if ((i%ps)>=rs) return nb+i-rs; else return -1;
    /* OUEST */
  case 2:        if (i%rs!= 0) return i-1; else return -1;
    /*SUD*/
  case 3:        if ((i%ps)<ps-rs) return nb+i; else return -1;
    /* DEVANT */
  case 4:        if (i>=ps) return (2*nb)+i-ps; else return -1;
    /* DERRIERE */
  case 5:        if (i<nb-ps) return (2*nb)+i; else return -1;
  default: return -1;
  }
}

/* =============================================================== */
int32_t incidente4d(int32_t i, int32_t k, int32_t rs, int32_t nb, int32_t ps, int32_t vs)   
/* ds un ga 4-connexe, retourne les index des aretes incidente à i */
/* i : index du point dans l'image                                 */
/* k : direction du voisin                                         */
/* rs : taille d'une rangee                                        */
/* ps : taille d'un plan                                           */
/* vs : taille d'un volume                                         */
/* nb : taille de la sequence                                      */
/* retourne -1 si le voisin n'existe pas                           */
/* =============================================================== */
{
  switch(k)
  {
    /* EST */
  case 0:        if (i%rs!=rs-1) return i; else return -1;
    /* NORD */
  case 1:        if ((i%ps)>=rs) return nb+i-rs; else return -1;
    /* OUEST */
  case 2:        if (i%rs != 0) return i-1; else return -1;
    /*SUD*/
  case 3:        if ((i%ps)<ps-rs) return nb+i; else return -1;
    /* DERRIERE */
  case 4:        if (i%vs >= ps) return (2*nb)+i-ps; else return -1;
    /* DEVANT */
  case 5:        if (i%vs < (vs-ps)) return (2*nb)+i; else return -1;
    /* AVANT */
  case 6:        if (i >= vs) return (3*nb)+i - vs; else return -1; 
    /* APRES */
  case 7:        if (i < (nb-vs) ) return (3*nb)+i; else return -1; 
  default: return -1;
  }
}


/* Etant donne un graphe 6connexe (en 3D) retourne l'extremite x */
/* (celles dont l'index est le plus petit) de l'arete u          */  
int32_t Sommetx3d(int32_t u, int32_t N,int32_t rs, int32_t ps)
{
  switch (u/N)
  {
  case 0:        return u;
  case 1:        return u-N;
  case 2:        return u-(2*N);
  default:       assert(1); exit(1);
  } 
}
/* Etant donne un graphe 6connexe (en 3D) retourne l'extremite x */
/* (celles dont l'index est le plus grand) de l'arete u          */ 
int32_t Sommety3d(int32_t u, int32_t N,int32_t rs, int32_t ps)
{
  switch(u/N)
  {
  case 0:        return u+1;
  case 1:        return u-N+rs;
  case 2:        return u-(2*N)+ps;
  default:       assert(1); exit(1);
  }
}

int32_t Sommetx4d(int32_t u, int32_t N, int32_t rs, int32_t ps, int32_t vs)
{
  switch (u/N)
  {
  case 0:        return u;
  case 1:        return u-N;
  case 2:        return u-(2*N);
  case 3:        return u-(3*N);
  default:       assert(1); exit(1);
  }
}

int32_t Sommety4d(int32_t u, int32_t N, int32_t rs, int32_t ps, int32_t vs)
{
  switch(u/N)
  {
  case 0:        return u+1;
  case 1:        return u-N+rs;
  case 2:        return u-(2*N)+ps;
  case 3:        return u-(3*N)+vs;
  default:       assert(1); exit(1);
  }
}

int32_t Arete(int32_t x, int32_t y, int32_t rs, int32_t N)
{
  int32_t z;
  if(y < x){z = x; x = y; y = z;}
  z =  y - x; 
  if(z == 1) return x;  
  if(z == rs) return x+N; 
  else return -1;
}

/* ==================================== */
int32_t voisin4D8(int32_t i, int32_t k, int32_t rs, int32_t ps, int32_t N, int32_t Nt)
/* i : index du point dans l'image */
/* k : direction du voisin */
/* rs : taille d'une rangee */
/* ps : taille d'un plan */
/* N : taille de l'image 3D */
/* Nt : taille de la sequence */
/* retourne -1 si le voisin n'existe pas */
/* ==================================== */
{
  switch(k)
  {
  case 0:  if (i%rs!=rs-1) return i+1; else return -1;
  case 1:  if ((i%ps)>=rs) return i-rs; else return -1;
  case 2:  if (i%rs!=0) return i-1; else return -1;
  case 3:  if ((i%ps)<ps-rs) return i+rs; else return -1;
  case 4:  if ((i%N) >=ps) return i-ps; else return -1;
  case 5:  if ((i%N) <N-ps) return i+ps; else return -1;
  case 6:  if (i > N) return i-N; else return -1;
  case 7:  if (i < Nt - N) return i+N; else return -1;
  }
  assert(1); exit(1);
}/* voisin4D8() */
