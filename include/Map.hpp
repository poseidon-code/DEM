#pragma once

#include <cstdint>
#include <filesystem>
#include <map>
#include <regex>
#include <string>

#include "DEM.hpp"


template <dem_datatype T, bool little_endian = true>
class Map {
public:
    using Grid = std::map<Coordinate, std::pair<typename DEM<T, little_endian>::Type, std::string>>;


    Map(const Grid& grid) {
        if (grid.empty()) {
            throw std::runtime_error("map grid is empty\n");
        }

        for (auto m = grid.cbegin(); m != grid.cend(); m++) {
            std::string file_path = m->second.second;

            if (!std::filesystem::exists(file_path)) {
                std::string e = "'" + file_path + "' file doesn't exists\n";
                throw std::runtime_error(e);
            }
        }

        this->grid = grid;

        auto first_dem = this->grid.cbegin();
        this->dem = DEM<T, little_endian>(this->grid[first_dem->first].first, this->grid[first_dem->first].second);
    };


    ~Map() = default;


    T altitude(float latitude, float longitude) {
        if (!this->dem.bounds.within(latitude, longitude)) {
            if (!this->load(latitude, longitude)) {
                return this->dem.type.nodata;
            }
        }

        return this->dem.altitude(latitude, longitude);
    };


    float interpolated_altitude(float latitude, float longitude) {
        if (!this->dem.bounds.within(latitude, longitude)) {
            if (!this->load(latitude, longitude)) {
                return this->dem.type.nodata;
            }
        }

        return this->dem.interpolated_altitude(latitude, longitude);
    };


    static Grid initialize(std::string dem_directory_path, size_t nrows, size_t ncols, float cellsize, T nodata) {
        Map<T, little_endian>::Grid grid;
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
                            typename DEM<T, little_endian>::Type type(nrows, ncols, latitude, longitude, cellsize, nodata);
                            grid[{latitude, longitude}] = {type, entry.path().string()};
                        }
                    }
                }
            }
        } catch (const std::filesystem::filesystem_error& e) {
            throw std::runtime_error(std::string(e.what()) + "\n");
        }

        return grid;
    };


private:
    DEM<T, little_endian> dem;
    Grid grid;


    bool load(float latitude, float longitude) {
        Coordinate grid_coordinate{
            std::floor(latitude),
            std::floor(longitude)
        };

        if (this->grid.count(grid_coordinate) == 0) return false;
        this->dem = DEM<T, little_endian>(this->grid[grid_coordinate].first, this->grid[grid_coordinate].second);

        return true;
    };
};
