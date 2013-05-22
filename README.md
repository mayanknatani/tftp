This in front of you is a FTP, which for now runs on localhost's two different ports.

Step 1: Open Two terminals
Step 2: Go to peer1_folder in one and Peer2_folder in second terminal
Step 3: execute ./peer1 

select a server port no on which you want to listen say "1234"
select a port on which you want to connect say "5678"
select a transfer protocol(tcp/udp) say "tcp"
now you can see that it will say Connection refused because there is no service on port "5678".

Step 4: execute ./peer2

select a server port no on which you want to listen (it should be the one which we gave to connect in peer1) i.e. "5678"
select a port on which you want to connect say (it should be the one which we gave to listen in peer1) i.e. "1234"
select a transfer protocol(tcp/udp) (same as of peer1) "tcp"

Now you will see that both the terminal are static and are connected with each other.. type help on one of the terminals for file transfer commands.

Ex: 
$ Download abc.mp3
$ upload abc.mp3
for more..
$ help
etc.
