#include <stdio.h>
#include <stdlib.h>

#define abs(x)		(x)<0? (-(x)):(x)
#define MAXLINE 	100
#define SQR(x) 		(x)*(x)
// #define WINDOW_SIZE 10
#define WINDOW_SIZE 50
#define	TIME_INTERVAL			0.05
// #define X_Threshold 		0.00011	//threshold to detect shake and motion
// #define Y_Threshold 		0.00010
// #define Z_Threshold 		0.00011

// #define X_Threshold 		0.01	//threshold filter most of shake
// #define Y_Threshold 		0.01
// #define Z_Threshold 		0.01
// #define GYRO_PITCH_Threshold 	0.012//threshold to detect shake and motion
// #define GYRO_ROLL_Threshold 	0.012
// #define GYRO_YAW_Threshold 		0.012
#define X_Threshold 		0.015	//threshold to detect motion
#define Y_Threshold 		0.015
#define Z_Threshold 		0.015
#define GYRO_PITCH_Threshold 	0.098
#define GYRO_ROLL_Threshold 	0.17
#define GYRO_YAW_Threshold 		0.198
typedef struct ACCELER
{
	float x;	
	float y;	
	float z;	
} Acceler; 

typedef struct GYRO
{
	float pitch;
	float roll;
	float yaw;
} Gyro;

typedef struct VELOCITY
{
	float x;
	float y;
	float z;
} Veloc;


typedef struct  SENSORWINDOW
{
	float 	time;
	Acceler acc;
	Gyro    gyro;
	Veloc 	veloc;

} SensorWin;

typedef struct PERIOD
{
	float StartTIme;
	float EndTime;	
} MotionPeriod;

typedef enum AXIS{

	ACCE_X	=0,
	ACCE_Y 	=1,
	ACCE_Z	=2,
	PITCH 	=3,
	ROLL 	=4,
	YAW 	=5,
	VELOC_X	=6,
	VELOC_Y 	=7,
	VELOC_Z 	=8
} Axis;


// function procotypes
void print_AcceGyro(SensorWin *ptr);
void update_Win(SensorWin *ptr, FILE *fpt);
float get_var(SensorWin *ptr,Axis axis_type);
float get_exp(SensorWin *ptr, Axis axis_type);
float get_rotation(float angular_speed, float time_eclipse);
void GaussianFilter(SensorWin *ptr);
float get_FinalVelocity(float accel, float prev_velocity, float time);
float get_AvgVelocity(float prev_velocity, float final_velocity);
float get_SamplingDistance(float avg_velocity, float time );
void get_CumulDistance(float *prev_velocity, float new_acc, float time, float *cumulative_distance );

