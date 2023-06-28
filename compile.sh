# make && ./modbus_test

make -C master && make -C slave

# ./slave/slave &
# master_pid=$!
# ./master/master
# kill $master_pid