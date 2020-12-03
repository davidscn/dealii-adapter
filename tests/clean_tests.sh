#!/bin/bash

# Run from this directory
cd ${0%/*} || exit 1

rm -fv *.log
rm -fv ./write_tester/*.log
rm -fv ./write_tester/*.json
rm -fvr ./write_tester/precice-run
rm -fv ./write_tester/linear_elasticity
rm -fv ./write_tester/nonlinear_elasticity
rm -fv ./write_tester/*.vtk
rm -fv ./write_tester/Makefile
rm -fv ./write_tester/CMakeCache.txt
rm -fv ./write_tester/*.cmake
rm -fvr ./write_tester/CMakeFiles
