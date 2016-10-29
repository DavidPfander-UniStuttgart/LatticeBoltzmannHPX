/*
 * lattice.cpp
 *
 *  Created on: Oct 29, 2016
 *      Author: pfandedd
 */

#include <hpx/hpx_init.hpp>
#include <hpx/include/iostreams.hpp>

#include "grid.hpp"

//#include <hpx/include/actions.hpp>
//#include <hpx/include/async.hpp>
//#include <hpx/include/util.hpp>
//#include <hpx/include/components.hpp>

boost::program_options::options_description desc_commandline(
        "Usage: " HPX_APPLICATION_STRING " [options]");

bool is_root_node;
bool display_help = false;

int hpx_main(boost::program_options::variables_map& vm) {

    // extract command line argument
//    N = vm["n-value"].as<std::uint64_t>();
    std::string grid_file_name = vm["grid-file"].as<std::string>();

    is_root_node = hpx::find_here() == hpx::find_root_locality();

    if (vm.count("help")) {
        display_help = true;
        if (is_root_node) {
            hpx::cout << desc_commandline << std::endl << hpx::flush;
        }
        return hpx::finalize();
    }

    lattice::grid my_grid = lattice::grid::from_file(grid_file_name);

    return hpx::finalize(); // Handles HPX shutdown
}

int main(int argc, char* argv[]) {

    desc_commandline.add_options()("help", "display help")(
            "grid-file",
            boost::program_options::value<std::string>()->default_value(""),
            "input file in csv format, describes grid with sources and drains and border cells");

// Initialize and run HPX
    int return_value = hpx::init(desc_commandline, argc, argv);

    if (display_help) {
        return return_value;
    }

    if (is_root_node) {

    }
    return return_value;
}

