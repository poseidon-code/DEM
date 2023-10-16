# Digital Elevation Model

A simple library replacement for `gdal` for lightweight deployment & use cases, for getting ground height (above mean sea level)
from provided latitude & longitude using the DEM data of the particular area. 



## Setting up DEM data

1. DEM data of the particular area should be available in any supported standard format.
The latitude & longitude bounds of the DEM data should be known.\
_e.g. DEM data available from [Bhuvan NRSC](https://bhuvan-app3.nrsc.gov.in/data/download/index.php) - Cartosat -1 : CartoDEM Version-3 R1_

2. The downloaded DEM data should be converted to a readable text file using a viable GIS tool 
as per the guidelines of the DEM data provider or the metadata from the downloaded DEM itself.\
_e.g.: [QGIS](https://qgis.org/en/site/) can be used to convert the downloaded `*.tiff` DEM data to ASCII text file format i.e. produces a `*.asc` file_

3. The filename of the generated ASCII text file must include the bottom-left (South-West) extreme 
DEM data bounds coordinates as the file names `<latitude>_<longitude>.asc`.\
_e.g.: DEM data of a region bounded by coordinates - `NW(15,76)`, `SW(14,76)`, `NE(15,77)` & `SE(14,77)`, 
the bottom-left coordinates (`SW(14,76)`) should be used as file name - `14_76.asc`_

4. The text based DEM data should be converted to binary file by using the library. 
    ```cpp
    #inculde "DEM.hpp"

    int main() {
        DEM::create_dem_asc_bin("./14_76.asc");
        // creates the `*.bin` file at the same directory as that of the `*.asc` file.

        return 0;
    }
    ```


## Reading DEM data

1. The different parameters of the downloaded DEM data should be known :
    - Resolution - distance covered by 1 DEM value _(e.g. 30 metres)_
    - Width - number of rows and columns present in the DEM data _(e.g. 3600)_
    - Range - the distance covered by the DEM data (in degree) _(e.g. 1.0 degree)_

2. File path to the created DEM data binary file _(e.g. `"/home/user/DEM/14_76.bin"`)_

```cpp
#include "DEM.hpp"

int main() {
    DEM::Type type = DEM::Type(30, 3600, 1); // resolution, width, range
    DEM dem = DEM(type, "/home/user/DEM/14_76.bin"); // initialise DEM

    return 0;
}
```


## Operations

1. **Altitude** : returns the DEM height of the given coordinate as it is in DEM data
    ```cpp
    double Latitude = 14.6705686, Longitude = 76.5106390;

    short int altitude = dem.altitude(Latitude, Longitude);
    std::cout << "Height :" << altitude << std::endl;
    ```

2. **Interpolated Altitude** : returns the interpolated DEM height of the given coordinate
    ```cpp
    double Latitude = 14.6705686, Longitude = 76.5106390;

    double interpolated_altitude = dem.interpolated_altitude(Latitude, Longitude);
    std::cout << "Interpolated Height : " << interpolated_altitude << std::endl;
    ```

3. **Patch** : returns an 1D vector of the surrounding DEM data of a valid coordinate.

    ```cpp
    double Latitude = 14.6705686, Longitude = 76.5106390;
    unsigned int radius = 10;

    std::vector<short int> patch = dem.patch(Latitude, Longitude, radius);
    ```


## File Conversion Operations

These functions converts files to the following formats and saves them in the same directory as that of the input files :
- `.asc` _(text file representation of DEM generated from a GIS application)_
- `.bin` _(file containing DEM data represented in binary format)_
- `.csv` _(comma seperated text file representation of the DEM data for usage with spreadsheets)_


1. **`.asc` to `.bin`** : converts `.asc` _(text)_ file to `.bin` _(binary)_ file
    ```cpp
    DEM::create_dem_asc_bin("./14_76.asc");
    ```

2. **`.asc` to `.csv`** : converts `.asc` _(text)_ file to `.csv` _(comma seperated values)_ file
    ```cpp
    DEM::create_dem_asc_csv("./14_76.asc", type);
    ```

3. **`.bin` to `.csv`** : converts `.bin` _(binary)_ file to `.csv` _(comma seperated values)_ file
    ```cpp
    DEM::create_dem_bin_csv("./14_76.bin", type);
    ```

4. **`.csv` to `.bin`** : converts `.csv` _(comma seperated values)_ file to `.bin` _(binary)_ file
    ```cpp
    DEM::create_dem_csv_bin("./14_76.csv");
    ```


## Usage

```cpp
// main.cpp
#include "DEM.hpp"

int main() {
    DEM::Type type = DEM::Type(30, 3600, 1); // resolution, width, range
    DEM dem = DEM(type, "/home/user/DEM/14_76.bin"); // initialise DEM

    return 0;
}
```

1. Use `DEM.hpp` as a single header-only file to include in your projects.
    ```sh
    g++ ./main.cpp -I./path/to/DEM -o main
    ```

2. Use seperate header file `include/DEM.hpp` and source file `src/DEM.cpp`
    ```sh
    g++ ./main.cpp ./path/to/DEM/src/DEM.cpp -I./path/to/DEM/include -o main
    ```


## Building

Use `CMake` **_(required)_** to build both shared and static libraries of DEM.\
Open a terminal inside DEM project directory and paste the following commands.

```sh
mkdir build
cd build
cmake ..
cmake --build .
```

