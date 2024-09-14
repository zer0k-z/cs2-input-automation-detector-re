#Requires AutoHotkey v2

#SingleInstance force
#MaxThreads 1
#MaxThreadsBuffer 1
Persistent true
SetKeyDelay(0)
ProcessSetPriority("High")

SetTimer(DoLoop, 400)

global mode := 0
global toggle := false
global count := 0

;; See cfg for what these keys mean

DoOverlap()
{
	if toggle {
		SendInput("{9 down}6")
		AccurateSleep(0.04)
		SendInput("{0 up}")
	} else {
		SendInput("{0 down}6")
		AccurateSleep(0.04)
		SendInput("{9 up}")
	}
}
DoNull()
{
	if toggle {
		SendInput("{0 up}{9 down}7")
	} else {
		SendInput("{9 up}{0 down}7")
	}
}
DoUnderlap()
{
	if toggle {
		SendInput("{0 up}8")
		AccurateSleep(0.04)
		SendInput("{9 down}")
	} else {
		SendInput("{9 up}8")
		AccurateSleep(0.04)
		SendInput("{0 down}")
	}
}

DoLoop()
{
	global
	if (mode != 0)
	{
		toggle := !toggle
	}
	if (mode == 1) ; start moving
	{
		if toggle 
		{	
			SendInput("{9 down}")
		} else {
			SendInput("{0 down}")
		}
		mode := 2
		count := 0
		return
	}
	if (mode == 2)
	{
		DoOverlap()
		count := count + 1
		if (count >= 3)
		{
			mode := 3
			count := 0
		}
		return
	}
	if (mode == 3)
	{
		DoNull()
		count := count + 1
		if (count >= 18)
		{
			mode := 4
			count := 0
		}
		return
	}
	if (mode == 4)
	{
		DoUnderlap()
		count := count + 1
		if (count >= 5)
		{
		 	mode := 3
		 	count := 0
		}
		return
	}
	if (mode == 10)
	{
		SendInput("{9 up}")
		SendInput("{0 up}")
		return
	}
}

; This will go right first so make space for it
F1:: {
	global
	mode := 1
	count := 0
	toggle := true
}

; Release all keys when F2 is pressed
F2:: {
	global
	mode := 0
	count := 0
    SendInput("{9 up}")
    SendInput("{0 up}")
}

AccurateSleep(time) {
    counterFrequency := 0, startCount := 0
    DllCall("QueryPerformanceFrequency", "Int64*", &counterFrequency)
    DllCall("QueryPerformanceCounter", "Int64*", &startCount)

    DllCall("Sleep", "UInt", Floor(time * 1000.0))

    loop {
        currentTime := 0
        DllCall("QueryPerformanceCounter", "Int64*", &currentTime)
        timepassed := (currentTime - startCount) / counterFrequency
        if timepassed >= time {
            break
        }
    }
}
