#pragma once

#include <map>
#include <string>

#include "DEM.hpp"



class Map {
public:
    using Type = std::map<DEM::Coordinate, std::pair<DEM::Type, std::string>>;

    Map(const Type& map);
    ~Map() = default;

    short int altitude(double latitude, double longitude);
    double interpolated_altitude(double latitude, double longitude);

    static Type InitializeDEMMap(std::string dem_directory_path);

private:
    DEM dem;
    Type map;

    bool load(double latitude, double longitude);
};
