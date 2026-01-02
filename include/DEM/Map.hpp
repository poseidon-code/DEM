/*
MIT License

Copyright (c) 2023 Pritam Halder

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without
limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

Author : Pritam Halder
Email : pritamhalder.portfolio@gmail.com
*/

#pragma once

#include <array>
#include <bit>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <map>
#include <optional>
#include <ranges>
#include <regex>
#include <string>


#include "DEM.hpp"


template <dem_datatype T, std::endian E = std::endian::native>
class Map {
public:
    struct GridAvailableEntry {
        std::filesystem::path path;
        DEM<T, E>::Type type;
    };

    struct Tile {
        Coordinate id;
        std::optional<DEM<T, E>> dem;
    };

    using Grid = std::array<Tile, 9>;
    using GridAvailable = std::map<Coordinate, GridAvailableEntry>;


    Map() = default;

    Map(const GridAvailable& available) {
        if (available.empty()) {
            throw std::runtime_error("available DEM list is empty\n");
        }

        for (auto entry = available.cbegin();  entry != available.cend(); entry++) {
            if (!std::filesystem::exists(entry->second.path) || !std::filesystem::is_regular_file(entry->second.path)) {
                std::string e = "'" + entry->second.path.string() + "' file doesn't exists or not a regular file\n";
                throw std::invalid_argument(e);
            }
        }

        this->available = std::move(available);
    };


    ~Map() = default;


    std::optional<std::reference_wrapper<DEM<T, E>>> get_dem(float latitude, float longitude) {
        Coordinate tile_coordinate{std::floor(latitude), std::floor(longitude)};

        auto tile = std::ranges::find_if(
            this->map.begin(),
            this->map.end(),
            [&](const Tile& t) -> bool { return (t.id == tile_coordinate); }
        );

        if (tile != this->map.end() && tile->dem) {
            return std::ref(*tile->dem);
        }

        return std::nullopt;
    }


    T altitude(float latitude, float longitude) {
        if (!this->load_bounds.within(latitude, longitude)) {
            this->load(latitude, longitude);
        }

        if (auto t_dem = this->get_dem(latitude, longitude)) {
            return t_dem->get().altitude(latitude, longitude);
        }

        return std::numeric_limits<T>::lowest();
    };


    float interpolated_altitude(float latitude, float longitude) {
        if (!this->load_bounds.within(latitude, longitude)) {
            this->load(latitude, longitude);
        }

        if (auto t_dem = this->get_dem(latitude, longitude)) {
            return t_dem->get().interpolated_altitude(latitude, longitude);
        }

        return std::numeric_limits<T>::lowest();
    };


    static GridAvailable list_available(const std::filesystem::path& dem_directory_path, size_t nrows, size_t ncols, float cellsize, T nodata) {
        const std::regex pattern = std::regex(R"(([-]?\d{1,2}|90)_([-]?\d{1,3}|180)\.bin)");
        GridAvailable result;

        try {
            for (const auto& entry : std::filesystem::directory_iterator(dem_directory_path)) {
                if (std::filesystem::is_regular_file(entry.status())) {
                    std::string filename = entry.path().filename().string();
                    std::smatch match;

                    if (std::regex_match(filename, match, pattern)) {
                        float y = std::stod(match[1]);
                        float x = std::stod(match[2]);

                        if ((y >= -90 && y <= 89) && (x >= -180 && x <= 179)) {
                            typename DEM<T, E>::Type type(nrows, ncols, y, x, cellsize, nodata);
                            result[{y, x}] = { entry.path(), type };
                        }
                    }
                }
            }
        } catch (const std::exception& e) {
            throw std::runtime_error(std::string(e.what()) + "\n");
        }

        return result;
    }


private:
    Grid map;
    Coordinate map_center;
    GridAvailable available;
    Bounds load_bounds;

    const std::array<std::pair<float,float>, 9> offsets{{
        {+1,-1}, {+1,0}, {+1,+1},
        { 0,-1}, { 0,0}, { 0,+1},
        {-1,-1}, {-1,0}, {-1,+1}
    }};


    void load(float latitude, float longitude) {
        Coordinate tile_coordinate{
            std::floor(latitude),
            std::floor(longitude)
        };

        if (!this->load_bounds.within(latitude, longitude)) {
            this->map_center.latitude = tile_coordinate.latitude + (((tile_coordinate.latitude + 1) - tile_coordinate.latitude) / 2.0);
            this->map_center.longitude = tile_coordinate.longitude + (((tile_coordinate.longitude + 1) - tile_coordinate.longitude) / 2.0);

            this->load_bounds = Bounds{
                {this->map_center.latitude + 1, this->map_center.longitude - 1},
                {this->map_center.latitude + 1, this->map_center.longitude + 1},
                {this->map_center.latitude - 1, this->map_center.longitude - 1},
                {this->map_center.latitude - 1, this->map_center.longitude + 1}
            };


            for (size_t i = 0; i < 9; i++) {
                Coordinate c{
                    tile_coordinate.latitude + this->offsets[i].first,
                    tile_coordinate.longitude + this->offsets[i].second
                };

                // tile is already avaialble and loaded
                auto tile = std::ranges::find_if(this->map.begin(), this->map.end(), [&](Tile t) -> bool {return (t.id == c);});
                if (tile != this->map.end()) {
                    this->map[i] = std::move(*tile);
                    continue;
                }

                // tile is avaialble but needs to be loaded
                this->map[i].id = c;
                auto entry = this->available.find(c);
                if (entry != this->available.end()) {
                    this->map[i].dem.emplace(entry->second.type, entry->second.path);
                    continue;
                }

                // tile is not avaialble
                this->map[i].dem.reset();
                this->map[i].dem = std::nullopt;
            }
        }
    };
};