int main(int argc, char const *argv[])
{
	FILE *fpt;
	char line[MAXLINE];
	// float time,x,y,z, pitch, roll, yaw, variance;
	int window_index=0;
	SensorWin *Win_pt, *AcceGyro_Win; 
	float final_velocity[3]={0,0,0}, cumul_distanceX=0,cumul_distanceY=0,cumul_distanceZ=0;
	float total_rotation_pitch=0,total_rotation_roll=0,total_rotation_yaw=0;
	int rest_flag=0;
	float variance=0;
	unsigned char AxisXMotionFlag=0,AxisYMotionFlag=0,AxisZMotionFlag=0,PitchMotionFlag=0,RollMotionFlag=0,YawMotionFlag=0;
	MotionPeriod period[7];

	if ((fpt=fopen("acc_data.txt","r"))==NULL)
		exit(0);
	fscanf(fpt,"%s\t%s\t%s\t%s\t%s\t%s\t%s\t\t\t\t\t\t\t", &line[0],&line[10],&line[20],&line[30],&line[40],&line[50],&line[60]);
	//allcate space for window 
	AcceGyro_Win= (SensorWin *) malloc(WINDOW_SIZE*sizeof(SensorWin));
	Win_pt=AcceGyro_Win;
	// initialize window data
	for (window_index = 0; window_index < WINDOW_SIZE-1; ++window_index)
	{
		update_Win(AcceGyro_Win,fpt);
	}
	
	// read data
	while(!feof(fpt)){
	// update window data
	update_Win(AcceGyro_Win,fpt);

	/**********************check period******************/
		// find variance of velocity along X axis
		/**************************acce X channel******************/
		variance= get_var(  AcceGyro_Win, VELOC_X );
		// printf("time: %f\tvariance: %f\n", AcceGyro_Win[WINDOW_SIZE/2].time ,variance);

		if (variance>= X_Threshold)
		{
			if (AxisXMotionFlag==0){
				// detected accelerometer X axis motion
				period[0].StartTIme= AcceGyro_Win[WINDOW_SIZE/2].time;
				AxisXMotionFlag=1;
			}
			// consider the status of the last period
			if (feof(fpt))
			{
				period[0].EndTime= AcceGyro_Win[WINDOW_SIZE-1].time;
				AxisXMotionFlag=0;
				printf("x\t%f - %f\t %f\n",period[0].StartTIme, period[0].EndTime, cumul_distanceX );
				cumul_distanceX=0;

			}
			else{
			// update velocity and distance
			get_CumulDistance(&final_velocity[0], AcceGyro_Win[WINDOW_SIZE/2].acc.x, TIME_INTERVAL, &cumul_distanceX );
			}
		}
		else {
			if(AxisXMotionFlag==1)
			{
				//period ended
				period[0].EndTime= AcceGyro_Win[WINDOW_SIZE/2].time;
				AxisXMotionFlag=0;
				printf("x\t%f - %f\t %f\n",period[0].StartTIme, period[0].EndTime, cumul_distanceX );
				cumul_distanceX=0;
				final_velocity[0]=0;//reset data
			}
		}


		/**************************acce Y channel******************/
		variance= get_var( AcceGyro_Win, VELOC_Y );
		if (variance>= Y_Threshold)
		{
			if (AxisYMotionFlag==0){
				period[1].StartTIme= AcceGyro_Win[WINDOW_SIZE/2].time;
				AxisYMotionFlag=1;// detected accelerometer X axis motion
			}
			// consider the status of the last period
			if (feof(fpt))
			{
				period[1].EndTime= AcceGyro_Win[WINDOW_SIZE-1].time;
				AxisYMotionFlag=0;
				printf("y\t%f - %f\t %f\n",period[1].StartTIme, period[1].EndTime, cumul_distanceY );
				cumul_distanceY=0;
			}
			else{
			// update velocity and distance
			get_CumulDistance(&final_velocity[1], AcceGyro_Win[WINDOW_SIZE/2].acc.y, TIME_INTERVAL, &cumul_distanceY );
			}
		}
		else{
			if(AxisYMotionFlag==1)
			{
			period[1].EndTime= AcceGyro_Win[WINDOW_SIZE/2].time;
			AxisYMotionFlag=0;
			printf("y\t%f - %f\t %f\n",period[1].StartTIme, period[1].EndTime, cumul_distanceY );
			cumul_distanceY=0;
			final_velocity[1]=0;//reset data	
			}
		} 

		/**************************aaccelerometer Z axis******************/
		variance= get_var(  AcceGyro_Win, VELOC_Z );
		// printf("time: %f\tvariance: %f\n", AcceGyro_Win[WINDOW_SIZE/2].time ,variance);
		if (variance>= Z_Threshold)
		{
			if (AxisZMotionFlag==0){
				period[2].StartTIme= AcceGyro_Win[WINDOW_SIZE/2].time;
				AxisZMotionFlag=1;// detected accelerometer X axis motion
			}
			// consider the status of the last period
			if (feof(fpt))
			{
				period[2].EndTime= AcceGyro_Win[WINDOW_SIZE-1].time;
				AxisZMotionFlag=0;
				printf("z\t%f - %f\t %f\n",period[2].StartTIme, period[2].EndTime,cumul_distanceZ );
				cumul_distanceZ=0;
			}
			else{
			// update velocity and distance
			get_CumulDistance(&final_velocity[2], AcceGyro_Win[WINDOW_SIZE/2].acc.z, TIME_INTERVAL, &cumul_distanceZ );
			}
		}
		else{
		 if(AxisZMotionFlag==1)
		{
			period[2].EndTime= AcceGyro_Win[WINDOW_SIZE/2].time;
			AxisZMotionFlag=0;
			final_velocity[2]=0;//reset data
			printf("z\t%f - %f\t %f\n",period[2].StartTIme, period[2].EndTime,cumul_distanceZ );
			cumul_distanceZ=0;
		}
		}

		/**************************gyroscope Z axis******************/
		variance= get_var(  AcceGyro_Win, PITCH );
		 // printf("time: %f\tvariance: %f\n", AcceGyro_Win[WINDOW_SIZE/2].time ,variance);
		if (variance>= GYRO_PITCH_Threshold )
		{
			if (PitchMotionFlag==0){
				period[3].StartTIme= AcceGyro_Win[WINDOW_SIZE/2].time;
				PitchMotionFlag=1;// detected accelerometer X axis motion
			}

			// consider the status of the last period
			if (feof(fpt))
			{
				period[3].EndTime= AcceGyro_Win[WINDOW_SIZE-1].time;
				PitchMotionFlag=0;
				printf("Pitch\t%2.6f - %2.6f\t %f\n",period[3].StartTIme, period[3].EndTime,total_rotation_pitch );
				total_rotation_pitch=0;
			}
			else{

			total_rotation_pitch+=abs(get_rotation(AcceGyro_Win[WINDOW_SIZE/2].gyro.pitch, TIME_INTERVAL));
			}
			
		}
		else{
		  if(PitchMotionFlag==1)
		{
			period[3].EndTime= AcceGyro_Win[WINDOW_SIZE/2].time;
			PitchMotionFlag=0;
			printf("Pitch\t%2.6f - %2.6f\t %f\n",period[3].StartTIme, period[3].EndTime,total_rotation_pitch );
			total_rotation_pitch=0;
		}
		}



		/**************************gyroscope roll axis******************/
		variance= get_var(AcceGyro_Win, ROLL );
		if (variance>= GYRO_ROLL_Threshold)
		{
			if (RollMotionFlag ==0){
				period[4].StartTIme= AcceGyro_Win[WINDOW_SIZE/2].time;
				RollMotionFlag=1;// detected accelerometer X axis motion
			}
			// consider the status of the last period
			if (feof(fpt))
			{
				period[5].EndTime= AcceGyro_Win[WINDOW_SIZE-1].time;
				RollMotionFlag=0;
				printf("Roll\t%2.6f - %2.6f\t %f\n",period[4].StartTIme, period[4].EndTime, total_rotation_roll );
				total_rotation_roll=0;
			}
			else{
				
			total_rotation_roll+=abs(get_rotation(AcceGyro_Win[WINDOW_SIZE/2].gyro.roll, TIME_INTERVAL));
			}
		}
		else{
			 if(RollMotionFlag==1)
			{
				period[4].EndTime= AcceGyro_Win[WINDOW_SIZE/2].time;
				RollMotionFlag=0;
				printf("Roll\t%2.6f - %2.6f\t %f\n",period[4].StartTIme, period[4].EndTime, total_rotation_roll );
				total_rotation_roll=0;
			}
		}


		/**************************gyroscope yaw axis******************/
		variance= get_var(  AcceGyro_Win, YAW );
		if (variance>= GYRO_YAW_Threshold)
		{
			if (YawMotionFlag ==0){
				period[5].StartTIme= AcceGyro_Win[WINDOW_SIZE/2].time;
				YawMotionFlag=1;// detected accelerometer X axis motion
			}
			// consider the status of the last period

			if (feof(fpt))
			{
				period[5].EndTime= AcceGyro_Win[WINDOW_SIZE-1].time;
				YawMotionFlag=0;
				printf("Yaw\t%2.6f - %2.6f\t %f\n",period[5].StartTIme, period[5].EndTime, total_rotation_yaw );
				total_rotation_yaw=0;
			}
			else{
			// calculate total rotation
			total_rotation_yaw+=abs(get_rotation(AcceGyro_Win[WINDOW_SIZE/2].gyro.yaw, TIME_INTERVAL));
			}
		}
		else{
			 if(YawMotionFlag==1)
			{
				period[5].EndTime= AcceGyro_Win[WINDOW_SIZE/2].time;
				YawMotionFlag=0;
				printf("Yaw\t%2.6f - %2.6f\t %f\n",period[5].StartTIme, period[5].EndTime, total_rotation_yaw );
				total_rotation_yaw=0;
			}

		}


		/**********************rest period**********************/
		if (AxisXMotionFlag || AxisYMotionFlag || AxisZMotionFlag || PitchMotionFlag || RollMotionFlag || YawMotionFlag )
		{

			if(rest_flag==1){
			period[6].EndTime=AcceGyro_Win[WINDOW_SIZE/2].time;
			printf("Rest\t%2.6f - %2.6f\n",period[6].StartTIme, period[6].EndTime );
			rest_flag=0;
			}

		}
		else{
			// start record time of rest
			if (rest_flag==0){
				rest_flag=1;
				period[6].StartTIme= AcceGyro_Win[WINDOW_SIZE/2].time;
			}
			// consider the status of the last period
			if (feof(fpt))
			{
			period[6].EndTime=AcceGyro_Win[WINDOW_SIZE-1].time;
			printf("Rest\t%2.6f - %2.6f\n",period[6].StartTIme, period[6].EndTime );
			rest_flag=0;
			}

		}
	}
	free(AcceGyro_Win);
	AcceGyro_Win=Win_pt=NULL;
	fclose(fpt);
	return 0;
}


