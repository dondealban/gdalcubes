// Generated by using Rcpp::compileAttributes() -> do not edit by hand
// Generator token: 10BE3573-1514-4C36-9D1C-5A225CD40393

#include <Rcpp.h>

using namespace Rcpp;

// gdalcubes_version
void gdalcubes_version();
RcppExport SEXP _gdalcubes_gdalcubes_version() {
BEGIN_RCPP
    Rcpp::RNGScope rcpp_rngScope_gen;
    gdalcubes_version();
    return R_NilValue;
END_RCPP
}

static const R_CallMethodDef CallEntries[] = {
    {"_gdalcubes_gdalcubes_version", (DL_FUNC) &_gdalcubes_gdalcubes_version, 0},
    {NULL, NULL, 0}
};

RcppExport void R_init_gdalcubes(DllInfo *dll) {
    R_registerRoutines(dll, NULL, CallEntries, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
}