buildDir=build

#change to Debug for debug mode
buildType=Release

all: build

cmake: CMakeLists.txt
	cd $(buildDir) && cmake -DCMAKE_BUILD_TYPE=$(buildType) ..

build: cmake
	$(MAKE) -C $(buildDir)

clean:
	$(MAKE) -C $(buildDir) clean
	
cleanup_cache:
	cd $(buildDir) && rm -rf *
	
run: all
	./bin/vision
	
runClient:
	./bin/client
	
runGraphicalClient:
	./bin/graphicalClient
