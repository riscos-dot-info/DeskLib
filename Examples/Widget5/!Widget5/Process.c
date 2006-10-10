/************************************************************************
 *                                                                      *
 *        File: Process.c                                               *
 *                                                                      *
 *     Purpose: Performs actual image processing functions              *
 *                                                                      *
 ************************************************************************/

#include "DeskLib:WimpSWIs.h"          /* Low-level WIMP commands         */
#include "DeskLib:Window.h"            /* Window handling automation      */
#include "DeskLib:Core.h"              /* usefull core functions          */
#include "DeskLib:Error.h"             /* Error despatcher                */
#include "DeskLib:Event.h"             /* Event despatcher                */
#include "DeskLib:EventMsg.h"          /* Wimp Message event dispatcher   */
#include "DeskLib:File.h"              /* Low level file handling         */
#include "DeskLib:Msgs.h"              /* Message translation code        */
#include "DeskLib:Resource.h"          /* Handles finding resource files  */
#include "DeskLib:Sound.h"             /* Beep!                           */
#include "DeskLib:Icon.h"              /* Icon handling automation        */
#include "DeskLib:Screen.h"            /* Caching screen info             */
#include "DeskLib:Sprite.h"            /* Sprite handling routines        */
#include "DeskLib:File.h"              /* OS file IO                      */
#include "DeskLib:KeyCodes.h"          /* Codes for wimp returned keys    */
#include "DeskLib:Hourglass.h"         /* control hourglass               */

#include "SpriteWin.h"
#include "Filter.h"
#include "ProcessWin.h"

#include <stdio.h>
#include <math.h>

void clip_pixel(int *pixel);
void clip_coord(wimp_point *coord, wimp_point *imagesize);
void bubblesort_subloop(int *outerloop, int *innerloop, int *pixel, int (*median)[10]);

#define getsource(coord) (sourceimagedata[coord.x + (coord.y * sourcelinelength)])
#define getsource2(coord) (source2imagedata[coord.x + (coord.y * source2linelength)])
#define putdest(coord,value) (destimagedata[coord.x + (coord.y * destlinelength)] = value)
#define getdest(coord) (destimagedata[coord.x + (coord.y * destlinelength)])

/******************************************************************************
 *                            extern routines                                 *
 ******************************************************************************/

void Process_Copy(int sourceimage, int destimage, wimp_point *size)
{
 /*
  * copies 'sourceimage' to 'destimage'
  */
  wimp_point pixel;

  spritewin_block *block;
  char *sourceimagedata;
  int sourcelinelength;
  char *destimagedata;
  int destlinelength;

 /*
  * set up variables for fast sprite access
  */
  block = SpriteWin_GetImageBlock(sourceimage);
  sourceimagedata = (char *) block->spritearea + block->image;
  sourcelinelength = block->linebytes;

  block = SpriteWin_GetImageBlock(destimage);
  destimagedata = (char *) block->spritearea + block->image;
  destlinelength = block->linebytes;

  Hourglass_On();

  for(pixel.y = 0; pixel.y < size->y; pixel.y++){

     Hourglass_Percentage( (pixel.y * 100) / size->y);

     for(pixel.x = 0; pixel.x < size->x; pixel.x++){

         putdest(pixel, getsource(pixel));
     }
  }

  Hourglass_Off();

}

/******************************************************************************/

void Process_Add(int sourceimage, int destimage, wimp_point *size)
{
 /*
  * adds 'sourceimage' to 'destimage'
  */
  wimp_point pixel;
  int value;

  spritewin_block *block;
  char *sourceimagedata;
  int sourcelinelength;
  char *destimagedata;
  int destlinelength;

 /*
  * set up variables for fast sprite access
  */
  block = SpriteWin_GetImageBlock(sourceimage);
  sourceimagedata = (char *) block->spritearea + block->image;
  sourcelinelength = block->linebytes;

  block = SpriteWin_GetImageBlock(destimage);
  destimagedata = (char *) block->spritearea + block->image;
  destlinelength = block->linebytes;

  Hourglass_On();

  for(pixel.y = 0; pixel.y < size->y; pixel.y++){

     Hourglass_Percentage( (pixel.y * 100) / size->y);

     for(pixel.x = 0; pixel.x < size->x; pixel.x++){

         value =  getsource(pixel);
         value += getdest(pixel);

         clip_pixel(&value);

         putdest(pixel, value);
     }
  }

  Hourglass_Off();

}

/******************************************************************************/

