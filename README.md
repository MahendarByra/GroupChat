# GroupChat

This project explores three different approaches for implementing group chat functionality in C.

## 1. Using Non-Blocking Sockets

This approach involves creating a TCP concurrent server with multiple processes using the fork() function. The TCP client program is designed using Non-Blocking TCP sockets with the fcntl() function. With these two programs, multiple processes (clients) can communicate with each other.

**Program Files**: Non_Blocking_Client.c,Non_Blocking_Server.c

## 2. Using I/O Multiplexing (Select)

In this approach, a TCP concurrent server is created using the "Select" system call. Similarly, the TCP client program is also designed to utilize the "select" system call. This enables multiple processes (clients) to communicate effectively.

**Program Files**: IOM_Client.c,IOM_Server.c

## 3. Multiple Group Chat

Building upon the second approach, this extension introduces the concept of multiple groups. Clients can join any group by specifying a port number. Once joined, they can communicate with other members of the group seamlessly.

**Program Files**: MultiGroup_Client.c,MultiGroup_Server.c,header1.h

**/headerfiles** has three files "unp.h","config.h" and "libunp.a". these are the necessary files two run any of this programs.

