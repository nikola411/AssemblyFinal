SUCCESS = "$(COLOR_BLUE)Success!$(COLOR_END)"

#LINKER_INCLUDE_DIR = ./inc/linker
#LINKER_INCLUDES = -I${LINKER_INCLUDE_DIR} -I${UTIL}
#LINKER_SOURCE =
#LINKER_SOURCE += $(wildcard ./src/linker/*.cpp)
#LINKER_SOURCE += ${UTIL_SOURCE}
#LINKER_OUTPUT = -o ./linker
#
#
#EMULATOR_INC_DIR = ./inc/emulator
#EMULATOR_INCLUDES = -I${EMULATOR_INC_DIR} -I${UTIL}
#EMULATOR_SOURCE = 
#EMULATOR_SOURCE += $(wildcard ./src/emulator/*.cpp)
#EMULATOR_SOURCE += ${UTIL_SOURCE}
#EMULATOR_OUTPUT = -o ./emulator

DEBUG_ENABLED = 0

all: flex bison asm
	@echo "$(COLOR_GREEN)Build successful!$(COLOR_END)"

#emu:
#	@echo "Building emulator..."
#	@g++ -g ${EMULATOR_INCLUDES} ${GPP_FLAGS} ${EMULATOR_SOURCE} ${EMULATOR_OUTPUT}
#	@echo "$(COLOR_BLUE)Success!$(COLOR_END)"

#lnk:
#	@echo "Building linker..."
#	@g++ -g ${LINKER_INCLUDES} ${GPP_FLAGS} ${LINKER_SOURCE} ${LINKER_OUTPUT}
#	@echo "$(COLOR_BLUE)Success!$(COLOR_END)"

#asm:
#	@echo "Building assembly..."
#	@g++ -g ${ASSEMBLY_INCLUDES} ${GPP_FLAGS} ${ASSEMBLY_SOURCE} ${ASSEMBLY_OUTPUT}
#	@echo ${SUCCESS}

all: flex bison asm

ASM_DIR = ./inc/assembly/

asm: flex bison
	@echo "Building assembly..."
	@$(MAKE) -s asm -C $(ASM_DIR)
	@echo ${SUCCESS}

FLEX_DIR = ./misc/
BISON_DIR = ./misc/

flex: 
	@echo "Building flex..."
	@$(MAKE) -s flex -C $(FLEX_DIR)
	@echo ${SUCCESS}

bison:
	@echo "Building bison..."
	@$(MAKE) -s bison -C $(BISON_DIR)
	@echo $(SUCCESS)

clean:
	@echo "Cleaning..."
	@$(MAKE) -s clean -C $(BISON_DIR)
	@rm -rf asembler
	@echo $(SUCCESS)

unit-tests:
	@echo "Builing unit-tests..."
	@sh ./tests.sh make
	@echo $(SUCCESS)
	
## coloring util
COLOR_GREEN=\033[0;32m
COLOR_RED=\033[0;31m
COLOR_BLUE=\033[0;34m
COLOR_END=\033[0m
