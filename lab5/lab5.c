#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <math.h>

#define WINDOW_ROWS 7
#define WINDOW_COLS 7
#define WINDOW_SIZE 7*7
#define ITERATION_TIMES 30
#define uchar 		unsigned char
#define SQR(x)      (x)*(x)

char Sobel_filter_X[3][3]={
		{-1, -2, -1},
		 {0,  0,  0},
		 {1,  2,  1}
	};
char Sobel_filter_Y[3][3]={
		 {-1, 0, 1},
		 {-2, 0, 2},
		 {-1, 0, 1}
	};
float get_avgDist(int **contour_p, int num);
void write_PPM(uchar *img, int COLS, int ROWS, const char *filename, FILE *fpt);
void MarkPixels(uchar *img, int COLS, int x, int y, uchar color);
void image_copy(uchar *img, int COLS, int ROWS, uchar *output);
void find_minEnergy(float *win, int WIN_COLS, int WIN_ROWS, int Img_COLS, int Img_ROWS, int *x, int *y);
void Normalize(float *win,int COLS, int ROWS);
void GradMag_Filter(uchar *img, int COLS, int ROWS, char (*filter)[3], int F_ROWS, int F_COLS,float *filtered_Img);
void Img_Normalize(float *img, int COLS, int ROWS, uchar *output);
int main(int argc, char const *argv[])
{
	FILE *fpt;
	char    header[100];
	uchar *OriginalImage, *MarkedImage, *FinalImage, *Filtered_Image;
	float *Grad_Image ;
	float *TotalEnergy;
	int **contour_points;
	int ROWS, COLS, BYTES, contour_amt=0, contour_index=0;
	int win_r=0, win_c=0,i=0;
	uchar iteration=1;
	float avg_dist=0, sqr_dist=0, sqr_dist1=0;
	float *InternalEnergy_DEV, *InternalEnergy_Avg, *ExEnergy;
	int contour_x1, contour_y1;
	int contour_x0, contour_y0;
	int contour_x2, contour_y2, new_x=0,new_y=0;

	// read hawk_init image
	if ((fpt= fopen("hawk.ppm", "rb"))==NULL)
	{
		printf("Can't open image hawk.ppm\n");
		exit(0);
	}
	fscanf(fpt, "%s %d %d %d\n ", header, &COLS, &ROWS, &BYTES);
	if (strcmp(header, "P5")!=0 || BYTES!=255)
	{
		printf("It is not a 8-bit PPM file\n");
		exit(0);
	}
	OriginalImage =(uchar *) calloc(ROWS*COLS,sizeof(uchar));
	
	FinalImage =(uchar *) calloc(ROWS*COLS,sizeof(uchar));
	TotalEnergy= (float *) calloc(WINDOW_SIZE,sizeof(float));
	InternalEnergy_Avg= (float *) calloc(WINDOW_SIZE,sizeof(float));
	InternalEnergy_DEV= (float *) calloc(WINDOW_SIZE,sizeof(float));
	ExEnergy= (float *) calloc(WINDOW_SIZE,sizeof(float));
	contour_points= (int **) calloc(42, sizeof(int *));
	for (int i = 0; i < 42; ++i)
	{
		contour_points[i]=(int * )calloc(2, sizeof(int));
	}
	header[0]=fgetc(fpt);	/* read white-space character that separates header */
	fread(OriginalImage,1,COLS*ROWS,fpt);
	image_copy(OriginalImage, COLS, ROWS,FinalImage);
	fclose(fpt);

	// read hawk.txt
 	if ((fpt=fopen("hawk_init.txt","rb"))== NULL)
 	{
 		printf("Can't open hawk_init.txt. \n");
 		exit(0);
 	}
 	while(!feof(fpt))
 	{	
 		fscanf(fpt, "%d %d\n",&contour_points[contour_amt][0], &contour_points[contour_amt][1] );
 		contour_amt++;
 	}
 	fclose(fpt);

 	printf(" Contour amount %d\n", contour_amt);

/*--------Write the initial marked image------------*/
	MarkedImage =(uchar *) calloc(ROWS*COLS,sizeof(uchar));
	image_copy(OriginalImage, COLS, ROWS,MarkedImage);
 	// mark initial contour points in original image using +"
	for (contour_index=0; contour_index < contour_amt; ++contour_index)
	    {
	    	MarkPixels(MarkedImage, COLS,
	    			 contour_points[contour_index][0],  // position x
	    			 contour_points[contour_index][1],0); // position y
	    }    
 	// output image marked with initial contour
 		write_PPM(MarkedImage,COLS,ROWS,"marked.ppm",fpt);
 		free(MarkedImage);

/*-----------filter and write filtered image--------*/
	Grad_Image =(float *) calloc(ROWS*COLS,sizeof(float));		//used to calculate external energy
	Filtered_Image =(uchar *) calloc(ROWS*COLS,sizeof(uchar));
 	// calculate gradient magnitude image using Sobel filter
		GradMag_Filter(OriginalImage,COLS, ROWS, Sobel_filter_Y,3, 3,Grad_Image );
		Img_Normalize(Grad_Image, COLS,ROWS,Filtered_Image);
		



/*------------------ iterate 30 times-----------------*/
	iteration=1;
 	while(iteration<=ITERATION_TIMES)
 	{
 		iteration++;
 		// calculate average distance in one iteration
 		// loop through each contour point
 		 contour_index=0;
 		 avg_dist= get_avgDist(contour_points, contour_amt);
 		 printf("avg %f\n", avg_dist);
 		while(contour_index<contour_amt)
 		{
 			// obtain two neighbour contour points (xi,yi), (xi+1,yi+1)
 			contour_x1= contour_points[contour_index][0];
 			contour_y1= contour_points[contour_index][1];
 			if(contour_index==contour_amt-1){
 			contour_x2= contour_points[0][0];
 			contour_y2= contour_points[0][1];
 			}
 			else{
 			contour_x2= contour_points[contour_index+1][0];
 			contour_y2= contour_points[contour_index+1][1];
 			}
 			if(contour_index==0){
 			contour_x0= contour_points[contour_amt-1][0];
 			contour_y0= contour_points[contour_amt-1][1];
 			}
 			else{
 			contour_x0= contour_points[contour_index-1][0];
 			contour_y0= contour_points[contour_index-1][1];
 			}
 				for(win_r=0; win_r< WINDOW_ROWS; ++win_r)
	 			for (win_c = 0; win_c < WINDOW_COLS; ++win_c)
	 			{
 				 ExEnergy[win_r*WINDOW_COLS+win_c]=0;
 				 InternalEnergy_Avg[win_r*WINDOW_COLS+win_c]=0;
 				 InternalEnergy_DEV[win_r*WINDOW_COLS+win_c]=0;
 				 TotalEnergy[win_r*WINDOW_COLS+win_c]=0;
	 			}
 			/*Find internal energy of distance between points*/
 			for(win_r=0; win_r< WINDOW_ROWS; ++win_r)
	 			for (win_c = 0; win_c < WINDOW_COLS; ++win_c)
	 			{
 					InternalEnergy_Avg[win_r*WINDOW_COLS+win_c]=abs(SQR((contour_x1-(WINDOW_COLS/2) +win_c)-contour_x2)
 																+SQR((contour_y1+win_r-(WINDOW_ROWS/2))- contour_y2))
 																 +abs(SQR((contour_x1-(WINDOW_COLS/2) +win_c)-contour_x0)
 																 +SQR((contour_y1+win_r-(WINDOW_ROWS/2))- contour_y0));
	 			}
	 			Normalize(InternalEnergy_Avg, WINDOW_COLS,WINDOW_ROWS );
	 		
			/*Find internal energy of square of deviation between average and distance between points*/
			for(win_r=0; win_r< WINDOW_ROWS; ++win_r)
	 			for (win_c = 0; win_c < WINDOW_COLS; ++win_c)
	 			{
	 				sqr_dist=sqrt(SQR((contour_x1-(WINDOW_COLS/2) +win_c)-contour_x2)
 							+SQR((contour_y1+win_r-(WINDOW_ROWS/2))- contour_y2));
 					sqr_dist1=sqrt(SQR((contour_x1-(WINDOW_COLS/2) +win_c)-contour_x0)
 							+SQR((contour_y1+win_r-(WINDOW_ROWS/2))- contour_y0));
 					InternalEnergy_DEV[win_r*WINDOW_COLS+win_c]= SQR(avg_dist-sqr_dist)+SQR(avg_dist-sqr_dist1);
	 			}
	 			Normalize(InternalEnergy_DEV, WINDOW_COLS,WINDOW_ROWS );
	 		/*Find external energy from magnitude image*/
	 		for(win_r=0; win_r< WINDOW_ROWS; ++win_r)
	 			for (win_c = 0; win_c < WINDOW_COLS; ++win_c)
	 			{
 				 ExEnergy[win_r*WINDOW_COLS+win_c]=SQR( Grad_Image[(contour_y1-(WINDOW_ROWS/2)+win_r)*WINDOW_COLS + contour_x1-(WINDOW_COLS/2)+win_c]);
	 			}
	 			Normalize(ExEnergy, WINDOW_COLS,WINDOW_ROWS );
	 		
 			/*sum up all energy and find total energy */
			for(win_r=0; win_r< WINDOW_ROWS; ++win_r)
	 			for (win_c = 0; win_c < WINDOW_COLS; ++win_c)
	 			{
	 				
	 				TotalEnergy[win_r*WINDOW_COLS+win_c]=InternalEnergy_Avg[win_r*WINDOW_COLS+win_c]
	 													+InternalEnergy_DEV[win_r*WINDOW_COLS+win_c]
	 													-ExEnergy[win_r*WINDOW_COLS+win_c];
	 			}
	 			Normalize(TotalEnergy, WINDOW_COLS,WINDOW_ROWS );
	 			
				/*Find minimum energy location of a 7x7 window and set new contour points*/
				find_minEnergy(TotalEnergy, WINDOW_COLS,WINDOW_ROWS,COLS, ROWS,
	 							&new_x,&new_y);
				contour_points[contour_index][0]+=(new_x-WINDOW_COLS/2);
				contour_points[contour_index][1]+=(new_y-WINDOW_ROWS/2) ;
 			contour_index++;
	 		
 		}

 	}
 	// print final contour coordinate positions
 	printf("Final contour coordinate\n");
	for (i = 0; i < contour_amt; ++i)
 	{
 		printf(" %d %d\n",contour_points[i][0], contour_points[i][1] );
 	}

 
	    	for (contour_index=0; contour_index < contour_amt; ++contour_index)
	    {
	    	MarkPixels(Filtered_Image, COLS,
	    			 contour_points[contour_index][0],  // position x
	    			 contour_points[contour_index][1],255); // position y
	    }
	// write image
	 	write_PPM(Filtered_Image,COLS,ROWS,"Gradient.ppm",fpt);
	//free memory
	 	free(Filtered_Image);
 	// mark final contour points using "+"
	for (contour_index=0; contour_index < contour_amt; ++contour_index)
	    {
	    	MarkPixels(FinalImage, COLS,
	    			 contour_points[contour_index][0],  // position x
	    			 contour_points[contour_index][1],0); // position y
	    }    
	    printf("Writing image 2...\n");
 	// output image marked with final contour
 	write_PPM(FinalImage,COLS,ROWS,"contour.ppm",fpt);

	//free memory
 	free(FinalImage);
	free(OriginalImage);		//free image memory
	free(ExEnergy);				//free external energy window
	free(InternalEnergy_DEV);	//free internal energy window
	free(InternalEnergy_Avg);
	free(TotalEnergy);			//free total energy window
	free(Grad_Image);
	// free contour points matrix
	for (int i = 0; i < 42; ++i)
	{
		free(contour_points[i]);
	}
	free(contour_points);		
	return 0;
}

