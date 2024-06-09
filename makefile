
CC=rbscc

a.bin: base_io.o c_basic.o Interrupt.o kernel.o keybord.o memory.o screen_ctrl.o session.o task.o time.o 
	$(CC) base_io.o c_basic.o Interrupt.o kernel.o keybord.o memory.o screen_ctrl.o session.o task.o time.o

base_io.o: base_io.c base_io.h screen_ctrl.h
	$(CC) base_io.c -c
c_basic.o: c_basic.c c_basic.h
	$(CC) c_basic.c -c
Interrupt.o: Interrupt.c Interrupt.h base_type.h task_basic_info.h base_io.h task.h keybord.h c_basic.h screen_ctrl.h basic_info.h base_type.h
	$(CC) Interrupt.c -mgeneral-regs-only -c
kernel.o: kernel.c c_basic.h base_io.h screen_ctrl.h Interrupt.h task.h base_type.h task_basic_info.h memory.h time.h keybord.h basic_info.h session.h
	$(CC) kernel.c -c
keybord.o: keybord.c keybord.h base_io.h task.h base_type.h task_basic_info.h session.h
	$(CC) keybord.c -c
memory.o: memory.c memory.h base_io.h basic_info.h task_basic_info.h base_type.h task.h
	$(CC) memory.c -c
screen_ctrl.o: screen_ctrl.c screen_ctrl.h base_io.h basic_info.h task.h task_basic_info.h base_type.h c_basic.h
	$(CC) screen_ctrl.c -c
session.o: session.c session.h c_basic.h screen_ctrl.h basic_info.h
	$(CC) session.c -c
task.o: task.c task.h basic_info.h session.h base_type.h task_basic_info.h c_basic.h screen_ctrl.h memory.h base_io.h
	$(CC) task.c -c
time.o: time.c time.h base_io.h
	$(CC) time.c -c
buffer.o: buffer.h buffer.c
	$(CC) buffer.c -c

clean:
	del *.o
	del a.bin
