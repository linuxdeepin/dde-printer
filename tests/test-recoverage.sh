#!/bin/bash

BUILD_DIR=build
REPORT_DIR=report
PROJECT_REALNAME=dde-printer

rm -rf $BUILD_DIR
mkdir $BUILD_DIR
rm -f asan*

cd $BUILD_DIR
qmake CONFIG+=debug ../../src/cppcups
make

qmake CONFIG+=debug ../
make -j 8

lcov -c -i -d ./ -o init.info
../tests --gtest_output=xml:dde_test_report_${PROJECT_REALNAME}.xml
lcov -c -d ./ -o coverage.info
lcov -a init.info -a coverage.info -o total.info
lcov -r total.info "*/tests/*" "*/usr/include*"  "*.h" "*build/*" "*/dbus/*" -o final.info

mv asan_printer.log* asan_${PROJECT_REALNAME}.log

cd ..
rm -rf $REPORT_DIR
mkdir -p $REPORT_DIR
genhtml -o $REPORT_DIR ./build/final.info

