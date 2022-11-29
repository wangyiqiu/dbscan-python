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

#ifndef ND_HASH_H
#define ND_HASH_H

#include "parallel.h"
#include "utils.h"
#include "sequence.h"
#include <atomic>
using namespace std;

extern intT g_dim;

template <class HASH, class intT>
class Table {
 public:
  // typedef typename std::atomic<typename HASH::eType> eType;
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
    parallel_for(0, n, [&](intT i) {A[i] = v;});
  }

/*
 // needs to be in separate routine due to Cilk bugs
  void clear() {
    parallel_for(0, m, [&](intT i) {TA[i] = empty;});
  }
*/

  struct notEmptyF {
    eType e; notEmptyF(eType _e) : e(_e) {}
    int operator() (eType a) {return e != a;}};

  uintT hashToRange(intT h) {return h & mask;}
  intT firstIndex(kType v) {return hashToRange(hashStruct.hash(v));}
  intT incrementIndex(intT h) {return hashToRange(h+1);}
  intT decrementIndex(intT h) {return hashToRange(h-1);}
  bool lessIndex(intT a, intT b) {return 2 * hashToRange(a - b) > m;}

/*
  // Size is the maximum number of values the hash table will hold.
  // Overfilling the table could put it into an infinite loop.
 Table(intT size, HASH hashF, float _load) :
    m((intT)1 << utils::log2Up(100+(intT)(_load*(float)size))),
    mask(m-1),
    empty(hashF.empty()),
    hashStruct(hashF),
    TA(newA(eType,m)),
    compactL(NULL),
	  load(_load)
      { clearA(TA,m,empty);
      }
*/

 Table(intT size, HASH hashF) :
    m((intT)1 << utils::log2Up(100+(intT)(2.0*(float)size))),
    mask(m-1),
    empty(hashF.empty()),
    hashStruct(hashF),
    TA(newA(eType,m)),
    compactL(NULL),
	  load(2.0)
      { clearA(TA,m,empty);
      }

/*
  // Constructor that takes an array for the hash table space.  The
  // passed size must be a power of 2 and will not be rounded.  Make
  // sure to not call del() if you are passing a pointer to the middle
  // of an array.
 Table(intT size, eType* _TA, HASH hashF) :
    m(size),
    mask(m-1),
    empty(hashF.empty()),
    hashStruct(hashF),
    TA(newA(eType,m)),
    compactL(NULL),
	  load(1.0)
      { clearA(TA,m,empty);
      }

  void setActive(intT mm) {
    m = (intT)1 << utils::log2Up(100+(intT)(load*(float)mm));
    mask = m-1;
  }
*/

  // Deletes the allocated arrays
  void del() {
    free(TA);
    if (compactL != NULL) free(compactL);
  }

  //for equal keys, first one to arrive at location wins, linear probing
  bool insert(eType v) {
    kType vkey = hashStruct.getKey(v);
    intT h = firstIndex(vkey);
    while (1) {
      eType c;
      c = TA[h];
      // intT cmp;
      // if(c==empty && utils::CAS(&TA[h],c,v)) return 1; 
      if(c==empty && hashStruct.cas(&TA[h],c,v)) return 1; 
      else if(0 == hashStruct.cmp(vkey,hashStruct.getKey(c))) {
	      if(!hashStruct.replaceQ(v,c))
	        return 0;
	      // else if (utils::CAS(&TA[h],c,v))
        else if (hashStruct.cas(&TA[h],c,v))
          return 1;
      }
      // move to next bucket
      h = incrementIndex(h);
    }
    return 0; // should never get here
  }

  /*
  //for equal keys, first one to arrive at location wins, linear probing
  bool insertWithDuplicates(eType v) {
    kType vkey = hashStruct.getKey(v);
    intT h = firstIndex(vkey);
    while (1) {
      eType c;
      c = TA[h];
      if(c==empty && utils::CAS(&TA[h],c,v)) return 1;
      // move to next bucket
      h = incrementIndex(h);
    }
    return 0; // should never get here
  }

  // needs to be more thoroughly tested
  // currently always returns true
  bool deleteVal(kType v) {
    intT i = firstIndex(v);
    int cmp = 1;

    // find first element less than or equal to v in priority order
    intT j = i;
    eType c = TA[j];

    if (c == empty) return true;

    // find first location with priority less or equal to v's priority
    while(c != empty && (cmp = hashStruct.cmp(v,hashStruct.getKey(c))) != 0) {
      j = incrementIndex(j);
      c = TA[j];
    }
    cmp=(c==empty)?1:hashStruct.cmp(v,hashStruct.getKey(c));
    while (1) {
      // Invariants:
      //   v is the key that needs to be deleted
      //   j is our current index into TA
      //   if v appears in TA, then at least one copy must appear at or before j
      //   c = TA[j] at some previous time (could now be changed)
      //   i = h(v)
      //   cmp = compare v to key of c (1 if greater, 0 equal, -1 less)

      if (cmp != 0){//why doesn't the following work as the condition???
	//c==empty || hashStruct.cmp(v,hashStruct.getKey(c)) != 0) {
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
*/

  // Returns the value if an equal value is found in the table
  // otherwise returns the "empty" element.
  // due to prioritization, can quit early if v is greater than cell
  eType find(kType v) {
    intT h = firstIndex(v);
    eType c = TA[h];
    while (1) {
      if (c == empty) return empty;
      else if (!hashStruct.cmp(v,hashStruct.getKey(c)))
	      return c;
      h = incrementIndex(h);
      c = TA[h];
    }
  }

