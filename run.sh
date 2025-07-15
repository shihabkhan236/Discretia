#!/bin/bash

# Compile the program
gcc main.c -lraylib -lm

# If compilation succeeds, run the program
if [ $? -eq 0 ]; then
    ./a.out
else
    echo "Compilation failed."
fi
