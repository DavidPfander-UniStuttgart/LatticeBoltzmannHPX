#include "grid2d_algorithms.hpp"

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cmath>

#include <hpx/hpx.hpp>

#include "index_iterator.hpp"

#include "types.hpp"
#include "memory_layout.hpp"

// include boost last to avoid hpx complaining
#include <boost/tokenizer.hpp>
#include <boost/range/functions.hpp>

namespace lattice {

//
// NW=0 N=1 NE=2
// W=3 C=4 E=5
// SW=6 S=7 SE=8

const double grid2d_algorithms::SPEEDS_X[DIRECTIONS_2D] = { -C, 0, C, -C, 0, C, -C, 0, C };
const double grid2d_algorithms::SPEEDS_Y[DIRECTIONS_2D] = { -C, -C, -C, 0, 0, 0, C, C, C };
const double grid2d_algorithms::weights[DIRECTIONS_2D] = { 1.0 / 36.0, 1.0 / 9.0, 1.0 / 36.0, 1.0 / 9.0, 4.0 / 9.0, 1.0
        / 9.0, 1.0 / 36.0, 1.0 / 9.0, 1.0 / 36.0 };

grid2d_algorithms grid2d_algorithms::from_file(std::string file_name, bool verbose) {
    std::stringstream ss;
    ss << std::ifstream(file_name).rdbuf();

//    std::string s = "Boost C++ Libraries";

    std::string content = ss.str();
    boost::char_separator<char> sep(" \n");
    typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
    tokenizer tok(content, sep);
    enum class STATE {
        XDIM, YDIM, LINE
    };
    STATE state = STATE::XDIM;

    size_t x_size;
    size_t y_size;
    std::vector<lattice::CELL_TYPES> cells;

    size_t row = 0;
    size_t col = 0;
    for (const auto &t : tok) {
//        std::cout << "token: \"" << t << "\"" << std::endl;

        std::string token = t;
        if (state == STATE::XDIM) {
            std::stringstream s;
            s << t;
            s >> x_size;
            state = STATE::YDIM;
        } else if (state == STATE::YDIM) {
            std::stringstream s;
            s << t;
            s >> y_size;
            cells.resize(x_size * y_size);
            state = STATE::LINE;
        } else if (state == STATE::LINE) {
            if (token.compare("B") == 0) {
                cells[row * y_size + col] = lattice::CELL_TYPES::BORDER;
            } else if (token.compare("W") == 0) {
                cells[row * y_size + col] = lattice::CELL_TYPES::WATER;
            } else if (t.compare("D") == 0) {
                cells[row * y_size + col] = lattice::CELL_TYPES::DRAIN;
            } else if (t.compare("S") == 0) {
                cells[row * y_size + col] = lattice::CELL_TYPES::SOURCE;
            }
            col++;
            if (col >= y_size) {
                col = 0;
                row++;
            }
        }
    }

//    std::vector<memory_layout::tiling_info_dim> tiling_information;
//
//    memory_layout::tiling_info_dim tiling_info_y;
//
//    memory_layout::tiling_info_dim tiling_info_x;
//    tiling_info_x.stride = x_size;
//    tiling_info_x.tile_size_dir = 10;
//    tiling_information.push_back(tiling_info_x);
//
//    tiling_info_y.stride = y_size;
//    tiling_info_y.tile_size_dir = 10;
//    tiling_information.push_back(tiling_info_y);
//
//    std::cout << "x_size: " << x_size << std::endl;
//    std::cout << "y_size: " << y_size << std::endl;
//    for (size_t x = 0; x < x_size; x++) {
//        if (x < 10) {
//            std::cout << " ";
//        }
//        std::cout << x << "| ";
//        for (size_t y = 0; y < y_size; y++) {
//            if (y > 0) {
//                std::cout << " ";
//            }
//            if (cells[x * y_size + y] == CELL_TYPES::BORDER) {
//                std::cout << "B";
//            } else if (cells[x * y_size + y] == CELL_TYPES::DRAIN) {
//                std::cout << "D";
//            } else if (cells[x * y_size + y] == CELL_TYPES::SOURCE) {
//                std::cout << "S";
//            } else if (cells[x * y_size + y] == CELL_TYPES::WATER) {
//                std::cout << "W";
//            }
//        }
//        std::cout << std::endl;
//    }
//    std::cout << "----------------------------------------" << std::endl;
//
//    std::vector<lattice::CELL_TYPES> cells_tiled = memory_layout::make_tiled<2>(cells, tiling_information);
//
//    std::cout << "tiled:" << std::endl;
//    for (size_t x = 0; x < x_size; x++) {
//        if (x < 10) {
//            std::cout << " ";
//        }
//        std::cout << x << "| ";
//        for (size_t y = 0; y < y_size; y++) {
//            if (y > 0) {
//                std::cout << " ";
//            }
//            if (cells_tiled[x * y_size + y] == CELL_TYPES::BORDER) {
//                std::cout << "B";
//            } else if (cells_tiled[x * y_size + y] == CELL_TYPES::DRAIN) {
//                std::cout << "D";
//            } else if (cells_tiled[x * y_size + y] == CELL_TYPES::SOURCE) {
//                std::cout << "S";
//            } else if (cells_tiled[x * y_size + y] == CELL_TYPES::WATER) {
//                std::cout << "W";
//            }
//        }
//        std::cout << std::endl;
//    }
//    std::cout << "----------------------------------------" << std::endl;

//    for (size_t tile_x = 0; tile_x < x_size / 10; tile_x++) {
//        for (size_t tile_y = 0; tile_y < y_size / 10; tile_y++) {
//            size_t tile_index[2];
//            tile_index[0] = tile_x;
//            tile_index[1] = tile_y;
//            memory_layout::tile_view<2, lattice::CELL_TYPES> v(cells_tiled, tile_index, tiling_information);
//            for (size_t x = 0; x < 10; x++) {
//                for (size_t y = 0; y < 10; y++) {
//                    if (y > 0) {
//                        std::cout << " ";
//                    }
//                    if (v[x * 10 + y] == CELL_TYPES::BORDER) {
//                        std::cout << "B";
//                    } else if (v[x * 10 + y] == CELL_TYPES::DRAIN) {
//                        std::cout << "D";
//                    } else if (v[x * 10 + y] == CELL_TYPES::SOURCE) {
//                        std::cout << "S";
//                    } else if (v[x * 10 + y] == CELL_TYPES::WATER) {
//                        std::cout << "W";
//                    }
//                }
//                std::cout << std::endl;
//            }
//            std::cout << "----------------------------------------------------------" << std::endl;
//        }
//    }

//    memory_layout::iterate_tiles<2>(cells_tiled, tiling_information,
//            [](memory_layout::tile_view<2, lattice::CELL_TYPES> v) {
//                for (size_t x = 0; x < 10; x++) {
//                    for (size_t y = 0; y < 10; y++) {
//                        if (y > 0) {
//                            std::cout << " ";
//                        }
//                        if (v(x, y) == CELL_TYPES::BORDER) {
//                            std::cout << "B";
//                        } else if (v(x, y) == CELL_TYPES::DRAIN) {
//                            std::cout << "D";
//                        } else if (v(x, y) == CELL_TYPES::SOURCE) {
//                            std::cout << "S";
//                        } else if (v(x, y) == CELL_TYPES::WATER) {
//                            std::cout << "W";
//                        }
//                    }
//                    std::cout << std::endl;
//                }
//                std::cout << "----------------------------------------------------------" << std::endl;
//            });

//    memory_layout::iterate_tiles<2>(cells_tiled, tiling_information,
//            [](memory_layout::tile_view<2, lattice::CELL_TYPES> v) {
//
//                size_t row_start[2];
//
//                for (size_t x = 0; x < 10; x++) {
//                    row_start[0] = x;
//                    row_start[1] = 0;
//
//                    memory_layout::tile_view_iterator<2, CELL_TYPES> v_iterator(v, row_start);
//
//                    bool first = true;
//                    hpx::parallel::for_each_n(hpx::parallel::seq, v_iterator, 10, [&first](const CELL_TYPES &cell_type) {
//                                if (first) {
//                                    first = false;
//                                } else {
//                                    std::cout << " ";
//                                }
//                                if (cell_type == CELL_TYPES::BORDER) {
//                                    std::cout << "B";
//                                } else if (cell_type == CELL_TYPES::DRAIN) {
//                                    std::cout << "D";
//                                } else if (cell_type == CELL_TYPES::SOURCE) {
//                                    std::cout << "S";
//                                } else if (cell_type == CELL_TYPES::WATER) {
//                                    std::cout << "W";
//                                }
//                            });
//                    std::cout << std::endl;
//                }

//                size_t col_counter = 0;
//                hpx::parallel::for_each_n(hpx::parallel::seq, v_iterator, 100, [&col_counter](const CELL_TYPES &cell_type) {
//                            if (col_counter > 0) {
//                                std::cout << " ";
//                            }
//                            if (cell_type == CELL_TYPES::BORDER) {
//                                std::cout << "B";
//                            } else if (cell_type == CELL_TYPES::DRAIN) {
//                                std::cout << "D";
//                            } else if (cell_type == CELL_TYPES::SOURCE) {
//                                std::cout << "S";
//                            } else if (cell_type == CELL_TYPES::WATER) {
//                                std::cout << "W";
//                            }
//
//                            col_counter += 1;
//                            if (col_counter == 10) {
//                                std::cout << std::endl;
//                                col_counter = 0;
//                            }
//                        });

//                for (size_t x = 0; x < 10; x++) {
//                    for (size_t y = 0; y < 10; y++) {
//                        if (y > 0) {
//                            std::cout << " ";
//                        }
//                        if (v(x, y) == CELL_TYPES::BORDER) {
//                            std::cout << "B";
//                        } else if (v(x, y) == CELL_TYPES::DRAIN) {
//                            std::cout << "D";
//                        } else if (v(x, y) == CELL_TYPES::SOURCE) {
//                            std::cout << "S";
//                        } else if (v(x, y) == CELL_TYPES::WATER) {
//                            std::cout << "W";
//                        }
//                    }
//                    std::cout << std::endl;
//                }
//                std::cout << "----------------------------------------------------------" << std::endl;
//            });

    return grid2d_algorithms(x_size, y_size, cells, verbose);
}

void grid2d_algorithms::serialize_as_csv(const std::string &file_name) {
    std::ofstream myfile;
    myfile.open(file_name);
    const std::string sep(",");
    myfile << "x" << sep << "y" << sep << "z (dummy)" << sep << "value" << std::endl;
    for (size_t x = 0; x < x_size; x++) {
        for (size_t y = 0; y < y_size; y++) {
            double mass_density = get_mass_density(x, y);
            if (mass_density > 0.0) {
                double momentum_density[2];
                get_momentum_density(x, y, momentum_density);

                double u[2] = { momentum_density[0] / mass_density, momentum_density[1] / mass_density };
                double velocity = sqrt(u[0] * u[0] + u[1] * u[1]);
                myfile << x << sep << y << sep << 0.0 << sep << velocity << std::endl;
            } else {
                myfile << x << sep << y << sep << 0.0 << sep << 0.0 << std::endl;
            }
        }
    }
    myfile.close();
}

size_t grid2d_algorithms::get_pop_index(int64_t x, int64_t y) {
    return (x + 1) * (y_size + 2) + (y + 1);
}

size_t grid2d_algorithms::get_cell_index(int64_t x, int64_t y) {
    return (x + 1) * (y_size + 2) + (y + 1);
}

size_t grid2d_algorithms::get_row_index_unpadded(int64_t x) {
    return (x + 1) * (y_size + 2);
}

size_t grid2d_algorithms::get_row_index_padded(int64_t x) {
    return (x + 1) * (y_size + 2) + 1;
}

grid2d_algorithms::grid2d_algorithms(size_t x_size, size_t y_size, std::vector<lattice::CELL_TYPES> &cells_unpadded,
        bool verbose) :
        verbose(verbose), x_size(x_size), y_size(y_size) {
    populations = std::make_unique<std::array<std::vector<double>, DIRECTIONS_2D>>();
    new_populations = std::make_unique<std::array<std::vector<double>, DIRECTIONS_2D>>();
    for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
        (*populations)[dir].resize((x_size + 2) * (y_size + 2));
        std::fill((*populations)[dir].begin(), (*populations)[dir].end(), 0.0);
    }
    for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
        (*new_populations)[dir].resize((x_size + 2) * (y_size + 2));
        std::fill((*new_populations)[dir].begin(), (*new_populations)[dir].end(), 0.0);
    }

    cells.resize((x_size + 2) * (y_size + 2));
    for (int64_t x = -1; x < static_cast<int64_t>(x_size + 1); x++) {
        for (int64_t y = -1; y < static_cast<int64_t>(y_size + 1); y++) {
            if (x >= 0 && x < static_cast<int64_t>(x_size) && y >= 0 && y < static_cast<int64_t>(y_size)) {
                get_cell(x, y) = cells_unpadded[x * y_size + y];
            } else {
                get_cell(x, y) = CELL_TYPES::WATER;
            }
        }
    }

    for (size_t x = 0; x < x_size; x++) {
        for (size_t y = 0; y < y_size; y++) {
            if (get_cell(x, y) == CELL_TYPES::WATER || get_cell(x, y) == CELL_TYPES::DRAIN
                    || get_cell(x, y) == CELL_TYPES::BORDER) {
                get_population(x, y, 4) = 0.1;
                get_new_population(x, y, 4) = 0.1;
            } else if (get_cell(x, y) == CELL_TYPES::SOURCE) {
                initialize_cell(x, y, 0.5);
            }
        }
    }
}

