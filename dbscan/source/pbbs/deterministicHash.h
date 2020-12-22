// This code is part of the Problem Based Benchmark Suite (PBBS)
// Copyright (c) 2010 Guy Blelloch and the PBBS team
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

#ifndef A_HASH_INCLUDED
#define A_HASH_INCLUDED

#include "parallel.h"
#include "utils.h"
#include "sequence.h"
//#include "gettime.h"
using namespace std;

// A "history independent" hash table that supports insertion, and searching 
// It is described in the paper
//   Guy E. Blelloch, Daniel Golovin
//   Strongly History-Independent Hashing with Applications
//   FOCS 2007: 272-282
// At any quiescent point (when no operations are actively updating the
//   structure) the state will depend only on the keys it contains and not
//   on the history of the insertion order.
// Insertions can happen in parallel, but they cannot overlap with searches
// Searches can happen in parallel
// Deletions must happen sequentially
template <class HASH, class intT>
class Table {
 private:
  typedef typename HASH::eType eType;
  typedef typename HASH::kType kType;
  intT m;
  intT mask;
  eType empty;
  HASH hashStruct;
  eType* TA;
  intT* compactL;
  float load;

  // needs to be in separate routine due to Cilk bugs
  static void clearA(eType* A, intT n, eType v) {
    parallel_for (0, n, [&](intT i) {A[i] = v;});
  }

  struct notEmptyF { 
    eType e; notEmptyF(eType _e) : e(_e) {} 
    int operator() (eType a) {return e != a;}};

  uintT hashToRange(intT h) {return h & mask;}
  intT firstIndex(kType v) {return hashToRange(hashStruct.hash(v));}
  intT incrementIndex(intT h) {return hashToRange(h+1);}
  intT decrementIndex(intT h) {return hashToRange(h-1);}
  bool lessIndex(intT a, intT b) {return 2 * hashToRange(a - b) > m;}
  bool lessEqIndex(intT a, intT b) {return a==b || 2 * hashToRange(a - b) > m;}


 public:
  // Size is the maximum number of values the hash table will hold.
  // Overfilling the table could put it into an infinite loop.
 Table(intT size, HASH hashF, float _load) :
  load(_load),
    m(1 << utils::log2Up(100+(intT)(_load*(float)size))), 
    mask(m-1),
    empty(hashF.empty()),
    hashStruct(hashF), 
    TA(newA(eType,m)),
    compactL(NULL) 
      { clearA(TA,m,empty); }

  Table(intT size, HASH hashF) :
    m(1 << utils::log2Up(100+2*size)), 
    mask(m-1),
    empty(hashF.empty()),
    hashStruct(hashF), 
    TA(newA(eType,m)),
    compactL(NULL) 
      { clearA(TA,m,empty); }

  // Deletes the allocated arrays
  void del() {
    free(TA); 
    if (compactL != NULL) free(compactL);
  }

  // prioritized linear probing
  //   a new key will bump an existing key up if it has a higher priority
  //   an equal key will replace an old key if replaceQ(new,old) is true
  // returns 0 if not inserted (i.e. equal and replaceQ false) and 1 otherwise
  bool insert(eType v) {
    intT i = firstIndex(hashStruct.getKey(v));
    while (1) {
      eType c = TA[i];
      if (c == empty) {
	if (utils::CAS(&TA[i],c,v)) return 1;
      } else {
	int cmp = hashStruct.cmp(hashStruct.getKey(v),hashStruct.getKey(c));
	if (cmp == 0) {
	  if (!hashStruct.replaceQ(v,c)) return 0; 
	  else if (utils::CAS(&TA[i],c,v)) return 1;
	} else if (cmp == -1) 
	  i = incrementIndex(i);
	else if (utils::CAS(&TA[i],c,v)) {
	  v = c;
	  i = incrementIndex(i);
	}
      }
    }
  }