void Process_Average(int sourceimage, int destimage, wimp_point *size)
{
 /*
  * adds 'sourceimage' to 'destimage' and divides by 2
  */
  wimp_point pixel;
  int value;

  spritewin_block *block;
  char *sourceimagedata;
  int sourcelinelength;
  char *destimagedata;
  int destlinelength;

 /*
  * set up variables for fast sprite access
  */
  block = SpriteWin_GetImageBlock(sourceimage);
  sourceimagedata = (char *) block->spritearea + block->image;
  sourcelinelength = block->linebytes;

  block = SpriteWin_GetImageBlock(destimage);
  destimagedata = (char *) block->spritearea + block->image;
  destlinelength = block->linebytes;

  Hourglass_On();

  for(pixel.y = 0; pixel.y < size->y; pixel.y++){

     for(pixel.x = 0; pixel.x < size->x; pixel.x++){

         Hourglass_Percentage( (pixel.y * 100) / size->y);

         value =  getsource(pixel);
         value += getdest(pixel);
         value =  value >> 1;

         clip_pixel(&value);

         putdest(pixel, value);
     }
  }

  Hourglass_Off();

}

/******************************************************************************/

void Process_Convolve(int sourceimage, int destimage, wimp_point *size,
                      wimp_point *anchor, int (*filter)[3][3], BOOL overwrite)
{
 /*
  * convolves 'sourceimage' to 'destimage' using 'filter'
  * if overwrite is FALSE then the destination image is added to rather than overwritten
  */

/* maps 0,0 centred matrix to 1,1 centred array */
#define matrix(x, y) (*filter)[x + 1][y + 1]

  int scalefactor;
  wimp_point coord;
  wimp_point coord2;
  wimp_point coord3;
  int pixel;

  spritewin_block *block;
  char *sourceimagedata;
  int sourcelinelength;
  char *destimagedata;
  int destlinelength;

 /*
  * set up variables for fast sprite access
  */
  block = SpriteWin_GetImageBlock(sourceimage);
  sourceimagedata = (char *) block->spritearea + block->image;
  sourcelinelength = block->linebytes;

  block = SpriteWin_GetImageBlock(destimage);
  destimagedata = (char *) block->spritearea + block->image;
  destlinelength = block->linebytes;

  Hourglass_On();

 /*
  * calculate scalefactor for filter matrix
  */
  scalefactor = 0;
  for(coord.y = -1; coord.y <= 1; coord.y++){

      for(coord.x = -1; coord.x <= 1; coord.x++){

          scalefactor += matrix(coord.x, coord.y);

      }

  }
  if(scalefactor == 0)
     scalefactor = 1;


 /*
  * convolve image
  */
  for(coord.y = 0; coord.y < size->y; coord.y++){

      Hourglass_Percentage( (coord.y * 100) / size->y);

      for(coord.x = 0; coord.x < size->x; coord.x++){

         /*
          * perform convolution
          */

         /*
          * set initial state of final pixel to 0 if overwrite of final image if not
          */
          if(overwrite)
             pixel = 0;
          else
             pixel = getdest(coord);


          for(coord2.y = -1; coord2.y <= 1; coord2.y++){

              for(coord2.x = -1; coord2.x <= 1; coord2.x++){

                  coord3.x = coord.x + coord2.x;
                  coord3.y = coord.y + coord2.y;
                  clip_coord(&coord3, size); /* make sure coord is not off image */
                  pixel += matrix(coord2.x, coord2.y) * getsource(coord3);

              }

          }

          pixel = pixel / scalefactor;

          clip_pixel(&pixel);

          putdest(coord, pixel);
        
      }
  }

  Hourglass_Off();

}

/******************************************************************************/

