#pragma once

#include <string>
#include <vector>

#include "types.hpp"

#define DIRECTIONS_2D 9

namespace lattice {

class grid2d {
public:

    static constexpr double OMEGA = 1.0;
    static constexpr double C = 1.0;
    static const double SPEEDS_X[DIRECTIONS_2D];
    static const double SPEEDS_Y[DIRECTIONS_2D];
    static constexpr double MAX_DIR_MASS = 1.5;
    static constexpr double MAX_TOTAL_MASS = 5.0;

    static grid2d from_file(std::string file_name);

    void step();

private:

    size_t x_size;
    size_t y_size;
    std::vector<lattice::CELL_TYPES> cells;
    std::vector<double> populations[DIRECTIONS_2D];
    std::vector<double> new_populations[DIRECTIONS_2D];

    grid2d(size_t x_size, size_t y_size,
            std::vector<lattice::CELL_TYPES> &cells);

//    void velocity_to_momentum(const double (&v)[9], const double mass_density, double (&momentum)[9]);

    void get_momentum_density(size_t flat_cell_index,
            double (&momentum_density)[2]);

    double get_mass_density(size_t flat_cell_index);

    void calculate_equilibrium(const double mass_density, const double (&momentum_density)[2], double (&equilibrium)[9]);

    void collide();

    void source();

    void drain();

    void boundary();

    void stream();
};

}