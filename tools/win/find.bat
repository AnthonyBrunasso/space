@echo off

findstr -s -n "%1" src/*.h
findstr -s -n "%1" src/*.cc
