
	/*
	** This program reads bridge.ppm, a 512 x 512 PPM image.
	** It smooths it using a standard 3x3 mean filter.
	** The program also demonstrates how to time a piece of code.
	**
	** To compile, must link using -lrt  (man clock_gettime() function).
	*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>


// functions declaration
void conv2D_Filter(int rows, int cols, unsigned char *pic,unsigned char *smoothed, int kernel_width, int kernel_height);
void SeparatedConv_Filter(int rows, int cols, unsigned char *pic, unsigned char *smoothed, int kernel_width, int kernel_height);
void SldWin_Filter(int rows, int cols, unsigned char *pic, unsigned char *smoothed, int kernel_width, int kernel_height);
int main(int argc, char const *argv[])
{
FILE		*fpt;
unsigned char	*image;
unsigned char *smoothed_con2D;
unsigned char *smoothed_Sepfilter;
unsigned char	*smoothed_SldWin;
char    header[320];
int		ROWS,COLS,BYTES;
// int		r,c,r2,c2,sum;
struct timespec	tp1,tp2;

	/* read image */
if ((fpt=fopen("/home/free/Pictures/bridge.ppm","rb")) == NULL)
  {
  printf("Unable to open bridge. for reading\n");
  exit(0);
  }
fscanf(fpt,"%s %d %d %d\n",header,&COLS,&ROWS,&BYTES);
if (strcmp(header,"P5") != 0  ||  BYTES != 255)
  {
  printf("Not a greyscale 8-bit PPM image\n");
  exit(0);
  }
image=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
header[0]=fgetc(fpt);	/* read white-space character that separates header */
fread(image,1,COLS*ROWS,fpt);
fclose(fpt);

  /* allocate memory for smoothed version of image */
smoothed_con2D=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
smoothed_Sepfilter=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));
smoothed_SldWin=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));


printf("Basic 2D convolution: \n");
clock_gettime(CLOCK_REALTIME,&tp1);
printf("begin: %ld %ld\n",(long int)tp1.tv_sec,tp1.tv_nsec);
  /*2D convolution*/
conv2D_Filter(ROWS,COLS,image,smoothed_con2D,7,7);
  /* query timer */
clock_gettime(CLOCK_REALTIME,&tp2);
fpt=fopen("smoothed_con2D.ppm","w");
fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
fwrite(smoothed_con2D,COLS*ROWS,1,fpt);

  /* report how long it took to smooth */
printf("end: %ld %ld\n",(long int)tp2.tv_sec,tp2.tv_nsec);
printf("elapse: %ld\n",tp2.tv_nsec-tp1.tv_nsec);
fclose(fpt);





printf("\n Separated filter convolution: \n");

clock_gettime(CLOCK_REALTIME,&tp1);                   //get start time
printf("begin: %ld %ld\n",(long int)tp1.tv_sec,tp1.tv_nsec);
/*Seperated filter convolution*/
SeparatedConv_Filter(ROWS,COLS,image,smoothed_Sepfilter,7,7);           //seperated filter smooth 
clock_gettime(CLOCK_REALTIME,&tp2);                   //get end time
/*Save smoothed image*/
fpt=fopen("smoothed_SeparatedFilter.ppm","w");
fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
fwrite(smoothed_Sepfilter,COLS*ROWS,1,fpt);

  /* report how long it took to smooth */
printf("end:  %ld %ld\n",(long int)tp2.tv_sec,tp2.tv_nsec);
printf("elapse: %ld\n",tp2.tv_nsec-tp1.tv_nsec);
fclose(fpt);




printf("\n Separated filter and sliding window: \n");
clock_gettime(CLOCK_REALTIME,&tp1);                   //get start time
printf("begin: %ld %ld\n",(long int)tp1.tv_sec,tp1.tv_nsec);
/*Seperated filter convolution*/
SldWin_Filter(ROWS, COLS, image, smoothed_SldWin, 7,7);
clock_gettime(CLOCK_REALTIME,&tp2);                   //get end time
/*Save smoothed image*/
fpt=fopen("smoothed_SlidingWin.ppm","w");
fprintf(fpt,"P5 %d %d 255\n",COLS,ROWS);
fwrite(smoothed_SldWin,COLS*ROWS,1,fpt);

  /* report how long it took to smooth */
printf("end:  %ld %ld\n",(long int)tp2.tv_sec,tp2.tv_nsec);
printf("elapse: %ld\n",tp2.tv_nsec-tp1.tv_nsec);



fclose(fpt);
}





