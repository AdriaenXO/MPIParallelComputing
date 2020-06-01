#include <mpi.h>
#include <string>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <random>
#include <vector>
#include <unistd.h>
auto list_to_string(const std::vector<int> list, const std::string text)
{
    std::stringstream ss;

    ss << text << " --> ";

    for (const auto &node : list)
    {
        ss << node << " ";
    }

    ss << std::endl;

    return ss.str().c_str();
}

int main(int argc, char **argv)
{
    int myrank;
    int comm_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    const int list_size = 512;
    const int cycle1 = 3;
    const int cycle2 = 6;
    const int GCD = 3;

    int elements_per_proc = list_size / comm_size;

    std::vector<int> list;
    std::vector<int> partial_result(elements_per_proc + GCD);
    std::vector<int> additional_array(comm_size * GCD);

    std::chrono::time_point<std::chrono::steady_clock> start;
    std::chrono::time_point<std::chrono::steady_clock> end;

    if (myrank == 0)
    {
        // generate random values
        // https://www.fluentcpp.com/2019/05/24/how-to-fill-a-cpp-collection-with-random-values/
        auto randomNumberBetween = [](int low, int high) {
            auto randomFunc =
                [distribution_ = std::uniform_int_distribution<int>(low, high),
                 random_engine_ = std::mt19937{std::random_device{}()}]() mutable {
                    return distribution_(random_engine_);
                };
            return randomFunc;
        };

        std::generate_n(std::back_inserter(list), list_size,
                        randomNumberBetween(0, 50));

        printf("%s", list_to_string(list, "Original list"));

        start = std::chrono::steady_clock::now();

        for (int i = 0; i < comm_size; i++)
        {
            for (int o = 0; o < GCD; o++)
            {
                additional_array[i * GCD + o] = list[elements_per_proc * (i + 1) + o + GCD];
            }
        }

        printf("%s", list_to_string(additional_array, "Additional list"));
    }

    printf("Process %d - scatter started\n", myrank);
    MPI_Scatter(list.data() + cycle1, elements_per_proc, MPI_INT, partial_result.data(), elements_per_proc, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Scatter(additional_array.data(), GCD, MPI_INT, partial_result.data() + elements_per_proc, GCD, MPI_INT, 0, MPI_COMM_WORLD);
    printf("Process %d - scatter finished\n", myrank);

    printf("Process %d - %s", myrank, list_to_string(partial_result, "Used list"));

    for (int i = 0; i < elements_per_proc; i++)
    {
        partial_result[i] += partial_result[i + (cycle2 - cycle1)];
        //sleep(1);
    }

    MPI_Gather(partial_result.data(), elements_per_proc, MPI_INT, list.data(), elements_per_proc, MPI_INT, 0, MPI_COMM_WORLD);

    end = std::chrono::steady_clock::now();

    if (0 == myrank)
    {
        printf("%s", list_to_string(list, "Final list"));
        auto diff = end - start;
        double time = std::chrono::duration<double, std::milli>(diff).count();
        printf("List_size: %d Number of CPUs: %d Time: %f ms", list_size, comm_size, time);
    }

    MPI_Finalize();
}