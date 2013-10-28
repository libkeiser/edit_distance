/*******
edit_distance: STL and Boost compatible edit distance functions for C++

Copyright (c) 2013 Erik Erlandson

Author:  Erik Erlandson <erikerlandson@yahoo.com>

Distributed under the Boost Software License, Version 1.0.
See accompanying file LICENSE or copy at
http://www.boost.org/LICENSE_1_0.txt
*******/


#if !defined(BOOST_ALGORITHM_SEQUENCE_ALIGNMENT_DETAIL_EDIT_DISTANCE_HPP)
#define BOOST_ALGORITHM_SEQUENCE_ALIGNMENT_DETAIL_EDIT_DISTANCE_HPP

#include <iterator>
#include <set>

// this requires me to link against boost system lib, which disappoints me,
// since it prevents my algorithms from being pure-template.
// maybe figure out how to use a pure-template pool allocator later
#include <boost/pool/object_pool.hpp>

#include <boost/heap/fibonacci_heap.hpp>

#include <boost/range/metafunctions.hpp>

#include <boost/algorithm/sequence_alignment/detail/types.hpp>

namespace boost {
namespace algorithm {
namespace sequence_alignment {
namespace detail {

using boost::begin;
using boost::end;
using boost::range_iterator;

using boost::algorithm::sequence_alignment::detail::path_head;
using boost::algorithm::sequence_alignment::detail::path_lessthan;

template <typename ItrCat1, typename ItrCat2>
struct dijkstra_sssp_cost_dispatch {

template <typename ForwardRange1, typename ForwardRange2, typename Cost>
typename Cost::cost_type 
cost(ForwardRange1 const& seq1, ForwardRange2 const& seq2, Cost& cost) {
    typedef typename Cost::cost_type cost_t;
    typedef typename range_iterator<ForwardRange1 const>::type itr1_t;
    typedef typename range_iterator<ForwardRange2 const>::type itr2_t;
    typedef path_head<itr1_t, itr2_t, cost_t> head_t;
    typedef typename head_t::idx_t idx_t;

    head_t* const hnull = static_cast<head_t*>(NULL);

    const itr1_t end1 = end(seq1);
    const itr2_t end2 = end(seq2);

    std::set<head_t*, visited_lessthan> visited;

    // pool allocators are born for node allocations in graph algorithms
    boost::object_pool<head_t> pool;

    // is fibonacci heap best here?  O(1) insertion seems well suited.
    boost::heap::fibonacci_heap<head_t*, boost::heap::compare<path_lessthan> > heap;

    // maintain an envelope where we have a known-best cost that
    // offers strong path pruning potential.  Runs of 'equal' 
    // provide this kind of opportunity.
    idx_t env1 = 0;
    idx_t env2 = 0;
    cost_t env_best_cost = 0;

    // kick off graph path frontier with initial node:
    heap.push(construct(pool, visited, begin(seq1), begin(seq2), cost_t(0), 0, 0));

    // update frontier from least-cost node at each iteration, until we hit sequence end
    while (true) {
        head_t* h = heap.top();
        heap.pop();
        if (h->idx1 < env1  &&  h->idx2 < env2  &&  h->cost >= env_best_cost) {
            // no edit path from this node can do better than the current
            // known best path, so we can drop this line of exploration
            continue;
        }
        if (h->j1 == end1) {
            // if we are at end of both sequences, then we have our final cost: 
            if (h->j2 == end2) return h->cost;
            // sequence 1 is at end, so only consider insertion from seq2
            itr2_t j2 = h->j2;  ++j2;
            head_t* t = construct(pool, visited, h->j1, j2, h->cost + cost.cost_ins(*(h->j2)), h->idx1, 1+h->idx2);
            if (t != hnull) heap.push(t);
        } else if (h->j2 == end2) {
            // sequence 2 is at end, so only consider deletion from seq1
            itr1_t j1 = h->j1;  ++j1;
            head_t* t = construct(pool, visited, j1, h->j2, h->cost + cost.cost_del(*(h->j1)), 1+h->idx1, h->idx2);
            if (t != hnull) heap.push(t);
        } else {
            // interior of both sequences: consider insertion deletion and sub/eql:
            itr1_t j1 = h->j1;  ++j1;
            itr2_t j2 = h->j2;  ++j2;
            idx_t n = 0;
            while (true) {
                cost_t csub = cost.cost_sub(*(h->j1), *(h->j2));
                head_t* t = construct(pool, visited, j1, j2, h->cost + csub, 1+n+h->idx1, 1+n+h->idx2);
                if (t != hnull  &&  csub <= 0) {
                    // on a run of 'eql', updating the 'best path' envelope will help prune
                    // edit paths that cannot improve on it, for potentially big savings
                    if (t->idx1 > env1) {
                        env1 = t->idx1;
                        env_best_cost = t->cost;
                    }
                    if (t->idx2 > env2) {
                        env2 = t->idx2;
                        env_best_cost = t->cost;
                    }
                }
                if (csub > cost_t(0)  ||  j1 == end1  ||  j2 == end2) {
                    if (t != hnull) heap.push(t);
                    t = construct(pool, visited, h->j1, j2, h->cost + cost.cost_ins(*h->j2), n+h->idx1, 1+n+h->idx2);
                    if (t != hnull) heap.push(t);
                    t = construct(pool, visited, j1, h->j2, h->cost + cost.cost_del(*h->j1), 1+n+h->idx1, n+h->idx2);
                    if (t != hnull) heap.push(t);
                    break;
                }
                ++j1;  ++j2;  ++(h->j1);  ++(h->j2);  ++n;
            }
        }
    }

    // control should not reach here
    return 0;
}

};


template <>
struct dijkstra_sssp_cost_dispatch<std::random_access_iterator_tag, std::random_access_iterator_tag> {

template <typename ForwardRange1, typename ForwardRange2, typename Cost>
typename Cost::cost_type 
cost(ForwardRange1 const& seq1, ForwardRange2 const& seq2, Cost& cost) {
    typedef typename Cost::cost_type cost_t;
    typedef typename range_iterator<ForwardRange1 const>::type itr1_t;
    typedef typename range_iterator<ForwardRange2 const>::type itr2_t;
    typedef path_head<itr1_t, itr2_t, cost_t> head_t;
    typedef typename head_t::idx_t idx_t;

    head_t* const hnull = static_cast<head_t*>(NULL);

    const itr1_t end1 = end(seq1);
    const itr2_t end2 = end(seq2);

    std::set<head_t*, visited_lessthan> visited;

    // pool allocators are born for node allocations in graph algorithms
    boost::object_pool<head_t> pool;

    // is fibonacci heap best here?  O(1) insertion seems well suited.
    boost::heap::fibonacci_heap<head_t*, boost::heap::compare<path_lessthan> > heap;

    // maintain an envelope where we have a known-best cost that
    // offers strong path pruning potential.  Runs of 'equal' 
    // provide this kind of opportunity.
    idx_t env1 = 0;
    idx_t env2 = 0;
    cost_t env_best_cost = 0;

    // kick off graph path frontier with initial node:
    heap.push(construct(pool, visited, begin(seq1), begin(seq2), cost_t(0), 0, 0));

    // update frontier from least-cost node at each iteration, until we hit sequence end
    while (true) {
        head_t* h = heap.top();
        heap.pop();
        if (h->idx1 < env1  &&  h->idx2 < env2  &&  h->cost >= env_best_cost) {
            // no edit path from this node can do better than the current
            // known best path, so we can drop this line of exploration
            continue;
        }
        if (h->j1 == end1) {
            // if we are at end of both sequences, then we have our final cost: 
            if (h->j2 == end2) return h->cost;
            // sequence 1 is at end, so only consider insertion from seq2
            itr2_t j2 = h->j2;  ++j2;
            head_t* t = construct(pool, visited, h->j1, j2, h->cost + cost.cost_ins(*(h->j2)), h->idx1, 1+h->idx2);
            if (t != hnull) heap.push(t);
        } else if (h->j2 == end2) {
            // sequence 2 is at end, so only consider deletion from seq1
            itr1_t j1 = h->j1;  ++j1;
            head_t* t = construct(pool, visited, j1, h->j2, h->cost + cost.cost_del(*(h->j1)), 1+h->idx1, h->idx2);
            if (t != hnull) heap.push(t);
        } else {
            // interior of both sequences: consider insertion deletion and sub/eql:
            itr1_t j1 = h->j1;  ++j1;
            itr2_t j2 = h->j2;  ++j2;
            idx_t n = 0;
            while (true) {
                cost_t csub = cost.cost_sub(*(h->j1), *(h->j2));
                head_t* t = construct(pool, visited, j1, j2, h->cost + csub, 1+n+h->idx1, 1+n+h->idx2);
                if (t != hnull  &&  csub <= 0) {
                    // on a run of 'eql', updating the 'best path' envelope will help prune
                    // edit paths that cannot improve on it, for potentially big savings
                    if (t->idx1 > env1) {
                        env1 = t->idx1;
                        env_best_cost = t->cost;
                    }
                    if (t->idx2 > env2) {
                        env2 = t->idx2;
                        env_best_cost = t->cost;
                    }
                }
                if (csub > cost_t(0)  ||  j1 == end1  ||  j2 == end2) {
                    if (t != hnull) heap.push(t);
                    t = construct(pool, visited, h->j1, j2, h->cost + cost.cost_ins(*h->j2), n+h->idx1, 1+n+h->idx2);
                    if (t != hnull) heap.push(t);
                    t = construct(pool, visited, j1, h->j2, h->cost + cost.cost_del(*h->j1), 1+n+h->idx1, n+h->idx2);
                    if (t != hnull) heap.push(t);
                    break;
                }
                ++j1;  ++j2;  ++(h->j1);  ++(h->j2);  ++n;
            }
        }
    }

    // control should not reach here
    return 0;
}

};


template <typename ForwardRange1, typename ForwardRange2, typename Cost>
typename Cost::cost_type 
dijkstra_sssp_cost(ForwardRange1 const& seq1, ForwardRange2 const& seq2, Cost& cost) {
    typedef typename range_iterator<ForwardRange1 const>::type itr1_t;
    typedef typename range_iterator<ForwardRange2 const>::type itr2_t;

    typedef typename std::iterator_traits<itr1_t>::iterator_category cat1;
    typedef typename std::iterator_traits<itr2_t>::iterator_category cat2;

    return dijkstra_sssp_cost_dispatch<cat1, cat2>().cost(seq1, seq2, cost);
}

}}}}

#endif
