buildDir=build

#change to Debug for debug mode
buildType=Release

all: build_cmake

$(buildDir)/CMakeLists.txt.copy: CMakeLists.txt
	mkdir -p $(buildDir) && \
	cd $(buildDir) && \
	cmake -DCMAKE_BUILD_TYPE=$(buildType) .. && \
	cp ../CMakeLists.txt ./CMakeLists.txt.copy

build_cmake: $(buildDir)/CMakeLists.txt.copy
	$(MAKE) -C $(buildDir)

clean:
	$(MAKE) -C $(buildDir) clean

cleanup_cache:
	rm -rf $(buildDir)

run: all
	./bin/vision

runClient:
	./bin/client

runGraphicalClient:
	./bin/graphicalClient

$(buildDir)/test-data.zip:
	wget -O $(buildDir)/test-data.zip https://cloud.robocup.org/s/qjKQEiKnGnLAkn9/download

install_test_data: $(buildDir)/test-data.zip
	unzip $(buildDir)/test-data.zip
	mv ssl-vision-test-data/* ./test-data
	rmdir ssl-vision-test-data
