#!/usr/bin/env bash

# This script download CGAL and build City3D executables with header only CGAL

BUILD_DIR="Release"
CGAL_VERSION="5.2.2"

# Download CGAL (to be used as header only)
wget "https://github.com/CGAL/cgal/releases/download/v$CGAL_VERSION/CGAL-$CGAL_VERSION.tar.xz" .
tar -xf CGAL-$CGAL_VERSION.tar.xz
rm CGAL-$CGAL_VERSION.tar.xz

# Recreate build dir
if [ -d "$BUILD_DIR" ]
then
    rm -rf $BUILD_DIR
fi

# Compile city3d shape executables
mkdir $BUILD_DIR
cd $BUILD_DIR
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(($(nproc)))
