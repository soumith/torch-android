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
   Librairie mcrbt :

   Fonctions pour la gestion d'un arbre rouge et noir

   D'apres "Introduction a l'algorithmique", 
     T. Cormen, C. Leiserson, R. Rivest, pp. 258, Dunod Ed. 

   Michel Couprie - aout 2000

   Modif avril 2001: reallocation si depassement de capacite


   Un RBT (Red-Black Tree, arbre rouge et noir) est une structure de donnes
   arborescente permettant l'insertion, l'effacement et la consultation 
   d'un ensemble d'elements classes selon un ordre total, avec une complexite
   O(log(N)) en moyenne pour une de ces operations, avec N = cardinal de 
   l'ensemble.

   La structure de donnees utilisee dans l'implementation est la suivante pour
   un noeud de l'arbre (cf ../../include/mcrbt.h) : 

typedef struct RBTELT {
  TypRbtAuxData auxdata;
  TypRbtKey key;
  char color;
  struct RBTELT * left;
  struct RBTELT * right;
  struct RBTELT * parent;
} RbtElt;

  Le classement des elements s'opere selon la valeur du champ "key", qui est par defaut 
  un entier index_t mais peut facilement etre change en tout type scalaire (double, etc): 
  il suffit de changer la declaration de "TypRbtKey".

  Un champ "auxdata" est prevu pour contenir une donnee quelconque utile pour l'application,
  eventuellement un pointeur si des structures de donnees complexes doivent etre associees
  aux elements contenus dans l'arbre.

  Les champs "color", "left", "right" et "parent" sont utilisees pour maintenir la 
  structure de l'arbre.

  L'utilisateur manipule la structure RBT exclusivement a travers les fonctions suivantes:

  Rbt * mcrbt_CreeRbtVide(index_t taillemax) : 
     alloue l'espace memoire necessaire pour un RBT dont la taille n'excedera pas taillemax
     (cette taille est indicative, le RBT est realloue en cas de depassement)

  RbtElt * RbtSearch(Rbt * T, TypRbtKey k) : 
     recherche dans l'arbre T l'element ayant une valeur de "key" egale a k, retourne
     un pointeur sur l'element en cas de succes, ou RBT->nil sinon.

  RbtElt * mcrbt_RbtInsert(Rbt ** T, TypRbtKey k, TypRbtAuxData d) : 
     insere un nouvel element dans l'arbre T, de valeur "key" egale a k 
     et de valeur "auxdata" egale a d
     retourne un pointeur sur l'element insere

  void mcrbt_RbtDelete(Rbt * T, RbtElt * z) : 
     efface l'element z de l'arbre T
  
  int32_t RbtVide(Rbt * T) : 
     teste si l'arbre T est vide

  void RbtFlush(Rbt * T) :  
     re-initialise l'arbre T (ens. vide)

  TypRbtAuxData RbtPopMin(Rbt * T) :
     retire du RBT l'element de valeur "key" minimale, et retourne la valeur de son champ "auxdata"

  TypRbtAuxData RbtPopMax(Rbt * T) : 
     retire du RBT l'element de valeur "key" maximale, et retourne la valeur de son champ "auxdata"

  RbtElt * mcrbt_RbtMinimum(Rbt * T, RbtElt * x) : 
     retourne un pointeur sur l'element de valeur "key" minimum, dans la sous-arborescence de racine x
     de l'arbre T 

  RbtElt * RbtMaximum(Rbt * T, RbtElt * x) : 
     retourne un pointeur sur l'element de valeur "key" maximum, dans la sous-arborescence de racine x
     de l'arbre T 

  TypRbtKey RbtMin(Rbt * T) : 
     retourne la clé de l'element de valeur "key" minimum, dans l'arbre T 

  TypRbtKey RbtMax(Rbt * T) : 
     retourne la clé de l'element de valeur "key" maximum, dans l'arbre T 

  RbtElt * mcrbt_RbtSuccessor(Rbt * T, RbtElt * x) : 
     retourne un pointeur sur l'element de valeur "key" immediatement superieure a celle de l'element x

  void RbtPrint(Rbt * T) : 
     affiche le contenu du RBT (pour le DEBUG)

  void mcrbt_RbtTermine(Rbt * T) : 
     libere l'espace memoire occupe par l'arbre T

