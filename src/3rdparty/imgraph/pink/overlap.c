/*
  Propagation algorithm for computating an overlap measure from 2 images
  author: Camille Couprie
  1 aug. 2012
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <mccodimage.h>
#include <mcimage.h>
#include <mclifo.h>
#include <mcindic.h>
#include <mcutil.h>

#define false 0
#define true 1


/*=====================================================================================*/
float * Overlap(struct xvimage * Isegment, struct xvimage * Imask,  int nb_classes)
/*=====================================================================================*/
// Computes the overlap score between a segment and the ground truth. 
// Overlap scores are given for each classes and stacked in a tensor.
// inputs : * segment: binary image: foreground backround segmentation
//          * mask: ground truth image: integers between 1 and 22
{
  int i, j, k, p, q;
  
 // get pointer to Isegment and Imask
  uint8_t *segment = UCHARDATA(Isegment);
  uint8_t *mask = UCHARDATA(Imask);
 
  int rs = rowsize(Isegment);
  int cs = colsize(Isegment);
  int N = rs*cs; 

  Lifo * LIFO;
  LIFO = CreeLifoVide(N);
  if (LIFO == NULL) { fprintf(stderr, "Overlap : CreeLifoVide failed\n"); exit(0); }

  float * overlaping_class = (float*)calloc(nb_classes ,sizeof(float));
  float * union_class = (float*)calloc(nb_classes ,sizeof(float));

  uint8_t * Mrk = (uint8_t*)calloc(N ,sizeof(uint8_t));
   if (Mrk == NULL) { fprintf(stderr, "Overlap : malloc failed\n"); exit(0); }


  // first pass to count the nb of different ground truth class intersecting with the segment
   int cpt_pixels_in_segment=0;
  for (i=0;i<N;i++) 
      if (segment[i]==255)
	{
	  cpt_pixels_in_segment ++;
	  overlaping_class[mask[i]-1]++; // useful for inter
	}

  // second pass to compute the union for each present class
  for (j=0;j<nb_classes;j++) 
    {
    if (overlaping_class[j]>0.5) //for each class present in the segment 
      {
	for (i=0;i<N;i++) // for each pixel
	  {
	    if ((segment[i]==255)&&(mask[i]==j+1))
	      {
		LifoPush(LIFO, i);
		Mrk[i] = true;
	      }
	  }
	union_class[j] = cpt_pixels_in_segment;
	while (!LifoVide(LIFO))
	  {
	    p = LifoPop(LIFO);
	    Mrk[p]=true;
	    
	    for (k = 0; k < 8; k += 2)
	      {
		q = voisin(p, k, rs, N);
		if (q!=-1)
		  if ((Mrk[q]==false)&&(mask[q]==mask[p]))
		    {
		      union_class[j]++;
		      LifoPush(LIFO, q);
		      Mrk[q]=true;
		    }
	      }
	  }
	//last pass: overlap = intersection/union
	overlaping_class[j]=overlaping_class[j]/union_class[j];
      }
    }

 
  LifoTermine(LIFO);
  free(Mrk);
  free(union_class);
  
  return overlaping_class;
}


// Compute overlap percentage of mask within segment per class
float * Overlap1(struct xvimage *Isegment, struct xvimage *Imask, int nb_classes) {
  int i,c;

  // get pointer to Isegment and Imask
  uint8_t *segment = UCHARDATA(Isegment);
  uint8_t *mask = UCHARDATA(Imask);
 
  int rs = rowsize(Isegment);
  int cs = colsize(Isegment);
  int N = rs*cs; 

  float *overlaping_class = (float*)calloc(nb_classes ,sizeof(float));
  if (overlaping_class == NULL) { fprintf(stderr, "Overlap : malloc failed\n"); exit(0); }

  // count the nb of different ground truth class intersecting with the segment
  int cpt_pixels_in_segment=0;
  for (i=0; i<N; ++i) 
    if (segment[i]==255) {
      ++cpt_pixels_in_segment;
      ++overlaping_class[mask[i]-1]; // useful for inter
    }

  for (c=0; c<nb_classes; ++c) {
    overlaping_class[c] /= cpt_pixels_in_segment;
  }

  return overlaping_class;
}
