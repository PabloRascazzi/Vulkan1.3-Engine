#include <time.h>
#include <SDL2/SDL.h>
#include <iostream>

double Time::DT, Time::elapsedTime;
Uint64 Time::timeInTicksAtStartup;
double Time::fpsTime; int Time::frameCount;

void Time::setup() {
	DT = 0, elapsedTime = 0;
	timeInTicksAtStartup = SDL_GetTicks64();
}

double Time::timeNow() {
	return (SDL_GetTicks64() - timeInTicksAtStartup) / 1000.0;
}

void Time::update() {
	//Compute times needed for controlling frame rate independent effects. 
	//Note: 30 frames per second means 1/30 seconds per frame = 0.03333...
	//If running slower than 30 frames per second, pretend it's 30.
	static double lastTimeInSeconds = 0.0f;
	double timeInSeconds = timeNow();
	DT = timeInSeconds - lastTimeInSeconds;
	if (DT > 0.3333) DT = 0.3333;  //Greater means took more time (slower)
	elapsedTime += DT;
	lastTimeInSeconds = timeInSeconds;
}

void Time::fpsCounter() {
	fpsTime += DT; frameCount++;
	if(fpsTime > 1) {
		std::cout << "FPS: " << frameCount << ", FPSTime: " << fpsTime << std::endl;
		frameCount = 0; fpsTime = 0;
	}
}

double Time::getDT() { return DT; }
double Time::getElapsedTime() { return elapsedTime; }