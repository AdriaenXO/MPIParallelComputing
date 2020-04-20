#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mpi.h>

void all2allPersHC(int *inMsg, int *outMsg, int myRank, int commSize){
	outMsg[myRank] = inMsg[myRank];
	int sendTo = (myRank + 1) % commSize;
	int receiveFrom = (((myRank - 1) % commSize) + commSize) % commSize; //indicates the previous processor
	
	int myIndex = myRank;

	for (int i = 1; i < commSize; i++)
	{

		int currSize = commSize - i;
		
		int *tempArr = new int[currSize];
		int tempIndex = 0;
		
		// copy all elements except for mine
		for (int j = 0; j <= currSize; j++)
		{
			if (j == myIndex)
				continue;
			tempArr[tempIndex++] = inMsg[j];
		}
		
		myIndex = (myRank - i) > 0 ? (myRank - i) : 0;
		
		printf("CPU number %d Step number %d Data size %d Data %d %d %d %d", myRank, i, currSize, tempArr[0], tempArr[1], tempArr[2], tempArr[3]);
		
		printf("CPU number %d wants to send data to CPU number %d\n", myRank, sendTo);
		MPI_Send(tempArr, currSize, MPI_INT, sendTo, 0, MPI_COMM_WORLD);
		
		int dataFrom = (((myRank - i) % commSize) + commSize) % commSize; // indicates number of CPU from which data was received
		
		printf("CPU number %d wants to receive data from CPU number %d\n", myRank, receiveFrom);
		MPI_Recv(inMsg, currSize, MPI_INT, receiveFrom, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("CPU number %d Step number %d received data from CPU number %d: %d %d %d %d Data from: %d Data size: %d My index: %d\n", myRank, i, receiveFrom, inMsg[0], inMsg[1], inMsg[2], inMsg[3], dataFrom, currSize, myIndex);
		outMsg[dataFrom] = inMsg[myIndex];
	}
}

int main(int argc, char **argv)
{ 
	int myrank;
	int comm_size;
	int message; 
	MPI_Status status; 
	MPI_Init(&argc, &argv); 
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank); 
	MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

	int *inMsg=new int[comm_size];
	int *outMsg=new int[comm_size];
	int *expectedOut = new int[comm_size];
	for (int i = 0; i < comm_size; i++)
	{
		expectedOut[i] = i * 1000 + myrank;
	} 

	for(int i=0;i<comm_size;i++)
		inMsg[i]=1000*myrank+i;
	printf("Starting CPU number %d\n", myrank);
	all2allPersHC(inMsg, outMsg, myrank, comm_size);
	printf("Finishing CPU number %d\n", myrank);

	for(int i=0;i<comm_size;i++)
	{
		if (expectedOut[i] != outMsg[i])
		{
			printf("WRONG %d %d", myrank, i);
		}
		printf("%d,",outMsg[i]);
	} 
	printf("%d: %d %d %d %d %d %d %d %d %d %d %d\\", myrank, outMsg[0], outMsg[1], outMsg[2], outMsg[3], outMsg[4], outMsg[5], outMsg[6], outMsg[7], outMsg[8], outMsg[9], outMsg[10]);
	MPI_Finalize(); 
	return 0;
}