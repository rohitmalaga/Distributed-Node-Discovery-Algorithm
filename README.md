# Distributed-Node-Discovery-Algorithm
In this project, you have to design and implement a distributed node discovery algorithm.
Assume a distributed system in which each node only knows a small subset of other nodes in
the system when the system starts. Each node executes a node discovery algorithm that allows it to
learn about other nodes in the system. Design your algorithm in such a way that: (a) it eventually
terminates, and (b) it is time-efficient.
Note that the initial knowledge graph may not be symmetric. You can assume that, even if a
node x does not know another node y in the beginning, x can send a message to y directly once
it learns about y (either on receiving a message from y or through another node z). You cannot
assume the existence of a distinguished node in the system; all nodes should perform their own node
discovery. If your algorithm requires a distinguished node, then such a node should be selected as
part of your node discovery algorithm itself.
For convenience, you can assume that there is a configuration file that contains the following
information for each node: (a) its location (host name or IP address as well as port number) and
(b) its initial knowledge about other nodes in the system. Feel free to design your own format for
the configuration file. Note that the configuration file is only used to make it easier to initialize
the state of each node. It should not be used as a mechanism for node discovery. After all, it
contains information about all the nodes in the system! (The ideal approach is to use a separate
configuration file for each node but I decided against it because it would have been too cumbersome
and error-prone.)
