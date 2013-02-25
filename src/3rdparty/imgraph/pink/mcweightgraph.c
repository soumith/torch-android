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

/* authors : J. Cousty - L. Najman and M. Couprie */


/* $Id: mcgraphe.c,v 1.4 2006/02/28 07:49:16 michel Exp $ */
/*
      Librairie mcgraphe :

      Gestion de graphes de regions

      Michel Couprie, aout 1997

      Principe du codage :
        - a chaque sommet (region) est associe une liste de codes d'aretes
        - les regions "atomiques" sont les pixels de l'image originale
        - dans une image a N pixels, pour un pixel i, on code par:
            i     l'arete liant le pixel i a son voisin est i+1,
            N+i   l'arete liant le pixel i a son voisin sud,
          et de plus, pour le 8-voisinage:
            2N+i  l'arete liant le pixel i a son voisin sud-est,
            3N+i  l'arete liant le pixel i a son voisin sud-ouest.
        - les regions sont obtenues par fusion de regions (voir mcfus).
*/
#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include <mcsort.h>
#include <mcweightgraph.h>

#define DEBUGGRAPHE

/* ================================================ */
/* ================================================ */
/* fonctions privees */
/* ================================================ */
/* ================================================ */

/* ========================================== */
static void erreur(char * mess)
/* ========================================== */
{
  fprintf(stderr, "%s\n", mess);
  exit(0);
} /* erreur() */

/*#ifdef DEBUGGRAPHE
/* ====================================================================== */
void printliste(pcell p)
/* ====================================================================== */
{
  for (; p != NULL; p = p->next)
    printf("%d", p->index);
    
    
  printf("\n");
} /* printliste() */

void printindex(graphe * g)
/* ====================================================================== */
{

uint32_t i;
pcell p;
  
  for (i = 0; i < g->nbsom; i++)
  {
    printf("[%d] ", i);
    p = *(g->listar+i);
    for (; p != NULL; p = p->next){
    	printf("%d ", p->index);
	printf("w=%d :", g->weight[p->index]);
	}
  printf("\n");
  }	
        
  printf("\n");
} /* printindex() */

/* ====================================================================== */
void printgraphe(graphe * g)
/* ====================================================================== */
{
  uint32_t i;
  
   printf("[#id_vertex]: list of adjacent vertex \n", i);
  for (i = 0; i < g->nbsom; i++)
  {
    printf("[%d]: ", i);
    printliste(*(g->listar+i));
  }
  printf("\n");

   printf("[#id_edge]:  vertex conforming the edge \n", i);  
  for (i = 0; i < g->ind; i++)
  {	
     printf("[%d] ", i);
     printf("%d ", g->tete[i]);	
     printf("%d ", g->queue[i]);
     printf("\n");	
   }
} /* printgraphe() */
/*#endif

/* ================================================ */
/* ================================================ */
/* fonctions publiques */
/* ================================================ */
/* ================================================ */


/* ====================================================================== */
graphe * initgraphe(uint32_t nbsom, uint32_t nbmaxar)
/* ====================================================================== */
{
  graphe * g;
  uint32_t i;
  
  g = (graphe *)calloc(1,sizeof(graphe));
  g->cs = -1;
  g->rs = -1;
  if (g == NULL)
  {   fprintf(stderr, "initgraph : malloc failed for g\n");
      return(0);
  }
  g->tasar = (cell *)calloc(1,nbmaxar * sizeof(cell));
  if (g->tasar == NULL)
  {   fprintf(stderr, "initgraph : malloc failed for g->tasar\n");
      return(0);
  }
  g->listar = (pcell *)calloc(nbsom, sizeof(pcell));
  if (g->listar == NULL)
  {   fprintf(stderr, "initgraph : calloc failed for g->listar\n");
      return(0);
  }

  g->weight = (double *)calloc(1,nbmaxar * sizeof(double));
  g->nbsom = nbsom;
  g->nbmaxar = nbmaxar;
  g->nbar = 0;
  g->ind = 0;
  for (i = 0; i < nbmaxar - 1; i++)
    (g->tasar+i)->next = g->tasar+i+1;
  (g->tasar+i)->next = NULL;
  g->librear = g->tasar;  

  g->tete = (uint32_t*)calloc(g->nbmaxar, sizeof(uint32_t));
  if(g->tete == NULL){
    fprintf(stderr,"initgrap: calloc failed for g->tete\n");
    exit(1);
  }
  g->queue = (uint32_t*)calloc(g->nbmaxar, sizeof(uint32_t));
  if(g->queue == NULL){
    fprintf(stderr,"initgrap: calloc failed for g->queue\n");
    exit(1);
  }
  return g;
} /* initgraphe() */

