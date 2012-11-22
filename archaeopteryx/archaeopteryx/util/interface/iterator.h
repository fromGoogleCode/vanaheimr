/*	\file   iterator.h
	\author Gregory Diamos <solusstultus@gmail.com>
	\date   November 14, 2012
	\brief  The header file for the iterator operations.
*/

#pragma once

// Archaeopteryx Includes
#include <archaeopteryx/util/interface/type_traits.h>

namespace archaeopteryx
{

namespace util
{

struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag       : public input_iterator_tag {};
struct bidirectional_iterator_tag : public forward_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};

template <class _Tp>
struct __has_iterator_category
{
private:
    struct __two {char __lx; char __lxx;};
    template <class _Up> static __two __test(...);
    template <class _Up> static char __test(typename _Up::iterator_category* = 0);
public:
    static const bool value = sizeof(__test<_Tp>(0)) == 1;
};

template <class _Iter, bool> struct ____iterator_traits {};

template <class _Iter>
struct ____iterator_traits<_Iter, true>
{
    typedef typename _Iter::difference_type   difference_type;
    typedef typename _Iter::value_type        value_type;
    typedef typename _Iter::pointer           pointer;
    typedef typename _Iter::reference         reference;
    typedef typename _Iter::iterator_category iterator_category;
};

template <class _Iter, bool> struct __iterator_traits {};

template <class _Iter>
struct __iterator_traits<_Iter, true>
    :  ____iterator_traits
      <
        _Iter,
        is_convertible<typename _Iter::iterator_category, input_iterator_tag>::value ||
        is_convertible<typename _Iter::iterator_category, output_iterator_tag>::value
      >
{};

// iterator_traits<Iterator> will only have the nested types if Iterator::iterator_category
//    exists.  Else iterator_traits<Iterator> will be an empty class.  This is a
//    conforming extension which allows some programs to compile and behave as
//    the client expects instead of failing at compile time.

template <class _Iter>
struct iterator_traits
    : __iterator_traits<_Iter, __has_iterator_category<_Iter>::value> {};

template<class _Tp>
struct iterator_traits<_Tp*>
{
    typedef ptrdiff_t difference_type;
    typedef typename remove_const<_Tp>::type value_type;
    typedef _Tp* pointer;
    typedef _Tp& reference;
    typedef random_access_iterator_tag iterator_category;
};

template <class _Tp, class _Up, bool = __has_iterator_category<iterator_traits<_Tp> >::value>
struct __has_iterator_category_convertible_to
    : public integral_constant<bool, is_convertible<typename iterator_traits<_Tp>::iterator_category, _Up>::value>
{};

template <class _Tp, class _Up>
struct __has_iterator_category_convertible_to<_Tp, _Up, false> : public false_type {};

template <class _Tp>
struct __is_input_iterator : public __has_iterator_category_convertible_to<_Tp, input_iterator_tag> {};

template <class _Tp>
struct __is_forward_iterator : public __has_iterator_category_convertible_to<_Tp, forward_iterator_tag> {};

template <class _Tp>
struct __is_bidirectional_iterator : public __has_iterator_category_convertible_to<_Tp, bidirectional_iterator_tag> {};

template <class _Tp>
struct __is_random_access_iterator : public __has_iterator_category_convertible_to<_Tp, random_access_iterator_tag> {};

template<class _Category, class _Tp, class _Distance = ptrdiff_t,
         class _Pointer = _Tp*, class _Reference = _Tp&>
struct iterator
{
    typedef _Tp        value_type;
    typedef _Distance  difference_type;
    typedef _Pointer   pointer;
    typedef _Reference reference;
    typedef _Category  iterator_category;
};

template <class _InputIter>
inline void __advance(_InputIter& __i,
             typename iterator_traits<_InputIter>::difference_type __n, input_iterator_tag)
{
    for (; __n > 0; --__n)
        ++__i;
}

template <class _BiDirIter>
inline void __advance(_BiDirIter& __i,
             typename iterator_traits<_BiDirIter>::difference_type __n, bidirectional_iterator_tag)
{
    if (__n >= 0)
        for (; __n > 0; --__n)
            ++__i;
    else
        for (; __n < 0; ++__n)
            --__i;
}

template <class _RandIter>
inline void __advance(_RandIter& __i,
             typename iterator_traits<_RandIter>::difference_type __n, random_access_iterator_tag)
{
   __i += __n;
}

template <class _InputIter>
inline void advance(_InputIter& __i,
             typename iterator_traits<_InputIter>::difference_type __n)
{
    __advance(__i, __n, typename iterator_traits<_InputIter>::iterator_category());
}

template <class _InputIter>
inline typename iterator_traits<_InputIter>::difference_type
__distance(_InputIter __first, _InputIter __last, input_iterator_tag)
{
    typename iterator_traits<_InputIter>::difference_type __r(0);
    for (; __first != __last; ++__first)
        ++__r;
    return __r;
}

template <class _RandIter>
inline typename iterator_traits<_RandIter>::difference_type
__distance(_RandIter __first, _RandIter __last, random_access_iterator_tag)
{
    return __last - __first;
}

template <class _InputIter>
inline typename iterator_traits<_InputIter>::difference_type
distance(_InputIter __first, _InputIter __last)
{
    return __distance(__first, __last, typename iterator_traits<_InputIter>::iterator_category());
}

template <class _ForwardIter>
inline _ForwardIter
next(_ForwardIter __x,
     typename iterator_traits<_ForwardIter>::difference_type __n = 1,
     typename enable_if<__is_forward_iterator<_ForwardIter>::value>::type* = 0)
{
    _VSTD::advance(__x, __n);
    return __x;
}

template <class _BidiretionalIter>
inline _BidiretionalIter
prev(_BidiretionalIter __x,
     typename iterator_traits<_BidiretionalIter>::difference_type __n = 1,
     typename enable_if<__is_bidirectional_iterator<_BidiretionalIter>::value>::type* = 0)
{
    _VSTD::advance(__x, -__n);
    return __x;
}

template <class _Iter>
class reverse_iterator
    : public iterator<typename iterator_traits<_Iter>::iterator_category,
                      typename iterator_traits<_Iter>::value_type,
                      typename iterator_traits<_Iter>::difference_type,
                      typename iterator_traits<_Iter>::pointer,
                      typename iterator_traits<_Iter>::reference>
{
private:
    mutable _Iter __t;
protected:
    _Iter current;
public:
    typedef _Iter                                            iterator_type;
    typedef typename iterator_traits<_Iter>::difference_type difference_type;
    typedef typename iterator_traits<_Iter>::reference       reference;
    typedef typename iterator_traits<_Iter>::pointer         pointer;

    reverse_iterator() : current() {}
    explicit reverse_iterator(_Iter __x) : __t(__x), current(__x) {}
    template <class _Up> reverse_iterator(const reverse_iterator<_Up>& __u)
        : __t(__u.base()), current(__u.base()) {}
    _Iter base() const {return current;}
    reference operator*() const {__t = current; return *--__t;}
    pointer  operator->() const {return &(operator*());}
    reverse_iterator& operator++() {--current; return *this;}
    reverse_iterator  operator++(int)
        {reverse_iterator __tmp(*this); --current; return __tmp;}
    reverse_iterator& operator--() {++current; return *this;}
    reverse_iterator  operator--(int)
        {reverse_iterator __tmp(*this); ++current; return __tmp;}
    reverse_iterator  operator+ (difference_type __n) const
        {return reverse_iterator(current - __n);}
    reverse_iterator& operator+=(difference_type __n)
        {current -= __n; return *this;}
    reverse_iterator  operator- (difference_type __n) const
        {return reverse_iterator(current + __n);}
    reverse_iterator& operator-=(difference_type __n)
        {current += __n; return *this;}
    reference         operator[](difference_type __n) const
        {return current[-__n-1];}
};

template <class _Iter1, class _Iter2>
inline bool
operator==(const reverse_iterator<_Iter1>& __x, const reverse_iterator<_Iter2>& __y)
{
    return __x.base() == __y.base();
}

template <class _Iter1, class _Iter2>
inline bool
operator<(const reverse_iterator<_Iter1>& __x, const reverse_iterator<_Iter2>& __y)
{
    return __x.base() > __y.base();
}

template <class _Iter1, class _Iter2>
inline bool
operator!=(const reverse_iterator<_Iter1>& __x, const reverse_iterator<_Iter2>& __y)
{
    return __x.base() != __y.base();
}

template <class _Iter1, class _Iter2>
inline bool
operator>(const reverse_iterator<_Iter1>& __x, const reverse_iterator<_Iter2>& __y)
{
    return __x.base() < __y.base();
}

template <class _Iter1, class _Iter2>
inline bool
operator>=(const reverse_iterator<_Iter1>& __x, const reverse_iterator<_Iter2>& __y)
{
    return __x.base() <= __y.base();
}

template <class _Iter1, class _Iter2>
inline bool
operator<=(const reverse_iterator<_Iter1>& __x, const reverse_iterator<_Iter2>& __y)
{
    return __x.base() >= __y.base();
}

template <class _Iter1, class _Iter2>
inline typename reverse_iterator<_Iter1>::difference_type
operator-(const reverse_iterator<_Iter1>& __x, const reverse_iterator<_Iter2>& __y)
{
    return __y.base() - __x.base();
}

template <class _Iter>
inline reverse_iterator<_Iter>
operator+(typename reverse_iterator<_Iter>::difference_type __n, const reverse_iterator<_Iter>& __x)
{
    return reverse_iterator<_Iter>(__x.base() - __n);
}

template <class _Container>
class back_insert_iterator
    : public iterator<output_iterator_tag,
                      void,
                      void,
                      void,
                      back_insert_iterator<_Container>&>
{
protected:
    _Container* container;
public:
    typedef _Container container_type;

    explicit back_insert_iterator(_Container& __x) : container(&__x) {}
    back_insert_iterator& operator=(const typename _Container::value_type& __value_)
        {container->push_back(__value_); return *this;}
    back_insert_iterator& operator*()     {return *this;}
    back_insert_iterator& operator++()    {return *this;}
    back_insert_iterator  operator++(int) {return *this;}
};

template <class _Container>
inline back_insert_iterator<_Container>
back_inserter(_Container& __x)
{
    return back_insert_iterator<_Container>(__x);
}

template <class _Container>
class front_insert_iterator
    : public iterator<output_iterator_tag,
                      void,
                      void,
                      void,
                      front_insert_iterator<_Container>&>
{
protected:
    _Container* container;
public:
    typedef _Container container_type;

    explicit front_insert_iterator(_Container& __x) : container(&__x) {}
    front_insert_iterator& operator=(const typename _Container::value_type& __value_)
        {container->push_front(__value_); return *this;}
    front_insert_iterator& operator*()     {return *this;}
    front_insert_iterator& operator++()    {return *this;}
    front_insert_iterator  operator++(int) {return *this;}
};

template <class _Container>
inline front_insert_iterator<_Container>
front_inserter(_Container& __x)
{
    return front_insert_iterator<_Container>(__x);
}

template <class _Container>
class insert_iterator
    : public iterator<output_iterator_tag,
                      void,
                      void,
                      void,
                      insert_iterator<_Container>&>
{
protected:
    _Container* container;
    typename _Container::iterator iter;
public:
    typedef _Container container_type;

    insert_iterator(_Container& __x, typename _Container::iterator __i)
        : container(&__x), iter(__i) {}
    insert_iterator& operator=(const typename _Container::value_type& __value_)
        {iter = container->insert(iter, __value_); ++iter; return *this;}
    insert_iterator& operator*()        {return *this;}
    insert_iterator& operator++()       {return *this;}
    insert_iterator& operator++(int)    {return *this;}
};

template <class _Container>
inline insert_iterator<_Container>
inserter(_Container& __x, typename _Container::iterator __i)
{
    return insert_iterator<_Container>(__x, __i);
}


// __wrap_iter

template <class _Iter> class __wrap_iter;

template <class _Iter1, class _Iter2>
bool
operator==(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&);

template <class _Iter1, class _Iter2>
bool
operator<(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&);

template <class _Iter1, class _Iter2>
bool
operator!=(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&);

template <class _Iter1, class _Iter2>
bool
operator>(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&);

template <class _Iter1, class _Iter2>
bool
operator>=(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&);

template <class _Iter1, class _Iter2>
bool
operator<=(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&);

template <class _Iter1, class _Iter2>
typename __wrap_iter<_Iter1>::difference_type
operator-(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&);

template <class _Iter>
__wrap_iter<_Iter>
operator+(typename __wrap_iter<_Iter>::difference_type, __wrap_iter<_Iter>);

template <class _Ip, class _Op> _Op copy(_Ip, _Ip, _Op);
template <class _B1, class _B2> _B2 copy_backward(_B1, _B1, _B2);
template <class _Ip, class _Op> _Op move(_Ip, _Ip, _Op);
template <class _B1, class _B2> _B2 move_backward(_B1, _B1, _B2);

template <class _Tp>
typename enable_if
<
    is_trivially_copy_assignable<_Tp>::value,
    _Tp*
>::type
__unwrap_iter(__wrap_iter<_Tp*>);

template <class _Iter>
class __wrap_iter
{
public:
    typedef _Iter                                                      iterator_type;
    typedef typename iterator_traits<iterator_type>::iterator_category iterator_category;
    typedef typename iterator_traits<iterator_type>::value_type        value_type;
    typedef typename iterator_traits<iterator_type>::difference_type   difference_type;
    typedef typename iterator_traits<iterator_type>::pointer           pointer;
    typedef typename iterator_traits<iterator_type>::reference         reference;
private:
    iterator_type __i;
public:
    __wrap_iter()
    {
    }
    template <class _Up> __wrap_iter(const __wrap_iter<_Up>& __u,
        typename enable_if<is_convertible<_Up, iterator_type>::value>::type* = 0)
        : __i(__u.base())
    {
    }
    reference operator*() const
    {
        return *__i;
    }
    pointer  operator->() const {return &(operator*());}
    __wrap_iter& operator++()
    {
        ++__i;
        return *this;
    }
    __wrap_iter  operator++(int)
        {__wrap_iter __tmp(*this); ++(*this); return __tmp;}
    __wrap_iter& operator--()
    {
        --__i;
        return *this;
    }
    __wrap_iter  operator--(int)
        {__wrap_iter __tmp(*this); --(*this); return __tmp;}
    __wrap_iter  operator+ (difference_type __n) const
        {__wrap_iter __w(*this); __w += __n; return __w;}
    __wrap_iter& operator+=(difference_type __n)
    {
        __i += __n;
        return *this;
    }
    __wrap_iter  operator- (difference_type __n) const
        {return *this + (-__n);}
    __wrap_iter& operator-=(difference_type __n)
        {*this += -__n; return *this;}
    reference        operator[](difference_type __n) const
    {
        return __i[__n];
    }

