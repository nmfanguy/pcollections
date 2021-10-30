PROGS = driver
OBJS = driver.o
CXXFLAGS = $(shell pkg-config --cflags libpmemobj++) -std=c++17 -O2
LDFLAGS = $(shell pkg-config --libs libpmemobj++) -O2
CXX = g++
RM = rm

%.o : %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o build/$@

all: $(PROGS)

driver: $(OBJS)
	$(CXX) build/$(OBJS) $(LDFLAGS) -o build/$@

clean:
	$(RM) build/*