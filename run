clean_() {
    echo
    echo "*** CLEANING ***"
    echo

    make clean
}

make_() {
    echo
    echo "*** MAKING ***"
    echo

    make
}

run_() {
    echo
    echo "*** RUNNING ***"
    echo

    cd build
    ./driver
    cd ..
}

clear

if [ -z "$1" ]
then
    run_
fi

if [ -n "$1" ] && [ $1 = "full" ] 
then
    clean_
    make_
    run_
fi

if [ -n "$1" ] && [ $1 = "make" ]
then
    make_
    run_
fi

if [ -n "$1" ] && [ $1 = "-h" ]
then
    echo "Pass no arguments to only run an existing executable"
    echo "Pass 'full' as the first argument to clean, make, and run"
    echo "Pass 'make' as the first argument to make and run"
fi
