INTRODUCTION:
------------
This script runs a contest between various file transfer apps and 
returns the winner. It must be run on users.isi.deterlab.net.
It will login to each of the nodes and run the tournament.

The script can run a complete tournament if you have 2^n competitors. It will
run through multiple rounds to find the eventual winner

It can be modified trivially for fewer competitors.



IMPORTANT NOTES (MUST READ):
------------
- Uniformly, this script uses the term "Server" to refer to the reciever
  of the file, and "Client" to refer to the sender of the file.

- Both client and server MUST exit after succesfull file
  transmission.

- Server MUST create a file with the same name as what is sent from the client
  in the directory from which it is executed. HINT: If it does not do so, you
  can use the server script to rename the file before exiting
  
- The client/server start scripts MUST be as small as possible to avoid
  unnecessary overhead in executing commands



CONTEST DETAILS:
------------
- Transmission time is measured from when client starts to when server exits
- This script uses MD5 to check file integrity

USAGE 

INSTRUCTIONS:
------------
After setting up the config file, 
run this:


./run_tournament.py -f <config file> -s
  -h, 

--help            show this help message and exit
  
-f FILE, --file=FILE  Tournament config filename - MANDATORY
  
-s, --setup           Run Setup and push test files onto clients - may be
 slow - needed once

See config instructions below



CONFIG INSTRUCTIONS:
------------

Thihs is a one time setup, once done you can repeat this any number of times:

Before running this script, update the following files:
- 
helpers/setup_client.sh|setup_server.sh: Runs any setup needed before
 executing anything, e.g. setting up filesystem on
	client/server machines


Before running this script, each app needs the following files:
- 
ServerPreStartCmd: A script to do any config needed before executing the
 server app, eg setting up kernel params.
 Can take any length of time since this time is ignored
- ClientPreStartCmd: A script to do any config needed before executing the
 client app, eg setting up kernel params
	

Can take any length of time since this time is ignored
- 
ServerStartCmd: A script to execute the server app. This MUST return only after the server exits. Given port number as input.
- ClientStartCmd:A script to execute the client app. This MUST return only
 after the client exits. Given server address, port, and
 filename of file to transfer as input

Samples of all the above are placed in the testdata directory along with this
package


The tournament can be setup in the config.json file. The contents are explained
here:

 - 
TestFile: Path to the file to transfer. This should be on the users.isi.deterlab.net server, and will be
 copied to clients

 - 
ServerAddr: Address used to login to server from users.isi server

 - 
ServerNodeAddr: Address used to login to server from other nodes in
   experiment

 - 
Client1Addr: Address used to login to client1 from users.isi server

 - 
Client2Addr: Address used to login to client2 from users.isi server

 - 
ServPort1: Port for first server to use to listen

 - 
ServPort2: Port for second server to use to listen



Each team's app needs the following setup, 1 per team:

 - - 
Name: Team name (can be anything)
 - - 
Server Data Dir: Path to where all server files are stored on users.isi, will be copied to the server
 - - 
ServerPreStartCmd:
     ServerStartCmd   :
     ClientPreStartCmd:
     ClientStartCmd   :
	Explained above



Example Config File:

{
"TestFile" : "/users/username/cs558l/testdata/test_100M.bin",
 
 "ServerAddr" : "node3.exptname.USC558L",
 
 "ServerNodeAddr" : "node3",
 
 "Client1Addr" : "node1.exptname.USC558L",
        
 "Client2Addr" : "node2.exptname.USC558L",
        
 "ServPort1" : 10000,
        
 "ServPort2" : 20000,
        
 "teams" : [ 	{ "Name" : "Team 001",
 "ServerDataDir" :" /users/username/cs558l/testdata/app1/server",
 "ServerPreStartCmd" : "ServerPreStart1.sh","ServerStartCmd" : "ServerStart1.sh",
"ClientDataDir" :" /users/username/cs558l/testdata/app1/client",
"ClientPreStartCmd" : "ClientPreStart1.sh" ,"ClientStartCmd" : "ClientStart1.sh" },

                        
		{ "Name" : "Team 002",
"ServerDataDir" :" /users/username/cs558l/testdata/app2/server","ServerPreStartCmd" : "ServerPreStart1.sh", "ServerStartCmd" : "ServerStart1.sh",
"ClientDataDir" :" /users/username/cs558l/testdata/app2/client",
            "ClientPreStartCmd" : "ClientPreStart1.sh" ,"ClientStartCmd" : "ClientStart1.sh"}
]
}

