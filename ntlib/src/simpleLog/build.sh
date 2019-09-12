#!/bin/sh
g++ -I../../../include/server/simpleLog -D XX_TEST_LOG -Wall -g -c *.cpp
g++ -D XX_TEST_LOG -Wall -g -o test_log *.o -O2 

rm -rf *.o 
#addr2line -e simpleLog -f 0x80487bc