void grid2d_algorithms::step() {

    // switch arrays
    std::swap(populations, new_populations);

    //TODO: is that efficient? check assembly
    for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
        std::fill((*new_populations)[dir].begin(), (*new_populations)[dir].end(), 0.0);
    }

    collide();
    stream();
    boundary();
    source();
    drain();

    if (verbose) {
        hpx::cout << "step complete" << std::endl;
    }
}

void grid2d_algorithms::get_momentum_density(
        hpx::util::tuple<double &, double &, double &, double &, double &, double &, double &, double &, double &,
                CELL_TYPES> &t, double (&momentum_density)[2]) {
    momentum_density[0] = 0.0;
    momentum_density[1] = 0.0;

    momentum_density[0] += hpx::util::get<0>(t) * grid2d_algorithms::SPEEDS_X[0];
    momentum_density[0] += hpx::util::get<1>(t) * grid2d_algorithms::SPEEDS_X[1];
    momentum_density[0] += hpx::util::get<2>(t) * grid2d_algorithms::SPEEDS_X[2];
    momentum_density[0] += hpx::util::get<3>(t) * grid2d_algorithms::SPEEDS_X[3];
    momentum_density[0] += hpx::util::get<4>(t) * grid2d_algorithms::SPEEDS_X[4];
    momentum_density[0] += hpx::util::get<5>(t) * grid2d_algorithms::SPEEDS_X[5];
    momentum_density[0] += hpx::util::get<6>(t) * grid2d_algorithms::SPEEDS_X[6];
    momentum_density[0] += hpx::util::get<7>(t) * grid2d_algorithms::SPEEDS_X[7];
    momentum_density[0] += hpx::util::get<8>(t) * grid2d_algorithms::SPEEDS_X[8];

    momentum_density[1] += hpx::util::get<0>(t) * grid2d_algorithms::SPEEDS_Y[0];
    momentum_density[1] += hpx::util::get<1>(t) * grid2d_algorithms::SPEEDS_Y[1];
    momentum_density[1] += hpx::util::get<2>(t) * grid2d_algorithms::SPEEDS_Y[2];
    momentum_density[1] += hpx::util::get<3>(t) * grid2d_algorithms::SPEEDS_Y[3];
    momentum_density[1] += hpx::util::get<4>(t) * grid2d_algorithms::SPEEDS_Y[4];
    momentum_density[1] += hpx::util::get<5>(t) * grid2d_algorithms::SPEEDS_Y[5];
    momentum_density[1] += hpx::util::get<6>(t) * grid2d_algorithms::SPEEDS_Y[6];
    momentum_density[1] += hpx::util::get<7>(t) * grid2d_algorithms::SPEEDS_Y[7];
    momentum_density[1] += hpx::util::get<8>(t) * grid2d_algorithms::SPEEDS_Y[8];
}

