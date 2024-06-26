#!/bin/bash
parser_include=../../misc/parser/
lexer_include=../../misc/scanner/

base_include=../../inc/
instruction_include=../../inc/instruction/
assembly_include=../../inc/assembly/
parser_include=../../misc/parser/
lexer_include=../../misc/scanner/

base_source=../../src/
instruction_source=../../src/instruction/
assembly_source=../../src/assembly/

include=($base_include $instruction_include $assembly_include)
source=($base_source $instruction_source $assembly_source)

include_all="-I$base_include -I$instruction_include -I$assembly_include -I$parser_include -I$lexer_include"

g++ $include_all -c ../../misc/scanner/scanner.cpp -o ./obj/Scanner.o
g++ $include_all -c ../../misc/parser/parser.cpp -o ./obj/Parser.o

for i in ${!include[@]};
do
    files=`ls ${include[$i]}`
    for entry in $files
    do
        filename="$(basename ${entry} .hpp)"
        if [[ "$filename" != "makefile" && "$filename" != "assembly" && "$filename" != "instruction" && "$filename" != "Instruction.def" && "$filename" != "linker" ]];
        then 
            #echo "$filename"           
            g++ $include_all -g -c ${source[$i]}/$filename.cpp -o ./obj/$filename.o
        fi;
    done
done

dependency_list=""
for file in `ls ./obj/`;
do
    dependency_list+=" "
    dependency_list+="./obj/$file"
done

echo "$dependency_list"
ar rv ./Assembly.a $dependency_list