    iterator_type base() const {return __i;}

private:
    __wrap_iter(iterator_type __x) : __i(__x) {}

    template <class _Up> friend class __wrap_iter;
    template <class _CharT, class _Traits, class _Alloc> friend class basic_string;
    template <class _Tp, class _Alloc> friend class vector;

    template <class _Iter1, class _Iter2>
    friend
    bool
    operator==(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&);

    template <class _Iter1, class _Iter2>
    friend
    bool
    operator<(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&);

    template <class _Iter1, class _Iter2>
    friend
    bool
    operator!=(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&);

    template <class _Iter1, class _Iter2>
    friend
    bool
    operator>(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&);

    template <class _Iter1, class _Iter2>
    friend
    bool
    operator>=(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&);

    template <class _Iter1, class _Iter2>
    friend
    bool
    operator<=(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&);

    template <class _Iter1, class _Iter2>
    friend
    typename __wrap_iter<_Iter1>::difference_type
    operator-(const __wrap_iter<_Iter1>&, const __wrap_iter<_Iter2>&);

    template <class _Iter1>
    friend
    __wrap_iter<_Iter1>
    operator+(typename __wrap_iter<_Iter1>::difference_type, __wrap_iter<_Iter1>);

    template <class _Ip, class _Op> friend _Op copy(_Ip, _Ip, _Op);
    template <class _B1, class _B2> friend _B2 copy_backward(_B1, _B1, _B2);
    template <class _Ip, class _Op> friend _Op move(_Ip, _Ip, _Op);
    template <class _B1, class _B2> friend _B2 move_backward(_B1, _B1, _B2);

