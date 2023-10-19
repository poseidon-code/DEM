#pragma once

#include <algorithm>
#include <cmath>
#include <fstream>
#include <numeric>
#include <regex>
#include <sstream>
#include <string>
#include <system_error>
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



    int read(const std::string &filepath) {
        std::ifstream fp = std::ifstream(filepath, std::ios::binary | std::ios::in);

        short int t_value;

        if (fp.good() && !fp.eof()) {
            while (fp.read(reinterpret_cast<char*>(&t_value), sizeof(short int)))
                data.push_back(t_value);
        } else {
            fp.close();
            return EXIT_FAILURE;
        }

        fp.close();
        return EXIT_SUCCESS;
    }


    Coordinate deduce_filename(const std::string &filepath) {
        size_t found = filepath.find_last_of("/\\");
        std::string filename;
        Coordinate coordinate;
        
        if (found != std::string::npos)
            filename = filepath.substr(found + 1);
        else 
            filename = filepath;

        size_t seperator_position = filename.find('_');

        if (seperator_position != std::string::npos) {
            std::string latitude_s = filename.substr(0, seperator_position);
            std::string longitude_s = filename.substr(seperator_position + 1);

            std::istringstream(latitude_s) >> coordinate.latitude;
            std::istringstream(longitude_s) >> coordinate.longitude;
        } 

        return coordinate;
    }


    bool check_filename(const std::string &filepath) {
        size_t found = filepath.find_last_of("/\\");
        std::string filename;

        if (found != std::string::npos)
            filename = filepath.substr(found + 1);
        else 
            filename = filepath;

        std::regex pattern(R"(\d+_\d+.bin)");

        return std::regex_match(filename, pattern);
    }


    bool check_coordinates_bounds(double latitude, double longitude) {
        if (
            (latitude >= this->bounds.SW.latitude && latitude < this->bounds.NE.latitude)
            && (longitude >= this->bounds.SW.longitude && longitude < this->bounds.NE.longitude)
        )
            return true;
        else
            return false;
    }


    Index index(double latitude, double longitude) {
        double dem_latitude_index = 0, dem_longitude_index = 0;

        if (check_coordinates_bounds(latitude, longitude)) {
            dem_latitude_index = (std::abs(this->bounds.NW.latitude) - latitude) * (this->type.width - 1);
            dem_longitude_index = (longitude - std::abs(this->bounds.SW.longitude)) * (this->type.width - 1);
        } else {
            std::ostringstream err; err.precision(8);
            err << "coordinates provided (" << latitude << "," << longitude << ") is out of bounds of currently loaded DEM file (" << (int)this->bounds.SW.latitude << "_" << (int)this->bounds.SW.longitude << ".bin).\n";
            throw std::runtime_error(err.str());
        }

        return {
            dem_latitude_index,
            dem_longitude_index
        };
    }


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

    
    DEM(const Type &type, const std::string &filepath) {
        if (!check_filename(filepath)) std::runtime_error("\nInvalid DEM file name. (file name not of type - '<latitude>_<longitude>.bin')");
        
        Coordinate fb = deduce_filename(filepath);
        this->type = type;
        this->bounds = {
            {fb.latitude+1, fb.longitude},
            {fb.latitude+1, fb.longitude+1},
            {fb.latitude, fb.longitude},
            {fb.latitude, fb.longitude+1}
        };

        // read the DEM file (sets: this->data)
        if (read(filepath) != 0) std::runtime_error("\nFailed to read DEM file");
    }

    ~DEM() = default;


    short int altitude(double latitude, double longitude) {
        Index rc = index(latitude, longitude);

        size_t r = static_cast<size_t>(std::round(rc.row));
        size_t c = static_cast<size_t>(std::round(rc.column));
        
        short int altitude = this->data[r * this->type.width + c];

        return altitude;
    }


    double interpolated_altitude(double latitude, double longitude) {
        Index rc = index(latitude, longitude);

        size_t r = static_cast<size_t>(rc.row);
        size_t c = static_cast<size_t>(rc.column);

        double del_latitude = std::min(rc.row, static_cast<double>(this->type.width-1)) - r;
        double del_longitude = std::min(rc.column, static_cast<double>(this->type.width-1)) - c;

        double altitude =   (1-del_latitude) * (1-del_longitude) * this->data[(r * this->type.width) + c] +
                            del_longitude * (1-del_latitude) * this->data[r * this->type.width + (c+1)] +
                            (1-del_longitude) * del_latitude * this->data[(r == 0 ? 0 : (r-1)) * this->type.width + c] +
                            del_latitude * del_longitude * this->data[(r == 0 ? 0 : (r-1)) * this->type.width + (c+1)];

        return altitude;
    }


    std::vector<short int> patch(double latitude, double longitude, int radius) {
        Index rc = index(latitude, longitude);
        std::vector<short int> data;

        size_t r = static_cast<size_t>(std::min(rc.row, static_cast<double>(this->type.width-1)));
        size_t c = static_cast<size_t>(std::min(rc.column, static_cast<double>(this->type.width-1)));

        size_t r_start = (r >= radius) ? r - radius : 0;
        size_t r_end = std::min(r + radius + 1, static_cast<size_t>(this->type.width));
        size_t c_start = (c >= radius) ? c - radius : 0;
        size_t c_end = std::min(c + radius + 1, static_cast<size_t>(this->type.width));

        size_t patch_width = radius * 2;

        if (r_start + r_end < patch_width + 1) {
            r_start = 0;
            r_end = std::min(static_cast<size_t>(this->type.width), patch_width + 1);
        }

        if (c_start + c_end < patch_width + 1) {
            c_start = 0;
            c_end = std::min(static_cast<size_t>(this->type.width), patch_width + 1);
        }

        if (r + radius >= this->type.width) {
            r_start = std::max<size_t>(this->type.width - patch_width - 1, 0);
            r_end = this->type.width;
        }

        if (c + radius >= this->type.width) {
            c_start = std::max<size_t>(this->type.width - patch_width - 1, 0);
            c_end = this->type.width;
        }

        for (int i = r_start; i < r_end; i++)
            for (int j = c_start; j < c_end; j++)
                data.push_back(this->data[i * this->type.width + j]);

        return data;
    }
    
    
    static void create_dem_asc_bin(const std::string &path) {
        std::vector<short int> dem_data;
        short int value = 0;
        
        // skip 1st 5 lines of metadata
        std::ifstream ifp(path);
        for (int i=0; i<5; i++) {
            std::string l;
            std::getline(ifp, l);
        }

        // read values to `dem_data`
        while (ifp >> value) dem_data.push_back(value);
        ifp.close();

        // set new file name for '.bin'
        std::string ofp_path;
        size_t ext_pos = path.find_last_of('.');
        if (ext_pos != std::string::npos) ofp_path = path.substr(0, ext_pos + 1) + std::string("bin");

        // write `dem_data` in binary format
        std::ofstream ofp(ofp_path, std::ios::binary | std::ios::trunc);
        ofp.write(reinterpret_cast<char*>(dem_data.data()), dem_data.size() * sizeof(short int));
        ofp.close();
    }


    static void create_dem_asc_csv(const std::string &path, Type type) {
        std::vector<short int> dem_data;
        short int value = 0;
        
        // skip 1st 5 lines of metadata
        std::ifstream ifp(path);
        for (int i=0; i<5; i++) {
            std::string l;
            std::getline(ifp, l);
        }

        // read values to `dem_data`
        while (ifp >> value) dem_data.push_back(value);
        ifp.close();

        // set new file name for '.bin'
        std::string ofp_path;
        size_t ext_pos = path.find_last_of('.');
        if (ext_pos != std::string::npos) ofp_path = path.substr(0, ext_pos + 1) + std::string("csv");

        // write as per '.csv' format
        std::ofstream ofp(ofp_path);
        for (int i = 0; i < type.width * type.width; ++i) {
            if (i % type.width == type.width - 1)
                ofp << dem_data[i] << "\n";
            else
                ofp << dem_data[i] << ",";
        }
        ofp.close();
    }


    static void create_dem_csv_bin(const std::string &path) {
        std::vector<short int> dem_data;
        short int value = 0;

        // read values to `dem_data`
        std::ifstream ifp(path);
        std::string line;
        while (std::getline(ifp, line)) {
            std::istringstream iss(line);
            std::string token;
            while (std::getline(iss, token, ','))
                dem_data.push_back(static_cast<short int>(std::stoi(token)));
        }
        ifp.close();

        // set new file name for '.bin'
        std::string ofp_path;
        size_t ext_pos = path.find_last_of('.');
        if (ext_pos != std::string::npos) ofp_path = path.substr(0, ext_pos + 1) + std::string("bin");

        // write `dem_data` in binary format
        std::ofstream ofp(ofp_path, std::ios::binary | std::ios::trunc);
        ofp.write(reinterpret_cast<char*>(dem_data.data()), dem_data.size() * sizeof(short int));
        ofp.close();
    }


    static void create_dem_bin_csv(const std::string &path, Type type) {
        std::vector<short int> dem_data;
        short int value = 0;

        // read from .bin file
        std::ifstream ifp(path);
        while (ifp.read(reinterpret_cast<char*>(&value), sizeof(short int))) dem_data.push_back(value);
        ifp.close();

        // set new file name for '.csv'
        std::string ofp_path;
        size_t ext_pos = path.find_last_of('.');
        if (ext_pos != std::string::npos) ofp_path = path.substr(0, ext_pos + 1) + std::string("csv");

        // write as per '.csv' format
        std::ofstream ofp(ofp_path);
        for (int i = 0; i < type.width*type.width; ++i) {
            if (i % type.width == type.width - 1)
                ofp << dem_data[i] << "\n";
            else
                ofp << dem_data[i] << ",";
        }
        ofp.close();
    }
};