double grid2d_algorithms::get_mass_density(
        hpx::util::tuple<double &, double &, double &, double &, double &, double &, double &, double &, double &,
                CELL_TYPES> &t) {
    double mass_density = 0.0;
    mass_density += hpx::util::get<0>(t);
    mass_density += hpx::util::get<1>(t);
    mass_density += hpx::util::get<2>(t);
    mass_density += hpx::util::get<3>(t);
    mass_density += hpx::util::get<4>(t);
    mass_density += hpx::util::get<5>(t);
    mass_density += hpx::util::get<6>(t);
    mass_density += hpx::util::get<7>(t);
    mass_density += hpx::util::get<8>(t);
    return mass_density;
}

void grid2d_algorithms::get_momentum_density(size_t x, size_t y, double (&momentum_density)[2]) {
    momentum_density[0] = 0.0;
    momentum_density[1] = 0.0;
    for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
        momentum_density[0] += get_population(x, y, dir) * grid2d_algorithms::SPEEDS_X[dir];
    }
    for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
        momentum_density[1] += get_population(x, y, dir) * grid2d_algorithms::SPEEDS_Y[dir];
    }
}

double grid2d_algorithms::get_mass_density(size_t x, size_t y) {
    double mass_density = 0.0;
    for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
        mass_density += get_population(x, y, dir);
    }
    return mass_density;
}

