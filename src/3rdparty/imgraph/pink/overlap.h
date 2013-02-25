float * Overlap(struct xvimage * segment, struct xvimage *mask,  int nb_classes);
float * Overlap1(struct xvimage *segment, struct xvimage *mask, int nb_classes);
int * DecisionSegmentation(float * Segments, int rs, int cs, int nb_segments, float *f, int nb_classes, float t1, float t2, float t3);
