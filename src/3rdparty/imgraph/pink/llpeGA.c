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
   Ligne de partage des eaux sur les aretes d'un graphe et segmentation hiérarchique

   Jean Cousty - 2004-2006 
*/
 
//#define ANIMATE

//#define PARANO                 /* even paranoid people have ennemies */
//#define VERBOSE

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <float.h>
#include <mccodimage.h>
#include <jccodimage.h>
#include <mcimage.h>
#include <jcimage.h>
#include <mcfifo.h>
#include <jcgraphes.h>
#include <llpeGA.h>
#include <mcrbt.h>
#include <jclabelextrema.h>
#include <mclifo.h>
#include <mcutil.h>

#define EN_FAH 0
int32_t Stream(uint8_t *F, GrapheBasic *g, int32_t sommet, Lifo *FIFO, int32_t *Label, int32_t *alt, uint8_t *G);
int32_t StreamGArecursif(struct xvimage *ga, int32_t x, Lifo *FIFO, int32_t *Label, int32_t *alt, uint8_t *G);
int32_t StreamGAFloat(struct xvimage *ga, int32_t x, Lifo *L, Lifo *B,int32_t *psi, float *G);
int32_t StreamGADouble(struct xvimage *ga, int32_t x, Lifo *L, Lifo *B,int32_t *psi, double *G);
int32_t StreamGA(struct xvimage *ga, int32_t x, Lifo *L, Lifo *B,int32_t *psi, uint8_t *G);


int32_t altitudePoint(struct xvimage *ga, int32_t i)
{
  int32_t rs = rowsize(ga);                 /* taille ligne */
  int32_t cs = colsize(ga);                 /* taille colonne */
  int32_t N = rs * cs;                      /* taille image */
  uint8_t *F = UCHARDATA(ga);     /* l'image de depart */
  int32_t k, min, u;
  min = 255;
  for(k = 0; k < 4; k++)
    if( (u = incidente(i, k, rs, N)) != -1) { 
      if((int32_t)F[u] < min) min = (int32_t)F[u];
    }
  return min;
} 

/*  ga en sortie est une M-border watershed de ga en entree.
    De plus retourne une carte de labels des sommets ds un minimum de F */
