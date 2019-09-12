killall test_log
nohup ./test_log > /dev/null 2>&1 &
ps -ef|grep test_log|grep -v grep