/*
* 
*/
void MarkPixels(uchar *img, int COLS, int x, int y, uchar color)
{
	int r=0, c=0;
	// mark the 3 cols in the center of window
	for (r=-3; r <=3; ++r)
	{
			img[(y+r)*COLS+c+x]=color;
	}
	// mark the 3 rows in the center of window
	r=0;
	for (c=-3; c <=3; ++c)
	{
			img[(r+y)*COLS+x+c]=color;
	}

}

void image_copy(uchar *img, int COLS, int ROWS, uchar *output)
{
	int r=0, c=0;
	for (r = 0; r < ROWS ; ++r)
		for (c = 0; c < COLS; ++c)
		{
			output[r*COLS+c]=img[r*COLS+c];
		}
}

/*
* 	@brief get_avgDist:
*			calculate the average distance of contour point
*/
float get_avgDist(int **contour_p, int num)
{
	double avg=0.0;
	int i=0;
	for ( i = 0; i < num; ++i)
	{
		if (i==num-1)
		{
			avg+=sqrt(SQR(contour_p[i][0]-contour_p[0][0])+SQR(contour_p[i][1]-contour_p[0][1]));
		}
		else
		{
			avg+=sqrt(SQR(contour_p[i][0]-contour_p[i+1][0])+SQR(contour_p[i][1]-contour_p[i+1][1]));
		}
	}
	avg/=num;
	return (float)avg;
}


