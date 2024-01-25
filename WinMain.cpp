#include "Precomp.h"

// Global instance
Program* App;

INT WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ INT nCmdShow)
{
	INT ReturnCode = 0;

	App = new Program(hInstance, lpCmdLine);

	if(App->InitializeProgram())
		ReturnCode = App->RunMessageLoop();

	delete App;

	return ReturnCode;
}