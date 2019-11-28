DIROBJ := obj/
DIREXE := exec/
DIRHEA := include/
DIRSRC := src/
DIRMAIN := ./
DIRUTILITIES := $(DIRHEA)utilities.o

CFLAGS := -I$(DIRHEA) -c -Wall -ansi -g 
IFLAGS :=  -c -Wall -ansi -g
LDLIBS := -lpthread -lrt
CC := gcc

all : dirs utilities manager pa pb pc pd

dirs:
	mkdir -p $(DIROBJ) $(DIREXE)

utilities: $(DIRHEA)utilities.c
	$(CC) $(IFLAGS) -o $(DIRUTILITIES) $^ $(LDLIBS)
	
manager: $(DIROBJ)manager.o 
	$(CC) $(DIRUTILITIES) -o $(DIRMAIN)$@ $^ $(LDLIBS)

pa: $(DIROBJ)pa.o 
	$(CC) $(DIRUTILITIES) -o $(DIREXE)$@ $^ $(LDLIBS)

pb: $(DIROBJ)pb.o 
	$(CC) $(DIRUTILITIES) -o $(DIREXE)$@ $^ $(LDLIBS)

pc: $(DIROBJ)pc.o 
	$(CC) $(DIRUTILITIES) -o $(DIREXE)$@ $^ $(LDLIBS)
	
pd: $(DIROBJ)pd.o 
	$(CC) $(DIRUTILITIES) -o $(DIREXE)$@ $^ $(LDLIBS)
	 
$(DIROBJ)%.o: $(DIRSRC)%.c
	$(CC) $(CFLAGS) $^ -o $@

test:
	./manager /home/joselu/SSOO2/P1/Resources/Estudiantes.txt /home/joselu/SSOO2/P1/Resources /home/joselu/SSOO2/P1/ALUMNOS

clean : 
	rm -rf *~ core $(DIROBJ) $(DIREXE) $(DIRHEA)*~ $(DIRSRC)*~ $(DIRUTILITIES) 
