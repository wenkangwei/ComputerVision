

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>



#define uchar unsigned char
#define uint unsigned int
#define MAXLETTER 1500
#define MAX_Threshold 255
#define Threshold_Amt 20		//define threshold amount 

typedef struct GROUND_TRUE
{
	uchar thrd;
	int TP;
	int FP;
	int TPR;
	int FPR;
} GT_Rate;

// function prototypes
void normalize(int *filtered_img,int img_cols, int img_rows, uchar *msf_img);
void MSF_Filter(uchar *img,int img_cols, int img_rows, int *temp, int temp_cols, int temp_rows, int *filtered_img );
void zero_meanFilter(uchar *temp, int *mean_temp ,int temp_cols, int temp_rows);
void threshold(uchar *img,int img_cols, int img_rows, int threshold);
int is_Detected(uchar *img,int img_cols ,int p_x, int p_y, uchar threshold);
int get_minValue(int *filtered_img,int img_cols, int img_rows);

int main(int argc, char const *argv[])
{
	
FILE		*fpt;
unsigned char *image;		//original image
uchar 	*temp;				//template 
int 	*mean_temp;
int 	*msf_img;			//MSF image without being normalized
uchar 	*norm_img;			//normalized file
unsigned char  GT_letter;	//ground true letter
int 	GT_Rows, GT_Cols;	//ground true pixl position x, y 
char    header[320];	
int		Temp_ROWS,Temp_COLS,Temp_BYTES;
int		ROWS,COLS,BYTES;
// int 	threshold_value[Threshold_Amt]={213,216,219,222,225,228,231,234,236,238};	//threshold array
int 	threshold_value[Threshold_Amt]={100,110,115,120,125,130,135,140,145,150,155,165,175,180,190,200,210,215,220,230};	//threshold array
int		TP_cnt=0, TN_cnt=0, FP_cnt=0, FN_cnt=0;
GT_Rate gt[Threshold_Amt];
int index=0;
int letter_e_amt=0;

	/*read image*/
if ((fpt=fopen("/home/free/Dropbox/ComputerVision/ece431/lab2/parenthood.ppm","rb")) == NULL)
  {
	  printf("Unable to open image. for reading\n");
	  exit(0);
  }
	fscanf(fpt,"%s %d %d %d\n",header,&COLS,&ROWS,&BYTES);
if (strcmp(header,"P5") != 0  ||  BYTES != 255)
  {
	  printf("Image not a greyscale 8-bit PPM image\n");
	  exit(0);
  }

	/*Read image file*/
	printf("COLS: %d,ROWS:  %d\n",COLS,ROWS );
	image=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));		//allocate space for original image
	norm_img=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));	//allocate space for normalized image
	msf_img=(int *)calloc(ROWS*COLS,sizeof(int));						//allocate space for MSF non-8-bit image
	header[0]=fgetc(fpt);												// read white-space character that separates header 
	fread(image,1,COLS*ROWS,fpt);
	fclose(fpt);


	/* read template */
	if ((fpt=fopen("/home/free/Dropbox/ComputerVision/ece431/lab2/parenthood_e_template.ppm","rb")) == NULL)
	  {
		  printf("Unable to open template. for reading\n");
		  exit(0);
	  }
		fscanf(fpt,"%s %d %d %d\n",header,&Temp_COLS,&Temp_ROWS,&Temp_BYTES);
	if (strcmp(header,"P5") != 0  ||  BYTES != 255)
	  {
	  printf("Template not a greyscale 8-bit PPM image\n");
	  exit(0);
	  }

	printf("Temp_COLS: %d,Temp_ROWS:  %d\n",Temp_COLS,Temp_ROWS );
	temp=(uchar *)calloc(Temp_ROWS*Temp_COLS,sizeof(uchar));		//allocate space for original template
	mean_temp=(int *)calloc(Temp_ROWS*Temp_COLS,sizeof(int)); 		//allocate space for zero_mean_centered template
	header[0]=fgetc(fpt);											// read white-space character that separates header 
	fread(temp,1,Temp_COLS*Temp_ROWS,fpt);	
	fclose(fpt);

	
	/*match filtering */
	zero_meanFilter(temp, mean_temp,Temp_COLS,Temp_ROWS);			//generate zero mean filter
	MSF_Filter(image,COLS, ROWS, mean_temp, Temp_COLS,
				 Temp_ROWS, msf_img );								//convert MSF filter image to image in int type

	/*normalize MSF image*/
	normalize(msf_img,COLS,ROWS, norm_img);
	// save MSF normalized value
	fpt=fopen("MSF_image_V2.ppm","wb");
	fprintf(fpt, "P5 %d %d 255\n",COLS,ROWS);
	fwrite( norm_img,COLS*ROWS,1,fpt);								//write data to file. size 1 byte, buffer COLS*ROWS
	fclose(fpt);

	/*Image Evaluation*/
	// stop reading file when fpt is in the last line
	while(index<Threshold_Amt)
	{

		/*repeat reading ground-true file until threshold is maximum*/
		if((fpt=fopen("/home/free/Dropbox/ComputerVision/ece431/lab2/ground_true.txt","r") )== NULL)
		{
		 	printf("Can't open ground-true.txt \n");
		 	exit(0);
		}
		// clean data for the next dataset
		TP_cnt=0;
		FP_cnt=0;
		TN_cnt=0;
		FN_cnt=0;
		letter_e_amt=0;
		while(!feof(fpt))												//read file till the last line is read
		{
		 	fscanf(fpt,"%c %d %d\n",&GT_letter,&GT_Cols,&GT_Rows);								//count total letter amount
		 	if(is_Detected(norm_img, COLS,GT_Cols,GT_Rows, threshold_value[index] )!=0)
		 		{
		 			if (GT_letter=='e')	
		 			{
		 				letter_e_amt++;
		 				TP_cnt++;
		 			}
		 			else
		 				FP_cnt++;
		 		}
		 		else
		 		{
		 			if (GT_letter=='e')
		 			{
		 				letter_e_amt++;
		 				FN_cnt++;
		 			}
		 			else
		 				TN_cnt++;
		 		}
		}
		
		gt[index].thrd=threshold_value[index];				//storev threshold
		gt[index].TPR=(TP_cnt*100)/(TP_cnt+ FN_cnt);		//store percentage of TPR
		gt[index].FPR=(100*FP_cnt)/(FP_cnt+ TN_cnt);		//store percentage of FPR
		gt[index].FP=FP_cnt;								//store percentage of FP
		gt[index].TP=TP_cnt;								//store percentage of TP
		index++;
	printf("e amount: %d, TN: %d  FN %d \n", letter_e_amt, TN_cnt, FN_cnt);
	}
	printf("Threshold  TP   FP     TPR     FPR\n");
	for (int i = 0; i < index; ++i)
	{
		printf("%d       %d    %d     %d    %d\n", gt[i].thrd,gt[i].TP,gt[i].FP,gt[i].TPR,gt[i].FPR);
	}
