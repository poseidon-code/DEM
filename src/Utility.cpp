#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include "DEM.hpp"
#include "Utility.hpp"



void create_dem_asc_bin(const std::string& path) {
    std::vector<short int> dem_data;
    short int value = 0;

    // skip 1st 5 lines of metadata
    std::ifstream ifp(path);
    for (int i=0; i<5; i++) {
        std::string l;
        std::getline(ifp, l);
    }

    // read values to `dem_data`
    while (ifp >> value) dem_data.push_back(value);
    ifp.close();

    // set new file name for '.bin'
    std::string ofp_path;
    size_t ext_pos = path.find_last_of('.');
    if (ext_pos != std::string::npos) ofp_path = path.substr(0, ext_pos + 1) + std::string("bin");

    // write `dem_data` in binary format
    std::ofstream ofp(ofp_path, std::ios::binary | std::ios::trunc);
    ofp.write(reinterpret_cast<char*>(dem_data.data()), dem_data.size() * sizeof(short int));
    ofp.close();
}


void create_dem_asc_csv(const std::string& path, DEM::Type type) {
    std::vector<short int> dem_data;
    short int value = 0;

    // skip 1st 5 lines of metadata
    std::ifstream ifp(path);
    for (int i=0; i<5; i++) {
        std::string l;
        std::getline(ifp, l);
    }

    // read values to `dem_data`
    while (ifp >> value) dem_data.push_back(value);
    ifp.close();

    // set new file name for '.bin'
    std::string ofp_path;
    size_t ext_pos = path.find_last_of('.');
    if (ext_pos != std::string::npos) ofp_path = path.substr(0, ext_pos + 1) + std::string("csv");

    // write as per '.csv' format
    std::ofstream ofp(ofp_path);
    for (int i = 0; i < type.nrows * type.ncols; ++i) {
        if (i % type.ncols == type.ncols - 1)
            ofp << dem_data[i] << "\n";
        else
            ofp << dem_data[i] << ",";
    }
    ofp.close();
}


void create_dem_csv_bin(const std::string& path) {
    std::vector<short int> dem_data;
    short int value = 0;

    // read values to `dem_data`
    std::ifstream ifp(path);
    std::string line;
    while (std::getline(ifp, line)) {
        std::istringstream iss(line);
        std::string token;
        while (std::getline(iss, token, ','))
            dem_data.push_back(static_cast<short int>(std::stoi(token)));
    }
    ifp.close();

    // set new file name for '.bin'
    std::string ofp_path;
    size_t ext_pos = path.find_last_of('.');
    if (ext_pos != std::string::npos) ofp_path = path.substr(0, ext_pos + 1) + std::string("bin");

    // write `dem_data` in binary format
    std::ofstream ofp(ofp_path, std::ios::binary | std::ios::trunc);
    ofp.write(reinterpret_cast<char*>(dem_data.data()), dem_data.size() * sizeof(short int));
    ofp.close();
}


void create_dem_bin_csv(const std::string& path, DEM::Type type) {
    std::vector<short int> dem_data;
    short int value = 0;

    // read from .bin file
    std::ifstream ifp(path);
    while (ifp.read(reinterpret_cast<char*>(&value), sizeof(short int))) dem_data.push_back(value);
    ifp.close();

    // set new file name for '.csv'
    std::string ofp_path;
    size_t ext_pos = path.find_last_of('.');
    if (ext_pos != std::string::npos) ofp_path = path.substr(0, ext_pos + 1) + std::string("csv");

    // write as per '.csv' format
    std::ofstream ofp(ofp_path);
    for (int i = 0; i < type.nrows * type.ncols; ++i) {
        if (i % type.ncols == type.ncols - 1)
            ofp << dem_data[i] << "\n";
        else
            ofp << dem_data[i] << ",";
    }
    ofp.close();
}