double grid2d_algorithms::calculate_equilibrium(const size_t dir, const double mass_density, const double (&u)[2]) {
    double u2 = u[0] * u[0] + u[1] * u[1];
    double vu = SPEEDS_X[dir] * u[0] + SPEEDS_Y[dir] * u[1];
    return mass_density * weights[dir] * (1.0 + 3.0 * vu + 4.5 * vu * vu - 1.5 * u2);
}

void grid2d_algorithms::collide() {

    auto zip_it_begin = hpx::util::make_zip_iterator(boost::begin((*populations)[0]), boost::begin((*populations)[1]),
            boost::begin((*populations)[2]), boost::begin((*populations)[3]), boost::begin((*populations)[4]),
            boost::begin((*populations)[5]), boost::begin((*populations)[6]), boost::begin((*populations)[7]),
            boost::begin((*populations)[8]), boost::begin(cells));

    auto zip_it_end = hpx::util::make_zip_iterator(boost::end((*populations)[0]), boost::end((*populations)[1]),
            boost::end((*populations)[2]), boost::end((*populations)[3]), boost::end((*populations)[4]),
            boost::end((*populations)[5]), boost::end((*populations)[6]), boost::end((*populations)[7]),
            boost::end((*populations)[8]), boost::end(cells));

    hpx::parallel::for_each(hpx::parallel::par, zip_it_begin, zip_it_end,
            [this](hpx::util::tuple<
                    double &, double &, double &,
                    double &, double &, double &,
                    double &, double &, double &,
                    CELL_TYPES> t) {

                if (hpx::util::get<9>(t) == CELL_TYPES::WATER) {
                    double mass_density = get_mass_density(t);
                    double momentum_density[2];
                    get_momentum_density(t, momentum_density);
                    double u[2] = {momentum_density[0] / mass_density, momentum_density[1] / mass_density};

                    hpx::util::get<0>(t) = std::max(0.0, (1 - OMEGA) * hpx::util::get<0>(t) + OMEGA * calculate_equilibrium(0, mass_density, u));
                    hpx::util::get<1>(t) = std::max(0.0, (1 - OMEGA) * hpx::util::get<1>(t) + OMEGA * calculate_equilibrium(1, mass_density, u));
                    hpx::util::get<2>(t) = std::max(0.0, (1 - OMEGA) * hpx::util::get<2>(t) + OMEGA * calculate_equilibrium(2, mass_density, u));
                    hpx::util::get<3>(t) = std::max(0.0, (1 - OMEGA) * hpx::util::get<3>(t) + OMEGA * calculate_equilibrium(3, mass_density, u));
                    hpx::util::get<4>(t) = std::max(0.0, (1 - OMEGA) * hpx::util::get<4>(t) + OMEGA * calculate_equilibrium(4, mass_density, u));
                    hpx::util::get<5>(t) = std::max(0.0, (1 - OMEGA) * hpx::util::get<5>(t) + OMEGA * calculate_equilibrium(5, mass_density, u));
                    hpx::util::get<6>(t) = std::max(0.0, (1 - OMEGA) * hpx::util::get<6>(t) + OMEGA * calculate_equilibrium(6, mass_density, u));
                    hpx::util::get<7>(t) = std::max(0.0, (1 - OMEGA) * hpx::util::get<7>(t) + OMEGA * calculate_equilibrium(7, mass_density, u));
                    hpx::util::get<8>(t) = std::max(0.0, (1 - OMEGA) * hpx::util::get<8>(t) + OMEGA * calculate_equilibrium(8, mass_density, u));
                }
            });
}

