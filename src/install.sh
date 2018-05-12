#!/bin/bash

# NiuTrans Platform install.sh
# written by ZhangHao (zhanghao1216@gmail.com)
# version 1.0.0

# export runtime environment variable(s)
echo "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/../lib/" >> ~/.bashrc
#export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/../lib/

# make all projects
if [ $# -ge 1 ] && [ $1 = "-m32" ]
then
make all32
else
make
fi

# make some tools executable
chmod a+x ./uninstall.sh ../bin/*

# remind the user(s) to update runtime environment variable(s)
echo
echo 'PLEASE USE "source ~/.bashrc" TO UPDATE ENVIRONMENT VARIABLE(s)'
echo