/* ====================================================================== */
graphe * sjinitgraphe(uint32_t nbsom, uint64_t nbmaxar)
/* ====================================================================== */
{
  graphe * g;
  uint32_t i;
  
  g = (graphe *)calloc(1,sizeof(graphe));
  g->cs = -1;
  g->rs = -1;
  if (g == NULL)
  {   fprintf(stderr, "initgraph : malloc failed for g\n");
      return(0);
  }
  g->tasar = (cell *)calloc(1,nbmaxar * sizeof(cell));
  if (g->tasar == NULL)
  {   fprintf(stderr, "initgraph : malloc failed for g->tasar\n");
      return(0);
  }
  g->listar = (pcell *)calloc(nbsom, sizeof(pcell));
  if (g->listar == NULL)
  {   fprintf(stderr, "initgraph : calloc failed for g->listar\n");
      return(0);
  }

  g->weight = (double *)calloc(1,nbmaxar * sizeof(double));
  g->nbsom = nbsom;
  g->nbmaxar = nbmaxar;
  g->nbar = 0;
  g->ind = 0;
  for (i = 0; i < nbmaxar - 1; i++)
    (g->tasar+i)->next = g->tasar+i+1;
  (g->tasar+i)->next = NULL;
  g->librear = g->tasar;  

  g->tete = (uint32_t*)calloc(g->nbmaxar, sizeof(uint32_t));
  if(g->tete == NULL){
    fprintf(stderr,"initgrap: calloc failed for g->tete\n");
    exit(1);
  }
  g->queue = (uint32_t*)calloc(g->nbmaxar, sizeof(uint32_t));
  if(g->queue == NULL){
    fprintf(stderr,"initgrap: calloc failed for g->queue\n");
    exit(1);
  }
  return g;
} /* initgraphe() */

void setSize(graphe *g, int32_t rs , int32_t cs)
{
  g->rs = rs;
  g->cs = cs;
}


/* ====================================================================== */
void terminegraphe(graphe * g)
/* ====================================================================== */
{
  free(g->tasar);
  free(g->listar);
  free(g->weight);
  free(g->tete);
  free(g->queue);
  free(g);
} /* terminegraphe() */

/* ====================================================================== */
pcell allouecell(graphe * g)
/* ====================================================================== */
{
  pcell p;
  if (g->librear == NULL) erreur("allouecell : plus de cellules libres");
  p = g->librear;
  g->librear = g->librear->next;
  return p;
} /* allouecell() */

/* ====================================================================== */
void liberecell(graphe * g, pcell p)
/* ====================================================================== */
{
  p->next = g->librear;
  g->librear = p;
} /* liberecell() */

/* ====================================================================== */
void retiretete(graphe * g, pcell * pliste)
/* ====================================================================== */
{
  pcell p;
  p = (*pliste)->next;
  liberecell(g, *pliste);
  *pliste = p;
} /* retiretete() */

/* /\* ====================================================================== *\/ */
/* int32_t estarete(graphe * g, uint32_t som, uint32_t a) */
/* /\* ====================================================================== *\/ */
/* { */
/*   pcell liste; */
/*   liste = *(g->listar + som); */
/*   while (liste != NULL) */
/*     if (liste->val == a) return 1; */
/*     else liste = liste->next; */
/*   return 0; */
/* } /\* estarete() *\/ */


 
/* /\* ====================================================================== *\/ */
/* void retirearete(graphe * g, uint32_t som, uint32_t a) */
/* /\* ====================================================================== *\/ */
/* { */
/*   pcell * pliste; */
/*   pliste = g->listar + som; */
/*   while ((*pliste != NULL) && (((*pliste)->val) != a)) */
/*     pliste = &((*pliste)->next); */
/*   if (*pliste != NULL) retiretete(g, pliste); */
/*   g->nbar--; */
/* } /\* retirearete() *\/ */

