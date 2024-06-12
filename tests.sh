main_test_dir=./tests/unit-tests/

if [ "$@" = "init" ]
then
    cd $main_test_dir
    make utility.o
    make base.o
fi


if [ "$@" = "make" ]
then
    cd $main_test_dir
    tests=`ls ./*.cpp`

    make utility.o
    make base.o
    
    for entry in $tests
    do
        filename="${entry%.*}"
        make object=$filename "$filename.o"
    done

    make tests-run
fi

if [ "$@" = "run" ]
then
    ./tests/unit-tests/out/tests/run
fi