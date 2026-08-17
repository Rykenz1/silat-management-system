#!/bin/bash

# Define output binary name
OUTPUT="app"
SOURCE="main.cpp"
IMPLEMENTATION="sys_function.cpp"
STUDENT="sys_student.cpp"
INSTRUCTOR="sys_instructor.cpp"
ADMIN="sys_admin.cpp"
PARENT="sys_parent.cpp"

# ANSI Color Codes
GREEN="\033[1;32m"
RED="\033[1;31m"
RESET="\033[0m"

echo "========================================"
echo " Compiling $SOURCE ..."
echo "========================================"

# Record start time
START_TIME=$(date +%s.%N)

# Execute g++ compiler command
g++ -I/usr/include/mysql-cppconn/jdbc "$SOURCE" "$IMPLEMENTATION" "$STUDENT" "$ADMIN" "$PARENT" "$INSTRUCTOR" -o "$OUTPUT" -lmysqlcppconn
COMPILE_STATUS=$?

# Record end time
END_TIME=$(date +%s.%N)

# Calculate duration
DURATION=$(echo "$END_TIME - $START_TIME" | bc -l | awk '{printf "%.2f", $0}')

# Check if compilation succeeded
if [ $COMPILE_STATUS -eq 0 ]; then
    echo -e " Build Successful in ${GREEN}${DURATION}s${RESET}!"
    echo " Running $OUTPUT"
    echo "|"
    echo "|"
    ./$OUTPUT
else
    echo -e " Build Failed in ${RED}${DURATION}s${RESET}! Check errors above."
    exit 1
fi