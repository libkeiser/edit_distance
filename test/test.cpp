/*******
edit_distance: STL and Boost compatible edit distance functions for C++

Copyright (c) 2013 Erik Erlandson

Author:  Erik Erlandson <erikerlandson@yahoo.com>

Distributed under the Boost Software License, Version 1.0.
See accompanying file LICENSE or copy at
http://www.boost.org/LICENSE_1_0.txt
*******/

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE edit_distance
#include <boost/test/unit_test.hpp>

#include "ut_common.h"

#include <boost/mpl/set.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/insert.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/next_prior.hpp>
#include <boost/mpl/plus.hpp>
#include <boost/mpl/arithmetic.hpp>

using boost::mpl::int_;
using boost::mpl::insert;

template <typename Set, typename Arg>
struct union_with {
    typedef typename boost::mpl::insert<Set, Arg>::type type;
};

int f_impl() { return 3; }

struct f_type {
    int operator()() { return f_impl(); }
};

f_type f;

template <typename F, typename Param>
struct f_adaptor_type {
    int operator()() {
        return Param::value + ff();
    }
    F ff;
};

template <typename Param, typename F>
f_adaptor_type<F, Param>
f_adaptor(F func) {
    f_adaptor_type<F, Param> ff;
    ff.ff = func;
    return ff;
}

BOOST_AUTO_TEST_CASE(adaptor) {
    std::cout << f() << "\n";
        //typedef boost::mpl::int_<1> n;
        //std::cout << n::value << "\n";
        //std::cout << boost::mpl::plus<n, boost::mpl::int_<1> >::value << "\n";
    std::cout << f_adaptor_type<f_type, int_<2> >()() << "\n";
    std::cout << f_adaptor<int_<7> >(f)() << "\n";
    std::cout << f_adaptor<int_<7> >(f_adaptor<int_<7> >(f))() << "\n";
}
