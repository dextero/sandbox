all:
%:
	[ -f build/Makefile ] || (mkdir -p build ; cd build ; cmake ..)
	$(MAKE) -C build $@

profile:
	rm -rf build ; mkdir -p build ; cd build ; cmake -DPROFILE_RUN=ON ..
	$(MAKE) -C build

clean:
	rm -rf build/

