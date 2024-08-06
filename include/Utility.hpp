#pragma once

#include <string>

#include "DEM.hpp"


static void create_dem_asc_bin(const std::string& path);
static void create_dem_asc_csv(const std::string& path, DEM::Type type);
static void create_dem_csv_bin(const std::string& path);
static void create_dem_bin_csv(const std::string& path, DEM::Type type);
