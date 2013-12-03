/*******
edit_distance: STL and Boost compatible edit distance functions for C++

Copyright (c) 2013 Erik Erlandson

Author:  Erik Erlandson <erikerlandson@yahoo.com>

Distributed under the Boost Software License, Version 1.0.
See accompanying file LICENSE or copy at
http://www.boost.org/LICENSE_1_0.txt
*******/

#include <iostream>

// get the edit_distance() function
#include <boost/algorithm/sequence_alignment/edit_distance.hpp>
using boost::algorithm::sequence_alignment::edit_distance;
using namespace boost::algorithm::sequence_alignment::parameter;


// define a custom cost function where insertion or deletion of space costs nothing
struct cost_free_space {
    // edit_distance uses cost_type to store cost values internally
    // you can define it smaller to save space, or define it as
    // floating point to support non-integer costs, etc.

    // cost_type is inferred from the return values of cost functions if not defined explicitly
    //typedef unsigned cost_type;

    // inserting or deleting a space is free:
    unsigned cost_ins(char c) const { return (c == ' ') ? 0 : 1; }
    unsigned cost_del(char c) const { return (c == ' ') ? 0 : 1; }
    // note that substitution cost also encompasses the definition of equality
    unsigned cost_sub(char c, char d) const { return (c == d) ? 0 : 1; }
};

int main(int argc, char** argv) {
    char const* str1 = " so   many spaces     ";
    char const* str2 = "    so many   spaces ";

    // with custom "free space" cost function, the distance should be zero:
    // here we also enable substitution
    unsigned dist = edit_distance(str1, str2, cost_free_space(), _allow_sub=true);
    std::cout << "The edit distance between \"" << str1 << "\" and \"" << str2 << "\" = " << dist << "\n";    

    return 0;
}