void grid2d_algorithms::initialize_cell(size_t x, size_t y, double factor) {
    for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
        get_new_population(x, y, dir) = factor * (1.0 / DIRECTIONS_2D);
    }
}

void grid2d_algorithms::source() {
    index_iterator::blocking_pseudo_execution_policy<size_t> policy(2);
    policy.add_blocking( { 1, y_size }, { true, false });

    index_iterator::iterate_indices<2>(policy, { 0, 0 }, { x_size, y_size }, [this](size_t x, size_t y) {
        if (get_cell(x, y) == CELL_TYPES::SOURCE) {
            initialize_cell(x, y, 0.5);
        }
    });
}

void grid2d_algorithms::drain() {
    index_iterator::blocking_pseudo_execution_policy<size_t> policy(2);
    policy.add_blocking( { 1, y_size }, { true, false });

    index_iterator::iterate_indices<2>(policy, { 0, 0 }, { x_size, y_size }, [this](size_t x, size_t y) {
        if (get_cell(x, y) == CELL_TYPES::DRAIN) {
            for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
                get_new_population(x, y, dir) = 0.0;
            }
        }
    });
}

// fake boundary treatment, only physical by accident
void grid2d_algorithms::boundary() {
    index_iterator::blocking_pseudo_execution_policy<size_t> policy(2);
    policy.add_blocking( { 1, y_size }, { true, false });

    index_iterator::iterate_indices<2>(policy, { 0, 0 }, { x_size, y_size }, [this](size_t x, size_t y) {
//    for (size_t x = 0; x < x_size; x++) {
//        for (size_t y = 0; y < y_size; y++) {
            if (get_cell(x, y) == CELL_TYPES::BORDER) {
                get_new_population(x + 1, y, 1) += get_new_population(x, y, 7);
                get_new_population(x - 1, y, 7) += get_new_population(x, y, 1);

                get_new_population(x, y - 1, 3) += get_new_population(x, y, 5);
                get_new_population(x, y + 1, 5) += get_new_population(x, y, 3);

                get_new_population(x - 1, y + 1, 8) += get_new_population(x, y, 0);
                get_new_population(x - 1, y - 1, 6) += get_new_population(x, y, 2);

                get_new_population(x + 1, y + 1, 2) += get_new_population(x, y, 6);
                get_new_population(x + 1, y - 1, 0) += get_new_population(x, y, 8);

//                get_new_population(x + 1, y, 1) += get_new_population(x, y, 7);
//                get_new_population(x - 1, y, 7) += get_new_population(x, y, 1);
//
//                get_new_population(x, y - 1, 3) += get_new_population(x, y, 5);
//                get_new_population(x, y + 1, 5) += get_new_population(x, y, 3);
//
//                // case dir 0
//                if (get_cell(x - 1, y) == CELL_TYPES::BORDER && get_cell(x, y + 1) == CELL_TYPES::BORDER) {
//                    get_new_population(x - 1, y + 1, 8) += get_new_population(x, y, 0);
//                } else if (get_cell(x - 1, y) == CELL_TYPES::BORDER && get_cell(x, y + 1) != CELL_TYPES::BORDER) {
//                    get_new_population(x, y + 1, 2) += get_new_population(x, y, 0);
//                } else if (get_cell(x - 1, y) != CELL_TYPES::BORDER && get_cell(x, y + 1) == CELL_TYPES::BORDER) {
//                    get_new_population(x - 1, y, 6) += get_new_population(x, y, 0);
//                } else if (get_cell(x - 1, y) != CELL_TYPES::BORDER && get_cell(x, y + 1) != CELL_TYPES::BORDER) {
//                    get_new_population(x, y + 1, 2) += get_new_population(x, y, 0) / 2.0;
//                    get_new_population(x - 1, y, 6) += get_new_population(x, y, 0) / 2.0;
//                }
//
//                // case dir 2
//                if (get_cell(x - 1, y) == CELL_TYPES::BORDER && get_cell(x, y - 1) == CELL_TYPES::BORDER) {
//                    get_new_population(x - 1, y - 1, 6) += get_new_population(x, y, 2);
//                } else if (get_cell(x - 1, y) == CELL_TYPES::BORDER && get_cell(x, y - 1) != CELL_TYPES::BORDER) {
//                    get_new_population(x, y - 1, 0) += get_new_population(x, y, 2);
//                } else if (get_cell(x - 1, y) != CELL_TYPES::BORDER && get_cell(x, y - 1) == CELL_TYPES::BORDER) {
//                    get_new_population(x - 1, y, 8) += get_new_population(x, y, 2);
//                } else if (get_cell(x - 1, y) != CELL_TYPES::BORDER && get_cell(x, y - 1) != CELL_TYPES::BORDER) {
//                    get_new_population(x, y - 1, 0) += get_new_population(x, y, 2) / 2.0;
//                    get_new_population(x - 1, y, 8) += get_new_population(x, y, 2) / 2.0;
//                }
//
//                // case dir 6
//                if (get_cell(x + 1, y) == CELL_TYPES::BORDER && get_cell(x, y + 1) == CELL_TYPES::BORDER) {
//                    get_new_population(x + 1, y + 1, 2) += get_new_population(x, y, 6);
//                } else if (get_cell(x + 1, y) == CELL_TYPES::BORDER && get_cell(x, y + 1) != CELL_TYPES::BORDER) {
//                    get_new_population(x, y + 1, 8) += get_new_population(x, y, 6);
//                } else if (get_cell(x + 1, y) != CELL_TYPES::BORDER && get_cell(x, y + 1) == CELL_TYPES::BORDER) {
//                    get_new_population(x + 1, y, 0) += get_new_population(x, y, 6);
//                } else if (get_cell(x + 1, y) != CELL_TYPES::BORDER && get_cell(x, y + 1) != CELL_TYPES::BORDER) {
//                    get_new_population(x, y + 1, 8) += get_new_population(x, y, 6) / 2.0;
//                    get_new_population(x + 1, y, 0) += get_new_population(x, y, 6) / 2.0;
//                }
//
//                // case dir 8
//                if (get_cell(x, y - 1) == CELL_TYPES::BORDER && get_cell(x + 1, y) == CELL_TYPES::BORDER) {
//                    get_new_population(x + 1, y - 1, 0) += get_new_population(x, y, 8);
//                } else if (get_cell(x, y - 1) == CELL_TYPES::BORDER && get_cell(x + 1, y) != CELL_TYPES::BORDER) {
//                    get_new_population(x + 1, y, 2) += get_new_population(x, y, 8);
//                } else if (get_cell(x, y - 1) != CELL_TYPES::BORDER && get_cell(x + 1, y) == CELL_TYPES::BORDER) {
//                    get_new_population(x, y - 1, 6) += get_new_population(x, y, 8);
//                } else if (get_cell(x, y - 1) != CELL_TYPES::BORDER && get_cell(x + 1, y) != CELL_TYPES::BORDER) {
//                    get_new_population(x + 1, y, 2) += get_new_population(x, y, 8) / 2.0;
//                    get_new_population(x, y - 1, 6) += get_new_population(x, y, 8) / 2.0;
//                }

                for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
                    get_new_population(x, y, dir) = 0.0;
                }
            }
        });
}

