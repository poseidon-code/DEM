#pragma once

#include <fstream>
#include <string>
#include <vector>


struct Coordinate {
    double latitude;
    double longitude;
};


class DEM {
private:
    struct Bounds {
        Coordinate NW;
        Coordinate NE;
        Coordinate SW;
        Coordinate SE;
    };

    struct Index {
        double row;
        double column;
    };

    int read(const std::string &filepath);
    Coordinate deduce_filename(const std::string &filepath);
    bool check_filename(const std::string &filepath);
    bool check_coordinates_bounds(double latitude, double longitude);
    Index index(double latitude, double longitude);


public:
    struct Type {
        unsigned int resolution;    // distance between 2 DEM values (in metres) i.e. '<x>' arc second(s) = '<resolution>' metres
        unsigned int width;         // no. of DEM values available in row/column
        double cellsize;            // distance (in radians) between every DEM values (distance (in radians) b/w DEM bounds / DEM width)
        
        Type(): resolution(), width(), cellsize() {};

        Type(unsigned int resolution, unsigned int width, double range) {
            this->resolution = resolution;
            this->width = width;
            this->cellsize = range / this->width;
        }
    };

    std::vector<short int> data;
    Type type;
    Bounds bounds;

    DEM(const Type &type, const std::string &filepath);
    ~DEM() = default;

    short int altitude(double latitude, double longitude);
    double interpolated_altitude(double latitude, double longitude);
    std::vector<short int> patch(double latitude, double longitude, unsigned int radius);
    
    static void create_dem_asc_bin(const std::string &path);
    static void create_dem_asc_csv(const std::string &path, Type type);
    static void create_dem_csv_bin(const std::string &path);
    static void create_dem_bin_csv(const std::string &path, Type type);
};
