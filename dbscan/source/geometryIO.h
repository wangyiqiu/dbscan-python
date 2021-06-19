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


#ifndef _BENCH_GEOMETRY_IO
#define _BENCH_GEOMETRY_IO
#include <string>
#include "IO.h"
#include "pbbs/parallel.h"
#include "geometry.h"
using namespace benchIO;

namespace benchIO {
  using namespace std;

  string HeaderPoint2d = "pbbs_sequencePoint2d";
  string HeaderPoint3d = "pbbs_sequencePoint3d";
  string HeaderPoint4d = "pbbs_sequencePoint4d";
  string HeaderPoint5d = "pbbs_sequencePoint5d";
  string HeaderPoint6d = "pbbs_sequencePoint6d";
  string HeaderPoint7d = "pbbs_sequencePoint7d";
  string HeaderPoint8d = "pbbs_sequencePoint8d";
  string HeaderPoint9d = "pbbs_sequencePoint9d";
  string HeaderTriangles = "pbbs_triangles";

  inline string headerPoint(int dim) {
    if (dim == 2) return HeaderPoint2d;
    else if (dim == 3) return HeaderPoint3d;
    else if (dim == 4) return HeaderPoint4d;
    else if (dim == 5) return HeaderPoint5d;
    else if (dim == 6) return HeaderPoint6d;
    else if (dim == 7) return HeaderPoint7d;
    else if (dim == 8) return HeaderPoint8d;
    else if (dim == 9) return HeaderPoint9d;
    else {
      cout << "headerPoint unsupported dimension, abort" << dim << endl; abort();
    }
  }

  template <class pointT>
  int writePointsToFile(pointT* P, intT n, char* fname) {
    string Header = (pointT::dim == 2) ? HeaderPoint2d : HeaderPoint3d;
    int r = writeArrayToFile(Header, P, n, fname);
    return r;
  }

  template <class pointT>
  void parsePoints(char** Str, pointT* P, intT n) {
    int d = pointT::dim;
    double* a = newA(double,n*d);
    {par_for (long i=0; i<d*n; i++)
	a[i] = atof(Str[i]);}
    {par_for (long i=0; i<n; i++)
	P[i] = pointT(a+(d*i));}
    free(a);
  }

  void parseDouble(char** Str, double* a, intT n) {
    par_for (long i=0; i<n; i++) {
      a[i] = atof(Str[i]);
    }
  }

  template <class pointT>
  void parseCsvPoints(char** Str, pointT* P, intT n, int col, int sCol, int eCol) {
    int d = pointT::dim;
    double* a = newA(double,n*d);
    par_for (long i=0; i<n/col; i++) {
      for (int j=sCol; j<eCol; j++) {
        a[i*d+j-sCol] = atof(Str[i*col+j]);
      }
    }
    par_for (long i=0; i<n/col; i++) {
      P[i] = pointT(a+(d*i));
      //cout << P[i] << endl;
    }
    free(a);
  }

  inline int readPointsDimensionFromFile(char* fname) {
    _seq<char> S = readStringFromFile(fname);
    words W = stringToWords(S.A, S.n);
    return (int)(W.Strings[0][18])-48;
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

  // template <class pointT>
  // _seq<pointT> readPointsFromFile(char* fname, int d) {
  //   _seq<char> S = readStringFromFile(fname);
  //   words W = stringToWords(S.A, S.n);
  //   //int d = pointT::dim;

  //   // if (W.m == 0 || W.Strings[0] != headerPoint(d)) {
  //   //   cout << "readPointsFromFile wrong file type" << endl;
  //   //   abort();
  //   // }

  //   long n;
  //   if (isGenericHeader(W.Strings[0])) {
  //     cout << "generic header" << endl;
  //     n = (W.m-1)/d;
  //   } else {
  //     cout << "no header" << endl;
  //     n = W.m/d;
  //   }
  //   cout << "n-in = " << n << endl;

  //   pointT *P = newA(pointT, n);
  //   parsePoints(W.Strings + 1, P, n);
  //   W.del();
  //   return _seq<pointT>(P, n);
  // }

  _seq<double> readDoubleFromFile(char* fname, int d) {
    _seq<char> S = readStringFromFile(fname);
    words W = stringToWords(S.A, S.n);

    long n;
    if (isGenericHeader(W.Strings[0])) {
      n = W.m - 1;
    } else {
      n = W.m;
    }

    double *P = newA(double, n);
    parseDouble(W.Strings + 1, P, n);
    W.del();
    return _seq<double>(P, n);
  }

  template <class pointT>
  _seq<pointT> readPointsFromFileCSV(char* fname, int col, int sCol, int eCol) {
    _seq<char> S = readStringFromFile(fname);
    cout << "S.n = " << S.n << endl;
    words W = stringToWordsCSV(S.A, S.n);
    int d = pointT::dim;
    if (W.m == 0) {
      cout << "invalid csv" << endl;
      abort();
    }
    cout << "csv columns: ";
    for(intT i=sCol; i<eCol; ++i) {
      cout << W.Strings[i] << " ";
    }
    cout << endl;
    long n = (W.m-col);
    pointT *P = newA(pointT, n/col);
    parseCsvPoints(W.Strings+col, P, n, col, sCol, eCol);
    // W.del();
    cout << "verify P[0] = " << P[0] << endl;
    cout << "verify P[last] = " << P[n/col-1] << endl;
    cout << "input n = " << n/col-1 << endl;
    return _seq<pointT>(P, n/col);
  }

  inline bool isNumber(char myChar) {
    if (myChar == '0' || myChar == '1' || myChar == '2' ||
	myChar == '3' || myChar == '4' || myChar == '5' ||
	myChar == '6' || myChar == '7' || myChar == '8' ||
	myChar == '9') {
      return true;
    } else {
      return false;
    }
  }
  
  inline intT extractDim(words *W) {
    int d;
    char *targetString = W->Strings[0];
    intT myPt = 18;
    while (isNumber(targetString[myPt])) myPt ++;
    targetString[myPt] = '\0'; // TODO Support 10+
    d = atoi(&targetString[18]);
    return d;
  }

};

#endif // _BENCH_GEOMETRY_IO
