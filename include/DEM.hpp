#pragma once

#include <fstream>
#include <string>
#include <vector>



class DEM {
private:
    struct Index {
        float row;
        float column;
    };

    int read(const std::string& filepath);
    Index index(float latitude, float longitude);


public:
    struct Coordinate {
        float latitude;
        float longitude;

        Coordinate() {
            this->latitude = 0;
            this->longitude = 0;
        };

        Coordinate(float latitude, float longitude) {
            if (latitude > 90 || latitude < -90 || longitude > 180 || longitude < -180) {
                std::string e = "invalid coordinates (" + std::to_string(latitude) +  ":" +  std::to_string(longitude) + ")";
                throw std::runtime_error(e);
            }
            this->latitude = latitude;
            this->longitude = longitude;
        };

        bool operator<(const Coordinate& other) const {
            if (latitude == other.latitude) {
                return longitude < other.longitude;
            }
            return latitude < other.latitude;
        }

        bool operator==(const Coordinate& other) const {
            return latitude == other.latitude && longitude == other.longitude;
        }
    };

    struct Bounds {
        Coordinate NW;
        Coordinate NE;
        Coordinate SW;
        Coordinate SE;
    };

    struct Type {
        unsigned int nrows;         // no. of DEM values available in row
        unsigned int ncols;         // no. of DEM values available in column
        float yllcorner;           // bottom left latitude
        float xllcorner;           // bottom left longitude
        float cellsize;            // distance (in radians) between every DEM values
        short int nodata;           // invalid DEM value representation

        Type() {
            this->nrows = 0;
            this->ncols = 0;
            this->yllcorner = 0;
            this->xllcorner = 0;
            this->cellsize = 0;
            this->nodata = 0;
        };

        Type (unsigned int nrows, unsigned int ncols, float yllcorner, float xllcorner, float cellsize, short int nodata) {
            if (yllcorner > 90 || yllcorner < -90 || xllcorner > 180 || xllcorner < -180) {
                std::string e = "invalid coordinates (" + std::to_string(yllcorner) +  ":" +  std::to_string(xllcorner) + ")";
                throw std::runtime_error(e);
            }
            this->nrows = nrows;
            this->ncols = ncols;
            this->yllcorner = yllcorner;
            this->xllcorner = xllcorner;
            this->cellsize = cellsize;
            this->nodata = nodata;
        };
    };

    std::vector<std::vector<short int>> data;
    Type type;
    Bounds bounds;

    DEM() = default;
    DEM(const Type& type, const std::string& filepath);
    ~DEM();

    bool check_coordinates_bounds(float latitude, float longitude);
    short int altitude(float latitude, float longitude);
    float interpolated_altitude(float latitude, float longitude);
};
