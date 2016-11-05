/*
 * loop_nest.hpp
 *
 *  Created on: Nov 5, 2016
 *      Author: pfandedd
 */

#pragma once

namespace util {

namespace detail {

template<size_t dim, size_t cur_dim, typename T, typename F>
typename std::enable_if<cur_dim == dim, void>::type execute_looped(const T (&min)[dim], const T (&max)[dim],
        const T (&step)[dim], T (&completed_index)[dim], F f) {
    f(completed_index);
}

template<size_t dim, size_t cur_dim, typename T, typename F>
typename std::enable_if<cur_dim != dim, void>::type execute_looped(const T (&min)[dim], const T (&max)[dim],
        const T (&step)[dim], T (&partial_index)[dim], F f) {
    for (T cur = min[cur_dim]; cur < max[cur_dim]; cur += step[cur_dim]) {
        partial_index[cur_dim] = cur;
        execute_looped<dim, cur_dim + 1>(min, max, step, partial_index, f);
    }
}

}

template<size_t dim, typename T, typename F>
void loop_nest(const T (&min)[dim], const T (&max)[dim], const T (&step)[dim], F f) {
    T partial_index[dim];
    detail::execute_looped<dim, 0>(min, max, step, partial_index, f);
}

}
