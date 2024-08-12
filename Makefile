CXX=clang++
ASANFLAGS="-fsanitize=address -fsanitize=undefined"
TSANFLAGS=-fsanitize=thread

all: adbg

dirs:
	mkdir -p build/dbg
	mkdir -p build/rls

dbg:
	cmake -B./build/dbg -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS=""
	cmake --build ./build/dbg -- -j 12

adbg: dirs
	cmake -B./build/dbg -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS=$(ASANFLAGS)
	cmake --build ./build/dbg -- -j 12

tdbg: dirs
	cmake -B./build/dbg -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS=$(TSANFLAGS)
	cmake --build ./build/dbg -- -j 12

rls: dirs
	cmake -B./build/rls -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=""
	cmake --build ./build/rls -- -j 12
	
arls: dirs
	cmake -B./build/rls -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=$(ASANFLAGS)
	cmake --build ./build/rls -- -j 12

trls: dirs
	cmake -B./build/rls -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS=$(TSANFLAGS)
	cmake --build ./build/rls -- -j 12

testdbg:
	./build/dbg/gjserveruttest

testrls:
	./build/rls/gjserveruttest

# TODO: win
# C:\"Program Files"\"Microsoft Visual Studio"\2022\Community\Common7\Tools/LaunchDevCmd.bat
# mkdir .\build\dbg
# cmake -B./build/dbg -DCMAKE_BUILD_TYPE=Debug
# cmake --build ./build/dbg  --config Debug
# .\build\dbg\gjserveruttest
