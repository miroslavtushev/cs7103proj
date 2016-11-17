# Public Bulletin Board

To compile use
```
gcc client.c -o client
gcc server.c -o server -std=c11 -lpthread
```

Then run using
```
./server 50000
./client 127.0.0.1 50000
```
