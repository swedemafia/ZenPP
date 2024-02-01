#include "Precomp.h"

// Global instance
std::unique_ptr<Program> App;

INT WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ INT nCmdShow)
{
	INT ReturnCode = EXIT_FAILURE;

	try
	{
		// Create an instance of our program
		App = std::make_unique<Program>(hInstance, lpCmdLine);

		// Initialize the program and run the core message loop
		if (App->InitializeProgram())
			ReturnCode = App->RunMessageLoop();

	}
	catch (CONST std::bad_alloc&) {
		MessageBox(NULL, L"Unable to allocate memory to load the program; please free up memory and try again.", L"Zen++", MB_ICONERROR | MB_OK);
	}

	return ReturnCode;
}