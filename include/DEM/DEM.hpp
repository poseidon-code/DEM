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

#include <algorithm>
#include <bit>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>
#include <type_traits>
#include <vector>



struct Coordinate {
    float latitude;
    float longitude;

    Coordinate()
        : latitude(0),
        longitude(0)
    {};

    Coordinate(float latitude, float longitude)
        : latitude(latitude),
        longitude(longitude)
    {
        if (latitude > 90 || latitude < -90 || longitude > 180 || longitude < -180) {
            std::string e = "invalid coordinates (" + std::to_string(latitude) +  ":" +  std::to_string(longitude) + ")";
            throw std::runtime_error(e);
        }
    };

    Coordinate(const Coordinate& o) = default;
    Coordinate& operator=(const Coordinate& o) = default;
    Coordinate(Coordinate&& o) noexcept = default;
    Coordinate& operator=(Coordinate&& o) noexcept = default;
    ~Coordinate() = default;

    bool operator<(const Coordinate& o) const {
        if (latitude == o.latitude) {
            return longitude < o.longitude;
        }
        return latitude < o.latitude;
    }

    bool operator==(const Coordinate& o) const {
        return latitude == o.latitude && longitude == o.longitude;
    }
};



struct Bounds {
    Coordinate NW;
    Coordinate NE;
    Coordinate SW;
    Coordinate SE;

    Bounds() = default;

    Bounds(const Coordinate& NW, const Coordinate& NE, const Coordinate& SW, const Coordinate& SE)
        : NW(NW),
        NE(NE),
        SW(SW),
        SE(SE)
    {};

    Bounds(const Bounds& o) = default;
    Bounds& operator=(const Bounds& o) = default;
    Bounds(Bounds&& o) noexcept = default;
    Bounds& operator=(Bounds&& o) noexcept = default;
    ~Bounds() = default;

    bool within(float latitude, float longitude) {
        if (
            latitude >= this->SW.latitude
            && latitude < this->NE.latitude
            && longitude >= this->SW.longitude
            && longitude < this->NE.longitude
        ) {
            return true;
        } else {
            return false;
        }
    }
};




template <typename T>
class GridView2D {
private:
    T* data;
    size_t rows;
    size_t columns;

public:
    GridView2D() : this->data(nullptr), this->rows(0), this->columns(0) {}
    GridView2D(T* data, size_t rows, size_t cols) : this->data(data), this->rows(rows), this->columns(cols) {}
    T& operator()(size_t r, size_t c) { return this->data[r * this->columns + c]; }
    const T& operator()(size_t r, size_t c) const { return this->data[r * this->columns + c]; }
};



template <typename T>
concept dem_datatype =
    std::is_arithmetic_v<T> &&
    !std::is_same_v<T, bool> &&
    !std::is_same_v<T, char> &&
    !std::is_same_v<T, signed char> &&
    !std::is_same_v<T, unsigned char> &&
    !std::is_same_v<T, char16_t> &&
    !std::is_same_v<T, char32_t> &&
    !std::is_same_v<T, wchar_t>;



template <dem_datatype T, std::endian E = std::endian::native>
class DEM {
private:
    struct Index {
        float row;
        float column;
    };

    std::vector<std::byte> raw_bytes;
    std::vector<T> buffer;
    GridView2D<T> grid{nullptr, 0, 0};


    void read(const std::filesystem::path& filepath) {
        const size_t elem_count = this->type.nrows * this->type.ncols;
        const size_t byte_count = elem_count * sizeof(T);

        std::ifstream fp(filepath, std::ios::binary | std::ios::ate);
        if (!fp) {
            throw std::runtime_error("failed to open DEM file");
        }

        const std::streamsize file_size = fp.tellg();
        if (file_size != static_cast<std::streamsize>(byte_count)) {
            throw std::runtime_error("DEM file size does not match expected dimensions");
        }

        fp.seekg(0, std::ios::beg);
        this->raw_bytes.resize(byte_count);
        if (!fp.read(reinterpret_cast<char*>(raw_bytes.data()), file_size)) {
            throw std::runtime_error("failed to read DEM file");
        }
        fp.close();


        this->buffer.resize(elem_count);
        if constexpr (E == std::endian::native) {
            std::copy(this->raw_bytes.begin(), this->raw_bytes.end(), reinterpret_cast<std::byte*>(this->buffer.data()));
        } else {
            T t_value = 0;

            for (size_t i = 0; i < elem_count; ++i) {
                std::copy(
                    this->raw_bytes.begin() + i * sizeof(T),
                    this->raw_bytes.begin() + (i + 1) * sizeof(T),
                    reinterpret_cast<std::byte*>(&t_value)
                );

                auto* b = reinterpret_cast<std::byte*>(&t_value);
                std::reverse(b, b + sizeof(T));

                this->buffer[i] = t_value;
            }
        }

        this->grid = GridView2D<T>(this->buffer.data(), this->type.nrows, this->type.ncols);
    }


