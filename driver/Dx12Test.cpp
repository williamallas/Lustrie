// Dx12Test.cpp�: d�finit le point d'entr�e pour l'application console.
//

#define D3DCOMPILE_DEBUG 1

#include "System.h"


int main(int, char*[]){
	freopen("log.txt", "a", stderr);

	System system;
	system.run();
    return 0;
}

