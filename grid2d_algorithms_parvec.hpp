#pragma once

#include <string>
#include <vector>
#include <memory>

#include <hpx/hpx.hpp>
#include <Vc/Vc>

#include "types.hpp"

#include "util.hpp"

#define DIRECTIONS_2D 9

namespace lattice {

class grid2d_algorithms_parvec {
public:

    static constexpr double OMEGA = 0.9;
    static constexpr double C = 1.0;
    static const double SPEEDS_X[DIRECTIONS_2D];
    static const double SPEEDS_Y[DIRECTIONS_2D];
    static constexpr double MAX_DIR_MASS = 1.5;
    static constexpr double MAX_TOTAL_MASS = 5.0;
    static const double weights[DIRECTIONS_2D];

    static grid2d_algorithms_parvec from_file(std::string file_name, bool verbose);

    void step();

    void serialize_as_csv(const std::string &file_name);

    void print_grid();

private:
    bool verbose;

    size_t x_size;
    size_t y_size;
    std::vector<lattice::CELL_TYPES> cells;
//    std::vector<double> populations[DIRECTIONS_2D];
//    std::vector<double> new_populations[DIRECTIONS_2D];
    std::unique_ptr<std::array<std::vector<double>, DIRECTIONS_2D>> populations; //[DIRECTIONS_2D];
    std::unique_ptr<std::array<std::vector<double>, DIRECTIONS_2D>> new_populations; //,.[DIRECTIONS_2D];

    grid2d_algorithms_parvec(size_t x_size, size_t y_size, std::vector<lattice::CELL_TYPES> &cells_unpadded,
            bool verbose);

    lattice::CELL_TYPES &get_cell(int64_t x, int64_t y);

    double &get_population(int64_t x, int64_t y, size_t dir);

    double &get_new_population(int64_t x, int64_t y, size_t dir);

//    void velocity_to_momentum(const double (&v)[9], const double mass_density, double (&momentum)[9]);

    size_t get_pop_index(int64_t x, int64_t y);

    size_t get_cell_index(int64_t x, int64_t y);

    size_t get_row_index_unpadded(int64_t x);

    size_t get_row_index_padded(int64_t x);

    template<typename T>
    void get_momentum_density(T (&cell_pop)[9], T (&momentum_density)[2]) {

        momentum_density[0] = 0.0;
        momentum_density[1] = 0.0;

        for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
            momentum_density[0] += cell_pop[dir] * grid2d_algorithms_parvec::SPEEDS_X[dir];
        }

        for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
            momentum_density[1] += cell_pop[dir] * grid2d_algorithms_parvec::SPEEDS_Y[dir];
        }
    }

    template<typename T>
    double get_mass_density(T (&cell_pop)[9]) {
        T mass_density = 0.0;
        for (size_t dir = 0; dir < DIRECTIONS_2D; dir++) {
            mass_density += cell_pop[dir];
        }
        return mass_density;
    }

//    template<typename T>
//    double calculate_equilibrium(const size_t dir, const T mass_density, const T (&u)[2]) {
//        T u2 = u[0] * u[0] + u[1] * u[1];
//        T vu = SPEEDS_X[dir] * u[0] + SPEEDS_Y[dir] * u[1];
//        return mass_density * weights[dir] * (1.0 + 3.0 * vu + 4.5 * vu * vu - 1.5 * u2);
//    }

    void initialize_cell(size_t x, size_t y, double factor);

    void collide();

    void get_momentum_density(size_t x, size_t y, double (&momentum_density)[2]);

    double get_mass_density(size_t x, size_t y);

    template<typename Tuple>
    auto get_mass_density(Tuple &t) {

        // remove reference and get type of first element
        using comp_type = typename hpx::util::tuple_element<0, typename hpx::util::decay<decltype(t)>::type>::type;
        using var_type = typename hpx::util::decay<comp_type>::type;

        var_type mass_density = 0.0;
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

    template<typename Tuple>
    void get_momentum_density(Tuple &t,
            typename hpx::util::decay<typename hpx::util::tuple_element<0, Tuple>::type>::type (&momentum_density)[2]) {

        momentum_density[0] = 0.0;
        momentum_density[1] = 0.0;

        momentum_density[0] += hpx::util::get<0>(t) * grid2d_algorithms_parvec::SPEEDS_X[0];
        momentum_density[0] += hpx::util::get<1>(t) * grid2d_algorithms_parvec::SPEEDS_X[1];
        momentum_density[0] += hpx::util::get<2>(t) * grid2d_algorithms_parvec::SPEEDS_X[2];
        momentum_density[0] += hpx::util::get<3>(t) * grid2d_algorithms_parvec::SPEEDS_X[3];
        momentum_density[0] += hpx::util::get<4>(t) * grid2d_algorithms_parvec::SPEEDS_X[4];
        momentum_density[0] += hpx::util::get<5>(t) * grid2d_algorithms_parvec::SPEEDS_X[5];
        momentum_density[0] += hpx::util::get<6>(t) * grid2d_algorithms_parvec::SPEEDS_X[6];
        momentum_density[0] += hpx::util::get<7>(t) * grid2d_algorithms_parvec::SPEEDS_X[7];
        momentum_density[0] += hpx::util::get<8>(t) * grid2d_algorithms_parvec::SPEEDS_X[8];

        momentum_density[1] += hpx::util::get<0>(t) * grid2d_algorithms_parvec::SPEEDS_Y[0];
        momentum_density[1] += hpx::util::get<1>(t) * grid2d_algorithms_parvec::SPEEDS_Y[1];
        momentum_density[1] += hpx::util::get<2>(t) * grid2d_algorithms_parvec::SPEEDS_Y[2];
        momentum_density[1] += hpx::util::get<3>(t) * grid2d_algorithms_parvec::SPEEDS_Y[3];
        momentum_density[1] += hpx::util::get<4>(t) * grid2d_algorithms_parvec::SPEEDS_Y[4];
        momentum_density[1] += hpx::util::get<5>(t) * grid2d_algorithms_parvec::SPEEDS_Y[5];
        momentum_density[1] += hpx::util::get<6>(t) * grid2d_algorithms_parvec::SPEEDS_Y[6];
        momentum_density[1] += hpx::util::get<7>(t) * grid2d_algorithms_parvec::SPEEDS_Y[7];
        momentum_density[1] += hpx::util::get<8>(t) * grid2d_algorithms_parvec::SPEEDS_Y[8];
    }

    template<typename var_type>
    var_type calculate_equilibrium(const size_t dir,
        const var_type mass_density, const var_type (&u)[2]) {
      var_type u2 = u[0] * u[0] + u[1] * u[1];
      var_type vu = SPEEDS_X[dir] * u[0] + SPEEDS_Y[dir] * u[1];
      return mass_density * weights[dir]
          * (1.0 + 3.0 * vu + 4.5 * vu * vu - 1.5 * u2);
    }

    void source();

    void drain();

    void boundary();

    void stream();
}
;

