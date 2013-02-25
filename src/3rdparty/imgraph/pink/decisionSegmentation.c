/*
  author: Camille Couprie
  13 aug. 2012
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

typedef struct max_elmt
{ float val;
  int index;
} max_elmt;

void maximum(float * f, int index_segment, int nb_classes, max_elmt * M)
{
  int i; int index = 0;
  M->val = 0;
  for (i=0;i<nb_classes;i++)
   if (f[index_segment*nb_classes+i] > M->val )
      {
	//	fprintf(stderr,"hello=%f %d\n",f[index_segment*nb_classes+i],i);
	M->val = f[index_segment*nb_classes+i];
	M->index = i;
      }
}

void maximumg(float * g, int index_pixel, int nb_classes, int N, max_elmt * M)
{
  int i; int index = 0; //g[k*N+p]
  M->val = 0;
  for (i=0;i<nb_classes;i++)
   if (g[index_pixel + N*i] > M->val )
      {
	M->val = g[index_pixel + N*i];
	M->index = i;
      }
}


float Intersect(float *S, int i1, int i2, int N)
//returns intersection score between 2 segments
{
  int i;
  float inter=0; float interS1 = 0;float interS2 = 0;
  for (i=0;i<N;i++) 
    {
    if ((S[i1*N+i]==1)&&(S[i2*N+i]==1))
      inter ++;
    if (S[i1*N+i]==1)
      interS1 ++;
    if (S[i2*N+i]==1)
      interS2 ++;
    }
  if(interS1<interS2)
    return inter/interS1;
  else 
    return inter/interS2;
}



/*=====================================================================================*/
int * DecisionSegmentation(float * Segments, int rs, int cs, int nb_segments, float *f, int nb_classes, float t1, float t2, float t3)
/*=====================================================================================*/
// Computes the final segmentation from an array of segments 
// with associated overlap scores for different classes of objects
// inputs : * Segments: Array of binary images: foreground-backround segmentations ( nb_segments x rs x cs)
//          * f: Array of overlap scores (nb_classes x nb_segments)
// output   * mask: ground truth image: integers between 1 and 22
{
  // 1) init
 
  int i, k, p;
  int N = rs*cs; 
  int * I = calloc (N, sizeof(int));
  
  max_elmt *M =malloc(sizeof(struct max_elmt));
  float* max_scores= malloc(sizeof(float)*nb_segments);
  int * index_max_scores= malloc(sizeof(int)*nb_segments);
  uint8_t * L= calloc(nb_segments, sizeof(uint8_t));
  float* g= calloc(nb_classes*N, sizeof(float));
  int * index_segments= malloc(sizeof(int)*nb_segments);
  for (i=0;i<nb_segments;i++) index_segments[i]=i ;
  
   
  // 2) Sort the segments descending by maximal score on all classes
    for (i=0;i<nb_segments;i++)
    {
      maximum(f, i, nb_classes, M);
      index_max_scores[i]  = M->index;
      max_scores[i]  = M->val;
    }
    //for (i=0;i<nb_segments;i++)
    //fprintf(stderr,"segment nb %d has a score %f associated with class %d \n ",i,  max_scores[i], index_max_scores[i]);
    
     TriRapideStochastique_dec(max_scores,index_segments, 0, nb_segments-1);

  int top = 0;
  int n=1;
  // 3) While S is not empty
  while(top < nb_segments)
    {
     
  // 4) Select the segment with the highest maximal score
      // S = Segments[index_max_scores[top]];

  // 5) Find all segments that have at least t1 intersection with S, 
  //    let them be indicated as true in L, still sorted by maximum scores
      for (i=1;i<nb_segments;i++)
	{ 
	  if (Intersect(Segments, index_segments[top], index_segments[i], N)>t1)
	  L[index_segments[i]]= true; 
	}
  //6) For each pixel p in the image, compute pixel score g for each class k as
      // gk(p) = sum_{Si in L} (w_i 1(p in Si) fk(Si)) 
      for (p=0;p<N;p++) 
	{
	for (i=0;i<nb_segments;i++)
	  {
	    if (L[i]==true)
	      {
		for (k=0;k<nb_classes;k++)
		  g[k*N+p] = g[k*N+p]+ (Segments[index_segments[i]*N+p]==1)*f[index_segments[i]*nb_classes+k];
	      }
	  }
	}

   //7) For each pixel p
      for (p=0;p<N;p++) 
	{
	  maximumg(g, p, nb_classes,N, M);
   // 8) if max gk < t2 9) Classify p as background
	  if(M->val<t2)
	    I[p]=2;
   // 10) else 11) Classify p as class k
	  else 
	    I[p]=M->index+3;
	   
	    
   // 12) end if 13) end for 
	}

   //14) if max_kj gk(pj) > t3

    //15) The score of the mask is given by the highest pixel score in the mask. 
    //It must exceed  threshold to be retained in the final semantic segmentation
      break;
    //16) end if

    //17) delete all segments L from S 
      for (i=1;i<nb_segments;i++)
	L[i]=false;
      //TODO top = ?

    //18) n=n+1
      n=n+1;
    }//19) end while

  free(L);
  free(max_scores);
  free(index_max_scores);
  free(g); 

  return I;
}



