#pragma once

#include <string>

#include "DEM.hpp"


void create_dem_asc_bin(const std::string& path, bool little_endian = false);
void create_dem_asc_csv(const std::string& path, DEM::Type type);
void create_dem_csv_bin(const std::string& path, bool little_endian = false);
void create_dem_bin_csv(const std::string& path, DEM::Type type, bool little_endian = false);
