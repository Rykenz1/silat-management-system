#!/bin/bash

# Define output binary name
OUTPUT="app"
SOURCE="main.cpp"
IMPLEMENTATION="sys_function.cpp"

echo "========================================"
echo " Compiling $SOURCE ..."
echo "========================================"

# Execute g++ compiler command
g++ -I/usr/include/mysql-cppconn/jdbc "$SOURCE" "$IMPLEMENTATION" -o "$OUTPUT" -lmysqlcppconn

# Check if compilation succeeded
if [ $? -eq 0 ]; then
    echo " Build Successful!!"
    echo " Running $OUTPUT"
    echo "|"
    echo "|"
    ./$OUTPUT
else
    echo " Build Failed! Check errors above."
    exit 1
fi