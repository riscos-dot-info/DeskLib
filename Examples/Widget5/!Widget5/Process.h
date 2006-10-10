/************************************************************************
 *                                                                      *
 *        File: Process.h                                               *
 *                                                                      *
 *     Purpose: Performs actual image processing functions              *
 *                                                                      *
 ************************************************************************/

/*
 * copies 'sourceimage' to 'destimage'
 */
void Process_Copy(int sourceimage, int destimage, wimp_point *size);


 /*
  * adds 'sourceimage' to 'destimage'
  */
void Process_Add(int sourceimage, int destimage, wimp_point *size);


 /*
  * adds 'sourceimage' to 'destimage' and divides by 2
  */
void Process_Average(int sourceimage, int destimage, wimp_point *size);


/*
 * convolves 'sourceimage' to 'destimage' using 'filter'
 * if overwrite is FALSE then the destination image is added to rather than overwritten
 */
void Process_Convolve(int sourceimage, int destimage, wimp_point *size,
                      wimp_point *anchor, int (*filter)[3][3], BOOL overwrite);

 /*
  * 3x3 median filters 'sourceimage' to 'destimage'
  */
void Process_Median(int sourceimage, int destimage, wimp_point *size);

 /*
  * calculates grey level histogram for 'sourceimage' and uses it to do calculate
  * the greymap for histogram equalization
  */
void Process_HistogramEq(int sourceimage, wimp_point *size, char (*greymap)[]);

 /*
  * calculates grey level histogram for 'sourceimage' and places it in the array
  * 'greyhist'
  */
void Process_GreyHistorgram(int sourceimage, wimp_point *size, int (*greyhist)[256]);

 /*
  * takes grey level histogram for 'greyhist' and calculates the cumulative
  * histogram and places it in 'cuhist'
  * NOTE: 'greyhist' and 'cuhist' can be the same array
  */
void Process_CumulativeHistorgram(int (*greyhist)[256], int (*cuhist)[256]);

 /*
  * scales 'sourceimage' to size of 'destimage' using biliner interpolation and
  * resampling
  */
void Process_Scale(int sourceimage, int destimage,
                   wimp_point *sourcesize, wimp_point *destsize);

 /*
  * mixes source1 and source2 according to the percentage in image1percent and
  * puts the result in destimage.
  * NOTE: destimage can be the same as source2
  */
void Process_Mix(int source1, int source2, int destimage, int image1percent,
                 wimp_point *size);

 /*
  * map the greylevels in the 'sourceimage' to the new values set out using the bargraph
  * from the window in 'barmap' and put the new image in 'destimage'
  */
void Process_GreyMap(int sourceimage, int destimage, wimp_point *size, char (*barmap)[]);

 /*
  * calculates grey level histogram for 'sourceimage' Saturating Linear Contrast 
  * Enhancement
  */
void Process_SLCE(int sourceimage, wimp_point *size, char (*greymap)[]);



