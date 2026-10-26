#Scripts to restore environment on Google Colab
#!/bin/bash
apt-get update
apt-get install -y llvm clang
mkdir MyPass
touch CMakeLists.txt
touch MyPass/CMakeLists.txt