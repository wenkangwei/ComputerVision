#include <stdio.h>
#include <stdlib.h>
#define uchar unsigned char

typedef enum be_point{ ENDPOINT,		//only one edge->non-edge transition
			   BRANCHPOINT, 	//more than two edge->non-edge transition
			   SEGPOINT,		//two edge->non-edge transition
			   SINGLEPOINT      //without edge transition
			} BE_Point;


void threshold(uchar *img,int img_cols, int img_rows, int threshold)
{
	int r=0, c=0;
	/*find mean of the template*/
	for (r = 0; r < img_rows; ++r)
		for (c = 0; c <img_cols ; ++c)
		{
			img[r*img_cols+c]=  (img[r*img_cols+c]>=threshold)? 0: 255;
		}

}




void clean_boundary(uchar *img, int img_cols, int img_rows)
{
	int r=0, c=0;
	for (r=0; r<img_rows; ++r)
	{
		for (c=0; c< img_cols; ++c)
		{
			if(r==0 || r== img_rows)
			{
				img[r*img_cols+c]=0;
			}
			else
			{
				img[r*img_cols]=0;
				img[r*img_cols+img_cols-1]=0;
			}
		}
	}


}


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
	//first line
	for (c=x_head; c< x_end;c ++)
	{
		front_val=img[(y+y_head )*img_cols+x+c];
		rear_val=img[(y+y_head)*img_cols+c+x+1];
		// check 8-neighbor pixel, if it is boundary pixel, do this
		if(y_head ==0)
		{
			trans_num= img[ y*img_cols+x-1] !=0? trans_num+1: trans_num;
			trans_num= img[ y*img_cols+x+1] !=0? trans_num+1: trans_num;
			break;
		}
		else if(front_val!= rear_val && (front_val & rear_val)==0) //check boundary transition
					trans_num++;
	}
	//last column
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

	//last line
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
	//first column

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



BE_Point check_BEPoints(uchar * sub_img, int img_cols, int img_rows,int x, int y)
{
	int edge_num;
	//skip checking non-edge point
	if(sub_img[y*img_cols+ x]!=0)
		edge_num= get_EdgeTrans(sub_img, img_cols, img_rows,x,y );
	else
		return SINGLEPOINT;
	if (edge_num==1)
	{
		sub_img[y*img_cols+x] =1;
		return ENDPOINT;
	}
	if (edge_num==2)
	{
		sub_img[y*img_cols+x] =2;
		return SEGPOINT;
	}

if (edge_num==3)
	{
		sub_img[y*img_cols+x] =3;
		return BRANCHPOINT;
	}
return SINGLEPOINT;
	// return edge_num==0? SINGLEPOINT : (edge_num ==1 ? ENDPOINT : (edge_num> 2 ? BRANCHPOINT :SEGPOINT )  );
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
*	@return	
*			1: it has 1 endpoint and branchpoint
*			0: it doesn't
*/
int eval_BEPoints(uchar *sub_img, int img_cols, int img_rows)
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
		printf("EP: %d   BP:  %d  \n",endpoint_cnt,branchpoint_cnt );
	if( endpoint_cnt==1 && branchpoint_cnt==1)	return 1;

	return 0;
}



