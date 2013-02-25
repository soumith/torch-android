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
Librairie mcunionfind :

Structures et algorithmes pour l'union find
Ref: R.E. Tarjan, Data Structures and Network Algorithms, SIAM, 1978.

Michel Couprie 2003

ATTENTION : L'opération de Link n'a de sens que si elle agit sur des représentants
(il faut faire find avant) - pas de vérification

*/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcutil.h>
#include <mcunionfind.h>

/*
#define TESTUnionfind
*/

/* ==================================== */
Tarjan * CreeTarjan(int32_t taille)
/* ==================================== */
/*! \fn Tarjan * CreeTarjan(int32_t taille)
    \param taille : nombre total d'éléments
    \return pointeur sur une structure Tarjan
    \brief crée une structure pour la fusion d'ensemble
    \warning ne fait pas l'initialisation de la structure
*/
#undef F_NAME
#define F_NAME "CreeTarjan"
{
  Tarjan * T = (Tarjan *)calloc(1,sizeof(Tarjan));
  T->Size = taille;
  if (T == NULL)
  {   fprintf(stderr, "%s : malloc failed for T\n", F_NAME);
      return(NULL);
  }
  T->Fth = (int32_t *)calloc(1,taille * sizeof(int32_t));
  if (T->Fth == NULL)
  {   fprintf(stderr, "%s : malloc failed for T->Fth\n", F_NAME);
      return(NULL);
  }
  T->Rank = (int32_t *)calloc(1,taille * sizeof(int32_t));
  if (T->Rank == NULL)
  {   fprintf(stderr, "%s : malloc failed for T->Rank\n", F_NAME);
      return(NULL);
  }
  return T;
} //CreeTarjan()

/* ==================================== */
void TarjanTermine(Tarjan * T)
/* ==================================== */
/*! \fn void TarjanTermine(Tarjan * T)
    \param T: une structure Tarjan
    \brief libère la mémoire
*/
{
  free(T->Fth);
  free(T->Rank);
  free(T);
} //TarjanTermine()

/* ==================================== */
void TarjanInit(Tarjan * T)
/* ==================================== */
/*! \fn void TarjanInit(Tarjan * T)
    \param T: une structure Tarjan
    \brief initialise la structure (crée les singletons)
*/
{
  int32_t i;
  for (i = 0; i < T->Size; i++) TarjanMakeSet(T, i);
} //TarjanInit()

/* ==================================== */
void TarjanPrint(Tarjan * T)
/* ==================================== */
/*! \fn void TarjanPrint(Tarjan * T)
    \param T: une structure Tarjan
    \brief affiche la structure
*/
{
  int32_t i;
  for (i = 0; i < T->Size; i++)
    printf("%d: Rank = %d ; Fth = %d\n", i, T->Rank[i], T->Fth[i]);
} //TarjanPrint()

/* ==================================== */
void TarjanMakeSet(Tarjan * T, int32_t x)
/* ==================================== */
/*! \fn void TarjanMakeSet(Tarjan * T, int32_t x)
    \param T: une structure Tarjan
    \param x: un élément
    \brief ajoute le singleton {x} à la famille d'ensembles
*/
{
  T->Fth[x] = x;
  T->Rank[x] = 0;
} //TarjanMakeSet()

/* ==================================== */
int32_t TarjanFind(Tarjan * T, int32_t x)
/* ==================================== */
/*! \fn int32_t TarjanFind(Tarjan * T, int32_t x)
    \param T: une structure Tarjan
    \param x: un élément
    \return un représentant
    \brief retourne le représentant de l'ensemble auquel appartient x
    \warning x doit appartenir à un ensemble de la famille - pas de vérification
*/
{
  if (T->Fth[x] != x) T->Fth[x] = TarjanFind(T, T->Fth[x]);
  return T->Fth[x];
} //TarjanFind()

/* ==================================== */
int32_t TarjanLink(Tarjan * T, int32_t x, int32_t y)
/* ==================================== */
/*! \fn int32_t TarjanLink(Tarjan * T, int32_t x, int32_t y)
    \param T: une structure Tarjan
    \param x, y: deux représentants
    \return un représentant
    \brief fusionne les ensembles représentés par x et y et retourne le représentant de la fusion
    \warning x et y doivent être des représentants - pas de vérification
*/
{
  if (T->Rank[x] > T->Rank[y]) { int32_t tmp = x; x = y; y = tmp; }
  if (T->Rank[x] == T->Rank[y]) T->Rank[y] += 1;
  T->Fth[x] = y;
  return y;
} //TarjanLink()

/* ==================================== */
int32_t TarjanLinkSafe(Tarjan * T, int32_t x, int32_t y)
/* ==================================== */
/*! \fn int32_t TarjanLinkSafe(Tarjan * T, int32_t x, int32_t y)
    \param T: une structure Tarjan
    \param x, y: deux éléments
    \return un représentant
    \brief fusionne les ensembles auxquels appartiennent x et y et retourne le représentant de la fusion
*/
{
  x = TarjanFind(T, x);
  y = TarjanFind(T, y);
  if (T->Rank[x] > T->Rank[y]) { int32_t tmp = x; x = y; y = tmp; }
  if (T->Rank[x] == T->Rank[y]) T->Rank[y] += 1;
  T->Fth[x] = y;
  return y;
} //TarjanLinkSafe()


#ifdef TESTUnionfind
int32_t main()
{
  Tarjan * T = CreeTarjan(7);
  char r[80];
  int32_t A, B;
  TarjanInit(T);

  do
  {
    printf("commande (qUIT, lINK, fIND, pRINT) > ");
    scanf("%s", r);
    switch (r[0])
    {
      case 'p': TarjanPrint(T); break;
      case 'f': scanf("%d", &A);
                A = TarjanFind(T, A);
                printf("Result = %d\n", A);
                break;
      case 'l': scanf("%d", &A);
                scanf("%d", &B);
                A = TarjanLinkSafe(T, A, B);
                printf("Result = %d\n", A);
                break;
      case 'q': break;
    }
  } while (r[0] != 'q');
  TarjanTermine(T);
  return 0;
}
#endif
