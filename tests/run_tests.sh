#!/bin/bash
echo "Running tests for the dealii-adapter"
RED='\033[0;31m'
GREEN='\033[0;32m'
NOCOLOR='\033[0m'

# Run from this directory
cd ${0%/*} || exit 1

exit_code=0

test_dir=$(pwd)

print_start(){
	echo -e " ***Test $@:"
	echo -ne "\t Starting ..."
}

print_result() {
	if [ $? -eq 0 ]
	then
      echo -ne "${GREEN} passed ${NOCOLOR}\n"
	else
      echo -ne "${RED} failed ${NOCOLOR}\n"
                exit_code=$[$exit_code +1]
                $cat $@.log
	fi
	mv $@.log $test_dir
}

test_name="linear-elasticity-building"
print_start ${test_name}
cd ../linear_elasticity
time (cmake . && make debug && make all) &>${test_name}.log
print_result ${test_name}

test_name="nonlinear-elasticity-building"
print_start ${test_name}
cd ../nonlinear_elasticity
(cmake . && make debug && make all) &>${test_name}.log
print_result ${test_name}

test_name="linear-elasticity-writing"
print_start ${test_name}
cd ../tests/write_tester
cp ../../linear_elasticity/linear_elasticity .
(cmake . && make) &> tester-build.log
./linear_elasticity &>linear-writer.log & ./write_tester &>tester-linear.log
sed -i '2d' tester-linear.log #Make log independent of preCICE version
sed -i '2d' tester-linear.log
numdiff  tester-linear.log ./reference/${test_name}.output &>${test_name}.log
print_result ${test_name}

test_name="solver-physics-linear"
print_start ${test_name}
sed -i '2d' solution-1.vtk
numdiff  solution-1.vtk ./reference/${test_name}.output &>${test_name}.log
print_result ${test_name}

test_name="nonlinear-elasticity-writing"
print_start ${test_name}
cp ../../nonlinear_elasticity/nonlinear_elasticity .
(cmake . && make) &> tester-build.log
./nonlinear_elasticity &>nonlinear-writer.log & ./write_tester &>tester-nonlinear.log
sed -i '2d' tester-nonlinear.log
sed -i '2d' tester-nonlinear.log
numdiff  tester-nonlinear.log ./reference/${test_name}.output &>${test_name}.log
print_result ${test_name}

test_name="solver-physics-nonlinear"
print_start ${test_name}
sed -i '2d' solution-1.vtk
numdiff  solution-1.vtk ./reference/${test_name}.output &>${test_name}.log
print_result ${test_name}

if [ $exit_code -eq 0 ]
then
    echo "All tests passed."
else
    echo "Errors occurred: $exit_code tests failed."
    exit 1
fi
