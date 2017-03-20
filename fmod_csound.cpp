/*
Copyright (C) 2016 Rory Walsh

CsoundFMOD is free software; you can redistribute it
and/or modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.
This software is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
You should have received a copy of the GNU Lesser General Public
License along with Csound; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA
*/
#define _CRT_SECURE_NO_WARNINGS

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <cstring>
#include <vector>
#include "dirent.h"
#include "fmod.hpp"
#ifdef WIN32
#include "windows.h"
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#else
#include <dlfcn.h>
#include <CoreFoundation/CoreFoundation.h>
#endif
#include <fstream>



#define MAXPLUGINS 512
using namespace std;


extern "C" {
	F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription();
}

const float FMOD_CSOUND_PARAM_GAIN_MIN = -80.0f;
const float FMOD_CSOUND_PARAM_GAIN_MAX = 10.0f;
const float FMOD_CSOUND_PARAM_GAIN_DEFAULT = 0.0f;

#include "csound.h"


//===========================================================
// simple class for holding information about Csound instruments and channels
//===========================================================
#define MIN 0
#define MAX 1
#define VALUE 2


struct CsoundChannel
{
	float range[3];
	string name, text, label, caption, type;
};

vector<CsoundChannel> csoundChannels;

//============================================================
enum FMOD_CSOUND_FORMAT
{
	FMOD_CSOUND_FORMAT_MONO = 0,
	FMOD_CSOUND_FORMAT_STEREO,
	FMOD_CSOUND_FORMAT_5POINT1
};

#define DECIBELS_TO_LINEAR(__dbval__)  ((__dbval__ <= FMOD_CSOUND_PARAM_GAIN_MIN) ? 0.0f : powf(10.0f, __dbval__ / 20.0f))
#define LINEAR_TO_DECIBELS(__linval__) ((__linval__ <= 0.0f) ? FMOD_CSOUND_PARAM_GAIN_MIN : 20.0f * log10f((float)__linval__))

FMOD_RESULT F_CALLBACK FMOD_Csound_dspcreate(FMOD_DSP_STATE *dsp);
FMOD_RESULT F_CALLBACK FMOD_Csound_dsprelease(FMOD_DSP_STATE *dsp);
FMOD_RESULT F_CALLBACK FMOD_Csound_dspreset(FMOD_DSP_STATE *dsp);
FMOD_RESULT F_CALLBACK FMOD_Csound_dspprocess(FMOD_DSP_STATE *dsp, unsigned int length, const FMOD_DSP_BUFFER_ARRAY *inbufferarray, FMOD_DSP_BUFFER_ARRAY *outbufferarray, FMOD_BOOL inputsidle, FMOD_DSP_PROCESS_OPERATION op);
FMOD_RESULT F_CALLBACK FMOD_Csound_dspsetparamfloat(FMOD_DSP_STATE *dsp, int index, float value);
FMOD_RESULT F_CALLBACK FMOD_Csound_dspsetparamint(FMOD_DSP_STATE *dsp, int index, int value);
FMOD_RESULT F_CALLBACK FMOD_Csound_dspsetparambool(FMOD_DSP_STATE *dsp, int index, bool value);
FMOD_RESULT F_CALLBACK FMOD_Csound_dspsetparamdata(FMOD_DSP_STATE *dsp, int index, void *data, unsigned int length);
FMOD_RESULT F_CALLBACK FMOD_Csound_dspgetparamfloat(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr);
FMOD_RESULT F_CALLBACK FMOD_Csound_dspgetparamint(FMOD_DSP_STATE *dsp, int index, int *value, char *valuestr);
FMOD_RESULT F_CALLBACK FMOD_Csound_dspgetparambool(FMOD_DSP_STATE *dsp, int index, bool *value, char *valuestr);
FMOD_RESULT F_CALLBACK FMOD_Csound_dspgetparamdata(FMOD_DSP_STATE *dsp, int index, void **value, unsigned int *length, char *valuestr);

//each plugin can use 1000 parameters
#define MAX_PARAMETERS 1000

static FMOD_DSP_PARAMETER_DESC csoundParameters[MAX_PARAMETERS];
string csdFilename;
FMOD_DSP_PARAMETER_DESC *FMOD_Csound_dspparam[MAX_PARAMETERS] = {};
bool debugMode = false;

