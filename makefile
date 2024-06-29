run_server: compile_server
	-./server

compile_server:
	-gcc ./server.c -o ./server 

run_client: compile_client
	-./client

compile_client:
	-gcc ./client.c -o ./client
