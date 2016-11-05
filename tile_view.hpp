/*
 * tile_array.hpp
 *
 *  Created on: Nov 4, 2016
 *      Author: pfandedd
 */

#pragma once

#include <vector>

#include "tile_array.hpp"

namespace memory_layout {

template<size_t dim, typename T>
class tile_view {
private:
    std::vector<T> &tiled;
    size_t base_offset;
    size_t tile_index[dim];
    std::vector<tiling_info_dim> tiling_info;
    size_t tile_size;

    template<size_t cur_dim>
    typename std::enable_if<cur_dim == dim, T&>::type operator_call_dim(size_t (&inner_index)[dim]) {
        return tiled[base_offset + flat_index(inner_index)];
    }

    template<size_t cur_dim, typename index_type, typename ... other_index_types>
    typename std::enable_if<cur_dim != dim, T&>::type operator_call_dim(size_t (&inner_index)[dim], index_type index,
            other_index_types ... other_indices) {
        inner_index[cur_dim] = index;
        return operator_call_dim<cur_dim + 1>(inner_index, other_indices...);
    }
public:
    tile_view(std::vector<T> &tiled, size_t (&tile_index)[dim], std::vector<tiling_info_dim> tiling_info) :
            tiled(tiled), tiling_info(tiling_info) {

        this->tile_index[0] = tile_index[0];
        this->tile_index[1] = tile_index[1];

        tile_size = 1;
        for (size_t d = 0; d < dim; d++) {
            tile_size *= tiling_info[d].tile_size_dir;
        }

        size_t cur_stride = 1;
        base_offset = 0;
        for (size_t d = 0; d < dim; d++) {
            base_offset += tile_index[(dim - 1) - d] * cur_stride;
            cur_stride *= (tiling_info[(dim - 1) - d].stride / tiling_info[(dim - 1) - d].tile_size_dir);
        }

        base_offset *= tile_size;
    }

    T& operator[](size_t tile_offset) {
        return tiled[base_offset + tile_offset];
    }

    template<typename ... index_types>
    T& operator()(index_types ... indices) {
        size_t inner_index[dim];
        return operator_call_dim<0>(inner_index, indices...);
    }

    size_t size() {
        return tile_size;
    }

    size_t flat_index(const size_t (&coord)[dim]) {
        size_t inner_flat_index = 0;
        size_t cur_stride = 1;
        for (size_t d = 0; d < dim; d++) {
            inner_flat_index += coord[(dim - 1) - d] * cur_stride;
            cur_stride *= tiling_info[(dim - 1) - d].tile_size_dir;
        }
        return inner_flat_index;
    }
};

}