/*  mBorderWshed2d was previously called flowLPE2d*/
struct xvimage *mBorderWshed2d(struct xvimage *ga)
#undef F_NAME
#define F_NAME "mBorderWshed2d"
{
  int32_t i,j,k,x,y,z,u, nlabels;
  struct xvimage *res;
  uint32_t *Eminima;
  uint32_t *Vminima; 
  int32_t rs = rowsize(ga);               /* taille ligne */
  int32_t cs = colsize(ga);               /* taille colonne */
  int32_t N = rs * cs;                    /* taille image */
  uint8_t *F = UCHARDATA(ga);            /* l'image de depart */ 
  uint8_t *VF;                           /* fonction indicatrice sur les
					     sommets */
  Lifo *L;
  
  /******************INITIALISATION*********************/ 
  if(!jclabelextrema(ga, &Eminima, 1, &nlabels)) {
    fprintf(stderr,"%s erreur de label extrema \n",F_NAME);
    exit(1);
  }
  
  //printf("nlabels %d \n", nlabels);
  
  if( (res = allocimage(NULL, rs, cs, 1, VFF_TYP_4_BYTE)) == NULL) {
    fprintf(stderr,"%s erreur de allocimage \n", F_NAME);
    exit(1);
  }
  Vminima = SLONGDATA(res);

  if( (VF = malloc(sizeof(uint8_t) * N)) == NULL) {
    fprintf(stderr,"%s ne peut allouer VF \n", F_NAME);
    exit(1);
  }  

  /* Valuation des sommets et calcul de V_M a partir de E_M */
  for(i = 0; i < N; i++) {
    VF[i] = 255;
    Vminima[i] = 0;
    for(k = 0; k < 4; k++)
      if( (u = incidente(i, k, rs, N)) != -1) { 
	if(F[u] < VF[i]) VF[i] = F[u];
	if(Eminima[u] > 0) Vminima[i] = Eminima[u];
      }
  }

  /* Initialisation de la FIFO */
  L = CreeLifoVide(2*N); /* nbre maximum d'arete ds un ga 4 connexe
			    sans regarder les bords */
  
  /* Les aretes adjacentes a un minimum sont inserees dans L */
  /* on explore d'abord les aretes horizontales */
  for(j = 0; j < cs; j++)
    for(i = 0; i < rs -1; i++){
      u = j * rs + i; x = Sommetx(u,N,rs); y = Sommety(u,N,rs);
      if( (mcmin(Vminima[x], Vminima[y]) == 0) && /* un des deux sommets non ds un minima */
	  (mcmax(Vminima[x], Vminima[y]) > 0) ) /* et l'autre dans un minima */
	LifoPush(L,u); 
    }
  /* puis les aretes verticales */
   for(j = 0; j < cs -1; j++)
    for(i = 0; i < rs; i++)
    {
      u = N + j * rs + i; x = Sommetx(u,N,rs); y = Sommety(u,N,rs);
      if( (mcmin(Vminima[x], Vminima[y]) == 0) && /* un des deux sommets non ds un minima */
	  (mcmax(Vminima[x], Vminima[y]) > 0) ) /* et l'autre dans un minima */
	LifoPush(L,u);
    } 
   
   /*************BOUCLE PRINCIPALE*****************/
   while(!LifoVide(L)) {
     u = LifoPop(L);
     x = Sommetx(u, N, rs);
     y = Sommety(u, N, rs);
     if (VF[x] > VF[y]) {z = y; y = x; x = z;}
     if((VF[x] < F[u]) && (VF[y] == F[u])){      /* u est une arete de bord */
       F[u] = VF[x];
       VF[y] = F[u];
       Eminima[u] = Vminima[x];
       Vminima[y] = Vminima[x];
       for(k = 0; k < 8; k +=2){
	 if( (z = voisin(y, k, rs, N)) != -1)
	   if( (Vminima[z] == 0))
	     LifoPush(L, Arete(y,z,rs,N));
       } /* for(k = 0 .. */
     } /* if( (VF[x] < ... */
   }/* while(!LifoVide... */
   free(Eminima); free(VF);
   return res;
}

/*  ga en sortie est une M-border watershed de ga en entree.
    De plus retourne une carte de labels des sommets ds un minimum de F */