  bool deleteVal(kType v) {
    intT i = firstIndex(v);
    int cmp;

    // find first element less than or equal to v in priority order
    intT j = i;
    eType c = TA[j];

    if (c == empty) return true;
      
    // find first location with priority less or equal to v's priority
    while ((cmp = (c==empty) ? 1 : hashStruct.cmp(v, hashStruct.getKey(c))) < 0) {
      j = incrementIndex(j);
      c = TA[j];
    }
    while (1) {
      // Invariants:
      //   v is the key that needs to be deleted
      //   j is our current index into TA
      //   if v appears in TA, then at least one copy must appear at or before j
      //   c = TA[j] at some previous time (could now be changed)
      //   i = h(v)
      //   cmp = compare v to key of c (1 if greater, 0 equal, -1 less)
      if (cmp != 0) {
        // v does not match key of c, need to move down one and exit if
        // moving before h(v)
	if (j == i) return true;
	j = decrementIndex(j);
	c = TA[j];
	cmp = (c == empty) ? 1 : hashStruct.cmp(v, hashStruct.getKey(c));      
      } else { // found v at location j (at least at some prior time)

	// Find next available element to fill location j.
        // This is a little tricky since we need to skip over elements for
        // which the hash index is greater than j, and need to account for
        // things being moved downwards by others as we search.
        // Makes use of the fact that values in a cell can only decrease 
        // during a delete phase as elements are moved from the right to left.
	intT jj = incrementIndex(j);
	eType x = TA[jj];
	while (x != empty && lessIndex(j, firstIndex(hashStruct.getKey(x)))) {
	  jj = incrementIndex(jj);
	  x = TA[jj];
	}
	intT jjj = decrementIndex(jj);
	while (jjj != j) {
	  eType y = TA[jjj];
	  if (y == empty || !lessIndex(j, firstIndex(hashStruct.getKey(y)))) {
	    x = y;
	    jj = jjj;
	  }
	  jjj = decrementIndex(jjj);
	}

	// try to copy the the replacement element into j
	if (utils::CAS(&TA[j],c,x)) {
          // swap was successful
          // if the replacement element was empty, we are done
	  if (x == empty) return true;

	  // Otherwise there are now two copies of the replacement element x
          // delete one copy (probably the original) by starting to look at jj.
          // Note that others can come along in the meantime and delete 
          // one or both of them, but that is fine.
	  v = hashStruct.getKey(x);
	  j = jj;
	  i = firstIndex(v);
	}
	c = TA[j];
	cmp = (c == empty) ? 1 : hashStruct.cmp(v, hashStruct.getKey(c));
      }
    } 
  }

  // Returns the value if an equal value is found in the table
  // otherwise returns the "empty" element.
  // due to prioritization, can quit early if v is greater than cell
  eType find(kType v) {
    intT h = firstIndex(v);
    eType c = TA[h]; 
    while (1) {
      if (c == empty) return empty;
      int cmp = hashStruct.cmp(v,hashStruct.getKey(c));
      if (cmp >= 0) {
	if (cmp == 1) return empty;
	else return c;
      }
      h = incrementIndex(h);
      c = TA[h];
    }
  }

  // returns the number of entries
  intT count() {
    return sequence::mapReduce<intT>(TA,m,utils::addF<intT>(),notEmptyF(empty));
  }

  // returns all the current entries compacted into a sequence
  _seq<eType> entries() {
    bool *FL = newA(bool,m);
    parallel_for (0, m, [&](intT i) {
		     FL[i] = (TA[i] != empty);});
    _seq<eType> R = sequence::pack(TA,FL,m);
    free(FL);
    return R;
  }

 // needs to be in separate routine due to Cilk bugs
  void clear() {
    parallel_for (0,m, [&](intT i) {TA[i] = empty;});
  }

