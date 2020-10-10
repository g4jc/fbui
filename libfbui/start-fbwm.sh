#/bin/sh

if [ "$1" == "" ]; then
	export A=4
else
	export A=$1
fi

Clock/fbclock  -c$A -geo100x100+0-0 &
MailCheck/fbcheck  -geo 150x50+200-0 -c$A &
LoadMonitor/fbload 4  -geo150x150-0-0 -c$A &
Viewer/fbview  -geo150x150-0+0 Viewer/meander.jpg -c$A &
Term/fbterm -geo90x33+10+135 -c$A &
sleep 1
Calc/fbcalc -geo150x150+220+50 -c$A &

sleep 2
# sleep needed because fbwm doesn't handle requests for autoplacement
WindowManager/fbwm -c$A 

#sleep 60
#Dump/fbdump
