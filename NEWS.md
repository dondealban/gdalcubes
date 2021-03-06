
# 0.2.3

* fixed clang compiler warnings
* fixed MODIS collection formats
* new collection formats MxD14A2 and MxD13A2


# 0.2.2

* support for GDAL subdatasets in collection formats
* add `query_points` operation to query data cube values at irregular spatiotemporal points


# 0.2.1

* fixed compiler warnings
* fix mean aggregation


# 0.2.0

* reimplementation of `image_collection_cube::read_chunk()`, supporting image masks and custom gdalwarp arguments
* renamed `filter_predicate` -> `filter_pixel`
* added gdalcubes namespace
* removed reduce and stream commands from command line interface
* added `gdalcubes exec` command to command line interface
* added experimental `gdalcubes exec` and `gdalcubes translate_cog` for batch processing of image collections
* added `stream_reduce_time operator`, applying external processes on time series
* added `keep_bands` option to `apply_pixel` operation, keeping all bands from the input data cube


# 0.1.1

* removed exprtk library
* output NetCDF files now contain bounds variables
* readthedocs theme for documentation
* gdalwarp now receives correct extent (in full double precision)

# 0.1.0

* First Release