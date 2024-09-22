#pragma once

#include <cmath>
#include <cstdint>
#include <map>
#include <regex>
#include <string>

#ifdef _WIN32
#include <windows.h>
#else
#include <dirent.h>
#endif

#include "DEM.hpp"


template <typename T, bool little_endian = true, enable_if_dem_datatype<T> = 0>
class Map {
public:
    using Grid = std::map<Coordinate, std::pair<typename DEM<T, little_endian>::Type, std::string>>;

    Map() = default;

    Map(const Grid& grid) {
        if (grid.empty()) {
            throw std::runtime_error("map grid is empty\n");
        }

        for (auto m = grid.cbegin(); m != grid.cend(); m++) {
            std::string file_path = m->second.second;

            if (!std::ifstream(file_path).good()) {
                std::string e = "'" + file_path + "' file doesn't exists\n";
                throw std::runtime_error(e);
            }
        }

        this->grid = grid;

        auto first_dem = this->grid.cbegin();
        this->dem = DEM<T, little_endian>(this->grid[first_dem->first].first, this->grid[first_dem->first].second);
    };


    ~Map() = default;


    const DEM<T, little_endian>& get_dem() const {
        return this->dem;
    }


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
        typename Map<T, little_endian>::Grid grid;
        std::regex pattern(R"(([-]?\d{1,2}|90)_([-]?\d{1,3}|180)\.bin)");

        std::vector<std::string> file_list;

        #ifdef _WIN32
            WIN32_FIND_DATA t_file;
            HANDLE finde_handle = FindFirstFile((dem_directory_path + "\\*").c_str(), &t_file);

            if (finde_handle == INVALID_HANDLE_VALUE) {
                throw std::runtime_error("error reading directory '" + dem_directory_path + "'");
            }

            do {
                if (!(t_file.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    file_list.push_back(dem_directory_path + t_file.cFileName);
                }
            } while (FindNextFile(finde_handle, &t_file) != 0);

            FindClose(finde_handle);
        #else
            DIR* dir;
            struct dirent* entry;

            if ((dir = opendir(dem_directory_path.c_str())) != nullptr) {
                while ((entry = readdir(dir)) != nullptr) {
                    if (entry->d_type != DT_DIR) {
                        file_list.push_back(dem_directory_path + entry->d_name);
                    }
                }
                closedir(dir);
            } else {
                throw std::runtime_error("error reading directory '" + dem_directory_path + "'");
            }
        #endif

        for (const auto& filepath : file_list) {
            std::string filename = filepath.substr(filepath.find_last_of("/\\") + 1);
            std::smatch match;
            if (std::regex_match(filename, match, pattern)) {
                float latitude = std::stof(match[1]);
                float longitude = std::stof(match[2]);

                if (
                    (latitude >= -90 && latitude <= 90)
                    && (longitude >= -180 && longitude <= 180)
                ) {
                    typename DEM<T, little_endian>::Type type(nrows, ncols, latitude, longitude, cellsize, nodata);
                    grid[{latitude, longitude}] = {type, filepath};
                }
            }
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
