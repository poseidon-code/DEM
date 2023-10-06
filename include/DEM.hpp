#pragma once

#include <fstream>
#include <string>
#include <vector>



struct DEMLEVEL {
    unsigned short int level;   // user determined, based on width & resolution
    unsigned int resolution;    // distance between 2 DEM values (in metres) i.e. 1 arc second = 30 metres, 15 arc seconds = 450 metres
    unsigned int width;         // no. of DEM values available in row/column
    double cellsize;            // distance (in radians) between every DEM values
};

const static DEMLEVEL 
    L1{1, 450, 3600, 1.0/3600},
    L2{2, 90, 3600, 1.0/3600},
    L3{3, 30, 3600, 1.0/3600};



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
    Coordinate deduce_filename(const std::string& filepath);
    bool check_filename(const std::string& filepath);
    bool check_coordinates_bounds(double latitude, double longitude);
    Index index(double latitude, double longitude);

public:
    std::vector<short int> data;
    DEMLEVEL dl;
    Bounds bounds;

    DEM(DEMLEVEL dl, const std::string &filepath);
    ~DEM() = default;

    short int altitude(double latitude, double longitude);
    double interpolated_altitude(double latitude, double longitude);
    std::vector<short int> patch(double latitude, double longitude, int radius);
    
    
    static void create_dem_asc_bin(const std::string &path);
    static void create_dem_asc_csv(const std::string &path, DEMLEVEL dl);
    static void create_dem_bin_csv(const std::string &path, DEMLEVEL dl);
    static void create_dem_csv_bin(const std::string &path);
};
