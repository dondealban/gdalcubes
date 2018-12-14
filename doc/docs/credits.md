
# Credits

gdalcubes wouldn't exist without all the great effort of other open-source projects.
This document presents a list of used third-party software including their purpose, copyright, and licensing information 
in no particular order. Please notice that some libaries are only used by the command line client or gdalcubes_server, but not 
by the core library.

- **[GDAL](https://www.gdal.org/):  A translator library for raster and vector geospatial data formats**
    - Copyright (c) 2000, Frank Warmerdam
    - Copyright (c) The GDAL/OGR project team
    - License:  [MIT license](https://opensource.org/licenses/MIT)
    - Parts of GDAL are licensed under
    different terms, see [https://github.com/OSGeo/gdal/blob/master/gdal/LICENSE.TXT](https://github.com/OSGeo/gdal/blob/master/gdal/LICENSE.TXT)
    - gdalcubes may statically or dynamically link to the gdal library depending on compilation flags

- **[json](https://github.com/nlohmann/json): JSON for Modern C++**
     - Copyright (c) 2013-2018 Niels Lohmann
     - License: [MIT license](https://opensource.org/licenses/MIT) 
     - gdalcubes distributes an unmodified version of the library under `src/external/json.hpp`
     
- **[SQLite](https://www.sqlite.org/): A self-contained, high-reliability, embedded, full-featured, public-domain, SQL database engine**
     - Copyright:  _public domain_
     - License: _public domain_
     - gdalcubes may statically or dynamically link to the sqlite library depending on compilation flags
    
- **[CURL](https://curl.haxx.se/): Command line tool and library for transferring data with URLs**
     - Copyright (c) 1996 - 2018, Daniel Stenberg, <daniel@haxx.se>
     - License: [curl license](https://curl.haxx.se/docs/copyright.html) 
     - gdalcubes may statically or dynamically link to the libcurl library depending on compilation flags
      
- **[ExprTk](http://www.partow.net/programming/exprtk/): A C++ Mathematical Expression Parsing and Evaluation Library**
    - Copyright (c) Arash Partow (1999-2018) 
    - License:  [MIT license](https://opensource.org/licenses/MIT) 
    - gdalcubes distributes an unmodified version of the library under `src/external/exprtk.hpp`

- **[netCDF](https://www.unidata.ucar.edu/software/netcdf): The Unidata network Common Data Form C library**
    - Copyright (c) 1993-2017 University Corporation for Atmospheric Research/Unidata
    - License: MIT-like, see [https://www.unidata.ucar.edu/software/netcdf/copyright.html](https://www.unidata.ucar.edu/software/netcdf/copyright.html)
    - DOI: http://doi.org/10.5065/D6H70CW6
    - gdalcubes may statically or dynamically link to the NetCDF C library depending on compilation flags


- **[Catch](https://www.gdal.org/): A modern, C++-native, header-only, test framework for unit-tests, TDD and BDD**
    - Copyright (c) 2010 Two Blue Cubes Ltd
    - License:  [Boost Software License 1.0](https://www.boost.org/LICENSE_1_0.txt)
    - gdalcubes distributes an unmodified version of the library under `src/external/catch.hpp`
    
- **[Boost.Date_time](https://www.boost.org/doc/libs/1_68_0/doc/html/date_time.html): A set of date-time libraries based on generic programming concepts**
    - Copyright (c)2001-2005 CrystalClear Software, Inc
    - License:  [Boost Software License 1.0](https://www.boost.org/LICENSE_1_0.txt)          
    - gdalcubes may statically or dynamically link to the Boost.Date_time library depending on compilation flags       
       
- **[Boost.Filesystem](https://www.boost.org/doc/libs/1_68_0/libs/filesystem/doc/index.htm)**
    - Copyright (c) Beman Dawes, 2011
    - License:  [Boost Software License 1.0](https://www.boost.org/LICENSE_1_0.txt)
    - gdalcubes may statically or dynamically link to the Boost.Filesystem library depending on compilation flags         
 
- **[Boost.Program_options](https://www.boost.org/doc/libs/1_68_0/doc/html/program_options.html)**
    - Copyright (c) 2002-2004 Vladimir Prus
    - License:  [Boost Software License 1.0](https://www.boost.org/LICENSE_1_0.txt)       
    - gdalcubes may statically or dynamically link to the Boost.Program_options library depending on compilation flags         
       
- **[Boost.Process](https://www.boost.org/doc/libs/1_68_0/doc/html/process.html): A library to manage system processes**
    - Copyright (c) 2006-2012 Julio M. Merino Vidal, Ilya Sokolov, Felipe Tanus, Jeff Flinn, Boris Schaeling
    - Copyright (c) 2016 Klemens D. Morgenstern
    - License:  [Boost Software License 1.0](https://www.boost.org/LICENSE_1_0.txt)       
    - gdalcubes includes header files in stream.h / stream.cpp         
               
- **[Boost.Asio](https://www.boost.org/doc/libs/1_68_0/doc/html/boost_asio.html): A cross-platform C++ library for network and low-level I/O programming**
    - Copyright (c) 2003-2018 Christopher M. Kohlhoff
    - License:  [Boost Software License 1.0](https://www.boost.org/LICENSE_1_0.txt)       
    - gdalcubes includes header files in stream.h / stream.cpp / server.h / server.cpp        
                                      
- **[Date](https://github.com/HowardHinnant/date): A date and time library based on the C++11/14/17 <chrono> header**   
    - Copyright (c) 2015, 2016, 2017 Howard Hinnant
    - Copyright (c) 2016 Adrian Colomitchi
    - Copyright (c) 2017 Florian Dang
    - Copyright (c) 2017 Paul Thompson
    - Copyright (c) 2018 Tomasz Kamiński    
    - License: [MIT license](https://opensource.org/licenses/MIT)       
    - gdalcubes distributes an unmodified version of the library under `src/external/date.h`
 
- **[cpprestsdk](https://github.com/Microsoft/cpprestsdk)**
    - Copyright (c) Microsoft Corporation
    - License:  [MIT license](https://opensource.org/licenses/MIT)      
    - gdalcubes_server includes may statically or dynamically link to the cpprestsdk library depending on compilation flags 


Derived work such as R or Python packages may use further external software (see their repositories or root directories).  