//struct collide_cell {
//  template<typename T>
//  void operator()(
//      hpx::util::tuple<T &, T &, T &, T &, T &, T &, T &, T &, T &> t) {
//    T cell_pop[9];
//    lattice::util::unpack_tuple<0, 9>(t, cell_pop);
//
//    T mass_density = get_mass_density(cell_pop);
//    T momentum_density[2];
//    get_momentum_density(cell_pop, momentum_density);
//    T u[2] = { momentum_density[0] / mass_density, momentum_density[1]
//        / mass_density };
//
//    //TODO: cannot do this without a "tuple_view"
//    hpx::util::get<0>(t) = std::max(0.0,
//        (1 - grid2d_algorithms_parvec::OMEGA) * hpx::util::get<0>(t)
//            + grid2d_algorithms_parvec::OMEGA * calculate_equilibrium(0, mass_density, u));
//    hpx::util::get<1>(t) = std::max(0.0,
//        (1 - grid2d_algorithms_parvec::OMEGA) * hpx::util::get<1>(t)
//            + grid2d_algorithms_parvec::OMEGA * calculate_equilibrium(1, mass_density, u));
//    hpx::util::get<2>(t) = std::max(0.0,
//        (1 - grid2d_algorithms_parvec::OMEGA) * hpx::util::get<2>(t)
//            + grid2d_algorithms_parvec::OMEGA * calculate_equilibrium(2, mass_density, u));
//    hpx::util::get<3>(t) = std::max(0.0,
//        (1 - grid2d_algorithms_parvec::OMEGA) * hpx::util::get<3>(t)
//            + grid2d_algorithms_parvec::OMEGA * calculate_equilibrium(3, mass_density, u));
//    hpx::util::get<4>(t) = std::max(0.0,
//        (1 - grid2d_algorithms_parvec::OMEGA) * hpx::util::get<4>(t)
//            + grid2d_algorithms_parvec::OMEGA * calculate_equilibrium(4, mass_density, u));
//    hpx::util::get<5>(t) = std::max(0.0,
//        (1 - grid2d_algorithms_parvec::OMEGA) * hpx::util::get<5>(t)
//            + grid2d_algorithms_parvec::OMEGA * calculate_equilibrium(5, mass_density, u));
//    hpx::util::get<6>(t) = std::max(0.0,
//        (1 - grid2d_algorithms_parvec::OMEGA) * hpx::util::get<6>(t)
//            + grid2d_algorithms_parvec::OMEGA * calculate_equilibrium(6, mass_density, u));
//    hpx::util::get<7>(t) = std::max(0.0,
//        (1 - grid2d_algorithms_parvec::OMEGA) * hpx::util::get<7>(t)
//            + grid2d_algorithms_parvec::OMEGA * calculate_equilibrium(7, mass_density, u));
//    hpx::util::get<8>(t) = std::max(0.0,
//        (1 - grid2d_algorithms_parvec::OMEGA) * hpx::util::get<8>(t)
//            + grid2d_algorithms_parvec::OMEGA * calculate_equilibrium(8, mass_density, u));
//  }
//};

}
