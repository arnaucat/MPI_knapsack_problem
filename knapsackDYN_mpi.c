// A Dynamic Programming based solution for 0-1 Knapsack problem
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <mpi.h>
MPI_Status status,status2; // store status of a MPI_Recv
MPI_Request request_snd,request_rcv; //capture request of a MPI_Isend

// A utility function that returns maximum of two integers
int max(int a, int b) { return (a > b)? a : b; }

// Returns the maximum value that can be put in a knapsack of capacity W
static void knapSack(long int W, long int N, int wt[], int val[],int rank,int size,int *K[])
{

	long int i, w,j;
	int rcv_rank = 0; //k-1)%size;// receive data
 	int snd_rank = 1; //(rank+1)%size;// send data   
 	int low_bound; //low bound of the number of rows  allocated to a slave
	int upper_bound; //upper bound of the number of rows  allocated to a slave
	int portion; //portion of the number of rows allocated to a slave

	// Build table K[][] in bottom up manner
	for (i = 0; i <= N; i++)
	{

		//printf("iteracion : %ld \n",i);	
		//rcv_rank = (rank-1)%size;   //rank to receive data
 	  	//snd_rank = (rank+1)%size; // rank to send data

		if (rank == 0) 
		 	rcv_rank=size-1;
	    else            
	    	rcv_rank=rank-1;
	    //Define the destiny the rigth neighbour
	    if(rank==size-1) 
	    	snd_rank=0;
	    else                 
	    	snd_rank=rank+1;

 	  	/*if(rcv_rank < 0)
 	  	{
 	  		rcv_rank = size-1;
 	  	}*/	  	
		
	    

	    portion = (W/ (size )); // calculate portion without master
		low_bound = (rank) * portion;
		if (((rank +1) == size) && ((W% (size - 1)) != 0)) {//if rows of [A] cannot be equally divided among slaves
			upper_bound =W; //last slave gets all the remaining rows
		} else {
			upper_bound = low_bound + portion; //rows of [A] are equally divisable among slaves
		}		

		for (w = low_bound; w < upper_bound; w++)
		{
			if (i==0 || w==0)
				K[i][w] = 0;
			else if (wt[i-1] <= w)
				K[i][w] = max(val[i-1] + K[i-1][w-wt[i-1]], K[i-1][w]);
			else
				K[i][w] = K[i-1][w];

			printf("position %ld iteracion %ld low_bound %d upper_bound %d core %d calculo : %i \n",i,w, low_bound,upper_bound,rank,K[i][w]);				
						
		}	   
		
		//printf("envia a : %d iteracion %ld \n",snd_rank,i);	
		MPI_Isend(K[i],upper_bound, MPI_INT, snd_rank, 0, MPI_COMM_WORLD,&request_snd);	
	
		
		if(rank==0 && rcv_rank==0)
		{
			MPI_Recv(K[i], W , MPI_INT, rcv_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);	
		}

		if(i > 0)
		{
			//printf("Recibe de : %d \n",rcv_rank);	
			//printf("recibe iteracion : %ld rcv_rank %d snd_rank %d \n",i,rcv_rank,snd_rank);	
			MPI_Recv(K[i-1], W , MPI_INT, rcv_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);		
			MPI_Wait(&request_snd, &status2);
		}			
			
		
	}
	if(rank == 0)
	{						
		MPI_Recv(K[i-1], W, MPI_INT, rcv_rank, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);	
		MPI_Wait(&request_snd, &status2);		
	}  
	

}

int main(int argc, char **argv){

    FILE *test_file;
    
    int *val, *wt;   // width and cost values
    long int Nitems; // Number of items
    long int Width;  // Max. load to carry
    long int cont;    // counter
    double tpivot1=0,tpivot2=0,tpivot3=0; //time counting
    struct timeval tim;
    int **K;
    int rank; //process rank
	int size; //number of processes
	int i,w;
   

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

		
	val = (int *)malloc(Nitems*sizeof(int)); //values for each element
	wt = (int *)malloc(Nitems*sizeof(int)); //width  for each element
    
    //Reading value and width for each element
	for (cont=0;cont<Nitems;cont++){
	  fscanf(test_file,"%d,%d\n",&val[cont],&wt[cont]);
	}



	K = (int**) malloc((Nitems+1)*sizeof(int*));
	for (i=0;i<=Nitems;i++)
		K[i] = (int*)malloc((Width+1)*sizeof(int));	


	MPI_Init(&argc, &argv); //initialize MPI operations
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); //get the rank
    MPI_Comm_size(MPI_COMM_WORLD, &size); //get number of processes

    //gettimeofday(&tim, NULL);
    tpivot2 = MPI_Wtime();    
    knapSack(Width,Nitems, wt, val,rank,size,K);
    MPI_Barrier(MPI_COMM_WORLD);
    
	if(rank == 0)
	{
		printf("%ld:%ld:%d",Width,Nitems, K[Nitems][Width] );
	    gettimeofday(&tim, NULL);
	    tpivot3 = (tim.tv_sec+(tim.tv_usec/1000000.0));
	 	printf(":%.6lf:%.6lf\n", tpivot3-tpivot2,tpivot3-tpivot1); 


	 	for (i = 0; i <= Nitems; i++)
	    {
	        for (w = 0; w <= Width; w++)
	            printf("%3d ",K[i][w]);
	        printf("\n");
	    }

	    free(val);
		free(wt);
		free(K);
		fclose(test_file); 
	 	
	 	MPI_Wait(&request_snd, &status2);
		
    }

    
	

	MPI_Finalize();
	return 0;
}
