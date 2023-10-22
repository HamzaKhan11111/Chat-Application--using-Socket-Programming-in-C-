# Chat-Application--using-Socket-Programming-in-C-
Compile:
	1) gcc server.c -o server -lpthread
	2) gcc client.c -o client

Execute:
	1)./server
	2)./client

TUTORIAL:
1) Firstly, enter valid username and password. Hardcoded usernames and passwords are:
	a) username : hamza , password : hamza
	b) username : ahmed , password : ahmed
	c) username : ali , password : ali

2) Send message directly to other users using 'direct' command.
	example) if hamza wants to send something to ahmed : 'direct ahmed hello, how are you'

3) Send message in a group chat using 'group' command.
	example) if hamza wants to send something to funchat group : 'group funchat hello everyone!'

4) See history of a group chat using 'history' command.
	example) if hamza wants to see history of funchat group : 'history funchat'

5) create new chat-rooms using 'creategroup' command.
	example) if hamza wants to create a chat-room called "newchat" : 'creategroup newchat'

6) join chat-rooms you created (or others created) using 'joingroup' command.
	example) if hamza wants to join chat-room called "newchat" : 'joingroup newchat'

7) If client want to disconnect with the server, simple write 'exit'.

SCENARIOS HANDLED:
1) Need to authenticate before being able to send messages.
2) 2 clients can't connect with the server using same username as all the active users are kept track of (in the server).
3) Users need to join chatroom before being able to message in them.
4) User can't create chatroom with names that are already present.
5) If a User tries to join a chat-room they are already part of, the will be notified: "You have already joined this group!"
6) If a user tries to join a chat-room that is not present, they will be notified: "No group with this name is present!"