printf(" TPR  \n");
	for (int i = 0; i < index; ++i)
	{
		printf("%d\n",gt[i].TPR);
	}
printf(" FPR\n");
	for (int i = 0; i < index; ++i)
	{
		printf("%d\n",gt[i].FPR);
	}
printf(" Threshold\n");
	for (int i = 0; i < index; ++i)
	{
		printf("%d\n",gt[i].thrd);
	}


	/*Threshold filtered image*/
    threshold(norm_img,COLS, ROWS, 234);

	/*output MSF binary image*/
	fpt=fopen("Bin_image.ppm","wb");
	fprintf(fpt, "P5 %d %d 255\n",COLS,ROWS);
	fwrite( norm_img,COLS*ROWS,1,fpt);
	fclose(fpt);



	return 0;
}



/*
*	@brief: zero_meanFilter
*			convert the given template filter to an average zero-mean filter
*	@para *temp
*			the template array
*	@para *mean_temp:
*			output calculated mean filter
*	@para temp_rows:
*			rows of template matrix
*	@para temp_cols:
*			columns of template matrix
*
*/
void zero_meanFilter(uchar *temp, int *mean_temp, int temp_cols, int temp_rows)
{
	int r=0, c=0;
	int mean=0;
	/*find mean of the template*/
	for (r = 0; r < temp_rows; ++r)
		for (c = 0; c <temp_cols ; ++c)
		{
			mean +=temp[r*temp_cols+c];
		}

	mean/=temp_cols*temp_rows;

	/*create zero mean centered filter*/
	for (r = 0; r < temp_rows; ++r)
		for (c = 0; c <temp_cols ; ++c)
		{
			mean_temp[r*temp_cols+c]=temp[r*temp_cols+c] -mean ;
			
		}

}




int get_minValue(int *filtered_img,int img_cols, int img_rows)
{
	int r=0, c=0;
	int min_value=0;
	for (r= 0; r < img_rows; ++r)
	{
		for (c = 0; c < img_cols; ++c)
		{
			if(min_value>filtered_img[r*img_cols+c])
				min_value= filtered_img[r*img_cols+c];

		}
	}

	return min_value;
}





