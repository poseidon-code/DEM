#pragma once

#include <map>
#include <string>

#include "DEM.hpp"



class Map {
public:
    using Type = std::map<DEM::Coordinate, std::pair<DEM::Type, std::string>>;

    Map(const Type& map);
    ~Map() = default;

    short int altitude(float latitude, float longitude);
    float interpolated_altitude(float latitude, float longitude);

    static Type initialize(std::string dem_directory_path, unsigned int nrows, unsigned int ncols, float cellsize, short int nodata);

private:
    DEM dem;
    Type map;

    bool load(float latitude, float longitude);
};
