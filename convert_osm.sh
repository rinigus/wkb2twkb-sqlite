#!/bin/bash

# Script to convert all tables from a given database starting with osm_ into use of TWKB (column geometry)
# Adjust the used precision below (argument to wkb2twkb-sqlite)

set -e

D=$1

for table in `sqlite3 "$D" "SELECT name FROM sqlite_master WHERE type='table' AND name GLOB 'osm_*'"`; do
    echo "Converting to TWKB:" $table
    ./wkb2twkb-sqlite $D $table geometry 6
done

echo "VACUUM"
sqlite3 "$D" "PRAGMA temp_store=MEMORY; VACUUM;"
