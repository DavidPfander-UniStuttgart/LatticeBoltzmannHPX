/*
 * tile_array.hpp
 *
 *  Created on: Nov 4, 2016
 *      Author: pfandedd
 */

#pragma once

#include <vector>

#include "index_iterator.hpp"

namespace memory_layout {

template<size_t dim, size_t remaining_dim, size_t cur_dim, typename T, typename F>
typename std::enable_if<remaining_dim == 0, void>::type execute_looped(const T (&min)[dim], const T (&max)[dim],
        const T (&step)[dim], T (&completed_index)[dim], F f) {
    f(completed_index);
}

template<size_t dim, size_t remaining_dim, size_t cur_dim, typename T, typename F>
typename std::enable_if<remaining_dim != 0, void>::type execute_looped(const T (&min)[dim], const T (&max)[dim],
        const T (&step)[dim], T (&partial_index)[dim], F f) {
    for (T cur = min[cur_dim]; cur < max[cur_dim]; cur += step[cur_dim]) {
        partial_index[cur_dim] = cur;
        execute_looped<dim, remaining_dim - 1, cur_dim + 1>(min, max, step, partial_index, f);
    }
}

template<size_t dim, typename T, typename F>
void loop_nest(const T (&min)[dim], const T (&max)[dim], const T (&step)[dim], F f) {
    T partial_index[dim];
    execute_looped<dim, dim, 0>(min, max, step, partial_index, f);
}

struct tiling_info_dim {
    size_t tile_size_dir;
    size_t stride;
};

template<size_t dim, size_t remaining_dim, size_t cur_dim, typename T>
typename std::enable_if<remaining_dim == 0, void>::type tile_dim(std::vector<T> &tiled, const std::vector<T> &org,
        const std::vector<tiling_info_dim> &tiling_info, size_t (&tile_index)[dim]) {

    std::cout << "tile_index: ";
    for (size_t d = 0; d < dim; d++) {
        if (d > 0) {
            std::cout << ", ";
        }
        std::cout << tile_index[d];
    }
    std::cout << std::endl;

    size_t cur_stride = 1;
    size_t skipped_blocks = 0;
    for (size_t d = 0; d < dim; d++) {
        skipped_blocks += tile_index[(dim - 1) - d] * cur_stride;
        cur_stride *= (tiling_info[(dim - 1) - d].stride / tiling_info[(dim - 1) - d].tile_size_dir);
    }

    std::cout << "skipped blocks: " << skipped_blocks << std::endl;

    for (size_t d = 0; d < dim; d++) {
        skipped_blocks *= tiling_info[d].tile_size_dir;
    }

    size_t min[dim];
    for (size_t d = 0; d < dim; d++) {
        min[d] = 0;
    }
    size_t max[dim];
    for (size_t d = 0; d < dim; d++) {
        max[d] = tiling_info[d].tile_size_dir;
    }
    size_t stride[dim];
    for (size_t d = 0; d < dim; d++) {
//        stride[d] = tiling_info[d].tile_size_dir;
        stride[d] = 1;
    }

    std::cout << "skipped cells: " << skipped_blocks << std::endl;

//    std::cout << "min: ";
//    for (size_t d = 0; d < dim; d++) {
//        if (d > 0) {
//            std::cout << ", ";
//        }
//        std::cout << min[d];
//    }
//    std::cout << std::endl;
//
//    std::cout << "max: ";
//    for (size_t d = 0; d < dim; d++) {
//        if (d > 0) {
//            std::cout << ", ";
//        }
//        std::cout << max[d];
//    }
//    std::cout << std::endl;

    loop_nest<dim>(min, max, stride,
            [&tiled, &org, &tiling_info, &tile_index, skipped_blocks](const size_t (&inner_index)[dim]) {
//                std::cout << "inner_index: ";
//                for (size_t d = 0; d < dim; d++) {
//                    if (d > 0) {
//                        std::cout << ", ";
//                    }
//                    std::cout << inner_index[d];
//                }
//                std::cout << std::endl;

                size_t inner_flat_index = 0;
                size_t cur_stride = 1;
                for (size_t d = 0; d < dim; d++) {
                    inner_flat_index += inner_index[(dim - 1) - d] * cur_stride;
                    cur_stride *= tiling_info[(dim - 1) - d].tile_size_dir;
                }
                size_t original_flat_index = 0;
                cur_stride = 1;

                std::cout << "org index: ";
                for (size_t d = 0; d < dim; d++) {
                    if (d > 0) {
                        std::cout << ", ";
                    }
                    std::cout << (tile_index[(dim - 1) - d] *  tiling_info[(dim - 1) - d].tile_size_dir + inner_index[(dim - 1) - d]);
                    original_flat_index += (tile_index[(dim - 1) - d] *  tiling_info[(dim - 1) - d].tile_size_dir + inner_index[(dim - 1) - d]) * cur_stride;
                    cur_stride *= tiling_info[(dim - 1) - d].stride;
                }
                std::cout << std::endl;
                std::cout << "inner flat index: " << inner_flat_index << std::endl;
                std::cout << "inner skipped cells: " << (skipped_blocks + inner_flat_index) << std::endl;
                std::cout << "original flat index: " << (original_flat_index) << std::endl;
                tiled[skipped_blocks + inner_flat_index] = org[original_flat_index];
            });
}

template<size_t dim, size_t remaining_dim, size_t cur_dim, typename T>
typename std::enable_if<remaining_dim != 0, void>::type tile_dim(std::vector<T> &tiled, const std::vector<T> &org,
        const std::vector<tiling_info_dim> &tiling_info, size_t (&partial_tile_index)[dim]) {
    const tiling_info_dim &cur_info = tiling_info[cur_dim];
//    std::cout << "dim d=" << cur_dim << " to: " << (cur_info.stride / cur_info.tile_size_dir) << std::endl;
    for (size_t tile_index_1d = 0; tile_index_1d < cur_info.stride / cur_info.tile_size_dir; tile_index_1d += 1) {
        partial_tile_index[cur_dim] = tile_index_1d;
        tile_dim<dim, remaining_dim - 1, cur_dim + 1>(tiled, org, tiling_info, partial_tile_index);
    }
}

// dimension of matrix, dimension of tiles
template<size_t dim, typename T>
std::vector<T> make_tiled(std::vector<T> &org, const std::vector<tiling_info_dim> &tiling_info) {
    std::vector<T> tiled;
    tiled.resize(org.size());
    if (tiling_info.size() != dim) {
        throw;
    }

    size_t tile_index[dim];
    tile_dim<dim, dim, 0>(tiled, org, tiling_info, tile_index);

    return tiled;
}

}
