#!/usr/bin/env python

import json, sys, random, subprocess, threading, time, os.path
from pprint import pprint
from optparse import OptionParser
from shlex import split
import glob

#######################
#  CMD line Configuration
#######################
parser = OptionParser()
parser.add_option("-f", "--file", dest="filename",
                  help="Tournament config filename - MANDATORY", metavar="FILE")

parser.add_option("-s", "--setup", dest="setup",
                  help="Run Setup and push test files onto clients - may be slow - needed once",
				  metavar="SETUP", action="store_true")
(options, args) = parser.parse_args()

if options.filename is None:
	parser.print_help()
	exit(-1)

json_data = open(options.filename);
tcfg = json.load(json_data);
#pprint(data)
json_data.close()
num_teams = len(tcfg["teams"])
print "Starting tournament with " + str(num_teams) + " teams!"

def run_cmd(cmd):
	#str = split(cmd.encode())
	#return subprocess.Popen(str, stdout=subprocess.PIPE, stderr=subprocess.PIPE).communicate()[0]
	#print cmd
	str = cmd.encode()
	sub = subprocess.Popen(str, shell=True, stdout = subprocess.PIPE, stdin = subprocess.PIPE)
	out = sub.communicate()[0]
	#print out
	return out

class runServer(threading.Thread):
	servercmd = ""
	finishtime = 0

	def set_cmd(self, server):
		self.servercmd = server

	def run(self):
		#print self.servercmd
		#print "EXECUTING SERVER"
		run_cmd(self.servercmd)
		#print "SERVER DONE"
		self.finishtime = time.time()

	def get_time(self):
		return self.finishtime

class runTest(threading.Thread):
	servercmd = ""
	clientcmd = ""
	starttime = 0
	finishtime = 0
	def set_cmds(self, server, client):
		self.servercmd = server
		self.clientcmd = client

	def run(self):
		s = runServer()
		s.set_cmd(self.servercmd)
		s.start()
		time.sleep(1)
		#print self.clientcmd
		self.starttime = time.time()
		#print "EXECUTING CLIENT"
		run_cmd(self.clientcmd)
		#print "CLIENT DONE"
		#print "WAITING FOR SERVER TO EXIT"
		s.join()
		#print "SERVER WAIT DONE"
		self.finishtime = s.get_time()

	def get_time(self):
		return self.finishtime - self.starttime

#######################
# Configure client nodes
#######################
def setup_client(client_addr, setup):
	print "CLIENT SETUP"
	print "Setting up " + client_addr

	if setup:
		#Copy setup script onto client
		run_cmd("ssh " + client_addr + " rm -f ~/setup_client.sh")
		run_cmd("scp helpers/setup_client.sh " + client_addr + ":~/")
		run_cmd("ssh " + client_addr + " chmod +x ~/setup_client.sh")

		#Execute setup script on client
		run_cmd("ssh " + client_addr + " ~/setup_client.sh")

	#Copy client apps onto client
	for i in range(num_teams):
		cdir = "/mnt/client" + str(i)
		cmd = "ssh " + client_addr + " \"rm -rf " + cdir + " && mkdir -p " + cdir + "\"";
		#print cmd
		run_cmd(cmd)

		teamdata = tcfg["teams"][i]
		cmd = "scp -r " + teamdata["ClientDataDir"] + "/* " + client_addr + ":" + cdir
		#print cmd
		run_cmd(cmd)

	if setup:
		print "Copying data file onto: " + client_addr;
		cmd = "scp " + tcfg["TestFile"] + " " + client_addr + ":/mnt/"
		run_cmd(cmd)
	return

def setup_server(server_addr, setup):
	print "SERVER SETUP"
	print "Setting up " + server_addr

	if setup:
		#Copy setup script onto server
		run_cmd("ssh " + server_addr + " rm -f ~/setup_server.sh")
		run_cmd("scp helpers/setup_server.sh " + server_addr + ":~/")
		run_cmd("ssh " + server_addr + " chmod +x ~/setup_server.sh")

		#Execute setup script on server
		run_cmd("ssh " + server_addr + " ~/setup_server.sh")

	#Copy server apps onto server
	for i in range(num_teams):
		cdir = "/mnt/server" + str(i)
		cmd = "ssh " + server_addr + " \"rm -rf " + cdir + " && mkdir -p " + cdir + "\"";
		#print cmd
		run_cmd(cmd)

		teamdata = tcfg["teams"][i]
		cmd = "scp -r " + teamdata["ServerDataDir"] + "/* " + server_addr + ":" + cdir
		#print cmd
		run_cmd(cmd)

	return

