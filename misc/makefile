FLEX_SOURCE = ./scanner.l 
BISON_SOURCE = ./parser.yy
BISON_OUTPUT = ./parser/parser.cpp

flex:
	@flex $(FLEX_SOURCE)

bison:
	@bison  -o $(BISON_OUTPUT) $(BISON_SOURCE) -v

clean:
	rm -rf $(wildcard ./parser/*)
	rm -rf $(wildcard ./scanner/*)