/*
*	@brief thin_edge:
*			this function is to thin the edge (pixel value>0 or 255)
*
*	fixed bugs:
*		1.	boundary problem, you should check the neighbor pixels in the image 
*		rather than the pixel out of the boundary. And the boundary pixels' value of image
*	    can't be edge, which will affect the thinning check 
*		2.	mark problem, the marked pixels should be edge pixels rather than non-edge pixels.
*		Otherwise, the marked non-edge pixels will lead to the infinite loop of checking
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
	// printf("marked_num: %d \n", marked_num);
		//clean marked pixels.
		for (r = 1; r < img_rows-1; ++r)
			for (c = 1; c < img_cols-1; ++c)
			{
				if(sub_img[r*img_cols+c]==127)
				{
					printf("c: %d  r: %d \n",c, r );
					sub_img[r*img_cols+c]=0;
				}					
			}

	}



}






int main(int argc, char const *argv[])
{

	unsigned char a[25]={
				1, 1,1,1,1,
				1, 1,0,0,1,
				1, 0,0,0,1,
				1, 1,0,0,1,
				1, 1,0,1,1
					};
	FILE *fpt;
	// int ROWS, COLS;
	char    header[320];	
	uchar *image;
	int		ROWS,COLS,BYTES;							//used to read original image format
	fpt=fopen("/home/free/Dropbox/ComputerVision/ece431/lab3/parenthood_e_template.ppm","rb");
	fscanf(fpt,"%s %d %d %d\n",header,&COLS,&ROWS,&BYTES);
	printf("COLS: %d,ROWS:  %d\n",COLS,ROWS );
	image=(unsigned char *)calloc(ROWS*COLS,sizeof(unsigned char));		//allocate space for original image
	header[0]=fgetc(fpt);												// read white-space character that separates header 
	fread(image,1,COLS*ROWS,fpt);
	fclose(fpt);

					printf("neighborNum: %d\n", get_neighborNum(a,5,5, 2,2));
					printf("EdgeTrans: %d\n", get_EdgeTrans(a,5,5,2,2));
					threshold(a, 5,5,1);	//threshold and reverse the value of letter since letter is low value representation
					// clean_boundary(a, 5,5);

					for (int i = 0; i < 5; ++i)
					{
						for (int j = 0; j < 5; ++j)
						{
							printf("%d ",a[i*5+j] );
						}
						printf(" \n");
					}



					thin_edge(a, 5,5);
					if (eval_BEPoints(a, 5, 5))
					{
						printf("This is exactly one EP and one BP\n");
					}
					else
					{
						printf("This is not EP and BP\n");
					}

					for (int i = 0; i < 5; ++i)
					{
						for (int j = 0; j < 5; ++j)
						{
							printf("%d ",a[i*5+j] );
						}
						printf(" \n");
					}


					if((fpt=fopen("/home/free/Dropbox/ComputerVision/ece431/lab3/letter_bin.ppm","wb") )!=NULL)
					 {

						fprintf(fpt, "P5 %d %d 255\n",COLS,ROWS);
						fwrite( image,COLS*ROWS,1,fpt);								//write data to file. size 1 byte, buffer COLS*ROWS
						fclose(fpt);
					 }

					for (int i = 0; i < ROWS; ++i)
					{
						for (int j = 0; j < COLS; ++j)
						{
							if(image[i*COLS+j]==0)
								printf("%d   ",image[i*COLS+j] );
							else
								printf("%d ",image[i*COLS+j] );
								
						}
						printf(" \n");
					}

					threshold(image,COLS, ROWS,128 );
					if((fpt=fopen("/home/free/Dropbox/ComputerVision/ece431/lab3/letter_threshold.ppm","wb") )!=NULL)
					 {

						fprintf(fpt, "P5 %d %d 255\n",COLS,ROWS);
						fwrite( image,COLS*ROWS,1,fpt);								//write data to file. size 1 byte, buffer COLS*ROWS
						fclose(fpt);
					 }


					 for (int i = 0; i < ROWS; ++i)
					{
						for (int j = 0; j < COLS; ++j)
						{
							if(image[i*COLS+j]==0)
								printf("%d   ",image[i*COLS+j] );
							else
								printf("%d ",image[i*COLS+j] );
								
						}
						printf(" \n");
					}
					thin_edge(image, COLS, ROWS);
					printf("Thinned image :\n");

					 if((fpt=fopen("/home/free/Dropbox/ComputerVision/ece431/lab3/letter.ppm","wb") )!=NULL)
					 {

						fprintf(fpt, "P5 %d %d 255\n",COLS,ROWS);
						fwrite( image,COLS*ROWS,1,fpt);								//write data to file. size 1 byte, buffer COLS*ROWS
						fclose(fpt);
					 }

					if (eval_BEPoints(image, COLS, ROWS))
					{
						printf("This is exactly one EP and one BP\n");
					}
					else
					{
						printf("This is not EP and BP\n");
					}


					for (int i = 0; i < ROWS; ++i)
					{
						for (int j = 0; j < COLS; ++j)
						{
							// if(image[i*COLS+j]==0)
							// 	printf("%d   ",image[i*COLS+j] );
							// else
								printf("%d ",image[i*COLS+j] );
								
						}
						printf(" \n");
					}


						
	return 0;
}