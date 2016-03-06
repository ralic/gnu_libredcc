socat PTY,link=$HOME/sprog,raw,echo=0,wait-slave,mode=666, EXEC:"git/libredcc/dcc/simple_dcc/unix/sprog2packet /dev/pwmdma",raw,pty,echo=0&