/*
*	@brief update_win
*		This function is to move the window along time line to update 
*		data in the window, which stores the info of acceleration, velocity and gyroscope data
*			
*	@para *ptr
*		window storing acc and gyro data
*	@para *fpt:
*		FILE pointer to read data
*/
void update_Win(SensorWin *ptr, FILE *fpt)
{
	int window_index;
	// left shift data
	for (window_index = 0; window_index < WINDOW_SIZE-1; ++window_index)
	{


		ptr[window_index].time=ptr[window_index+1].time;
		ptr[window_index].acc.x= ptr[window_index+1].acc.x;
		ptr[window_index].acc.y= ptr[window_index+1].acc.y;
		ptr[window_index].acc.z= ptr[window_index+1].acc.z;

		ptr[window_index].veloc.x= ptr[window_index+1].veloc.x;
		ptr[window_index].veloc.y= ptr[window_index+1].veloc.y;
		ptr[window_index].veloc.z= ptr[window_index+1].veloc.z;

		ptr[window_index].gyro.pitch=ptr[window_index+1].gyro.pitch;
		ptr[window_index].gyro.roll=ptr[window_index+1].gyro.roll;
		ptr[window_index].gyro.yaw=ptr[window_index+1].gyro.yaw;
		
	}
	// obtain new acc_data at next moment
	fscanf(fpt,"%f\t%f\t%f\t%f\t%f\t%f\t%f\t",&(ptr[WINDOW_SIZE-1].time), 
													&(ptr[WINDOW_SIZE-1].acc.x),
													&(ptr[WINDOW_SIZE-1].acc.y),
													&(ptr[WINDOW_SIZE-1].acc.z),
													&(ptr[WINDOW_SIZE-1].gyro.pitch),
													&(ptr[WINDOW_SIZE-1].gyro.roll),
													&(ptr[WINDOW_SIZE-1].gyro.yaw ));
	ptr[WINDOW_SIZE-1].acc.z+=0.985;
	ptr[WINDOW_SIZE-1].veloc.x= (ptr[WINDOW_SIZE-2].veloc.x)+(ptr[WINDOW_SIZE-1].acc.x * TIME_INTERVAL);
	ptr[WINDOW_SIZE-1].veloc.y= (ptr[WINDOW_SIZE-2].veloc.y)+(ptr[WINDOW_SIZE-1].acc.y * TIME_INTERVAL);
	ptr[WINDOW_SIZE-1].veloc.z= (ptr[WINDOW_SIZE-2].veloc.z)+(ptr[WINDOW_SIZE-1].acc.z * TIME_INTERVAL);

}