//===========================================================
// generic descriptor for plguins. Members are updated once 
// the .csd file has been read. 
//===========================================================
FMOD_DSP_DESCRIPTION FMOD_Csound_Desc =
{
	FMOD_PLUGIN_SDK_VERSION,        /* [w] The plugin SDK version this plugin is built for.  set to this to FMOD_PLUGIN_SDK_VERSION defined above. */
	"FMOD Csound",                  /* [w] The identifier of the DSP. This will also be used as the name of DSP and shouldn't change between versions. */
	0x00010000,                     /* [w] Plugin writer's version number. */
	0,                              /* [w] Number of input buffers to process.  Use 0 for DSPs that only generate sound and 1 for effects that process incoming sound. */
	1,                              /* [w] Number of audio output buffers.  Only one output buffer is currently supported. */
	FMOD_Csound_dspcreate,          /* [w] Create callback.  This is called when DSP unit is created.  Can be null. */
	FMOD_Csound_dsprelease,         /* [w] Release callback.  This is called just before the unit is freed so the user can do any cleanup needed for the unit.  Can be null. */
	FMOD_Csound_dspreset,           /* [w] Reset callback.  This is called by the user to reset any history buffers that may need resetting for a filter, when it is to be used or re-used for the first time to its initial clean state.  Use to avoid clicks or artifacts. */
	0,                              /* [w] Read callback.  Processing is done here.  Can be null. */
	FMOD_Csound_dspprocess,         /* [w] Process callback.  Can be specified instead of the read callback if any channel format changes occur between input and output.  This also replaces shouldiprocess and should return an error if the effect is to be bypassed.  Can be null. */
	0,                              /* [w] Set position callback.  This is called if the unit wants to update its position info but not process data, or reset a cursor position internally if it is reading data from a certain source.  Can be null. */

	MAX_PARAMETERS,      /* [w] Number of parameters used in this filter.  The user finds this with DSP::getNumParameters */
	FMOD_Csound_dspparam,           /* [w] Variable number of parameter structures. */
	FMOD_Csound_dspsetparamfloat,   /* [w] This is called when the user calls DSP::setParameterFloat. Can be null. */
	FMOD_Csound_dspsetparamint,     /* [w] This is called when the user calls DSP::setParameterInt.   Can be null. */
	0,    /* [w] This is called when the user calls DSP::setParameterBool.  Can be null. */
	0,                              /* [w] This is called when the user calls DSP::setParameterData.  Can be null. */
	FMOD_Csound_dspgetparamfloat,   /* [w] This is called when the user calls DSP::getParameterFloat. Can be null. */
	FMOD_Csound_dspgetparamint,     /* [w] This is called when the user calls DSP::getParameterInt.   Can be null. */
	0,    /* [w] This is called when the user calls DSP::getParameterBool.  Can be null. */
	0,                              /* [w] This is called when the user calls DSP::getParameterData.  Can be null. */
	0,                              /* [w] This is called before processing.  You can detect if inputs are idle and return FMOD_OK to process, or any other error code to avoid processing the effect.  Use a count down timer to allow effect tails to process before idling! */
	0,                              /* [w] Optional. Specify 0 to ignore. This is user data to be attached to the DSP unit during creation.  Access via DSP::getUserData. */

	0,                              /* [w] Register callback.  This is called when DSP unit is loaded/registered.  Useful for 'global'/per system object init for plugin.  Can be null. */
	0,                              /* [w] Deregister callback.  This is called when DSP unit is unloaded/deregistered.  Useful as 'global'/per system object shutdown for plugin.  Can be null. */
	0,                              /* [w] System mix stage callback.  This is called when the mixer starts to execute or is just finishing executing.  Useful for 'global'/per system object once a mix update calls for a plugin.  Can be null. */
};


//===========================================================
// utility function to get name of library that was loaded
//===========================================================
#ifdef WIN32
string GetCsdFilename()
{
	char   DllPath[MAX_PATH] = { 0 };
#ifdef WIN32
	GetModuleFileName((HINSTANCE)&__ImageBase, DllPath, _countof(DllPath));
	string fileName = DllPath;
	size_t lastindex = fileName.find_last_of(".");
	string fullFilename = fileName.substr(0, lastindex);
	fullFilename.append(".csd");
	return fullFilename;
#endif

}
#else
const char* GetCsdFilename(void)
{
	Dl_info info;
	if (dladdr((void*)"GetCsdFilename", &info))
	{
		string fileName = info.dli_fname;
		size_t lastindex = fileName.find_last_of(".");
		string fullFilename = fileName.substr(0, lastindex);
		fullFilename.append(".csd");
		return fullFilename.c_str();
	}
}
#endif

//===========================================================
// to remove leading and trailing spaces from strings....
//===========================================================
string Trim(string s)
{
	//trim spaces at start
	s.erase(0, s.find_first_not_of(" \t\n"));
	//trim spaces at end
	s.erase(s.find_last_not_of(" \t\n") + 1);
	return s;
}