/*  mBorderWshed2d was previously called flowLPE2d*/
struct xvimage *mBorderWshed2drapide(struct xvimage *ga)
#undef F_NAME
#define F_NAME "mBorderWshed2drapide"
{
  int32_t i,j,k,x,y,z,w,u, nlabels, label;
  struct xvimage *res;
  //  uint32_t *Eminima;
  int32_t *Vminima; 
  int32_t rs = rowsize(ga);               /* taille ligne */
  int32_t cs = colsize(ga);               /* taille colonne */
  int32_t N = rs * cs;                    /* taille image */
  uint8_t *F = UCHARDATA(ga);            /* l'image de depart */ 
  uint8_t *VF;                           /* fonction indicatrice sur les
					     sommets */
  Lifo *L;
  
  /******************INITIALISATION*********************/ 
  /* if(!jclabelextrema(ga, &Eminima, 1, &nlabels)) {
    fprintf(stderr,"%s erreur de label extrema \n",F_NAME);
    exit(1);
    }*/

  if( (res = allocimage(NULL, rs, cs, 1, VFF_TYP_4_BYTE)) == NULL) {
    fprintf(stderr,"%s erreur de allocimage \n", F_NAME);
    exit(1);
  }
  Vminima = SLONGDATA(res);

  if( (VF = malloc(sizeof(uint8_t) * N)) == NULL) {
    fprintf(stderr,"%s ne peut allouer VF \n", F_NAME);
    exit(1);
  }  

  /* Valuation des sommets */
  for(i = 0; i < N; i++) {
    VF[i] = 255;
    Vminima[i] = -1;
    for(k = 0; k < 4; k++)
      if( (u = incidente(i, k, rs, N)) != -1)
	if(F[u] < VF[i]) VF[i] = F[u];
  }
  
  /* Initialisation de la FIFO */
  L = CreeLifoVide(2*N); /* nbre maximum d'arete ds un ga 4 connexe
			    sans regarder les bords */
  /* Calcul des sommets qui sont dans des minima de F */
 
  nlabels = 0;

  for(x = 0; x < N; x++){
    if(Vminima[x] == -1){ /* On trouve un sommet non encore etiquete */ 
      nlabels ++;
      Vminima[x] = nlabels;
      LifoPush(L,x);
      while(!LifoVide(L)) {
	w = LifoPop(L);
	label = Vminima[w];
	for(k = 0; k < 4; k++)
	  if( (u = incidente(w, k, rs, N)) != -1){
	    if(F[u] == VF[w]){
	      switch(k){
	      case 0: y = w+1; break;      /* EST   */ 
	      case 1: y = w-rs; break;     /* NORD  */
	      case 2: y = w-1; break;      /* OUEST */
	      case 3: y = w+rs; break;     /* SUD   */ 
	      }
	      if( (label > 0) && (VF[y] < VF[w]) ){
		label = 0;
		nlabels --;
		Vminima[w] = label;
		LifoPush(L,w);
	      }
	      else if(VF[y] == VF[w]){
		if( ( (label > 0) && (Vminima[y] == -1) ) ||
		    ( (label == 0) && (Vminima[y] != 0)) ){
		  Vminima[y] = label;
		  LifoPush(L,y);
		}
	      }
	    }
	  }
      }
    }
  }
  //  printf("nlabels %d \n",nlabels);

  /* Les aretes adjacentes a un minimum sont inserees dans L */
  /* on explore d'abord les aretes horizontales */
  for(j = 0; j < cs; j++)
    for(i = 0; i < rs -1; i++){
      u = j * rs + i; x = Sommetx(u,N,rs); y = Sommety(u,N,rs);
      if( (mcmin(Vminima[x], Vminima[y]) == 0) && /* un des deux sommets non ds un minima */
	  (mcmax(Vminima[x], Vminima[y]) > 0) ) /* et l'autre dans un minima */
	LifoPush(L,u); 
    }
  /* puis les aretes verticales */
   for(j = 0; j < cs -1; j++)
    for(i = 0; i < rs; i++)
    {
      u = N + j * rs + i; x = Sommetx(u,N,rs); y = Sommety(u,N,rs);
      if( (mcmin(Vminima[x], Vminima[y]) == 0) && /* un des deux sommets non ds un minima */
	  (mcmax(Vminima[x], Vminima[y]) > 0) ) /* et l'autre dans un minima */
	LifoPush(L,u);
    } 
   
   /*************BOUCLE PRINCIPALE*****************/
   while(!LifoVide(L)) {
     u = LifoPop(L);
     x = Sommetx(u, N, rs);
     y = Sommety(u, N, rs);
     if (VF[x] > VF[y]) {z = y; y = x; x = z;}
     if((VF[x] < F[u]) && (VF[y] == F[u])){      /* u est une arete de bord */
       F[u] = VF[x];
       VF[y] = F[u];
       //  Eminima[u] = Vminima[x];
       Vminima[y] = Vminima[x];
       for(k = 0; k < 8; k +=2){
	 if( (z = voisin(y, k, rs, N)) != -1)
	   if( (Vminima[z] == 0))
	     LifoPush(L, Arete(y,z,rs,N));
       } /* for(k = 0 .. */
     } /* if( (VF[x] < ... */
   }/* while(!LifoVide... */
   // free(Eminima); 
   free(VF);
   return res;
}// mBorderWshed2drapide(...)

/* LPE d'un graphe a aretes valuees, base sur l'algo recursif pour le
   calcul des streams */