/*
*	@brief print_AcceGyro
*		print window info
*
*/
void print_AcceGyro(SensorWin *ptr)
{
	printf("%f\t%f\t%f\t%f\t%f\t%f\t%f\t \n",ptr->time,
											 ptr->acc.x,
											 ptr->acc.y,
											 ptr->acc.z,
											 ptr->gyro.pitch,
											 ptr->gyro.roll,
											 ptr->gyro.yaw	);
}




/*
*	@brief get_var
*			The function to get varicnce of the data along a axis in the window
*	@para *ptr
*			window array
*	@para axis_type
*			axis of the variance
*
*
*/
float get_var(SensorWin *ptr,Axis axis_type)
{
	float var=0;
	int window_index;
	float expectation= get_exp(ptr, axis_type );

	switch(axis_type)
		{
			case VELOC_X:
			for (window_index = 0,var=0; window_index < WINDOW_SIZE; ++window_index)
				{
					var+=SQR(ptr[window_index].veloc.x- expectation);
				}
				var=var/WINDOW_SIZE;
			break;

			case VELOC_Y:
			for (window_index = 0,var=0; window_index < WINDOW_SIZE; ++window_index)
				{
					var+=SQR(ptr[window_index].veloc.y- expectation);
				}
				var=var/WINDOW_SIZE;
			break;

			case VELOC_Z:
			for (window_index = 0,var=0; window_index < WINDOW_SIZE; ++window_index)
				{
					var+=SQR(ptr[window_index].veloc.z- expectation);
				}
				var=var/WINDOW_SIZE;
			break;

			case ACCE_X:
				for (window_index = 0,var=0; window_index < WINDOW_SIZE; ++window_index)
				{
					var+=SQR(ptr[window_index].acc.x- expectation);
				}
				var=var/WINDOW_SIZE;
			
				break;

			case ACCE_Y:
				for (window_index = 0,var=0; window_index < WINDOW_SIZE; ++window_index)
				{
					var+=SQR(ptr[window_index].acc.y- expectation);
				}
				var=var/WINDOW_SIZE;
			
				break;
			case ACCE_Z:
				for (window_index = 0,var=0; window_index < WINDOW_SIZE; ++window_index)
				{
					var+=SQR(ptr[window_index].acc.z- expectation);
				}
				var=var/WINDOW_SIZE;
				break;
			case PITCH:
				for (window_index = 0,var=0; window_index < WINDOW_SIZE; ++window_index)
				{
					var+=SQR(ptr[window_index].gyro.pitch- expectation);
				}
				var=var/WINDOW_SIZE;
			

			break;
			case ROLL:
			for (window_index = 0; window_index < WINDOW_SIZE; ++window_index)
				{
					var+=SQR(ptr[window_index].gyro.roll- expectation);
				}
				var=var/WINDOW_SIZE;
			break;
			case YAW:
			for (window_index = 0; window_index < WINDOW_SIZE; ++window_index)
				{
					var+=SQR(ptr[window_index].gyro.yaw- expectation);
				}
				var=var/WINDOW_SIZE;
			break;
			

			default: break;
		}

	
	return var;
}