//===========================================================
// get csd file. This file must reside in the same directory
// as the plugin library
//===========================================================
vector<string> GetCsdFiles()
{
	vector<string> csdnames;
	DIR             *dip = NULL;
	struct dirent   *dit;
	string          temp, name, path;
	int             i = 0;
	size_t    indx = 0;
	char csd_path[1024];
	sprintf(csd_path, "%s", GetCsdFilename());
	char *src = NULL;

	if (strlen(csd_path) == 0) dip = opendir(".");
	else {
		path = csd_path;
#ifdef WIN32
		indx = path.find(";");
#else
		indx = path.find(":");
#endif
		if (indx != string::npos) {
			dip = opendir(path.substr(0, indx).c_str());
			strncpy(csd_path, path.substr(0, indx).c_str(), 1023);
			csd_path[1023] = '\0';
		}
		else dip = opendir(csd_path);
	}
	if (dip == NULL) {
		free(src);
		return csdnames;
	}
	while ((dit = readdir(dip)) != NULL)
	{
		temp = dit->d_name;
		indx = temp.find(".csd", 0);
		string validExt = Trim(temp.substr(indx + 1));
		if (!validExt.compare("csd"))
		{
			if (strlen(csd_path) != 0) {
				name = csd_path;
				name.append("/");
				name.append(temp);
			}
			else name = temp;
			if (i < MAXPLUGINS) {
				csdnames.push_back(name);
				i++;
			}
		}
	}
	closedir(dip);
	free(src);
	return csdnames;
}

//================================================================
// simple function for loading information about controls 
// and Csound channels to vector
//================================================================
static vector<CsoundChannel> GetCsoundChannelVector(string csdFile)
{
	vector<CsoundChannel> csndChannels;

	std::ifstream input(csdFile.c_str());

	std::string line;
	while (std::getline(input, line))
	{
		if (line.find("</") != std::string::npos)
			break;

		string newLine = line;
		string control = line.substr(0, line.find(" ") != std::string::npos ? line.find(" ") : 0);
		std::string::size_type i = newLine.find(control);

		if (i != std::string::npos)
			newLine.erase(i, control.length());

		if (control.find("slider") != std::string::npos ||
			control.find("button") != std::string::npos ||
			control.find("checkbox") != std::string::npos ||
			control.find("groupbox") != std::string::npos ||
			control.find("form") != std::string::npos)
		{
			CsoundChannel csndChannel;
			csndChannel.type = control;
			//init range
			csndChannel.range[MIN] = 0;
			csndChannel.range[MAX] = 1;
			csndChannel.range[VALUE] = 0;

			if (line.find("debug") != std::string::npos)
			{
				debugMode = true;
			}

			if (line.find("caption(") != std::string::npos)
			{
				string infoText = line.substr(line.find("caption(") + 9);
				infoText = infoText.substr(0, infoText.find(")") - 1);
				csndChannel.caption = infoText;
			}

			if (line.find("text(") != std::string::npos)
			{
				string text = line.substr(line.find("text(") + 6);
				text = text.substr(0, text.find(")") - 1);
				csndChannel.text = text;
			}

			if (line.find("channel(") != std::string::npos)
			{
				string channel = line.substr(line.find("channel(") + 9);
				channel = channel.substr(0, channel.find(")") - 1);
				csndChannel.name = channel;
			}

			if (line.find("range(") != std::string::npos)
			{
				string range = line.substr(line.find("range(") + 6);
				range = range.substr(0, range.find(")"));
				char *p = strtok(&range[0u], ",");
				int argCount = 0;
				while (p)
				{
					csndChannel.range[argCount] = atof(p);
					argCount++;
					//not handling increment or log sliders yet
					if (argCount == 3)
						break;
					p = strtok(NULL, ",");
				}
			}

			if (line.find("value(") != std::string::npos)
			{
				string value = line.substr(line.find("value(") + 6);
				value = value.substr(0, value.find(")"));
				csndChannel.range[VALUE] = value.length() > 0 ? atof(value.c_str()) : 0;
			}
			csndChannels.push_back(csndChannel);
		}
	}

	return csndChannels;
}


