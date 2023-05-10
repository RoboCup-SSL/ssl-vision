.PHONY: all clean build_cmake cleanup_cache run run_client run_graphical_client install_test_data configure_spinnaker
buildDir=build

#change to Debug for debug mode
buildType=Release

all: build_cmake

$(buildDir):
	mkdir -p $(buildDir)

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
	LC_NUMERIC=en_US.UTF-8 ./bin/vision -s

run_client: all
	./bin/client

run_graphical_client: all
	./bin/graphicalClient

$(buildDir)/test-data.zip:
	wget -O $(buildDir)/test-data.zip https://cloud.robocup.org/s/qjKQEiKnGnLAkn9/download

install_test_data: $(buildDir)/test-data.zip
	unzip $(buildDir)/test-data.zip
	mkdir -p ./test-data
	mv ssl-vision-test-data/* ./test-data
	rmdir ssl-vision-test-data

$(buildDir)/spinnaker-3.0.0.118-amd64-pkg.tar.gz: $(buildDir)
	wget -O $(buildDir)/spinnaker-3.0.0.118-amd64-pkg.tar.gz https://cloud.robocup.org/s/DoQqtmnrq8pNYCc/download/spinnaker-3.0.0.118-amd64-pkg.tar.gz

install_spinnaker_sdk: $(buildDir)/spinnaker-3.0.0.118-amd64-pkg.tar.gz
	tar xvf $(buildDir)/spinnaker-3.0.0.118-amd64-pkg.tar.gz -C $(buildDir)
	echo libgentl	libspinnaker/accepted-flir-eula	boolean	true | sudo debconf-set-selections
	cd $(buildDir)/spinnaker-3.0.0.118-amd64 && \
		sudo dpkg -i libgentl_*.deb && \
    sudo dpkg -i libspinnaker_*.deb && \
    sudo dpkg -i libspinnaker-dev_*.deb
