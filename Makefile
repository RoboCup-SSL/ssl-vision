buildDir=build
#buildDir=build_debug

#change to Debug for debug mode
buildType=Release
#buildType=Debug

all: build_cmake

$(buildDir)/CMakeLists.txt.copy: CMakeLists.txt
	cd $(buildDir) && cmake -DCMAKE_BUILD_TYPE=$(buildType) .. && \
		cp ../CMakeLists.txt ./CMakeLists.txt.copy

build_cmake: $(buildDir)/CMakeLists.txt.copy
	$(MAKE) -C $(buildDir)

clean:
	$(MAKE) -C $(buildDir) clean
	
cleanup_cache:
	rm -rf $(buildDir) && mkdir $(buildDir)
	
run: all
	./bin/vision
	
runClient:
	./bin/client
	
runGraphicalClient:
	./bin/graphicalClient
