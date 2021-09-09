# Shell-c
# Unix shell with c.

It is a simple program in c.Implemented in linux and in the environment Vc Code.

Implements the following command execution procedures.
```
  Simple commands ls,pwd
  
  Commands with parameters ls -l /home/user/
  
  Commands with exit input redirect 
  
  Commands with a pipeline ls -l | wc -l > out.txt.
``` 
  
You can terminate the main loop with exit.

To execute the main program compile it with the follow command
```
  gcc -o s shell.c
``` 
And execute it with
```
  ./s
```
