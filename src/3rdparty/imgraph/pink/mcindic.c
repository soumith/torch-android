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
/* gestion d'indicateurs binaires (jusqu'a 8) */
/* les indicateurs sont numerotes de 0 a 7 */
/* M. Couprie juillet 1996 */

/* gestion d'un indicateur binaire compact */
/* M. Couprie novembre 1999 */

/*
#define TESTINDIC
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <mcindic.h>

Indicstype *Indics = NULL;       /* en global pour etre efficace */

/* ==================================== */
void IndicsInit(index_t Size)
/* ==================================== */
{
  Indics = (Indicstype *)calloc(Size, sizeof(Indicstype));
  if (Indics == NULL)
  {
    fprintf(stderr, "erreur allocation Indics\n");
    exit(1);
  }
}

/* ==================================== */
void Indics1bitInit(index_t Size)
/* ==================================== */
{
  Indics = (Indicstype *)calloc((Size-1)/8 + 1, sizeof(Indicstype));
  if (Indics == NULL)
  {
    fprintf(stderr, "erreur allocation Indics\n");
    exit(1);
  }
}

/* ==================================== */
void Indics1bitReInit(index_t Size)
/* ==================================== */
{
  memset(Indics, 0, ((Size-1)/8 + 1) * sizeof(Indicstype));
}

/* ==================================== */
void IndicsTermine()
/* ==================================== */
{
  free(Indics);
}

#ifdef TESTINDIC
void main()
{
  IndicsInit(3);
  Set(0, 0);   if (IsSet(0, 0)) printf("test1 ok\n");
printf("->%d\n", Indics[0]);
  Set(0, 1);   if (IsSet(0, 1)) printf("test2 ok\n");
printf("->%d\n", Indics[0]);
  UnSet(0, 1); if (!IsSet(0, 1)) printf("test3 ok\n");
printf("->%d\n", Indics[0]);
               if (IsSet(0, 0)) printf("test4 ok\n");
  UnSetAll(0); if (!IsSet(0, 0)) printf("test5 ok\n");
printf("->%d\n", Indics[0]);

  IndicsTermine();
}
#endif
