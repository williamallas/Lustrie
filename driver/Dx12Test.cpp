// Dx12Test.cpp : définit le point d'entrée pour l'application console.
//

#define D3DCOMPILE_DEBUG 1

#include "graphics/System.h"
#include "core/LinearAllocator.h"
#include <iostream>
#include <array>

int main(int, char*[])
	{
	const char* n = nullptr;
	uintptr_t t = reinterpret_cast<uintptr_t>(n);
	FILE* file;
	freopen_s(&file, "log.txt", "a", stderr);

	System system;
	system.run();

	system.close();

	std::cout << "Press enter to exit\n";
	getchar();
    return 0;
}

