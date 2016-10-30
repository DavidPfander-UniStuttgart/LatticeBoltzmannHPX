#include "grid2d.hpp"

#include <boost/tokenizer.hpp>
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include "types.hpp"

namespace lattice {

//
// NW=0 N=1 NE=2
// W=3 C=4 E=5
// SW=6 S=7 SE=8

const double grid2d::SPEEDS_X[DIRECTIONS_2D] = { -C, 0, C, -C, 0, C, -C, 0, C };
const double grid2d::SPEEDS_Y[DIRECTIONS_2D] = { -C, -C, -C, 0, 0, 0, C, C, C };

grid2d grid2d::from_file(std::string file_name) {
    std::stringstream ss;
    ss << std::ifstream(file_name).rdbuf();

//    std::string s = "Boost C++ Libraries";

    boost::char_separator<char> sep(" \n");
    typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
    tokenizer tok(ss.str(), sep);
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
        std::cout << "token: \"" << t << "\"" << std::endl;

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

    std::cout << "x_size: " << x_size << std::endl;
    std::cout << "y_size: " << y_size << std::endl;
    for (size_t x = 0; x < x_size; x++) {
        for (size_t y = 0; y < y_size; y++) {
            if (y > 0) {
                std::cout << " ";
            }
            if (cells[x * y_size + y] == CELL_TYPES::BORDER) {
                std::cout << "B";
            } else if (cells[x * y_size + y] == CELL_TYPES::DRAIN) {
                std::cout << "D";
            } else if (cells[x * y_size + y] == CELL_TYPES::SOURCE) {
                std::cout << "S";
            } else if (cells[x * y_size + y] == CELL_TYPES::WATER) {
                std::cout << "W";
            }
        }
        std::cout << std::endl;
    }

    return grid2d(x_size, y_size, cells);
}

grid2d::grid2d(size_t x_size, size_t y_size,
        std::vector<lattice::CELL_TYPES> &cells) :
        x_size(x_size), y_size(y_size), cells(cells) {
    for (std::vector<double> &dir : populations) {
        dir.resize((x_size + 1) * (y_size + 1));
    }

    collide();
//    double massDensity = 1.0;
//    double momentumDensity[2] = { 0.0, 0.0 };
//    population = self.calculateEquilibiumDistribution(massDensity,
//            momentumDensity)

}

//void grid2d::velocity_to_momentum(const double (&v)[9],
//        const double mass_density, double (&momentum)[9]) {
//    for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
//        momentum[dir] = v[dir] / mass_density;
//    }
//}

void grid2d::step() {
    collide();
    source();
    drain();
    stream();
}

void grid2d::get_momentum_density(size_t flat_cell_index,
        double (&momentum_density)[2]) {
    momentum_density[0] = 0.0;
    momentum_density[1] = 0.0;
    for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
        momentum_density[0] += populations[dir][flat_cell_index]
                * grid2d::SPEEDS_X[dir];
    }
    for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
        momentum_density[1] += populations[dir][flat_cell_index]
                * grid2d::SPEEDS_Y[dir];
    }
}

double grid2d::get_mass_density(size_t flat_cell_index) {
    double mass_density = 0.0;
    for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
        mass_density += populations[dir][flat_cell_index];
    }
    return mass_density;
}

void grid2d::calculate_equilibrium(const double mass_density,
        const double (&momentum_density)[2], double (&equilibrium)[9]) {
    double u[2] = { momentum_density[0] / mass_density, momentum_density[1]
            / mass_density };
    double uu = u[0] * u[0] + u[1] * u[1];
    double c2 = C * C;
    double c4 = c2 * c2;
    double forth = -1.0 * (3.0 * uu) / (2.0 * c2);

    for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
        double cu = SPEEDS_X[dir] * u[0] + SPEEDS_Y[dir] * u[1];
        double second = (3.0 * cu) / (c2);
        double third = (9.0 * cu * cu) / (2.0 * c4);
        if (dir == 4) { // == center cell
            equilibrium[dir] = (4.0 / 9.0) * mass_density * (1.0 + forth);
        } else if (dir == 1 || dir == 3 or dir == 5 || dir == 7) { // == NSWE
            equilibrium[dir] = (1.0 / 9.0) * mass_density
                    * (1.0 + second + third + forth);
        } else { // other directions
            equilibrium[dir] = (1.0 / 36.0) * mass_density
                    * (1.0 + second + third + forth);
        }
    }
}

