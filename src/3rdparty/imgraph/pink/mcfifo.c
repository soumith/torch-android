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
   Librairie mcfifo :

   fonctions pour la gestion d'une liste fifo

   Michel Couprie 1996
*/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <mcfifo.h>

/* ==================================== */
Fifo * CreeFifoVide(
  index_t taillemax)
/* ==================================== */
{
  Fifo * L = (Fifo *)calloc(1,sizeof(Fifo) + sizeof(index_t) * taillemax); /* sic (+1) */
  if (L == NULL)
  {   
#ifdef MC_64_BITS
    fprintf(stderr, "CreeFifoVide() : malloc failed : %lld bytes\n", sizeof(Fifo) + sizeof(index_t) * taillemax);
#else
    fprintf(stderr, "CreeFifoVide() : malloc failed : %ld bytes\n", sizeof(Fifo) + sizeof(index_t) * taillemax);
#endif
    return NULL;
  }
  L->Max = taillemax+1;
  L->In = 0;
  L->Out = 0;
  return L;
}

/* ==================================== */
void FifoFlush(
  Fifo * L)
/* ==================================== */
{
  L->In = 0;
  L->Out = 0;
}

/* ==================================== */
int32_t FifoVide(
  Fifo * L)
/* ==================================== */
{
  return (L->In == L->Out);
}

/* ==================================== */
index_t FifoPop(
  Fifo * L)
/* ==================================== */
{
  index_t V;
  if (L->In == L->Out)
  {
    fprintf(stderr, "erreur fifo vide\n");
    exit(1);
  }
  V = L->Pts[L->Out];
  L->Out = (L->Out + 1) % L->Max;
  return V;
}

/* ==================================== */
void FifoPushFirst(
  Fifo * L,
  index_t V)
/* ==================================== */
{
  L->Out = (L->Out - 1) % L->Max;
  L->Pts[L->Out] = V;
  if (L->In == L->Out)
  {
    fprintf(stderr, "erreur fifo pleine\n");
    exit(1);
  }
}
  
/* ==================================== */
void FifoPush(
  Fifo * L,
  index_t V)
/* ==================================== */
{
  L->Pts[L->In] = V;
  L->In = (L->In + 1) % L->Max;
  if (L->In == L->Out)
  {
    fprintf(stderr, "erreur fifo pleine\n");
    exit(1);
  }
}

/* ==================================== */
void FifoPrint(
  Fifo * L)
/* ==================================== */
{
  index_t i;
  if (FifoVide(L)) {printf("[]\n"); return;}
#ifdef MC_64_BITS
  printf("Taille Fifo: %lld \n",FifoTaille(L));
#else
  printf("Taille Fifo: %d \n",FifoTaille(L));
#endif
#ifdef MC_64_BITS
  printf("Max = %lld ; Out = %lld ; In = %lld\n", L->Max, L->Out, L->In);
#else
  printf("Max = %d ; Out = %d ; In = %d\n", L->Max, L->Out, L->In);
#endif
  printf("[ ");
  for (i = L->Out; i != L->In; i = (i+1) % L->Max)
#ifdef MC_64_BITS
  printf("%lld ", L->Pts[i]);
#else
  printf("%d ", L->Pts[i]);
#endif
  printf("]\n");
}

/* ==================================== */
void FifoTermine(
  Fifo * L)
/* ==================================== */
{
  free(L);
}

/* ==================================== */
index_t FifoTaille(
  Fifo * L)
/* ==================================== */
{
  if (L->In < L->Out)
     return (L->Max - (L->Out-L->In));
  else
     return (L->In - L->Out);
}

#ifdef TESTFIFO
void main()
{
  Fifo * L = CreeFifoVide(3);
  FifoPrint(L);
  if (FifoVide(L)) printf("FifoVide OUI\n");
  FifoPush(L,1);
  FifoPrint(L);
  if (!FifoVide(L)) printf("FifoVide NON\n");
  FifoPush(L,2);
  FifoPrint(L);
  FifoPush(L,3);
  FifoPrint(L);
  printf("FifoPop %d attendu 1\n", FifoPop(L));
  FifoPrint(L);
  FifoPushFirst(L,225);
  FifoPrint(L);
  printf("FifoPop %d attendu par christophe 225\n", FifoPop(L));
  FifoPrint(L);
  FifoPush(L,4);
  FifoPrint(L);
  printf("FifoPop %d attendu 2\n", FifoPop(L));
  FifoPrint(L);
  printf("FifoPop %d attendu 3\n", FifoPop(L));
  FifoPrint(L);
  printf("FifoPop %d attendu 4\n", FifoPop(L));
  FifoPrint(L);
  if (FifoVide(L)) printf("FifoVide OUI\n");
}
#endif