*/

#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include <assert.h>
#include <mcrbt.h>

/* #define TESTRBT */
/* #define VERBOSE */

//#define DEBUGDELETE
//#define DEBUGINSERT

/* ==================================== */
Rbt * mcrbt_CreeRbtVide(
  index_t taillemax)
/* ==================================== */
{
  index_t i;
  Rbt * T = (Rbt *)calloc(1,sizeof(Rbt) + taillemax*sizeof(RbtElt));
  /* le tableau Elts du Rbt peut stocker taillemax+1 elements, dont 1 pour nil */
  /* l'element 0 du tableau est reserve pour representer nil */
  if (T == NULL)
  {   fprintf(stderr, "mcrbt_CreeRbtVide() : malloc failed\n");
      return NULL;
  }
  T->max = taillemax;
  T->util = 0;
  T->maxutil = 0;
  T->nil = &(T->elts[0]);
  T->nil->left = T->nil->right = T->nil->parent = NULL;
  T->root = T->nil;

  /* chaine les elements libres a l'aide du pointeur right */
  for (i = 1; i < taillemax; i++) T->elts[i].right = &(T->elts[i+1]);
  T->elts[taillemax].right = NULL;
  T->libre = &(T->elts[1]);

  return T;
} /* mcrbt_CreeRbtVide() */

/* ==================================== */
void RbtTransRec(
  Rbt **T, Rbt * A, RbtElt * x)
/* ==================================== */
{
  if (x == A->nil) return;
  mcrbt_RbtInsert(T, x->key, x->auxdata);
  RbtTransRec(T, A, x->left);
  RbtTransRec(T, A, x->right);
} /* RbtTransRec() */

/* ==================================== */
void RbtReAlloc(Rbt **A)
/* ==================================== */
{
  index_t taillemax;
  Rbt * T, *Tmp;

#ifdef VERBOSE
#ifdef MC_64_BITS
  printf("RbtReAlloc: ancienne taille %lld nouvelle taille %lld\n", (*A)->max, 2 * (*A)->max);
#else
  printf("RbtReAlloc: ancienne taille %d nouvelle taille %d\n", (*A)->max, 2 * (*A)->max);
#endif
#endif
  taillemax = 2 * (*A)->max;  /* alloue le double de l'ancienne taille */ 
  T = mcrbt_CreeRbtVide(taillemax);
  RbtTransRec(&T, *A, (*A)->root);
  Tmp = *A;
  *A = T;
  free(Tmp);
} /* RbtReAlloc() */

/* ==================================== */
void RbtFlush(
  Rbt * T)
/* ==================================== */
{
  index_t i;
  T->util = 0;
  for (i = 0; i < T->max - 1; i++) T->elts[i].right = &(T->elts[i+1]);
  T->elts[T->max - 1].right = NULL;
  T->root = T->nil;
} /* RbtFlush() */

/* ==================================== */
int32_t mcrbt_RbtVide(
  Rbt * T)
/* ==================================== */
{
  return (T->util == 0);
} /* mcrbt_RbtVide() */

/* ==================================== */
void mcrbt_RbtTermine(
  Rbt * T)
/* ==================================== */
{
#ifdef VERBOSE
  printf("Rbt: taux d'utilisation: %g\n", (double)T->maxutil / (double)T->max);
#endif
  free(T);
} /* mcrbt_RbtTermine() */

/* ==================================== */
void RbtPrintRec(
  Rbt * T, RbtElt * x, index_t niv)
