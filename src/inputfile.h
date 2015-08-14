#ifndef inputfile_H__
#define inputfile_H__

#define NOMINMAX
#include <Windows.h>
#include "SampleSource.h"

class SampleBuffer;

struct InputFileOptions
{
	enum channel_layout_e
	{
		MONO = 0,
		STEREO,
	};

	channel_layout_e outputChannelLayout;
	int outputSampleRate;

	InputFileOptions() :
		outputChannelLayout(STEREO),
		outputSampleRate(48000)
	{}

	inline int getNumChannels() const
	{
		switch(outputChannelLayout)
		{
		case MONO:   return 1;
		case STEREO: return 2;
		default: return 0;
		}
	}
};


class InputFile : public SampleSource
{
public:
	virtual ~InputFile() {};
	virtual int open(const char *filename, double startPosSeconds = 0.0, double playTimeSeconds = -1.0) = 0;
	virtual int close() = 0;
	virtual bool done() const {DebugBreak(); return 0;}
	virtual int seek(double seconds) = 0;
};

extern InputFile *CreateInputFileFFmpeg(InputFileOptions options = InputFileOptions());
extern void InitFFmpegLibrary();


#endif // inputfile_H__
