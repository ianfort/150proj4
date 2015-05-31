#include "VirtualMachine.h"
#include <fcntl.h>

void DumpDirectoryEntry(SVMDirectoryEntry *DirectoryEntry)
{
    int Mil, Kil, One;
    
    VMPrint("%04d/%02d/%02d %02d:%02d %s ",DirectoryEntry->DModify.DYear, DirectoryEntry->DModify.DMonth, DirectoryEntry->DModify.DDay, (DirectoryEntry->DModify.DHour % 12) ? (DirectoryEntry->DModify.DHour % 12) : 12 , DirectoryEntry->DModify.DMinute, DirectoryEntry->DModify.DHour >= 12 ? "PM" : "AM");
    VMPrint("%s ", DirectoryEntry->DAttributes & VM_FILE_SYSTEM_ATTR_DIRECTORY ? "<DIR> " : "<FILE>");
    Mil = DirectoryEntry->DSize / 1000000;
    Kil = (DirectoryEntry->DSize / 1000) % 1000;
    One = DirectoryEntry->DSize % 1000;
    if(Mil){
        VMPrint("%3d,%03d,%03d ",Mil, Kil, One);   
    }
    else if(Kil){
        VMPrint("    %3d,%03d ", Kil, One);
    }
    else if(0 == (DirectoryEntry->DAttributes & VM_FILE_SYSTEM_ATTR_DIRECTORY)){
        VMPrint("        %3d ",One);
    }
    else{
        VMPrint("            ");   
    }
    VMPrint("%-13s %s\n",DirectoryEntry->DShortFileName, DirectoryEntry->DLongFileName);
}

void CatFile(int FileDescriptor, char* DataBuffer, int BufferLength)
{
    int Length = BufferLength;
    
    while (VM_STATUS_SUCCESS == VMFileRead(FileDescriptor, DataBuffer, &Length)) {
        if (Length) {
            VMFileWrite(1, DataBuffer,&Length);
        }
        if (Length < BufferLength) {
            break;
        }
        Length = BufferLength;
    }
}

void VMMain(int argc, char *argv[]){
    int DirDescriptor, FileDescriptor, Length;
    char DataBuffer[1024];
    char DirectoryName[VM_FILE_SYSTEM_MAX_PATH];
    SVMDirectoryEntry DirectoryEntry;
    
    //
    // Test Directory-Related operations
    //
    
    // ls
    VMPrint("Testing Directory APIs...\n");
    {
        if (VM_STATUS_SUCCESS == VMDirectoryOpen(DirectoryName, &DirDescriptor)){
            VMPrint("   DATE   |  TIME  | TYPE |    SIZE   |    SFN      |  LFN\n");
            while (VM_STATUS_SUCCESS == VMDirectoryRead(DirDescriptor, &DirectoryEntry)) {
                DumpDirectoryEntry(&DirectoryEntry);
            }
            VMDirectoryClose(DirDescriptor);
            VMPrint("\n");
        }
        else{
            VMPrint("Failed to open directory %s!\n", DirectoryName);   
        }
        
        // cd and get current
        VMPrint("Change Directory and Get Current Directory...\n");
        if (VM_STATUS_SUCCESS != VMDirectoryChange("apps")) {
            VMPrint("Failed to change directory to apps!\n");
        }
        VMDirectoryCurrent(DirectoryName);
        VMPrint("Current Directory Name: %s.\n\n", DirectoryName);
        
        // Open and read
        VMPrint("Open Current Directory and Read...\n");
        VMDirectoryOpen(DirectoryName, &DirDescriptor);
        while (VM_STATUS_SUCCESS == VMDirectoryRead(DirDescriptor, &DirectoryEntry)) {
            DumpDirectoryEntry(&DirectoryEntry);
        }
        VMPrint("\n");
        
        // Rewind and read
        VMPrint("Rewind Directory and Read...\n");
        VMDirectoryRewind(DirDescriptor);
        VMDirectoryRead(DirDescriptor, &DirectoryEntry);
        while (VM_STATUS_SUCCESS == VMDirectoryRead(DirDescriptor, &DirectoryEntry)) {
            DumpDirectoryEntry(&DirectoryEntry);
        }
        VMDirectoryClose(DirDescriptor);
        VMPrint("\n");
    }

    // 
    // Test file-related operations
    // 
    {
        VMPrint("Testing cat file.c...\n");
        if (VM_STATUS_SUCCESS == VMFileOpen("file.c", O_RDONLY, 0644, &FileDescriptor)) {
            CatFile(FileDescriptor, DataBuffer, sizeof(DataBuffer));
            VMFileClose(FileDescriptor);
            
            VMFileOpen("test.txt", O_CREAT | O_TRUNC | O_RDWR, 0644, &FileDescriptor);

            Length = sizeof(DataBuffer) - 48;
            VMFileWrite(FileDescriptor, DataBuffer + 48, &Length);
            VMFileClose(FileDescriptor);
        }
        else {
            VMPrint("Failed to open file file.c\n");
        }
    }
    
    // Verification
    // Open and read
    {
        VMPrint("Open Current Directory and Read...\n");
        VMDirectoryOpen(DirectoryName, &DirDescriptor);
        while (VM_STATUS_SUCCESS == VMDirectoryRead(DirDescriptor, &DirectoryEntry)) {
            DumpDirectoryEntry(&DirectoryEntry);
        }
        VMPrint("\n");
        
        VMPrint("Cat test.txt...\n");
        VMFileOpen("test.txt", O_RDONLY, 0644, &FileDescriptor);
        CatFile(FileDescriptor, DataBuffer, sizeof(DataBuffer));
        VMFileClose(FileDescriptor);
        VMPrint("\n");
    }
}

