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
#include <mccodimage.h>
#include <mckhalimsky3d.h>

int32_t lga2khalimsky(struct xvimage *ga,     /* graphe d'arete en entree */  
		  struct xvimage *out,    /* mapping ds la grille de khalimsky */
		  int32_t bar                 /* pour visualiser un ensemble de type X ou \Bar{X} */
		                          /* ie minima de type \Bar{X} divide de type X */
		  )
{
  int32_t i,j,k;                              /* index muet */
  int32_t x,w;                                /* un pixel et son voisin */
  uint8_t tmp;
  int32_t rs = rowsize(ga);                   /* taille ligne */ 
  int32_t cs = colsize(ga);                   /* taille colone */
  int32_t N = rs * cs;                        /* taille image */
  uint8_t *GA = UCHARDATA(ga);      
  uint8_t *F = UCHARDATA(out);      /* composante rouge */
  /* on alloue les elmts 1D ds khalimsky ie les aretes du ga */
  if(bar == 0)
  {
    for(j = 0; j < cs; j++)
      for(i = 0; i < rs -1; i++)
      {
	F[4*j* rs+2*i+1] = GA[j*rs+i];
      }
    for(j = 0; j < cs-1; j++)
      for(i = 0; i < rs; i++)
      {
	F[(4*j+2)*rs +2*i] = GA[N+j*rs+i];
      }
    cs = 2*cs;
    rs = 2*rs;
    N = rs*cs;
    for(j = 0; j < cs; j++)
      for(i = 0; i < rs; i++)
	if( !(i%2) && !(j%2)) /* si l'on est sur une elmt 2D de khal */
	{
	  x = j*rs+i;
	  tmp = 255;
	  for(k = 0; k < 8; k+=2)
	  {
	    w = voisin(x,k,rs,N);
	    if( (w > -1) && (F[w] < tmp)) tmp = F[w];
	  }
	  F[x] = tmp;  /* on conserve le min des elmts 1D contenu ds l'elmt 2D*/
	}
	else
	  if((i%2) && (j%2)) /* si l'on est sur un elmt 0D de khal */
	  {
	    x = j*rs+i;
	    tmp = 0;
	    for(k = 0; k < 8; k+=2)
	    {
	      w = voisin(x,k,rs,N);
	      if((w > -1) && (F[w] > tmp)) tmp = F[w];
	    }
	    F[x] = tmp;   /* on conserve le max des elmts 1D contenant l'elmt 0D */
	  }
  }
  else
  {
    for(j = 0; j < cs; j++)
      for(i = 0; i < rs -1; i++)
      {
	F[ 4*j* rs + 2*i+1] = GA[j*rs+i];
      }
    for(j = 0; j < cs-1; j++)
      for(i = 0; i < rs; i++)
      {
	F[(4*j+2)*rs +2*i] = GA[N+j*rs+i];
      }
   
    cs = 2*cs;
    rs = 2*rs;
    N = rs*cs;
    for(j = 0; j < cs; j++)
      for(i = 0; i < rs; i++)
	if(!(i%2) && !(j%2)) /* si l'on est sur une elmt 2D de khal */
	{
	  x = j*rs+i;
	  tmp = 0;
	  for(k = 0; k < 8; k+=2)
	  {
	    w = voisin(x,k,rs,N);
	    if( (w > - 1)  && (F[w] > tmp) ) tmp = F[w];
	  }
	  F[x] = tmp;  /* on conserve le max des elmts 1D contenu ds l'elmt 2D*/
	}
	else
	  if((i%2) && (j%2)) /* si l'on est sur un elmt 0D de khal */
	  {
	    x = j*rs+i;
	    tmp = 255;
	    for(k = 0; k < 8; k+=2)
	    {
	      w = voisin(x,k,rs,N);
	      if( (w > -1) && (F[w] < tmp)) tmp = F[w];
	    }
	    F[x] = tmp;   /* on conserve le min des elmts 1D contenant l'elmt 0D */
	  }
  }
  return 1;
}

