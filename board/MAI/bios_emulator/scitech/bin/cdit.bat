@echo off
cd %1
k_rm -f *.lib *.a
shift 1
%1 %2 %3 %4 %5 %6 %7 %8 %9
