#!/usr/bin/env bash

run-clang-tidy.py -checks='modernize-use-using' -header-filter='.*' -fix -p=../bin -quiet
