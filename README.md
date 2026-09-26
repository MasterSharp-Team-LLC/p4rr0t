# a pigeon that bypasses!?
![P4rr0t](parrot.png)
A multi exploit process to self-kill management softwares

# How it works
Assuming I don't want to break anything: if I had created a simple C# application to kill a process, it would have required administrator privileges. With this method, however, we use an external program to inject a DLL that tricks Windows into thinking the program itself wants to terminate. As a first approach, let's try creating an intentional bug that uses int pointers (*) to crash the program. If this fails, it might be due to insufficient privileges. In this case, a second method involves having the program close itself from within, without triggering any exit errors. If this method fails, the program start a receiver and if you connect the sender with keyboard/mouse on an other computer or Arduino, Windows will automatically accept these input and move the mouse or keyboard for you. 

# Exploits
First method: *Normal Process kill* - 85cs/Itelcan3 (aka. @MasterSharp3210)

Second method: *DLL INJECTION with intention bug and internal exit* - Itelcan3/FranciPlay (aka. @MasterSharp3210 - @franciplay)

Third method: *Piccione - input bypass* - FranciPlay (aka. @franciplay)

# How to compile
To compile extra-recipe you have to manually compile dll project and piccione project as release. For the main you have to write this command in Developer Powershell for Visual Studio:
```cl main.c process_kill.c dll_inject.c utils.c user32.lib shell32.lib advapi32.lib /link /subsystem:windows```

# Note
I'm working on a future exploit to bypass permission escalation and so inject DLLs into classmate programs that are executed with elevated privileges

# Thanks to <3
Thanks @franciplay for **piccione** exploit <3
