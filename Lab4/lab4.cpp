#include <mpi.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>
#include <sstream>
#include <string>
#include <vector>
typedef int DATATYPE;

auto matrix_to_string(const std::vector<DATATYPE> matrix,
                      const std::string text) {
  std::stringstream ss;

  ss << text << " --> ";

  for (const auto& node : matrix) {
    ss << node << " ";
  }

  ss << std::endl;

  return ss.str();
}

void parallelMatrixVectorMult(std::vector<DATATYPE> submatrix,
                              std::vector<DATATYPE> subvector,
                              std::vector<DATATYPE>& subresult,
                              const int matrixDimension, const int myRank) {
  // obtain full vector
  std::vector<DATATYPE> full_vector(matrixDimension);
  MPI_Allgather(subvector.data(), subvector.size(), MPI_INT, full_vector.data(),
                subvector.size(), MPI_INT, MPI_COMM_WORLD);

  // calculate partial vector
  for (int i = 0; i < subvector.size(); i++) {
    for (int j = 0; j < matrixDimension; j++) {
      subresult[i] += submatrix[i * matrixDimension + j] * full_vector[j];
    }
  }
}

int main(int argc, char** argv) {
  int myrank;
  int comm_size;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
  MPI_Comm_size(MPI_COMM_WORLD, &comm_size);
  int strip_size;
  int matrix_size;
  std::vector<DATATYPE> matrix;
  std::vector<DATATYPE> vector;
  bool testing = false;
  bool printResults = false;

  // https://solarianprogrammer.com/2012/10/14/cpp-11-timing-code-performance/
  std::chrono::time_point<std::chrono::steady_clock> start;
  std::chrono::time_point<std::chrono::steady_clock> end;
  if (myrank == 0) {
    if (testing) {
      // data used for testing
      matrix.insert(matrix.end(), {
                                      2,
                                      -1,
                                      3,
                                      5,
                                      1,
                                      3,
                                      0,
                                      4,
                                      3,
                                      0,
                                      -1,
                                      -2,
                                      0,
                                      0,
                                      0,
                                      1,
                                  });

      matrix_size = matrix.size();
      vector.insert(vector.end(), {
                                      2,
                                      0,
                                      -1,
                                      1,
                                  });
    } else {
      // generate random values
      int dimensions = 2048;
      matrix_size = dimensions * dimensions;

      // https://www.fluentcpp.com/2019/05/24/how-to-fill-a-cpp-collection-with-random-values/
      auto randomNumberBetween = [](int low, int high) {
        auto randomFunc =
            [distribution_ = std::uniform_int_distribution<int>(low, high),
             random_engine_ = std::mt19937{std::random_device{}()}]() mutable {
              return distribution_(random_engine_);
            };
        return randomFunc;
      };

      std::generate_n(std::back_inserter(matrix), matrix_size,
                      randomNumberBetween(-10, 10));

      std::generate_n(std::back_inserter(vector), dimensions,
                      randomNumberBetween(-10, 10));
    }
  }

  if (myrank == 0) {
    if (printResults) {
      printf("%s", matrix_to_string(matrix, "Matrix").c_str());
      printf("%s", matrix_to_string(vector, "Vector").c_str());
    }
    // start = std::chrono::steady_clock::now();
  }
  // get matrix size
  MPI_Bcast(&matrix_size, 1, MPI_INT, 0, MPI_COMM_WORLD);
  // printf("CPU %d matrix_size %d\n", myrank, matrix_size);

  int matrix_elements_per_proc = matrix_size / comm_size;
  // printf("CPU %d elements_per_proc %d\n", myrank, elements_per_proc);

  // get submatrix
  std::vector<DATATYPE> submatrix(matrix_elements_per_proc);
  MPI_Scatter(matrix.data(), matrix_elements_per_proc, MPI_INT,
              submatrix.data(), matrix_elements_per_proc, MPI_INT, 0,
              MPI_COMM_WORLD);
  // printf("CPU %d %s", myrank, matrix_to_string(submatrix,
  // "Submatrix").c_str());

  // get subvector
  int matrix_dimension = std::sqrt(matrix_size);
  int vector_elements_per_proc = matrix_dimension / comm_size;
  std::vector<DATATYPE> subvector(vector_elements_per_proc);
  MPI_Scatter(vector.data(), vector_elements_per_proc, MPI_INT,
              subvector.data(), vector_elements_per_proc, MPI_INT, 0,
              MPI_COMM_WORLD);
  // printf("CPU %d %s", myrank, matrix_to_string(subvector,
  // "Subvector").c_str());

  // multiply submatrix
  std::vector<DATATYPE> subresult(vector_elements_per_proc, 0);
  start = std::chrono::steady_clock::now();
  parallelMatrixVectorMult(submatrix, subvector, subresult, matrix_dimension,
                           myrank);
  end = std::chrono::steady_clock::now();
  // printf("CPU %d %s", myrank, matrix_to_string(subresult,
  // "Subresult").c_str());

  // gather
  std::vector<DATATYPE> final_result(matrix_dimension);
  MPI_Gather(subresult.data(), subresult.size(), MPI_INT, final_result.data(),
             subresult.size(), MPI_INT, 0, MPI_COMM_WORLD);
  if (myrank == 0) {
    // end = std::chrono::steady_clock::now();
    if (printResults) {
      printf("%s", matrix_to_string(final_result, "Final result").c_str());
    }
    auto diff = end - start;
    double time = std::chrono::duration<double, std::milli>(diff).count();
    printf("Matrix dimension: %d Number of CPUs: %d Time: %f ms",
           matrix_dimension, comm_size, time);
  }

  MPI_Finalize();
  return 0;
}
