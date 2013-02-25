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
/* structure de file d'attente hierarchique de points d'image */
/* Michel Couprie */

/* un point d'une image F est reperee par son index */
/* dans le tableau mono-dimension ou les valeurs sont rangees */
/* par ligne, puis par colonne */

/* d'apres F. Meyer: "Un Algorithme Optimal de Ligne de Partage des Eaux" */
/* version "salembier" (un point insere sous le niveau courant recree une file a ce niveau, 
   de plus on peut retirer un point de n'importe quel niveau a tout moment) */

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcfahsalembier.h>

/*
#define TESTFahs
#define VERBOSE
*/

/* ==================================== */
Fahs * CreeFahsVide(
  index_t taillemax)
/* ==================================== */
{
  index_t i;
  Fahs * L = (Fahs *)calloc(1,sizeof(Fahs) + (taillemax - 1) * sizeof(FahsElt));
  if (L == NULL)
  {
    fprintf(stderr, "erreur allocation Fahs\n");
    exit(1);
  }
  L->Max = taillemax;
  L->Util = 0;
  L->Maxutil = 0;
  L->Niv = 0;
  for (i = 0; i < taillemax - 1; i++) L->Elts[i].Next = &(L->Elts[i+1]);
  L->Elts[taillemax - 1].Next = NULL;
  L->Libre = &(L->Elts[0]);
  for (i = 0; i < FAHS_NPRIO; i++) L->Tete[i]= NULL;
  for (i = 0; i < FAHS_NPRIO; i++) L->Queue[i]= NULL;
  return L;
} /* CreeFahsVide() */

/* ==================================== */
void FahsFlush(
  Fahs * L)
/* ==================================== */
{
  index_t i;
  L->Niv = 0;
  L->Util = 0;
  for (i = 0; i < L->Max - 1; i++) L->Elts[i].Next = &(L->Elts[i+1]);
  L->Elts[L->Max - 1].Next = NULL;
  L->Libre = &(L->Elts[0]);
  for (i = 0; i < FAHS_NPRIO; i++) L->Tete[i]= NULL;
  for (i = 0; i < FAHS_NPRIO; i++) L->Queue[i]= NULL;
} /* FahsFlush() */

/* ==================================== */
int32_t FahsVide(
  Fahs * L)
/* ==================================== */
{
  if (L->Util == 0) return 1;
  return 0;
} /* FahsVide() */

/* ==================================== */
int32_t FahsVideNiveau(
  Fahs * L,
  int32_t niv)
/* ==================================== */
{
  if (L->Queue[niv] == NULL) return 1;
  return 0;
} /* FahsVideNiveau() */

/* ==================================== */
index_t FahsPopNiveau(
  Fahs * L, 
  int32_t niv)
/* ==================================== */
{
  index_t V;
  FahsElt * FE;
  if (L->Queue[niv] == NULL)
  {
    fprintf(stderr, "erreur Fahs vide au niveau %d\n", niv);
    exit(1);
  }

  L->Util--;
  V = L->Queue[niv]->Point;
  FE = L->Queue[niv]->Prev;

  L->Queue[niv]->Next = L->Libre; /* recupere la cellule dans la liste libre */
  L->Libre = L->Queue[niv];
  L->Queue[niv] = FE;

  if (FE == NULL) /* plus aucun element dans la liste */
  {
    L->Tete[niv] = NULL;
    if (niv == L->Niv)
      do (L->Niv)++;                 /* incrementer le niveau */
      while ((L->Niv < FAHS_NPRIO)          
              && (L->Tete[L->Niv] == NULL));
  }
  else if (L->Tete[niv] == L->Queue[niv]) /* seul un element reste dans la liste */
  {
    L->Tete[niv]->Next = L->Tete[niv]->Prev = NULL;
  }

  return V;
} /* FahsPopNiveau() */

/* ==================================== */
index_t FahsPop(
  Fahs * L)
/* ==================================== */
{
  if (L->Util == 0)
  {
    fprintf(stderr, "erreur Fahs vide\n");
    exit(1);
  }
  return FahsPopNiveau(L, L->Niv);
} /* FahsPop() */
  
/* ==================================== */
void FahsPush(
  Fahs * L,
  index_t Po,
  int32_t Ni)
/* ==================================== */
{
  if (L->Libre == NULL)
  {
    fprintf(stderr, "erreur Fahs pleine\n");
    exit(1);
  }
  if (Ni >= FAHS_NPRIO)
  {
    fprintf(stderr, "erreur niveau = %d; max autorise = %d\n", Ni, FAHS_NPRIO-1);
    exit(1);
  }

  if ((L->Util == 0) || (Ni < L->Niv)) L->Niv = Ni;

  L->Util++;
  if (L->Util > L->Maxutil) L->Maxutil = L->Util;
  if (L->Tete[Ni] != NULL)   /* insertion dans la liste de niveau Ni non vide */
  {
    FahsElt * FE = L->Tete[Ni];
    L->Tete[Ni] = L->Libre;
    L->Libre = L->Libre->Next;
    L->Tete[Ni]->Next = FE;
    L->Tete[Ni]->Prev = NULL;
    L->Tete[Ni]->Point = Po;      
    FE->Prev = L->Tete[Ni];
  }
  else /* (L->Tete[Ni] == NULL) */
  {
    L->Tete[Ni] = L->Queue[Ni] = L->Libre;
    L->Libre = L->Libre->Next;
    L->Tete[Ni]->Next = NULL;
    L->Tete[Ni]->Prev = NULL;
    L->Tete[Ni]->Point = Po;      
  }
} /* FahsPush() */

/* ==================================== */
void FahsTermine(
  Fahs * L)
/* ==================================== */
{
#ifdef VERBOSE
  printf("Fahs: taux d'utilisation: %g\n", (double)L->Maxutil / (double)L->Max);
#endif
  free(L);
} /* FahsTermine() */

/* ==================================== */
void FahsPrint(
  Fahs * L)
/* ==================================== */
{
  int32_t i;
  FahsElt * FE;
  if (FahsVide(L)) {printf("[]\n"); return;}
  printf("niveau courant = %d\n", L->Niv);
  for (i = 0; i < FAHS_NPRIO; i++)
    if (L->Tete[i] != NULL) 
    {
      printf("%d [ ", i);
      for (FE = L->Tete[i]; FE != NULL; FE = FE->Next)
#ifdef MC_64_BITS
        printf("%lld ", FE->Point);
#else
        printf("%d ", FE->Point);
#endif
      printf("]\n");
    }  
} /* FahsPrint() */

#ifdef TESTFAHS
void main()
{
  Fahs * L = CreeFahsVide(5);
  char r[80];
  int32_t p, n;

  do
  {
    printf("commande (qUIT, PuSH, PoP, pRINT, TESTvIDE) > ");
    scanf("%s", r);
    switch (r[0])
    {
      case 'u':
        printf("niveau > ");
        scanf("%d", &n);
        printf("valeur > ");
        scanf("%d", &p);
        FahsPush(L, p, n);
        break;
      case 'o': 
        printf("pop: %d\n", FahsPop(L));
        break;
      case 'p': FahsPrint(L); break;
      case 'v': 
        printf("vide: %d\n", FahsVide(L));
        break;
      case 'q': break;
    }
  } while (r[0] != 'q');
  FahsTermine(L);
}
#endif
