#include <mpi.h>
#include <string>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <chrono>
#include <random>
#include <vector>
#include <iostream>

auto list_to_string(const std::vector<int> list, const std::string text)
{
    std::stringstream ss;

    ss << text << " --> ";

    for (const auto &node : list)
    {
        ss << node << " ";
    }

    ss << std::endl;

    return ss.str();
}

int main(int argc, char **argv)
{
    int myrank;
    int comm_size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    const int list_size = 512;
    const int cycle1 = 2;
    const int cycle2 = 3;
    const int GCD = 1;

    int processes_send;
    if (comm_size >= 2)
    {
        processes_send = 2;
    }
    else
    {
        processes_send = 1;
    }

    std::vector<int> list;
    std::vector<int> partial_result;
    std::vector<int> additional_array(comm_size * GCD);

    std::chrono::time_point<std::chrono::steady_clock> start;
    std::chrono::time_point<std::chrono::steady_clock> end;

    std::vector<int> helper_array(processes_send * 2);
    std::vector<int> amounts_send(processes_send);
    std::vector<int> displs_send(processes_send);
    std::vector<int> amounts_rec(processes_send);
    std::vector<int> disps_rec(processes_send);

    if (processes_send == 2)
    {
        amounts_send = {2, 2};

        displs_send = {0, 2};

        amounts_rec = {1, 1};

        disps_rec = {0, 1};
    }
    else
    {
        amounts_send[0] = 4;
        displs_send[0] = 0;

        amounts_rec[0] = 2;
        disps_rec[0] = 0;
    }

    std::vector<int> subresults(amounts_send[myrank]);

    if (myrank >= 2)
    {
        MPI_Finalize();
        return 0;
    }

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

        //printf("%s", list_to_string(list, "Original list"));
        std::cout << list_to_string(list, "Original list");
    }
    start = std::chrono::steady_clock::now();
    for (int i = 0; i < list_size / 2; i++)
    {
        if (0 == myrank)
        {
            int pos = 0;
            for (int p = 0; p < 2; p++)
            {
                helper_array[p * 2] = list[i * 2 + p + cycle1];
                helper_array[p * 2 + 1] = list[i * 2 + p + cycle2];
            }
            MPI_Scatterv(helper_array.data(), amounts_send.data(), displs_send.data(), MPI_INT, subresults.data(), amounts_send[myrank], MPI_INT, 0, MPI_COMM_WORLD);
        }
        else
        {
            MPI_Scatterv(NULL, amounts_send.data(), displs_send.data(), MPI_INT, subresults.data(), amounts_send[myrank], MPI_INT, 0, MPI_COMM_WORLD);
        }

        for (int p = 0; p < amounts_rec[myrank]; p++)
            subresults[p] = subresults[p * 2] + subresults[p * 2 + 1];

        if (0 == myrank)
            MPI_Gatherv(subresults.data(), amounts_rec[myrank], MPI_INT, list.data() + i * 2, amounts_rec.data(), disps_rec.data(), MPI_INT, 0, MPI_COMM_WORLD);
        else
            MPI_Gatherv(subresults.data(), amounts_rec[myrank], MPI_INT, NULL, amounts_rec.data(), disps_rec.data(), MPI_INT, 0, MPI_COMM_WORLD);
    }

    end = std::chrono::steady_clock::now();

    if (0 == myrank)
    {
        std::cout << list_to_string(list, "Final list");
        auto diff = end - start;
        double time = std::chrono::duration<double, std::milli>(diff).count();
        printf("List_size: %d Number of CPUs: %d Time: %f ms", list_size, comm_size, time);
    }

    MPI_Finalize();
}