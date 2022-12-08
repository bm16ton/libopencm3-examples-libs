.PHONY: clean all

all: 
	cd libopencm3 && make
	cd openfat && make
	cd proj && make

clean:
	cd libopencm3 && make $@
	cd openfat && make $@
	cd proj && make $@