/*
  eType findWithDuplicates(eType v) {
    kType vKey = hashStruct.getKey(v);
    intT h = firstIndex(vKey);
    eType c = TA[h];
    while (1) {
      if (c == empty) {
	return empty;
      } else if (!hashStruct.cmp(vKey,hashStruct.getKey(c))) {
	intT* vCoordinates = (intT *)v.first;
	intT* cCoordinates = (intT *)c.first;
	long i = 0;
	for(;i<g_dim;i++) {
	  if(vCoordinates[i] != cCoordinates[i])
	    break;
	}
	if(i == g_dim) {
	  // never reach here
	  return c;
	}
      }
      h = incrementIndex(h);
      c = TA[h];
    }
  }
*/

  // returns the number of entries
  intT count() {
    return sequence::mapReduce<intT>(TA,m,utils::addF<intT>(),notEmptyF(empty));
  }

/*
  // returns all the current entries compacted into a sequence
  _seq<eType> entries() {
    bool *FL = newA(bool,m);
    parallel_for (0, m, [&](intT i) {
			  FL[i] = (TA[i] != empty);});
    _seq<eType> R = sequence::pack(TA,FL,m);
    free(FL);
    return R;
  }

  // prints the current entries along with the index they are stored at
  void print() {
    cout << "vals = ";
    for (intT i=0; i < m; i++)
      if (TA[i] != empty)
	cout << i << ":" << TA[i] << ",";
    cout << endl;
  }

*/
};

/*
template <class HASH, class ET, class intT>
_seq<ET> removeDuplicates(_seq<ET> S, intT m, HASH hashF) {
  Table<HASH,intT> T(m,hashF,1.0);
  ET* A = S.A;
  parallel_for(0, S.n, [&](intT i) { T.insert(A[i]);});
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

//typedef Table<hashInt> IntTable;
//static IntTable makeIntTable(int m) {return IntTable(m,hashInt());}
template <class intT>
static Table<hashInt<intT>,intT > makeIntTable(intT m, float load) {
  return Table<hashInt<intT>,intT >(m,hashInt<intT>(),load);}

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

template <class intT>
static Table<hashStr,intT> makeStrTable(intT m, float load) {
  return Table<hashStr,intT>(m,hashStr(),load);}

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
    return 0;}//s->second > s2->second;}
};

static _seq<pair<char*,intT>*> removeDuplicates(_seq<pair<char*,intT>*> S) {
  return removeDuplicates(S,hashPair<hashStr,intT>(hashStr()));}

*/

template<typename myPair>
struct hashSimplePair {
  // typedef pair<intT,intT> eType;
  typedef myPair eType;
  typedef intT kType;
  // eType empty() {return pair<intT,intT>(-1,-1);}
  eType empty() {return myPair();}
  kType getKey(eType v) { return v.first; }
  uintT hash(intT s) { return utils::hash(s);}
  int cmp(intT v, intT b) {return (v > b) ? 1 : ((v == b) ? 0 : -1);}
  bool replaceQ(eType s, eType s2) {return 0;}//return s.second > s2.second;}
  bool cas(eType* p, eType o, eType n) {
    return std::atomic_compare_exchange_strong_explicit(
      reinterpret_cast<std::atomic<eType>*>(p), &o, n, std::memory_order_relaxed, std::memory_order_relaxed);
  }
};

/*
// static _seq<pair<intT,intT> > removeDuplicates(_seq<pair<intT,intT> > A) {
//   return removeDuplicates(A,hashSimplePair());
// }

template<typename myPair>
static _seq<myPair> removeDuplicates(_seq<myPair> A) {
  return removeDuplicates(A,hashSimplePair<myPair>());
}
*/

#endif
