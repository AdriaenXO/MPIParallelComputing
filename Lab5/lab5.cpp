#include <mpi.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <tuple>
#include <vector>
typedef int DATATYPE;
MPI_Datatype mpi_data = MPI_INT;
MPI_Datatype mpi_data2 = MPI_2INT;

auto list_to_string(const std::vector<DATATYPE> list, const std::string text)
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

auto list_to_string(const std::vector<bool> list, const std::string text)
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

void parallelSelectionSort(std::vector<DATATYPE> &sublist,
                           const int list_size, const int myRank)
{
  std::vector<bool> checkList(sublist.size(), false);
  struct
  {
    DATATYPE value;
    int rank;
  } local_minimum, global_minimum;
  local_minimum.rank = myRank;
  int step = 0;
  int currentProc;
  int currentIndex;
  while (step < list_size)
  {
    if (step == 0)
    {
      currentProc = 0;
      currentIndex = 0;
    }
    else
    {
      currentProc = step / sublist.size();
      currentIndex = step % sublist.size();
    }

    // find smallest element and its index that has not been chosen before
    local_minimum.value = std::numeric_limits<DATATYPE>::max();
    int smallestElementIndex;
    for (int i = 0; i < sublist.size(); i++)
    {
      if ((sublist[i] < local_minimum.value) && !checkList[i])
      {
        local_minimum.value = sublist[i];
        smallestElementIndex = i;
      }
    }

    printf("Step %d CPU %d smallest number %d index %d\n", step, myRank,
           local_minimum.value, smallestElementIndex);

    MPI_Allreduce(&local_minimum, &global_minimum, 1, mpi_data2, MPI_MINLOC,
                  MPI_COMM_WORLD);
    if (myRank == 0)
    {
      printf("Step %d global_minimum %d rank %d\n", step,
             global_minimum.value, global_minimum.rank);
      printf("Step %d currentProc %d currentIndex %d\n", step, currentProc, currentIndex);
    }

    // exchange data between chosen processes
    DATATYPE swapElement;
    if (currentProc == global_minimum.rank)
    {
      if (currentProc == myRank)
      {
        swapElement = sublist[currentIndex];
        sublist[currentIndex] = global_minimum.value;
        sublist[smallestElementIndex] = swapElement;
        checkList[currentIndex] = true;
      }
    }
    else
    {
      if (currentProc == myRank)
      {
        swapElement = sublist[currentIndex];
        MPI_Send(&swapElement, 1, mpi_data, global_minimum.rank, 0,
                 MPI_COMM_WORLD);
        sublist[currentIndex] = global_minimum.value;
        checkList[currentIndex] = true;
      }

      if (global_minimum.rank == myRank)
      {
        MPI_Recv(&swapElement, 1, mpi_data, currentProc, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
        sublist[smallestElementIndex] = swapElement;
      }
    }
    printf("Step %d CPU %d %s", step, myRank, list_to_string(sublist, "Sublist").c_str());
    printf("Step %d CPU %d %s", step, myRank, list_to_string(checkList, "Checklist").c_str());
    step++;
  }
}

int main(int argc, char **argv)
{
  int myrank;
  int comm_size;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  int strip_size;
  int list_size;
  std::vector<DATATYPE> list;
  bool testing = true;

  // https://solarianprogrammer.com/2012/10/14/cpp-11-timing-code-performance/
  std::chrono::time_point<std::chrono::steady_clock> start;
  std::chrono::time_point<std::chrono::steady_clock> end;
  if (myrank == 0)
  {
    if (testing)
    {
      // data used for testing
      list.insert(list.end(), {12, 8, 2, 7, 16, 3, 4, 11, 15, 1, 10, 13, 9, 5, 14, 6});

      list_size = list.size();
    }
    else
    {
      // generate random values
      list_size = 1000;
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
                      randomNumberBetween(-50, 50));
    }
  }

  if (myrank == 0)
  {
    printf("%s", list_to_string(list, "Initial list").c_str());
  }

  strip_size = list_size / comm_size;

  // propagate strip_size
  MPI_Bcast(&list_size, 1, mpi_data, 0, MPI_COMM_WORLD);
  MPI_Bcast(&strip_size, 1, mpi_data, 0, MPI_COMM_WORLD);
  // printf("CPU %d strip_size %d\n", myrank, strip_size);

  // propagate parts of list
  std::vector<DATATYPE> sublist(strip_size);
  MPI_Scatter(list.data(), strip_size, mpi_data, sublist.data(), strip_size,
              mpi_data, 0, MPI_COMM_WORLD);
  printf("CPU %d %s", myrank, list_to_string(sublist, "Sublist").c_str());

  // sort values
  //std::vector<DATATYPE> subresult(strip_size, 0);
  start = std::chrono::steady_clock::now();
  parallelSelectionSort(sublist, list_size, myrank);
  end = std::chrono::steady_clock::now();

  // gather
  std::vector<DATATYPE> final_result(list_size);
  MPI_Gather(sublist.data(), sublist.size(), mpi_data, final_result.data(),
             sublist.size(), mpi_data, 0, MPI_COMM_WORLD);
  if (myrank == 0)
  {
    // end = std::chrono::steady_clock::now();
    printf("%s", list_to_string(final_result, "Final result").c_str());
    auto diff = end - start;
    double time = std::chrono::duration<double, std::milli>(diff).count();
    printf("List_size: %d Number of CPUs: %d Time: %f ms", list_size, comm_size,
           time);
  }

  MPI_Finalize();
  return 0;
}
