#include "communication.hpp"
#include "info.hpp"
#include <chrono>
#include <random>
#include <thread>

using std::default_random_engine;
using std::uniform_real_distribution;
using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::steady_clock;
using std::this_thread::sleep_for;

int main(int argc, char **argv) {
  using type = double;
  using node_type = node<type>;
  size_t num = (argc > 1) ? atoi(argv[1]) : 15;

#if defined(_OPENMP)
  double end, begin, start, finish;
  int size, rank, master = 0;
  unsigned short axis = 2;
  dataset<type> data{};
  unique_ptr<node_type> tree{new node_type{}};

  if (argc > 3) {
    auto nthreads = atoi(argv[3]);
    omp_set_num_threads(nthreads);
  }

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // Generating an uniform distribution along the 2 directions and putting the
  // data generated in a class 'dataset' (defined in the header 'dataset.hpp')
  // which is basically a 2 dimensional array with and enhanced interface useful
  // to deal with the recquirements of the tree construction
  if (rank == master) {
    default_random_engine gen;
    uniform_real_distribution<type> x(0.0, 100.0);
    uniform_real_distribution<type> y(0.0, 100.0);
    dataset<type> tmp(num);

    for (size_t i = 0; i < num; ++i) {
      tmp[i][0] = x(gen);
      tmp[i][1] = y(gen);
    }

    data = move(tmp);
  }

  // Building the tree: each node will have a different branch of the tree
  if (size / 2 > 0) {
    start = MPI_Wtime();
    auto tmp = distribute(master, size / 2, size / 2, axis, tree, data);
    finish = MPI_Wtime();

    if (rank % 2 == 0) {
      begin = MPI_Wtime();
      tmp->left_ptr.reset(build(data, axis));
      end = MPI_Wtime();
    } else {
      begin = MPI_Wtime();
      tmp->right_ptr.reset(build(data, axis));
      end = MPI_Wtime();
    }
  } else {
    finish = 0;
    start = 0;
    begin = MPI_Wtime();
    tree.reset(build(data, axis));
    end = MPI_Wtime();
  }

  // Follows a commented example of how it is possible to move through the
  // tree using properly the functions
  /*
  if (rank == master)
    cout << "Root: " << tree << endl;

  // Each process considers the head of its own subtree at the beginning
  auto tmp_ptr = tree.get();

  // We know that the master (processor 0) will always have a copy of the
  head
  // of the tree
  auto tmp_prc = right(tmp_ptr, master);

  if (rank == tmp_prc)
    cout << "Going to the right: " << '(' << tmp_ptr->pnt[0] << ", " <<
  tmp_ptr->pnt[1] << ')' << endl;

  tmp_prc = right(tmp_ptr, tmp_prc);
  if (rank == tmp_prc)
    cout << "Going to the right: " << '(' << tmp_ptr->pnt[0] << ", " <<
  tmp_ptr->pnt[1] << ')' << endl;
  */

  // Printing some info if the user requires it
  if (argc > 2) {
    MPI_Barrier(MPI_COMM_WORLD);

    for (int k = 0; k < size; ++k) {
      if (rank == k) {
        string command{argv[2]};
        char name[MPI_MAX_PROCESSOR_NAME];
        int len;
        MPI_Get_processor_name(name, &len);
        cout << "\nProcessor " << rank << " running on node " << name << endl;
        info(command, tree, end - begin, finish - start);
      }

      sleep_for(microseconds(100000));
    }

    cout << endl;
  }
  MPI_Finalize();
#else
  size_t max = 1 << atoi(argv[1]);
  for (size_t i = 1; i < max; i = i * 2) {
    num = i - 1;
    default_random_engine gen;
    uniform_real_distribution<type> x(0.0, 100.0);
    uniform_real_distribution<type> y(0.0, 100.0);
    dataset<type> tmp(num);

    for (size_t i = 0; i < num; ++i) {
      tmp[i][0] = x(gen);
      tmp[i][1] = y(gen);
    }

    auto begin = steady_clock::now();
    unique_ptr<node_type> tree{build(tmp, 2)};
    auto end = steady_clock::now();
    auto time = duration_cast<microseconds>(end - begin).count() / 1e+06;
  }
#endif

  return 0;
}
