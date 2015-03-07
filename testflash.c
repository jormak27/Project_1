//tell VS you are using User32.lib
# pragma comment (lib, "User32.lib")
# include <windows.h>
# include <stdlib.h> 
int main() {
	HWND hwnd = GetConsoleWindow() ;
	//   sizeof, hwnd, flags, count, rate in milliseconds (I guess the delay between flashes?)
	FLASHWINFO flashInfo = { sizeof(FLASHWINFO), hwnd, FLASHW_ALL, 5, 100 };

	int i;
	for (i = 0; i<20; i++) {
		FlashWindowEx(&flashInfo);
		sleep(10);
	}
	// some delay to make sure there is time to finish the flashing...
 	//     (your program will probably not need this but since I don't actually do anything I do).
	//system("pause");
	return 0;
}

// from http://www.dreamincode.net/forums/topic/169669-screen-flashing-in-console/

// also why we should not use paush
// http://stackoverflow.com/questions/1107705/systempause-why-is-it-wrong



/* 
For step four, think of buffer like youtube.
It buffers faster than receive, but you 'poll' at set interval.

OK. That's for -b.

*/