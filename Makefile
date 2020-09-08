CFLAGS = -g
CXXFLAGS += $(CFLAGS)

BINS = synomdmapper
SRCS = $(shell echo *.cpp)
OBJS = $(SRCS:.cpp=.o)

.PHONY: all clean
all: $(BINS)

$(BINS): $(OBJS)
		$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
		$(CXX) -c $< -o $@ $(CXXFLAGS) $(LD_FLAGS)

clean:
		/bin/rm -f *.o *~ $(BINS)
