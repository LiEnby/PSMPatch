int pss_crypto_close_p(PSM_handle *handle)
{
	sceClibPrintf("[PSMPatch] [CLOSE] before run: PSM_handle:\n");
	sceClibPrintf("[PSMPatch] unk0 %lx\n",handle->unk0);
	sceClibPrintf("[PSMPatch] unk1 %lx\n",handle->unk1);
	sceClibPrintf("[PSMPatch] Size %lx\n",handle->filesz);
	sceClibPrintf("[PSMPatch] unk3 %lx\n",handle->unk3);
	sceClibPrintf("[PSMPatch] unk4 %lx\n",handle->unk4);

	int ret;
	ret = TAI_CONTINUE(int, cCloseHook_ref, handle);
	
	if(GetFD(handle) != 0xFF)
	{
			RemoveFD(handle);
	}
	
	sceClibPrintf("[PSMPatch] [CLOSE] after run: PSM_handle:\n");
	sceClibPrintf("[PSMPatch] unk0 %lx\n",handle->unk0);
	sceClibPrintf("[PSMPatch] unk1 %lx\n",handle->unk1);
	sceClibPrintf("[PSMPatch] Size %lx\n",handle->filesz);
	sceClibPrintf("[PSMPatch] unk3 %lx\n",handle->unk3);
	sceClibPrintf("[PSMPatch] ret: %x\n",ret);
	return ret;
}

int pss_crypto_open_p(PSM_handle *handle, char *path) {

	
	sceClibPrintf("[PSMPatch] [OPEN] before run: PSM_handle %x:\n",handle);
	sceClibPrintf("[PSMPatch] unk0 %lx\n",handle->unk0);
	sceClibPrintf("[PSMPatch] unk1 %lx\n",handle->unk1);
	sceClibPrintf("[PSMPatch] Size %lx\n",handle->filesz);
	sceClibPrintf("[PSMPatch] unk3 %lx\n",handle->unk3);
	sceClibPrintf("[PSMPatch] Path: %s\n",path);
	
	int ret;
	ret = TAI_CONTINUE(int, cOpenHook_ref, handle, path);
	
	char NPath[1028];
	GetNewPath(NPath,path);
	sceClibPrintf("[PSMPatch] NewPath: %s\n",NPath);
	
	if(getFileSize(NPath) > 0)
	{
		sceClibPrintf("[PSMPatch] Patched File Found. updating size!\n");
		StoreFD(handle,path);
		handle->filesz = getFileSize(NPath);
	}
	else
	{
		sceClibPrintf("[PSMPatch] File not found\n");
	}
	
	sceClibPrintf("[PSMPatch] [OPEN] after run: PSM_handle:\n");
	sceClibPrintf("[PSMPatch] unk0 %lx\n",handle->unk0);
	sceClibPrintf("[PSMPatch] unk1 %lx\n",handle->unk1);
	sceClibPrintf("[PSMPatch] Size %lx\n",handle->filesz);
	sceClibPrintf("[PSMPatch] unk3 %lx\n",handle->unk3);
	sceClibPrintf("[PSMPatch] Path: %s\n",path);
	sceClibPrintf("[PSMPatch] ret: %x\n",ret);

	return ret;
}

char *pss_crypto_read_p(PSM_handle *handle, int ctx) {
	sceClibPrintf("[PSMPatch] [READ] before run: PSM_handle: %x\n",handle);
	sceClibPrintf("[PSMPatch] unk0 %lx\n",handle->unk0);
	sceClibPrintf("[PSMPatch] unk1 %lx\n",handle->unk1);
	sceClibPrintf("[PSMPatch] Size %lx\n",handle->filesz);
	sceClibPrintf("[PSMPatch] unk3 %lx\n",handle->unk3);
	sceClibPrintf("[PSMPatch] ctx: %x\n",ctx);
		
	if(GetFD(handle) != 0xFF)
	{
		char *file;
		file = TAI_CONTINUE(char*, cReadHook_ref, handle, ctx);
		char Path[1024];
		memset(Path,0x00,1024);
		int i = GetFD(handle);
		strcpy(Path,Handles[i].FilePath);
		
		char NewPath[1024];
		memset(NewPath,0x00,1024);
		GetNewPath(NewPath,Path);
		
		
		sceClibPrintf("[PSMPatch] Replacing Files %s with %s\n",Path,NewPath);
		
		int size = getFileSize(NewPath);
		char *buf = malloc(size);
		ReadFile(NewPath,buf,size);
		free(file);
		//sceKernelExitProcess(0x00);
		return buf;
	}
	else
	{
	char *ret;
	ret = TAI_CONTINUE(char*, cReadHook_ref, handle, ctx);
	
	sceClibPrintf("[PSMPatch] [READ] after run: PSM_handle:\n");
	sceClibPrintf("[PSMPatch] unk0 %lx\n",handle->unk0);
	sceClibPrintf("[PSMPatch] unk1 %lx\n",handle->unk1);
	sceClibPrintf("[PSMPatch] Size %lx\n",handle->filesz);
	sceClibPrintf("[PSMPatch] unk3 %lx\n",handle->unk3);
	sceClibPrintf("[PSMPatch] ctx: %x\n",ctx);
	
	return ret;
	}
	return 0;
}

