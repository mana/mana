CXX = g++
# optimizations
CXXFLAGS = -Wall -O3 -fexpensive-optimizations -pipe -fomit-frame-pointer -funroll-loops -fexceptions
CXXFLAGS += `sdl-config --cflags` -fexceptions `pkg-config --cflags libxml-2.0`

LIBS := `sdl-config --libs` -lguichan_sdl -lguichan -lSDL_mixer -lSDL_image \
        `pkg-config --libs libxml-2.0` -lphysfs

include debianfile.list

OBJS := $(patsubst src%, obj%, $(patsubst %.cpp, %.o, $(MODULES)))

tmw: $(OBJS)
	@echo $(OBJS)
	$(CXX) -o ../../tmw $(OBJS) $(LIBS)
	
clean:
	- rm -fv ../../obj/*.o
	- rm -fv ../../obj/net/*.o
	- rm -fv ../../obj/gui/*.o
	- rm -fv ../../obj/sound/*.o
	- rm -fv ../../obj/graphic/*.o
	- rm -fv ../../obj/resources/*.o
	- rm -fv ../../tmw

obj/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
