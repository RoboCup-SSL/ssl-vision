buildDir=build
#buildDir=build_debug

#change to Debug for debug mode
buildType=Release
#buildType=Debug

all: build_cmake

cmake: CMakeLists.txt
	cd $(buildDir) && cmake -DCMAKE_BUILD_TYPE=$(buildType) ..

build_cmake: cmake
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
