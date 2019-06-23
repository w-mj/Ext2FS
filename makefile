OBJ_PATH=obj
BIN_PATH=bin
LIB_PATH=bin

# 不取测试文件
SRC_FILE= $(shell find . -name '*.cpp' | grep -v -E '^.+test_.+\.cpp')

SRC_NAME = $(notdir $(SRC_FILE))
OBJ_FILE = $(addprefix $(OBJ_PATH)/, $(patsubst %.cpp,%.o,$(SRC_NAME)))

INC_PATH = include .
INC_FLAG = $(addprefix -I, $(INC_PATH))

DEFINES = DEBUG
DEF_FLAG = $(addprefix -D, $(DEFINES))

CC_FLAG = $(INC_FLAG) $(DEF_FLAG)

# MAIN_NAME = config/test_config
# OBJ_FILE= $(addsuffix .o, $(addprefix $(OBJ_PATH)/, $(SRC_NAME)))
# MAIN_FILE= $(addsuffix .c, $(addprefix $(SRC_PATH)/, $(MAIN_NAME)))
# LIB_FILE= $(addsuffix .a, $(addprefix $(OBJ_PATH)/, $(SRC_NAME)))
# BIN_FILE = $(addprefix $(BIN_PATH)/, $(MAIN_NAME))


CLEAN_OBJ =$(OBJ_FILE) .depend bin/* #$(BIN_FILE) $(LIB_FILE)
build: .depend $(OBJ_FILE)
# 	echo $(INC_FLAG)

$(BIN_FILE): $(OBJ_FILE)# $(MAIN_FILE)
	g++ $(CC_FLAG) $^ -g -o $@ 

.depend: $(SRC_FILE)
	g++ -MM $(CC_FLAG) $^ > $@
	sed -i '/.o:/ s,^,obj/,' $@

include .depend

$(OBJ_PATH)/%.o:
	g++ -c $<  $(CC_FLAG) -g -o $@

# $(BIN_PATH)/%.out:
# 	TEST_NAME = $(notdir $@)
# 	TEST_FILE = $(patsubst %.out, %.cpp, $(TEST_NAME))
# 	TEST_FILE = $(shell find . -name $(TEST_FILE).cpp)



rebuild: clean build

clean:
	-rm $(CLEAN_OBJ)


.PHONY: build rebuild clean