extern "C"
{
	//================================================================
	// this function is called when FMOD loads first
	//================================================================
	F_EXPORT FMOD_DSP_DESCRIPTION* F_CALL FMODGetDSPDescription()
	{
		csdFilename = GetCsdFilename();
		//        char filestring[1000];
		//        sprintf(filestring, "%s", csdFilename.c_str());
		//        CFStringRef ref = CFStringCreateWithCString(NULL, filestring, kCFStringEncodingUTF8);
		//        CFUserNotificationDisplayNotice(0, kCFUserNotificationPlainAlertLevel,
		//                                        NULL, NULL, NULL, CFSTR("Result"),
		//                                        ref, CFSTR("OK"));


		csoundChannels = GetCsoundChannelVector(csdFilename);
		int params = csoundChannels.size();
		CsoundChannel csndChannel;

		for (int i = 0; i < params; i++)
		{
			if (csoundChannels[i].type == "form")
			{
				sprintf(FMOD_Csound_Desc.name, "%s", csoundChannels[i].caption.c_str());
				//remove form from control array as it does not control anything...
				csoundChannels.erase(csoundChannels.begin() + i);
				params = csoundChannels.size();
			}
		}

		params = csoundChannels.size();
		FMOD_Csound_Desc.numparameters = params;

		for (int i = 0; i < params; i++)
		{
			FMOD_Csound_dspparam[i] = &csoundParameters[i];
			// FMOD only allows automation of float parameters?!

			//if (csoundChannels[i].type == "button" || csoundChannels[i].type == "checkbox")
			//{
			//	FMOD_DSP_INIT_PARAMDESC_INT(
			//		csoundParameters[i],
			//		csoundChannels[i].name.c_str(),
			//		"",
			//		csoundChannels[i].text.c_str(),
			//		0,
			//		1,
			//		csoundChannels[i].range[VALUE],
			//		0,
			//		0);
			//}
			//else
			{
				FMOD_DSP_INIT_PARAMDESC_FLOAT(
					csoundParameters[i],
					csoundChannels[i].name.c_str(),
					"",
					csoundChannels[i].text.c_str(),
					csoundChannels[i].range[MIN],
					csoundChannels[i].range[MAX],
					csoundChannels[i].range[VALUE]);
			}
			//FMOD_DSP_INIT_PARAMDESC_FLOAT(csoundParameters[i], csoundChannels[i].name.c_str(), "", csoundChannels[i].text.c_str(), csoundChannels[i].range[Range::MIN], csoundChannels[i].range[Range::MAX], csoundChannels[i].range[Range::VALUE]);
		}

		return &FMOD_Csound_Desc;
	}

}

//========================================================================
// Simple Csound class that handles compiling of Csound and generating audio
//========================================================================
class FMODCsound
{
public:
	FMODCsound();

	void generate(float *outbuffer, unsigned int length, int channels);
	void setFormat(FMOD_CSOUND_FORMAT format) { m_format = format; }
	FMOD_CSOUND_FORMAT format() const { return m_format; }
	CSOUND* csound;
	int csoundReturnCode;
	int ksmpsIndex, ksmps;
	MYFLT cs_scale;
	MYFLT *csoundInput, *csoundOutput;

	int sampleIndex = 0;
	int CompileCsound(string csdFile);

	bool csoundCompileOk()
	{
		if (csoundReturnCode == 0)
			return true;
		else
			return false;
	}

private:
	float m_target_level;
	float m_current_level;
	int m_ramp_samples_left;
	FMOD_CSOUND_FORMAT m_format;
};

FMODCsound::FMODCsound()
{

}

int FMODCsound::CompileCsound(string csdFile)
{
	csoundInitialize(CSOUNDINIT_NO_ATEXIT);

	csound = csoundCreate(NULL);
	csoundCreateMessageBuffer(csound, 0);


	char* args[2];
	args[0] = "csound";
	char fileName[1024];
	sprintf(fileName, "%s", csdFile.c_str());
	args[1] = fileName;


	csoundReturnCode = csoundCompile(csound, 2, (const char **)args);

	if (csoundReturnCode == 0)
	{
		ksmps = csoundGetKsmps(csound);
		csoundPerformKsmps(csound);
		cs_scale = csoundGet0dBFS(csound);
		csoundInput = csoundGetSpin(csound);
		csoundOutput = csoundGetSpout(csound);
	}
	else
	{
		// fmod doesn't allow logging but if it does in the future, this should print information to the user
		// about possible problems in their instruments
	}

	return csoundReturnCode;

}

void FMODCsound::generate(float *outbuffer, unsigned int length, int channels)
{
	if (csoundCompileOk())
	{
		unsigned int samples = length;
		unsigned int position = 0;
		while (samples--)
		{
			if (ksmpsIndex >= ksmps)
			{
				csoundPerformKsmps(csound);
				ksmpsIndex = 0;
			}

			for (int chans = 0; chans < channels; chans++)
			{
				position = ksmpsIndex*channels;
				*outbuffer++ = csoundOutput[chans + position];
			}

			ksmpsIndex++;
		}

	}
}

