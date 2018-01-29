

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>



#define uchar unsigned char
#define uint unsigned int
#define MAXLETTER 1500
#define MAX_Threshold 255
#define Threshold_Amt 40		//define threshold amount 
#define LETTER_ROWS	15
#define LETTER_COLS	9
typedef enum be_point{ ENDPOINT,		//only one edge->non-edge transition
			   BRANCHPOINT, 	//more than two edge->non-edge transition
			   SEGPOINT,		//two edge->non-edge transition
			   SINGLEPOINT      //without edge transition
			} BE_Point;
typedef struct GROUND_TRUE
{
	uchar thrd;
	int TP;
	int FP;
	int TN;
	int FN;
	int TPR;
	int FPR;
} GT_Rate;

// function prototypes
void print_image(uchar *img, int cols, int rows);
void save_image(const char *s, uchar *img, int cols, int rows);
void update_template(uchar *img, int x, int y, int img_cols,int img_rows, uchar *letter_tmp);
void threshold(uchar *img,int img_cols, int img_rows, int threshold);
int is_Detected(uchar *img,int img_cols ,int p_x, int p_y, uchar threshold);
int get_EdgeTrans(uchar *img, int img_cols, int img_rows,int x, int y);
int get_neighborNum(uchar *img, int img_cols,int img_rows ,int x, int y);
BE_Point check_BEPoints(uchar * sub_img, int img_cols, int img_rows,int x, int y);
int eval_BEPoints(uchar *sub_img, int img_cols, int img_rows, int *EP, int *BP);
void thin_edge(uchar *sub_img,	//subimage address
				int img_cols,	//sub-image size
				int img_rows
				 );