/*
*	@brief get_exp
*			The function to calculate expectation of window along an axis
*	@para *ptr
*			window storing info
*	@para axis_type
*			axis type
*/

float get_exp(SensorWin *ptr, Axis axis_type)
{
	float exp=0;
	int window_index=0;
	switch(axis_type)
	{
		case VELOC_X:
		// find corresponding velocity of acceleration
			for (window_index = 0; window_index < WINDOW_SIZE; ++window_index)
				exp+=ptr[window_index].veloc.x;
			exp/=WINDOW_SIZE;
			break;

		case VELOC_Y:
				for (window_index = 0; window_index < WINDOW_SIZE; ++window_index)
				exp+=ptr[window_index].veloc.y;
			exp/=WINDOW_SIZE;
			break;
		case VELOC_Z:
			for (window_index = 0; window_index < WINDOW_SIZE; ++window_index)
				exp+=ptr[window_index].veloc.z;
			exp/=WINDOW_SIZE;
			break;

		case ACCE_X:
			for (window_index = 0; window_index < WINDOW_SIZE; ++window_index)
				exp+=ptr[window_index].acc.x;
			exp/=WINDOW_SIZE;
			break;
		case ACCE_Y:
			for (window_index = 0; window_index < WINDOW_SIZE; ++window_index)
				exp+=ptr[window_index].acc.y;
			exp/=WINDOW_SIZE;
			break;
		case ACCE_Z:
			for (window_index = 0; window_index < WINDOW_SIZE; ++window_index)
				exp+=ptr[window_index].acc.z;
			exp/=WINDOW_SIZE;
			break;
		case PITCH:
			for (window_index = 0; window_index < WINDOW_SIZE; ++window_index)
				exp+=ptr[window_index].gyro.pitch;
			exp/=WINDOW_SIZE;
			break;
		case ROLL:
			for (window_index = 0; window_index < WINDOW_SIZE; ++window_index)
				exp+=ptr[window_index].gyro.roll;
			exp/=WINDOW_SIZE;
			break;
		case YAW:
			for (window_index = 0; window_index < WINDOW_SIZE; ++window_index)
				exp+=ptr[window_index].gyro.yaw;
			exp/=WINDOW_SIZE;
			break;
		default:break;
	}
return exp;
	
}

