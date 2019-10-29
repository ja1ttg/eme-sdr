/** @file paex_record.c
	@ingroup examples_src
	@brief Record input into an array; Save array to a file; Playback recorded data.
	@author Phil Burk  http://www.softsynth.com
*/
/*
 * $Id$
 *
 * This program uses the PortAudio Portable Audio Library.
 * For more information see: http://www.portaudio.com
 * Copyright (c) 1999-2000 Ross Bencina and Phil Burk
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * The text above constitutes the entire PortAudio license; however, 
 * the PortAudio community also makes the following non-binding requests:
 *
 * Any person wishing to distribute modifications to the Software is
 * requested to send the modifications to the original developer so that
 * they can be incorporated into the canonical version. It is also 
 * requested that these non-binding requests be included along with the 
 * license above.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "portaudio.h"

#define SAMPLE_RATE  192000  /* Max rate for UR12 */
#define FRAMES_PER_BUFFER (512*4)
#define NUM_SPLIT     (10)

/* Select sample format. */
#if 0
#define PA_SAMPLE_TYPE  paFloat32
typedef float SAMPLE;
#define SAMPLE_SILENCE  (0.0f)
#define PRINTF_S_FORMAT "%.8f"
#elif 1
#define PA_SAMPLE_TYPE  paInt32
typedef int SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#elif 0
#define PA_SAMPLE_TYPE  paInt8
typedef char SAMPLE;
#define SAMPLE_SILENCE  (0)
#define PRINTF_S_FORMAT "%d"
#else
#define PA_SAMPLE_TYPE  paUInt8
typedef unsigned char SAMPLE;
#define SAMPLE_SILENCE  (128)
#define PRINTF_S_FORMAT "%d"
#endif

typedef struct
{
    int      index;  /* Index into sample array. */
    int      maxIndex;
	int      finished;
	int      *recording_l; /* Buffer for recording from device */
	int      *recording_r;
    int      *recorded_l;  /* Buffer for flashing to file */
	int      *recorded_r;
}
paTestData;


static void swapBuff(paTestData* data)
{
	int* tmp;

	//printf("Swapping buffers\n");
	tmp = data->recording_l;
	data->recording_l = data->recorded_l;
	data->recorded_l = tmp;
	tmp = data->recording_r;
	data->recording_r = data->recorded_r;
	data->recorded_r = tmp;
}

#define TRUE 1
#define FALSE 0

/* This routine will be called by the PortAudio engine when audio is needed.
** It may be called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
static int recordCB( const void *inputBuffer, void *outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void *data )
{
    const SAMPLE *rptr = (const SAMPLE*)inputBuffer;
    int *wptr_r = &((paTestData*)data)->recording_r[((paTestData*)data)->index];
	int *wptr_l = &((paTestData*)data)->recording_l[((paTestData*)data)->index];
	int add;
    int left;
	int i;

    (void) outputBuffer;
    (void) timeInfo;
    (void) statusFlags;

	left = ((paTestData*)data)->maxIndex - ((paTestData*)data)->index;
	//printf("left=%d, max=%d, index=%d\n", framesLeft, ((paTestData*)data)->maxIndex, ((paTestData*)data)->index);

	if (left < (int)framesPerBuffer) {
		add = left;
	}
	else {
		add = framesPerBuffer;
	}

    if( inputBuffer != NULL ) {
        for( i=0; i < add; i++ ) {
			*wptr_l++ = *rptr++; 
            *wptr_r++ = *rptr++; 
        }
    }
	((paTestData*)data)->index += add;
	if (((paTestData*)data)->index >= ((paTestData*)data)->maxIndex) {
		if (((paTestData*)data)->finished == TRUE) {
			fprintf(stderr, "Buffer overflow\n");
			return paContinue; //  paComplete;
		}
		swapBuff((paTestData*)data);
		if (inputBuffer != NULL) {
			for (i = 0; i < ((int)framesPerBuffer - left); i++) {
				*wptr_l++ = *rptr++;
				*wptr_r++ = *rptr++;
			}
		}
		((paTestData*)data)->finished = TRUE;
		((paTestData*)data)->index = 0;
	}
    return paContinue;
}

/*******************************************************************/
#define SIZEOF_INTSTR 12
#define BUFF_SIZE (SAMPLE_RATE / NUM_SPLIT)*SIZEOF_INTSTR
static char buff[BUFF_SIZE];

int main(void)
{
	PaStreamParameters  inputParameters;
	PaStream* stream;
	PaError             err = paNoError;
	paTestData          data;
	int                 i;
	int len;

	data.maxIndex = SAMPLE_RATE / NUM_SPLIT;
	data.index = 0;
	data.finished = FALSE;
	data.recording_l = (int*)malloc(data.maxIndex * sizeof(double));
	data.recording_r = (int*)malloc(data.maxIndex * sizeof(double));
	data.recorded_l  = (int*)malloc(data.maxIndex * sizeof(double));
	data.recorded_r  = (int*)malloc(data.maxIndex * sizeof(double));
	if ((data.recording_r == NULL) ||
		(data.recording_l == NULL) ||
		(data.recorded_r == NULL) ||
		(data.recorded_l == NULL))
	{
		printf("Could not allocate record array.\n");
		goto done;
	}

	for (i = 0; i < data.maxIndex; i++) {
		data.recording_r[i] = 0;
		data.recording_l[i] = 0;
		data.recorded_r[i] = 0;
		data.recorded_l[i] = 0;
	}

	err = Pa_Initialize();
	if (err != paNoError) goto done;

	inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
	if (inputParameters.device == paNoDevice) {
		fprintf(stderr, "Error: No default input device.\n");
		goto done;
	}
	inputParameters.channelCount = 2; /* stereo input */
	inputParameters.sampleFormat = PA_SAMPLE_TYPE;
	inputParameters.hostApiSpecificStreamInfo = NULL;
	inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;

	/* Record audio input */
	err = Pa_OpenStream(
		&stream,
		&inputParameters,
		NULL,    /* &outputParameters, */
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,
		paClipOff,
		recordCB,
		&data);
	if (err != paNoError) goto done;

	err = Pa_StartStream(stream);
	if (err != paNoError) goto done;

	fprintf(stderr, "\n=== Now recording ===\n"); fflush(stderr);

	while (1)
	{
		err = Pa_IsStreamActive(stream);
		if (err != paComplete) {
			printf("Pa_IsStreamActive: %d\n", err);
			goto done;
		}
		if (data.finished == TRUE) {
			//printf("index = %d, err = %d\n", data.frameIndex, err); fflush(stdout);
			for (i = 0, len = 0; i < data.maxIndex; i++) {
				//len += sprintf(&buff[len], PRINTF_S_FORMAT "\n", data.recorded_r[i]);
				len += strlen(itoa(data.recorded_r[i], &buff[len], 10));
				buff[len++] = ',';
			}
			buff[len-1] = '\n';
			fwrite(buff, 1, len, stdout);
			data.finished = FALSE;
		}
		Pa_Sleep(1);
	}

	err = Pa_CloseStream(stream);
	if (err != paNoError) goto done;

	Pa_Terminate();

	if (data.recording_l) free(data.recording_l);
	if (data.recording_r) free(data.recording_r);
	if (data.recorded_l) free(data.recorded_l);
	if (data.recorded_r) free(data.recorded_r);

done:
	fprintf(stderr, "ERROR: ENDING\n");
    return err;
}

