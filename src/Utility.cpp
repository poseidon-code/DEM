#include <algorithm>
#include <cstdint>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include "DEM.hpp"
#include "Utility.hpp"



static constexpr bool is_system_little_endian() {
    union {int16_t value; uint8_t bytes[sizeof(value)];} x{};
    x.value = 1;
    return x.bytes[0] == 1;
};

static union {
    int16_t value;
    uint8_t bytes[sizeof(int16_t)];
} t{};

static int16_t serialize(int16_t value, bool le = true) {
    t.value = value;
    if (le && is_system_little_endian()) {
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

static std::string generate_output_file_path(const std::string& input_path, const std::string extension) {
    std::string output_path;
    size_t ext_pos = input_path.find_last_of('.');
    if (ext_pos != std::string::npos) output_path = input_path.substr(0, ext_pos + 1) + std::string(extension);
    return output_path;
}



void create_dem_asc_bin(const std::string& path, bool little_endian) {
    std::vector<short int> dem_data;
    short int value = 0;
    std::ifstream ifp(path);
    dynamic_metadata_skip(ifp);

    // read values to `dem_data`
    while (ifp >> value) dem_data.push_back(serialize(value, little_endian));
    ifp.close();

    // write `dem_data` in binary format
    std::ofstream ofp(generate_output_file_path(path, "bin"), std::ios::binary | std::ios::trunc);
    ofp.write(reinterpret_cast<char*>(dem_data.data()), dem_data.size() * sizeof(short int));
    ofp.close();
}


void create_dem_asc_csv(const std::string& path, DEM::Type type) {
    std::vector<short int> dem_data;
    short int value = 0;
    std::ifstream ifp(path);
    dynamic_metadata_skip(ifp);

    // read values to `dem_data`
    while (ifp >> value) dem_data.push_back(value);
    ifp.close();

    // write as per '.csv' format
    std::ofstream ofp(generate_output_file_path(path, "csv"));
    for (int i = 0; i < type.nrows * type.ncols; ++i) {
        if (i % type.ncols == type.ncols - 1)
            ofp << dem_data[i] << "\n";
        else
            ofp << dem_data[i] << ",";
    }
    ofp.close();
}


void create_dem_csv_bin(const std::string& path, bool little_endian) {
    std::vector<short int> dem_data;
    short int value = 0;

    // read values to `dem_data`
    std::ifstream ifp(path);
    std::string line;
    while (std::getline(ifp, line)) {
        std::istringstream iss(line);
        std::string token;
        while (std::getline(iss, token, ','))
            dem_data.push_back(static_cast<short int>(serialize(std::stoi(token), little_endian)));
    }
    ifp.close();

    // write `dem_data` in binary format
    std::ofstream ofp(generate_output_file_path(path, "bin"), std::ios::binary | std::ios::trunc);
    ofp.write(reinterpret_cast<char*>(dem_data.data()), dem_data.size() * sizeof(short int));
    ofp.close();
}


void create_dem_bin_csv(const std::string& path, DEM::Type type, bool little_endian) {
    std::vector<short int> dem_data;
    short int value = 0;

    // read from .bin file
    std::ifstream ifp(path);
    while (ifp.read(reinterpret_cast<char*>(&value), sizeof(short int))) dem_data.push_back(serialize(value, little_endian));
    ifp.close();

    // write as per '.csv' format
    std::ofstream ofp(generate_output_file_path(path, "csv"));
    for (int i = 0; i < type.nrows * type.ncols; ++i) {
        if (i % type.ncols == type.ncols - 1)
            ofp << dem_data[i] << "\n";
        else
            ofp << dem_data[i] << ",";
    }
    ofp.close();
}
