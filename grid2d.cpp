#include "grid2d.hpp"

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cmath>

#include "index_iterator.hpp"
#include "types.hpp"
#include <boost/tokenizer.hpp>

namespace lattice {

//
// NW=0 N=1 NE=2
// W=3 C=4 E=5
// SW=6 S=7 SE=8

const double grid2d::SPEEDS_X[DIRECTIONS_2D] = { -C, 0, C, -C, 0, C, -C, 0, C };
const double grid2d::SPEEDS_Y[DIRECTIONS_2D] = { -C, -C, -C, 0, 0, 0, C, C, C };
const double grid2d::weights[DIRECTIONS_2D] = { 1.0 / 36.0, 1.0 / 9.0, 1.0 / 36.0, 1.0 / 9.0, 4.0 / 9.0, 1.0 / 9.0, 1.0
        / 36.0, 1.0 / 9.0, 1.0 / 36.0 };

grid2d grid2d::from_file(std::string file_name, bool verbose) {
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

    return grid2d(x_size, y_size, cells, verbose);
}

void grid2d::serialize_as_csv(const std::string &file_name) {
    std::ofstream myfile;
    myfile.open(file_name);
    const std::string sep(",");
    myfile << "x" << sep << "y" << sep << "z (dummy)" << sep << "value" << std::endl;
    for (size_t x = 0; x < x_size; x++) {
        for (size_t y = 0; y < y_size; y++) {
//            myfile << x << sep << y << sep << 0.0 << sep << get_mass_density(x, y) << std::endl;
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

////    std::cout << " is x < x_size + 1? " << (-1 < x_size + 1) << std::endl;
//    for (int64_t x = -1; x < static_cast<int64_t>(x_size + 1); x++) {
//        for (int64_t y = -1; y < static_cast<int64_t>(y_size + 1); y++) {
//            if (x >= 0 && x < static_cast<int64_t>(x_size) && y >= 0 && y < static_cast<int64_t>(y_size)) {
////                if (get_cell(x, y) != CELL_TYPES::BORDER) {
//                myfile << x << sep << y << sep << 0.0 << sep << get_mass_density(x, y) << std::endl;
////                } else {
////                    myfile << x << sep << y << sep << 0.0 << sep << 2.0 << std::endl;
////                }
//            } else {
//                double mass_density = 0.0;
//                for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
//                    mass_density += get_population(x, y, dir);
//                }
//                myfile << x << sep << y << sep << 0.0 << sep << mass_density << std::endl;
//            }
//        }
//    }
    myfile.close();
}

size_t grid2d::get_pop_index(int64_t x, int64_t y) {
    return (x + 1) * (y_size + 2) + (y + 1);
}

size_t grid2d::get_cell_index(int64_t x, int64_t y) {
    return (x + 1) * (y_size + 2) + (y + 1);
}

grid2d::grid2d(size_t x_size, size_t y_size, std::vector<lattice::CELL_TYPES> &cells_unpadded, bool verbose) :
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

void grid2d::step() {

    // switch arrays
    std::swap(populations, new_populations);

    for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
        std::fill((*new_populations)[dir].begin(), (*new_populations)[dir].end(), 0.0);
    }

//    print_grid();

    collide();
    stream();
    boundary();
    source();
    drain();

    if (verbose) {
        hpx::cout << "step complete" << std::endl;
    }
}

void grid2d::get_momentum_density(size_t x, size_t y, double (&momentum_density)[2]) {
    momentum_density[0] = 0.0;
    momentum_density[1] = 0.0;
    for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
        momentum_density[0] += get_population(x, y, dir) * grid2d::SPEEDS_X[dir];
    }
    for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
        momentum_density[1] += get_population(x, y, dir) * grid2d::SPEEDS_Y[dir];
    }
}

double grid2d::get_mass_density(size_t x, size_t y) {
    double mass_density = 0.0;
    for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
        mass_density += get_population(x, y, dir);
    }
    return mass_density;
}

double grid2d::calculate_equilibrium(const size_t dir, const double mass_density, const double (&u)[2]) {
    double u2 = u[0] * u[0] + u[1] * u[1];
    double vu = SPEEDS_X[dir] * u[0] + SPEEDS_Y[dir] * u[1];
    return mass_density * weights[dir] * (1.0 + 3.0 * vu + 4.5 * vu * vu - 1.5 * u2);
}

