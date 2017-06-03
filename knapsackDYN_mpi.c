
/*** 
 * Part: MPI Implementation for 0-1 Knapsack problem 
 * MPI Dynamic Programming based solution 
 */

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
#include <string.h>

// A utility function that returns maximum of two integers
int max(int a, int b) { return (a > b)? a : b; }


// Returns the maximum value that can be put in a knapsack of capacity W
static int knapSack(long int W, long int N, int wt[], int val[], double* timeSpent)
{
	
	long int i, w;	
    int result=0;     
 	int  size,rank,slave;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); 
     
    
    //initial time
	double start=MPI_Wtime();

     /* master initializes work*/
	if (rank==0){
        //define the matrix.
        int K[1][W+3] ;
		int i,count;
        int m[W+3];
        memset(K[0], 0, (W+3)*sizeof(int));	
		
        
		for(i=1; i<=N ; i++)
		{			
			for (slave=1;slave<size ;slave++)  
     			MPI_Send((void*)K[0], W+1, MPI_INT, slave, 1, MPI_COMM_WORLD);               

			
			for(count = 1; count< size; count++)
			{     		
                //recive the vector
				MPI_Recv((void*)m, W+3, MPI_INT, MPI_ANY_SOURCE, 10, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                //allocate in the matrix.
                memcpy(K[0]+m[W+1], m+m[W+1], sizeof(int) * (m[W+2]-m[W+1])+sizeof(int));                
            }		
		}	
        
        result=K[0][W];
	}
    /* work done by slaves*/
	else{    
        
        int  Kp[2][W+3];
     	int k,portion,low_bound,upper_bound;       
        
		for(k=1;k<=N;k++)
     	{	
        	i=1;
            //receive the message from the master
			MPI_Recv((void*)Kp[i-1], W+1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
         	
            // calculate portion without master
            portion = (W/ (size - 1)); 
            low_bound = (rank - 1) * portion;
            //if rows of [K] cannot be equally divided among slaves
            if (((rank + 1) == size) && ((W% (size - 1)) != 0)) {
                //last slave gets all the remaining rows
                upper_bound =W; 
            } else {
                //rows of [K] are equally divisable among slaves
                upper_bound = low_bound + portion; 
            }

            //printf("low_bound %d  upper_bound %d \n",low_bound,upper_bound);

            // each core only computes a part of the data.
            
	  		for (w =low_bound; w <= upper_bound; w++)
			{
				if (i==0 || w==0)
					Kp[i][w] = 0;
				else if (wt[k-1] <= w)
					Kp[i][w] = max(val[k-1] + Kp[i-1][w-wt[k-1]], Kp[i-1][w]);
				else
					Kp[i][w] = Kp[i-1][w];
            }
            //Initial and final 'w' per core
            Kp[i][W+1]=low_bound;
            Kp[i][W+2]=upper_bound;
            //Send a message 
            MPI_Send((void*)Kp[i], W+3, MPI_INT, 0, 10, MPI_COMM_WORLD);   		 
        }         
	}
    //compute the time spent in the parallelization
    (*timeSpent)=MPI_Wtime()-start;

    MPI_Finalize();   

    return result;

}

int main(int argc, char **argv){

    FILE *test_file;
    double start_time, elapsed_time; /* for time measurement */

    int *val, *wt;   // width and cost values
    long int Nitems; // Number of items
    long int Width;  // Max. load to carry
    long int cont;    // counter
    double tpivot1=0,tpivot2=0,tpivot3=0; //time counting
    struct timeval tim;

	if (argc!=2) {
	  printf("\n\nError, mising parameters:\n");
	  printf("format: test_file \n");	 
	  return 1;
	}
    

    //Capture first token time - init execution
    gettimeofday(&tim, NULL);
    tpivot1 = tim.tv_sec+(tim.tv_usec/1000000.0);
    
	if (!(test_file=fopen(argv[1],"r"))) {
	  printf("Error opening Value file: %s\n",argv[1]);
	  return 1;
	}

    //Reading number of items and Maximum width
	fscanf(test_file,"%ld %ld\n",&Nitems, &Width);
    
    //printf("%ld\n",Nitems*sizeof(int));
	double timeSpent=0;
	val = (int *)malloc(Nitems*sizeof(int)); //values for each element
	 wt = (int *)malloc(Nitems*sizeof(int)); //width  for each element
    
    //Reading value and width for each element
	for (cont=0;cont<Nitems;cont++){
	  fscanf(test_file,"%d,%d\n",&val[cont],&wt[cont]);
	}

     //    printf("\n\n ---- Results TCapacity:items:Tcost:toptim:ttime ----\n\n");
    gettimeofday(&tim, NULL);
    tpivot2 = (tim.tv_sec+(tim.tv_usec/1000000.0));
    //printf("HOLA2\n");
    
    //int K[Nitems+1][Width+1] ;
    int result=  knapSack(Width,Nitems, wt, val,&timeSpent);
      // we only print the result for the core with result greater than 0 (core 0).
      if (result>0){
         printf("%ld:%ld:%d", Width, Nitems, result);

    	gettimeofday(&tim, NULL);

    	tpivot3 = (tim.tv_sec+(tim.tv_usec/1000000.0));
   		 printf(":%.6lf:%.6lf\n", timeSpent,tpivot3-tpivot1);
   }

	free(val);
	free(wt);
	
	fclose(test_file);

	return 0;
}