/* ==================================== */
{
  index_t i;
  if (x == T->nil) return;
  RbtPrintRec(T, x->left, niv+1);
  for (i = 0; i < niv; i++) printf("    ");
#ifdef MC_64_BITS
  printf("%g [%lld] (", x->key, x->auxdata);
#else
  printf("%g [%d] (", x->key, x->auxdata);
#endif
  if (x->color == RBT_Red) printf("r"); else  printf("b");
  printf(")\n");
  RbtPrintRec(T, x->right, niv+1);
} /* RbtPrintRec() */

/* ==================================== */
void RbtPrint(
  Rbt * T)
/* ==================================== */
{
  RbtPrintRec(T, T->root, 0);
} /* RbtPrint() */

/* ==================================== */
RbtElt * RbtSearch(
  Rbt * T, TypRbtKey k)
/* ==================================== */
{
  RbtElt * x = T->root;
  while ((x != T->nil) && (k != x->key))
    if (k < x->key) x = x->left; else x = x->right;
  return x;
} /* RbtSearch() */

/* ==================================== */
RbtElt * mcrbt_RbtMinimum(
  Rbt * T, RbtElt * x)
/* ==================================== */
{
  while (x->left != T->nil) x = x->left;
  return x;
} /* mcrbt_RbtMinimum() */

/* ==================================== */
RbtElt * RbtMaximum(
  Rbt * T, RbtElt * x)
/* ==================================== */
{
  while (x->right != T->nil) x = x->right;
  return x;
} /* RbtMaximum() */

/* ==================================== */
RbtElt * mcrbt_RbtSuccessor(
  Rbt * T, RbtElt * x)
/* ==================================== */
{
  RbtElt * y;
  if (x->right != T->nil) return mcrbt_RbtMinimum(T, x->right);
  y = x->parent;
  while ((y != T->nil) && (x == y->right))
  {
    x = y;
    y = y->parent;
  }
  return y;
} /* mcrbt_RbtSuccessor() */

/* ==================================== */
void mcrbt_RbtInsertSimple(
  Rbt * T, RbtElt * z)
/* ==================================== */
{
  RbtElt * x;
  RbtElt * y;

#ifdef DEBUGINSERT
printf("mcrbt_RbtInsertSimple  ");
printf("z=%x ; z->key = %g\n", (int32_t)z, z->key);
#endif

  y = T->nil;
  x = T->root;
  while (x != T->nil)
  {
    y = x;
    if (z->key < x->key) x = x->left; else x = x->right;
  }
  z->parent = y;
  if (y == T->nil)
    T->root = z;
  else
    if (z->key < y->key) y->left = z; else y->right = z;

#ifdef DEBUGINSERT
printf("FIN mcrbt_RbtInsertSimple\n");
#endif

} /* mcrbt_RbtInsertSimple() */

/* ==================================== */
RbtElt * mcrbt_RbtInsertAux(  /* allocation et insertion simple */
  Rbt ** T, TypRbtKey k, TypRbtAuxData d)
/* ==================================== */
{
  RbtElt * z;

#ifdef DEBUGINSERT
printf("mcrbt_RbtInsertAux\n");
#endif

  if ((*T)->libre == NULL) RbtReAlloc(T);
  (*T)->util++;
  if ((*T)->util > (*T)->maxutil) (*T)->maxutil = (*T)->util;
  z = (*T)->libre;
  (*T)->libre = (*T)->libre->right;
  z->key = k;
  z->auxdata = d;
  z->left = (*T)->nil;
  z->right = (*T)->nil;
  mcrbt_RbtInsertSimple((*T), z);

#ifdef DEBUGINSERT
printf("FIN mcrbt_RbtInsertAux\n");
#endif

  return z;
} /* mcrbt_RbtInsertAux() */

/* ==================================== */
static void LeftRotate(
  Rbt * T, RbtElt * x)