/*
*   @brief  conv2D_Filter:
*           implement the 2D convolution operation to image and return the 
*           processed image by *smoothed parameter 
*   @para rows:
*           the row  mumber of the picture to be processed
*   @para cols:
*           the columns number of the  picture to be processed
*   @para *pic :
*           address of the array storing the image
*   @para *smoothed:
*           address of the array storing processed image
*   @para kernel_width:
*           the width of filter mask
*   @para kernel_height:
*           the height of filter mask
*
*/
void conv2D_Filter(int rows, int cols, unsigned char *pic,unsigned char *smoothed, int kernel_width, int kernel_height)
{
  int r=0, c=0;
  double sum=0;
  int r2=0, c2=0;           //set window  
  for (r=kernel_height/2; r<rows-1; r++)
    for (c=kernel_width/2; c<cols-1; c++)
      {
      sum=0;
      for (r2=-3; r2<=3; r2++)
        for (c2=-3; c2<=3; c2++)
       {
          sum+=pic[(r+r2)*cols+(c+c2)];
       }
      smoothed[r*cols+c]=sum/(kernel_height*kernel_width);
      }

}


/*
*   @brief  Separated_Filter:
*           implement the 2D convolution operation to image by separating 
*           2D mask to two 1D arrays to reduce processing time and return 
*           the processed image by *smoothed parameter 
*   @para rows:
*           the row  mumber of the picture to be processed
*   @para cols:
*           the columns number of the  picture to be processed
*   @para *pic :
*           address of the array storing the image
*   @para *smoothed:
*           address of the array storing processed image
*   @para kernel_width:
*           the width of filter mask
*   @para kernel_height:
*           the height of filter mask
*
*/
void SeparatedConv_Filter(int rows, int cols, unsigned char *pic, unsigned char *smoothed, int kernel_width, int kernel_height)
{
  int r=0, c=0, r2=0, c2=0;
  double sum=0;
  
  /*7x1 filter 7rows 1 column filter*/
  for (c =kernel_height/2 ; c <cols-1; c++)       //scan each column
    for (r = kernel_height/2; r<rows-1; r++)      //scan elements in the same column
    {
      sum=0;
      for (r2=-3; r2 <=3; r2++)     //scan a vertical window 
        {
        sum+=pic[(r+r2)*cols+c];
        }

      smoothed[r*cols+c]=sum/kernel_height;
    }


  // 1x7 filter. 1 row, 7 cols filter
  for (r = kernel_width/2; r < rows-1; r++)
    for (c = kernel_width/2; c<cols-1; c++)
    {
      sum=0;
      for (c2=-3; c2 <=3; c2++)
      {
        sum+=smoothed[r*cols+(c+c2)];
      }
      smoothed[r*cols+c]=sum/kernel_width;
    }
  


}


/*
*   @brief  SldWin_Filter:
*           implement the sliding window operation and 2 separated masks operation
*           to image to reduce processing time and return 
*           the processed image by *smoothed parameter 
*   @para rows:
*           the row  mumber of the picture to be processed
*   @para cols:
*           the columns number of the  picture to be processed
*   @para *pic :
*           address of the array storing the image
*   @para *smoothed:
*           address of the array storing processed image
*   @para kernel_width:
*           the width of filter mask
*   @para kernel_height:
*           the height of filter mask
*
*/

void SldWin_Filter(int rows, int cols, unsigned char *pic, unsigned char *smoothed, int kernel_width, int kernel_height)
{

int r=0, c=0, r2=0, c2=0;
double sum=0;


 

// 1x7 filter. 1 row, 7 cols filter
  for (r = kernel_width/2; r < rows-1; r++)    //scan each row
    for (c = kernel_height/2; c<cols-1; c++)      //scan each element in horizontal window
    {
      // if it is the first window, sum all elements
      if (c==kernel_height/2)
      {
        sum=0;
        for (c2=-(kernel_width/2); c2 <=kernel_height/2; c2++)
          sum+=pic[r*cols+(c+c2)];
      }
      else
      {
        // if it isn't the first window, delete the first element and add new one
        sum=sum-pic[r*cols+(c-1-kernel_width/2)]+pic[r*cols+(c+kernel_width/2)];
      }
        smoothed[r*cols+c]=sum/kernel_width;
    }
  


 // 7x1 filter 7rows 1 column filter
  for (c =kernel_height/2 ; c <cols-1; c++)       //scan each column
    for (r = kernel_width/2; r<rows-1; r++)     //scan elements in vertial window
    {
        // if it is the first window, sum all elements
      if (r==kernel_width/2)
      {
        sum=0;
        for (r2=-(kernel_height/2); r2 <=kernel_height/2; r2++)     //scan a vertical window 
          sum+=smoothed[(r+r2)*cols+c];
      }
      else
      {
        // if it isn't the first window, delete the first element and add new one
        sum=sum-smoothed[(r-1-kernel_width/2)*cols+c]+smoothed[(r+kernel_width/2)*cols+c];

      }
        smoothed[r*cols+c]=sum/kernel_height;
    }
  
  

  

}