int32_t lpeGrapheAreteValuee(GrapheValue *gv, int32_t* Label)
#undef F_NAME
#define F_NAME "LPEGrapheAreteValuee" 
{
  int32_t i,som, alt, nb_labs, labstream;
  GrapheBasic *g = gv->g;
  int32_t nb_som = g->nsom;
  Lifo *FIFO;  
  FIFO = CreeLifoVide(nb_som);
  PBasicCell p;
  uint8_t *G;
  uint8_t min_som ;

  if( (G = malloc(sizeof(uint8_t) * nb_som)) == NULL){
    fprintf(stderr,"%s: erreur de malloc \n", F_NAME);
    exit(0);
  }

  /* Initialisation */
  for(i = 0; i < nb_som; i++)
  {
    min_som = 255;
    for(p = g->gamma[i]; p != NULL; p = p->next){
      if(gv->F[p->edge] < min_som) min_som = gv->F[p->edge];
    }
    G[i] = min_som;
    Label[i] = NO_LABEL;
  }
  nb_labs = -1;
  /* Boucle principale */
  for(i = 0; i < nb_som; i++){
    if(Label[i] == NO_LABEL){
      alt = INT32_MAX;
      labstream = Stream(gv->F, g, i, FIFO, Label, &alt, G);
      if(labstream == NO_LABEL){
	nb_labs++;
	//	printf("nouvelle region\n");
 	while(!LifoVide(FIFO)){
	  som = LifoPop(FIFO);
	  Label[som] = nb_labs;
	}// while(!LifoVide(L))
      } else {
	while(!LifoVide(FIFO)){
	  som = LifoPop(FIFO);
	  Label[som] = labstream;
	}//while(!LifoVide(L))
      }//if(!labstream) 
    }//if(Label[i] == -1)
  }
  LifoTermine(FIFO);
  free(G);
  return nb_labs+1;
}

/* Stream versions recursive ds le cas d'un graphe a aretes valuees*/
int32_t Stream(uint8_t *F, GrapheBasic *g, int32_t sommet, Lifo *FIFO, int32_t *Label, int32_t *alt, uint8_t *G)
#undef F_NAME
#define F_NAME "LPEGrapheAreteValuee" 
{
  PBasicCell p;
  int32_t labStream;
  Label[sommet] = IN_PROCESS;
  LifoPush(FIFO, sommet);
  for(p = g->gamma[sommet]; p != NULL; p = p->next) 
    if(F[p->edge] == G[sommet]){
      if(Label[p->vertex] == NO_LABEL){
	labStream = Stream(F, g, (int32_t)(p->vertex), FIFO, Label, alt,G);
	if( (labStream >= 0) || ( (*alt) < (int32_t)G[sommet]) )
	  return labStream;
      }
      else if(Label[p->vertex] >= 0)
	return Label[p->vertex];
    }
  
  (*alt) = (int32_t)G[sommet];
  return NO_LABEL;
} 

/* Returns (in the form of a 4-connected GA) the edges that link two
   points with different labels */
struct xvimage *SeparatingEdge(struct xvimage *labels)
#undef F_NAME
#define F_NAME "mSeparatingEdge"
{
  struct xvimage *ga;
  int32_t *lab = SLONGDATA(labels);
  int32_t rs = rowsize(labels);     /* taille ligne */
  int32_t cs = colsize(labels);     /* taille colonne */
  int32_t N = rs * cs;              /* taille image */
  int32_t i,j,u,x,y;

  if( (ga = allocGAimage(NULL, rs, cs, 1, VFF_TYP_GABYTE)) == NULL) {
    fprintf(stderr,"%s: ne peut allouer de GA \n", F_NAME);
      exit(1);
  }
  uint8_t *F = UCHARDATA(ga);      /* le resultat */
  memset(F,0,2*N);
  /* les aretes horizontales */ 
  for(j = 0; j < cs; j++)
    for(i = 0; i < rs -1; i++){
      u = j * rs + i; x = Sommetx(u,N,rs); y = Sommety(u,N,rs);
      if(lab[x] != lab[y])
	F[u] = 255;
    }
  /* puis les aretes verticales */
  for(j = 0; j < cs -1; j++)
    for(i = 0; i < rs; i++){
      u = N + j * rs + i; x = Sommetx(u,N,rs); y = Sommety(u,N,rs);
      if(lab[x] != lab[y])
	F[u] = 255;
    }
  return ga;
}


