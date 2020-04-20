#include <mpi.h>
#include <cstdlib>
#include <ctime>
#include <cmath>

using namespace std;

int main(int argc, char **argv)
{
	srand(time(0));
	int number = 22;
	int cpu_number;
	int number_of_cpus;
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &cpu_number);
	MPI_Comm_size(MPI_COMM_WORLD, &number_of_cpus);
	int cpu_from = 0;
	if (cpu_number == cpu_from)
	{
		MPI_Bcast(&number, 1, MPI_INT, cpu_from, MPI_COMM_WORLD);
		printf("CPU number %d is broadcasting number %d\n", cpu_from, number);
		/* int MPI_Bcast(
		  void *buffer,
		  int count,
		  MPI_Datatype datatype,
		  int root,
		  MPI_Comm comm
		);
		Parameters
			buffer
				[in/out] starting address of buffer (choice)
			count
				[in] number of entries in buffer (integer)
			datatype
				[in] data type of buffer (handle)
			root
				[in] rank of broadcast root (integer)
			comm
				[in] communicator (handle) */
	}
	else 
	{
		// https://stackoverflow.com/questions/2367300/corresponding-receive-routine-of-mpi-bcast
		// Apparently the same function works as sender and receiver
		MPI_Bcast(&number, 1, MPI_INT, cpu_from, MPI_COMM_WORLD);
		printf("CPU number %d is receiving number %d\n", cpu_number, number);
	}
	MPI_Finalize();
	return 0;
}
