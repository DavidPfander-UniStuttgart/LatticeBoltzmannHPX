/*
 * lattice.cpp
 *
 *  Created on: Oct 29, 2016
 *      Author: pfandedd
 */

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>

#include "grid2d.hpp"
#include "grid2d_algorithms.hpp"

//#include <hpx/include/actions.hpp>
//#include <hpx/include/async.hpp>
//#include <hpx/include/util.hpp>
//#include <hpx/include/components.hpp>

boost::program_options::options_description desc_commandline("Usage: " HPX_APPLICATION_STRING " [options]");

bool is_root_node;
bool display_help = false;

int hpx_main(boost::program_options::variables_map& vm) {

    // extract command line argument
    bool verbose = vm["verbose"].as<bool>();
    std::string grid_file_name = vm["grid-file"].as<std::string>();
    size_t steps = vm["steps"].as<size_t>();
    bool write_steps = vm["write-steps"].as<bool>();
    bool write_result = vm["write-result"].as<bool>();
    std::string algorithm = vm["algorithm"].as<std::string>();

    is_root_node = hpx::find_here() == hpx::find_root_locality();

    if (vm.count("help")) {
        display_help = true;
        if (is_root_node) {
            hpx::cout << desc_commandline << std::endl << hpx::flush;
        }
        return hpx::finalize();
    }

    if (algorithm.compare("simple") == 0) {
        lattice::grid2d grid = lattice::grid2d::from_file(grid_file_name, verbose);

        for (size_t step = 0; step < steps; step++) {
            grid.step();
            if (write_steps) {
                grid.serialize_as_csv("serialized.csv." + std::to_string(step));
            }
        }

        if (write_result) {
            grid.serialize_as_csv("result.csv");
        }
    } else if (algorithm.compare("algorithms") == 0) {
        lattice::grid2d_algorithms grid = lattice::grid2d_algorithms::from_file(grid_file_name, verbose);

        for (size_t step = 0; step < steps; step++) {
            grid.step();
            if (write_steps) {
                grid.serialize_as_csv("serialized.csv." + std::to_string(step));
            }
        }

        if (write_result) {
            grid.serialize_as_csv("result.csv");
        }
    }

    return hpx::finalize(); // Handles HPX shutdown
}

int main(int argc, char* argv[]) {

    desc_commandline.add_options()("help", "display help")("grid-file",
            boost::program_options::value<std::string>()->default_value(""),
            "input file in csv format, describes grid with sources and drains and border cells")("steps",
            boost::program_options::value<size_t>()->default_value(1), "steps for the simulation to run")("write-steps",
            boost::program_options::value<bool>()->default_value(false),
            "write a csv file after every lattice boltzmann step")("write-result",
            boost::program_options::value<bool>()->default_value(true),
            "write a velocity field of the final state of the domain")("verbose",
            boost::program_options::value<bool>()->default_value(true), "more stuff printed on the console")(
            "algorithm", boost::program_options::value<std::string>()->default_value(""), "choices are: simple, tiled");

// Initialize and run HPX
    int return_value = hpx::init(desc_commandline, argc, argv);

    if (display_help) {
        return return_value;
    }

    if (is_root_node) {
        std::cout << "simulation finished" << std::endl;
    }
    return return_value;
}

