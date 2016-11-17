#!/usr/bin/expect --

spawn ./client 127.0.0.1 50002;
expect ">";
send "testusr\r";
expect ">";
send "write this is a test message\r";
expect ">";
send "read\r";
expect ">";


    
