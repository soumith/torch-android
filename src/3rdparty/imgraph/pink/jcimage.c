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
#include <string.h>
#include <ctype.h>
#include <mcutil.h>
#include <mccodimage.h>
#include <jccodimage.h>
#include <mcimage.h>
#include <jcimage.h>


// ! jcimage remplace mcimage!
// Les deux deux fichiers ne peuvent pas etre utilisé en meme temps car
// l'intersection est non vide

#define BUFFERSIZE 10000

/* ==================================== */
struct xvimage *allocGAimage(
  char * name,
  int32_t rs,   /* row size */
  int32_t cs,   /* col size */
  int32_t d,    /* depth */
  int32_t t)    /* data type */
/* ==================================== */
#undef F_NAME
#define F_NAME "allocimage"
{
  int32_t N = rs * cs * d;             /* taille image */
  struct xvimage *g;
  int32_t ts;                          /* type size */
  switch (t)
  {
  case VFF_TYP_GABYTE: if(d == 1) ts = 2; else ts = 3; break;      /* cas d'une image d'arete en 2D, chaque pixel a 2 aretes */
  case VFF_TYP_GAFLOAT: if(d == 1) ts = 2*sizeof(float); else ts = 3*sizeof(float); break;
  case VFF_TYP_GADOUBLE: if(d == 1) ts = 2*sizeof(double); else ts = 3*sizeof(float); break;
  default: fprintf(stderr,"%s: bad data type, ne gère que les GAs %d\n", F_NAME, t);
    return NULL;
  } /* switch (t) */

  g = (struct xvimage *)malloc(sizeof(struct xvimage));
  if (g == NULL)
  {   fprintf(stderr,"%s: malloc failed (%ld bytes)\n", F_NAME, sizeof(struct xvimage));
      return NULL;
  }
  g->image_data = malloc((N*ts)); //-1 ?
  if (g->image_data == NULL) {
    fprintf(stderr,"%s: malloc failed (%d bytes)\n", F_NAME, ((N*ts-1)));
    return NULL;
  }


  if (name != NULL)
  {
    g->name = (char *)malloc(strlen(name)+1);
    if (g->name == NULL)
    {   fprintf(stderr,"%s: malloc failed for name\n", F_NAME);
        return NULL;
    }
    strcpy((char *)(g->name), name);
  }
  else
    g->name = NULL;

  rowsize(g) = rs;
  colsize(g) = cs;
  depth(g) = d;
  datatype(g) = t;
  g->xdim = g->ydim = g->zdim = 0.0;

    
  return g;
} /* allocGAimage() */
 

/* ==================================== */
void writerawGAimage(struct xvimage * image, char *filename)
/* ==================================== */
#undef F_NAME
#define F_NAME "writerawGAimage"
{
  FILE *fd = NULL;
  int32_t rs, cs, d, N, ret;
  int32_t ts;

  rs = rowsize(image);
  cs = colsize(image);
  d = depth(image);
  N = rs * cs * d;

#ifdef UNIXIO
  fd = fopen(filename,"w");
#endif
#ifdef DOSIO
  fd = fopen(filename,"wb");
#endif
  if (!fd)
  {
    fprintf(stderr, "%s: cannot open file: %s\n", F_NAME, filename);
    exit(0);
  }
  switch(datatype(image)){
  case VFF_TYP_GABYTE:
    if(d == 1) ts = 2; else ts =3;
    if (d > 1) fputs("PC\n", fd); else fputs("PC\n", fd);
    if ((image->xdim != 0.0) && (d > 1))
      fprintf(fd, "#xdim %g\n#ydim %g\n#zdim %g\n", image->xdim, image->ydim, image->zdim);
    if ((image->xdim != 0.0) && (d == 1))
      fprintf(fd, "#xdim %g\n#ydim %g\n", image->xdim, image->ydim);
    if (d > 1) fprintf(fd, "%d %d %d\n", rs, cs, d); else  fprintf(fd, "%d %d\n", rs, cs);
    fprintf(fd, "255\n");
    
    ret = fwrite(UCHARDATA(image), sizeof(char), ts*N, fd);
    if (ret != ts*N)
    {
      fprintf(stderr, "%s: only %d items written\n", F_NAME, ret);
      exit(0);
    }
    break;
  case VFF_TYP_GAFLOAT:
    if(d == 1) ts = 2*sizeof(float); else ts = 3*sizeof(float);
    if (d > 1) fputs("PD\n", fd); else fputs("PD\n", fd);
    if ((image->xdim != 0.0) && (d > 1))
      fprintf(fd, "#xdim %g\n#ydim %g\n#zdim %g\n", image->xdim, image->ydim, image->zdim);
    if ((image->xdim != 0.0) && (d == 1))
      fprintf(fd, "#xdim %g\n#ydim %g\n", image->xdim, image->ydim);
    if (d > 1) fprintf(fd, "%d %d %d\n", rs, cs, d); else  fprintf(fd, "%d %d\n", rs, cs);
    fprintf(fd, "65535\n");
    ret = fwrite(UCHARDATA(image), sizeof(char), ts*N, fd);
    if (ret != ts*N)
    {
      fprintf(stderr, "%s: only %d items written\n", F_NAME, ret);
      exit(0);
    }
    break;
  default:
    fprintf(stderr,"%s: bad datatype, ne traite que les GAs : %d\n", F_NAME, datatype(image));
    exit(0);
  }//switch

  fclose(fd);
} /* writerawGAimage() */