void grid2d::collide() {
    for (size_t flat_cell_index = 0; flat_cell_index < x_size * y_size;
            flat_cell_index++) {
        double mass_density = get_mass_density(flat_cell_index);
        double momentum_density[2];
        get_momentum_density(flat_cell_index, momentum_density);
        double equilibrium[9];
        calculate_equilibrium(mass_density, momentum_density, equilibrium);

        for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
            populations[dir][flat_cell_index] = (1 - OMEGA)
                    * populations[dir][flat_cell_index]
                    + OMEGA * equilibrium[dir];
        }
        //TODO: add correct() step?
    }
}

void grid2d::source() {
    for (size_t x = 1; x < x_size * y_size + 1; x++) {
        for (size_t y = 1; y < x_size * y_size + 1; y++) {
            if (cells[x * (y_size + 1) + y] == CELL_TYPES::SOURCE) {
//TODO: port that piece of code?
//                double momentum[9];
//                velocity_to_momentum(v, 1.0, momentum);
//                double equilibrium[9];
//                calculate_equilibrium(1.0, momentum, equilibrium);
                for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
                    new_populations[dir][x * (y_size + 1) + y] = 1.0
                            / DIRECTIONS_2D;
                }
            }
        }
    }
}

void grid2d::drain() {
    for (size_t x = 1; x < x_size * y_size + 1; x++) {
        for (size_t y = 1; y < x_size * y_size + 1; y++) {
            if (cells[x * (y_size + 1) + y] == CELL_TYPES::SOURCE) {
                for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
                    new_populations[dir][x * (y_size + 1) + y] = 0.0;
                }
            }
        }
    }
}

void grid2d::boundary() {
    for (size_t x = 1; x < x_size * y_size + 1; x++) {
        for (size_t y = 1; y < x_size * y_size + 1; y++) {
            if (cells[x * (y_size + 1) + y] == CELL_TYPES::BORDER) {
                new_populations[0][x * (y_size + 1) + y] = populations[7][x * (y_size + 1) + y]; // NW
                new_populations[1][x * (y_size + 1) + y] = populations[8][x * (y_size + 1) + y]; // N
                new_populations[2][x * (y_size + 1) + y] = populations[9][x * (y_size + 1) + y]; // NE
                new_populations[3][x * (y_size + 1) + y] = populations[5][x * (y_size + 1) + y]; // W
                new_populations[4][x * (y_size + 1) + y] = populations[4][x * (y_size + 1) + y]; // C
                new_populations[5][x * (y_size + 1) + y] = populations[3][x * (y_size + 1) + y]; // E
                new_populations[6][x * (y_size + 1) + y] = populations[0][x * (y_size + 1) + y]; // SW
                new_populations[7][x * (y_size + 1) + y] = populations[1][x * (y_size + 1) + y]; // S
                new_populations[8][x * (y_size + 1) + y] = populations[2][x * (y_size + 1) + y]; // SE
            }
        }
    }
}

void grid2d::stream() {
    for (size_t x = 1; x < x_size * y_size + 1; x++) {
        for (size_t y = 1; y < x_size * y_size + 1; y++) {
            new_populations[0][(x + 1) * (y_size + 1) + (y - 1)] =
                    populations[0][x * (y_size + 1) + y]; // NW
            new_populations[1][(x + 1) * (y_size + 1) + y] = populations[1][x
                    * (y_size + 1) + y]; // N
            new_populations[2][(x + 1) * (y_size + 1) + (y + 1)] =
                    populations[2][x * (y_size + 1) + y]; // NE
            new_populations[3][x * (y_size + 1) + (y - 1)] = populations[3][x
                    * (y_size + 1) + y]; // W
            new_populations[4][x * (y_size + 1) + y] = populations[4][x
                    * (y_size + 1) + y]; // C
            new_populations[5][x * (y_size + 1) + (y + 1)] = populations[5][x
                    * (y_size + 1) + y]; // E
            new_populations[6][(x - 1) * (y_size + 1) + (y - 1)] =
                    populations[6][x * (y_size + 1) + y]; // NW
            new_populations[7][(x - 1) * (y_size + 1) + y] = populations[7][x
                    * (y_size + 1) + y]; // S
            new_populations[8][(x - 1) * (y_size + 1) + (y + 1)] =
                    populations[8][x * (y_size + 1) + y]; // NE
        }
    }
}

}