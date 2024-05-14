#pragma once

class Time {
public: 
	static void setup();
	static void update();
	static void fpsCounter();

	static double getDT();
	static double getElapsedTime();

private:
	static double DT, elapsedTime;
	static unsigned long long timeInTicksAtStartup;
	static double fpsTime; static int frameCount;

	static double timeNow();
};