/*
*	@brief get_rotation
*		calculate the rotation at a moment
*
*
*/
float get_rotation(float angular_speed, float time_eclipse)
{
	return (angular_speed*time_eclipse);
}



/*
*	@brief get_Finalvelocity
*		calculate the final velocity of the last moment
*
*
*/
float get_FinalVelocity(float accel, float prev_velocity, float time)
{
	return (prev_velocity+ (time*accel));
}

/*
*	@brief get_rotation
*		calculate the average velocity 
*
*
*/
float get_AvgVelocity(float prev_velocity, float final_velocity)
{
	return (prev_velocity+ final_velocity)/2;
}

/*
*	@brief get_rotation
*		calculate the distance each 0.05s
*
*
*/
float get_SamplingDistance(float avg_velocity, float time )
{
	return (avg_velocity*time);
}





/*
*	@brief get_CumculDistance
*		calculate the cumulative distance of a period.
*	@para *prev_velocity
*		the velocity of the last moment and update the velocity
*	@para new_acc
*		the current acceleration
*	@para time
*		time interval
*
*	@para *cumulaive_distance
*		return the cumulative distance value of a period
*/
void get_CumulDistance(float *prev_velocity, float new_acc, float time, float *cumulative_distance )
{
	float final_velocity=0, avg_velocity=0;
	final_velocity= get_FinalVelocity(new_acc, *prev_velocity, time); 
	avg_velocity=get_AvgVelocity(*prev_velocity, final_velocity);
	*cumulative_distance+=get_SamplingDistance(avg_velocity,time);	
	*prev_velocity=final_velocity;//update previous velocity
}









/*
*
*
*
*
*
*/

void GaussianFilter(SensorWin *ptr)
{
	int window_index,i;
	float sum_x=0, sum_y=0,sum_z=0;
	char GaussFilter[]={ 2,4,2};
	// int filter_windowsize=5;
	for (window_index = 1; window_index < WINDOW_SIZE-1; ++window_index)	
	{
		sum_z=0;
		sum_x=0;
		sum_y=0;
		for ( i = -1; i <= 1; ++i)
		{
			sum_x+=GaussFilter[i+1]*ptr[window_index+i].acc.x;
			sum_y+= GaussFilter[i+1]*ptr[window_index+i].acc.y;
			sum_z+= GaussFilter[i+1]*ptr[window_index+i].acc.z;
		}
		ptr[window_index].acc.x=sum_x/8;
		ptr[window_index].acc.y=sum_y/8;
		ptr[window_index].acc.z=sum_z/8;
	}

	for (window_index = 1; window_index < WINDOW_SIZE-1; ++window_index)	
	{
		sum_z=0;
		sum_x=0;
		sum_y=0;
		for ( i = -1; i <= 1; ++i)
		{
			sum_x+= GaussFilter[i+1]*ptr[window_index+i].veloc.x;
			sum_y+=GaussFilter[i+1]* ptr[window_index+i].veloc.y;
			sum_z+=GaussFilter[i+1]* ptr[window_index+i].veloc.z;
		}
		ptr[window_index].veloc.x=sum_x/8;
		ptr[window_index].veloc.y=sum_y/8;
		ptr[window_index].veloc.z=sum_z/8;
	}

	for (window_index = 1; window_index < WINDOW_SIZE-1; ++window_index)	
	{
		sum_z=0;
		sum_x=0;
		sum_y=0;
		for ( i = -1; i <= 1; ++i)
		{
			sum_x+= GaussFilter[i+1]*ptr[window_index+i].gyro.pitch;
			sum_y+= GaussFilter[i+1]*ptr[window_index+i].gyro.roll;
			sum_z+= GaussFilter[i+1]*ptr[window_index+i].gyro.yaw;
		}
		ptr[window_index].gyro.pitch =sum_x/8;
		ptr[window_index].gyro.roll =sum_y/8;
		ptr[window_index].gyro.yaw =sum_z/8;
	}
		
}