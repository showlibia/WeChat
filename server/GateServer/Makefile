.PHONY: run

build:
	cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

run: build
	cmake --build build -j8 && (cd build && ./GateServer)

clean:
	rm -rf build