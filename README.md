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