/* ==================================== */
struct xvimage * readGAimage(char *filename)
/* ==================================== */
#undef F_NAME
#define F_NAME "readGAimage"
{
  char buffer[BUFFERSIZE];
  FILE *fd = NULL;
  int32_t rs, cs, d, ndgmax, N;
  struct xvimage * image;
  int32_t ascii;  
  int32_t typepixel;
  int32_t c;
  double xdim=1.0, ydim=1.0, zdim=1.0;
  int32_t ts;

#ifdef UNIXIO
  fd = fopen(filename,"r");
#endif
#ifdef DOSIO
  fd = fopen(filename,"rb");
#endif
  if (!fd)
  {
    fprintf(stderr, "%s: file not found: %s\n", F_NAME, filename);
    return NULL;
  }

  fgets(buffer, BUFFERSIZE, fd); /* P5: raw byte bw  ; P2: ascii bw */
                                 /* P6: raw byte rgb ; P3: ascii rgb */
                                 /* P7: raw byte 3d  ; P4: ascii 3d  ==  extensions MC */
                                 /* P8: raw int32_t 2d-3d  ==  extension MC */
                                 /* P9: raw float 2d-3d  ==  extension MC */
                                 /* PA: ascii float 2d-3d  ==  extension LN */
                                 /* PB: ascii int32_t 2d-3d  ==  extension MC */
                                 /* PC: graphe d'arete == extension JC */
                                 /* PD: graphe d'arete flottant == extension JC */ 

  if (buffer[0] != 'P')
  {   fprintf(stderr,"%s : invalid image format\n", F_NAME);
      return NULL;
  }
  switch (buffer[1])
  {
  case 'C': ascii = 0; typepixel = VFF_TYP_GABYTE; break;
  case 'D': ascii = 0; typepixel = VFF_TYP_GAFLOAT; break;
  default: 
    fprintf(stderr,"%s : invalid image format, ne traite que les GAs\n", F_NAME);
      return NULL;
  } /* switch */


  do 
  {
    fgets(buffer, BUFFERSIZE, fd); /* commentaire */
    if (strncmp(buffer, "#xdim", 5) == 0)
      sscanf(buffer+5, "%lf", &xdim);
    else if (strncmp(buffer, "#ydim", 5) == 0)
      sscanf(buffer+5, "%lf", &ydim);
    else if (strncmp(buffer, "#zdim", 5) == 0)
      sscanf(buffer+5, "%lf", &zdim);
  } while (!isdigit(buffer[0]));

  c = sscanf(buffer, "%d %d %d", &rs, &cs, &d);
  if (c == 2) d = 1;
  else if (c != 3)
  {   fprintf(stderr,"%s : invalid image format\n", F_NAME);
      return NULL;
  }

  fgets(buffer, BUFFERSIZE, fd);

  sscanf(buffer, "%d", &ndgmax);
  N = rs * cs * d;
  image = allocGAimage(NULL, rs, cs, d, typepixel);
  if (image == NULL)
  {   fprintf(stderr,"%s : alloc failed\n", F_NAME);
      return(NULL);
  }

  image->xdim = xdim;
  image->ydim = ydim;
  image->zdim = zdim;
  if (typepixel == VFF_TYP_GABYTE)
  {
    if(d == 1) ts=2; else ts = 3;
    int32_t ret = fread(UCHARDATA(image), sizeof(char), ts*N, fd);
    if (ret != ts*N)
    {
      fprintf(stderr,"%s : fread failed : %d asked ; %d read\n", F_NAME, N, ret);
      return(NULL);
    }
    
  } /* if (typepixel == VFF_TYP_GABYTE) */

  if (typepixel == VFF_TYP_GAFLOAT)
  {
    if(d == 1) ts=2; else ts = 3;
    int32_t ret = fread(UCHARDATA(image), sizeof(float), ts*N, fd);
    if (ret != ts*N)
    {
      fprintf(stderr,"%s : fread failed : %d asked ; %d read\n", F_NAME, N, ret);
      return(NULL);
    }
    
  } /* if (typepixel == VFF_TYP_GAFLOAT) */
  
  fclose(fd);
  return image;
} /* readGAimage() */
 