void Process_Median(int sourceimage, int destimage, wimp_point *size)
{
 /*
  * 3x3 median filters 'sourceimage' to 'destimage'
  */
  wimp_point coord;
  int pixelno;
  int outerloop, innerloop;
  int median[10];
  wimp_point coord2;
  wimp_point coord3;
  int pixel;

  spritewin_block *block;
  char *sourceimagedata;
  int sourcelinelength;
  char *destimagedata;
  int destlinelength;

 /*
  * set up variables for fast sprite access
  */
  block = SpriteWin_GetImageBlock(sourceimage);
  sourceimagedata = (char *) block->spritearea + block->image;
  sourcelinelength = block->linebytes;

  block = SpriteWin_GetImageBlock(destimage);
  destimagedata = (char *) block->spritearea + block->image;
  destlinelength = block->linebytes;
                                                                                        
  Hourglass_On();

 /*
  * filter image
  */
  for(coord.y = 0; coord.y < size->y; coord.y++){

      Hourglass_Percentage( (coord.y * 100) / size->y);
      
      for(coord.x = 0; coord.x < size->x; coord.x++){

         /*
          * perform median filtering
          */

        /*
         * fill array 'median' with the 9 pixels contained by the 3x3
         * matrix around the current pixel in the source image.
         * the array entries are median[1] to median[9]
         */

         pixelno = 1;

         for(coord2.y = -1; coord2.y <= 1; coord2.y++){

             for(coord2.x = -1; coord2.x <= 1; coord2.x++){

                 coord3.x = coord.x + coord2.x;
                 coord3.y = coord.y + coord2.y;
                 clip_coord(&coord3, size); /* make sure coord is not off image */
                 median[pixelno++] = getsource(coord3);

             }

         }

        /*
         * sort array median into numerical order so that the median (middle)
         * value can be found
         */

         for(outerloop = 2; outerloop <= 9; outerloop++){

             pixel = median[outerloop];

             bubblesort_subloop(&outerloop, &innerloop, &pixel, &median);

             median[innerloop + 1] = pixel;

         }

        /*
         * place median value in destination image
         */
         putdest(coord, median[5]);
      
     }

  }

  Hourglass_Off();

}

void bubblesort_subloop(int *outerloop, int *innerloop, int *pixel, int (*median)[10])
{
 /*
  * subloop for bubblesort used in median filter
  * removes need for gotos (algorithm was originally in FORTRAN!)
  */

  for(*innerloop = *outerloop - 1; *innerloop >= 1; *innerloop = *innerloop - 1){

      if( (*median)[*innerloop] < *pixel )
         return;

      (*median)[*innerloop + 1] = (*median)[*innerloop];

  }

  *innerloop = 0;

}

/******************************************************************************/

void Process_GreyHistorgram(int sourceimage, wimp_point *size, int (*greyhist)[256])
{
 /*
  * calculates grey level histogram for 'sourceimage' and places it in the array
  * 'greyhist'
  */
  wimp_point coord;
  int greylevel;

  Hourglass_On();

 /*
  * initialise 'greyhist' array to zero
  */
  for(greylevel = 0; greylevel < 256; greylevel++){

      (*greyhist)[greylevel] = 0;

  }

 /*
  * calculate histogram
  */
  for(coord.y = 0; coord.y < size->y; coord.y++){

      for(coord.x = 0; coord.x < size->x; coord.x++){

          greylevel = SpriteWin_GetPixel(&coord, sourceimage);

          (*greyhist)[greylevel]++;

      }

  }

  Hourglass_Off();

}

/******************************************************************************/

void Process_CumulativeHistorgram(int (*greyhist)[256], int (*cuhist)[256])
{
 /*
  * takes grey level histogram for 'greyhist' and calculates the cumulative
  * histogram and places it in 'cuhist'
  * NOTE: 'greyhist' and 'cuhist' can be the same array
  */
  int greylevel;
  int sum;

  Hourglass_On();

  sum = 0;

  for(greylevel = 0; greylevel < 256; greylevel++){

      sum += (*greyhist)[greylevel];

      (*cuhist)[greylevel] = sum;

  }

  Hourglass_Off();

}

/******************************************************************************/

void Process_HistogramEq(int sourceimage, wimp_point *size, char (*greymap)[])
{
 /*
  * calculates grey level histogram for 'sourceimage' and uses it to do calculate
  * the greymap for histogram equalization
  */
  int histogram[256];
  int cumulative[256];
  int totalpixels;
  int greylevel;
  float numerator;

  Hourglass_On();

 /*
  * calculate grey level histogram
  */
  Process_GreyHistorgram(sourceimage, size, &histogram);
      
 /*
  * calculate cumulative grey level histogram
  */
  Process_CumulativeHistorgram(&histogram, &cumulative);

 /*
  * calculate lookup table for histogram equalization
  * equation is:              nogreys - 1
  *              newgrey[n] = -----------  * cumulative[n]
  *                           totalpixels
  */
  totalpixels = size->x * size->y;
  numerator = (float)(256.0 - 1.0) / (float)totalpixels;

  for(greylevel = 0; greylevel < 256; greylevel++){

      (*greymap)[greylevel] = (char) (numerator * cumulative[greylevel]);

  }

  Hourglass_Off();

}

/******************************************************************************/