/* Calcul le flow mapping (cf. cite{XXX}) d'un GA 4-connexe */
/* Algo. recursif : l'exploration est en profondeur d'abord */
/* Attention Label est suppose alloue a la bonne taille ... */
/* SI Pb avec cet algo, taper d'abord: unlimit stacksize    */ 
int32_t flowMappingRecursif(struct  xvimage* ga, int32_t* Label) 
#undef F_NAME
#define F_NAME "LPEGrapheAreteValuee" 
{
  int32_t rs = rowsize(ga);               /* taille ligne */
  int32_t cs = colsize(ga);               /* taille colonne */
  int32_t N = rs * cs;                    /* nb_som GA */
  int32_t i,som, alt, nb_labs, labstream;
  uint8_t *G;
  Lifo *FIFO; 
  FIFO = CreeLifoVide(N);
  if( (G = malloc(sizeof(uint8_t) * N)) == NULL){
    fprintf(stderr,"%s: erreur de malloc \n", F_NAME);
    exit(0);
  }

  /* Initialisation */
  for(i = 0; i < N; i++){
    G[i] = (uint8_t)altitudePoint(ga,i);
    Label[i] = NO_LABEL;
  }
  nb_labs = -1;
  /* Boucle principale */
  for(i = 0; i < N; i++){
    if(Label[i] == NO_LABEL){
      alt = INT32_MAX;
      labstream = StreamGArecursif(ga,i,FIFO,Label, &alt, G);
      if(labstream == NO_LABEL){
	nb_labs++;
 	while(!LifoVide(FIFO)){
	  som = LifoPop(FIFO);
	  Label[som] = nb_labs;
	}// while(!LifoVide(L))
      } else {
	while(!LifoVide(FIFO)){
	  som = LifoPop(FIFO);
	  Label[som] = labstream;
	}//while(!LifoVide(L))
      }//if(!labstream) 
    }//if(Label[i] == -1)
  }
  LifoTermine(FIFO);
  free(G);
  return nb_labs+1;
} 

int32_t StreamGArecursif(struct xvimage *ga, int32_t x, Lifo *FIFO, int32_t *Label, int32_t *alt, uint8_t *G)
#undef F_NAME
#define F_NAME "LPEGrapheAreteValuee" 
{
  int32_t rs = rowsize(ga);               /* taille ligne */
  int32_t cs = colsize(ga);               /* taille colonne */
  int32_t N = rs * cs;                    /* taille image */
  uint8_t *F = UCHARDATA(ga);
  int32_t labStream,k,u,y;
  Label[x] = IN_PROCESS;
  LifoPush(FIFO, x);
  for(k = 0; k < 4; k++)
    if((u = incidente(x, k, rs, N)) != -1) 
      if(F[u] == G[x]){
	switch(k){
	case 0: y = x+1; break;      /* EST   */ 
	case 1: y = x-rs; break;     /* NORD  */
	case 2: y = x-1; break;      /* OUEST */
	case 3: y = x+rs; break;     /* SUD   */ 
	}
	if(Label[y] == NO_LABEL) {
	  labStream = StreamGArecursif(ga, y, FIFO, Label, alt,G);
	  if( (labStream >= 0) || ( (*alt) < (int32_t)G[x]) )
	    return labStream;
	}
	else if (Label[y] >= 0){
	  //(*alt) = G[y];
	  return Label[y];
	}	  
      }
  (*alt) = (int32_t)G[x];
  return NO_LABEL;
}

/* Calcul le flow mapping (cf. cite{XXX}) d'un GA 4-connexe */
/* Algo. non recursif : l'exploration est, à la fois en     */
/* profondeur et en largeur d'abord.                        */
/* Attention Label est suppose alloue a la bonne taille ... */
int32_t flowMapping(struct  xvimage* ga, int32_t* Label) 
#undef F_NAME
#define F_NAME "LPEGrapheAreteValuee" 
{
  int32_t rs = rowsize(ga);               /* taille ligne */
  int32_t cs = colsize(ga);               /* taille colonne */
  int32_t N = rs * cs;                    /* nb_som GA */
  int32_t i,som, nb_labs, labstream;
  uint8_t *G;
  Lifo *FIFO, *B; 
  FIFO = CreeLifoVide(N);
  B = CreeLifoVide(N);
  if( (G = malloc(sizeof(uint8_t) * N)) == NULL){
    fprintf(stderr,"%s: erreur de malloc \n", F_NAME);
    exit(0);
  }

  /* Initialisation */
  for(i = 0; i < N; i++){
    G[i] = (uint8_t)altitudePoint(ga,i);
    Label[i] = NO_LABEL;
  }
  
  nb_labs = -1;
  /* Boucle principale */
  for(i = 0; i < N; i++){
    if(Label[i] == NO_LABEL){
      labstream = StreamGA(ga,i,FIFO,B,Label, G);
      if(labstream == NO_LABEL){
	nb_labs++;
 	while(!LifoVide(FIFO)){
	  som = LifoPop(FIFO);
	  Label[som] = nb_labs;
	}// while(!LifoVide(L))
      } else {
	while(!LifoVide(FIFO)){
	  som = LifoPop(FIFO);
	  Label[som] = labstream;
	}//while(!LifoVide(L))
      }//if(!labstream) 
    }//if(Label[i] == -1)
  }
  LifoTermine(FIFO);
  LifoTermine(B);
  free(G);
  return nb_labs+1;
}