/* /\* ====================================================================== *\/ */
/* void ajoutearete(graphe * g, uint32_t som, uint32_t a) */
/* /\* ====================================================================== *\/ */
/* { */
/*   pcell p; */
/*   p = allouecell(g); */
/*   p->next = *(g->listar + som); */
/*   p->val = a; */
/*   *(g->listar + som) = p; */
/*   g->nbar++; */
/* } /\* ajoutearete() *\/ */

/* ====================================================================== */
void ajoutearete2(graphe * g, uint32_t som, uint32_t a, uint32_t ind)
/* ====================================================================== */
{
  pcell p;
  p = allouecell(g);
  p->next = *(g->listar + som);
  //  p->val = a;
  p->index = ind;
  *(g->listar + som) = p;
  g->nbar++;
} /* ajoutearete() */

/* ====================================================================== */
void addarete(graphe * g, uint32_t som, uint32_t a, double weight)
/* ====================================================================== */
{
uint32_t i;
	
	i = g->ind;
	ajoutearete2(g, som, a,i);
/* 	indexarete(g, som, a, i); */
	ajoutearete2(g, a, som,i);
/* 	indexarete(g, a, som, i); */
	g->tete[i] = a;
	g->queue[i] = som;
	setweight(g, i, weight);
	g->ind= i+1;
  
} /* addarete() */


/* ====================================================================== */
void setweight(graphe * g, uint32_t index, double value)
/* ====================================================================== */
{
	g->weight[index]= value;
  
} /* setweight */

/* ====================================================================== */
double getweight(graphe * g, uint32_t index)
/* ====================================================================== */
{
	return g->weight[index];
 
} /* setweight */


/* /\* ====================================================================== *\/ */
/* void maille4graphe(graphe * g, uint32_t rs, uint32_t cs) */
/* /\* ====================================================================== *\/ */
/* { */
/*   uint32_t i, j; */
/*   uint32_t N = rs * cs; */
  
/*   for (j = 0; j < cs; j++) */
/*     for (i = 0; i < rs; i++) */
/*     { */
/*       if (i < rs - 1) */
/*       { */
/*         ajoutearete(g, j * rs + i, j * rs + i); */
/*         ajoutearete(g, j * rs + i + 1, j * rs + i); */
/*       } */
/*       if (j < cs - 1) */
/*       { */
/*         ajoutearete(g, j * rs + i, j * rs + i + N); */
/*         ajoutearete(g, (j+1) * rs + i, j * rs + i + N); */
/*       } */
/*     } */
/* } /\* maille4graphe() *\/ */


/* ====================================================================== */
void aretesommets(uint32_t a, uint32_t N, uint32_t rs, uint32_t * s1, uint32_t * s2)
/* ====================================================================== */
{
  if (a < N)          { *s1 = a; *s2 = a + 1; }  
  else if (a < 2 * N) { *s1 = a - N; *s2 = a - N + rs; }  
  else if (a < 3 * N) { *s1 = a - 2*N; *s2 = a - 2*N + rs + 1; }  
  else                { *s1 = a - 3*N; *s2 = a - 3*N + rs - 1; }  
} /* aretesommets() */

/* ================================================ */
/* ================================================ */
/* fonctions publiques pour la vision 'liste d'adjacence' */
/* (a chaque sommet est associee la liste des sommets adjacents) */
/* ================================================ */
/* ================================================ */

/* /\* ====================================================================== *\/ */
/* int32_t estsuccesseur(graphe * g, uint32_t som, uint32_t a) */
/* /\* ====================================================================== *\/ */
/* { /\* idem estarete, mais c'est la vision 'liste d'adjacence' *\/ */
/*   return estarete(g, som, a); */
/* } /\* estsuccesseur() *\/ */

/* /\* ====================================================================== *\/ */
/* int32_t estsymetrique(graphe * g) */
/* /\* ====================================================================== *\/ */
/* { */
/*   uint32_t i, j; */
  
/*   for (i = 0; i < g->nbsom - 1; i++) */
/*     for (j = i+1; j < g->nbsom; j++) */
/*     { */
/*       if (estsuccesseur(g, i, j)) */
/*       { */
/*         if (!estsuccesseur(g, j, i)) return 0; */
/*       } */
/*       else */
/*       { */
/*         if (estsuccesseur(g, j, i)) return 0; */
/*       } */
/*     } */
/*   return 1; */
/* } /\* estsymetrique() *\/ */


