apt install cmake clang gcc g++ libasio-dev curl libcurl4-openssl-dev libssl-dev libboost-all-dev make aria2 git -y
aria2c -x 10 https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.14.3.tar.gz
tar -xvf cpp-httplib-0.14.3.tar.gz
rm -rf cpp-httplib-0.14.3.tar.gz
cd cpp-httplib-0.14.3
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --target install
cd ../..
git clone https://github.com/nlohmann/json.git
cd json
mkdir -p build
cd build
cmake ..
make
make install
cd ../..
