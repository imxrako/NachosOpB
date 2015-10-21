// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "machine.h"
#include "syscall.h"
#include "addrspace.h"
#include "stats.h"


//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

//Agarra la excepcion generada
void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);
	int r,numPag;
	int dirFis;
	char *dat = new char[PageSize];

    if ((which == SyscallException) && (type == SC_Halt)) {
	DEBUG('a', "Shutdown, initiated by user program.\n");
   	interrupt->Halt();
    }
	else if(which == PageFaultException) // si la excepción fue un fallo de página
	{
//		ASSERT(machine->numMarco < NumPhysPages); //para que siga miprimiendo las paginas usadas hasta el fallo 33
			r = machine->ReadRegister(BadVAddrReg);	// Lee en el registro la dirección lógica en la que se generó el fallo
			numPag = (unsigned) r / PageSize; // obtiene el número de página
			fileSystem->archivito->ReadAt(dat,PageSize,numPag*PageSize); //Lee en el archivo de intercambio el bloque a guardar en la memoria principal
			for(int i=0;i<PageSize;i++) // Recorrido para leer la información y guardarlo poco a poco
			{
				dirFis = (machine->numMarco*PageSize)+i; //se obtiene la dirección física
				machine->mainMemory[dirFis]=dat[i]; // se guarda en la memoria principal
			}
			// Se actualiza la información de la página
			machine->pageTable[numPag].valid=TRUE; // Indica que esta cargado
			machine->pageTable[numPag].physicalPage=machine->numMarco++; // En este caso el número de marco es igual al numero de pagina por la inicialización en addrSpace
			stats->numPageFaults=machine->numMarco;
			printf("\n---------Fallo  #%d------------\n\n",stats->numPageFaults);
			ASSERT(machine->numMarco < NumPhysPages);
	}
	else {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(FALSE);
    }
	delete dat;
}
