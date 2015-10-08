// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void 
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable)
{
    NoffHeader noffH;
    unsigned int i, size;

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;
	
//	fileSystem->Create(nom,(int)size); //----------------------------------Modificacion
    ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);
// first, set up the translation 
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
	pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
	pageTable[i].physicalPage = i;
	pageTable[i].valid = TRUE;
	pageTable[i].use = FALSE;
	pageTable[i].dirty = FALSE;
	pageTable[i].readOnly = FALSE;  // if the code segment was entirely on 
					// a separate page, we could set its 
					// pages to be read-only
    }

    
// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment
    bzero(machine->mainMemory, size);

// then, copy in the code and data segments into memory
    if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
			noffH.code.virtualAddr, noffH.code.size);
        executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]),
			noffH.code.size, noffH.code.inFileAddr);
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", 
			noffH.initData.virtualAddr, noffH.initData.size);
        executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]),
			noffH.initData.size, noffH.initData.inFileAddr);
    }

}

AddrSpace::AddrSpace(OpenFile *executable,char *nombresito)
{
    NoffHeader noffH;
    unsigned int i, size;
    int length=strlen(nombresito);
    int ii=0;
    int j=0;
    int d=0;
    //int *buffer;		
    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?

    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size 
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
    printf(" tamanio del proceso: %d bytes\n",size);//-----------------------------------------------------Modificado aqui mero 
					
    numPages = divRoundUp(size, PageSize);
	
    printf(" numero de paginas: %d \n",numPages);//-----------------------------------------------------Modificado aqui mero 
    size = numPages * PageSize;//-----------------------------------------------------Modificado aqui mero 

    /*/////////////////////// para quitar la ruta del archivo y dejar solamente el nombre  
    *////////////////////////////////////////////////////////////////////////////////////
    while(nombresito[ii]!='\0')
    {
	if(nombresito[ii]=='/')
		d=ii;
		ii++;
    }
    char nombre[length-d-1];
    j=length-d-1;
    while(nombresito[length]!='/')
    {
	nombre[j]=nombresito[length];		
	length--;
	j--;
    }
    /*////////////////////////////////////////////////////////////////////////////////////
    */////////////////////////////////////////////////////////////////////////////////////
    
    /* Crea un archivo con el nombre del proceso a ejecutar y una extension .swp */
    fileSystem->Create(strcat(nombre,".swp"),size);
    archivito=fileSystem->Open(nombre);
    /* Crea un arreglo de caracteres del tamaño	del código y los datos */
    char buffer[noffH.code.size+noffH.initData.size];

    /* copia en el arreglo el codigo que se encuetra en el ejecutable */
    executable->ReadAt(&(buffer[0]),noffH.code.size,noffH.code.inFileAddr);
    /* Copia en el arreglo los datos que se encuentran en el ejecutable */
    executable->ReadAt(&(buffer[noffH.code.size]),noffH.initData.size,noffH.initData.inFileAddr);
    /*escribe en el archivo creado con extension .swp el arreglo que contiene los datos leidos*/
    archivito->WriteAt((char*)buffer,noffH.code.size+noffH.initData.size,0);
    ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", 
					numPages, size);
// first, set up the translation 
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
	pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
	//printf("Pagina virtual: %d \n",i);//---------------------------------------------------------Modificado aqui mero
	pageTable[i].physicalPage = i;
	//printf("Pagina fisica: %d \n",i);//---------------------------------------------------------Modificado aqui mero
	pageTable[i].valid = TRUE;
	//printf(pageTable[i].valid ? "valido: true \n" : "valido: false \n");//---------------------------------Modificado aqui mero
	pageTable[i].use = FALSE;
	//printf(pageTable[i].use ? "uso: true \n" : "uso: false \n");//----------------------------------Modificado aqui mero
	pageTable[i].dirty = FALSE;
	//printf(pageTable[i].dirty ? "dirty: true \n" : "dirty: false \n");//---------------------------Modificado aqui mero
	pageTable[i].readOnly = FALSE;  // if the code segment was entirely on 
	//printf(pageTable[i].readOnly ? "solo lectura: true \n" : "solo lectura: false \n");//---------------------Modificado aqui mero	
	//printf("\n");		// a separate page, we could set its 
					// pages to be read-only
    }
    
// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment
    bzero(machine->mainMemory, size);

// then, copy in the code and data segments into memory
    if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
			noffH.code.virtualAddr, noffH.code.size);
        executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr]),
			noffH.code.size, noffH.code.inFileAddr);
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", 
			noffH.initData.virtualAddr, noffH.initData.size);
        executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr]),
			noffH.initData.size, noffH.initData.inFileAddr);
    }

}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
   delete pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}
