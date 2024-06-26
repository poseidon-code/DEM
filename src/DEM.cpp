#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

#include "DEM.hpp"


int DEM::read(const std::string& filepath) {
    std::ifstream fp = std::ifstream(filepath, std::ios::binary | std::ios::in);
    short int t_value;

    if (fp.good() && !fp.eof()) {
        int column_count = 0;
        std::vector<short int> row_data(this->type.nrows, 0);

        while (fp.read(reinterpret_cast<char*>(&t_value), sizeof(short int))) {
            if (column_count == this->type.ncols) {
                this->data.push_back(row_data);
                column_count = 0;
                row_data.clear();
            }

            row_data.push_back(t_value);
            column_count++;
        }
    } else {
        fp.close();
        return EXIT_FAILURE;
    }

    fp.close();
    return EXIT_SUCCESS;
}


DEM::Index DEM::index(double latitude, double longitude) {
    double dem_latitude_index = 0, dem_longitude_index = 0;

    if (check_coordinates_bounds(latitude, longitude)) {
        if (latitude >= this->bounds.SW.latitude) {
            // Northern Hemisphere
            dem_latitude_index = (this->bounds.NE.latitude - latitude) / this->type.cellsize;
        } else {
            // Southern Hemisphere
            dem_latitude_index = (latitude - this->bounds.SW.latitude) / this->type.cellsize;
        }

        if (longitude >= this->bounds.SW.longitude) {
            // Eastern Hemisphere
            dem_longitude_index = (longitude - this->bounds.SW.longitude) / this->type.cellsize;
        } else {
            // Western Hemisphere
            dem_longitude_index = (this->bounds.NE.longitude - longitude) / this->type.cellsize;
        }
    } else {
        return {
            static_cast<double>(this->type.nodata),
            static_cast<double>(this->type.nodata)
        };
    }

    return {
        dem_latitude_index,
        dem_longitude_index
    };
}


DEM::DEM(const Type& type, const std::string& filepath) {
    this->type = type;
    this->bounds = {
        {this->type.yllcorner + (this->type.cellsize * this->type.nrows), this->type.xllcorner},
        {this->type.yllcorner + (this->type.cellsize * this->type.nrows), this->type.xllcorner + (this->type.cellsize * this->type.ncols)},
        {this->type.yllcorner, this->type.xllcorner},
        {this->type.yllcorner, this->type.xllcorner + (this->type.cellsize * this->type.ncols)}
    };

    // read the DEM file (sets: this->data)
    if (read(filepath) != 0) std::runtime_error("\nfailed to read DEM file");
}


DEM::~DEM() {
    this->data.clear();
}


bool DEM::check_coordinates_bounds(double latitude, double longitude) {
    if (
        latitude >= this->bounds.SW.latitude
        && latitude < this->bounds.NE.latitude
        && longitude >= this->bounds.SW.longitude
        && longitude < this->bounds.NE.longitude
    )
        return true;
    else
        return false;
}


short int DEM::altitude(double latitude, double longitude) {
    Index rc = index(latitude, longitude);

    if (rc.row == this->type.nodata || rc.column == this->type.nodata) {
        return this->type.nodata;
    }

    size_t r = static_cast<size_t>(std::round(rc.row));
    size_t c = static_cast<size_t>(std::round(rc.column));

    short int altitude = this->data[r][c];

    return altitude;
}


double DEM::interpolated_altitude(double latitude, double longitude) {
    Index rc = index(latitude, longitude);

    if (rc.row == this->type.nodata || rc.column == this->type.nodata) {
        return this->type.nodata;
    }

    size_t r = static_cast<size_t>(rc.row);
    size_t c = static_cast<size_t>(rc.column);

    double del_latitude = std::min(rc.row, static_cast<double>(this->type.nrows-1)) - r;
    double del_longitude = std::min(rc.column, static_cast<double>(this->type.ncols-1)) - c;

    double altitude =   (1-del_latitude) * (1-del_longitude) * this->data[r][c] +
                        del_longitude * (1-del_latitude) * this->data[r][c + 1] +
                        (1-del_longitude) * del_latitude * this->data[r + 1][c] +
                        del_latitude * del_longitude * this->data[r + 1][c + 1];

    return altitude;
}


void DEM::create_dem_asc_bin(const std::string& path) {
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


void DEM::create_dem_asc_csv(const std::string& path, Type type) {
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


void DEM::create_dem_csv_bin(const std::string& path) {
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


void DEM::create_dem_bin_csv(const std::string& path, Type type) {
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