int get_maxValue(int *filtered_img,int img_cols, int img_rows)
{
	int r=0, c=0;
	int max_value=0;
	for (r= 0; r < img_rows; ++r)
	{
		for (c = 0; c < img_cols; ++c)
		{
			if(max_value<filtered_img[r*img_cols+c])
				max_value= filtered_img[r*img_cols+c];

		}
	}

	return max_value;
}


/*
*	@brief: MSF_Filter
*			mean specific filter, convolute the original image with 
*			zero mean filter 
*	@para *img
*			the original image used to convolute with template
*
*	@para *filtered_img:
*			the MSF image without normalizing
*	@para img_cols:
*			column amount of filtered image
*	@para img_rows:
*			row amount of filtered image
*	@para *temp:
*			zero mean template matrix
*
*/
void MSF_Filter(uchar *img,int img_cols, int img_rows, int *temp, int temp_cols, int temp_rows, int *filtered_img )
{
	int r=0, c=0, r1=0,c1=0;
	int sum=0;

	/* using matched-spatial filtering*/
	for ( r =0 ; r < img_rows; ++r)		//scan image vertically 
		for (c =0 ; c < img_cols; ++c)	//scan image horizontally
		{
			sum=0;
			// calculate MSF image values
			for (r1= -temp_rows/2; r1 <temp_rows/2 ; ++r1)
				for (c1 =-temp_cols/2; c1 < temp_cols/2 ; ++c1)
				{
					sum+=img[(r+r1)*img_cols+(c1+c)] * temp[((temp_rows/2) + r1)*temp_cols+((temp_cols/2) +c1)];
				}
			filtered_img[r*img_cols+c]=sum;
		}


}



/*
*	@brief: normalize
*			convert the pixel with value out of 0~255 back to 8-bit data
*	@para *filtered_img:
*			the MSF image without normalizing
*	@para img_cols:
*			column amount of filtered image
*	@para img_rows:
*			row amount of filtered image
*	@para *msf_image:
*			normalized image
*
*/
void  normalize(int *filtered_img,int img_cols, int img_rows, uchar *msf_img)
{
	int r=0, c=0;
	int max_value=0;
	int min_value=0;
	/*find mean of the template*/
	max_value=get_maxValue(filtered_img,img_cols,img_rows);	//get maximum pixel value
	min_value=get_minValue(filtered_img,img_cols,img_rows);	//get maximum pixel value
	printf("max value : %d  min value: %d \n",max_value,min_value );
	// printf("0: %d\n", );
	for (r = 0; r < img_rows; ++r)
		for (c = 0; c <img_cols ; ++c)
		{
				if (filtered_img[r*img_cols+c]>0)
				{
				// 	/* code */
					msf_img[r*img_cols+c]=(filtered_img[r*img_cols+c])*255/(max_value);
				}
				else
				{
					msf_img[r*img_cols+c]=0;
				}

		}
}


/*
*	@brief: threshold
*			convert the pixel with value out of 0~255 back to 8-bit data
*	@para  *img:
*			the MSF image without normalizing
*	@para img_cols:
*			column amount of filtered image
*	@para img_rows:
*			row amount of filtered image
*	@para threshold
*			 threshold value 
*
*/

void threshold(uchar *img,int img_cols, int img_rows, int threshold)
{
	int r=0, c=0;
	/*find mean of the template*/
	for (r = 0; r < img_rows; ++r)
		for (c = 0; c <img_cols ; ++c)
		{
			// printf(" %d \n", img[r*img_cols+c]);
			img[r*img_cols+c]= img[r*img_cols+c]>threshold ? 255 : 0;
		}

}


/*
*	@brief: is_Detected
*			detect the area centered at the given position. if pixel
*			value is greater than threshold, return 1. Otherwise, return 0
*	@para  *img:
*			the MSF image without normalizing
*	@para img_cols:
*			column amount of filtered image
*	@para p_x:
*			x postion of the given pixel
*	@para p_y:
*			y postion of the given pixel
*
*	@para threshold
*			 threshold value 
*	@return 
*			return the number of value >threshold in a 9x15 area
*/
int is_Detected(uchar *img,int img_cols ,int p_x, int p_y, uchar threshold)
{
	int c=0, r=0;
	int detected_cnt=0;
	/*scan 9x15 area value*/
	for (r = -15/2; r < 15/2; ++r)				//scan area
		for ( c = -9/2; c < 9/2; ++c)		//scan column
		{	
			if (img[(r+p_y)*img_cols+(c+p_x)]>threshold)
				detected_cnt++;
		}

	return detected_cnt;
}






