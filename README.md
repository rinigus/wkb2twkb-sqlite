# wkb2twkb-sqlite
Convert WKB geometry BLOBs to TWKB

This a wrapper around PostGIS liblwgeom allowing you to convert WKB blobs in SQLite database into TWKB BLOBs. As a src/main.cpp for command line options.

External dependencies are sqlite and geos_c. 

The relevant code of liblwgeom has been copied into the repository. In addition, sqlite3pp has been used to communicate with SQLite. This C++ library is pulled as a submodule into this project.
