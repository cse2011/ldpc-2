#sudo kldload mqueuefs 
# sudo sysctl -w net.inet.udp.recvspace=1250000 
# sudo sysctl -w net.inet.udp.maxdgram=1250000 
# sudo sysctl -w kern.ipc.maxsockbuf=26214400 
sudo killall -9 server