#######################
# Individual contest
#######################
def run_contest(first, second):

	teams = [first, second]
	filename = "/mnt/" + os.path.basename(tcfg["TestFile"])
	print "Pre-launching both servers on " + tcfg["ServerAddr"]
	for team in teams:
		cmd = "ssh " + tcfg["ServerAddr"] + " /mnt/server" + str(team) + "/" + tcfg["teams"][team]["ServerPreStartCmd"]
		run_cmd(cmd)

	print "Pre-launching client[" + tcfg["teams"][first]["Name"] +  "] on " + tcfg["Client1Addr"]
	cmd = "ssh " + tcfg["Client1Addr"] + " /mnt/client" + str(first) + "/" + tcfg["teams"][first]["ClientPreStartCmd"]
	#print cmd
	run_cmd(cmd)

	print "Pre-launching client[" + tcfg["teams"][second]["Name"] +  "] on " + tcfg["Client2Addr"]
	cmd = "ssh " + tcfg["Client2Addr"] + " /mnt/client" + str(second) + "/" + tcfg["teams"][second]["ClientPreStartCmd"]
	print cmd
	run_cmd(cmd)

	print "Launch [" + tcfg["teams"][first]["Name"] +  "] server on  "+ tcfg["ServerAddr"] + ":" + str(tcfg["ServPort1"])
	s1cmd = "ssh " + tcfg["ServerAddr"] + " \" cd  /mnt/server" + str(first) + " && " + " /mnt/server" + str(first) + "/" + tcfg["teams"][first]["ServerStartCmd"] + " " + str(tcfg["ServPort1"]) + "\""
	#print s1cmd

	print "Launch [" + tcfg["teams"][second]["Name"] +  "] server on "+ tcfg["ServerAddr"] + ":" + str(tcfg["ServPort2"])
	s2cmd = "ssh " + tcfg["ServerAddr"]  +" \" cd  /mnt/server" + str(second) + " && " + " /mnt/server" + str(second) + "/" + tcfg["teams"][second]["ServerStartCmd"] + " " + str(tcfg["ServPort2"]) + "\""
	#print s2cmd

	print "Launch [" + tcfg["teams"][first]["Name"] +  "] client on "+ tcfg["Client1Addr"] + " to conn to " + tcfg["ServerAddr"] + ":" + str(tcfg["ServPort1"])
	c1cmd = "ssh " + tcfg["Client1Addr"] + " \" cd  /mnt/client" + str(first) + " && " + " /mnt/client" + str(first) + "/" + tcfg["teams"][first]["ClientStartCmd"]+ " " + tcfg["ServerNodeAddr"] + " " + str(tcfg["ServPort1"]) + " " + filename + "\""
	#print c1cmd

	print "Launch [" + tcfg["teams"][second]["Name"] +  "] client on "+ tcfg["Client2Addr"] + " to conn to " + tcfg["ServerAddr"] + ":" + str(tcfg["ServPort2"])
	c2cmd = "ssh " + tcfg["Client2Addr"] +" \" cd  /mnt/client" + str(second) + " && " + " /mnt/client" + str(second) + "/" + tcfg["teams"][second]["ClientStartCmd"]+ " " + tcfg["ServerNodeAddr"] + " " + str(tcfg["ServPort2"]) + " " + filename + "\""
	#print c2cmd

	t1 = runTest();
	t2 = runTest();
	t1.set_cmds(s1cmd, c1cmd)
	t2.set_cmds(s2cmd, c2cmd)
	t1.start()
	t2.start()
	t1.join()
	t2.join()

	filename = os.path.basename(tcfg["TestFile"])
	server1_md5cmd = "ssh " + tcfg["ServerAddr"] + " \" cd  /mnt/server" + str(first) + " && md5sum /mnt/server" + str(first) +"/" + filename + " | cut -b-32\""
	server1_md5 = run_cmd(server1_md5cmd)
	
	server2_md5cmd = "ssh " + tcfg["ServerAddr"] + " \" cd  /mnt/server" + str(second) + " && md5sum /mnt/server" + str(second) +"/" + filename + " | cut -b-32\""
	server2_md5 = run_cmd(server2_md5cmd)
	
	md5cmd =  "md5 -q " + tcfg["TestFile"]
	md5 = run_cmd(md5cmd)

	t1fail = False
	t2fail = False
	if(server1_md5 != md5):
		print tcfg["teams"][first]["Name"] + " file transfer checksum failed!"
		t1fail = True
	
	if(server2_md5 != md5):
		print tcfg["teams"][second]["Name"] + " file transfer checksum failed!"
		t2fail = True
	
	if t1fail and not t2fail:
		return first
	if t2fail and not t1fail:
		return second
	if t1fail and t2fail:
		print "Both teams checksum failed! fall through to check time"
	
	fsize = os.path.getsize(tcfg["TestFile"])
	t1time = t1.get_time()
	t1rate = (fsize*8/t1time)/(1024*1024)
	print tcfg["teams"][first]["Name"] + " took " + str(t1time) + " seconds at " + str(t1rate) + " Mbps"
	
	t2time = t2.get_time()
	t2rate = (fsize*8/t2time)/(1024*1024)
	print tcfg["teams"][second]["Name"] + " took " + str(t2time) + " seconds at " + str(t2rate) + " Mbps"

	if(t1time < t2time):
		return second
	elif(t2time < t1time):
		return first
	else:
		print "***********************"
		print "TIE! - randomly failing the first guy, but you might want to run this contest again"
		print "***********************"
		return first

setup_server(tcfg["ServerAddr"], options.setup)
setup_client(tcfg["Client1Addr"], options.setup)
setup_client(tcfg["Client2Addr"], options.setup)

#######################
# Tournament
#######################
contestants = range(num_teams)
heat = 1
while len(contestants) > 1:
	print "Round: " + str(heat)
	heat += 1
	print "="*10
	print "Teams in contest:"
	for x in contestants:
		print tcfg["teams"][x]["Name"] + ",",
	print ""
	print "="*10
	i = 0
	while i < len(contestants) - 1:
		print "Contest between [" + tcfg["teams"][contestants[i]]["Name"] + "] and [" + tcfg["teams"][contestants[i+1]]["Name"] + "]"
		loser = run_contest(i, i+1);
		print "  > " + tcfg["teams"][contestants[loser]]["Name"] + " eliminated"
		del(contestants[loser])
		if loser == i:
			i = loser + 1
		else:
			i = loser

	print "#"*30
print "The winner is: " + tcfg["teams"][contestants[0]]["Name"]

