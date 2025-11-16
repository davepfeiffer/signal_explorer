
INCLUDES = -isystem libaries/argparse/include/
INCLUDES+= -isystem libaries/stb
INCLUDES+= -isystem libaries/spdlog/include/
INCLUDES+= -isystem libaries/glfw/include/
INCLUDES+= -I/usr/include

#LOG_LVL=SPDLOG_LEVEL_OFF
LOG_LVL=SPDLOG_LEVEL_DEBUG

LIBS := -lcurand -lcufft -lcudart
LIBS += -lglfw3 -Llibaries/glfw/src/ -lGL
ifeq ($(OS),Windows_NT)
	LIBS +=  -lopengl32
else
	LIBS +=  -lGL
endif

# Add the imgui library to build it from source
IMGUI_ROOT = libaries/imgui
IMGUI_SRC = $(IMGUI_ROOT)/imgui.cpp \
	$(IMGUI_ROOT)/imgui_demo.cpp \
	$(IMGUI_ROOT)/imgui_draw.cpp \
	$(IMGUI_ROOT)/imgui_tables.cpp \
	$(IMGUI_ROOT)/imgui_widgets.cpp
IMGUI_BE_ROOT = $(IMGUI_ROOT)/backends
IMGUI_BE_SRC = \
	$(IMGUI_BE_ROOT)/imgui_impl_glfw.cpp \
	$(IMGUI_BE_ROOT)/imgui_impl_opengl3.cpp

INCLUDES += -isystem $(IMGUI_ROOT)
INCLUDES += -isystem $(IMGUI_BE_ROOT)

# Add the implot library to build it from source
IMPLOT_ROOT = libaries/implot
IMPLOT_SRC = $(IMPLOT_ROOT)/implot.cpp \
	$(IMPLOT_ROOT)/implot_demo.cpp \
	$(IMPLOT_ROOT)/implot_items.cpp
INCLUDES += -isystem $(IMPLOT_ROOT)

SRC := src
BLD := build

# collect all source files
CUDA_SRC := $(wildcard $(SRC)/*.cu)
CPP_SRC := $(wildcard $(SRC)/*cpp)
SRCS := $(CUDA_SRC) $(CPP_SRC) 

NVCC := nvcc
NVCCFLAGS := -Wno-deprecated-gpu-targets $(INCLUDES) -DSPDLOG_ACTIVE_LEVEL=$(LOG_LVL)

CXX := g++
CXXFLAGS := -g --std=c++23 $(INCLUDES) -DSPDLOG_ACTIVE_LEVEL=$(LOG_LVL)

# create all object file paths
CUDA_OBJ := $(patsubst $(SRC)/%.cu, $(BLD)/%.o, $(CUDA_SRC))
CPP_OBJ := $(patsubst $(SRC)/%.cpp, $(BLD)/%.o, $(CPP_SRC))
IMGUI_OBJ := $(patsubst $(IMGUI_ROOT)/%.cpp, $(BLD)/%.o, $(IMGUI_SRC))
IMGUI_BE_OBJ := $(patsubst $(IMGUI_BE_ROOT)/%.cpp, $(BLD)/%.o, $(IMGUI_BE_SRC))
IMPLOT_OBJ := $(patsubst $(IMPLOT_ROOT)/%.cpp, $(BLD)/%.o, $(IMPLOT_SRC))
OBJS := $(CUDA_OBJ) $(CPP_OBJ) $(IMGUI_OBJ) $(IMGUI_BE_OBJ) $(IMPLOT_OBJ)
$(info $(OBJS))

# create all dependency lookup file paths
DEPS := $(patsubst $(SRC)/%, $(BLD)/%.d ,$(SRCS)) \
	$(patsubst $(IMGUI_ROOT)/%, $(BLD)/%.d ,$(IMGUI_SRC)) \
	$(patsubst $(IMGUI_BE_ROOT)/%, $(BLD)/%.d ,$(IMGUI_BE_SRC)) \
	$(patsubst $(IMPLOT_ROOT)/%, $(BLD)/%.d ,$(IMPLOT_SRC))
$(info deps := $(DEPS))

EXE := sig_exp

.PHONY: all clean

all: $(EXE)

clean:
	@rm -rf $(BLD)
	@rm $(EXE)

# link the final executable (object files and CUDA runtime)
$(EXE): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $@ $^ -L$(CUDA_PATH)/lib64 $(LIBS)

$(BLD)/%.o: $(SRC)/%.cu | $(BLD)
	$(NVCC) $(NVCCFLAGS) -MMD -MP -c $< -o $@

$(BLD)/%.o: $(SRC)/%.cpp | $(BLD)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BLD)/%.o: $(IMGUI_ROOT)/%.cpp | $(BLD) 
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BLD)/%.o: $(IMGUI_BE_ROOT)/%.cpp | $(BLD) 
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BLD)/%.o: $(IMPLOT_ROOT)/%.cpp | $(BLD)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

-include $(DEPS)

$(BLD):
	@mkdir -p $(BLD)