void grid2d::collide() {

    index_iterator::blocking_pseudo_execution_policy<size_t> policy(2);
    policy.add_blocking( { 1, y_size }, { true, false });

    index_iterator::iterate_indices<2>(policy, { 0, 0 }, { x_size, y_size }, [this](size_t x, size_t y) {
        if (get_cell(x, y) == CELL_TYPES::WATER) {
            double mass_density = get_mass_density(x, y);
            double momentum_density[2];
            get_momentum_density(x, y, momentum_density);
            double u[2] = {momentum_density[0] / mass_density, momentum_density[1] / mass_density};

            for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
                double probability_dir = (1 - OMEGA) * get_population(x, y, dir)
                + OMEGA * calculate_equilibrium(dir, mass_density, u);
                if (probability_dir >= 0.0) {
                    get_population(x, y, dir) = probability_dir;
                } else {
                    get_population(x, y, dir) = 0.0;
                }

            }
        }
    });
}

void grid2d::initialize_cell(size_t x, size_t y, double factor) {
    for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
        get_new_population(x, y, dir) = factor * (1.0 / DIRECTIONS_2D);
    }
}

void grid2d::source() {
    index_iterator::blocking_pseudo_execution_policy<size_t> policy(2);
    policy.add_blocking( { 1, y_size }, { true, false });

    index_iterator::iterate_indices<2>(policy, { 0, 0 }, { x_size, y_size }, [this](size_t x, size_t y) {
        if (get_cell(x, y) == CELL_TYPES::SOURCE) {
            initialize_cell(x, y, 0.5);
        }
    });
}

void grid2d::drain() {
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
void grid2d::boundary() {
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
//        }
        });
}

void grid2d::stream() {

    index_iterator::blocking_pseudo_execution_policy<size_t> policy(2);
    policy.add_blocking( { 1, y_size }, { true, false });

    index_iterator::iterate_indices<2>(policy, { 0, 0 }, { x_size, y_size }, [this](size_t x, size_t y) {
        if (get_cell(x, y) != CELL_TYPES::BORDER) {
            get_new_population(x + 1, y - 1, 0) = get_population(x, y, 0);
            get_new_population(x + 1, y + 0, 1) = get_population(x, y, 1);
            get_new_population(x + 1, y + 1, 2) = get_population(x, y, 2);
            get_new_population(x + 0, y - 1, 3) = get_population(x, y, 3);
            get_new_population(x + 0, y + 0, 4) = get_population(x, y, 4);
            get_new_population(x + 0, y + 1, 5) = get_population(x, y, 5);
            get_new_population(x - 1, y - 1, 6) = get_population(x, y, 6);
            get_new_population(x - 1, y + 0, 7) = get_population(x, y, 7);
            get_new_population(x - 1, y + 1, 8) = get_population(x, y, 8);
        }
    });
//    for (size_t x = 0; x < x_size; x++) {
//        for (size_t y = 0; y < y_size; y++) {
//            if (get_cell(x, y) != CELL_TYPES::BORDER) {
//                get_new_population(x + 1, y - 1, 0) = get_population(x, y, 0);
//                get_new_population(x + 1, y + 0, 1) = get_population(x, y, 1);
//                get_new_population(x + 1, y + 1, 2) = get_population(x, y, 2);
//                get_new_population(x + 0, y - 1, 3) = get_population(x, y, 3);
//                get_new_population(x + 0, y + 0, 4) = get_population(x, y, 4);
//                get_new_population(x + 0, y + 1, 5) = get_population(x, y, 5);
//                get_new_population(x - 1, y - 1, 6) = get_population(x, y, 6);
//                get_new_population(x - 1, y + 0, 7) = get_population(x, y, 7);
//                get_new_population(x - 1, y + 1, 8) = get_population(x, y, 8);
//            }
//        }
//    }
}

void grid2d::print_grid() {
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

lattice::CELL_TYPES &grid2d::get_cell(int64_t x, int64_t y) {
    return cells.at(get_cell_index(x, y));
}

double &grid2d::get_population(int64_t x, int64_t y, size_t dir) {
    return (*populations)[dir].at(get_pop_index(x, y));
}

double &grid2d::get_new_population(int64_t x, int64_t y, size_t dir) {
    return (*new_populations)[dir].at(get_pop_index(x, y));
}

}