  // prints the current entries along with the index they are stored at
  void print() {
    cout << "vals = ";
    for (intT i=0; i < m; i++) 
      if (TA[i] != empty)
	cout << i << ":" << TA[i] << ",";
    cout << endl;
  }
};
template <class HASH, class ET, class intT>
_seq<ET> removeDuplicates(_seq<ET> S, intT m, HASH hashF) {
  Table<HASH,intT> T(m,hashF,1.5);
  ET* A = S.A;
  //timer remdupstime;
  //remdupstime.start();
  parallel_for(0, S.n, [&](intT i) { T.insert(A[i]);});
  //remdupstime.reportTotal("remdups time");
  _seq<ET> R = T.entries();
  T.del(); 
  return R;
}

template <class HASH, class ET>
_seq<ET> removeDuplicates(_seq<ET> S, HASH hashF) {
  return removeDuplicates(S, S.n, hashF);
}

template <class intT>
struct hashInt {
  typedef intT eType;
  typedef intT kType;
  eType empty() {return -1;}
  kType getKey(eType v) {return v;}
  intT hash(kType v) {return utils::hash(v);}
  int cmp(kType v, kType b) {return (v > b) ? 1 : ((v == b) ? 0 : -1);}
  bool replaceQ(eType v, eType b) {return 0;}
};

// works for non-negative integers (uses -1 to mark cell as empty)

static _seq<intT> removeDuplicates(_seq<intT> A) {
  return removeDuplicates(A,hashInt<intT>());
}

//typedef Table<hashInt> IntTable;
//static IntTable makeIntTable(int m) {return IntTable(m,hashInt());}
template <class intT>
static Table<hashInt<intT>,intT > makeIntTable(intT m) {
  return Table<hashInt<intT>,intT >(m,hashInt<intT>());}

struct hashStr {
  typedef char* eType;
  typedef char* kType;

  eType empty() {return NULL;}
  kType getKey(eType v) {
    return v;}

  uintT hash(kType s) {
    uintT hash = 0;
    while (*s) hash = *s++ + (hash << 6) + (hash << 16) - hash;
    return hash;
  }

  int cmp(kType s, kType s2) {
    while (*s && *s==*s2) {s++; s2++;};
    return (*s > *s2) ? 1 : ((*s == *s2) ? 0 : -1);
  }

  bool replaceQ(eType s, eType s2) {return 0;}
};

static _seq<char*> removeDuplicates(_seq<char*> S) {
  return removeDuplicates(S,hashStr());}

template <class intT>
static Table<hashStr,intT> makeStrTable(intT m) {
  return Table<hashStr,intT>(m,hashStr());}

template <class KEYHASH, class DTYPE>
struct hashPair {
  KEYHASH keyHash;
  typedef typename KEYHASH::kType kType;
  typedef pair<kType,DTYPE>* eType;
  eType empty() {return NULL;}

  hashPair(KEYHASH _k) : keyHash(_k) {}

  kType getKey(eType v) { return v->first; }

  uintT hash(kType s) { return keyHash.hash(s);}
  int cmp(kType s, kType s2) { return keyHash.cmp(s, s2);}

  bool replaceQ(eType s, eType s2) {
    return s->second > s2->second;}
};

static _seq<pair<char*,intT>*> removeDuplicates(_seq<pair<char*,intT>*> S) {
  return removeDuplicates(S,hashPair<hashStr,intT>(hashStr()));}

struct hashSimplePair {
  typedef pair<intT,intT> eType;
  typedef intT kType;
  eType empty() {return pair<intT,intT>(-1,-1);}
  kType getKey(eType v) { return v.first; }
  uintT hash(intT s) { return utils::hash(s);}
  int cmp(intT v, intT b) {return (v > b) ? 1 : ((v == b) ? 0 : -1);}
  bool replaceQ(eType s, eType s2) {return s.second > s2.second;}
};

static _seq<pair<intT,intT> > removeDuplicates(_seq<pair<intT,intT> > A) {
  return removeDuplicates(A,hashSimplePair());
}


#endif // _A_HASH_INCLUDED
