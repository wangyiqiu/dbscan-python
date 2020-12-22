#ifndef JE_ALLOCATOR_H_
#define JE_ALLOCATOR_H_

#include <bits/c++config.h>
#include <new>
#include <bits/functexcept.h>
#include <bits/move.h>
#if __cplusplus >= 201103L
#include <type_traits>
#endif

#ifdef USEJEMALLOC
#include<jemalloc/jemalloc.h>
#endif

#define JE_MALLOC_FUNC je_custom_prefix_malloc
#define JE_MALLOC_FREE je_custom_prefix_free

using std::size_t;
using std::ptrdiff_t;

/**
 *  @brief  An allocator that uses global new, as per [20.4].
 *  @ingroup allocators
 *
 *  This is precisely the allocator defined in the C++ Standard.
 *    - all allocation calls operator new
 *    - all deallocation calls operator delete
 *
 *  @tparam  _Tp  Type of allocated object.
 */
template<typename _Tp>
  class je_allocator
  {
  public:
    typedef size_t     size_type;
    typedef ptrdiff_t  difference_type;
    typedef _Tp*       pointer;
    typedef const _Tp* const_pointer;
    typedef _Tp&       reference;
    typedef const _Tp& const_reference;
    typedef _Tp        value_type;

    template<typename _Tp1>
      struct rebind
      { typedef je_allocator<_Tp1> other; };

#if __cplusplus >= 201103L
    // _GLIBCXX_RESOLVE_LIB_DEFECTS
    // 2103. propagate_on_container_move_assignment
    typedef std::true_type propagate_on_container_move_assignment;
#endif

    je_allocator() _GLIBCXX_USE_NOEXCEPT { }

    je_allocator(const je_allocator&) _GLIBCXX_USE_NOEXCEPT { }

    template<typename _Tp1>
      je_allocator(const je_allocator<_Tp1>&) _GLIBCXX_USE_NOEXCEPT { }

    ~je_allocator() _GLIBCXX_USE_NOEXCEPT { }

    pointer
    address(reference __x) const _GLIBCXX_NOEXCEPT
    { return std::__addressof(__x); }

    const_pointer
    address(const_reference __x) const _GLIBCXX_NOEXCEPT
    { return std::__addressof(__x); }

    // NB: __n is permitted to be 0.  The C++ standard says nothing
    // about what the return value is when __n == 0.
    pointer
    allocate(size_type __n, const void* = 0)
    {
      if (__n > this->max_size())
        std::__throw_bad_alloc();

#if defined(USEJEMALLOC)
      return static_cast<_Tp*>(JE_MALLOC_FUNC(__n*sizeof(_Tp)));
#else
      return static_cast<_Tp*>(::operator new(__n * sizeof(_Tp)));
#endif
    }

    // __p is not permitted to be a null pointer.
    void
    deallocate(pointer __p, size_type)
    {
#if defined(USEJEMALLOC)
      JE_MALLOC_FREE(__p);
#else
      ::operator delete(__p);
#endif
    }

    size_type
    max_size() const _GLIBCXX_USE_NOEXCEPT
    { return size_t(-1) / sizeof(_Tp); }

#if __cplusplus >= 201103L
    template<typename _Up, typename... _Args>
      void
      construct(_Up* __p, _Args&&... __args)
{ ::new((void *)__p) _Up(std::forward<_Args>(__args)...); }

    template<typename _Up>
      void
      destroy(_Up* __p) { __p->~_Up(); }
#else
    // _GLIBCXX_RESOLVE_LIB_DEFECTS
    // 402. wrong new expression in [some_] allocator::construct
    void
    construct(pointer __p, const _Tp& __val)
    { ::new((void *)__p) _Tp(__val); }

    void
    destroy(pointer __p) { __p->~_Tp(); }
#endif
  };

template<typename _Tp>
  inline bool
  operator==(const je_allocator<_Tp>&, const je_allocator<_Tp>&)
  { return true; }

template<typename _Tp>
  inline bool
  operator!=(const je_allocator<_Tp>&, const je_allocator<_Tp>&)
  { return false; }

#endif
