#pragma once

namespace lattice {

class grid {
public:
    static grid from_file(std::string file_name);
private:
    grid();
};

}