void grid2d_algorithms::stream() {

//    std::copy((*populations)[4].begin(), (*populations)[4].end(), (*new_populations)[4].begin());
    hpx::parallel::for_loop(hpx::parallel::par, 0, x_size, [this](size_t x){
//    for (size_t x = 0; x < x_size; x++) {
        std::copy_n((*populations)[4].begin() + get_row_index_padded(x), y_size,
                (*new_populations)[4].begin() + get_row_index_unpadded(x) + 1);

        std::copy_n((*populations)[3].begin() + get_row_index_padded(x), y_size,
                (*new_populations)[3].begin() + get_row_index_unpadded(x));
        std::copy_n((*populations)[5].begin() + get_row_index_padded(x), y_size,
                (*new_populations)[5].begin() + get_row_index_unpadded(x) + 2);

        std::copy_n((*populations)[0].begin() + get_row_index_padded(x), y_size,
                (*new_populations)[0].begin() + get_row_index_unpadded(x + 1));
        std::copy_n((*populations)[1].begin() + get_row_index_padded(x), y_size,
                (*new_populations)[1].begin() + +get_row_index_unpadded(x + 1) + 1);
        std::copy_n((*populations)[2].begin() + get_row_index_padded(x), y_size,
                (*new_populations)[2].begin() + get_row_index_unpadded(x + 1) + 2);

        std::copy_n((*populations)[6].begin() + get_row_index_padded(x), y_size,
                (*new_populations)[6].begin() + get_row_index_unpadded(x - 1));
        std::copy_n((*populations)[7].begin() + get_row_index_padded(x), y_size,
                (*new_populations)[7].begin() + get_row_index_unpadded(x - 1) + 1);
        std::copy_n((*populations)[8].begin() + get_row_index_padded(x), y_size,
                (*new_populations)[8].begin() + get_row_index_unpadded(x - 1) + 2);
    });

//    index_iterator::blocking_pseudo_execution_policy<size_t> policy(2);
//    policy.add_blocking( { 1, y_size }, { true, false });
//
//    index_iterator::iterate_indices<2>(policy, { 0, 0 }, { x_size, y_size }, [this](size_t x, size_t y) {
//        if (get_cell(x, y) != CELL_TYPES::BORDER) {
//            get_new_population(x + 1, y - 1, 0) = get_population(x, y, 0);
//            get_new_population(x + 1, y + 0, 1) = get_population(x, y, 1);
//            get_new_population(x + 1, y + 1, 2) = get_population(x, y, 2);
//            get_new_population(x + 0, y - 1, 3) = get_population(x, y, 3);
//            get_new_population(x + 0, y + 0, 4) = get_population(x, y, 4);
//            get_new_population(x + 0, y + 1, 5) = get_population(x, y, 5);
//            get_new_population(x - 1, y - 1, 6) = get_population(x, y, 6);
//            get_new_population(x - 1, y + 0, 7) = get_population(x, y, 7);
//            get_new_population(x - 1, y + 1, 8) = get_population(x, y, 8);
//        }
//    });
}

