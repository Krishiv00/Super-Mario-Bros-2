SRC_DIR := src
INCLUDE_DIR := include
SFML_INCLUDE := C:/SFML/include
SFML_LIB := C:/SFML/lib
BUILD_DIR := build
EXE_NAME := Super Mario

CXXFLAGS := -I$(SFML_INCLUDE) -I$(INCLUDE_DIR) -std=c++23 -O2 -ffunction-sections -fdata-sections -fstrict-aliasing -fno-plt -fno-exceptions -fvisibility=hidden -fvisibility-inlines-hidden -fmerge-all-constants -fno-semantic-interposition -ffast-math -MMD -MP

CPP_FILES := $(wildcard $(SRC_DIR)/*.cpp)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CPP_FILES))

LIBRARIES := -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
LINK_FLAGS := -Wl,--gc-sections -O2 -s --icf=all

all: precompile compile dynamic clean run

precompile: $(BUILD_DIR) $(BUILD_DIR)/resource.o

$(BUILD_DIR)/resource.o:
	@echo Running Scripts:
	@echo     -resource.rc
	@echo IDI_ICON1 ICON "E:/Projects/C++/My Games/Mario 2/Resources/icon.ico" > resource.rc
	@windres resource.rc -o $@
	@del /Q resource.rc
	@echo.

print_compile_title:
	@echo Compiling Source:

compile: print_compile_title $(OBJ_FILES)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo     -$<
	@g++ $(CXXFLAGS) -c $< -o $@

dynamic: precompile compile link_dynamic clean run

link_dynamic: LINK_FLAGS :=
link_dynamic: link

build_static: LIBRARIES += -lfreetype -lvorbis -lvorbisfile -lflac -logg -mwindows -lopengl32 -lgdi32 -lwinmm -static-libstdc++ -static-libgcc
build_static: LINK_FLAGS := -static
build_static: precompile compile link clean

static:
	@make build_static -B

link:
	@echo.
	@echo Linking Libraries:
	@for %%L in ($(LIBRARIES)) do echo     %%L
	@g++ $(LINK_FLAGS) -L$(SFML_LIB) $(OBJ_FILES) -o "$(EXE_NAME).exe" $(BUILD_DIR)/resource.o $(LIBRARIES)
	@echo.

clean:
	@strip "$(EXE_NAME).exe"

run:
	@echo Running: $(EXE_NAME).exe
	@$(EXE_NAME).exe
	@cls