int psmIoOpen_p(char *path, int flags, SceMode mode, PSM_handle *handle, int arg5) //Might remove honestly not needed :P
{
	sceClibPrintf("[PSMPatch] [psmIoOpen()] before run:\n");
	sceClibPrintf("[PSMPatch] path: %s\n",path);
	sceClibPrintf("[PSMPatch] flags: %x\n",flags);
	sceClibPrintf("[PSMPatch] mode: %x\n",mode);
	sceClibPrintf("[PSMPatch] handle: %x\n",handle);
	sceClibPrintf("[PSMPatch] arg5: %x\n",arg5);

	int ret;
	ret = TAI_CONTINUE(int, ccOpenHook_ref, path, flags, mode,handle, arg5);
	
	sceClibPrintf("[PSMPatch] [psmIoOpen()] after run:\n");
	sceClibPrintf("[PSMPatch] path: %s\n",path);
	sceClibPrintf("[PSMPatch] flags: %x\n",flags);
	sceClibPrintf("[PSMPatch] mode: %x\n",mode);
	sceClibPrintf("[PSMPatch] handle: %x\n",handle);
	sceClibPrintf("[PSMPatch] arg5: %x\n",arg5);
	
	sceClibPrintf("[PSMPatch] ret %lx\n",ret);
	
	return ret;
}

/*
	if(GetFD(handle) != 0xFF)
		{
			int ret;

			ret = TAI_CONTINUE(int, ccReadHook_ref, arg1, buf, size, handle, arg5);
			memset(buf,0x00,size);
			char Path[1024];
			memset(Path,0x00,1024);
			int i = GetFD(handle);
			strcpy(Path,fdToPath[i].FilePath);
			
			char NewPath[1024];
			memset(NewPath,0x00,1024);
			GetNewPath(NewPath,Path);
			
			ReadFile(NewPath,buf,size);
			
			sceClibPrintf("[PSMPatch] Replacing Files %s with %s\n",Path,NewPath);
			
			return ret;
		}
		else
		{
			sceClibPrintf("[PSMPatch] Continuing PSMIoRead\n");
			int ret;
			ret = TAI_CONTINUE(int, ccReadHook_ref, arg1, buf, size, handle, arg5);
			return ret;
		}
		*/
		/*cOpenHook = taiHookFunctionExport(&cOpenHook_ref, 
										  "SceLibMonoBridge",
										  TAI_ANY_LIBRARY, 
										  0x6B4125E4, //pss_crypto_open
										  pss_crypto_open_p);
								  
		cReadHook = taiHookFunctionExport(&cReadHook_ref, 
										  "SceLibMonoBridge",
										  TAI_ANY_LIBRARY, 
										  0x32BA8444, //pss_crypto_read
										  pss_crypto_read_p);*/
		/*
		cCloseHook = taiHookFunctionExport(&cCloseHook_ref, 
										  "SceLibMonoBridge",
										  TAI_ANY_LIBRARY, 
										  0x37483E03, //pss_crypto_close
										  pss_crypto_close_p);*/
		/*
		ccOpenHook = taiHookFunctionExport(&ccOpenHook_ref, 
										  "SceLibMonoBridge",
										  TAI_ANY_LIBRARY, 
										  0xC8E1B6B3, //SceLibMonoBridge_C8E1B6B3 "psmIoOpen"
										  psmIoOpen_p); */