echo 'trade ,book run in background..'
nohup ./uncertentrade3m >/dev/null 2>&1 &
nohup ./uncertentbook3m >/dev/null 2>&1 &