    Index index(float latitude, float longitude) {
        float dem_latitude_index = 0, dem_longitude_index = 0;

        if (this->bounds.within(latitude, longitude)) {
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
    };


public:
    struct Type {
        size_t nrows;       // no. of DEM values available in row
        size_t ncols;       // no. of DEM values available in column
        float yllcorner;    // bottom left latitude
        float xllcorner;    // bottom left longitude
        float cellsize;     // distance (in radians) between every DEM values
        T nodata;           // invalid DEM value representation

        Type()
            : nrows(0),
            ncols(0),
            yllcorner(0),
            xllcorner(0),
            cellsize(0),
            nodata(0)
        {};

        Type (size_t nrows, size_t ncols, float yllcorner, float xllcorner, float cellsize, T nodata)
            : nrows(nrows),
            ncols(ncols),
            yllcorner(yllcorner),
            xllcorner(xllcorner),
            cellsize(cellsize),
            nodata(nodata)
        {
            if (nrows == 0 || ncols == 0) {
                throw std::runtime_error("invalid data dimensions, nrows = 0 & ncols = 0");
            }
            if (yllcorner > 89 || yllcorner < -90 || xllcorner > 179 || xllcorner < -180) {
                std::string e = "invalid coordinates (" + std::to_string(yllcorner) +  ":" +  std::to_string(xllcorner) + ")";
                throw std::runtime_error(e);
            }
        };

        Type(const Type& o) = default;
        Type& operator=(const Type& o) = default;
        Type(Type&& o) noexcept = default;
        Type& operator=(Type&& o) noexcept = default;
        ~Type() = default;
    };


    Type type;
    Bounds bounds;


    DEM() = default;


    DEM(const Type& type, const std::filesystem::path& filepath) {
        this->type = type;
        this->bounds = {
            {this->type.yllcorner + (this->type.cellsize * this->type.nrows), this->type.xllcorner},
            {this->type.yllcorner + (this->type.cellsize * this->type.nrows), this->type.xllcorner + (this->type.cellsize * this->type.ncols)},
            {this->type.yllcorner, this->type.xllcorner},
            {this->type.yllcorner, this->type.xllcorner + (this->type.cellsize * this->type.ncols)}
        };

        if (!std::filesystem::exists(filepath)) {
            std::string e = "DEM file '" + filepath.string() + "' not found";
            throw std::runtime_error(e);
        }

        this->read(filepath);
    };


    DEM(const DEM& other) = default;
    DEM& operator=(const DEM& other) = default;
    DEM(DEM&& other) noexcept = default;
    DEM& operator=(DEM&& other) noexcept = default;
    ~DEM() = default;


    T altitude(float latitude, float longitude) {
        auto [row, column] = this->index(latitude, longitude);

        if (row == this->type.nodata || column == this->type.nodata) {
            return this->type.nodata;
        }

        size_t r = static_cast<size_t>(std::round(row));
        size_t c = static_cast<size_t>(std::round(column));

        r = r == this->type.nrows ? r - 1 : r;
        c = c == this->type.ncols ? c - 1 : c;

        T altitude = this->grid(r, c);

        return altitude;
    };


    float interpolated_altitude(float latitude, float longitude) {
        auto [row, column] = this->index(latitude, longitude);

        if (row == this->type.nodata || column == this->type.nodata) {
            return this->type.nodata;
        }

        size_t r = static_cast<size_t>(row);
        size_t c = static_cast<size_t>(column);

        float del_latitude = std::min(row, static_cast<float>(this->type.nrows - 1)) - r;
        float del_longitude = std::min(column, static_cast<float>(this->type.ncols - 1)) - c;

        size_t next_r = (r == this->type.nrows - 1) ? r : r + 1;
        size_t next_c = (c == this->type.ncols - 1) ? c : c + 1;

        float altitude =    (1 - del_latitude) * (1 - del_longitude) * this->grid(r, c) +
                            del_longitude * (1 - del_latitude) * this->grid(r, next_c) +
                            (1 - del_longitude) * del_latitude * this->grid(next_r, c) +
                            del_latitude * del_longitude * this->grid(next_r, next_c);

        return altitude;
    };
};
