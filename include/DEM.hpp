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
