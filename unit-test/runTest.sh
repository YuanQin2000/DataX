#
# See the file LICENSE for redistribution information.
#
# Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
# All rights reserved.
#


# Run the test case and calculate the coverage

#!/bin/bash

function retrive_files()
{
    func=$1
    shift

    for file in $*
    do
        if [ -d $file ]; then
            pushd "$PWD" 1>/dev/null
            cd "$file"
            retrive_files $func $(ls)
            popd 1>/dev/null
        else
            eval $func $file
        fi
    done
}

function retrive_dirs()
{
    func=$1
    shift

    for file in $*
    do
        if [ -d $file ]; then
            pushd "$PWD" 1>/dev/null
            eval $func $file
            cd "$file"
            retrive_dirs $func $(ls)
            popd 1>/dev/null
        fi
    done
}

function setup_source_soft_link()
{
    file_extension=${1##*.}
    if [ "x$file_extension" = "xc" ] ||
       [ "x$file_extension" = "xcpp" ] ||
       [ "x$file_extension" = "xh" ]; then
        ln -s $(pwd)/$1 $COMPILE_OBJECT_PATH/$folder_name/$1
    fi
}

function do_coverage
{
    cd $1
    gcov *.gcno
    lcov --capture --test-name $RUNNER_NAME --directory . --output-file $COVERAGE_PATH/$1.info
    cd ..
}

function clean_folder()
{
    lcov --directory $1 --zerocounters
    rm -rf $1/*.h
    rm -rf $1/*.cpp
    rm -rf $1/*.c
    rm -rf $1/*.gcda
}

function file_size()
{
    stat -c %s $1 | tr -d '\n'
}

function remove_empty_file()
{
    if [ ! -s $1 ]; then
        rm -rf $1
    fi
}

function usage_exit()
{
    echo "Usage: $1 [all|ut|coverage|clean]"
    exit
}


# Start at here.
if [ $# -ne 1 ]; then
    usage_exit $0
fi

# Constant variable defines
WORK_PATH=$(pwd)
RUNNER_PATH=$WORK_PATH/build
RUNNER_NAME=testAll
TEST_CASE_PATH=$WORK_PATH/TestCases
COVERAGE_PATH=$WORK_PATH/coverage
COVERAGE_REPORTS=$WORK_PATH/reports

pushd "$PWD" 1>/dev/null
cd ../src
COMPILE_SRC_PATH=$(pwd)
popd 1>/dev/null

declare -a EXCLUDE_LIST=(CoreApp CookieApp CtrlApp)
declare -a INCLUDE_LIST

pushd "$PWD" 1>/dev/null
cd ../build/obj
COMPILE_OBJECT_PATH=$(pwd)
for folder_name in $(ls)
do
    bFound="false"
    for exclude_folder_name in ${EXCLUDE_LIST[@]}
    do
        if [ $folder_name = $exclude_folder_name ]; then
            bFound="true"
            break
        fi
    done
    if [ "x$bFound" = "xfalse" ]; then
        INCLUDE_LIST=("${INCLUDE_LIST[@]}" $folder_name)
    fi
done

mkdir -p $RUNNER_PATH
mkdir -p $COVERAGE_PATH
mkdir -p $COVERAGE_REPORTS

if [ "x$1" = "xall" -o "x$1" = "xclean" ]; then
    # Step1: Clean all history data which will impact current running.
    retrive_dirs clean_folder ${INCLUDE_LIST[@]}
    clean_folder $RUNNER_PATH
    lcov --directory $COVERAGE_PATH --zerocounters
    rm -rf $COVERAGE_PATH/*
    popd 1>/dev/null

    if [ "x$1" = "xclean" ]; then
        exit
    fi
fi

if [ "x$1" = "xut" -o "x$1" = "xall" ]; then
    # Step2: Running the unit test case
    pushd "$PWD" 1>/dev/null
    cd $RUNNER_PATH
    ./$RUNNER_NAME
    popd 1>/dev/null
    # if only do the UT or the UT running is failed, then exit
    if [ "x$1" = "xut" -o $? -ne 0 ]; then
        exit
    fi
elif [ "x$1" != "xcoverage" ]; then
    usage_exit
fi

# Step3: Running the coverage
pushd "$PWD" 1>/dev/null
cd $COMPILE_SRC_PATH
for folder_name in ${INCLUDE_LIST[@]}
do
    retrive_files setup_source_soft_link $folder_name
done
popd 1>/dev/null

# set soft link for unit-test
all_test_case=$(ls $TEST_CASE_PATH)
for test_case in $all_test_case
do
    file_extension=${test_case##*.}
    if [ "x$file_extension" = "xc" ] ||
       [ "x$file_extension" = "xcpp" ] ||
       [ "x$file_extension" = "xh" ]; then
        ln -s $TEST_CASE_PATH/$test_case $RUNNER_PATH/$test_case
    fi
done

pushd "$PWD" 1>/dev/null
cd $COMPILE_OBJECT_PATH
retrive_dirs do_coverage ${INCLUDE_LIST[@]}
popd 1>/dev/null

# coverage the unit-test case source hence we can collect the inline/template functions.
pushd "$PWD" 1>/dev/null
cd $RUNNER_PATH
gcov *.gcno
lcov --capture --test-name $RUNNER_NAME --directory . --output-file $COVERAGE_PATH/$RUNNER_NAME.info
popd 1>/dev/null

# Step4: Generate the HTML page
date_str=$(date)
report_name=${date_str// /}
report_name=${report_name//:/}
retrive_files remove_empty_file $COVERAGE_PATH
genhtml $COVERAGE_PATH/*.info \
        --title $RUNNER_NAME \
        --output-directory $COVERAGE_REPORTS/$report_name
if [ -f $COVERAGE_REPORTS/$report_name/index.html ]; then
    firefox $COVERAGE_REPORTS/$report_name/index.html &
fi