void Process_Scale(int sourceimage, int destimage,
                   wimp_point *sourcesize, wimp_point *destsize)
{
 /*
  * scales 'sourceimage' to size of 'destimage' using biliner interpolation and
  * resampling
  */

  /*
   * Using the grey levels of the 4 nearest neighbours, the new grey level is
   * a*x + b*y + c*x*y + d = greylevel
   * where a, b, c and d are calculated by solving the 4 simultanious equations.
   * By making the top left NN be always 0,0 when calculating, the same solution
   * to the equation can be used every time, ie.
   * where the NN are at   1     2
   *                      0,0   1,0
   *                      1,0   1,1
   *                       3     4
   * a = g2 - g1     b = g3 - g1    c = g4 - g2 + g1 - g3    d = g1
   */
  int a, b, c, d, xi, yi;
  int scalefactorx, scalefactory;
  int g1, g2, g3, g4;
  wimp_point sourcepixel;
  wimp_point destpixel;
  wimp_point NNtopleft;
  int newgrey;

  spritewin_block *block;
  char *sourceimagedata;
  int sourcelinelength;
  char *destimagedata;
  int destlinelength;

 /*
  * set up variables for fast sprite access
  */
  block = SpriteWin_GetImageBlock(sourceimage);
  sourceimagedata = (char *) block->spritearea + block->image;
  sourcelinelength = block->linebytes;

  block = SpriteWin_GetImageBlock(destimage);
  destimagedata = (char *) block->spritearea + block->image;
  destlinelength = block->linebytes;

  Hourglass_On();

 /*
  * calculate scalefactor that maps coordinate in destination image plane to
  * those in the source image plane
  */
  scalefactorx = (sourcesize->x << 10) / destsize->x;
  scalefactory = (sourcesize->y << 10) / destsize->y;

  for(destpixel.y = 0; destpixel.y < destsize->y; destpixel.y++){

     for(destpixel.x = 0; destpixel.x < destsize->x; destpixel.x++){

         Hourglass_Percentage( (destpixel.y * 100) / destsize->y);

        /*
         * calculate coord of destination image point in source frame of reference
         */
         xi = destpixel.x * scalefactorx;
         yi = destpixel.y * scalefactory;

        /*
         * calculate coord of top left nearest neighbour in source image
         */
         NNtopleft.x = xi >> 10;
         NNtopleft.y = yi >> 10;

        /*
         * get grey scale values of nearest neighbours
         */
         sourcepixel = NNtopleft;
         g1 = getsource(sourcepixel);
         sourcepixel.x++;
         clip_coord(&sourcepixel, sourcesize); /* make sure coord is not off image */
         g2 = getsource(sourcepixel);
         sourcepixel.x = NNtopleft.x;
         sourcepixel.y = NNtopleft.y + 1;
         g3 = getsource(sourcepixel);
         sourcepixel.x++;
         clip_coord(&sourcepixel, sourcesize); /* make sure coord is not off image */
         g4 = getsource(sourcepixel);

        /*
         * calculate constants
         */
         a = (g2 - g1);
         b = (g3 - g1);
         c = (g4 - g2 + g1 - g3);
         d = g1 << 10;

        /*
         * calculate coords of point in nearest neighbour frame of reference
         */
         xi = xi - (NNtopleft.x << 10);
         yi = yi - (NNtopleft.y << 10);

        /*
         * calculate new grey level for destination image
         */
         newgrey = (a*xi + b*yi + c* ((xi*yi) >> 10) + d) >> 10;

         clip_pixel(&newgrey);

         putdest(destpixel, newgrey);
     }
  }

  Hourglass_Off();

}

/******************************************************************************/

void Process_Mix(int source1, int source2, int destimage, int image1percent,
                 wimp_point *size)
{
 /*
  * mixes source1 and source2 according to the percentage in image1percent and
  * puts the result in destimage.
  * NOTE: destimage can be the same as source2
  */
  wimp_point pixel;
  int value;
  int image2percent;

  spritewin_block *block;
  char *sourceimagedata;
  int sourcelinelength;
  char *source2imagedata;
  int source2linelength;
  char *destimagedata;
  int destlinelength;

 /*
  * set up variables for fast sprite access
  */
  block = SpriteWin_GetImageBlock(source1);
  sourceimagedata = (char *) block->spritearea + block->image;
  sourcelinelength = block->linebytes;

  block = SpriteWin_GetImageBlock(source2);
  source2imagedata = (char *) block->spritearea + block->image;
  source2linelength = block->linebytes;

  block = SpriteWin_GetImageBlock(destimage);
  destimagedata = (char *) block->spritearea + block->image;
  destlinelength = block->linebytes;

  image2percent = 100 - image1percent;

  Hourglass_On();

  for(pixel.y = 0; pixel.y < size->y; pixel.y++){

     Hourglass_Percentage( (pixel.y * 100) / size->y);

     for(pixel.x = 0; pixel.x < size->x; pixel.x++){

         value =  ((getsource(pixel) << 2) * image1percent) / 100;
         value += ((getsource2(pixel) << 2) * image2percent) / 100;

         value = value >> 2;

         /*clip_pixel(&value);*/

         putdest(pixel, value);
     }
  }

  Hourglass_Off();

}

