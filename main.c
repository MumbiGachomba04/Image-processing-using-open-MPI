/**
 *
 * CENG342 Project-1
 *
 * Histogram equalization 
 *
 * Usage:  main <input.jpg> <output.jpg> 
 * 
 * @version 1.0, 02 April 2022
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image.h"
#include "stb_image_write.h"
#define CHANNEL_NUM 1

void par_histogram_equalizer(uint8_t* local_image,int width, int height);

int main(int argc,char* argv[]) 
{	
    MPI_Init(&argc,&argv);
    int num_proc, rank;
    MPI_Comm_size(MPI_COMM_WORLD,&num_proc);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
	
    int width, height, bpp;
    uint8_t* rgb_image;
    int *send_cnt = (int*)malloc(sizeof(int) * num_proc); // size of image block sent to each process
    uint8_t* loc_image;
	if(rank ==0){
		// Reading the image in grey colors	
		rgb_image = stbi_load(argv[1], &width, &height, &bpp, CHANNEL_NUM);
		printf("Width: %d  Height: %d \n",width,height);
		printf("Input: %s , Output: %s  \n",argv[1],argv[2]);
		
		for(int i=0;i<num_proc; i++){
		   send_cnt[i] = height / num_proc;	
		   if( i == num_proc-1){
			  send_cnt[i]  +=  (height  % (num_proc)) ; // add any remainder block to the last process
		   }
		}
		loc_image = (uint8_t*) malloc( sizeof(uint8_t) * send_cnt[0] * width );
                memcpy(loc_image, rgb_image, send_cnt[0] * width * sizeof(uint8_t)); // assign portion of 
		MPI_Bcast(&width,1 ,MPI_INT,0,MPI_COMM_WORLD); // broadcast width to all processes
		MPI_Bcast(send_cnt ,num_proc ,MPI_INT,0,MPI_COMM_WORLD); // broadcast send_count Array to all processes
		int cum_height = send_cnt[0];
		for(int i=1; i< num_proc;i++){
		   //send rgb_image	
		   MPI_Send(&rgb_image[cum_height * width], send_cnt[i] * width ,MPI_UINT8_T,i,123,MPI_COMM_WORLD);
           cum_height+=send_cnt[i];
		}
	}
	else{		    
		MPI_Bcast(&width, 1 , MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(send_cnt ,num_proc ,MPI_INT,0,MPI_COMM_WORLD);
		printf("Width: %d  Height: %d \n",width,send_cnt[rank]);
		loc_image = (uint8_t*) malloc( sizeof(uint8_t) * send_cnt[rank] * width );
		MPI_Recv(loc_image, send_cnt[rank] * width , MPI_UINT8_T,0,123,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	}
	// start the timer
	double time1= MPI_Wtime();	
	
	par_histogram_equalizer(loc_image, width, send_cnt[rank]);
	
	int *recv_counts =(int*) malloc( sizeof(int) * num_proc );
        int *disp= (int*)calloc(num_proc,sizeof(int)); 
   	
   	for(int i=0; i< num_proc;i++){
   		recv_counts[i] = send_cnt[i]*width;
         	disp[i+1] = disp[i] + send_cnt[i] * width ;
         	}
	
	MPI_Gatherv(loc_image,send_cnt[rank] * width,MPI_UINT8_T, rgb_image, recv_counts,disp, MPI_UINT8_T,0 ,MPI_COMM_WORLD);

	double time2= MPI_Wtime();	
	printf("Elapsed time: %lf \n",time2-time1);	
	
	// Storing the image 
	if (rank ==0)
		{stbi_write_jpg(argv[2], width, height, CHANNEL_NUM, rgb_image, 100);}

	MPI_Finalize();
    return 0;
}


void par_histogram_equalizer(uint8_t* local_image,int width, int local_height)
{
   int *hist = (int*)calloc(256,sizeof(int));
   int *hist_final = (int*)calloc(256,sizeof(int));
   int num_proc;
   MPI_Comm_size(MPI_COMM_WORLD,&num_proc);
   
   double size = width * local_height;
    
	for(int i=0; i<local_height; i++){
		for(int j=0; j<width; j++){
			hist[local_image[i*width + j]]++;
		}
	}
     
    MPI_Allreduce(hist, hist_final, 256, MPI_INT, MPI_SUM, MPI_COMM_WORLD);
  
     //cumulative sum for histogram values
    int cumhistogram[256];
	cumhistogram[0] = (hist_final[0]/num_proc) ;
    for(int i = 1; i < 256; i++)
    {
        cumhistogram[i] = (hist_final[i]/num_proc) + cumhistogram[i-1];
    }    
	
    int alpha[256];
    for(int i = 0; i < 256; i++)
    {
        alpha[i] = round((double)cumhistogram[i] * (255.0/size));
    }
			
    // histogram equlized image		
    for(int y = 0; y <local_height ; y++){
        for(int x = 0; x < width; x++){
            local_image[y*width + x] = alpha[local_image[y*width + x]];
		}
	}


};

	

