#pragma once

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <system_error>

#include "DEM.hpp"


template <typename T, bool little_endian = true, enable_if_dem_datatype<T> = 0>
class Utility {
private:
    static constexpr bool is_system_little_endian() {
        union {int16_t value; uint8_t bytes[sizeof(value)];} x{};
        x.value = 1;
        return x.bytes[0] == 1;
    };

    static T serialize(T value) {
        static union {T value; uint8_t bytes[sizeof(T)];} t{};
        t.value = value;
        if ((little_endian ^ is_system_little_endian()) == 0) {
            return t.value;
        } else {
            std::reverse(t.bytes, t.bytes + sizeof(t.value));
            return t.value;
        }
    }

    static void dynamic_metadata_skip(std::ifstream& ifp) {
        std::string line;
        std::streampos data_position;

        while (std::getline(ifp, line)) {
            line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));

            if (!line.empty() && std::isdigit(line[0])) {
                data_position = ifp.tellg();
                data_position -= line.size()+1;
                break;
            }
        }

        ifp.clear();
        ifp.seekg(data_position);
    }

    static std::string generate_output_file_path(const std::string& input_path, const std::string& extension) {
        std::string output_path;
        size_t ext_pos = input_path.find_last_of('.');
        if (ext_pos != std::string::npos) output_path = input_path.substr(0, ext_pos + 1) + std::string(extension);
        return output_path;
    }

public:
    static void create_dem_asc_bin(const std::string& path) {
        std::vector<T> dem_data;
        T value = 0;
        std::ifstream ifp(path);
        if (!ifp.good()) {
            std::string e = "file '" + path + "' not found";
            throw std::runtime_error(e);
        }

        dynamic_metadata_skip(ifp);

        // read values to `dem_data`
        while (ifp >> value) dem_data.push_back(serialize(value));
        ifp.close();

        // write `dem_data` in binary format
        std::ofstream ofp(generate_output_file_path(path, "bin"), std::ios::binary | std::ios::trunc);
        ofp.write(reinterpret_cast<char*>(dem_data.data()), dem_data.size() * sizeof(T));
        ofp.close();
    };


    static void create_dem_asc_csv(const std::string& path, const typename DEM<T, little_endian>::Type type) {
        std::vector<T> dem_data;
        T value = 0;
        std::ifstream ifp(path);
        if (!ifp.good()) {
            std::string e = "file '" + path + "' not found";
            throw std::runtime_error(e);
        }

        dynamic_metadata_skip(ifp);

        // read values to `dem_data`
        while (ifp >> value) dem_data.push_back(value);
        ifp.close();

        // write as per '.csv' format
        std::ofstream ofp(generate_output_file_path(path, "csv"));
        for (size_t i = 0; i < type.nrows * type.ncols; ++i) {
            if (i % type.ncols == type.ncols - 1)
                ofp << dem_data[i] << "\n";
            else
                ofp << dem_data[i] << ",";
        }
        ofp.close();
    };


    static void create_dem_csv_bin(const std::string& path){
        std::vector<T> dem_data;
        T value = 0;
        std::ifstream ifp(path);
        if (!ifp.good()) {
            std::string e = "file '" + path + "' not found";
            throw std::runtime_error(e);
        }

        // read values to `dem_data`
        std::string line;
        while (std::getline(ifp, line)) {
            std::istringstream iss(line);
            std::string token;
            while (std::getline(iss, token, ','))
                dem_data.push_back(serialize(static_cast<T>(std::stod(token))));
        }
        ifp.close();

        // write `dem_data` in binary format
        std::ofstream ofp(generate_output_file_path(path, "bin"), std::ios::binary | std::ios::trunc);
        ofp.write(reinterpret_cast<char*>(dem_data.data()), dem_data.size() * sizeof(T));
        ofp.close();
    };


    static void create_dem_bin_csv(const std::string& path, const typename DEM<T, little_endian>::Type type) {
        std::vector<T> dem_data;
        T value = 0;
        std::ifstream ifp(path);
        if (!ifp.good()) {
            std::string e = "file '" + path + "' not found";
            throw std::runtime_error(e);
        }

        // read from .bin file
        while (ifp.read(reinterpret_cast<char*>(&value), sizeof(T))) dem_data.push_back(serialize(value));
        ifp.close();

        // write as per '.csv' format
        std::ofstream ofp(generate_output_file_path(path, "csv"));
        for (size_t i = 0; i < type.nrows * type.ncols; ++i) {
            if (i % type.ncols == type.ncols - 1)
                ofp << dem_data[i] << "\n";
            else
                ofp << dem_data[i] << ",";
        }
        ofp.close();
    };
};
