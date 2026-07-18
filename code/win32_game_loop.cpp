#include <windows.h>
#include <stdint.h>
#include <stdio.h>


uint64_t globalPerfCounterFrequency;

inline float Win32GetSecondsElapsed(uint64_t start, uint64_t End)
{
	float result = 0;
	result = (float)(End - start) / (float)globalPerfCounterFrequency;
	return result;
}

inline uint64_t Win32GetPerfCounter()
{
	LARGE_INTEGER result;
	QueryPerformanceCounter(&result);
	return result.QuadPart;
}

void main()
{
	float targetSecondsPerFrame = 1.0f / 30.0f;

	LARGE_INTEGER counterPerSecond;
	QueryPerformanceFrequency(&counterPerSecond);
	globalPerfCounterFrequency = counterPerSecond.QuadPart;

	uint64_t lastCounter = Win32GetPerfCounter();

	int my_clock_hours = 0;
	int my_clock_minutes = 0;
	int my_clock_seconds = 0;
	int my_frame_counter = -1;
	float MSPerFrame = 0.0f;

	bool sleepIsGranular = (timeBeginPeriod(1) == TIMERR_NOERROR);

	while(true)
	{
		// input
		
		// update
		my_frame_counter++;
		if(my_frame_counter == 30)
		{
			my_clock_seconds++;
			if(my_clock_seconds == 60)
			{
				my_clock_seconds = 0;
				
				my_clock_minutes++;
				if(my_clock_minutes == 60)
				{
					my_clock_minutes = 0;

					my_clock_hours++;
					if(my_clock_hours == 60)
					{
						my_clock_hours = 0;
					}
				}
			}
			my_frame_counter = 0;
		}

		// draw
		printf("\r %02d:%02d:%02d\t%f ms per frame", my_clock_hours, my_clock_minutes, my_clock_seconds, MSPerFrame);

		// Lock the frame rate
		float secondsElapsed = Win32GetSecondsElapsed(lastCounter, Win32GetPerfCounter());
		if(secondsElapsed < targetSecondsPerFrame) {
			if(sleepIsGranular) {
				DWORD sleepMS = (DWORD)(1000 * (targetSecondsPerFrame - secondsElapsed));
				if(sleepMS > 5.0) 
				{
					Sleep(sleepMS - 4.0);
				}
			}
			
			// spin lock for remaining time
			while(secondsElapsed < targetSecondsPerFrame) 
			{
				secondsElapsed = Win32GetSecondsElapsed(lastCounter, Win32GetPerfCounter());
			}
		} 
		else 
		{
			// We missed a frame
		}

		uint64_t endCounter = Win32GetPerfCounter();

		int64_t CounterElapsed = endCounter - lastCounter;
		MSPerFrame = 1000.0f * (float)CounterElapsed / (float)counterPerSecond.QuadPart;

		lastCounter = endCounter;
	}
	timeEndPeriod(1);
}
