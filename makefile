SUCCESS = "$(COLOR_BLUE)Success!$(COLOR_END)"

DEBUG_ENABLED = 0

all: flex bison asm
	@echo "$(COLOR_GREEN)Build successful!$(COLOR_END)"


ASM_DIR = ./inc/assembly/
EMU_DIR = ./inc/emulator/

asm: flex bison
	@echo "Building assembly..."
	@$(MAKE) -s asm -C $(ASM_DIR)
	@echo ${SUCCESS}

emu:
	@echo "Building emulator..."
	@$(MAKE) -s emu -C $(EMU_DIR)
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