int main(int argc, char const *argv[])
{
	
FILE	*fpt;
uchar 	*image;				//original image
uchar 	*temp;				//template of 9x15 region 
uchar 	*msf_img;			//MSF image without being normalized
uchar   GT_letter;			//ground true letter
int 	GT_Rows, GT_Cols;	//ground true pixl position x, y 
char    header[320];	
int 	T=0;
int		ROWS,COLS,BYTES;	//used to read original image format
int		TP_cnt=0, TN_cnt=0, FP_cnt=0, FN_cnt=0;	
GT_Rate gt[Threshold_Amt];	//store ground-truth data
int     index=0;
int     letter_e_amt=0;
int 	copied_flag=0;		//used as a flag for showing template image
int 	EP_cnt, BP_cnt;		//the number of endpoint and branchpoint

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

	/*store original image in buffer*/
	printf("COLS: %d,ROWS:  %d\n",COLS,ROWS );
	image=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));		//allocate space for original image
	header[0]=fgetc(fpt);												// read white-space character that separates header 
	fread(image,1,COLS*ROWS,fpt);
	fclose(fpt);


	/* read MSF image */
	if ((fpt=fopen("/home/free/Dropbox/ComputerVision/ece431/lab3/MSF_image.ppm","rb")) == NULL)
	  {
		  printf("Unable to open template. for reading\n");
		  exit(0);
	  }
		fscanf(fpt,"%s %d %d %d\n",header,&COLS,&ROWS,&BYTES);
	if (strcmp(header,"P5") != 0  ||  BYTES != 255)
	  {
	  printf("MSF image not a greyscale 8-bit PPM image\n");
	  exit(0);
	  }

	 // store MSF image in buffer
	printf("Temp_COLS: %d,Temp_ROWS:  %d\n",COLS,ROWS );
	msf_img=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));	//allocate space for normalized image
	header[0]=fgetc(fpt);											// read white-space character that separates header 
	fread(msf_img,1,COLS*ROWS,fpt);	
	fclose(fpt);
	// allocate spaces for template image.
	temp=(uchar *) calloc(LETTER_COLS*LETTER_ROWS,sizeof(uchar));
	
	/*Image Evaluation*/
	// start thresholding at 210
	T=174;
	while(index<Threshold_Amt)
	{

		T+=2;
		/*repeat reading ground-true file until threshold is maximum*/
		if((fpt=fopen("/home/free/Dropbox/ComputerVision/ece431/lab3/ground_true.txt","r") )== NULL)
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

		while(!feof(fpt))																	//read file till the last line is read
		{
		 	fscanf(fpt,"%c %d %d\n",&GT_letter,&GT_Cols,&GT_Rows);							//count total letter amount
		 	// create and update
		 	update_template(image,GT_Cols,GT_Rows,COLS,ROWS,temp);
	 		// threshold 9x15 template at 128
	 		threshold(temp,LETTER_COLS,LETTER_ROWS,128);	

	 		/*print thresholded image, thinned image and marked image example in terminal */
	 		// print letter 'k' at x=55,y=58 in ground true text
		 	if(GT_Cols==55 && GT_Rows==58 && copied_flag ==0)
		 	{
		 		copied_flag=1;
		 		printf("Copying image\n");
		 		printf("\n\n Thresholded Image\n");
		 		save_image("letter_bin.ppm", temp, LETTER_COLS, LETTER_ROWS);
		 		print_image(temp, LETTER_COLS, LETTER_ROWS);
		 		// thin and print image
		 		thin_edge(temp,LETTER_COLS, LETTER_ROWS);
				printf("\n\n Thinned Image: \n");
				save_image("letter_thinned.ppm", temp, LETTER_COLS, LETTER_ROWS);
				print_image(temp, LETTER_COLS, LETTER_ROWS);
		 		// check BP and EP of thinned image
				printf("\n\n Detected and marked Image: \n");
		 		eval_BEPoints(temp,LETTER_COLS, LETTER_ROWS, &EP_cnt,&BP_cnt);
				print_image(temp, LETTER_COLS, LETTER_ROWS);
				printf("endpoint: %d, branchpoint: %d\n",EP_cnt, BP_cnt );


		 	}
	 		// thin 9x15 template
	 		thin_edge(temp,LETTER_COLS, LETTER_ROWS);

		 	if(is_Detected(msf_img, COLS,GT_Cols,GT_Rows, T )!=0)		//check 9x15 region to detect letter
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
		 			if(!eval_BEPoints(temp, LETTER_COLS,LETTER_ROWS, &EP_cnt,&BP_cnt))
					{

						if (GT_letter=='e')
			 			{
			 				letter_e_amt++;
			 				FN_cnt++;	// real'e' can't be detected by two detection
			 			}
			 			else
			 				TN_cnt++;	// Not 'e' is true
	
					}
					else 
						if (GT_letter=='e')
			 			{
			 				letter_e_amt++;
			 				TP_cnt++;	// real'e' can't be detected by two detection
			 			}
			 			else
			 				FP_cnt++;	// Not 'e' is true
		 		
		 		}
		}
		gt[index].thrd=T;				//storev threshold
		gt[index].TPR=(TP_cnt*100)/(TP_cnt+ FN_cnt);		//store percentage of TPR
		gt[index].FPR=(100*FP_cnt)/(FP_cnt+ TN_cnt);		//store percentage of FPR
		gt[index].FP=FP_cnt;								//store percentage of FP
		gt[index].TP=TP_cnt;								//store percentage of TP
		gt[index].TN=TN_cnt;								//store percentage of FP
		gt[index].FN=FN_cnt;								//store percentage of FP
		index++;
	}


		printf("e amount: %d\n", letter_e_amt);
		printf("Threshold  TP   FP   TN  FN  TPR     FPR\n");
	for (int i = 0; i < index; ++i)
	{
		printf("%d       %d    %d    %d   %d   %d    %d\n", gt[i].thrd,gt[i].TP,  gt[i].FP,  gt[i].TN, gt[i].FN ,gt[i].TPR,gt[i].FPR);
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

	fclose(fpt);
	return 0;
}

/*
*	@brief print_image:
*			print image in terminal in matrix form
*/
void print_image(uchar *img, int cols, int rows)
{
	 for (int i = 0; i < rows; ++i)
		{
			for (int j = 0; j < cols; ++j)
			{
				if(img[i*cols+j]<10)
					printf("%d   ",img[i*cols+j] );
				else
					printf("%d ",img[i*cols+j] );
					
			}
			printf(" \n");
		}
}


/*
*	@brief save_image:
*			save image to local directroy with input of 
*			image name, image address and image size, 
*/
void save_image(const char *s, uchar *img, int cols, int rows)
{
	FILE *w_fpt;
		w_fpt=fopen(s,"wb");
		fprintf(w_fpt, "P5 %d %d 255\n",cols,rows);
		fwrite( img,cols*rows,1,w_fpt);								//write data to file. size 1 byte, buffer COLS*ROWS
		fclose(w_fpt);
}


/*
*	@brief update_template:
*			update the 9x15 letter template for each
*			location indicated in ground truth text
*	@para *img 
*			original image
*	@para x
*			x position of ground truth pixel
*	@para y
*			y position of ground truth pixels
*	@para img_cols
*			columns amount of original image used to get 9x15 region value
*	@letter_tmp
*			return image of letter 
*/

