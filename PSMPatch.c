
#include <taihen.h>
#include <vitasdk.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


static SceUID LoadModuleHook = -1;
static tai_hook_ref_t LoadModuleHook_ref;

static SceUID cOpenHook = -1;
static tai_hook_ref_t cOpenHook_ref;

static SceUID cReadHook = -1;
static tai_hook_ref_t cReadHook_ref;

static SceUID cCloseHook = -1;
static tai_hook_ref_t cCloseHook_ref;

static SceUID ccReadHook = -1;
static tai_hook_ref_t ccReadHook_ref;

int ReplaceFile = 0xFF;


int getFileSize(const char *file) {
	SceUID fd = sceIoOpen(file, SCE_O_RDONLY, 0);
	if (fd < 0)
		return fd;
	int fileSize = sceIoLseek(fd, 0, SCE_SEEK_END);
	sceIoClose(fd);
	return fileSize;
}

int ReadFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, SCE_O_RDONLY, 0);
	if (fd < 0)
		return fd;

	int written = sceIoRead(fd, buf, size);

	sceIoClose(fd);
	return written;
}

int WriteFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	if (fd < 0)
		return fd;

	int written = sceIoWrite(fd, buf, size);

	sceIoClose(fd);
	return written;
}


typedef struct Storage{
SceUID fd;
char FilePath[1024];
} Storage;

Storage fdToPath[128];

void StoreFD(SceUID fd, char *path)
{
	for(int i = 0;i<128;i++)
	{
		if(fdToPath[i].fd == 0x00)
		{
			fdToPath[i].fd = (uint32_t)fd;
			strcpy(fdToPath[i].FilePath,path);
		}
		
	}
}
void RemoveFD(SceUID fd)
{
	for(int i = 0;i<128;i++)
	{
		if(fdToPath[i].fd == (uint32_t)fd)
		{
			fdToPath[i].fd = 0x00;
			memset(fdToPath[i].FilePath,0x00,1024);
		}
		
	}
}
int GetFD(SceUID fd)
{
	for(int i = 0;i<128;i++)
	{
		if(fdToPath[i].fd == (uint32_t)fd)
		{
			return i;
		}
	}
	return 0xFF;
}
void GetNewPath(char *buf,char *OldPath)
{
	memset(buf,0x00,1024);
	char *NewPath;
	NewPath = OldPath + sizeof("pss0:/top/Application/") - 1;
	sprintf(buf,"pss0:/top/Documents/p/%s",NewPath);
}
int psmIoRead_p(int arg1, char *buf, int size, int arg4, int arg5)
{
	
	int ret;
	ret = TAI_CONTINUE(int, ccReadHook_ref, arg1, buf, size, arg4, arg5);
	
	if(ReplaceFile != 0xFF)
	{
		
		char Path[1024];
		strcpy(Path,fdToPath[ReplaceFile].FilePath);
		char NewPath[1024];
		memset(NewPath,0x00,1024);
		GetNewPath(NewPath,Path);
		size = getFileSize(NewPath);
		ReadFile(NewPath,buf,size);
		
		sceClibPrintf("[PSMPatch] Replacing %s with %s\n",Path,NewPath);
		ReplaceFile = 0xFF;
	}
	
	return ret;
	}
SceUID sceIoOpen_p(const char *file, int flags, SceMode mode)
{
		SceUID ret;
		ret = TAI_CONTINUE(SceUID,cOpenHook_ref, file,flags,mode);
		sceClibPrintf("[PSMPatch] OPEN fd %x = %s\n",ret,file);
		
		char NewPath[1028];
		GetNewPath(NewPath,(char *)file);

		if(getFileSize(NewPath) > 0)
		{
			sceClibPrintf("[PSMPatch] NewPath: %s Storing %x\n",NewPath,ret);
			StoreFD(ret,(char *)file);
		}
		
		return ret;
	}
