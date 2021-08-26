// This code is part of the Problem Based Benchmark Suite (PBBS)
// Copyright (c) 2011 Guy Blelloch and the PBBS team
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once

#include <string>
#include "IO.h"
#include "pbbs/parallel.h"
#include "point.h"

using namespace benchIO;

namespace benchIO {
  using namespace std;

  void parseDouble(char** Str, double* a, intT n) {
    par_for (long i=0; i<n; i++) {
      a[i] = atof(Str[i]);
    }
  }

  auto is_newline = [] (char c) {
		      switch (c)  {
		      case '\r':
		      case '\n': return true;
		      default : return false;
		      }
		    };

  auto is_delim = [] (char c) {
		    switch (c)  {
		    case '\t':
		    case ';':
		    case ',':
		    case ' ' : return true;
		    default : return false;
		    }
		  };

  auto is_space = [] (char c) {
		    return is_newline(c) || is_delim(c) || c==0;
		  };

  auto is_number = [] (char c) {
		     switch (c)  {
		     case '0':
		     case '1':
		     case '2':
		     case '3':
		     case '4':
		     case '5':
		     case '6':
		     case '7':
		     case '8':
		     case '9':
		     case '.':
		     case '+':
		     case '-':
		     case 'e' : return true;
		     default : return false;
		     }
		   };

  bool isGenericHeader(std::string line) {
    for (auto c: line) {
      if (!is_number(c) && !is_delim(c)) return true;
    }
    return false;
  }

  int countEntry(std::string line) {
    while (is_delim(line.back()) ||
	   is_space(line.back()) ||
	   is_newline(line.back())) {
      line.pop_back();
    }

    int count = 0;
    for (auto c: line) {
      if (is_delim(c)) count ++;
    }
    return count + 1;
  }

  // returns dim
  int readHeader(const char* fileName) {
    std::ifstream file (fileName);
    if (!file.is_open())
      throw std::runtime_error("Unable to open file");

    std::string line1; std::getline(file, line1);
    if (isGenericHeader(line1)) {
      std::string line2; std::getline(file, line2);
      return countEntry(line2);
    } else {
      return countEntry(line1);
    }
  }

  _seq<double> readDoubleFromFile(char* fname, int d) {
    _seq<char> S = readStringFromFile(fname);
    words W = stringToWords(S.A, S.n);

    long n;
    double *P;

    if (isGenericHeader(W.Strings[0])) {
      n = W.m - 1;
      P = newA(double, n);
      parseDouble(W.Strings + 1, P, n);
    } else {
      n = W.m;
      P = newA(double, n);
      parseDouble(W.Strings, P, n);
    }

    W.del();
    return _seq<double>(P, n);
  }

};
