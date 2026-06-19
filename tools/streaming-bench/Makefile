# Build the streaming benchmark. Requires libav/libswscale dev headers
# (Debian/Ubuntu: apt install libavcodec-dev libavformat-dev libavutil-dev libswscale-dev)
CXXFLAGS := -std=c++11 -O3 -march=native $(shell pkg-config --cflags libavcodec libavformat libavutil libswscale)
LDLIBS := $(shell pkg-config --libs libavcodec libavformat libavutil libswscale)

streaming_bench: streaming_bench.cpp
	$(CXX) $(CXXFLAGS) $< $(LDLIBS) -o $@

clean:
	rm -f streaming_bench

.PHONY: clean