int32_t lga2khalimsky3d(struct xvimage *ga,     /* graphe d'arete en entree */  
		    struct xvimage *out,    /* mapping ds la grille de khalimsky */
		    int32_t bar                 /* pour visualiser un ensemble de type X ou \Bar{X} */
		                            /* ie minima de type \Bar{X} divide de type X */
		  )
{
  int32_t i,j,k,l;                              /* index muet */
  int32_t x,w;                                  /* un pixel et son voisin */
  uint8_t tmp;
  int32_t rs = rowsize(ga);                     /* taille ligne */ 
  int32_t cs = colsize(ga);                     /* taille colone */
  int32_t ds = depth(ga);
  int32_t ps = rs * cs;                         /* taille image */
  int32_t N = ps * ds;
  uint8_t *GA = UCHARDATA(ga);      
  uint8_t *F = UCHARDATA(out);        /* composante rouge */
  
  int32_t cs_k = 2*cs;
  int32_t rs_k = 2*rs;
  int32_t ds_k = 2*ds;
  int32_t ps_k = rs_k*cs_k;
  int32_t N_k = ps_k * ds_k;
  
  if(N_k != rowsize(out) * depth(out) * colsize(out)) printf("Erreur icompatible image size \n");
  
  /* Les conventions adoptés (à la difference du cas 2D) sont */
  /* celles de mckhalim3d.h c'est à dire:                     */
  /* (i%2) + (j%2) + (k%2) == 0 --> elmt 0D                   */
  /* (i%2) + (j%2) + (k%2) == 1 --> elmt 1D                   */
  /* (i%2) + (j%2) + (k%2) == 2 --> elmt 2D                   */
  /* (i%2) + (j%2) + (k%2) == 3 --> elmt 3D                   */
  
  /* on alloue les elmts 2D ds khalimsky ie les aretes du ga */
#ifdef DEBUG
  printf("cs_k %d, rs_k %d, ds_k %d, ps_k %d, N_k %d", cs_k, rs_k,ds_k,ps_k,N_k);
#endif
  if(bar == 0)
  {    
    printf("lga2khalimsky3d: bar == 0\n");
    /*    for(i=0;i<3*N;i++)
      if( ( (i < N) && (i%rs < rs-1)) ||
	  ((i >= N) && (i < 2*N) && ( (i%ps) < (ps-rs))) ||
	  ((i >= 2*N) && (((i- (2*N))/ps) < (ds-1)))){
	printf("%d ",GA[i]);
	}*/
    for(k = 0; k < ds; k++)
      for(j = 0; j < cs; j++)
	for(i = 0; i < rs -1; i++)
	{
	  /* elmt du type 2(i+1),2j+1, 2k+1 */	  
	  F[((2*k)+1)*ps_k + ((2*j)+1)*rs_k + 2*(i+1)] = GA[k*ps + j*rs +i];
	}   
    for(k = 0; k < ds; k++)
      for(j = 0; j < cs-1; j++)
	for(i = 0; i < rs; i++)
	{
	  /* elmt du type 2i+1 ,2(j+1), 2k+1 */ 
	  F[((2*k)+1)*ps_k + 2*(j+1)*rs_k + (2*i)+1] = GA[N + k*ps +j*rs + i];
	  /* printf("F[%d,%d,%d] = %d ; %d \n", 2*i+1 , 2*(j+1), (2*k+1),
		 F[(2*k+1)*ps_k + 2*(j+1)*rs_k + 2*i+1],
		 N+ k*ps +j*rs + i); */
	}
    for(k = 0; k < ds-1; k++)
      for(j = 0; j < cs; j++)
	for(i = 0; i < rs; i++)
	{
	  /* elmt du type 2i+1, 2j+1, 2(k+1) */
	  F[2*(k+1)*ps_k + ((2*j)+1)*rs_k + (2*i)+1] = GA[2*N + k*ps + j*rs + i];
	  /* printf("F[%d,%d,%d] = %d; %d \n", 2*i+1 , (2*j+1), 2*(k+1),
		 F[2*(k+1)*ps_k + (2*j+1)*rs_k + 2*i+1],
		 2*N + k*ps + j*rs + i);*/
	}
    printf("les elmts 2D ont etes calcules \n");
    
    for(k = 0; k < ds_k; k++)
      for(j = 0; j < cs_k; j++)
	for(i = 0; i < rs_k; i++)
	  if(CUBE3D(i,j,k))
	  {
	    x = i + j*rs_k + k*ps_k; 
	    tmp = 255;
	    for(l = 0; l < 6; l++)
	    {
	      w = voisin6(x,l,rs_k,ps_k,N_k); 
	      if((w > -1) && (F[w] < tmp)) tmp = F[w];
	    }
 	    F[x] = tmp;
	  }
	  else if(INTER3D(i, j ,k))
	  {
	    x = i +j*rs_k + k*ps_k; 
	    tmp = 0;
	    for(l = 0; l < 12; l+=2)
	    {
	      w = voisin6(x,l,rs_k,ps_k,N_k); 
	      if((w > -1) && (CARRE3D(w%rs_k, (w%ps_k) / rs_k , w/ps_k)) && (F[w] > tmp)) tmp = F[w];
	    }
	    F[x] = tmp;
	  } 
	  else if(SINGL3D(i,j,k))
	  {
	    x = i + j*rs_k + k*ps_k; 
	    tmp = 0;
	    for(l = 0; l < 18; l++)
	    {
	      w = voisin18(x,l,rs_k,ps_k,N_k); 
	      if((w > -1) && (CARRE3D(w%rs_k, (w%ps_k)/rs_k , w/ps_k)) && (F[w] > tmp)) tmp = F[w];
	    }
	    F[x] = tmp;
	  }
  }
  else
  {
    for(k = 0; k < ds; k++)
      for(j = 0; j < cs; j++)
	for(i = 0; i < rs -1; i++)
	{
	  /* elmt du type 2(i+1),2j+1, 2k+1 */	  
	  F[(2*k+1)*ps_k + (2*j+1)*rs_k + 2*(i+1)] = GA[j*rs+i];
	}   
    for(k = 0; k < ds; k++)
      for(j = 0; j < cs-1; j++)
	for(i = 0; i < rs; i++)
	{
	  /* elmt du type 2i+1 ,2(j+1), 2k+1 */ 
	  F[(2*k+1)*ps_k + 2*(j+1)*rs_k + 2*i+1] = GA[N+j*rs+i];
	}
    for(k = 0; k < ds-1; k++)
      for(j = 0; j < cs; j++)
	for(i = 0; i < rs; i++)
	{
	  /* elmt du type 2i+1, 2j+1, 2(k+1) */
	  F[2*(k+1)*ps_k + (2*j+1)*rs_k + 2*i+1] = GA[2*N+j*rs+i];
	}
    for(k = 0; k < ds_k; k++)
      for(j = 0; j < cs_k; j++)
	for(i = 0; i < rs_k; i++)
	  if(CUBE3D(i,j,k))
	  {
	    x = i +j*rs_k+k*ps_k;
	    tmp = 0;
	    for(l = 0; l < 6; l++)
	    {
 	      w = voisin6(x,l,rs_k,ps_k,N_k);
	      if((w > -1) && (F[w] > tmp)) tmp = F[w];
	    }
 	    F[x] = tmp;
	  }
	  else if(!CARRE3D(i, j ,k) )
	  {
	    x = i +j*rs_k+k*ps_k;
	    tmp = 255;
	    for(l = 0; l < 18; l++)
	    {
	      w = voisin18(x,l,rs_k,ps_k,N_k);
	      if((w > -1) && (CARRE3D(w%rs_k, (w%ps_k)/rs_k , w/ps_k)) && (F[w] < tmp)) tmp = F[w];
	    }
	    F[x] = tmp;
	  } 
  }
  return 1;
}
 
