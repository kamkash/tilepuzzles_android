export LD_LIBRARY_PATH=`pwd`/../../build:`pwd`/../lib:$LD_LIBRARY_PATH
export GLOG_log_dir=`pwd`/log 
export GLOG_alsologtostderr=1
export GLOG_colorlogtostderr=1
#code-insiders --disable-gpu
#code
code-insiders

