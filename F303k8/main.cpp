//#include "f303k8.hpp"
#include "clock.hpp"
#include "comport.hpp"
#include "wait.hpp"

int main(){;
	// Components
	Clock clock;
	Comport com;
	Wait wait;

	// Initialization
	com.init();
	wait.mswait(1000);
	com.print("Hu\r\n");
	com.print("Hu\r\n");
	com.print("Hu\r\n");
	while(1){
		
	}
}
