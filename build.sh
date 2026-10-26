rm -rf /content/build
mkdir /content/build

cd /content/build
cmake .. -DLLVM_DIR=/usr/lib/llvm-18/lib/cmake/llvm

cmake --build .