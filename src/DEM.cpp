#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <string>
#include <system_error>
#include <vector>

#include "DEM.hpp"


int16_t DEM::read(const std::string& filepath) {
    std::ifstream fp(filepath, std::ios::binary);
    int16_t t_value = 0;

    if (fp.good() && !fp.eof()) {
        size_t column_count = 0;
        std::vector<int16_t> row_data;

        while (fp.read(reinterpret_cast<char*>(&t_value), sizeof(int16_t))) {
            row_data.push_back(t_value);
            column_count++;

            if (column_count == this->type.ncols) {
                this->data.push_back(row_data);
                column_count = 0;
                row_data.clear();
            }
        }
    } else {
        fp.close();
        return EXIT_FAILURE;
    }

    fp.close();
    return EXIT_SUCCESS;
}


DEM::Index DEM::index(float latitude, float longitude) {
    float dem_latitude_index = 0, dem_longitude_index = 0;

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
            static_cast<float>(this->type.nodata),
            static_cast<float>(this->type.nodata)
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


bool DEM::check_coordinates_bounds(float latitude, float longitude) {
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


short int DEM::altitude(float latitude, float longitude) {
    Index rc = index(latitude, longitude);

    if (rc.row == this->type.nodata || rc.column == this->type.nodata) {
        return this->type.nodata;
    }

    size_t r = static_cast<size_t>(std::round(rc.row));
    size_t c = static_cast<size_t>(std::round(rc.column));

    int16_t altitude = this->data[r][c];

    return altitude;
}


float DEM::interpolated_altitude(float latitude, float longitude) {
    Index rc = index(latitude, longitude);

    if (rc.row == this->type.nodata || rc.column == this->type.nodata) {
        return this->type.nodata;
    }

    size_t r = static_cast<size_t>(rc.row);
    size_t c = static_cast<size_t>(rc.column);

    float del_latitude = std::min(rc.row, static_cast<float>(this->type.nrows-1)) - r;
    float del_longitude = std::min(rc.column, static_cast<float>(this->type.ncols-1)) - c;

    size_t next_r = (r == this->type.nrows-1) ? r : r + 1;
    size_t next_c = (c == this->type.ncols-1) ? c : c + 1;

    float altitude =   (1-del_latitude) * (1-del_longitude) * this->data[r][c] +
                        del_longitude * (1-del_latitude) * this->data[r][next_c] +
                        (1-del_longitude) * del_latitude * this->data[next_r][c] +
                        del_latitude * del_longitude * this->data[next_r][next_c];

    return altitude;
}
