javah -classpath ../bin com.intellidrive.iscsi.IntellidriveBlockDeviceController

g++ -c -O4 -funroll-loops -I"%JAVA_HOME%/include" -I"%JAVA_HOME%/include/win32" intellidrive.cpp
g++ -shared -Wl,--add-stdcall-alias intellidrive.o -lwsock32 -o intellidrive.dll

g++ -c -O4 -funroll-loops -I"../lib/galib247" genetic.cpp