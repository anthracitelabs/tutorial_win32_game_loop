#include <windows.h>
#include <stdint.h>
#include <stdio.h>


typedef int32_t i32;
typedef int64_t int64;
typedef uint64_t u64;
typedef float f32;
typedef i32 bool32;

u64 globalPerfCounterFrequency;

inline float Win32GetSecondsElapsed(u64 start, u64 End)
{
	f32 result = 0;
	result = (f32)(End - start) / (f32)globalPerfCounterFrequency;
	return result;
}

inline u64 Win32GetPerfCounter()
{
	LARGE_INTEGER result;
	QueryPerformanceCounter(&result);
	return result.QuadPart;
}

void main()
{
	f32 targetSecondsPerFrame = 1.0f / 30.0f;

	LARGE_INTEGER counterPerSecond;
	QueryPerformanceFrequency(&counterPerSecond);
	globalPerfCounterFrequency = counterPerSecond.QuadPart;

	u64 lastCounter = Win32GetPerfCounter();

	int my_clock_hours = 0;
	int my_clock_minutes = 0;
	int my_clock_seconds = 0;
	int my_frame_counter = -1;
	f32 MSPerFrame = 0.0f;

	bool32 sleepIsGranular = (timeBeginPeriod(1) == TIMERR_NOERROR);

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
		printf("\r %02d:%02d:%02d", my_clock_hours, my_clock_minutes, my_clock_seconds);
		//if(MSPerFrame > 33.35)
			//printf("target: %f, mspf: %f\n", targetSecondsPerFrame, MSPerFrame);

		// Lock the frame rate
		f32 secondsElapsed = Win32GetSecondsElapsed(lastCounter, Win32GetPerfCounter());
		if(secondsElapsed < targetSecondsPerFrame) {
			if(sleepIsGranular) {
				DWORD sleepMS = (DWORD)(1000 * (targetSecondsPerFrame - secondsElapsed));
				if(sleepMS > 5) {
					Sleep(sleepMS - 4);
				}
			}

			while(secondsElapsed < targetSecondsPerFrame) {
				secondsElapsed = Win32GetSecondsElapsed(lastCounter, Win32GetPerfCounter());
			}
		} else 
		{
			// We missed a frame
		}

		u64 endCounter = Win32GetPerfCounter();

		//swap new_input & old_input


		int64 CounterElapsed = endCounter - lastCounter;
		MSPerFrame = 1000.0f * (f32)CounterElapsed / (f32)counterPerSecond.QuadPart;

#if 0
		f32 MSPerFrame = 1000.0f * (f32)CounterElapsed / (f32)counterPerSecond.QuadPart;
		f32 FPS = (f32)counterPerSecond.QuadPart / (f32)CounterElapsed;
		f32 MCPF = (f32)CycleElapsed / (1000.0f * 1000.0f);

		char OutputBuffer[256];
		sprintf_s(OutputBuffer, sizeof(OutputBuffer), "ms/f: %.2f,  fps: %.2f,  mc/f: %.2f\n", MSPerFrame, FPS, MCPF);
		OutputDebugStringA(OutputBuffer);
#endif

		lastCounter = endCounter;
	}
	timeEndPeriod(1);
}
