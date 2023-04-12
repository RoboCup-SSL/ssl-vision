.PHONY: all clean build_cmake cleanup_cache run run_client run_graphical_client install_test_data configure_spinnaker
buildDir=build

#change to Debug for debug mode
buildType=Release

all: build_cmake

$(buildDir)/CMakeLists.txt.copy: CMakeLists.txt
	cmake -B $(buildDir) -DCMAKE_BUILD_TYPE=$(buildType)

build_cmake: $(buildDir)/CMakeLists.txt.copy
	$(MAKE) -C $(buildDir)

clean:
	$(MAKE) -C $(buildDir) clean

cleanup_cache:
	rm -rf $(buildDir)

configure_spinnaker: $(buildDir)/CMakeLists.txt.copy
	cmake -S . -B $(buildDir) -DUSE_SPINNAKER=true

backup_configs: $(wildcard robocup-*) settings.xml
		$(eval dir="backup/$(shell date +%Y-%m-%d_%H-%M-%S)")
		mkdir -p $(dir)
		cp $? $(dir)

run: all
	./bin/vision -s

run_client: all
	./bin/client

run_graphical_client: all
	./bin/graphicalClient

$(buildDir)/test-data.zip:
	wget -O $(buildDir)/test-data.zip https://cloud.robocup.org/s/qjKQEiKnGnLAkn9/download

install_test_data: $(buildDir)/test-data.zip
	unzip $(buildDir)/test-data.zip
	mv ssl-vision-test-data/* ./test-data
	rmdir ssl-vision-test-data