void find_minEnergy(float *win, int WIN_COLS, int WIN_ROWS, int Img_COLS, int Img_ROWS, int *x, int *y)
{
	int r,c;
	float min_energy=0.0;
	min_energy= win[0];

	for (r = 0; r < WIN_ROWS; ++r)
		for(c=0; c< WIN_COLS; ++c)
		{
			if (win[r*WIN_COLS+c]<= min_energy){
				*x=c;
				*y=r;
				min_energy=win[r*WIN_COLS+c];
			}
		}
}


void write_PPM(uchar *img, int COLS, int ROWS, const char *filename, FILE *fpt)
{
	if ((fpt=fopen(filename,"w"))==NULL)
	{
		printf("can't write PPM file %s \n", filename);
	}

	fprintf(fpt, "P5 %d %d 255\n", COLS, ROWS);
	fwrite(img, COLS*ROWS,1, fpt );
 	fclose(fpt);

}


void Normalize(float *win,int COLS, int ROWS)
{
	int r,c;
	float max_value=win[0] , min_value= win[0];
	// find max value
	for (r = 0; r <ROWS ; ++r)
		for(c=0; c< COLS; c++)
			max_value= max_value> win[r*COLS+c] ? max_value : win[r*COLS+c];
	// find min value
	for (r = 0; r <ROWS ; ++r)
		for(c=0; c< COLS; c++)
			min_value= min_value< win[r*COLS+c] ? min_value : win[r*COLS+c];
	// normalize data back to 0~1
	for (r = 0; r <ROWS ; ++r)
		for(c=0; c< COLS; c++)
		{
			win[r*COLS+c]=  (win[r*COLS+c]-min_value)/(max_value-min_value);
		}
}

