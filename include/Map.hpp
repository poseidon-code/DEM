#pragma once

#include <cstdint>
#include <map>
#include <string>

#include "DEM.hpp"



class Map {
public:
    using Type = std::map<DEM::Coordinate, std::pair<DEM::Type, std::string>>;

    Map(const Type& map);
    ~Map() = default;

    int16_t altitude(float latitude, float longitude);
    float interpolated_altitude(float latitude, float longitude);

    static Type initialize(std::string dem_directory_path, size_t nrows, size_t ncols, float cellsize, int16_t nodata);

private:
    DEM dem;
    Type map;

    bool load(float latitude, float longitude);
};
