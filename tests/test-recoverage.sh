#!/bin/bash

BUILD_DIR=build
REPORT_DIR=report
PROJECT_REALNAME=dde-printer

cd ../
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
rm -f asan*

cd $BUILD_DIR
qmake CONFIG+=debug  ../src/cppcups
make

cd ../tests/
rm -rf $BUILD_DIR
mkdir $BUILD_DIR
cd $BUILD_DIR

qmake CONFIG+=debug ../
make check

mv ../asan_printer.log* asan_${PROJECT_REALNAME}.log

lcov -c -d ./ -o coverage.info
lcov -r coverage.info "*/tests/*" "*/usr/include*"  "*.h" "*build/*" "*/dbus/*" -o final.info
rm -rf ../../tests/$REPORT_DIR
mkdir -p ../../tests/$REPORT_DIR
genhtml -o ../../tests/$REPORT_DIR final.info