void Img_Normalize(float *img, int COLS, int ROWS, uchar *output)
{
	int r, c;
	for (r = 0; r < ROWS; ++r)
		for(c=0;c<COLS;c++)
	{
		output[r*COLS+c]=img[r*COLS+c] >255? 255: (uchar)img[r*COLS+c] ;
		// output[r*COLS+c]=img[r*COLS+c] <0? 0: (uchar)img[r*COLS+c] ;
	}
}

void GradMag_Filter(uchar *img, int COLS, int ROWS, char (*filter)[3], int F_ROWS, int F_COLS,float *filtered_Img)
{
	int r1, c1, r2,c2;
	float sum=0;
	for (r1 = F_ROWS/2; r1 < ROWS-(F_ROWS/2); ++r1)
		for (c1 = F_COLS/2; c1 < COLS-(F_COLS/2); ++c1)
		{
			sum=0;
			/*Convolution in 3x3 window*/
			for (r2 = -F_ROWS/2; r2 <= F_ROWS/2; ++r2)
				for (c2 = -F_COLS/2; c2 <= F_COLS/2; ++c2)
				{
					sum+=filter[r2+1][c2+1]*img[(r1+r2)*COLS+(c1+c2)];
				}
				// obtain gradient magnitude image
				filtered_Img[r1*COLS+c1]=abs(sum);
		}
}