/******************************************************************************/

void Process_GreyMap(int sourceimage, int destimage, wimp_point *size, char (*greymap)[])
{
 /*
  * map the greylevels in the 'sourceimage' to the new values set out using the bargraph
  * from the window in 'barmap' and put the new image in 'destimage'
  */
  wimp_point pixel;

  spritewin_block *block;
  char *sourceimagedata;
  int sourcelinelength;
  char *destimagedata;
  int destlinelength;

 /*
  * set up variables for fast sprite access
  */
  block = SpriteWin_GetImageBlock(sourceimage);
  sourceimagedata = (char *) block->spritearea + block->image;
  sourcelinelength = block->linebytes;

  block = SpriteWin_GetImageBlock(destimage);
  destimagedata = (char *) block->spritearea + block->image;
  destlinelength = block->linebytes;

  Hourglass_On();

 /*
  * create new image using greylevel lookup table
  */

  for(pixel.y = 0; pixel.y < size->y; pixel.y++){

     Hourglass_Percentage( (pixel.y * 100) / size->y);

     for(pixel.x = 0; pixel.x < size->x; pixel.x++){

         putdest(pixel, (*greymap)[getsource(pixel)]);
     }
  }

  Hourglass_Off();

}

/******************************************************************************/

void Process_SLCE(int sourceimage, wimp_point *size, char (*greymap)[])
{
 /*
  * calculates grey level histogram for 'sourceimage' Saturating Linear Contrast 
  * Enhancement
  */
  int histogram[256];
  int greylevel;
  int sum;
  double mean;
  double sumsquares;
  double sd;
  double grad, yinter;
  int dhigh, dlow;

  Hourglass_On();

 /*
  * calculate grey level histogram
  */
  Process_GreyHistorgram(sourceimage, size, &histogram);

 /*
  * calculate mean of the grey level histogram
  */
  for(greylevel = 0; greylevel < 256; greylevel++)
      sum += histogram[greylevel] * greylevel;

  mean = (double) sum / (double) (size->x * size->y);

 /*
  * calculate standard deviation of the grey level histogram
  */
  for(greylevel = 0; greylevel < 256; greylevel++)
      sumsquares += ((mean - (double) greylevel) * (mean - (double) greylevel)) *
                    (double) histogram[greylevel];

  sd = sqrt( sumsquares / (double) (size->x * size->y) );


 /*
  * calculate lookup table for SLCE
  */
  dhigh = (int) ((3.0 * sd) + mean + 0.5);
  dlow = (int) (mean - (3 * sd) + 0.5);

  if(dlow < 0)
     dlow = 0;

  if(dhigh > 255)
     dhigh = 255;

 /*
  * below low point all are 0
  */
  for(greylevel = 0; greylevel < dlow; greylevel++)
      (*greymap)[greylevel] = 0;

 /*
  * between dlow and dhigh the levels are a linear line
  * gradient 'grad' and yintercept 'yinter'
  */
  grad = 255.0 / ( ((double) dhigh) - ((double) dlow) );
  yinter = - ( grad * ((double) dlow) );
  for(greylevel = dlow; greylevel <= dhigh; greylevel++)
      (*greymap)[greylevel] = (int) ( ( grad * ((double) greylevel) ) + yinter + 0.5);

 /*
  * above dhigh the levels are all 255
  */
  for(greylevel = dhigh; greylevel < 256; greylevel++)
      (*greymap)[greylevel] = 255;

  Hourglass_Off();

}

/******************************************************************************
 *                             misc routines                                  *
 ******************************************************************************/

void clip_pixel(int *pixel)
{
 /*
  * clip the pixel value to the allowed range
  */

  if(*pixel > 255)
     *pixel = 255;

  if(*pixel < 0)
     *pixel = 0;
     
}

/******************************************************************************/

void clip_coord(wimp_point *coord, wimp_point *imagesize)
{
 /*
  * clips coordinate so that it is in the image.
  * Therefore, outside values will use those on the edge of the images
  */

  if(coord->x < 0)
     coord->x = 0;

  if(coord->x >= imagesize->x)
     coord->x = imagesize->x - 1;

  if(coord->y < 0)
     coord->y = 0;

  if(coord->y >= imagesize->y)
     coord->y = imagesize->y - 1;
             
}
