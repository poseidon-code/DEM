#pragma once

#include <fstream>
#include <string>
#include <vector>



class DEM {
private:
    struct Index {
        double row;
        double column;
    };

    int read(const std::string& filepath);
    bool check_coordinates_bounds(double latitude, double longitude);
    Index index(double latitude, double longitude);


public:
    struct Coordinate {
        double latitude;
        double longitude;

        Coordinate() {
            this->latitude = 0;
            this->longitude = 0;
        };

        Coordinate(double latitude, double longitude) {
            if (latitude > 90 || latitude < -90 || longitude > 180 || longitude < -180) {
                std::string e = "invalid coordinates (" + std::to_string(latitude) +  ":" +  std::to_string(longitude) + ")";
                throw std::runtime_error(e);
            }
            this->latitude = latitude;
            this->longitude = longitude;
        };
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
        double yllcorner;           // bottom left latitude
        double xllcorner;           // bottom left longitude
        double cellsize;            // distance (in radians) between every DEM values
        short int nodata;           // invalid DEM value representation

        Type() {
            this->nrows = 0;
            this->ncols = 0;
            this->yllcorner = 0;
            this->xllcorner = 0;
            this->cellsize = 0;
            this->nodata = 0;
        };

        Type (unsigned int nrows, unsigned int ncols, double yllcorner, double xllcorner, double cellsize, short int nodata) {
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

    std::vector<short int> data;
    Type type;
    Bounds bounds;

    DEM(const Type& type, const std::string& filepath);
    ~DEM() = default;

    short int altitude(double latitude, double longitude);
    double interpolated_altitude(double latitude, double longitude);
    std::vector<std::vector<short int>> patch(double latitude, double longitude, unsigned int radius);

    static void create_dem_asc_bin(const std::string& path);
    static void create_dem_asc_csv(const std::string& path, Type type);
    static void create_dem_csv_bin(const std::string& path);
    static void create_dem_bin_csv(const std::string& path, Type type);
};