/* ==================================== */
struct xvimage4D *allocimage4D(
    int32_t ss)    /* sequence size */
/* ==================================== */
#undef F_NAME
#define F_NAME "allocimage4D"
{
  struct xvimage4D *g;
  if (ss < 1)
  {
    fprintf(stderr, "%s: bad sequence size\n", F_NAME);
    return(NULL);
  }
  g = (struct xvimage4D*)malloc(sizeof(struct xvimage4D));
  if(g == NULL)
  {
    fprintf(stderr, "%s: Erreur de malloc \n", F_NAME);
    return(NULL);
  }
  g->ss = ss;
  g->frame = (struct xvimage**)malloc(sizeof(struct xvimage*) * ss);
  if(g->frame == NULL)
  {
    fprintf(stderr, "%s: Erreur de malloc \n", F_NAME);
    return(NULL);
  }
  return g;
} /* allocimage4D()  */


/* ==================================== */
void freeimage4D(struct xvimage4D * im)     /* derniere frame */
/* ==================================== */
{
  int32_t i;
  for(i = 0; i < im->ss; i++)
    freeimage(im->frame[i]);
  free(im);
}

/* ==================================== */
struct xvimage4D *readimage4D(char *prefix,   /* prefixe des noms d'images */
			      int32_t first,    /* premiere frame */
			      int32_t last)     /* derniere frame */
/* ==================================== */
#undef F_NAME
#define F_NAME "readimage4D"
{
  struct xvimage4D * image;
  int32_t seqsize, prefixlen, cs, rs, ds,j;
  char bufname[1024];
  
  if( first > last)
  {
    fprintf(stderr, "%s: première frame supérieure à dernière frame\n", F_NAME);
    return(NULL);
  }
  seqsize = last - first + 1;
  printf("seqsize %d \n",seqsize);
  
  image = allocimage4D(seqsize);
  if(image == NULL) 
  {
    fprintf(stderr, "%s: Erreur allocation\n", F_NAME);
    return(NULL);
  }
  strcpy(bufname, prefix);
  prefixlen = strlen(prefix);  
  bufname[prefixlen] =   '0' + (first / 1000) % 10;
  bufname[prefixlen+1] = '0' + (first / 100) % 10;
  bufname[prefixlen+2] = '0' + (first / 10) % 10;
  bufname[prefixlen+3] = '0' + (first / 1) % 10;
  bufname[prefixlen+4] = '.';
  bufname[prefixlen+5] = 'p';
  bufname[prefixlen+6] = 'g';
  bufname[prefixlen+7] = 'm';
  bufname[prefixlen+8] = '\0';
  printf("Valeur du fichier a lire : %s \n",bufname);
  image->frame[0] = readimage(bufname);
  if( image->frame[0] == NULL )
  {
    fprintf(stderr, "%s: readimage failed: %s\n", F_NAME , bufname);
    freeimage4D(image);
    return(NULL);
  }
  printf("toto4 \n");
  rs = rowsize(image->frame[0]);     /* taille ligne */
  cs = colsize(image->frame[0]);     /* taille colonne */
  ds = depth(image->frame[0]);   /* taille profondeur */
  printf("volume de type : %d %d %d \n", rs, cs,ds);
  strcpy(bufname, prefix);
  prefixlen = strlen(prefix);
  for (j = 1; j < seqsize; j++)
  {  
    bufname[prefixlen] =   '0' + ((first+j) / 1000) % 10;
    bufname[prefixlen+1] = '0' + ((first+j) / 100) % 10; 
    bufname[prefixlen+2] = '0' + ((first+j) / 10) % 10; 
    bufname[prefixlen+3] = '0' + ((first+j) / 1) % 10; 
    bufname[prefixlen+4] = '.';
    bufname[prefixlen+5] = 'p';
    bufname[prefixlen+6] = 'g';
    bufname[prefixlen+7] = 'm';
    bufname[prefixlen+8] = '\0';
    image->frame[j] =  readimage(bufname);
    if( (cs != colsize(image->frame[j])) || (rs != rowsize(image->frame[j])) || (ds != depth(image->frame[j])) )
    {
      fprintf(stderr, "%s: %s bad image size\n", F_NAME, bufname);
      freeimage4D(image);
      return(NULL);
    }
  }
  return image;
} /* readimage4D()  */