    template <class _Tp>
    friend
    typename enable_if
    <
        is_trivially_copy_assignable<_Tp>::value,
        _Tp*
    >::type
    __unwrap_iter(__wrap_iter<_Tp*>);
};

template <class _Iter1, class _Iter2>
inline bool
operator==(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter2>& __y)
{
    return __x.base() == __y.base();
}

template <class _Iter1, class _Iter2>
inline bool
operator<(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter2>& __y)
{
    return __x.base() < __y.base();
}

template <class _Iter1, class _Iter2>
inline bool
operator!=(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter2>& __y)
{
    return !(__x == __y);
}

template <class _Iter1, class _Iter2>
inline bool
operator>(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter2>& __y)
{
    return __y < __x;
}

template <class _Iter1, class _Iter2>
inline bool
operator>=(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter2>& __y)
{
    return !(__x < __y);
}

template <class _Iter1, class _Iter2>
inline bool
operator<=(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter2>& __y)
{
    return !(__y < __x);
}

template <class _Iter1>
inline bool
operator!=(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter1>& __y)
{
    return !(__x == __y);
}

template <class _Iter1>
inline bool
operator>(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter1>& __y)
{
    return __y < __x;
}

template <class _Iter1>
inline bool
operator>=(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter1>& __y)
{
    return !(__x < __y);
}

template <class _Iter1>
inline bool
operator<=(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter1>& __y)
{
    return !(__y < __x);
}

template <class _Iter1, class _Iter2>
inline typename __wrap_iter<_Iter1>::difference_type
operator-(const __wrap_iter<_Iter1>& __x, const __wrap_iter<_Iter2>& __y)
{
    return __x.base() - __y.base();
}

template <class _Iter>
inline __wrap_iter<_Iter>
operator+(typename __wrap_iter<_Iter>::difference_type __n,
          __wrap_iter<_Iter> __x)
{
    __x += __n;
    return __x;
}




template <class _Cp>
inline typename _Cp::iterator
begin(_Cp& __c)
{
    return __c.begin();
}

template <class _Cp>
inline typename _Cp::const_iterator
begin(const _Cp& __c)
{
    return __c.begin();
}

template <class _Cp>
inline typename _Cp::iterator
end(_Cp& __c)
{
    return __c.end();
}

template <class _Cp>
inline typename _Cp::const_iterator
end(const _Cp& __c)
{
    return __c.end();
}

template <class _Tp, size_t _Np>
inline _Tp*
begin(_Tp (&__array)[_Np])
{
    return __array;
}

template <class _Tp, size_t _Np>
inline _Tp*
end(_Tp (&__array)[_Np])
{
    return __array + _Np;
}

}

}


