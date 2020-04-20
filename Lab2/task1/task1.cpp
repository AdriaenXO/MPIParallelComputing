#include <mpi.h>
#include <cstdlib>
#include <ctime>

using namespace std;

void send_number(int &number, int source, int destination)
{
	printf("CPU number %d wants to send a number %d to CPU number %d\n", source, number, destination);
	MPI_Send(&number, 1, MPI_INT, destination, 0, MPI_COMM_WORLD);
	printf("CPU number %d sent a number %d to CPU number %d\n", source, number, destination);
}

void receive_number(int &number, int source, int destination)
{
	printf("CPU number %d wants to receive a number %d from CPU number %d\n", destination, number, source);
	MPI_Recv(&number, 1, MPI_INT, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	printf("CPU number %d received a number %d from CPU number %d\n", destination, number, source);
}

int main(int argc, char **argv)
{
	int cpu_number;
	int number_of_cpus;
	int number;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &cpu_number);
	MPI_Comm_size(MPI_COMM_WORLD, &number_of_cpus);
	int cpu_from = 0;
	int cpu_to = 1;
	
	if (cpu_number == cpu_from)
	{
		srand(time(NULL));
		number = rand() % 100;
		send_number(number, cpu_from, cpu_to);
	}
	else if (cpu_number == cpu_to)
	{
		receive_number(number, cpu_from, cpu_to);
	}
	MPI_Finalize();
	return 0;
}