/* Calcul de Stream par un algo mixant exploration en profondeur et
   largeur d'abord des chemins de plus grande pente */
int32_t StreamGA(struct xvimage *ga, int32_t x, Lifo *L, Lifo *B,int32_t *psi, uint8_t *G)
#undef F_NAME
#define F_NAME "LPEGrapheAreteValuee" 
{
  //  Lifo *B;                                /* Les bottoms non encore exploré de L */ 
  int32_t rs = rowsize(ga);               /* taille ligne */
  int32_t cs = colsize(ga);               /* taille colonne */
  int32_t N = rs * cs;                    /* taille image */
  uint8_t *F = UCHARDATA(ga);
  int32_t y, k, u, z;
  uint8_t breadth_first;
  
  LifoPush(L,x);
  psi[x] = IN_PROCESS;
  LifoPush(B,x);

  while(!LifoVide(B)){
    y = LifoPop(B);
    breadth_first = TRUE;
    for(k = 0; (k < 4) && (breadth_first == TRUE); k++)
      if((u = incidente(y, k, rs, N)) != -1) 
	if(F[u] == G[y]){
	  switch(k){
	  case 0: z = y+1; break;      /* EST   */ 
	  case 1: z = y-rs; break;     /* NORD  */
	  case 2: z = y-1; break;      /* OUEST */
	  case 3: z = y+rs; break;     /* SUD   */
	  }
	  if(psi[z] != IN_PROCESS)
	  {
	    if(psi[z] != NO_LABEL){
	      /* There is an inf-stream under L */
	      LifoFlush(B);  
	      return psi[z];
	    }
	    else
	    {
	      if(G[z] < G[y]){
		LifoPush(L,z);  
		psi[z] = IN_PROCESS;
		/* z is now the only bottom of L */
		LifoFlush(B);
		LifoPush(B,z); /* hence, switch to depth first */
		breadth_first = FALSE;
	      }
	      else{
		psi[z] = IN_PROCESS;
		LifoPush(L,z); /* G[z] == G[y], then z is also a bottom of L */
		LifoPush(B,z);
	      }
	    }
	  }
	}
  }
  LifoFlush(B);
  return NO_LABEL;
}

float altitudePointFloat(struct xvimage *ga, int32_t i)
{
  int32_t rs = rowsize(ga);                 /* taille ligne */
  int32_t cs = colsize(ga);                 /* taille colonne */
  int32_t N = rs * cs;                      /* taille image */
  float *F = FLOATDATA(ga);     /* l'image de depart */
  int32_t k, u;
  float min = 255; // En theorie ca peut aller bien plus haut attention !!! MAX_FLOAT
  for(k = 0; k < 4; k++)
    if( (u = incidente(i, k, rs, N)) != -1) { 
      if((float)F[u] < min) min = (float)F[u];
    }
  return min;
}