int sceIoRead_p(SceUID fd, void *data, SceSize size)
{
		int ret;
		sceClibPrintf("[PSMPatch] READ %x\n",fd);
		if(GetFD(fd) != 0xFF)
		{
			sceClibPrintf("[PSMPatch] Found file to replace! %x\n",fd);
			ReplaceFile = GetFD(fd);
			
		}
		ret = TAI_CONTINUE(int, cReadHook_ref, fd,data,size);
		return ret;
	}
int sceIoClose_p(SceUID fd)
{
		if(GetFD(fd) != 0xFF)
		{
			RemoveFD(fd);
		}
		int ret;
		ret = TAI_CONTINUE(int, cCloseHook_ref,fd);
		return ret;
	}
SceUID sceKernelLoadStartModule_p(char *path, SceSize args, void *argp, int flags, SceKernelLMOption *option, int *status)
{
	sceClibPrintf("[PSMPatch] Starting Module: %s\n",path);
	
	SceUID ret;
	ret = TAI_CONTINUE(SceUID, LoadModuleHook_ref, path, args, argp, flags, option, status);
	
	if(!strcmp(path,"app0:/module/libmono_bridge.suprx"))
	{
		sceClibPrintf("[PSMPatch] SceLibMonoBridge Detected!\n");

		ccReadHook = taiHookFunctionExport(&ccReadHook_ref, 
										  "SceLibMonoBridge",
										  TAI_ANY_LIBRARY, 
										  0x32553C73, //SceLibMonoBridge_32553C73 "psmIoRead"
										  psmIoRead_p);
										  
		cOpenHook = taiHookFunctionImport(&cOpenHook_ref, 
										  "SceLibMonoBridge",
										  TAI_ANY_LIBRARY, 
										  0x6C60AC61, //sceIoOpen
										  sceIoOpen_p);
		cReadHook = taiHookFunctionImport(&cReadHook_ref, 
										  "SceLibMonoBridge",
										  TAI_ANY_LIBRARY, 
										  0xFDB32293, //sceIoRead
										  sceIoRead_p);
		cCloseHook = taiHookFunctionImport(&cCloseHook_ref, 
										  "SceLibMonoBridge",
										  TAI_ANY_LIBRARY, 
										  0xC70B8886, //sceIoClose
										  sceIoClose_p);
								  
		//SceLibMonoBridge_32553C73 PsmIoRead
		//SceLibMonoBridge_C8E1B6B3 PsmIoOpen (didnt end up needing this one :D
		sceClibPrintf("[PSMPatch] cOpenHook %x, %x\n",cOpenHook,cOpenHook_ref);
		sceClibPrintf("[PSMPatch] cReadHook %x, %x\n",cReadHook,cReadHook_ref);
		sceClibPrintf("[PSMPatch] cCloseHook %x, %x\n",cCloseHook,cCloseHook_ref);
	}
	return ret;
}


void _start() __attribute__ ((weak, alias ("module_start"))); 

void module_start(SceSize argc, const void *args) {
	char titleid[12];
	sceAppMgrAppParamGetString(0, 12, titleid, 256);

	if(!strcmp(titleid,"PCSI00011")) // PSM Runtime
	{
		memset(fdToPath,0x00,sizeof(fdToPath));
		sceClibPrintf("[PSMPatch] Loaded!\n");
		sceClibPrintf("[PSMPatch] Running on %s\n",titleid);

		LoadModuleHook = taiHookFunctionImport(&LoadModuleHook_ref, 
										  TAI_MAIN_MODULE,
										  TAI_ANY_LIBRARY,
										  0x2DCC4AFA, //sceKernelLoadStartModule
										  sceKernelLoadStartModule_p);

		
		sceClibPrintf("[PSMPatch] LoadModuleHook %x, %x\n",LoadModuleHook,LoadModuleHook_ref);
	}
}

int module_stop(SceSize argc, const void *args) {
	return SCE_KERNEL_STOP_SUCCESS;
}
