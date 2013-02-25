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
   Operateur de calcul de la ligne de partage des eaux
   d'apres "Un algorithme optimal de ligne de partage des eaux"
           F. Meyer - actes du 8eme congres AFCET - Lyon-Villeurbanne
           1991 
   variante de la section VI (ligne d'epaisseur 1 pixel)

   Utilise une File d'Attente Hierarchique.

   Michel Couprie - juin 1997 

   Update janvier 2000 : generation d'una animation (flag ANIMATE)
   Update janvier 2001 : mise a jour 2D-3D 
   Update juin 2004  : "écriture" d'une version sans ligne
*/

/*
*/
//#define ANIMATE

#define PARANO                 /* even paranoid people have ennemies */
//#define VERBOSE

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mcfah.h>
#include <mcindic.h>
#include <llpemeyer.h>
#include <mckhalimsky2d.h>

#define EN_FAH   0

/* ==================================== */
int32_t llpemeyer_NotIn(int32_t e, int32_t *list, int32_t n)                       
/* ==================================== */
{
/* renvoie 1 si e n'est pas dans list, 0 sinon */
/* e : l'element a rechercher */
/* list : la liste (tableau d'entiers) */
/* n : le nombre d'elements dans la liste */
  while (n > 0)
    if (list[--n] == e) return 0;
  return 1;
} /* llpemeyer_NotIn() */

/* ==================================== */
int32_t llpemeyer(
        struct xvimage *image,
        struct xvimage *marqueurs,
        struct xvimage *marqueursfond,
        struct xvimage *masque,
        int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "llpemeyer"
{
  register index_t x;                       /* index muet de pixel */
  register index_t y;                       /* index muet (generalement un voisin de x) */
  register index_t w;                       /* index muet (generalement un voisin de x) */
  register int32_t k;                       /* index muet */
  index_t rs = rowsize(image);     /* taille ligne */
  index_t cs = colsize(image);     /* taille colonne */
  index_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *B = UCHARDATA(marqueurs);       /* l'image de marqueurs */
  uint8_t *BF;                             /* l'image de marqueurs du fond */
  uint8_t *MA;                             /* l'image de masque */
  int32_t *M;             /* l'image d'etiquettes */
  index_t nlabels;                 /* nombre de labels differents */
  Fah * FAH;                   /* la file d'attente hierarchique */
  int32_t etiqcc[4];
  int32_t ncc;  
  int32_t incr_vois;
#ifdef ANIMATE
  int32_t curlev = -1, nimage = 0; 
  char imname[128];
  struct xvimage *animimage;
  uint8_t *A;
#endif

  if (datatype(image) != VFF_TYP_1_BYTE) 
  {
    fprintf(stderr, "%s: image type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (datatype(marqueurs) != VFF_TYP_1_BYTE) 
  {
    fprintf(stderr, "%s: marker type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (marqueursfond && (datatype(marqueursfond) != VFF_TYP_1_BYTE))
  {
    fprintf(stderr, "%s: bgnd marker type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (masque && (datatype(masque) != VFF_TYP_1_BYTE))
  {
    fprintf(stderr, "%s: mask type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    exit(0);
  }

  if ((rowsize(marqueurs) != rs) || (colsize(marqueurs) != cs))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }

  if (marqueursfond && ((rowsize(marqueursfond) != rs) || (colsize(marqueursfond) != cs)))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (masque && ((rowsize(masque) != rs) || (colsize(masque) != cs)))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (masque) MA = UCHARDATA(masque);
  if (marqueursfond) BF = UCHARDATA(marqueursfond);

  IndicsInit(N);
  FAH = CreeFahVide(N+1);
  if (FAH == NULL)
  {   fprintf(stderr, "%s : CreeFah failed\n", F_NAME);
      return(0);
  }

  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
    default: 
      fprintf(stderr, "%s: mauvaise connexite: %d\n", F_NAME, connex);
      return 0;
  } /* switch (connex) */    

  /* ================================================ */
  /* CREATION DES LABELS INITIAUX                     */
  /* ================================================ */

  M = (int32_t *)calloc(N, sizeof(int32_t));
  if (M == NULL)
  {   fprintf(stderr, "%s : calloc failed\n", F_NAME);
      return(0);
  }
  nlabels = 0;

  if (marqueursfond)
  {
    nlabels += 1;                 /* tous les marqueurs du fond ont le meme label (1) */
    for (x = 0; x < N; x++)
    {
      if (BF[x] && (M[x] == 0) && (!masque || MA[x]))
      {
        M[x] = nlabels;
        FahPush(FAH, x, 0);
        while (! FahVide(FAH))
        {
          w = FahPop(FAH);
          for (k = 0; k < 8; k += incr_vois)
          {
            y = voisin(w, k, rs, N);
            if ((y != -1) &&  BF[y] && (M[y] == 0) && (!masque || MA[y]))
            {
              M[y] = nlabels;
              FahPush(FAH, y, 0);
            } /* if y ... */
          } /* for k ... */
        } /* while (! FahVide(FAH)) */
      } /* if (M[x] == 0) */
    } /* for (x = 0; x < N; x++) */
  } /* if (marqueursfond) */

  for (x = 0; x < N; x++)
  {
    if (B[x] && (M[x] == 0) && (!masque || MA[x]))
    {
      nlabels += 1;
      M[x] = nlabels;
      FahPush(FAH, x, 0);
      while (! FahVide(FAH))
      {
        w = FahPop(FAH);
        for (k = 0; k < 8; k += incr_vois)
        {
          y = voisin(w, k, rs, N);
          if ((y != -1) &&  B[y] && (M[y] == 0) && (!masque || MA[y]))
          {
            M[y] = nlabels;
            FahPush(FAH, y, 0);
          } /* if y ... */
        } /* for k ... */
      } /* while (! FahVide(FAH)) */
    } /* if (M[x] == 0) */
  } /* for (x = 0; x < N; x++) */

  /* ================================================ */
  /* INITIALISATION DE LA FAH                         */
  /* ================================================ */

  FahFlush(FAH);
  FahPush(FAH, -1, 0);   /* force la creation du niveau 0 dans la Fah. */
                         /* NECESSAIRE pour eviter la creation prematuree */
                         /* de la file d'urgence */ 

  for (x = 0; x < N; x++)
  {
    if (M[x] && (!masque || MA[x]))            /* on va empiler les voisins des regions marquees */
    {
      for (k = 0; k < 8; k += incr_vois)
      {
        y = voisin(x, k, rs, N);
        if ((y != -1) && !M[y] && !IsSet(y, EN_FAH))
        {        
          FahPush(FAH, y, F[y]);
          Set(y, EN_FAH);
        }
      } /* for (k = 0; k < 8; k += 2) */
    } /* if (M[x]) */
  } /* for (x = 0; x < N; x++) */

  x = FahPop(FAH);
#ifdef PARANO
  if (x != -1)
  {   
     fprintf(stderr,"%s : ORDRE FIFO NON RESPECTE PAR LA FAH !!!\n", F_NAME);
     return(0);
  }
#endif

  /* ================================================ */
  /* INONDATION                                       */
  /* ================================================ */

#ifdef ANIMATE
  animimage = copyimage(image);
  A = UCHARDATA(animimage);
#endif
  nlabels += 1;          /* cree le label pour les points de la LPE */
  while (! FahVide(FAH))
  {
    x = FahPop(FAH);
    UnSet(x, EN_FAH);
#ifdef ANIMATE
    if (F[x] > curlev)
    {
      printf("Niveau %d\n", F[x]);
      sprintf(imname, "anim%03d.pgm", nimage); nimage++;
      for (y = 0; y < N; y++)
        if ((M[y] == nlabels) || (M[y] == 0)) A[y] = 255; else A[y] = 0;
      writeimage(animimage, imname);
      curlev = F[x];
    }
#endif

    ncc = 0;
    for (k = 0; k < 8; k += incr_vois)
    {
      y = voisin(x, k, rs, N);
      if ((y != -1) && (M[y] != 0) && (M[y] != nlabels) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
      {
        etiqcc[ncc] = M[y];        
        ncc += 1;
      }
    } /* for k */

    if (ncc == 1)
    {
      M[x] = etiqcc[0];
      for (k = 0; k < 8; k += incr_vois)
      {
        y = voisin(x, k, rs, N);     
        if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
        {          
          FahPush(FAH, y, F[y]); 
          Set(y, EN_FAH);
        } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
      } /* for k */
    } 
    else 
    if (ncc > 1)
    {
      M[x] = nlabels;
    }

  } /* while (! FahVide(FAH)) */
  /* FIN PROPAGATION */

  for (x = 0; x < N; x++)
  {
    if ((M[x] == nlabels) || (M[x] == 0)) F[x] = 255; else F[x] = 0;
  }

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahTermine(FAH);
  free(M);
  return(1);
} // llpemeyer()

/* ==================================== */
int32_t llpemeyer2(
        struct xvimage *image,
        struct xvimage *marqueurs,
        struct xvimage *masque,
        int32_t connex)
/* ==================================== */
// marqueurs: image initiale de labels
// le résultat du traitement se trouve dans marqueurs (image de labels)
// et dans image (binaire)
// LPE avec ligne de séparation
#undef F_NAME
#define F_NAME "llpemeyer2"
{
  register index_t x;                       /* index muet de pixel */
  register index_t y;                       /* index muet (generalement un voisin de x) */
  register int32_t k;                       /* index muet */
  index_t rs = rowsize(image);     /* taille ligne */
  index_t cs = colsize(image);     /* taille colonne */
  index_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);  /* l'image de depart */
  int32_t *M = SLONGDATA(marqueurs);   /* l'image de marqueurs */
  uint8_t *MA;                         /* l'image de masque */
  Fah * FAH;                                 /* la file d'attente hierarchique */
  int32_t etiqcc[4];
  int32_t ncc;  
  int32_t incr_vois;
  index_t nlabels;

  if (datatype(image) != VFF_TYP_1_BYTE) 
  {
    fprintf(stderr, "%s: image type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (datatype(marqueurs) != VFF_TYP_4_BYTE) 
  {
    fprintf(stderr, "%s: marker type must be VFF_TYP_4_BYTE\n", F_NAME);
    return 0;
  }

  if (masque && (datatype(masque) != VFF_TYP_1_BYTE))
  {
    fprintf(stderr, "%s: mask type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    exit(0);
  }

  if ((rowsize(marqueurs) != rs) || (colsize(marqueurs) != cs))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (masque && ((rowsize(masque) != rs) || (colsize(masque) != cs)))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (masque) MA = UCHARDATA(masque);

  if (datatype(marqueurs) != VFF_TYP_4_BYTE)
  {
    fprintf(stderr, "%s: marker image must be int32_t\n", F_NAME);
    return 0;
  }

  IndicsInit(N);
  FAH = CreeFahVide(N+1);
  if (FAH == NULL)
  {   fprintf(stderr, "%s : CreeFah failed\n", F_NAME);
      return(0);
  }

  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
    default: 
      fprintf(stderr, "%s: mauvaise connexite: %d\n", F_NAME, connex);
      return 0;
  } /* switch (connex) */    

  nlabels = 0;
  for (x = 0; x < N; x++) if (M[x] > nlabels) nlabels = M[x];

  /* ================================================ */
  /* INITIALISATION DE LA FAH                         */
  /* ================================================ */

  FahFlush(FAH);
  FahPush(FAH, -1, 0);   /* force la creation du niveau 0 dans la Fah. */
                         /* NECESSAIRE pour eviter la creation prematuree */
                         /* de la file d'urgence */ 

  for (x = 0; x < N; x++)
  {
    if (M[x] && (!masque || MA[x]))            /* on va empiler les voisins des regions marquees */
    {
      for (k = 0; k < 8; k += incr_vois)
      {
        y = voisin(x, k, rs, N);
        if ((y != -1) && !M[y] && !IsSet(y, EN_FAH))
        {        
          FahPush(FAH, y, F[y]);
          Set(y, EN_FAH);
        }
      } /* for (k = 0; k < 8; k += 2) */
    } /* if (M[x]) */
  } /* for (x = 0; x < N; x++) */

  x = FahPop(FAH);
#ifdef PARANO
  if (x != -1)
  {   
     fprintf(stderr,"%s : ORDRE FIFO NON RESPECTE PAR LA FAH !!!\n", F_NAME);
     return(0);
  }
#endif

  /* ================================================ */
  /* INONDATION                                       */
  /* ================================================ */

  nlabels += 1;          /* cree le label pour les points de la LPE */
  while (! FahVide(FAH))
  {
    x = FahPop(FAH);
    UnSet(x, EN_FAH);

    ncc = 0;
    for (k = 0; k < 8; k += incr_vois)
    {
      y = voisin(x, k, rs, N);
      if ((y != -1) && (M[y] != 0) && (M[y] != nlabels) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
      {
        etiqcc[ncc] = M[y];        
        ncc += 1;
      }
    } /* for k */

    if (ncc == 1)
    {
      M[x] = etiqcc[0];
      for (k = 0; k < 8; k += incr_vois)
      {
        y = voisin(x, k, rs, N);     
        if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
        {          
          FahPush(FAH, y, F[y]); 
          Set(y, EN_FAH);
        } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
      } /* for k */
    } 
    else 
    if (ncc > 1)
    {
      M[x] = nlabels;
    }
  } /* while (! FahVide(FAH)) */
  /* FIN PROPAGATION */

  for (x = 0; x < N; x++)
  {
    if ((M[x] == nlabels) || (M[x] == 0)) F[x] = 255; else F[x] = 0;
  }

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahTermine(FAH);
  return(1);
} // llpemeyer2()

/* ==================================== */
int32_t llpemeyer3(
        struct xvimage *image,
        struct xvimage *marqueurs,
        struct xvimage *masque,
        int32_t connex)
/* ==================================== */
// version sans ligne de séparation
// le résultat se trouve dans marqueurs
#undef F_NAME
#define F_NAME "llpemeyer3"
{
  register index_t x;                       /* index muet de pixel */
  register index_t y;                       /* index muet (generalement un voisin de x) */
  register int32_t k;                       /* index muet */
  index_t rs = rowsize(image);     /* taille ligne */
  index_t cs = colsize(image);     /* taille colonne */
  index_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);  /* l'image de depart */
  int32_t *M = SLONGDATA(marqueurs);   /* l'image de marqueurs */
  uint8_t *MA;                         /* l'image de masque */
  Fah * FAH;                                 /* la file d'attente hierarchique */
  int32_t etiqcc[4];
  int32_t ncc;  
  int32_t incr_vois;

  if (datatype(image) != VFF_TYP_1_BYTE) 
  {
    fprintf(stderr, "%s: image type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (datatype(marqueurs) != VFF_TYP_1_BYTE) 
  {
    fprintf(stderr, "%s: marker type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (masque && (datatype(masque) != VFF_TYP_1_BYTE))
  {
    fprintf(stderr, "%s: mask type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    exit(0);
  }

  if ((rowsize(marqueurs) != rs) || (colsize(marqueurs) != cs))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (masque && ((rowsize(masque) != rs) || (colsize(masque) != cs)))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (masque) MA = UCHARDATA(masque);

  if (datatype(marqueurs) != VFF_TYP_4_BYTE)
  {
    fprintf(stderr, "%s: marker image must be int32_t\n", F_NAME);
    return 0;
  }

  IndicsInit(N);
  FAH = CreeFahVide(N+1);
  if (FAH == NULL)
  {   fprintf(stderr, "%s : CreeFah failed\n", F_NAME);
      return(0);
  }

  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
    default: 
      fprintf(stderr, "%s: mauvaise connexite: %d\n", F_NAME, connex);
      return 0;
  } /* switch (connex) */    

  /* ================================================ */
  /* INITIALISATION DE LA FAH                         */
  /* ================================================ */

  FahFlush(FAH);
  FahPush(FAH, -1, 0);   /* force la creation du niveau 0 dans la Fah. */
                         /* NECESSAIRE pour eviter la creation prematuree */
                         /* de la file d'urgence */ 

  for (x = 0; x < N; x++)
  {
    if (M[x] && (!masque || MA[x]))            /* on va empiler les voisins des regions marquees */
    {
      for (k = 0; k < 8; k += incr_vois)
      {
        y = voisin(x, k, rs, N);
        if ((y != -1) && !M[y] && !IsSet(y, EN_FAH) && (!masque || MA[y]))
        {        
          FahPush(FAH, y, F[y]);
          Set(y, EN_FAH);
        }
      } /* for (k = 0; k < 8; k += 2) */
    } /* if (M[x]) */
  } /* for (x = 0; x < N; x++) */

  x = FahPop(FAH);
#ifdef PARANO
  if (x != -1)
  {   
     fprintf(stderr,"%s : ORDRE FIFO NON RESPECTE PAR LA FAH !!!\n", F_NAME);
     return(0);
  }
#endif

  /* ================================================ */
  /* INONDATION                                       */
  /* ================================================ */

  while (! FahVide(FAH))
  {
    x = FahPop(FAH);
    UnSet(x, EN_FAH);

    ncc = 0;
    for (k = 0; k < 8; k += incr_vois)
    {
      y = voisin(x, k, rs, N);
      if ((y != -1) && (M[y] != 0) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
      {
        etiqcc[ncc] = M[y];        
        ncc += 1;
      }
    } /* for k */

    M[x] = etiqcc[0];

    for (k = 0; k < 8; k += incr_vois)
    {
      y = voisin(x, k, rs, N);     
      if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
      {          
	FahPush(FAH, y, F[y]); 
	Set(y, EN_FAH);
      } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
    } /* for k */
  } /* while (! FahVide(FAH)) */
  /* FIN PROPAGATION */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahTermine(FAH);
  return(1);
} // llpemeyer3()

 /* ==================================== */
 int32_t llpemeyerkhalimsky(
         struct xvimage *image,
         struct xvimage *marqueurs,
         struct xvimage *marqueursfond,
         struct xvimage *masque)
 /* ==================================== */
#undef F_NAME
#define F_NAME "llpemeyerkhalimsky"
 /* 
 Pour forcer la lpe a passer par des elements de rang < 2,
 l'image "image" doit avoir ete generee a partir de Z2
 en utilisant la  strategie Max pour passer dans la grille de Khalimsky.
 De plus, les elements de rang 0 et 1, a ndg egal, ont une priorite 
 superieure a celle des elements de rang 2.
 */

 {
   register index_t x;                       /* index muet de pixel */
   register index_t y;                       /* index muet (generalement un voisin de x) */
   register index_t w;                       /* index muet (generalement un voisin de x) */
   register int32_t k;                       /* index muet */
   index_t rs = rowsize(image);     /* taille ligne */
   index_t cs = colsize(image);     /* taille colonne */
   index_t N = rs * cs;             /* taille image */
   uint8_t *F = UCHARDATA(image);      /* l'image de depart */
   uint8_t *B = UCHARDATA(marqueurs);       /* l'image de marqueurs */
   uint8_t *BF;                             /* l'image de marqueurs du fond */
   uint8_t *MA;                             /* l'image de masque */
   int32_t *M;             /* l'image d'etiquettes */
   index_t nlabels;                 /* nombre de labels differents */
   Fah * FAH;                   /* la file d'attente hierarchique */
   int32_t etiqcc[4];
   int32_t ncc;  
   index_t tab[27]; 
   int32_t n;

  if (datatype(image) != VFF_TYP_1_BYTE) 
  {
    fprintf(stderr, "%s: image type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (datatype(marqueurs) != VFF_TYP_1_BYTE) 
  {
    fprintf(stderr, "%s: marker type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (marqueursfond && (datatype(marqueursfond) != VFF_TYP_1_BYTE))
  {
    fprintf(stderr, "%s: bgnd marker type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (masque && (datatype(masque) != VFF_TYP_1_BYTE))
  {
    fprintf(stderr, "%s: mask type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

   if (depth(image) != 1) 
   {
     fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
     exit(0);
   }

   if ((rowsize(marqueurs) != rs) || (colsize(marqueurs) != cs))
   {
     fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
     return 0;
   }

   if (marqueursfond && ((rowsize(marqueursfond) != rs) || (colsize(marqueursfond) != cs)))
   {
     fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
     return 0;
   }
   if (marqueursfond) BF = UCHARDATA(marqueursfond);
   if (masque && ((rowsize(masque) != rs) || (colsize(masque) != cs)))
   {
     fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
     return 0;
   }
   if (masque) MA = UCHARDATA(masque);

   IndicsInit(N);
   FAH = CreeFahVide(N+1);
   if (FAH == NULL)
   {   fprintf(stderr, "%s() : CreeFah failed\n", F_NAME);
       return(0);
   }

   /* ================================================ */
   /* CREATION DES LABELS INITIAUX                     */
   /* ================================================ */

   M = (int32_t *)calloc(N, sizeof(int32_t));
   if (M == NULL)
   {   fprintf(stderr, "%s() : calloc failed\n", F_NAME);
       return(0);
   }
   nlabels = 0;

   if (marqueursfond)
   {
     nlabels += 1;                 /* tous les marqueurs du fond ont le meme label (1) */
     for (x = 0; x < N; x++)
     {
       if (BF[x] && (M[x] == 0) && (!masque || MA[x]))
       {
         M[x] = nlabels;
         FahPush(FAH, x, 0);
         while (! FahVide(FAH))
         {
           w = FahPop(FAH);
           Thetacarre2d(rs, cs, w%rs, w/rs, tab, &n);
           for (k = 0; k < n; k++) /* parcourt les eventuels theta-voisins */
           {
             y = tab[k];
             if (BF[y] && (M[y] == 0) && (!masque || MA[y]))
             {
               M[y] = nlabels;
               FahPush(FAH, y, 0);
             } /* if y ... */
           } /* for k ... */
         } /* while (! FahVide(FAH)) */
       } /* if (M[x] == 0) */
     } /* for (x = 0; x < N; x++) */
   } /* if (marqueursfond) */

   for (x = 0; x < N; x++)
   {
     if (B[x] && (M[x] == 0) && (!masque || MA[x]))
     {
       nlabels += 1;
       M[x] = nlabels;
       FahPush(FAH, x, 0);
       while (! FahVide(FAH))
       {
         w = FahPop(FAH);
         Thetacarre2d(rs, cs, w%rs, w/rs, tab, &n);
         for (k = 0; k < n; k++) /* parcourt les eventuels theta-voisins */
         {
           y = tab[k];
           if (B[y] && (M[y] == 0) && (!masque || MA[y]))
           {
             M[y] = nlabels;
             FahPush(FAH, y, 0);
           } /* if y ... */
         } /* for k ... */
       } /* while (! FahVide(FAH)) */
     } /* if (M[x] == 0) */
   } /* for (x = 0; x < N; x++) */

   /* ================================================ */
   /* INITIALISATION DE LA FAH                         */
   /* ================================================ */

   FahFlush(FAH);
   FahPush(FAH, -1, 0);   /* force la creation du niveau 0 dans la Fah. */
                          /* NECESSAIRE pour eviter la creation prematuree */
                          /* de la file d'urgence */ 

   for (x = 0; x < N; x++)
   {
     if (M[x] && (!masque || MA[x]))            /* on va empiler les voisins des regions marquees */
     {
       Thetacarre2d(rs, cs, x%rs, x/rs, tab, &n);
       for (k = 0; k < n; k++) /* parcourt les eventuels theta-voisins */
       {
         y = tab[k]; 
         if (!M[y] && !IsSet(y, EN_FAH))
         { 
           if (CARRE(y%rs,y/rs))           
           FahPush(FAH, y, F[y]*2);
           else FahPush(FAH, y, F[y]*2+1); //plus grande priorite pour les elements de rang <2
           Set(y, EN_FAH);
         }
       } /* for (k = 0; k < 8; k += 2) */
     } /* if (M[x]) */
   } /* for (x = 0; x < N; x++) */

   x = FahPop(FAH);
 #ifdef PARANO
   if (x != -1)
   {   
      fprintf(stderr,"%s : ORDRE FIFO NON RESPECTE PAR LA FAH !!!\n", F_NAME);
      return(0);
   }
 #endif

   /* ================================================ */
   /* INONDATION                                       */
   /* ================================================ */

   nlabels += 1;          /* cree le label pour les points de la LPE */
   while (! FahVide(FAH))
   {
     x = FahPop(FAH);
     UnSet(x, EN_FAH);

     ncc = 0;
     Thetacarre2d(rs, cs, x%rs, x/rs, tab, &n);
     for (k = 0; k < n; k++) /* parcourt les eventuels theta-voisins */
     {
       y = tab[k];
       if ((M[y] != 0) && (M[y] != nlabels) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
       {
         etiqcc[ncc] = M[y];        
         ncc += 1;
       }
     } /* for k */

     if (ncc == 1)
     {
       M[x] = etiqcc[0];
       Thetacarre2d(rs, cs, x%rs, x/rs, tab, &n);
       for (k = 0; k < n; k++) /* parcourt les eventuels theta-voisins */
       {
         y = tab[k];
         if ((M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
         {
           if (CARRE(y%rs,y/rs))    
           FahPush(FAH, y, F[y]*2);
           else FahPush(FAH, y, F[y]*2+1); // priorite superieure pour les element de rang <2
           Set(y, EN_FAH);
         } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
       } /* for k */
     } 
     else 
     if (ncc > 1)
     {
       M[x] = nlabels;
     }

   } /* while (! FahVide(FAH)) */
   /* FIN PROPAGATION */

   for (x = 0; x < N; x++)
   {
     if ((M[x] == nlabels) || (M[x] == 0)) F[x] = 255; else F[x] = 0;
   }

   /* ================================================ */
   /* UN PEU DE MENAGE                                 */
   /* ================================================ */

   IndicsTermine();
   FahTermine(FAH);
   free(M);
   return(1);
 } /* llpemeyerkhalimsky() */

/* ==================================== */
int32_t llpemeyersansligne(
        struct xvimage *image,
        struct xvimage *marqueurs,
        struct xvimage *marqueursfond,
        struct xvimage *masque,
        int32_t connex,
        struct xvimage *result	
)
/* ==================================== */
#undef F_NAME
#define F_NAME "llpemeyersansligne"
{
  register index_t x;                       /* index muet de pixel */
  register index_t y;                       /* index muet (generalement un voisin de x) */
  register index_t w;                       /* index muet (generalement un voisin de x) */
  register int32_t k;                       /* index muet */
  index_t rs = rowsize(image);     /* taille ligne */
  index_t cs = colsize(image);     /* taille colonne */
  index_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *B = UCHARDATA(marqueurs);       /* l'image de marqueurs */
  uint8_t *BF;                             /* l'image de marqueurs du fond */
  uint8_t *MA;                             /* l'image de masque */
  int32_t *M;             /* l'image d'etiquettes */
  index_t nlabels;                 /* nombre de labels differents */
  Fah * FAH;                   /* la file d'attente hierarchique */
  int32_t etiqcc[4];
  int32_t ncc;  
  int32_t incr_vois;
#ifdef ANIMATE
  int32_t curlev = -1, nimage = 0; 
  char imname[128];
  struct xvimage *animimage;
  uint8_t *A;
#endif

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: net yet implemented for 3D images\n", F_NAME);
    exit(0);
  }

  if (datatype(image) != VFF_TYP_1_BYTE) 
  {
    fprintf(stderr, "%s: image type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (datatype(marqueurs) != VFF_TYP_1_BYTE) 
  {
    fprintf(stderr, "%s: marker type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (marqueursfond && (datatype(marqueursfond) != VFF_TYP_1_BYTE))
  {
    fprintf(stderr, "%s: bgnd marker type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (masque && (datatype(masque) != VFF_TYP_1_BYTE))
  {
    fprintf(stderr, "%s: mask type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (datatype(result) != VFF_TYP_4_BYTE) 
  {
    fprintf(stderr, "%s: result type must be VFF_TYP_4_BYTE\n", F_NAME);
    return 0;
  }

  if ((rowsize(marqueurs) != rs) || (colsize(marqueurs) != cs))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }

  if (marqueursfond && ((rowsize(marqueursfond) != rs) || (colsize(marqueursfond) != cs)))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (masque && ((rowsize(masque) != rs) || (colsize(masque) != cs)))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (masque) MA = UCHARDATA(masque);
  if (marqueursfond) BF = UCHARDATA(marqueursfond);

  IndicsInit(N);
  FAH = CreeFahVide(N+1);
  if (FAH == NULL)
  {   fprintf(stderr, "%s : CreeFah failed\n", F_NAME);
      return(0);
  }

  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
    default: 
      fprintf(stderr, "%s: mauvaise connexite: %d\n", F_NAME, connex);
      return 0;
  } /* switch (connex) */    

  /* ================================================ */
  /* CREATION DES LABELS INITIAUX                     */
  /* ================================================ */

  M = SLONGDATA(result);
  memset(M, 0, N*sizeof(int32_t));
  nlabels = 0;

  if (marqueursfond)
  {
    nlabels += 1;                 /* tous les marqueurs du fond ont le meme label (1) */
    for (x = 0; x < N; x++)
    {
      if ((M[x] == 0) && (!masque || MA[x]))
      {
        M[x] = nlabels;
        FahPush(FAH, x, 0);
        while (! FahVide(FAH))
        {
          w = FahPop(FAH);
          for (k = 0; k < 8; k += incr_vois)
          {
            y = voisin(w, k, rs, N);
            if ((y != -1) && (M[y] == 0) && (!masque || MA[y]))
            {
              M[y] = nlabels;
              FahPush(FAH, y, 0);
            } /* if y ... */
          } /* for k ... */
        } /* while (! FahVide(FAH)) */
      } /* if (M[x] == 0) */
    } /* for (x = 0; x < N; x++) */
  } /* if (marqueursfond) */

  for (x = 0; x < N; x++)
  {
    if (B[x] && (M[x] == 0) && (!masque || MA[x]))
    {
      nlabels += 1;
      M[x] = nlabels;
      FahPush(FAH, x, 0);
      while (! FahVide(FAH))
      {
        w = FahPop(FAH);
        for (k = 0; k < 8; k += incr_vois)
        {
          y = voisin(w, k, rs, N);
          if ((y != -1) &&  B[y] && (M[y] == 0) && (!masque || MA[y]))
          {
            M[y] = nlabels;
            FahPush(FAH, y, 0);
          } /* if y ... */
        } /* for k ... */
      } /* while (! FahVide(FAH)) */
    } /* if (M[x] == 0) */
  } /* for (x = 0; x < N; x++) */

  /* ================================================ */
  /* INITIALISATION DE LA FAH                         */
  /* ================================================ */

  FahFlush(FAH);
  FahPush(FAH, -1, 0);   /* force la creation du niveau 0 dans la Fah. */
                         /* NECESSAIRE pour eviter la creation prematuree */
                         /* de la file d'urgence */ 

  for (x = 0; x < N; x++)
  {
    if (M[x] && (!masque || MA[x]))            /* on va empiler les voisins des regions marquees */
    {
      for (k = 0; k < 8; k += incr_vois)
      {
        y = voisin(x, k, rs, N);
        if ((y != -1) && !M[y] && !IsSet(y, EN_FAH))
        {        
          FahPush(FAH, y, F[y]);
          Set(y, EN_FAH);
        }
      } /* for (k = 0; k < 8; k += 2) */
    } /* if (M[x]) */
  } /* for (x = 0; x < N; x++) */

  x = FahPop(FAH);
#ifdef PARANO
  if (x != -1)
  {   
     fprintf(stderr,"%s : ORDRE FIFO NON RESPECTE PAR LA FAH !!!\n", F_NAME);
     return(0);
  }
#endif

  /* ================================================ */
  /* INONDATION                                       */
  /* ================================================ */

#ifdef ANIMATE
  animimage = copyimage(image);
  A = UCHARDATA(animimage);
#endif
  nlabels += 1;          /* cree le label pour les points de la LPE */
  while (! FahVide(FAH))
  {
    x = FahPop(FAH);
    UnSet(x, EN_FAH);
#ifdef ANIMATE
    if (F[x] > curlev)
    {
      printf("Niveau %d\n", F[x]);
      sprintf(imname, "anim%03d.pgm", nimage); nimage++;
      for (y = 0; y < N; y++)
        if ((M[y] == nlabels) || (M[y] == 0)) A[y] = 255; else A[y] = 0;
      writeimage(animimage, imname);
      curlev = F[x];
    }
#endif

    ncc = 0;
    for (k = 0; k < 8; k += incr_vois)
    {
      y = voisin(x, k, rs, N);
      if ((y != -1) && (M[y] != 0) && (M[y] != nlabels) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
      {
        etiqcc[ncc] = M[y];        
        ncc += 1;
      }
    } /* for k */

    /* Here is the labelling */
    M[x] = etiqcc[0];
    for (k = 0; k < 8; k += incr_vois)
    {
      y = voisin(x, k, rs, N);     
      if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
      {          
	FahPush(FAH, y, F[y]); 
	Set(y, EN_FAH);
      } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
    } /* for k */

  } /* while (! FahVide(FAH)) */
  /* FIN PROPAGATION */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahTermine(FAH);
  return(1);
} // llpemeyersansligne()

/* ==================================== */
int32_t llpemeyersanslignelab(
        struct xvimage *image,
        struct xvimage *marqueurs, // marqueur initial et resultat
        struct xvimage *masque,
        int32_t connex
)
/* ==================================== */
#undef F_NAME
#define F_NAME "llpemeyersanslignelab"
{
  register index_t x;                       /* index muet de pixel */
  register index_t y;                       /* index muet (generalement un voisin de x) */
  register int32_t k;                       /* index muet */
  index_t rs = rowsize(image);     /* taille ligne */
  index_t cs = colsize(image);     /* taille colonne */
  index_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  int32_t *M = SLONGDATA(marqueurs);      /* l'image de marqueurs */
  uint8_t *MA;                             /* l'image de masque */
  Fah * FAH;                   /* la file d'attente hierarchique */
  int32_t etiqcc[4];
  int32_t ncc;  
  int32_t incr_vois;

  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: net yet implemented for 3D images\n", F_NAME);
    exit(0);
  }

  if (datatype(image) != VFF_TYP_1_BYTE) 
  {
    fprintf(stderr, "%s: image type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (datatype(marqueurs) != VFF_TYP_4_BYTE) 
  {
    fprintf(stderr, "%s: marker type must be VFF_TYP_4_BYTE\n", F_NAME);
    return 0;
  }

  if (masque && (datatype(masque) != VFF_TYP_1_BYTE))
  {
    fprintf(stderr, "%s: mask type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if ((rowsize(marqueurs) != rs) || (colsize(marqueurs) != cs))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }

  if (masque && ((rowsize(masque) != rs) || (colsize(masque) != cs)))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (masque) MA = UCHARDATA(masque);

  IndicsInit(N);
  FAH = CreeFahVide(N+1);
  if (FAH == NULL)
  {   fprintf(stderr, "%s : CreeFah failed\n", F_NAME);
      return(0);
  }

  switch (connex)
  {
    case 4: incr_vois = 2; break;
    case 8: incr_vois = 1; break;
    default: 
      fprintf(stderr, "%s: mauvaise connexite: %d\n", F_NAME, connex);
      return 0;
  } /* switch (connex) */    

  /* ================================================ */
  /* INITIALISATION DE LA FAH                         */
  /* ================================================ */

  FahFlush(FAH);
  FahPush(FAH, -1, 0);   /* force la creation du niveau 0 dans la Fah. */
                         /* NECESSAIRE pour eviter la creation prematuree */
                         /* de la file d'urgence */ 

  for (x = 0; x < N; x++)
  {
    if (M[x] && (!masque || MA[x]))            /* on va empiler les voisins des regions marquees */
    {
      for (k = 0; k < 8; k += incr_vois)
      {
        y = voisin(x, k, rs, N);
        if ((y != -1) && !M[y] && !IsSet(y, EN_FAH))
        {        
          FahPush(FAH, y, F[y]);
          Set(y, EN_FAH);
        }
      } /* for (k = 0; k < 8; k += 2) */
    } /* if (M[x]) */
  } /* for (x = 0; x < N; x++) */

  x = FahPop(FAH);
#ifdef PARANO
  if (x != -1)
  {   
     fprintf(stderr,"%s : ORDRE FIFO NON RESPECTE PAR LA FAH !!!\n", F_NAME);
     return(0);
  }
#endif

  /* ================================================ */
  /* INONDATION                                       */
  /* ================================================ */

  while (! FahVide(FAH))
  {
    x = FahPop(FAH);
    UnSet(x, EN_FAH);

    ncc = 0;
    for (k = 0; k < 8; k += incr_vois)
    {
      y = voisin(x, k, rs, N);
      if ((y != -1) && (M[y] != 0) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
      {
        etiqcc[ncc] = M[y];        
        ncc += 1;
      }
    } /* for k */

    /* Here is the labelling */
    M[x] = etiqcc[0];
    for (k = 0; k < 8; k += incr_vois)
    {
      y = voisin(x, k, rs, N);     
      if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
      {          
	FahPush(FAH, y, F[y]); 
	Set(y, EN_FAH);
      } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
    } /* for k */

  } /* while (! FahVide(FAH)) */
  /* FIN PROPAGATION */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahTermine(FAH);
  return(1);
} // llpemeyersanslignelab()

/* ==================================== */
int32_t llpemeyer3d(
        struct xvimage *image,
        struct xvimage *marqueurs,
        struct xvimage *marqueursfond,
        struct xvimage *masque,
        int32_t connex)
/* ==================================== */
#undef F_NAME
#define F_NAME "llpemeyer3d"
{
  register index_t x;                       /* index muet de pixel */
  register index_t y;                       /* index muet (generalement un voisin de x) */
  register index_t w;                       /* index muet (generalement un voisin de x) */
  register int32_t k;                       /* index muet */
  index_t rs = rowsize(image);     /* taille ligne */
  index_t cs = colsize(image);     /* taille colonne */
  index_t d = depth(image);        /* nb plans */
  index_t n = rs * cs;             /* taille plan */
  index_t N = n * d;               /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *B = UCHARDATA(marqueurs);       /* l'image de marqueurs */
  uint8_t *BF;                             /* l'image de marqueurs du fond */
  uint8_t *MA;                             /* l'image de masque */
  int32_t *M;             /* l'image d'etiquettes */
  index_t nlabels;                 /* nombre de labels differents */
  Fah * FAH;                   /* la file d'attente hierarchique */
  int32_t etiqcc[6];
  int32_t ncc;  

  if (datatype(image) != VFF_TYP_1_BYTE) 
  {
    fprintf(stderr, "%s: image type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (datatype(marqueurs) != VFF_TYP_1_BYTE) 
  {
    fprintf(stderr, "%s: marker type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (marqueursfond && (datatype(marqueursfond) != VFF_TYP_1_BYTE))
  {
    fprintf(stderr, "%s: bgnd marker type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (masque && (datatype(masque) != VFF_TYP_1_BYTE))
  {
    fprintf(stderr, "%s: mask type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if ((rowsize(marqueurs) != rs) || (colsize(marqueurs) != cs))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }

  if (marqueursfond && ((rowsize(marqueursfond) != rs) || (colsize(marqueursfond) != cs) || (depth(marqueursfond) != d)))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (marqueursfond) BF = UCHARDATA(marqueursfond);
  if (masque && ((rowsize(masque) != rs) || (colsize(masque) != cs) || (depth(masque) != d)))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (masque) MA = UCHARDATA(masque);

  IndicsInit(N);
  FAH = CreeFahVide(N+1);
  if (FAH == NULL)
  {   fprintf(stderr, "%s() : CreeFah failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /* CREATION DES LABELS INITIAUX                     */
  /* ================================================ */

  M = (int32_t *)calloc(N, sizeof(int32_t));
  if (M == NULL)
  {   fprintf(stderr, "%s() : calloc failed\n", F_NAME);
      return(0);
  }
  nlabels = 0;

  if (marqueursfond)
  {
    nlabels += 1;                 /* tous les marqueurs du fond ont le meme label (1) */
    for (x = 0; x < N; x++)
    {
      if (BF[x] && (M[x] == 0) && (!masque || MA[x]))
      {
        M[x] = nlabels;
        FahPush(FAH, x, 0);
        while (! FahVide(FAH))
        {
          w = FahPop(FAH);
          switch (connex)
          {
	    case 6:
              for (k = 0; k <= 10; k += 2) /* parcourt les 6 voisins */
              {
                y = voisin6(w, k, rs, n, N);
                if ((y != -1) && BF[y] && (M[y] == 0) && (!masque || MA[y]))
                { M[y] = nlabels; FahPush(FAH, y, 0); }
              } /* for k ... */
              break;
	    case 18:
              for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
              {
                y = voisin18(w, k, rs, n, N);
                if ((y != -1) && BF[y] && (M[y] == 0) && (!masque || MA[y]))
                { M[y] = nlabels; FahPush(FAH, y, 0); }
              } /* for k ... */
              break;
	    case 26:
              for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
              {
                y = voisin26(w, k, rs, n, N);
                if ((y != -1) && BF[y] && (M[y] == 0) && (!masque || MA[y]))
                { M[y] = nlabels; FahPush(FAH, y, 0); }
              } /* for k ... */
              break;
	  } /* switch (connex) */
        } /* while (! FahVide(FAH)) */
      } /* if (M[x] == 0) */
    } /* for (x = 0; x < N; x++) */
  } /* if (marqueursfond) */

  for (x = 0; x < N; x++)
  {
    if (B[x] && (M[x] == 0) && (!masque || MA[x]))
    {
      nlabels += 1;
      M[x] = nlabels;
      FahPush(FAH, x, 0);
      while (! FahVide(FAH))
      {
        w = FahPop(FAH);
        switch (connex)
        {
	  case 6:
            for (k = 0; k <= 10; k += 2)
            {
              y = voisin6(w, k, rs, n, N);
              if ((y!=-1) && (M[y]==0) && (B[y]==B[w]) && (!masque || MA[y]))
              { M[y] = nlabels; FahPush(FAH, y, 0); } /* if y ... */
            } /* for k ... */
            break;
	  case 18:
            for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
            {
              y = voisin18(w, k, rs, n, N);
              if ((y!=-1) && (M[y]==0) && (B[y]==B[w]) && (!masque || MA[y]))
              { M[y] = nlabels; FahPush(FAH, y, 0); } /* if y ... */
            } /* for k ... */
            break;
	  case 26:
            for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
            {
              y = voisin26(w, k, rs, n, N);
              if ((y!=-1) && (M[y]==0) && (B[y]==B[w]) && (!masque || MA[y]))
              { M[y] = nlabels; FahPush(FAH, y, 0); } /* if y ... */
            } /* for k ... */
            break;
	} /* switch (connex) */
      } /* while (! FahVide(FAH)) */
    } /* if (M[x] == 0) */
  } /* for (x = 0; x < N; x++) */

  /* ================================================ */
  /* INITIALISATION DE LA FAH                         */
  /* ================================================ */

  FahFlush(FAH);
  FahPush(FAH, -1, 0);   /* force la creation du niveau 0 dans la Fah. */
                         /* NECESSAIRE pour eviter la creation prematuree */
                         /* de la file d'urgence */ 

  for (x = 0; x < N; x++)
  {
    if (M[x] && (!masque || MA[x]))    /* on va empiler les voisins des regions marquees */
    {
      switch (connex)
      {
        case 6:
          for (k = 0; k <= 10; k += 2)
          {
            y = voisin6(x, k, rs, n, N);
            if ((y!=-1)&&!M[y]&&!IsSet(y,EN_FAH)){ FahPush(FAH, y, F[y]); Set(y, EN_FAH); }
          } /* for (k = 0; k < 8; k += 2) */
          break;
	  case 18:
            for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
            {
              y = voisin18(x, k, rs, n, N);
              if ((y!=-1)&&!M[y]&&!IsSet(y,EN_FAH)){ FahPush(FAH, y, F[y]); Set(y, EN_FAH); }
            } /* for (k = 0; k < 18; k += 1) */
          break;
	  case 26:
            for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
            {
              y = voisin26(x, k, rs, n, N);
              if ((y!=-1)&&!M[y]&&!IsSet(y,EN_FAH)){ FahPush(FAH, y, F[y]); Set(y, EN_FAH); }
            } /* for (k = 0; k < 26; k += 1) */
          break;
      } /* switch (connex) */
    } /* if (B[x]) */
  } /* for (x = 0; x < N; x++) */

  x = FahPop(FAH);
#ifdef PARANO
  if (x != -1)
  {   
     fprintf(stderr,"%s : ORDRE FIFO NON RESPECTE PAR LA FAH !!!\n", F_NAME);
     return(0);
  }
#endif

  /* ================================================ */
  /* INONDATION                                       */
  /* ================================================ */

  nlabels += 1;          /* cree le label pour les points de la LPE */
  while (! FahVide(FAH))
  {
    x = FahPop(FAH);
    UnSet(x, EN_FAH);

    ncc = 0;
    switch (connex)
    {
      case 6:
        for (k = 0; k <= 10; k += 2)
        {
          y = voisin6(x, k, rs, n, N);
          if ((y != -1) && (M[y] != 0) && (M[y] != nlabels) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
          {
            etiqcc[ncc] = M[y];        
            ncc += 1;
          }
        } /* for k */
        break;
      case 18:
        for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
        {
          y = voisin18(x, k, rs, n, N);
          if ((y != -1) && (M[y] != 0) && (M[y] != nlabels) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
          {
            etiqcc[ncc] = M[y];        
            ncc += 1;
          }
        } /* for k */
        break;
      case 26:
        for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
        {
          y = voisin26(x, k, rs, n, N);
          if ((y != -1) && (M[y] != 0) && (M[y] != nlabels) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
          {
            etiqcc[ncc] = M[y];        
            ncc += 1;
          }
        } /* for k */
        break;
    } /* switch (connex) */

    if (ncc == 1)
    {
      M[x] = etiqcc[0];
      switch (connex)
      {
        case 6:
          for (k = 0; k <= 10; k += 2)
          {
            y = voisin6(x, k, rs, n, N);     
            if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
            {
              FahPush(FAH, y, F[y]);
              Set(y, EN_FAH);
            } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
          } /* for k */
          break;
        case 18:
          for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
          {
            y = voisin18(x, k, rs, n, N);
            if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
            {
              FahPush(FAH, y, F[y]);
              Set(y, EN_FAH);
            } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
          } /* for k */
          break;
        case 26:
          for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
          {
            y = voisin26(x, k, rs, n, N);
            if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
            {
              FahPush(FAH, y, F[y]);
              Set(y, EN_FAH);
            } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
          } /* for k */
          break;
      } /* switch (connex) */
    } 
    else 
    if (ncc > 1)
    {
      M[x] = nlabels;
    }
  } /* while (! FahVide(FAH)) */
  /* FIN PROPAGATION */

  for (x = 0; x < N; x++)
  {
    if ((M[x] == nlabels) || (M[x] == 0)) F[x] = 255; else F[x] = 0;
  }

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahTermine(FAH);
  free(M);
  return(1);
} /* llpemeyer3d() */

/* ==================================== */
int32_t llpemeyer3dsansligne(
        struct xvimage *image,
        struct xvimage *marqueurs,
        struct xvimage *marqueursfond,
        struct xvimage *masque,
        int32_t connex,
        struct xvimage *result	
)
/* ==================================== */
#undef F_NAME
#define F_NAME "llpemeyer3dsansligne"
{
  register index_t x;                       /* index muet de pixel */
  register index_t y;                       /* index muet (generalement un voisin de x) */
  register index_t w;                       /* index muet (generalement un voisin de x) */
  register int32_t k;                       /* index muet */
  index_t rs = rowsize(image);     /* taille ligne */
  index_t cs = colsize(image);     /* taille colonne */
  index_t d = depth(image);        /* nb plans */
  index_t n = rs * cs;             /* taille plan */
  index_t N = n * d;               /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *B = UCHARDATA(marqueurs);       /* l'image de marqueurs */
  uint8_t *BF;                             /* l'image de marqueurs du fond */
  uint8_t *MA;                             /* l'image de masque */
  int32_t *M;             /* l'image d'etiquettes */
  index_t nlabels;                 /* nombre de labels differents */
  Fah * FAH;                   /* la file d'attente hierarchique */
  int32_t etiqcc[6];
  int32_t ncc;  

  if (datatype(image) != VFF_TYP_1_BYTE) 
  {
    fprintf(stderr, "%s: image type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (datatype(marqueurs) != VFF_TYP_1_BYTE) 
  {
    fprintf(stderr, "%s: marker type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (marqueursfond && (datatype(marqueursfond) != VFF_TYP_1_BYTE))
  {
    fprintf(stderr, "%s: bgnd marker type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (masque && (datatype(masque) != VFF_TYP_1_BYTE))
  {
    fprintf(stderr, "%s: mask type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (depth(image) == 1) 
  {
    fprintf(stderr, "%s: cette version ne traite que les images volumiques\n", F_NAME);
    exit(0);
  }

  if (datatype(result) != VFF_TYP_4_BYTE) 
  {
    fprintf(stderr, "%s: le resultat doit etre de type VFF_TYP_4_BYTE\n", F_NAME);
    return 0;
  }

  if ((rowsize(marqueurs) != rs) || (colsize(marqueurs) != cs) || (depth(marqueurs) != d))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }

  if (marqueursfond && ((rowsize(marqueursfond) != rs) || (colsize(marqueursfond) != cs) || (depth(marqueursfond) != d)))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (marqueursfond) BF = UCHARDATA(marqueursfond);
  if (masque && ((rowsize(masque) != rs) || (colsize(masque) != cs) || (depth(masque) != d)))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (masque) MA = UCHARDATA(masque);

  IndicsInit(N);
  FAH = CreeFahVide(N+1);
  if (FAH == NULL)
  {   fprintf(stderr, "%s() : CreeFah failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /* CREATION DES LABELS INITIAUX                     */
  /* ================================================ */

  M = SLONGDATA(result);
  memset(M, 0, N*sizeof(int32_t));
  nlabels = 0;

  if (marqueursfond)
  {
    nlabels += 1;                 /* tous les marqueurs du fond ont le meme label (1) */
    for (x = 0; x < N; x++)
    {
      if (BF[x] && (M[x] == 0) && (!masque || MA[x]))
      {
        M[x] = nlabels;
        FahPush(FAH, x, 0);
        while (! FahVide(FAH))
        {
          w = FahPop(FAH);
          switch (connex)
          {
	    case 6:
              for (k = 0; k <= 10; k += 2) /* parcourt les 6 voisins */
              {
                y = voisin6(w, k, rs, n, N);
                if ((y != -1) && BF[y] && (M[y] == 0) && (!masque || MA[y]))
                { M[y] = nlabels; FahPush(FAH, y, 0); }
              } /* for k ... */
              break;
	    case 18:
              for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
              {
                y = voisin18(w, k, rs, n, N);
                if ((y != -1) && BF[y] && (M[y] == 0) && (!masque || MA[y]))
                { M[y] = nlabels; FahPush(FAH, y, 0); }
              } /* for k ... */
              break;
	    case 26:
              for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
              {
                y = voisin26(w, k, rs, n, N);
                if ((y != -1) && BF[y] && (M[y] == 0) && (!masque || MA[y]))
                { M[y] = nlabels; FahPush(FAH, y, 0); }
              } /* for k ... */
              break;
	  } /* switch (connex) */
        } /* while (! FahVide(FAH)) */
      } /* if (M[x] == 0) */
    } /* for (x = 0; x < N; x++) */
  } /* if (marqueursfond) */

  for (x = 0; x < N; x++)
  {
    if (B[x] && (M[x] == 0) && (!masque || MA[x]))
    {
      nlabels += 1;
      M[x] = nlabels;
      FahPush(FAH, x, 0);
      while (! FahVide(FAH))
      {
        w = FahPop(FAH);
        switch (connex)
        {
	  case 6:
            for (k = 0; k <= 10; k += 2)
            {
              y = voisin6(w, k, rs, n, N);
              if ((y!=-1) && (M[y]==0) && (B[y]==B[w]) && (!masque || MA[y]))
              { M[y] = nlabels; FahPush(FAH, y, 0); } /* if y ... */
            } /* for k ... */
            break;
	  case 18:
            for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
            {
              y = voisin18(w, k, rs, n, N);
              if ((y!=-1) && (M[y]==0) && (B[y]==B[w]) && (!masque || MA[y]))
              { M[y] = nlabels; FahPush(FAH, y, 0); } /* if y ... */
            } /* for k ... */
            break;
	  case 26:
            for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
            {
              y = voisin26(w, k, rs, n, N);
              if ((y!=-1) && (M[y]==0) && (B[y]==B[w]) && (!masque || MA[y]))
              { M[y] = nlabels; FahPush(FAH, y, 0); } /* if y ... */
            } /* for k ... */
            break;
	} /* switch (connex) */
      } /* while (! FahVide(FAH)) */
    } /* if (M[x] == 0) */
  } /* for (x = 0; x < N; x++) */

  /* ================================================ */
  /* INITIALISATION DE LA FAH                         */
  /* ================================================ */

  FahFlush(FAH);
  FahPush(FAH, -1, 0);   /* force la creation du niveau 0 dans la Fah. */
                         /* NECESSAIRE pour eviter la creation prematuree */
                         /* de la file d'urgence */ 

  for (x = 0; x < N; x++)
  {
    if (M[x] && (!masque || MA[x]))    /* on va empiler les voisins des regions marquees */
    {
      switch (connex)
      {
        case 6:
          for (k = 0; k <= 10; k += 2)
          {
            y = voisin6(x, k, rs, n, N);
            if ((y!=-1)&&!M[y]&&!IsSet(y,EN_FAH)){ FahPush(FAH, y, F[y]); Set(y, EN_FAH); }
          } /* for (k = 0; k < 8; k += 2) */
          break;
	  case 18:
            for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
            {
              y = voisin18(x, k, rs, n, N);
              if ((y!=-1)&&!M[y]&&!IsSet(y,EN_FAH)){ FahPush(FAH, y, F[y]); Set(y, EN_FAH); }
            } /* for (k = 0; k < 18; k += 1) */
          break;
	  case 26:
            for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
            {
              y = voisin26(x, k, rs, n, N);
              if ((y!=-1)&&!M[y]&&!IsSet(y,EN_FAH)){ FahPush(FAH, y, F[y]); Set(y, EN_FAH); }
            } /* for (k = 0; k < 26; k += 1) */
          break;
      } /* switch (connex) */
    } /* if (B[x]) */
  } /* for (x = 0; x < N; x++) */

  x = FahPop(FAH);
#ifdef PARANO
  if (x != -1)
  {   
     fprintf(stderr,"%s : ORDRE FIFO NON RESPECTE PAR LA FAH !!!\n", F_NAME);
     return(0);
  }
#endif

  /* ================================================ */
  /* INONDATION                                       */
  /* ================================================ */

  nlabels += 1;          /* cree le label pour les points de la LPE */
  while (! FahVide(FAH))
  {
    x = FahPop(FAH);
    UnSet(x, EN_FAH);

    ncc = 0;
    switch (connex)
    {
      case 6:
        for (k = 0; k <= 10; k += 2)
        {
          y = voisin6(x, k, rs, n, N);
          if ((y != -1) && (M[y] != 0) && (M[y] != nlabels) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
          {
            etiqcc[ncc] = M[y];        
            ncc += 1;
          }
        } /* for k */
        break;
      case 18:
        for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
        {
          y = voisin18(x, k, rs, n, N);
          if ((y != -1) && (M[y] != 0) && (M[y] != nlabels) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
          {
            etiqcc[ncc] = M[y];        
            ncc += 1;
          }
        } /* for k */
        break;
      case 26:
        for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
        {
          y = voisin26(x, k, rs, n, N);
          if ((y != -1) && (M[y] != 0) && (M[y] != nlabels) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
          {
            etiqcc[ncc] = M[y];        
            ncc += 1;
          }
        } /* for k */
        break;
    } /* switch (connex) */

    /* Label */
      M[x] = etiqcc[0];
      switch (connex)
      {
        case 6:
          for (k = 0; k <= 10; k += 2)
          {
            y = voisin6(x, k, rs, n, N);     
            if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
            {
              FahPush(FAH, y, F[y]);
              Set(y, EN_FAH);
            } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
          } /* for k */
          break;
        case 18:
          for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
          {
            y = voisin18(x, k, rs, n, N);
            if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
            {
              FahPush(FAH, y, F[y]);
              Set(y, EN_FAH);
            } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
          } /* for k */
          break;
        case 26:
          for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
          {
            y = voisin26(x, k, rs, n, N);
            if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
            {
              FahPush(FAH, y, F[y]);
              Set(y, EN_FAH);
            } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
          } /* for k */
          break;
      } /* switch (connex) */
  } /* while (! FahVide(FAH)) */
  /* FIN PROPAGATION */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahTermine(FAH);
  return(1);
} /* llpemeyer3dsansligne() */

/* ==================================== */
int32_t llpemeyer3dsanslignelab(
        struct xvimage *image,
        struct xvimage *marqueurs, // entree-sortie
        struct xvimage *masque,
        int32_t connex
)
/* ==================================== */
#undef F_NAME
#define F_NAME "llpemeyer3dsanslignelab"
{
  register index_t x;                       /* index muet de pixel */
  register index_t y;                       /* index muet (generalement un voisin de x) */
  register int32_t k;                       /* index muet */
  index_t rs = rowsize(image);     /* taille ligne */
  index_t cs = colsize(image);     /* taille colonne */
  index_t d = depth(image);        /* nb plans */
  index_t n = rs * cs;             /* taille plan */
  index_t N = n * d;               /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  int32_t *M = SLONGDATA(marqueurs);      /* l'image de marqueurs */
  uint8_t *MA;                             /* l'image de masque */
  Fah * FAH;                   /* la file d'attente hierarchique */
  int32_t etiqcc[6];
  int32_t ncc;  

  if (datatype(image) != VFF_TYP_1_BYTE) 
  {
    fprintf(stderr, "%s: image type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (datatype(marqueurs) != VFF_TYP_4_BYTE) 
  {
    fprintf(stderr, "%s: marker type must be VFF_TYP_4_BYTE\n", F_NAME);
    return 0;
  }

  if (masque && (datatype(masque) != VFF_TYP_1_BYTE))
  {
    fprintf(stderr, "%s: mask type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (depth(image) == 1) 
  {
    fprintf(stderr, "%s: cette version ne traite que les images volumiques\n", F_NAME);
    exit(0);
  }

  if ((rowsize(marqueurs) != rs) || (colsize(marqueurs) != cs) || (depth(marqueurs) != d))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }

  if (masque && ((rowsize(masque) != rs) || (colsize(masque) != cs) || (depth(masque) != d)))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (masque) MA = UCHARDATA(masque);

  IndicsInit(N);
  FAH = CreeFahVide(N+1);
  if (FAH == NULL)
  {   fprintf(stderr, "%s() : CreeFah failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /* INITIALISATION DE LA FAH                         */
  /* ================================================ */

  FahFlush(FAH);
  FahPush(FAH, -1, 0);   /* force la creation du niveau 0 dans la Fah. */
                         /* NECESSAIRE pour eviter la creation prematuree */
                         /* de la file d'urgence */ 

  for (x = 0; x < N; x++)
  {
    if (M[x] && (!masque || MA[x]))    /* on va empiler les voisins des regions marquees */
    {
      switch (connex)
      {
        case 6:
          for (k = 0; k <= 10; k += 2)
          {
            y = voisin6(x, k, rs, n, N);
            if ((y!=-1)&&!M[y]&&!IsSet(y,EN_FAH)){ FahPush(FAH, y, F[y]); Set(y, EN_FAH); }
          } /* for (k = 0; k < 8; k += 2) */
          break;
	  case 18:
            for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
            {
              y = voisin18(x, k, rs, n, N);
              if ((y!=-1)&&!M[y]&&!IsSet(y,EN_FAH)){ FahPush(FAH, y, F[y]); Set(y, EN_FAH); }
            } /* for (k = 0; k < 18; k += 1) */
          break;
	  case 26:
            for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
            {
              y = voisin26(x, k, rs, n, N);
              if ((y!=-1)&&!M[y]&&!IsSet(y,EN_FAH)){ FahPush(FAH, y, F[y]); Set(y, EN_FAH); }
            } /* for (k = 0; k < 26; k += 1) */
          break;
      } /* switch (connex) */
    } /* if (B[x]) */
  } /* for (x = 0; x < N; x++) */

  x = FahPop(FAH);
#ifdef PARANO
  if (x != -1)
  {   
     fprintf(stderr,"%s : ORDRE FIFO NON RESPECTE PAR LA FAH !!!\n", F_NAME);
     return(0);
  }
#endif

  /* ================================================ */
  /* INONDATION                                       */
  /* ================================================ */

  while (! FahVide(FAH))
  {
    x = FahPop(FAH);
    UnSet(x, EN_FAH);

    ncc = 0;
    switch (connex)
    {
      case 6:
        for (k = 0; k <= 10; k += 2)
        {
          y = voisin6(x, k, rs, n, N);
          if ((y != -1) && (M[y] != 0) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
          {
            etiqcc[ncc] = M[y];        
            ncc += 1;
          }
        } /* for k */
        break;
      case 18:
        for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
        {
          y = voisin18(x, k, rs, n, N);
          if ((y != -1) && (M[y] != 0) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
          {
            etiqcc[ncc] = M[y];        
            ncc += 1;
          }
        } /* for k */
        break;
      case 26:
        for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
        {
          y = voisin26(x, k, rs, n, N);
          if ((y != -1) && (M[y] != 0) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
          {
            etiqcc[ncc] = M[y];        
            ncc += 1;
          }
        } /* for k */
        break;
    } /* switch (connex) */

    /* Label */
      M[x] = etiqcc[0];
      switch (connex)
      {
        case 6:
          for (k = 0; k <= 10; k += 2)
          {
            y = voisin6(x, k, rs, n, N);     
            if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
            {
              FahPush(FAH, y, F[y]);
              Set(y, EN_FAH);
            } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
          } /* for k */
          break;
        case 18:
          for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
          {
            y = voisin18(x, k, rs, n, N);
            if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
            {
              FahPush(FAH, y, F[y]);
              Set(y, EN_FAH);
            } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
          } /* for k */
          break;
        case 26:
          for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
          {
            y = voisin26(x, k, rs, n, N);
            if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
            {
              FahPush(FAH, y, F[y]);
              Set(y, EN_FAH);
            } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
          } /* for k */
          break;
      } /* switch (connex) */
  } /* while (! FahVide(FAH)) */
  /* FIN PROPAGATION */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahTermine(FAH);
  return(1);
} /* llpemeyer3dsanslignelab() */

/* ==================================== */
int32_t llpemeyer3d2(
        struct xvimage *image,
        struct xvimage *marqueurs,
        struct xvimage *masque,
        int32_t connex)
/* ==================================== */
// marqueurs: image initiale de labels
// le résultat du traitement se trouve dans marqueurs (image de labels)
// et dans image (binaire)
// LPE avec ligne de séparation
{
#undef F_NAME
#define F_NAME "llpemeyer3d2"
  register index_t x, y, k;
  index_t rs = rowsize(image);       /* taille ligne */
  index_t cs = colsize(image);       /* taille colonne */
  index_t d = depth(image);          /* nb plans */
  index_t n = rs * cs;               /* taille plan */
  index_t N = n * d;                 /* taille image */
  uint8_t *F = UCHARDATA(image);     /* l'image de depart */
  uint8_t *MA;                       /* l'image de masque */
  int32_t *M = SLONGDATA(marqueurs); /* l'image d'etiquettes */
  Fah * FAH;                         /* la file d'attente hierarchique */
  int32_t etiqcc[6];
  int32_t ncc;  
  index_t nlabels;

#ifdef DEBUG_llpemeyer3d3
  printf("%s: begin\n", F_NAME);
#endif

  if (d == 1) 
  {
    fprintf(stderr, "%s: 3D images only\n", F_NAME);
    exit(0);
  }

  if (datatype(image) != VFF_TYP_1_BYTE) 
  {
    fprintf(stderr, "%s: image type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (masque && (datatype(masque) != VFF_TYP_1_BYTE))
  {
    fprintf(stderr, "%s: mask type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (datatype(marqueurs) != VFF_TYP_4_BYTE) 
  {
    fprintf(stderr, "%s: marker image must by 4 byte\n", F_NAME);
    return 0;
  }

  if ((rowsize(marqueurs) != rs) || (colsize(marqueurs) != cs)  || (depth(marqueurs) != d))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }

  if (masque && ((rowsize(masque) != rs) || (colsize(masque) != cs) || (depth(masque) != d)))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (masque) MA = UCHARDATA(masque);

  IndicsInit(N);
  FAH = CreeFahVide(N+1);
  if (FAH == NULL)
  {   fprintf(stderr, "%s() : CreeFah failed\n", F_NAME);
      return(0);
  }

  nlabels = 0;
  for (x = 0; x < N; x++) if (M[x] > nlabels) nlabels = M[x];

  /* ================================================ */
  /* INITIALISATION DE LA FAH                         */
  /* ================================================ */

  FahFlush(FAH);
  FahPush(FAH, -1, 0);   /* force la creation du niveau 0 dans la Fah. */
                         /* NECESSAIRE pour eviter la creation prematuree */
                         /* de la file d'urgence */ 

  for (x = 0; x < N; x++)
  {
    if (M[x] && (!masque || MA[x]))    /* on va empiler les voisins des regions marquees */
    {
      switch (connex)
      {
        case 6:
          for (k = 0; k <= 10; k += 2)
          {
            y = voisin6(x, k, rs, n, N);
            if ((y!=-1) && !M[y] && !IsSet(y,EN_FAH) && (!masque || MA[y]))
	    { FahPush(FAH, y, F[y]); Set(y, EN_FAH); }
          } /* for (k = 0; k < 8; k += 2) */
          break;
	  case 18:
            for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
            {
              y = voisin18(x, k, rs, n, N);
              if ((y!=-1) && !M[y] && !IsSet(y,EN_FAH) && (!masque || MA[y]))
	      { FahPush(FAH, y, F[y]); Set(y, EN_FAH); }
            } /* for (k = 0; k < 18; k += 1) */
          break;
	  case 26:
            for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
            {
              y = voisin26(x, k, rs, n, N);
              if ((y!=-1) && !M[y] && !IsSet(y,EN_FAH) && (!masque || MA[y]))
	      { FahPush(FAH, y, F[y]); Set(y, EN_FAH); }
            } /* for (k = 0; k < 26; k += 1) */
          break;
      } /* switch (connex) */
    } /* if (B[x]) */
  } /* for (x = 0; x < N; x++) */

  x = FahPop(FAH);
#ifdef PARANO
  if (x != -1)
  {   
     fprintf(stderr,"%s : ORDRE FIFO NON RESPECTE PAR LA FAH !!!\n", F_NAME);
     return(0);
  }
#endif

  /* ================================================ */
  /* INONDATION                                       */
  /* ================================================ */

  nlabels += 1;          /* cree le label pour les points de la LPE */
  while (! FahVide(FAH))
  {
    x = FahPop(FAH);
    UnSet(x, EN_FAH);

    ncc = 0;
    switch (connex)
    {
      case 6:
        for (k = 0; k <= 10; k += 2)
        {
          y = voisin6(x, k, rs, n, N);
          if ((y != -1) && (M[y] != 0) && (M[y] != nlabels) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
          {
            etiqcc[ncc] = M[y];        
            ncc += 1;
          }
        } /* for k */
        break;
      case 18:
        for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
        {
          y = voisin18(x, k, rs, n, N);
          if ((y != -1) && (M[y] != 0) && (M[y] != nlabels) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
          {
            etiqcc[ncc] = M[y];        
            ncc += 1;
          }
        } /* for k */
        break;
      case 26:
        for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
        {
          y = voisin26(x, k, rs, n, N);
          if ((y != -1) && (M[y] != 0) && (M[y] != nlabels) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
          {
            etiqcc[ncc] = M[y];        
            ncc += 1;
          }
        } /* for k */
        break;
    } /* switch (connex) */

    if (ncc == 1)
    {
      M[x] = etiqcc[0];

      switch (connex)
      {
      case 6:
	for (k = 0; k <= 10; k += 2)
	{
	  y = voisin6(x, k, rs, n, N);     
	  if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
          {
	    FahPush(FAH, y, F[y]);
	    Set(y, EN_FAH);
	  } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
	} /* for k */
	break;
      case 18:
	for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
        {
	  y = voisin18(x, k, rs, n, N);
	  if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
          {
	    FahPush(FAH, y, F[y]);
	    Set(y, EN_FAH);
	  } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
	} /* for k */
	break;
      case 26:
	for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
        {
	  y = voisin26(x, k, rs, n, N);
	  if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
          {
	    FahPush(FAH, y, F[y]);
	    Set(y, EN_FAH);
	  } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
	} /* for k */
	break;
      } /* switch (connex) */
    } // if (ncc == 1)
    else 
    if (ncc > 1)
    {
      M[x] = nlabels;
    }
  } /* while (! FahVide(FAH)) */
  /* FIN PROPAGATION */

  for (x = 0; x < N; x++)
  {
    if ((M[x] == nlabels) || (M[x] == 0)) F[x] = 255; else F[x] = 0;
  }

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahTermine(FAH);

  return(1);
} /* llpemeyer3d2() */

/* ==================================== */
int32_t llpemeyer3d2b(
        struct xvimage *image,
        struct xvimage *marqueurs,
        struct xvimage *masque,
        int32_t connex)
/* ==================================== */
// marqueurs: image initiale de labels
// le résultat du traitement se trouve dans marqueurs (image de labels)
// et dans image (binaire)
// LPE avec ligne de séparation
// labels différents pour les voxels de la LPE :
//   soit n le dernier label de l'image marqueurs, 
//   si v est voisin de 2 labels i et j alors son label sera i*j+n
//   s'il est voisin de plus de 2 labels alors son label sera n+1
{
#undef F_NAME
#define F_NAME "llpemeyer3d2b"
  register int32_t k;
  register index_t x, y;
  index_t rs = rowsize(image);       /* taille ligne */
  index_t cs = colsize(image);       /* taille colonne */
  index_t d = depth(image);          /* nb plans */
  index_t n = rs * cs;               /* taille plan */
  index_t N = n * d;                 /* taille image */
  uint8_t *F = UCHARDATA(image);     /* l'image de depart */
  uint8_t *MA;                       /* l'image de masque */
  int32_t *M = SLONGDATA(marqueurs); /* l'image d'etiquettes */
  Fah * FAH;                         /* la file d'attente hierarchique */
  int32_t etiqcc[6];
  int32_t ncc;  
  index_t nlabels;

#ifdef DEBUG_llpemeyer3d3
  printf("%s: begin\n", F_NAME);
#endif

  if (d == 1) 
  {
    fprintf(stderr, "%s: 3D images only\n", F_NAME);
    exit(0);
  }

  if (datatype(image) != VFF_TYP_1_BYTE) 
  {
    fprintf(stderr, "%s: image type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (masque && (datatype(masque) != VFF_TYP_1_BYTE))
  {
    fprintf(stderr, "%s: mask type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (datatype(marqueurs) != VFF_TYP_4_BYTE) 
  {
    fprintf(stderr, "%s: marker image must by 4 byte\n", F_NAME);
    return 0;
  }

  if ((rowsize(marqueurs) != rs) || (colsize(marqueurs) != cs)  || (depth(marqueurs) != d))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }

  if (masque && ((rowsize(masque) != rs) || (colsize(masque) != cs) || (depth(masque) != d)))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (masque) MA = UCHARDATA(masque);

  IndicsInit(N);
  FAH = CreeFahVide(N+1);
  if (FAH == NULL)
  {   fprintf(stderr, "%s() : CreeFah failed\n", F_NAME);
      return(0);
  }

  nlabels = 0;
  for (x = 0; x < N; x++) if (M[x] > nlabels) nlabels = M[x];

  /* ================================================ */
  /* INITIALISATION DE LA FAH                         */
  /* ================================================ */

  FahFlush(FAH);
  FahPush(FAH, -1, 0);   /* force la creation du niveau 0 dans la Fah. */
                         /* NECESSAIRE pour eviter la creation prematuree */
                         /* de la file d'urgence */ 

  for (x = 0; x < N; x++)
  {
    if (M[x] && (!masque || MA[x]))    /* on va empiler les voisins des regions marquees */
    {
      switch (connex)
      {
        case 6:
          for (k = 0; k <= 10; k += 2)
          {
            y = voisin6(x, k, rs, n, N);
            if ((y!=-1) && !M[y] && !IsSet(y,EN_FAH) && (!masque || MA[y]))
	    { FahPush(FAH, y, F[y]); Set(y, EN_FAH); }
          } /* for (k = 0; k < 8; k += 2) */
          break;
	  case 18:
            for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
            {
              y = voisin18(x, k, rs, n, N);
              if ((y!=-1) && !M[y] && !IsSet(y,EN_FAH) && (!masque || MA[y]))
	      { FahPush(FAH, y, F[y]); Set(y, EN_FAH); }
            } /* for (k = 0; k < 18; k += 1) */
          break;
	  case 26:
            for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
            {
              y = voisin26(x, k, rs, n, N);
              if ((y!=-1) && !M[y] && !IsSet(y,EN_FAH) && (!masque || MA[y]))
	      { FahPush(FAH, y, F[y]); Set(y, EN_FAH); }
            } /* for (k = 0; k < 26; k += 1) */
          break;
      } /* switch (connex) */
    } /* if (B[x]) */
  } /* for (x = 0; x < N; x++) */

  x = FahPop(FAH);
#ifdef PARANO
  if (x != -1)
  {   
     fprintf(stderr,"%s : ORDRE FIFO NON RESPECTE PAR LA FAH !!!\n", F_NAME);
     return(0);
  }
#endif

  /* ================================================ */
  /* INONDATION                                       */
  /* ================================================ */

  while (! FahVide(FAH))
  {
    x = FahPop(FAH);
    UnSet(x, EN_FAH);

    ncc = 0;
    switch (connex)
    {
      case 6:
        for (k = 0; k <= 10; k += 2)
        {
          y = voisin6(x, k, rs, n, N);
          if ((y != -1) && (M[y] != 0) && (M[y] <= nlabels) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
          {
            etiqcc[ncc] = M[y];        
            ncc += 1;
          }
        } /* for k */
        break;
      case 18:
        for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
        {
          y = voisin18(x, k, rs, n, N);
          if ((y != -1) && (M[y] != 0) && (M[y] <= nlabels) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
          {
            etiqcc[ncc] = M[y];        
            ncc += 1;
          }
        } /* for k */
        break;
      case 26:
        for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
        {
          y = voisin26(x, k, rs, n, N);
          if ((y != -1) && (M[y] != 0) && (M[y] <= nlabels) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
          {
            etiqcc[ncc] = M[y];        
            ncc += 1;
          }
        } /* for k */
        break;
    } /* switch (connex) */

    if (ncc == 1)
    {
      M[x] = etiqcc[0];

      switch (connex)
      {
      case 6:
	for (k = 0; k <= 10; k += 2)
	{
	  y = voisin6(x, k, rs, n, N);     
	  if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
          {
	    FahPush(FAH, y, F[y]);
	    Set(y, EN_FAH);
	  } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
	} /* for k */
	break;
      case 18:
	for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
        {
	  y = voisin18(x, k, rs, n, N);
	  if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
          {
	    FahPush(FAH, y, F[y]);
	    Set(y, EN_FAH);
	  } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
	} /* for k */
	break;
      case 26:
	for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
        {
	  y = voisin26(x, k, rs, n, N);
	  if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
          {
	    FahPush(FAH, y, F[y]);
	    Set(y, EN_FAH);
	  } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
	} /* for k */
	break;
      } /* switch (connex) */
    } // if (ncc == 1)
    else if (ncc > 1)
    {
      if (ncc > 2) M[x] = nlabels + 1;
      else M[x] = nlabels + (etiqcc[0] * etiqcc[1]);
    }
  } /* while (! FahVide(FAH)) */
  /* FIN PROPAGATION */

  for (x = 0; x < N; x++)
  {
    if ((M[x] > nlabels) || (M[x] == 0)) F[x] = 255; else F[x] = 0;
  }

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahTermine(FAH);

  return(1);
} /* llpemeyer3d2b() */

/* ==================================== */
int32_t llpemeyer3d3(
        struct xvimage *image,
        struct xvimage *marqueurs,
        struct xvimage *masque,
        int32_t connex)
/* ==================================== */
// marqueurs: image initiale de labels
// le résultat du traitement se trouve dans marqueurs
// LPE sans ligne de séparation
#undef F_NAME
#define F_NAME "llpemeyer3d3"
{
  register int32_t k;
  register index_t x, y;
  index_t rs = rowsize(image);       /* taille ligne */
  index_t cs = colsize(image);       /* taille colonne */
  index_t d = depth(image);          /* nb plans */
  index_t n = rs * cs;               /* taille plan */
  index_t N = n * d;                 /* taille image */
  uint8_t *F = UCHARDATA(image);     /* l'image de depart */
  uint8_t *MA;                       /* l'image de masque */
  int32_t *M = SLONGDATA(marqueurs); /* l'image d'etiquettes */
  Fah * FAH;                         /* la file d'attente hierarchique */
  int32_t etiqcc[6];
  int32_t ncc;  

#ifdef DEBUG_llpemeyer3d3
  printf("%s: begin\n", F_NAME);
#endif

  if (d == 1) 
  {
    fprintf(stderr, "%s: 3D images only\n", F_NAME);
    exit(0);
  }

  if (datatype(image) != VFF_TYP_1_BYTE) 
  {
    fprintf(stderr, "%s: image type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (masque && (datatype(masque) != VFF_TYP_1_BYTE))
  {
    fprintf(stderr, "%s: mask type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (datatype(marqueurs) != VFF_TYP_4_BYTE) 
  {
    fprintf(stderr, "%s: marker image must by 4 byte\n", F_NAME);
    return 0;
  }

  if ((rowsize(marqueurs) != rs) || (colsize(marqueurs) != cs)  || (depth(marqueurs) != d))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }

  if (masque && ((rowsize(masque) != rs) || (colsize(masque) != cs) || (depth(masque) != d)))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (masque) MA = UCHARDATA(masque);

  IndicsInit(N);
  FAH = CreeFahVide(N+1);
  if (FAH == NULL)
  {   fprintf(stderr, "%s() : CreeFah failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /* INITIALISATION DE LA FAH                         */
  /* ================================================ */

  FahFlush(FAH);
  FahPush(FAH, -1, 0);   /* force la creation du niveau 0 dans la Fah. */
                         /* NECESSAIRE pour eviter la creation prematuree */
                         /* de la file d'urgence */ 

  for (x = 0; x < N; x++)
  {
    if (M[x] && (!masque || MA[x]))    /* on va empiler les voisins des regions marquees */
    {
      switch (connex)
      {
        case 6:
          for (k = 0; k <= 10; k += 2)
          {
            y = voisin6(x, k, rs, n, N);
            if ((y!=-1) && !M[y] && !IsSet(y,EN_FAH) && (!masque || MA[y]))
	    { FahPush(FAH, y, F[y]); Set(y, EN_FAH); }
          } /* for (k = 0; k < 8; k += 2) */
          break;
	  case 18:
            for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
            {
              y = voisin18(x, k, rs, n, N);
              if ((y!=-1) && !M[y] && !IsSet(y,EN_FAH) && (!masque || MA[y]))
	      { FahPush(FAH, y, F[y]); Set(y, EN_FAH); }
            } /* for (k = 0; k < 18; k += 1) */
          break;
	  case 26:
            for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
            {
              y = voisin26(x, k, rs, n, N);
              if ((y!=-1) && !M[y] && !IsSet(y,EN_FAH) && (!masque || MA[y]))
	      { FahPush(FAH, y, F[y]); Set(y, EN_FAH); }
            } /* for (k = 0; k < 26; k += 1) */
          break;
      } /* switch (connex) */
    } /* if (B[x]) */
  } /* for (x = 0; x < N; x++) */

  x = FahPop(FAH);
#ifdef PARANO
  if (x != -1)
  {   
     fprintf(stderr,"%s : ORDRE FIFO NON RESPECTE PAR LA FAH !!!\n", F_NAME);
     return(0);
  }
#endif

  /* ================================================ */
  /* INONDATION                                       */
  /* ================================================ */

  while (! FahVide(FAH))
  {
    x = FahPop(FAH);
    UnSet(x, EN_FAH);

    ncc = 0;
    switch (connex)
    {
      case 6:
        for (k = 0; k <= 10; k += 2)
        {
          y = voisin6(x, k, rs, n, N);
          if ((y != -1) && (M[y] != 0) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
          {
            etiqcc[ncc] = M[y];        
            ncc += 1;
          }
        } /* for k */
        break;
      case 18:
        for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
        {
          y = voisin18(x, k, rs, n, N);
          if ((y != -1) && (M[y] != 0) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
          {
            etiqcc[ncc] = M[y];        
            ncc += 1;
          }
        } /* for k */
        break;
      case 26:
        for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
        {
          y = voisin26(x, k, rs, n, N);
          if ((y != -1) && (M[y] != 0) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
          {
            etiqcc[ncc] = M[y];        
            ncc += 1;
          }
        } /* for k */
        break;
    } /* switch (connex) */

    // Label with any label found in the neighborhood
    M[x] = etiqcc[0];

    // Continue propagation
    switch (connex)
    {
      case 6:
	for (k = 0; k <= 10; k += 2)
	{
	  y = voisin6(x, k, rs, n, N);     
	  if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
          {
	    FahPush(FAH, y, F[y]);
	    Set(y, EN_FAH);
	  } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
	} /* for k */
	break;
      case 18:
	for (k = 0; k < 18; k += 1) /* parcourt les 18 voisins */
        {
	  y = voisin18(x, k, rs, n, N);
	  if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
          {
	    FahPush(FAH, y, F[y]);
	    Set(y, EN_FAH);
	  } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
	} /* for k */
	break;
      case 26:
	for (k = 0; k < 26; k += 1) /* parcourt les 26 voisins */
        {
	  y = voisin26(x, k, rs, n, N);
	  if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
          {
	    FahPush(FAH, y, F[y]);
	    Set(y, EN_FAH);
	  } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
	} /* for k */
	break;
    } /* switch (connex) */
  } /* while (! FahVide(FAH)) */
  /* FIN PROPAGATION */

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahTermine(FAH);

  return(1);
} /* llpemeyer3d3() */

/* ==================================== */
int32_t llpemeyerbiconnecte(
        struct xvimage *image,
        struct xvimage *marqueurs,
        struct xvimage *marqueursfond,
        struct xvimage *masque,
        int32_t parite)
/* ==================================== */
#undef F_NAME
#define F_NAME "llpemeyerbiconnecte"
{
  register index_t x;                       /* index muet de pixel */
  register index_t y;                       /* index muet (generalement un voisin de x) */
  register index_t w;                       /* index muet (generalement un voisin de x) */
  register int32_t k;                       /* index muet */
  index_t rs = rowsize(image);     /* taille ligne */
  index_t cs = colsize(image);     /* taille colonne */
  index_t N = rs * cs;             /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *B = UCHARDATA(marqueurs);       /* l'image de marqueurs */
  uint8_t *BF;                             /* l'image de marqueurs du fond */
  uint8_t *MA;                             /* l'image de masque */
  int32_t *M;             /* l'image d'etiquettes */
  index_t nlabels;                 /* nombre de labels differents */
  Fah * FAH;                   /* la file d'attente hierarchique */
  int32_t etiqcc[4];
  int32_t ncc;  
  const int32_t incr_vois=1;
#ifdef ANIMATE
  int32_t curlev = -1, nimage = 0; 
  char imname[128];
  struct xvimage *animimage;
  uint8_t *A;
#endif
  if (depth(image) != 1) 
  {
    fprintf(stderr, "%s: cette version ne traite pas les images volumiques\n", F_NAME);
    exit(0);
  }

  if (datatype(image) != VFF_TYP_1_BYTE) 
  {
    fprintf(stderr, "%s: image type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (datatype(marqueurs) != VFF_TYP_1_BYTE) 
  {
    fprintf(stderr, "%s: marker type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (marqueursfond && (datatype(marqueursfond) != VFF_TYP_1_BYTE))
  {
    fprintf(stderr, "%s: bgnd marker type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (masque && (datatype(masque) != VFF_TYP_1_BYTE))
  {
    fprintf(stderr, "%s: mask type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if ((rowsize(marqueurs) != rs) || (colsize(marqueurs) != cs))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }

  if (marqueursfond && ((rowsize(marqueursfond) != rs) || (colsize(marqueursfond) != cs)))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (masque && ((rowsize(masque) != rs) || (colsize(masque) != cs)))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (masque) MA = UCHARDATA(masque);
  if (marqueursfond) BF = UCHARDATA(marqueursfond);

  IndicsInit(N);
  FAH = CreeFahVide(N+1);
  if (FAH == NULL)
  {   fprintf(stderr, "%s : CreeFah failed\n", F_NAME);
      return(0);
  }


  /* ================================================ */
  /* CREATION DES LABELS INITIAUX                     */
  /* ================================================ */

  M = (int32_t *)calloc(N, sizeof(int32_t));
  if (M == NULL)
  {   fprintf(stderr, "%s : calloc failed\n", F_NAME);
      return(0);
  }
  nlabels = 0;

  if (marqueursfond)
  {
    nlabels += 1;                 /* tous les marqueurs du fond ont le meme label (1) */
    for (x = 0; x < N; x++)
    {
      if (BF[x] && (M[x] == 0) && (!masque || MA[x]))
      {
        M[x] = nlabels;
        FahPush(FAH, x, 0);
        while (! FahVide(FAH))
        {
          w = FahPop(FAH);
          for (k = 0; k < 6; k += incr_vois)
          {
            y = voisin6b(w, k, rs, N, parite);
            if ((y != -1) &&  BF[y] && (M[y] == 0) && (!masque || MA[y]))
            {
              M[y] = nlabels;
              FahPush(FAH, y, 0);
            } /* if y ... */
          } /* for k ... */
        } /* while (! FahVide(FAH)) */
      } /* if (M[x] == 0) */
    } /* for (x = 0; x < N; x++) */
  } /* if (marqueursfond) */

  for (x = 0; x < N; x++)
  {
    if (B[x] && (M[x] == 0) && (!masque || MA[x]))
    {
      nlabels += 1;
      M[x] = nlabels;
      FahPush(FAH, x, 0);
      while (! FahVide(FAH))
      {
        w = FahPop(FAH);
        for (k = 0; k < 6; k += incr_vois)
        {
          y = voisin6b(w, k, rs, N, parite);
          if ((y != -1) &&  B[y] && (M[y] == 0) && (!masque || MA[y]))
          {
            M[y] = nlabels;
            FahPush(FAH, y, 0);
          } /* if y ... */
        } /* for k ... */
      } /* while (! FahVide(FAH)) */
    } /* if (M[x] == 0) */
  } /* for (x = 0; x < N; x++) */

  /* ================================================ */
  /* INITIALISATION DE LA FAH                         */
  /* ================================================ */

  FahFlush(FAH);
  FahPush(FAH, -1, 0);   /* force la creation du niveau 0 dans la Fah. */
                         /* NECESSAIRE pour eviter la creation prematuree */
                         /* de la file d'urgence */ 

  for (x = 0; x < N; x++)
  {
    if (M[x] && (!masque || MA[x]))            /* on va empiler les voisins des regions marquees */
    {
      for (k = 0; k < 6; k += incr_vois)
      {
        y = voisin6b(x, k, rs, N, parite);
        if ((y != -1) && !M[y] && !IsSet(y, EN_FAH))
        {        
          FahPush(FAH, y, F[y]);
          Set(y, EN_FAH);
        }
      } /* for (k = 0; k < 8; k += 2) */
    } /* if (M[x]) */
  } /* for (x = 0; x < N; x++) */

  x = FahPop(FAH);
#ifdef PARANO
  if (x != -1)
  {   
     fprintf(stderr,"%s : ORDRE FIFO NON RESPECTE PAR LA FAH !!!\n", F_NAME);
     return(0);
  }
#endif

  /* ================================================ */
  /* INONDATION                                       */
  /* ================================================ */

#ifdef ANIMATE
  animimage = copyimage(image);
  A = UCHARDATA(animimage);
#endif
  nlabels += 1;          /* cree le label pour les points de la LPE */
  while (! FahVide(FAH))
  {
    x = FahPop(FAH);
    UnSet(x, EN_FAH);
#ifdef ANIMATE
    if (F[x] > curlev)
    {
      printf("Niveau %d\n", F[x]);
      sprintf(imname, "anim%03d.pgm", nimage); nimage++;
      for (y = 0; y < N; y++)
        if ((M[y] == nlabels) || (M[y] == 0)) A[y] = 255; else A[y] = 0;
      writeimage(animimage, imname);
      curlev = F[x];
    }
#endif

    ncc = 0;
    for (k = 0; k < 6; k += incr_vois)
    {
      y = voisin6b(x, k, rs, N, parite);
      if ((y != -1) && (M[y] != 0) && (M[y] != nlabels) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
      {
        etiqcc[ncc] = M[y];        
        ncc += 1;
      }
    } /* for k */

    if (ncc == 1)
    {
      M[x] = etiqcc[0];
      for (k = 0; k < 6; k += incr_vois)
      {
        y = voisin6b(x, k, rs, N, parite);     
        if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
        {          
          FahPush(FAH, y, F[y]); 
          Set(y, EN_FAH);
        } /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
      } /* for k */
    } 
    else 
    if (ncc > 1)
    {
      M[x] = nlabels;
    }

  } /* while (! FahVide(FAH)) */
  /* FIN PROPAGATION */

  for (x = 0; x < N; x++)
  {
    if ((M[x] == nlabels) || (M[x] == 0)) F[x] = 255; else F[x] = 0;
  }

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahTermine(FAH);
  free(M);
  return(1);
} // llpemeyerbiconnecte()


/* ==================================== */
int32_t llpemeyerbiconnecte3d(
        struct xvimage *image,
        struct xvimage *marqueurs,
        struct xvimage *marqueursfond,
        struct xvimage *masque)
/* ==================================== */
#undef F_NAME
#define F_NAME "llpemeyerbiconnecte3d"
{
  register index_t x;                       /* index muet de pixel */
  register index_t y;                       /* index muet (generalement un voisin de x) */
  register index_t w;                       /* index muet (generalement un voisin de x) */
  register int32_t k;                       /* index muet */
  index_t rs = rowsize(image);     /* taille ligne */
  index_t cs = colsize(image);     /* taille colonne */
  index_t d = depth(image);        /* nb plans */
  index_t n = rs * cs;             /* taille plan */
  index_t N = n * d;               /* taille image */
  uint8_t *F = UCHARDATA(image);      /* l'image de depart */
  uint8_t *B = UCHARDATA(marqueurs);       /* l'image de marqueurs */
  uint8_t *BF;                             /* l'image de marqueurs du fond */
  uint8_t *MA;                             /* l'image de masque */
  int32_t *M;             /* l'image d'etiquettes */
  index_t nlabels;                 /* nombre de labels differents */
  Fah * FAH;                   /* la file d'attente hierarchique */
  int32_t etiqcc[6];
  int32_t ncc;  

  if (datatype(image) != VFF_TYP_1_BYTE) 
  {
    fprintf(stderr, "%s: image type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (datatype(marqueurs) != VFF_TYP_1_BYTE) 
  {
    fprintf(stderr, "%s: marker type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (marqueursfond && (datatype(marqueursfond) != VFF_TYP_1_BYTE))
  {
    fprintf(stderr, "%s: bgnd marker type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if (masque && (datatype(masque) != VFF_TYP_1_BYTE))
  {
    fprintf(stderr, "%s: mask type must be VFF_TYP_1_BYTE\n", F_NAME);
    return 0;
  }

  if ((rowsize(marqueurs) != rs) || (colsize(marqueurs) != cs))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }

  if (marqueursfond && ((rowsize(marqueursfond) != rs) || (colsize(marqueursfond) != cs) || (depth(marqueursfond) != d)))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (marqueursfond) BF = UCHARDATA(marqueursfond);
  if (masque && ((rowsize(masque) != rs) || (colsize(masque) != cs) || (depth(masque) != d)))
  {
    fprintf(stderr, "%s: incompatible image sizes\n", F_NAME);
    return 0;
  }
  if (masque) MA = UCHARDATA(masque);

  IndicsInit(N);
  FAH = CreeFahVide(N+1);
  if (FAH == NULL)
  {   fprintf(stderr, "%s() : CreeFah failed\n", F_NAME);
      return(0);
  }

  /* ================================================ */
  /* CREATION DES LABELS INITIAUX                     */
  /* ================================================ */
  printf("%s: creation des labels initiaux\n", F_NAME);
  M = (int32_t *)calloc(N, sizeof(int32_t));
  if (M == NULL)
  {   fprintf(stderr, "%s() : calloc failed\n", F_NAME);
      return(0);
  }
  nlabels = 0;

  if (marqueursfond)
  {
    nlabels += 1;                 /* tous les marqueurs du fond ont le meme label (1) */
    for (x = 0; x < N; x++)
    {
      if (BF[x] && (M[x] == 0) && (!masque || MA[x]))
      {
        M[x] = nlabels;
        FahPush(FAH, x, 0);
        while (! FahVide(FAH))
        {
          w = FahPop(FAH);
	  for (k = 0; k < 14; k ++ ) /* parcourt les 14 voisins */
              {
                y = voisin14b(w, k, rs, n, N);
                if ((y != -1) && BF[y] && (M[y] == 0) && (!masque || MA[y]))
                { M[y] = nlabels; FahPush(FAH, y, 0); }
              } /* for k ... */
        } /* while (! FahVide(FAH)) */
      } /* if (M[x] == 0) */
    } /* for (x = 0; x < N; x++) */
  } /* if (marqueursfond) */

  for (x = 0; x < N; x++)
  {
    if (B[x] && (M[x] == 0) && (!masque || MA[x]))
    {
      nlabels += 1;
      M[x] = nlabels;
      FahPush(FAH, x, 0);
      while (! FahVide(FAH))
      {
        w = FahPop(FAH);
	for (k = 0; k < 14; k ++)
            {
              y = voisin14b(w, k, rs, n, N);
              if ((y!=-1) && (M[y]==0) && (B[y]==B[w]) && (!masque || MA[y]))
              {M[y] = nlabels; FahPush(FAH, y, 0); } /* if y ... */
            } /* for k ... */
      } /* while (! FahVide(FAH)) */
    } /* if (M[x] == 0) */
  } /* for (x = 0; x < N; x++) */
  
  /* ================================================ */
  /* INITIALISATION DE LA FAH                         */
  /* ================================================ */

  printf("%s: initialisation FAH\n", F_NAME);
  FahFlush(FAH);
  FahPush(FAH, -1, 0);   /* force la creation du niveau 0 dans la Fah. */
                         /* NECESSAIRE pour eviter la creation prematuree */
                         /* de la file d'urgence */ 

  for (x = 0; x < N; x++)
  {
    if (M[x] && (!masque || MA[x]))    /* on va empiler les voisins des regions marquees */
    {
      for (k = 0; k < 14; k ++)
      {
	y = voisin14b(x, k, rs, n, N);
	if ((y!=-1)&&!M[y]&&!IsSet(y,EN_FAH)){FahPush(FAH, y, F[y]); Set(y, EN_FAH); }
      } /* for (k = 0; k < 8; k += 2) */
    } /* if (B[x]) */
  } /* for (x = 0; x < N; x++) */  
  x = FahPop(FAH);
#ifdef PARANO
  if (x != -1)
  {   
    fprintf(stderr,"%s : ORDRE FIFO NON RESPECTE PAR LA FAH !!!\n", F_NAME);
    return(0);
  }
#endif
  
  /* ================================================ */
  /* INONDATION                                       */
  /* ================================================ */
  printf("%s: Innondation \n", F_NAME);
  nlabels += 1;          /* cree le label pour les points de la LPE */
  while (! FahVide(FAH))
  {
    x = FahPop(FAH);
    UnSet(x, EN_FAH);
    
    ncc = 0;
    
    for (k = 0; k < 14; k ++)
    {
      y = voisin14b(x, k, rs, n, N);
      assert(y < N);     // sinon on innonde en dehors de l'image
      if ((y != -1) && (M[y] != 0) && (M[y] != nlabels) && llpemeyer_NotIn(M[y], etiqcc, ncc)) 
      {
	etiqcc[ncc] = M[y];        	
	ncc += 1;
      }
    } /* for k */
        
    if (ncc == 1)
    {
      M[x] = etiqcc[0];
      
      for (k = 0; k < 14; k ++)
      {
	y = voisin14b(x, k, rs, n, N);     
	if ((y != -1) && (M[y] == 0) && (! IsSet(y, EN_FAH)) && (!masque || MA[y]))
	{
	  FahPush(FAH, y, F[y]);
	  Set(y, EN_FAH);
	} /* if ((y != -1) && (! IsSet(y, EN_FAH))) */
      } /* for k */
      
    } 
    else 
      if (ncc > 1)
      {
	M[x] = nlabels;
      }
  } /* while (! FahVide(FAH)) */
  /* FIN PROPAGATION */
  
  for (x = 0; x < N; x++)
  {
    if ((M[x] == nlabels) || (M[x] == 0)) F[x] = 255; else F[x] = 0;
  }

  /* ================================================ */
  /* UN PEU DE MENAGE                                 */
  /* ================================================ */

  IndicsTermine();
  FahTermine(FAH);
  free(M);
  return(1);
} /* llpemeyerbiconnecte3d() */