/* Calcul le flow mapping (cf. cite{XXX}) d'un GA 4-connexe */
/* Algo. non recursif : l'exploration est, à la fois en     */
/* profondeur et en largeur d'abord.                        */
/* Attention Label est suppose alloue a la bonne taille ... */
int32_t flowMappingFloat(struct  xvimage* ga, int32_t* Label) 
#undef F_NAME
#define F_NAME "LPEGrapheAreteValuee" 
{
  int32_t rs = rowsize(ga);               /* taille ligne */
  int32_t cs = colsize(ga);               /* taille colonne */
  int32_t N = rs * cs;                    /* nb_som GA */
  int32_t i,som, nb_labs, labstream;
  float *G;
  Lifo *FIFO, *B; 
  FIFO = CreeLifoVide(N);
  B = CreeLifoVide(N);
  if( (G = malloc(sizeof(float) * N)) == NULL){
    fprintf(stderr,"%s: erreur de malloc \n", F_NAME);
    exit(0);
  }

  /* Initialisation */
  for(i = 0; i < N; i++){
    G[i] = (float)altitudePointFloat(ga,i);
    Label[i] = NO_LABEL;
  }
  
  nb_labs = -1;
  /* Boucle principale */
  for(i = 0; i < N; i++){
    if(Label[i] == NO_LABEL){
      labstream = StreamGAFloat(ga,i,FIFO,B,Label, G);
      if(labstream == NO_LABEL){
	nb_labs++;
 	while(!LifoVide(FIFO)){
	  som = LifoPop(FIFO);
	  Label[som] = nb_labs;
	}// while(!LifoVide(L))
      } else {
	while(!LifoVide(FIFO)){
	  som = LifoPop(FIFO);
	  Label[som] = labstream;
	}//while(!LifoVide(L))
      }//if(!labstream) 
    }//if(Label[i] == -1)
  }
  LifoTermine(FIFO);
  LifoTermine(B);
  free(G);
  return nb_labs+1;
}

/* Calcul de Stream par un algo mixant exploration en profondeur et
   largeur d'abord des chemins de plus grande pente */
int32_t StreamGAFloat(struct xvimage *ga, int32_t x, Lifo *L, Lifo *B,int32_t *psi, float *G)
#undef F_NAME
#define F_NAME "LPEGrapheAreteValuee" 
{
  //  Lifo *B;                                /* Les bottoms non encore exploré de L */ 
  int32_t rs = rowsize(ga);               /* taille ligne */
  int32_t cs = colsize(ga);               /* taille colonne */
  int32_t N = rs * cs;                    /* taille image */
  float *F = FLOATDATA(ga);
  int32_t y, k, u, z;
  uint8_t breadth_first;
  
  LifoPush(L,x);
  psi[x] = IN_PROCESS;
  LifoPush(B,x);

  while(!LifoVide(B)){
    y = LifoPop(B);
    breadth_first = TRUE;
    for(k = 0; (k < 4) && (breadth_first == TRUE); k++)
      if((u = incidente(y, k, rs, N)) != -1) 
	if(F[u] == G[y]){
	  switch(k){
	  case 0: z = y+1; break;      /* EST   */ 
	  case 1: z = y-rs; break;     /* NORD  */
	  case 2: z = y-1; break;      /* OUEST */
	  case 3: z = y+rs; break;     /* SUD   */
	  }
	  if(psi[z] != IN_PROCESS)
	  {
	    if(psi[z] != NO_LABEL){
	      /* There is an inf-stream under L */
	      LifoFlush(B);  
	      return psi[z];
	    }
	    else
	    {
	      if(G[z] < G[y]){
		LifoPush(L,z);  
		psi[z] = IN_PROCESS;
		/* z is now the only bottom of L */
		LifoFlush(B);
		LifoPush(B,z); /* hence, switch to depth first */
		breadth_first = FALSE;
	      }
	      else{
		psi[z] = IN_PROCESS;
		LifoPush(L,z); /* G[z] == G[y], then z is also a bottom of L */
		LifoPush(B,z);
	      }
	    }
	  }
	}
  }
  LifoFlush(B);
  return NO_LABEL;
}

double altitudePointDouble(struct xvimage *ga, int32_t i)
{
  int32_t rs = rowsize(ga);                 /* taille ligne */
  int32_t cs = colsize(ga);                 /* taille colonne */
  int32_t N = rs * cs;                      /* taille image */
  double *F = DOUBLEDATA(ga);     /* l'image de depart */
  int32_t k, u;
  //double min = 956036423.000000;
  double min = 1000000000000000000000000000000.0; // En theorie ca peut aller bien plus haut attention !!! MAX_FLOAT
  for(k = 0; k < 4; k++)
    if( (u = incidente(i, k, rs, N)) != -1) { 
      if(F[u] < min) min = F[u];
    }
  return min;
}

