#!/usr/bin/env bash

readonly FILES=(./includes/*.h)

for f in ${FILES[*]};
do
  file_name="$(basename ${f} .h)"
  test_name="test/src/${file_name}HeaderTest.cxx"
  echo "#include <${file_name}.h>
#include <cstdlib>
int main() {
  return EXIT_SUCCESS;
}" > ${test_name}
done