FMOD_RESULT F_CALLBACK FMOD_Csound_dspcreate(FMOD_DSP_STATE *dsp_state)
{
	dsp_state->plugindata = (FMODCsound *)FMOD_DSP_ALLOC(dsp_state, sizeof(FMODCsound));

	int result = ((FMODCsound *)dsp_state->plugindata)->CompileCsound(csdFilename);
	if (!dsp_state->plugindata || result != 0)
	{
		return FMOD_ERR_MEMORY;
	}
	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_Csound_dsprelease(FMOD_DSP_STATE *dsp)
{
	FMODCsound *state = (FMODCsound *)dsp->plugindata;
	FMOD_DSP_FREE(dsp, state);
	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_Csound_dspprocess(FMOD_DSP_STATE *dsp, unsigned int length, const FMOD_DSP_BUFFER_ARRAY * /*inbufferarray*/, FMOD_DSP_BUFFER_ARRAY *outbufferarray, FMOD_BOOL /*inputsidle*/, FMOD_DSP_PROCESS_OPERATION op)
{
	FMODCsound *state = (FMODCsound *)dsp->plugindata;

	if (op == FMOD_DSP_PROCESS_QUERY)
	{
		FMOD_SPEAKERMODE outmode = FMOD_SPEAKERMODE_DEFAULT;
		int outchannels = 0;

		// fixed at stereo for now....
		switch (FMOD_CSOUND_FORMAT_STEREO)
		{
		case FMOD_CSOUND_FORMAT_MONO:
			outmode = FMOD_SPEAKERMODE_MONO;
			outchannels = 1;
			break;

		case FMOD_CSOUND_FORMAT_STEREO:
			outmode = FMOD_SPEAKERMODE_STEREO;
			outchannels = 2;
			break;

		case FMOD_CSOUND_FORMAT_5POINT1:
			outmode = FMOD_SPEAKERMODE_5POINT1;
			outchannels = 6;
		}

		if (outbufferarray)
		{
			outbufferarray->speakermode = outmode;
			outbufferarray->buffernumchannels[0] = outchannels;
			outbufferarray->bufferchannelmask[0] = 0;
		}

		return FMOD_OK;
	}

	if (debugMode == true)
	{
		const int messageCnt = csoundGetMessageCnt(state->csound);

		for (int i = 0; i < messageCnt; i++)
		{

			FMOD_DSP_LOG(dsp, FMOD_DEBUG_LEVEL_WARNING, "", csoundGetFirstMessage(state->csound));
			csoundPopFirstMessage(state->csound);
		}
	}

	state->generate(outbufferarray->buffers[0], length, outbufferarray->buffernumchannels[0]);
	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_Csound_dspsetparamfloat(FMOD_DSP_STATE *dsp_state, int index, float value)
{
	FMODCsound *state = (FMODCsound *)dsp_state->plugindata;

	if (index < csoundChannels.size())
	{
		string channelName = csoundChannels[index].name;
		csoundSetControlChannel(state->csound, csoundChannels[index].name.c_str(), value);
		return FMOD_OK;
	}

	return FMOD_ERR_INVALID_PARAM;

}


FMOD_RESULT F_CALLBACK FMOD_Csound_dspsetparamint(FMOD_DSP_STATE *dsp_state, int index, int value)
{
	FMODCsound *state = (FMODCsound *)dsp_state->plugindata;
	// this function, and the setbool one are not being used becase fmod doesn't allow automation of anything 
	// other than float parameters...
	if (index < csoundChannels.size())
	{
		string channelName = csoundChannels[index].name;
		csoundSetControlChannel(state->csound, csoundChannels[index].name.c_str(), value);
		return FMOD_OK;
	}

	return FMOD_ERR_INVALID_PARAM;
}


FMOD_RESULT F_CALLBACK FMOD_Csound_dspreset(FMOD_DSP_STATE *dsp)
{
	return FMOD_OK;
}


FMOD_RESULT F_CALLBACK FMOD_Csound_dspgetparamfloat(FMOD_DSP_STATE *dsp, int index, float *value, char *valuestr)
{
	return FMOD_OK;
}

FMOD_RESULT F_CALLBACK FMOD_Csound_dspsetparambool(FMOD_DSP_STATE *dsp, int index, FMOD_BOOL value)
{
	return FMOD_OK;
}


FMOD_RESULT F_CALLBACK FMOD_Csound_dspgetparamint(FMOD_DSP_STATE *dsp, int index, int *value, char *valuestr)
{
	return FMOD_OK;
}