/* Calcul le flow mapping (cf. cite{XXX}) d'un GA 4-connexe */
/* Algo. non recursif : l'exploration est, à la fois en     */
/* profondeur et en largeur d'abord.                        */
/* Attention Label est suppose alloue a la bonne taille ... */
int32_t flowMappingDouble(struct  xvimage* ga, int32_t* Label) 
#undef F_NAME
#define F_NAME "LPEGrapheAreteValuee" 
{
  int32_t rs = rowsize(ga);               /* taille ligne */
  int32_t cs = colsize(ga);               /* taille colonne */
  int32_t N = rs * cs;                    /* nb_som GA */
  int32_t i,som, nb_labs, labstream;
  double *G;
  Lifo *FIFO, *B; 
  FIFO = CreeLifoVide(N);
  B = CreeLifoVide(N);
  //  printf("MAX_DBL %lf \n", DBL_MAX);
  if( (G = malloc(sizeof(double) * N)) == NULL){
    fprintf(stderr,"%s: erreur de malloc \n", F_NAME);
    exit(0);
  }

  /* Initialisation */
  for(i = 0; i < N; i++){
    G[i] = altitudePointDouble(ga,i);
    Label[i] = NO_LABEL;
  }
  
  nb_labs = -1;
  /* Boucle principale */
  for(i = 0; i < N; i++){
    if(Label[i] == NO_LABEL){
      labstream = StreamGADouble(ga,i,FIFO,B,Label, G);
      if(labstream == NO_LABEL){
	nb_labs++;
 	while(!LifoVide(FIFO)){
	  som = LifoPop(FIFO);
	  Label[som] = nb_labs;
	}// while(!LifoVide(L))
      } else {
	while(!LifoVide(FIFO)){
	  som = LifoPop(FIFO);
	  Label[som] = labstream;
	}//while(!LifoVide(L))
      }//if(!labstream) 
    }//if(Label[i] == -1)
  }
  LifoTermine(FIFO);
  LifoTermine(B);
  free(G);
  return nb_labs+1;
}

/* Calcul de Stream par un algo mixant exploration en profondeur et
   largeur d'abord des chemins de plus grande pente */
int32_t StreamGADouble(struct xvimage *ga, int32_t x, Lifo *L, Lifo *B,int32_t *psi, double *G)
#undef F_NAME
#define F_NAME "LPEGrapheAreteValuee" 
{
  int32_t rs = rowsize(ga);               /* taille ligne */
  int32_t cs = colsize(ga);               /* taille colonne */
  int32_t N = rs * cs;                    /* taille image */
  double *F = DOUBLEDATA(ga);
  int32_t y, k, u, z;
  uint8_t breadth_first;
  
  LifoPush(L,x);
  psi[x] = IN_PROCESS;
  LifoPush(B,x);

  while(!LifoVide(B)){
    y = LifoPop(B);
    breadth_first = TRUE;
    for(k = 0; (k < 4) && (breadth_first == TRUE); k++)
      if((u = incidente(y, k, rs, N)) != -1) 
	if(F[u] == G[y]){
	  switch(k){
	  case 0: z = y+1; break;      /* EST   */ 
	  case 1: z = y-rs; break;     /* NORD  */
	  case 2: z = y-1; break;      /* OUEST */
	  case 3: z = y+rs; break;     /* SUD   */
	  }
	  if(psi[z] != IN_PROCESS)
	  {
	    if(psi[z] != NO_LABEL){
	      /* There is an inf-stream under L */
	      LifoFlush(B);  
	      return psi[z];
	    }
	    else
	    {
	      if(G[z] < G[y]){
		LifoPush(L,z);  
		psi[z] = IN_PROCESS;
		/* z is now the only bottom of L */
		LifoFlush(B);
		LifoPush(B,z); /* hence, switch to depth first */
		breadth_first = FALSE;
	      }
	      else{
		psi[z] = IN_PROCESS;
		LifoPush(L,z); /* G[z] == G[y], then z is also a bottom of L */
		LifoPush(B,z);
	      }
	    }
	  }
	}
  }
  LifoFlush(B);
  return NO_LABEL;
}

