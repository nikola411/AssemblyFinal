main_test_dir=./tests/unit-tests/

dependencies_init ()
{
    make src=Utility Utility.src
    make src=Base Base.src
    make src=AssemblyInstruction path='./instruction/' AssemblyInstruction
}

if [ "$@" = "make" ]
then
    cd $main_test_dir
    tests=`ls ./*.cpp`

    dependencies_init
    
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