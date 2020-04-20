#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mpi.h>

int main(int argc, char **argv)
{
        int myrank;
        int comm_size;
        int message;
        MPI_Status status;
        MPI_Init(&argc, &argv);
        sleep(5);
        MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
        MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
        if (myrank != 0) {
                MPI_Send(&myrank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
        }
        else
        {
                int sum = 0;
                int i;
                for(i = 1; i < comm_size; i++) {
                        MPI_Recv(&message, 1, MPI_INT, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                        printf("received %d\n", message);
                        sum += message;
                }
                printf("sum = %d\n", sum);
        }
        MPI_Finalize();
        return 0;
}
