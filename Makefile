SRC_DIR := src
INCLUDE_DIR := include
SFML_INCLUDE := C:/SFML/include
SFML_LIB := C:/SFML/lib
BUILD_DIR := build
OUT_DIR := dist
EXE_NAME := Super Mario

DIST_ASSETS := Resources

CXX := g++
EXTRA_LDFLAGS :=

MAKEFLAGS += -j

CPP_FILES := $(wildcard $(SRC_DIR)/*.cpp)
DEV_DIR := $(BUILD_DIR)/dev
REL_DIR := $(BUILD_DIR)/release
DEV_OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(DEV_DIR)/%.o,$(CPP_FILES))
REL_OBJ := $(patsubst $(SRC_DIR)/%.cpp,$(REL_DIR)/%.o,$(CPP_FILES))
RES_OBJ := $(BUILD_DIR)/resource.o
DEV_EXE := $(EXE_NAME).exe
REL_EXE := $(OUT_DIR)/$(EXE_NAME).exe
ASSET_TARGETS := $(addprefix copy-asset-,$(DIST_ASSETS))

COMMON_FLAGS := -I$(SFML_INCLUDE) -I$(INCLUDE_DIR) -std=c++23 -fno-exceptions -ffast-math

DEV_FLAGS := $(COMMON_FLAGS) -Os -pipe -MMD -MP
DEV_LIBS := -lsfml-graphics -lsfml-window -lsfml-audio -lsfml-system

REL_FLAGS := $(COMMON_FLAGS) -O3 -DNDEBUG -DSFML_STATIC -ffunction-sections -fdata-sections -fmerge-all-constants
REL_LDFLAGS := -static -s -mwindows -Wl,--gc-sections
REL_LIBS := -lsfml-graphics-s -lsfml-window-s -lsfml-audio-s -lsfml-system-s \
            -lfreetype -lopengl32 -lgdi32 -lwinmm \
            -lflac -lvorbisenc -lvorbisfile -lvorbis -logg \
            -lpthread

mkdir_cmd = if not exist "$(subst /,\,$1)" mkdir "$(subst /,\,$1)"

.PHONY: all dev release clean FORCE $(ASSET_TARGETS)

all: dev
	@echo Running $(DEV_EXE)
	@"$(DEV_EXE)"
	@cls

dev: $(DEV_OBJ) $(RES_OBJ)
	@echo Linking $(DEV_EXE)
	@$(CXX) $(EXTRA_LDFLAGS) -L$(SFML_LIB) $(DEV_OBJ) $(RES_OBJ) -o "$(DEV_EXE)" $(DEV_LIBS)

$(DEV_DIR)/%.o: $(SRC_DIR)/%.cpp Makefile | $(DEV_DIR)
	@echo Compiling $<
	@$(CXX) $(DEV_FLAGS) -c $< -o $@

release: $(RES_OBJ) $(REL_OBJ) $(ASSET_TARGETS) | $(OUT_DIR)
	@echo Linking $(REL_EXE)
	@$(CXX) $(REL_LDFLAGS) $(EXTRA_LDFLAGS) -L$(SFML_LIB) $(REL_OBJ) $(RES_OBJ) -o "$(REL_EXE)" $(REL_LIBS)
	@echo Checking DLL imports - Windows system DLLs only is good:
	@objdump -p "$(REL_EXE)" | findstr /C:"DLL Name"
	@echo Done $(OUT_DIR)/ is ready to ship

$(REL_DIR)/%.o: $(SRC_DIR)/%.cpp FORCE | $(REL_DIR)
	@echo Compiling $<
	@$(CXX) $(REL_FLAGS) -c $< -o $@

FORCE:

$(ASSET_TARGETS): copy-asset-%: | $(OUT_DIR)
	@echo Copying $*
	@if exist "$(subst /,\,$*)\" (if exist "$(OUT_DIR)\$(subst /,\,$*)" rmdir /S /Q "$(OUT_DIR)\$(subst /,\,$*)")
	@if exist "$(subst /,\,$*)\" (xcopy "$(subst /,\,$*)" "$(OUT_DIR)\$(subst /,\,$*)" /E /I /Y /Q >nul) else (copy /Y "$(subst /,\,$*)" "$(OUT_DIR)" >nul)

$(RES_OBJ): Resources/icon.ico | $(BUILD_DIR)
	@echo Compiling Resources/icon.ico
	@echo IDI_ICON1 ICON "Resources/icon.ico" > resource.rc
	@windres resource.rc -o $@
	@del /Q resource.rc

$(BUILD_DIR) $(OUT_DIR):
	@$(call mkdir_cmd,$@)

$(DEV_DIR) $(REL_DIR): | $(BUILD_DIR)
	@$(call mkdir_cmd,$@)

clean:
	@echo Cleaning $(BUILD_DIR)/
	@if exist "$(BUILD_DIR)" rmdir /S /Q "$(BUILD_DIR)"

-include $(DEV_OBJ:.o=.d)