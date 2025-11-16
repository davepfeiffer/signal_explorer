
INCLUDES = -isystem ../libaries/argparse/include/
INCLUDES+= -isystem ../libaries/stb
INCLUDES+= -isystem ../libaries/spdlog/include/
INCLUDES+= -isystem ../libaries/glfw/include/
INCLUDES+= -I/usr/include

#LOG_LVL=SPDLOG_LEVEL_OFF
LOG_LVL=SPDLOG_LEVEL_DEBUG

NVCC := nvcc
NVCCFLAGS := -Wno-deprecated-gpu-targets $(INCLUDES) -DSPDLOG_ACTIVE_LEVEL=$(LOG_LVL)

CXX := g++
CXXFLAGS := -g --std=c++23 $(INCLUDES) -DSPDLOG_ACTIVE_LEVEL=$(LOG_LVL)

LIBS := -lcurand -lcufft -lcudart
LIBS += -lglfw3 -L../libaries/glfw/src/

# Add the imgui library to build it from source
IMGUI_ROOT := ../libaries/imgui/
IMGUI_SRC =
IMGUI_SRC += $(IMGUI_ROOT)/imgui.cpp
IMGUI_SRC += $(IMGUI_ROOT)/imgui_demo.cpp
IMGUI_SRC += $(IMGUI_ROOT)/imgui_draw.cpp
IMGUI_SRC += $(IMGUI_ROOT)/imgui_tables.cpp
IMGUI_SRC += $(IMGUI_ROOT)/imgui_widgets.cpp
IMGUI_SRC += $(IMGUI_ROOT)/backends/imgui_impl_glfw.cpp
IMGUI_SRC += $(IMGUI_ROOT)/backends/imgui_impl_opengl3.cpp
INCLUDES += -isystem $(IMGUI_ROOT)

# Add the implot library to build it from source
IMPLOT_ROOT := ../libaries/implot/
IMPLOT_SRC =
IMPLOT_SRC += $(IMPLOT_ROOT)/implot.cpp
IMPLOT_SRC += $(IMPLOT_ROOT)/implot_demo.cpp
IMPLOT_SRC += $(IMPLOT_ROOT)/implot_items.cpp
INCLUDES += -isystem $(IMPLOT_ROOT)

SRC := src
BLD := build

# collect all source files
CUDA_SRC := $(wildcard $(SRC)/*.cu)
CPP_SRC := $(wildcard $(SRC)/*cpp)
SRCS := $(CUDA_SRC) $(CPP_SRC) $(IMGUI_SRC) $(IMPLOT_SRC)

# create all object file paths
CUDA_OBJ := $(patsubst $(SRC)/%.cu, $(BLD)/%.o, $(CUDA_SRC))
CPP_OBJ := $(patsubst $(SRC)/%.cpp, $(BLD)/%.o, $(CPP_SRC))
OBJS := $(CUDA_OBJ) $(CPP_OBJ)

# create all dependency lookup file paths
DEPS := $(patsubst $(SRC)/%, $(BLD)/%.d ,$(SRCS))
#$(info deps := $(DEPS))

EXE := assignment.exe

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

-include $(DEPS)

$(BLD):
	@mkdir -p $(BLD)