/* ==================================== */
void writeimage4D(struct xvimage4D * image, char *prefix, int32_t first, int32_t last)
/* ==================================== */
{
  char bufname[1024];
  int32_t prefixlen,j;

  strcpy(bufname, prefix);
  prefixlen = strlen(prefix);
  for(j = 0; j < image->ss; j++)
  {
    bufname[prefixlen] =   '0' + ((first+j) / 1000) % 10;
    bufname[prefixlen+1] = '0' + ((first+j) / 100) % 10;
    bufname[prefixlen+2] = '0' + ((first+j) / 10) % 10;
    bufname[prefixlen+3] = '0' + ((first+j) / 1) % 10;
    bufname[prefixlen+4] = '.';
    bufname[prefixlen+5] = 'p';
    bufname[prefixlen+6] = 'g';
    bufname[prefixlen+7] = 'm';
    bufname[prefixlen+8] = '\0';  
    writeimage(image->frame[j], bufname);
  }
} /* writeimage4D */

/* ==================================== */
struct GA4d * readGA4d(char *filename)
/* ==================================== */
#undef F_NAME
#define F_NAME "readGA4d"
{
  char buffer[BUFFERSIZE];
  FILE *fd = NULL;
  int32_t rs, cs, d, ndgmax, N, ss;
  struct GA4d * image;
  double xdim=1.0, ydim=1.0, zdim=1.0, tdim=1.0;
  int32_t ret,c;
  
#ifdef UNIXIO
  fd = fopen(filename,"r");
#endif
#ifdef DOSIO
  fd = fopen(filename,"rb");
#endif
  if (!fd)
  {
    fprintf(stderr, "%s: file not found: %s\n", F_NAME, filename);
    return NULL;
  }

  fgets(buffer, BUFFERSIZE, fd); /* P5: raw byte bw  ; P2: ascii bw */
                                 /* P6: raw byte rgb ; P3: ascii rgb */
                                 /* P7: raw byte 3d  ; P4: ascii 3d  ==  extensions MC */
                                 /* P8: raw int32_t 2d-3d  ==  extension MC               */
                                 /* P9: raw float 2d-3d  ==  extension MC              */
                                 /* PA: ascii float 2d-3d  ==  extension LN            */
                                 /* PB: ascii int32_t 2d-3d  ==  extension MC             */
                                 /* PC: graphe d'arete == extension JC                 */
                                 /* PD: graphe d'arete 4d == extension JC              */
  if (buffer[0] != 'P')
  {   fprintf(stderr,"%s : invalid image format\n", F_NAME);
      return NULL;
  }
  if (buffer[1] != 'D')
  {
    fprintf(stderr,"%s: not 4d weighted-edge graph", F_NAME);
    return NULL;
  } /* if */

  do 
  {
    fgets(buffer, BUFFERSIZE, fd); /* commentaire */
    if (strncmp(buffer, "#xdim", 5) == 0)
      sscanf(buffer+5, "%lf", &xdim);
    else if (strncmp(buffer, "#ydim", 5) == 0)
      sscanf(buffer+5, "%lf", &ydim);
    else if (strncmp(buffer, "#zdim", 5) == 0)
      sscanf(buffer+5, "%lf", &zdim);
  } while (!isdigit(buffer[0]));

  c = sscanf(buffer, "%d %d %d %d", &rs, &cs, &d, &ss);
  switch(c != 4){
    fprintf(stderr,"%s : invalid image format\n", F_NAME);
    return NULL; 
  }

  fgets(buffer, BUFFERSIZE, fd);

  sscanf(buffer, "%d", &ndgmax);
  N = rs * cs * d * ss;

  image = allocGA4d(NULL, rs, cs, d, ss); 
  if (image == NULL){   
    fprintf(stderr,"%s : alloc failed\n", F_NAME);
    return(NULL);
  }
  image->xdim = xdim;
  image->ydim = ydim;
  image->zdim = zdim;
  image->tdim = tdim;
  ret = fread(UCHARDATA(image), sizeof(char), 4*N, fd);
  if (ret != 4*N)
  {
    fprintf(stderr,"%s : fread failed : %d asked ; %d read\n", F_NAME, N, ret);
    return(NULL);
  }
  fclose(fd);
  return image;
} /* readGA4d() */