/* ==================================== */
{
  RbtElt * y;

  y = x->right;                    /* assume right(x) != NIL */
  x->right = y->left;              /* move y's child over */
  if (y->left != T->nil)
    y->left->parent = x;
  y->parent = x->parent;           /* move y up to x's position */
  if (x->parent == T->nil)
    T->root = y;
  else 
  {
    if (x == x->parent->left)
      x->parent->left = y;
    else x->parent->right = y;
  }
  y->left = x;                     /* move x down */
  x->parent = y;
} /* LeftRotate() */

/* ==================================== */
static void RightRotate(
  Rbt * T, RbtElt * x)
/* ==================================== */
{
  RbtElt * y;

  y = x->left;              /* assume left(x) != NIL */
  x->left = y->right;
  if (y->right != T->nil)
    y->right->parent = x;
  y->parent = x->parent;
  if (x->parent == T->nil)
    T->root = y;
  else 
  {
    if (x == x->parent->right)
       x->parent->right = y;
    else x->parent->left = y;
  }
  y->right = x;
  x->parent = y;
} /* RightRotate() */

/* ==================================== */
RbtElt * mcrbt_RbtInsert(
  Rbt ** T, TypRbtKey k, TypRbtAuxData d)
/* ==================================== */
{
  RbtElt * x;
  RbtElt * xc;            /* pour retourner le pointeur sur l'element alloue */
  RbtElt * uncle;

#ifdef DEBUGINSERT
printf("mcrbt_RbtInsert: data = %d ; key = %g\n", d, k);
#endif

  xc = x = mcrbt_RbtInsertAux(T, k, d);          /* allocation et insertion simple */
  x->color = RBT_Red;

  /* re-equilibrage de l'arbre */
  while ((x != (*T)->root) && (x->parent->color == RBT_Red))
  {
    if (x->parent == x->parent->parent->left)
    {
      uncle = x->parent->parent->right;
      if (uncle->color == RBT_Red)
      {
        x->parent->color = RBT_Black;                    /* Case I */
        uncle->color = RBT_Black;
        x->parent->parent->color = RBT_Red;
        x = x->parent->parent;
      }
      else 
      {
        if (x == x->parent->right)
        {
          x = x->parent;                             /* Case II */
          LeftRotate((*T),x);
        }
        x->parent->color = RBT_Black;                    /* Case III */
        x->parent->parent->color = RBT_Red;
        RightRotate((*T), x->parent->parent);
      }
    }
    else /* same as "then" with "right" and "left" swapped */
    {
      uncle = x->parent->parent->left;
      if (uncle->color == RBT_Red)
      {
        x->parent->color = RBT_Black;                     /* Case I */
        uncle->color = RBT_Black;
        x->parent->parent->color = RBT_Red;
        x = x->parent->parent;
      }
      else 
      {
        if (x == x->parent->left)
        {
          x = x->parent;                             /* Case II */
          RightRotate((*T),x);
        }
        x->parent->color = RBT_Black;                    /* Case III */
        x->parent->parent->color = RBT_Red;
        LeftRotate((*T), x->parent->parent);
      }
    }
  } /* while */
  (*T)->root->color = RBT_Black;

#ifdef DEBUGINSERT
printf("FIN mcrbt_RbtInsert xc->data = %d ; xc->key = %g\n", xc->auxdata, xc->key);
#endif

  assert(xc->auxdata == d);

  return xc;                      /* modif mc: retourne xc plutot que x (sinon: BUG) */
} /* mcrbt_RbtInsert() */

/* ==================================== */
void mcrbt_RbtDeleteFixup(
  Rbt * T, RbtElt * x)