/* ====================================================================== */
/* ====================================================================== */
/* FONCTIONS D'ENTREE / SORTIE FICHIER POUR UN GRAPHE */
/* ====================================================================== */
/* ====================================================================== */

/* ====================================================================== */
/*! \fn graphe * ReadGraphe(char * filename)
    \param   filename (entr�) : nom du fichier graphe.
    \return un graphe.
    \brief Lit les donn�s d'un graphe dans le fichier filename, retourne un pointeur sur la structure graphe construite. 
*/
graphe * ReadGraphe(char * filename,double **Fv)
/* ====================================================================== */
#undef F_NAME
#define F_NAME "ReadGraphe"
{
#define TAILLEBUF 4096
  graphe * g;
  int i, n, m, t, q, rs = -1, cs = -1;
  char buf[TAILLEBUF];
  char *ret;

  FILE *fd = NULL;

  fd = fopen(filename,"rb");
  if (!fd)
  {
    fprintf(stderr, "%s: file not found: %s\n", filename, F_NAME);
    return NULL;
  }

  ret = fgets(buf, TAILLEBUF, fd);
  if( sscanf(buf,"#rs %d cs %d\n", &rs, &cs) == 2)
    ret = fgets(buf, TAILLEBUF, fd);

  sscanf(buf, "%d %d\n", &n, &m);
  
  g = initgraphe(n, 2*m);
  setSize(g,rs,cs);

  double *F = (double *)calloc(1,n * sizeof(double));

  do
  {
    ret = fgets(buf, TAILLEBUF, fd);
    
    if ((ret != NULL) && (strncmp(buf, "val sommets", 11) == 0))
    {
      double v;
      for (i = 0; i < n; i++)  
      {
        fscanf(fd, "%d %lf\n", &t, &v);
        F[i] = v; 
      }
    } /*  if ((ret != NULL) && (strncmp(buf, "val sommets", 11) == 0)) */
    
    else if ((ret != NULL) && (strncmp(buf, "arcs values", 11) == 0))
    {
      double e;
      for (i = 0; i < m; i++)  
      {
        fscanf(fd, "%d %d %lf\n", &t, &q, &e);
        addarete(g , t, q, e);
      }
    } /*  if ((ret != NULL) && (strncmp(buf, "arcs values", 11) == 0)) */
    
  } while (ret != NULL);
	
//for(i=0;i<20;i++)printf("BAD FILTER MODE %d %g\n",i,F[i]);	
	
  (*Fv)=F;

  fclose(fd);
  return g;
} /* ReadGraphe() */

/* ====================================================================== */
/*! \fn void SaveGraphe(graphe * g, char *filename) 
    \param g (entr�) : un graphe.
    \param filename (entr�) : nom du fichier �g��er.
    \brief sauve le graphe g dans le fichier filename. 
*/
void SaveGraphe(graphe * g, char *filename, double *Fv ) 
/* ====================================================================== */
#undef F_NAME
#define F_NAME "SaveGraphe"
{
  int i, j, n = g->nbsom, m = g->ind,a,b;
  pcell p;
  FILE * fd = NULL;
  double v;

  fd = fopen(filename,"wb");
  if (!fd)
  {
    fprintf(stderr, "%s: cannot open file: %s\n", F_NAME, filename);
    return;
  }
  
  if( (g->cs != -1) && (g->rs != -1))
    fprintf(fd,"#rs %d cs %d\n",g->rs, g->cs);

  fprintf(fd, "%d %d\n", n, m);

  
  fprintf(fd, "val sommets\n");
  for (i = 0; i < n; i++) 
    fprintf(fd, "%d %f\n", i, Fv[i]);
  
  fprintf(fd, "arcs values\n");
  for (i = 0; i < m; i++) 
   {
     a = g->tete[i];
     b = g->queue[i];
     v = getweight (g,i);
            
     fprintf(fd, "%d %d %f\n", a, b, v);
   }
  
  fclose(fd);
} /* SaveGraphe() */

/*uint32_t voisin(graphe *g, uint32_t x, uint32_t u)
  // returns the vertex of g linked to x by edge u 
{
  if(g->tete[u] != x)
    return g->tete[u];
  else return g->queue[u];
}
*/
