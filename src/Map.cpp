#include <cmath>
#include <filesystem>
#include <regex>
#include <string>
#include <system_error>

#include "DEM.hpp"
#include "Map.hpp"


Map::Map(const Type& map) {
    if (map.empty()) {
        throw std::runtime_error("map is empty\n");
    }

    for (auto m = map.cbegin(); m != map.cend(); m++) {
        std::string file_path = m->second.second;

        if (!std::filesystem::exists(file_path)) {
            std::string e = "'" + file_path + "' file doesn't exists\n";
            throw std::runtime_error(e);
        }
    }

    this->map = map;

    auto first_dem = this->map.cbegin();
    this->dem = DEM(this->map[first_dem->first].first, this->map[first_dem->first].second);
}


bool Map::load(float latitude, float longitude) {
    DEM::Coordinate map_coordinate{
        std::floor(latitude),
        std::floor(longitude)
    };

    if (this->map.count(map_coordinate) == 0) return false;
    this->dem = DEM(this->map[map_coordinate].first, this->map[map_coordinate].second);

    return true;
}


short int Map::altitude(float latitude, float longitude) {
    if (!this->dem.check_coordinates_bounds(latitude, longitude)) {
        if (!this->load(latitude, longitude)) {
            return this->dem.type.nodata;
        }
    }

    return this->dem.altitude(latitude, longitude);
}


float Map::interpolated_altitude(float latitude, float longitude) {
    if (!this->dem.check_coordinates_bounds(latitude, longitude)) {
        if (!this->load(latitude, longitude)) {
            return this->dem.type.nodata;
        }
    }

    return this->dem.interpolated_altitude(latitude, longitude);
}


Map::Type Map::initialize(std::string dem_directory_path, unsigned int nrows, unsigned int ncols, float cellsize, short int nodata) {
    Map::Type map;
    std::regex pattern(R"(([-]?\d{1,2}|90)_([-]?\d{1,3}|180)\.bin)");

    try {
        for (const auto& entry : std::filesystem::directory_iterator(dem_directory_path)) {
            if (std::filesystem::is_regular_file(entry.status())) {
                std::string filename = entry.path().filename().string();
                std::smatch match;

                if (std::regex_match(filename, match, pattern)) {
                    float latitude = std::stod(match[1]);
                    float longitude = std::stod(match[2]);

                    if (
                        (latitude >= -90 && latitude <= 90)
                        && (longitude >= -180 && longitude <= 180)
                    ) {
                        DEM::Type type(nrows, ncols, latitude, longitude, cellsize, nodata);
                        map[{latitude, longitude}] = {type, entry.path().string()};
                    }
                }
            }
        }
    } catch (const std::filesystem::filesystem_error& e) {
        throw std::runtime_error(std::string(e.what()) + "\n");
    }

    return map;
}
