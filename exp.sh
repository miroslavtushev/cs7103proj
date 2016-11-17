#!/usr/bin/expect --

set USER [lindex $argv 1];
set COUNTER 1;

while {$COUNTER > 0} {
    spawn ./client 127.0.0.1 50002;
    expect ">";
    send "user$USER\r";
    expect ">";
    send "write hey there\r";
    expect ">";
    set COUNTER [expr $COUNTER-1];
}

for {set i 1} {$i < 100} {incr i 1} {
    #sleep .$[ ( $RANDOM % 10 ) + 1 ]s;
    sleep 0.5;
    send "write random write\r";
    expect ">";
}

sleep 999
