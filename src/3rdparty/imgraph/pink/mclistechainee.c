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
/***********************************/
/*   module de gestion de liste    */
/* Michel COUPRIE  -  Janvier 1990 */
/***********************************/

/* ATTENTION : allocation dynamique à chaque ajout d'élément - très inefficace
   utiliser plutôt mcliste.c
 */

/* ce module implemente le type abstrait:

   TYPE_ABSTRAIT Liste
   OPERATIONS
     ListeVide : -> Liste
     Tete      : Liste -> Element
     Suite     : Liste -> Liste
     Cons      : Element * Liste -> Liste
     Vide      : Liste -> Boolean
   RESTRICTIONS
     Tete(ListeVide)   = ERROR
     Suite(ListeVide)  = ERROR
   AXIOMES
     Tete(Cons(e,l))   = e
     Suite(Cons(e,l))  = l         
     Vide(ListeVide)   = TRUE
     Vide(Cons(e,l))   = FALSE
     
On pourra enrichir ce type par les operations:

     AfficheListe : Liste ->    (utilise AfficheElement)
     DetruitListe : Liste ->    (utilise DetruitElement)
     Longueur     : Liste -> Entier
     Concat       : Liste * Liste -> Liste
     Renverse     : Liste -> Liste
     InListe      : Element * Liste -> Liste
     Union        : Liste * Liste -> Liste
     etc...
     
*/

#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>

//#define TEST_LISTE            /* pour compiler la fonction de test */

/*************************/
/* structures de donnees */
/*************************/

#include "mclistechainee.h"

/*****************************/
/* definition des operations */
/*****************************/

/*************************************************/
TypListechainee * ListechaineeVide() 
{   /* retourne une listechainee vide */
  return(NULL);
} /* ListechaineeVide() */
  
/*************************************************/
int32_t EstVideListechainee(TypListechainee * l) 
{ 
                        /* retourne 1 si listechainee vide */
  if (l == NULL) return(1); else return(0);
} /* Vide() */
  
/*************************************************/
TypElement Tete (TypListechainee * lis) 
{
                        /* retourne l'element en tete
                           de la listechainee lis */
  return(lis->elt);
} /* Tete() */
  
/*************************************************/
TypListechainee * Suite (TypListechainee * lis) 
{
                        /* retourne la Suite de la listechainee lis,
                                 privee de son premier element */
  return(lis->suite);
} /* Suite() */

/*************************************************/
TypListechainee * Cons (TypElement el, TypListechainee * lis) 
{
                              /* retourne la listechainee dont la tete est el
                                 et dont la Suite est lis */
  TypListechainee * temp;
  temp = (TypListechainee *) malloc (sizeof (TypListechainee));
  temp->elt = el;
  temp->suite = lis;
  return(temp);
} /* Cons() */

/*************************************************/
void AfficheListechainee(TypListechainee * lis) 
{
                              /* affiche les elements de la listechainee */
  for (; lis != NULL; lis = lis->suite) 
    printf("%d ", lis->elt);
} /* AfficheListechainee() */

/*************************************************/
void DetruitListechainee(TypListechainee * lis) 
{
                              /* recupere la place occupee par les
                                 elements de la listechainee et par les
                                 liens du chainage. Attention: 
                                 ne modifie pas le contenu de lis */
  TypListechainee *temp, *temp1;
  for (temp = lis; temp != NULL; ) {
    temp1 = temp;
    temp = temp->suite;
    free(temp1);
  } /* for */
} /* DetruitListechainee() */

/*************************************************/
TypListechainee * InListechainee(TypElement el, TypListechainee * lis) 
{
                              /* retourne la premiere sous-listechainee qui a el en tete */
  for (; lis != NULL; lis = lis->suite)
    if (lis->elt == el) return lis;                        
  return NULL;
} /* InListechainee() */

/*************************************************/
TypListechainee * UnionListechainee (TypListechainee * lis1, TypListechainee * lis2) 
{
                              /* retourne la listechainee lis1 qui contient, si lis1 et lis2
                                 representent des ensembles, l'union des elements */
  for (; lis2 != NULL; lis2 = lis2->suite)
    if (InListechainee(lis2->elt, lis1) == NULL)
      lis1 = Cons(lis2->elt, lis1);
  return lis1;
} /* Union() */

/*******************************/
/* programme principal de test */
#ifdef TEST_LISTECHAINEE
main () {
  int32_t erreurs = 0;
  TypListechainee *listechainee1, *listechainee2;
                  
  listechainee1 = ListechaineeVide();
  if (!Vide(listechainee1)) {
    printf("Erreur 1: Vide(ListechaineeVide()) != 1\n");
    erreurs++;  
  }
  listechainee2 = Cons(1, listechainee1); /* (1) */
  if (Vide(listechainee2)) {
    printf("Erreur 2: Vide(Cons()) == 1\n");
    erreurs++;  
  }
  if (Tete(listechainee2) != 1) {
    printf("Erreur 3: Tete(Cons(e,l)) != e\n");
    erreurs++;
  }
  if (Suite(listechainee2) != listechainee1) {
    printf("Erreur 4: Suite(Cons(e,l)) != l\n");
    erreurs++;
  }
  listechainee1 = listechainee2;                 /* (1) */
  listechainee2 = Cons(2, listechainee1); /* (2 1) */
  if (Tete(listechainee2) != 2) {
    printf("Erreur 5: Tete(Cons(e,l)) != e\n");
    erreurs++;
  }
  if (Suite(listechainee2) != listechainee1) {
    printf("Erreur 6: Suite(Cons(e,l)) != l\n");
    erreurs++;
  }                    
  printf("affichage attendu = 2 1 ----> ");
  AfficheListechainee(listechainee2);                    
  printf("\n");

  listechainee1 = ListechaineeVide();
  listechainee1 = Union(listechainee1, listechainee2);
  printf("affichage attendu = 2 1 ----> ");
  AfficheListechainee(listechainee1);                    
  printf("\n");

  listechainee1 = ListechaineeVide();
  listechainee1 = Cons(1, listechainee1);
  listechainee1 = Union(listechainee1, listechainee2);
  printf("affichage attendu = 2 1 ----> ");
  AfficheListechainee(listechainee1);                    
  printf("\n");

  listechainee1 = ListechaineeVide();
  listechainee1 = Cons(3, listechainee1);
  listechainee1 = Union(listechainee1, listechainee2);
  printf("affichage attendu = 3 2 1 ----> ");
  AfficheListechainee(listechainee1);                    
  printf("\n");

  DetruitListechainee(listechainee2);
  printf("destruction listechainee effectuee\n");
  printf("%d erreurs\n", erreurs);
} /* main() */
#endif /* TEST_LISTECHAINEE */
