// Dx12Test.cpp�: d�finit le point d'entr�e pour l'application console.
//

#define D3DCOMPILE_DEBUG 1

#include "graphics/System.h"


int main(int, char*[]){
	FILE* file;
	freopen_s(&file, "log.txt", "a", stderr);

	System system;
	system.run();

	system.close();

	std::cout << "Press enter to exit\n";
	getchar();
    return 0;
}

