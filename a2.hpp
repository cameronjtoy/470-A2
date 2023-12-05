/*  Cameron
 *  Toy
 *  cjtoy
 */

#ifndef A2_HPP
#define A2_HPP

#include <vector>
#include <mpi.h>
#include <queue>

//Helper Function for Binary Search Index 
int binary_search_index(std::vector<short int>& v, short int value, int left, int right) {
    while (left <= right) {
        int mid = left + (right - left) / 2;
        if (v[mid] == value) {
            return mid;
        } else if (v[mid] < value) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }
    return left;
}

void isort(std::vector<short int>& Xi, MPI_Comm comm) {
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);


    //mpiexec -n 4 ./a2 64
    // Rank: 3 Size: 4 n: 16
    // Rank: 0 Size: 4 n: 16
    // Rank: 1 Size: 4 n: 16
    // Rank: 2 Size: 4 n: 16

    //Local Sample Size
    int n = Xi.size();

    // Phase 1: Local Sort
    std::sort(Xi.begin(), Xi.end());


    // Phase 2: Sample Selection and Communication
    std::vector<short int> pivots(size - 1);
    int d = n / size;

    // First process will select the pivots
    if (rank ==  0) {
        for (int i = 0; i < size - 1; ++i) {
            pivots[i] = Xi[(i + 1) * d];
        }
    }

    MPI_Bcast(pivots.data(), size - 1, MPI_SHORT, size - 1, comm);

    // Phase 3: Find buffers boundary and prepare for data exchange
    std::vector<int> scount(size, 0);
    std::vector<int> sdisp(size, 0);
    std::vector<short int> sbuffer;
    std::vector<short int> totalAmount;
    
    auto x = Xi.begin();
    for (int i = 0; i < size - 1; ++i) {
        auto upper = std::upper_bound(x, Xi.end(), pivots[i]);
        scount[i] = std::distance(x, upper);
        sdisp[i] = std::distance(Xi.begin(), x);
        x = upper;
    }

    scount[size - 1] = std::distance(x, Xi.end());
    sdisp[size - 1] = std::distance(Xi.begin(), x);
    sbuffer.assign(Xi.begin(), Xi.end());

    // Phase 4: Exchange data with other processes
    std::vector<int> rcount(size);
    // Alltoall Use to know how much each process will send to each other process, expensive, but necessary because 
    // we don't know how much data each process will send to each other process
    MPI_Alltoall(scount.data(), 1, MPI_INT, rcount.data(), 1, MPI_INT, comm);
    std::vector<int> rdisp(size);
    int receive = 0;
    for (int i = 0; i < size; ++i) {
        rdisp[i] = receive;
        receive += rcount[i];
    }
    std::vector<short int> rbuffer(receive);
    // All parameters are known to use Alltoallv
    MPI_Alltoallv(sbuffer.data(), scount.data(), sdisp.data(), MPI_SHORT, rbuffer.data(), rcount.data(), rdisp.data(), MPI_SHORT, comm);

    std::sort(rbuffer.begin(), rbuffer.end());
    Xi = rbuffer;

        
} 
#endif