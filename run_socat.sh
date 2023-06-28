
# Start master process in the background
sudo socat PTY,link=/dev/virtualport_master,raw,echo=0 - &
master_pid=$!

# Wait for master process to start
sleep 1

# Print master PID
echo "Master PID: $master_pid"

# Start slave process in the background
sudo socat PTY,link=/dev/virtualport_slave,raw,echo=0 - &
slave_pid=$!

# Wait for slave process to start
sleep 1

# Print slave PID
echo "Slave PID: $slave_pid"


# Function to handle the signal and kill the last PID
cleanup() {
    echo "Killing PID: $master_pid"
    kill $master_pid
    echo "Killing PID: $slave_pid"
    kill $slave_pid
    exit
}

# Set up the signal handler to call the cleanup function when a specific signal is received
trap cleanup SIGINT

while true; do
    sleep 1
done