/* ==================================== */
struct GA4d *allocGA4d(
			   char * name,
			   int32_t rs,   /* row size */
			   int32_t cs,   /* col size */
			   int32_t d,    /* depth */
			   int32_t ss)   /* sequence size */
/* ==================================== */
#undef F_NAME
#define F_NAME "allocGA4d"
{
  int32_t N = rs * cs * d * ss;             /* taille image */
  struct GA4d *g;
  g = (struct GA4d *)malloc(sizeof(struct GA4d) - 1 + (N * 4));
  if (g == NULL)
  {   fprintf(stderr,"%s: malloc failed (%ld bytes)\n", F_NAME, sizeof(struct xvimage) - 1 + (N * 4));
      return NULL;
  }
  if (name != NULL)
  {
    g->name = (char *)malloc(strlen(name)+1);
    if (g->name == NULL)
    {   fprintf(stderr,"%s: malloc failed for name\n", F_NAME);
        return NULL;
    }
    strcpy((char *)(g->name), name);
  }
  else
    g->name = NULL;

  rowsize(g) = rs;
  colsize(g) = cs;
  depth(g) = d;
  seqsizeGA(g) = ss; 

  g->xdim = g->ydim = g->zdim = g->tdim = 0.0; 
  return g;
} /* allocimage() */

/* ==================================== */
void writeGA4d(struct GA4d * image, char *filename)
/* ==================================== */
#undef F_NAME
#define F_NAME "writeGA4d"
{
  FILE *fd = NULL;
  int32_t rs, cs, d, N, ret, ss;

  rs = rowsize(image);
  cs = colsize(image);
  d = depth(image);
  ss = seqsizeGA(image);
  N = rs * cs * d * ss;

#ifdef UNIXIO
  fd = fopen(filename,"w");
#endif
#ifdef DOSIO
  fd = fopen(filename,"wb");
#endif
  if (!fd)
  {
    fprintf(stderr, "%s: cannot open file: %s\n", F_NAME, filename);
    exit(0);
  }

  fputs("PD\n", fd);
  fprintf(fd, "#xdim %g\n#ydim %g\n#zdim %g\n#tdim %g\n", image->xdim, image->ydim, image->zdim, image->tdim);
  fprintf(fd, "%d %d %d %d\n", rs, cs, d, ss);
  fprintf(fd, "255\n");
  ret = fwrite(UCHARDATA(image), sizeof(char), 4*N, fd);
  if (ret != 4*N)
  {
    fprintf(stderr, "%s: only %d items written\n", F_NAME, ret);
    exit(0);
  }
  fclose(fd);
} /* writeGA4d() */

void freeGA4d(struct GA4d  *im)
{
  free(im);
}
