#include <mpi.h>
#include <cstdlib>
#include <ctime>
#include <cmath>

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
	printf("CPU number %d received a number from CPU number %d\n", destination, source);
}

/* 1: Assume that p = 2^d
2: mask ← 2^d − 1 (set all bits)
3: for k = d − 1, d − 2, . . . , 0 do
4: 		mask ← mask XOR 2^k (clear bit k)
5: 		if me AND mask = 0 then: (lower k bits of me are 0)
7: 			partner ← me XOR 2^k (partner has opposite bit k)
8: 			if me AND 2^k = 0 then
9: 				Send M to partner
10: 		else
11: 			Receive M from partner
12: 		end if
13:		end if
14: end for */

void one2AllBroadcastHypercube(int &msg, int srcProc)
{
	int cpu_number;
	int number_of_cpus;
	int number;
	MPI_Comm_rank(MPI_COMM_WORLD, &cpu_number);
	MPI_Comm_size(MPI_COMM_WORLD, &number_of_cpus);
	int cpu_from = 0;
	int d = log2(number_of_cpus);
	int mask = pow(2, d) - 1;
	int me = cpu_number ^ srcProc;
	for(int k = d - 1; k >= 0; k--) 
	{
		int flag = pow(2, k);
		mask = mask ^ flag;
		if ((me & mask) == 0) 
		{
			int partner = me ^ flag;
			if ((me & flag) == 0)
			{
				send_number(msg, me, partner);
			}
			else 
			{
				receive_number(msg, partner, me);
			}
		}
	} 
	/* for (int i = d - 1; i >= 0; i--)
	{
		int temp = pow(2, i);
		mask = mask ^ temp;
		if (cpu_number & mask == 0)
		{
			if (cpu_number & temp == 0)
			{
				int msg_destination = cpu_number ^ temp;
				send_number(msg, cpu_number, msg_destination);
			}
			else
			{
				int msg_source = cpu_number ^ temp;
				receive_number(msg, msg_source, cpu_number);
			}
		}
	} */
}

int main(int argc, char **argv)
{
	srand(time(0));
	int number = rand() % 100;
	int cpu_number;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &cpu_number);
	number += cpu_number;
	printf("CPU number %d Before value %d\n", cpu_number, number);
	one2AllBroadcastHypercube(number , 0);
	printf("CPU number %d After value %d\n", cpu_number, number);
	MPI_Finalize();
	return 0;
}