void grid2d_algorithms::print_grid() {
    std::cout << "------------------STEP START----------------------" << std::endl;
//    std::cout << "x_size: " << x_size << std::endl;
//    std::cout << "y_size: " << y_size << std::endl;
//    for (size_t x = 0; x < x_size; x++) {
//        if (x < 10) {
//            std::cout << " ";
//        }
//        std::cout << x << "| ";
//        for (size_t y = 0; y < y_size; y++) {
//            if (y > 0) {
//                std::cout << " ";
//            }
//            if (get_cell(x, y) == CELL_TYPES::BORDER) {
//                std::cout << "B";
//                double mass_density = get_mass_density(x, y);
//                if (mass_density > 0.0) {
//                    std::cout << "+";
//                } else {
//                    std::cout << " ";
//                }
//            } else if (get_cell(x, y) == CELL_TYPES::DRAIN) {
//                std::cout << "D ";
//            } else if (get_cell(x, y) == CELL_TYPES::SOURCE) {
//                std::cout << "S ";
//            } else if (get_cell(x, y) == CELL_TYPES::WATER) {
//                std::cout << "W";
//                double mass_density = get_mass_density(x, y);
//                if (mass_density > 0.0) {
//                    std::cout << "+";
//                } else {
//                    std::cout << " ";
//                }
//            }
//        }
//        std::cout << std::endl;
//    }
//    std::cout << "----------------------------------------" << std::endl;
    std::cout << std::setprecision(2);
    std::cout << "x_size: " << x_size << std::endl;
    std::cout << "y_size: " << y_size << std::endl;
    for (size_t x = 0; x < x_size; x++) {
        if (x < 10) {
            std::cout << " ";
        }
        std::cout << x << "| ";
        for (size_t y = 0; y < y_size; y++) {
            if (y > 0) {
                std::cout << " ";
            }
            double mass_density = get_mass_density(x, y);
            if (mass_density > 0.0) {
                std::cout << mass_density;
            } else {
                std::cout << "   ";
            }
        }
        std::cout << std::endl;
    }
}

lattice::CELL_TYPES &grid2d_algorithms::get_cell(int64_t x, int64_t y) {
    return cells.at(get_cell_index(x, y));
}

double &grid2d_algorithms::get_population(int64_t x, int64_t y, size_t dir) {
    return (*populations)[dir].at(get_pop_index(x, y));
}

double &grid2d_algorithms::get_new_population(int64_t x, int64_t y, size_t dir) {
    return (*new_populations)[dir].at(get_pop_index(x, y));
}

}