void update_template(uchar *img, int x, int y, int img_cols,int img_rows, uchar *letter_tmp)
{

	int c, r,c1,r1;
	// Since some neighbor ground true images are overlap at 9x15region
	// rectify the location by shifting it to right 1 bit
	if(x>4)	x=x-1;
	for(r=y-7, r1=0;r<=(y+7); r++,r1++)
	{
		for(c=x-4, c1=0; c<=x+4; c++,c1++)
			letter_tmp[r1*9+c1] = img[r*img_cols+c];
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
	// threshold the image to image with 255 or 0 value
	for (r = 0; r < img_rows; ++r)
		for (c = 0; c <img_cols ; ++c)
		{
			img[r*img_cols+c]=  (img[r*img_cols+c]>=threshold)? 0: 255;
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
	for (r = -7; r <=7; ++r)				//scan area
		for ( c = -4; c <=4; ++c)		//scan column
		{	
			if (img[(r+p_y)*img_cols+(c+p_x)]>threshold)
				detected_cnt++;
		}

	return detected_cnt;
}


/*
*	@brief get_EdgeTrans: 
*			it takes image address and size as input 
*			and output the number of edge-nonedge transition
*/

int get_EdgeTrans(uchar *img, int img_cols, int img_rows,int x, int y)
{
	int r, c;
	int trans_num=0;
	uchar front_val=0, rear_val=0;
	int x_head=-1, x_end=1, y_head=-1, y_end=1;
	// check valid position
	if(x <0 ||x>=img_cols||y<0|| y>=img_rows) return -1;
	if( y-1<0) y_head=0;
	if(y+1>= img_rows) y_end=0;
	if(x-1 <0) x_head=0;
	if(x +1>=img_cols) x_end=0;
	//check top line
	for (c=x_head; c< x_end;c ++)
	{
		front_val=img[(y+y_head )*img_cols+x+c];
		rear_val=img[(y+y_head)*img_cols+c+x+1];
		// check 8-neighbor pixel, if it is boundary pixel, do the following
		if(y_head ==0)
		{
			trans_num= img[ y*img_cols+x-1] !=0? trans_num+1: trans_num;
			trans_num= img[ y*img_cols+x+1] !=0? trans_num+1: trans_num;
			break;
		}
		else if(front_val!= rear_val && (front_val & rear_val)==0) //check boundary transition
					trans_num++;
	}
	//check rightmost column
	for (r=y_head; r< y_end; ++r)
	{
		front_val=img[(y+r )*img_cols+x+x_end];
		rear_val=img[(y+r+1)*img_cols+x+x_end];

		if(x_end ==0)
		{	
			trans_num= img[ (y-1)*img_cols+x] !=0? trans_num+1: trans_num;
			trans_num= img[ (y+1)*img_cols+x] !=0? trans_num+1: trans_num;
			break;
		}
		else if(front_val!= rear_val &&(front_val & rear_val)==0 )
					trans_num++;
	}

	//check bottom line
	for (c= x_end;c >x_head ; --c)
	{
		front_val=img[(y+y_end )*img_cols+x+c];
		rear_val= img[(y+y_end)*img_cols+c+x-1];
		if(y_end ==0)
		{
			trans_num= img[ y*img_cols+x-1] !=0? trans_num+1: trans_num;
			trans_num= img[ y*img_cols+x+1] !=0? trans_num+1: trans_num;
			break;
		}
		else if( front_val!=rear_val && (front_val & rear_val) ==0)
					trans_num++;
	}
	//check leftmost column
	for ( r=y_end; r> y_head; --r)
	{
		front_val =img[(y+r-1)*img_cols+x+x_head];
		rear_val=img[(y+r )*img_cols+x+x_head];
		if(x_head ==0)
		{
			trans_num= img[ (y-1)*img_cols+x] !=0? trans_num+1: trans_num;
			trans_num= img[ (y+1)*img_cols+x] !=0? trans_num+1: trans_num;
			break;
		}
		else
		if(front_val !=rear_val && (front_val & rear_val)==0 )
					trans_num++;
	}
	return trans_num/2;
}


/*
*	@brief get_neighborNum
*			return the neighbor edge pixels' amount of each
*			location
*	@para x, y:
*			location of current pixel
*	@para img_cols, img_rows:
*			size of image
*	@return amount of neighbor pixels
*/

int get_neighborNum(uchar *img, int img_cols,int img_rows ,int x, int y)
{
	int c=0, r=0, num=0;
	for(r=y-1; r<= y+1; r++)
		for (c = x-1; c <= x+1; ++c)
		{
			if (c==x && r==y)
				continue;
				if(c>=0 && c <img_cols && r>=0 && r< img_rows)
				num= img[r*img_cols+c]!=0? num+1: num;
		}
	return num;
}



/**
*	@brief thin_edge
*			it is to thin the edge in the binary template image
*			to single-pixel edge image. The input must be 9x15 binary image
*	@para *sub_img
*			binary letter image as input
*	@para img_cols
*			number of image's column
*	@para img_rows
*			number of image's row
*	@para *dest
*			return thinned destination binary image
*/

void thin_edge(uchar *sub_img,	//subimage address
				int img_cols,	//sub-image size
				int img_rows
				
				 )	
{
	// uchar temp[img_rows*img_cols];
	int c,r;
	int neighbor_num;
	int edge_num=0;
	int marked_num=1;
	// clean the boundary value before thinning


	/*scan and label nonedge pixels*/
	 while(marked_num>0)
	{

	// clean marked number before next marking
	marked_num=0;
	
	// scan each pixel for marking
	for (r = 1; r < img_rows-1; ++r)
	{
		for (c = 1; c < img_cols-1; ++c)
		{
			// scan neighbor pixel for each pixel
			// mark pixels for erasure
			neighbor_num=get_neighborNum(sub_img,img_cols, img_rows,c,r);
			edge_num= get_EdgeTrans(sub_img, img_cols,img_rows,c,r);
			// check edge->noedge transition number
			if( neighbor_num >=3 && neighbor_num <=7 &&  edge_num==1)
			{
				
					// check N S W E pixels states
					if((sub_img[r*img_cols+c-1]==0&& sub_img[(r+1)*img_cols+c]==0) ||sub_img[(r-1)*img_cols+c] ==0 || sub_img[r*img_cols+c+1]==0 )
					{
					//if it is non-edge doesn't mark the point
						if( sub_img[r*img_cols+c]!=0)
						{

							sub_img[r*img_cols+c]= 127 ;
							marked_num++;
						}
					}
			}
		
		}
	}
		//clean marked pixels.
		for (r = 1; r < img_rows-1; ++r)
			for (c = 1; c < img_cols-1; ++c)
			{
				if(sub_img[r*img_cols+c]==127)
				{
					// printf("c: %d  r: %d \n",c, r );
					sub_img[r*img_cols+c]=0;
				}					
			}

	}



}

/*
*	@brief: check_BEPoints
*			iterate the letter image and check whether it 
*			is a endpoint or branchpoint
*	@para:	*sub_img
*			address of letter image, which should be binary value
*	@para:	img_cols
*			the number of image columns used to iterate image
*	@para:	x
*			x position of current pixel
*	@para:	y
*			y position of current pixel 
*	@return	
*			SINGLEPOINT:    no edge transition
*			ENDPOINT: 		1 edge->non edge transition 
*			BRANCHPOINT:	more than 2 edge-> non-edge transition
*			SEGPOINT:		2 edge->non-edge transition
*/

BE_Point check_BEPoints(uchar *sub_img, int img_cols, int img_rows,int x, int y)
{
	int edge_num;
	//skip checking non-edge point
	if(sub_img[y*img_cols+ x]!=0)
		edge_num= get_EdgeTrans(sub_img, img_cols, img_rows,x,y );
	else
		return SINGLEPOINT;
	if (edge_num==1)
	{	//mark endpoint pixels with 1
		sub_img[y*img_cols+x] =1;
		return ENDPOINT;
	}
	if (edge_num==2)
	{	//mark segmentation point pixels with 2
		sub_img[y*img_cols+x] =2;
		return SEGPOINT;
	}

if (edge_num==3)
	{	//mark branchpoint pixels with 3
		sub_img[y*img_cols+x] =3;
		return BRANCHPOINT;
	}
	return SINGLEPOINT;
}



/*
*	@brief: eval_BEPoints
*			iterate the letter image and check whether it has exactly 
*			1 endpoint and 1 branchpoint. If it is, return 1
*			else 0;
*	@para:	*sub_img
*			address of letter image, which should be binary value
*	@para:	img_cols
*			the number of image's columns 
*	@para:	img_rows
*			the number of rows of image
*	@para : *EP, *BP
*			return the amount of endpoint and branchpoint
*	@return	
*			1: it has 1 endpoint and branchpoint
*			0: it doesn't
*/
int eval_BEPoints(uchar *sub_img, int img_cols, int img_rows, int *EP, int *BP)
{
	int endpoint_cnt=0;
	int branchpoint_cnt=0;
	BE_Point status;
	int c=0, r=0;
	for(r=0; r< img_rows; r++)
		for(c=0; c<img_cols; c++)
		{
			status			=check_BEPoints(sub_img, img_cols,img_rows, c, r);
			endpoint_cnt	=status==ENDPOINT ? endpoint_cnt+1 :endpoint_cnt;
			branchpoint_cnt =status==BRANCHPOINT ? branchpoint_cnt+1 : branchpoint_cnt;
		}
	*EP=endpoint_cnt;
	*BP=branchpoint_cnt;
	if( endpoint_cnt==1 && branchpoint_cnt==1)	return 1;

	return 0;
}

