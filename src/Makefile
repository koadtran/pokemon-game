CXX      = \g++
CXXFLAGS = -g -Wall -Werror
LDFLAGS  = -lncurses

OBJS = game.o world.o map.o distance.o characters.o data.o

game: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDFLAGS)

game.o:       game.cpp       map.h distance.h characters.h world.h data.h
world.o:      world.cpp      map.h distance.h characters.h world.h
characters.o: characters.cpp characters.h map.h distance.h data.h
distance.o:   distance.cpp   distance.h map.h data.h
map.o:        map.cpp        map.h characters.h world.h
data.o:       data.cpp

%.o:
	$(CXX) $(CXXFLAGS) -c $<

debug: game
	lldb ./game

clean:
	rm -f -r game $(OBJS) *~ core *.dSYM a.out

.PHONY: clean