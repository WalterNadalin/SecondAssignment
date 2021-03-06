#ifndef __TREEINFO__
#define __TREEINFO__

#include "node.hpp"
#include <map>
#include <string>

#if defined(_OPENMP)
#define TIME time(c, cc)
#else
#define TIME time(c)
#endif

using std::map;
using std::string;

enum class values { nd, all, info, print, time };

template <typename T> void initialize(T &m) {
  m["all"] = values::all;
  m["print"] = values::print;
  m["info"] = values::info;
  m["time"] = values::time;
}

// Prints some information about the threads running
void info();

#if defined(_OPENMP)
template <typename T, typename R> void time(T c, R cc) {
  cout << "\nConstruction time: " << c
       << " | Communication time: " << cc << endl;
}
#else
template <typename T> void time(T c) {
  cout << "\nConstruction time: " << c << endl;
}
#endif

// Prints required information evaluating the commands passed throught the
// command line
#if defined(_OPENMP)
template <typename T, typename R, typename S>
void info(string x, const T &tree, R c, S cc) noexcept {
#else
template <typename T, typename R>
void info(string x, const T &tree, R c) noexcept {
#endif
  map<string, values> mapped_strings;
  initialize(mapped_strings);

  switch (mapped_strings[x]) {
  case values::all:
    info();
    TIME;
    break;
  case values::info:
    info();
    break;
  case values::print:
    print(tree);
    break;
  case values::time:
    TIME;
    break;
  default:
    cout << "Command not recognized." << endl;
    break;
  }

  #if defined(_OPENMP)
    cout << "________________________________________"
         << "________________________________________" << endl;
  #endif
}

#endif
