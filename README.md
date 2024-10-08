# Digital Elevation Model

A simple library for accessing elevation values from provided latitude & longitude
using the DEM data of the particular area.

## Project Setup (CMake)

**Prerequisites**

1. CMake _(version 3.25 or above)_
2. C++ compiler with support for C++ 20 standard (or above)

```cmake
# CMakeLists.txt (at the root of your project)

cmake_minimum_required(VERSION 3.25)
project(Test LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20) # C++ standard 20 or above
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# including the downloaded DEM library (<project_root>/external/DEM)
add_subdirectory(external/DEM)

add_executable(${PROJECT_NAME} main.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC ${LIBDEM_INCLUDE_DIRECTORIES}) # include DEM headers
target_link_libraries(${PROJECT_NAME} PUBLIC DEM) # links DEM
```

## Setting up DEM data

1. DEM data of the particular area should be available in any supported standard format.
   The latitude & longitude bounds of the DEM data should be known.\
   _e.g. DEM data available from [Bhuvan NRSC](https://bhuvan-app3.nrsc.gov.in/data/download/index.php) - Cartosat -1 : CartoDEM Version-3 R1_

2. The downloaded DEM data should be converted to a readable text file using a viable GIS tool
   as per the guidelines of the DEM data provider or the metadata from the downloaded DEM itself.\
   _e.g.: [QGIS](https://qgis.org/en/site/) can be used to convert the downloaded `*.tiff` DEM data to ASCII text file format i.e. produces a `*.asc` file_

3. The text based DEM data should be converted to binary file by using the library.

    ```cpp
    #inculde "DEM/Utility.hpp"

    int main() {
        // creates the `*.bin` file at the same directory as that of the `*.asc` file.
        Utility<int16_t, std::endian::big>::create_dem_asc_bin("./14_76.asc");
        // Utility<datatype = int16_t, endiannnes = std::endian::big> : writes 2 byte signed integer
        // values with big-endian (endianness = std::endian::big) byte order
        // (default: endianness = std::endian::native)

        return 0;
    }
    ```

## DEM Operations

1. The different parameters of the downloaded DEM data should be known (also available after conversion to ASCII representation) :

    - Rows - _number of rows in DEM data_
    - Columns - _number of columns in DEM data_
    - Lower Left Corner Latitude - _bottom left latitude of the DEM data_
    - Lower Left Corner Longitude - _bottom left longitude of the DEM data_
    - Cell Size - _distance (in radians) between every DEM values_
    - No Data Value - _invalid DEM value representation_

2. File path to the created DEM data binary file _(e.g. `"/home/user/DEM/14_76.bin"`)_

    ```cpp
    #include "DEM/DEM.hpp"

    int main() {
        // DEM<datatype = int16_t, endiannnes = std::endian::big> : writes 2 byte signed integer
        // values with big-endian (endianness = std::endian::big) byte order
        // (default: endianness = std::endian::native)

        DEM<int16_t, std::endian::big>::Type type = DEM::Type(3600, 3600, 14, 76, 0.000277777, INT16_MIN); // nrows, ncols, yllcorner, xllcorner, cellsize, nodata

        // the byte order of the '*.bin' file should be known
        DEM<int16_t, std::endian::big> dem(type, "/home/user/DEM/14_76.bin"); // initialise

        return 0;
    }
    ```

### Operations

1. **Altitude** : returns the DEM height of the given coordinate as the type as in DEM data

    ```cpp
    float Latitude = 14.6705686, Longitude = 76.5106390;

    auto altitude = dem.altitude(Latitude, Longitude);
    std::cout << "Height :" << altitude << std::endl;
    ```

2. **Interpolated Altitude** : returns the interpolated DEM height of the given coordinate

    ```cpp
    float Latitude = 14.6705686, Longitude = 76.5106390;

    float interpolated_altitude = dem.interpolated_altitude(Latitude, Longitude);
    std::cout << "Interpolated Height : " << interpolated_altitude << std::endl;
    ```

## Utility Operations

These functions converts files to the following formats and saves them in the same directory as that of the input files :

-   `.asc` _(text file representation of DEM generated from a GIS application)_
-   `.bin` _(file containing DEM data represented in binary format)_
-   `.csv` _(comma seperated text file representation of the DEM data for usage with spreadsheets)_

1. **`.asc` to `.bin`** : converts `.asc` _(text)_ file to `.bin` _(binary)_ file

    ```cpp
    Utility<int16_t, std::endian::big>::create_dem_asc_bin("./14_76.asc");
    ```

2. **`.asc` to `.csv`** : converts `.asc` _(text)_ file to `.csv` _(comma seperated values)_ file

    ```cpp
    Utility<int16_t, std::endian::big>::create_dem_asc_csv("./14_76.asc", type);
    ```

3. **`.bin` to `.csv`** : converts `.bin` _(binary)_ file to `.csv` _(comma seperated values)_ file

    ```cpp
    Utility<int16_t, std::endian::big>::create_dem_bin_csv("./14_76.bin", type);
    ```

4. **`.csv` to `.bin`** : converts `.csv` _(comma seperated values)_ file to `.bin` _(binary)_ file

    ```cpp
    Utility<int16_t, std::endian::big>::create_dem_csv_bin("./14_76.csv");
    ```

## Usage

```cpp
// main.cpp
#include "DEM/DEM.hpp"

int main() {
    DEM<int16_t, std::endian::big>::Type type = DEM::Type(3600, 3600, 14, 76, 0.000277777, INT16_MIN); // nrows, ncols, yllcorner, xllcorner, cellsize, nodata
    DEM<int16_t, std::endian::big> dem = DEM(type, "/home/user/DEM/14_76.bin"); // initialise

    return 0;
}
```

## Map Operations

Creates a grid of DEM data which can be dynamically loaded into memory at runtime
when accessing a coordinate which is not bounded by the currently loaded DEM data.

1. Create a mapping of DEM files to coordinates.

    ```cpp
    #include "DEM/Map.hpp"

    int main() {
        // Map<datatype = int16_t, endiannnes = std::endian::big> : writes 2 byte signed integer
        // values with big-endian (endianness = std::endian::big) byte order
        // (default: endianness = std::endian::native)

        Map<int16_t, std::endian::big>::Grid grid;
        grid[{14, 76}] = {{3600, 3600, 14, 76, 0.000277777, INT16_MIN}, "/home/user/DEM/14_76.bin"};
        grid[{14, 77}] = {{3600, 3600, 14, 77, 0.000277777, INT16_MIN}, "/home/user/DEM/14_77.bin"};
        grid[{14, 78}] = {{3600, 3600, 14, 78, 0.000277777, INT16_MIN}, "/home/user/DEM/14_77.bin"};

        Map<int16_t, std::endian::big> map(grid); // initialise

        return 0;
    }
    ```

    **OR**, dynamically create grid from DEM files in a directory \
    _(**NOTE** : all the DEM files in the directory must conform to `<latitude>_<longitude>.bin` file name and must have same properties (`nrows`, `ncols`, `cellsize`, `nodata`))\_

    ```cpp
    #include "DEM/Map.hpp"

    int main() {
        Map<int16_t, std::endian::big>::Grid grid = Map<int16_t, true>::initialize("/home/user/DEM/", 3600, 3600, 0.00027777, INT16_MIN); // `/` (`\\` in Windows) required at end of the directory path

        Map<int16_t, std::endian::big> map(grid); // initialise

        return 0;
    }
    ```

### Operations

1. **Altitude** : returns the DEM height of the given coordinate as the type as in DEM data

    ```cpp
    float Latitude = 14.6705686, Longitude = 76.5106390;

    auto altitude = map.altitude(Latitude, Longitude);
    std::cout << "Height :" << altitude << std::endl;
    ```

2. **Interpolated Altitude** : returns the interpolated DEM height of the given coordinate

    ```cpp
    float Latitude = 14.6705686, Longitude = 76.5106390;

    float interpolated_altitude = map.interpolated_altitude(Latitude, Longitude);
    std::cout << "Interpolated Height : " << interpolated_altitude << std::endl;
    ``
    ```

# [MIT License](./LICENSE)

Copyright (c) 2023 Pritam Halder

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without
limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
