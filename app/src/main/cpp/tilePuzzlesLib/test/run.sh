#!/usr/bin/bash
export LD_LIBRARY_PATH=`pwd`/../../build:`pwd`/../lib:$LD_LIBRARY_PATH
export GLOG_log_dir=`pwd`/log 
# export GLOG_alsologtostderr=1
# export GLOG_colorlogtostderr=1
export GLOG_logbuflevel=-1
`pwd`/../../build/tile_puzzles $*