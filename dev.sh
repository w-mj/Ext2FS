#!/bin/bash

test_module() {
    if [ $# -eq "0" ]; then
        echo "module name must be exist.";
        exit
    fi
    test_name='test_'$1'.cpp'
    path=`find . -name $test_name`
    if [ -z $path ]; then
        echo 'test_'$1'.cpp is not found.'
        exit
    fi
    echo 'found test_'$1'.cpp at '$path
    if [ ! -d 'bin' ]; then
        mkdir bin
    fi
    # echo "Compiling..."
    set -x
    rm 'bin/test_'$1'.out'
    make
    g++ $path obj/*.o -I. -Iinclude -o 'bin/test_'$1'.out'
    set +x

    echo -e "Run test case.\n=======\n"
    'bin/test_'$1'.out'
}


help() {
    echo "Useage: dev.sh <subcommand>"
    echo ""
    echo "Available subcommands:"
    echo "  test <module name>"
}

if [ $# -eq "0" ]; then
    help
    exit
fi
if [ $1 = "test" ]; then
    test_module $2
fi
if [ $1 = "make" ]; then
    make_make
fi