/* ==================================== */
{
  RbtElt * s;

#ifdef DEBUGDELETE
printf("mcrbt_RbtDeleteFixup\n");
#endif

  while ((x != T->root) && (x->color == RBT_Black))
  {
    if (x == x->parent->left)
    {
      s = x->parent->right;               /* Get x's sibling */
      if (s->color == RBT_Red)
      {
        s->color = RBT_Black;              /* Case I */
        x->parent->color = RBT_Red;
        LeftRotate(T, x->parent);
        s = x->parent->right;
      }
      if ((s->left->color == RBT_Black) && (s->right->color == RBT_Black))
      {
        s->color = RBT_Red;                /* Case II */
        x = x->parent;
      }              
      else 
      {
        if (s->right->color == RBT_Black)
	{
          s->left->color = RBT_Black;      /* Case III */
          s->color = RBT_Red;                        
          RightRotate(T,s);
          s = x->parent->right;
        }
        s->color = x->parent->color;   /* Case IV */
        x->parent->color = RBT_Black;
        s->right->color = RBT_Black;
        LeftRotate(T, x->parent);                   
        x = T->root;
      }
    }
    else
    {            /* Same as "then" with right and left swapped */
      s = x->parent->left;               /* Get x's sibling */
      if (s->color == RBT_Red)
      {
        s->color = RBT_Black;              /* Case I */
        x->parent->color = RBT_Red;
        RightRotate(T, x->parent);
        s = x->parent->left;
      }
      if ((s->right->color == RBT_Black) && (s->left->color == RBT_Black))
      {
        s->color = RBT_Red;                /* Case II */
        x = x->parent;
      }              
      else 
      {
        if (s->left->color == RBT_Black)
	{
          s->right->color = RBT_Black;     /* Case III */
          s->color = RBT_Red;                        
          LeftRotate(T,s);
          s = x->parent->left;
        }
        s->color = x->parent->color;   /* Case IV */
        x->parent->color = RBT_Black;
        s->left->color = RBT_Black;
        RightRotate(T, x->parent);                   
        x = T->root;
      }
    }
  } /* while */
  x->color = RBT_Black;

#ifdef DEBUGDELETE
printf("FINmcrbt_RbtDeleteFixup\n");
#endif

} /* mcrbt_RbtDeleteFixup() */

/* ==================================== */
RbtElt * mcrbt_RbtDeleteAux(         /* return deleted node */
  Rbt * T, RbtElt * z)
/* ==================================== */
{
  RbtElt * c;
  RbtElt * d;

#ifdef DEBUGDELETE
printf("mcrbt_RbtDeleteAux\n");
#endif

  if ((z->left == T->nil) || (z->right == T->nil))
    d = z;
  else 
    d = mcrbt_RbtSuccessor(T, z);
  if (d->left != T->nil)
    c = d->left;
  else 
    c = d->right;
  c->parent = d->parent;      /* no test for NIL with sentinel */
  if (d->parent == T->nil)
    T->root = c;
  else 
  {
    if (d == d->parent->left)
      d->parent->left = c;
    else 
      d->parent->right = c;
  }
  if (d != z)
  {
    z->key = d->key;
    z->auxdata = d->auxdata;
  }
  if (d->color == RBT_Black)
    mcrbt_RbtDeleteFixup(T, c);     /* c is now "Double-Black" */

#ifdef DEBUGDELETE
printf("FIN mcrbt_RbtDeleteAux\n");
#endif

  return d;
} /* mcrbt_RbtDeleteAux() */

/* ==================================== */
void mcrbt_RbtDelete(
  Rbt * T, RbtElt * z)
/* ==================================== */
{

#ifdef DEBUGDELETE
printf("mcrbt_RbtDelete\n");
#endif

  z = mcrbt_RbtDeleteAux(T, z);
  z->right = T->libre;
  T->libre = z;
  T->util -= 1;

#ifdef DEBUGDELETE
printf("FIN mcrbt_RbtDelete\n");
#endif

} /* mcrbt_RbtDelete() */

/* ==================================== */
TypRbtAuxData RbtPopMin(
  Rbt * T)
/* ==================================== */
/* 
  Retire de l'arbre l'element de cle min.
  ATTENTION: pas de test arbre vide.
*/
{
  RbtElt * z = T->root;
  while (z->left != T->nil) z = z->left; /* recherche le min */
  z = mcrbt_RbtDeleteAux(T, z);                /* efface de l'arbre */
  z->right = T->libre;
  T->libre = z;
  T->util -= 1;
  return z->auxdata;
} /* RbtPopMin() */

/* ==================================== */
TypRbtAuxData RbtPopMax(
  Rbt * T)
/* ==================================== */
/* 
  Retire de l'arbre l'element de cle max.
  ATTENTION: pas de test arbre vide.
*/
{
  RbtElt * z = T->root;
  while (z->left != T->nil) z = z->right; /* recherche le max */
  z = mcrbt_RbtDeleteAux(T, z);                 /* efface de l'arbre */
  z->right = T->libre;
  T->libre = z;
  T->util -= 1;
  return z->auxdata;
} /* RbtPopMax() */

/* ==================================== */
TypRbtKey RbtMinLevel(
  Rbt * T)
/* ==================================== */
{
  RbtElt * x = T->root;
  while (x->left != T->nil) x = x->left;
  return x->key;
} /* RbtMinLevel() */

/* ==================================== */
TypRbtKey RbtMaxLevel(
  Rbt * T)
/* ==================================== */
{
  RbtElt * x = T->root;
  while (x->right != T->nil) x = x->right;
  return x->key;
} /* RbtMaxLevel() */

#ifdef TESTRBTINTERACTIVE
int32_t main()
{
  Rbt * T = mcrbt_CreeRbtVide(1);
  char r[80];
  double p;
  RbtElt * x;

  do
  {
    printf("commande (qUIT, PuSH, PoP, pRINT, TESTvIDE\n");
    printf("          sEARCH MiNIMUM MaXIMUM SUcCESSOR dELETE) > \n");
    scanf("%s", r);
    switch (r[0])
    {
      case 'u':
        printf("valeur > ");
        scanf("%lf", &p);
        (void)mcrbt_RbtInsert(&T, p, 0);
        break;
      case 'd':
        printf("valeur > ");
        scanf("%lf", &p);
        x = RbtSearch(T, p);
        if (x != T->nil) mcrbt_RbtDelete(T, x);
        else printf("pas trouve !\n");
        break;
      case 's':
        printf("valeur > ");
        scanf("%lf", &p);
        x = RbtSearch(T, p);
        printf("trouve: %d\n", x != T->nil);
        break;
      case 'i':
        x = mcrbt_RbtMinimum(T, T->root);
        printf("minimum: %g\n", x->key);
        break;
      case 'a':
        x = RbtMaximum(T, T->root);
        printf("maximum: %g\n", x->key);
        break;
      case 'c':
        printf("valeur > ");
        scanf("%lf", &p);
        x = RbtSearch(T, p);
        printf("trouve: %d\n", x != T->nil);
        if (x != T->nil)
	{
          x = mcrbt_RbtSuccessor(T, x);
          if (x != T->nil) printf("succ: %g\n", x->key);
	}
        break;
      case 'o': 
        if (mcrbt_RbtVide(T)) 
          printf("vide\n");
        else
          (void)RbtPopMin(T); 
        break;
      case 'p': RbtPrint(T); break;
      case 'v': printf("vide: %d\n", mcrbt_RbtVide(T)); break;
      case 'q': break;
    }
  } while (r[0] != 'q');
  mcrbt_RbtTermine(T);
}
#endif

#ifdef TESTRBTRANDOM
int32_t main()
{
  Rbt * T = mcrbt_CreeRbtVide(1);
  int32_t n = 0, d;

  do
  {
    if (rand()%2)
    {
       d = rand();
       (void)mcrbt_RbtInsert(&T, (double)d, d);
       n++;
       printf("j'insère %ld; n = %d\n", d, n);
    }
    else
    {
      if (mcrbt_RbtVide(T)) 
        printf("vide\n");
      else
      {
        d = RbtPopMin(T); 
        n--;
        printf("je retire %ld; n = %d\n", d, n);
      }
    }
  } while (1);
  mcrbt_RbtTermine(T);
}
#endif
