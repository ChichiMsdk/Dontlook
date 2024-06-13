#ifndef APP_H
#define APP_H

#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN 
#define SDL_MAIN_HANDLED
#include <SDL3/SDL.h>

/* mandatory to launch app without console */
#include <SDL3/SDL_main.h>

#include <stdio.h>
#include <stdbool.h>

typedef enum 
{
	OUTPUT = 0,
	CAPTURE = 1
}DeviceType;

typedef struct wav_header
{
  char					riff[4];           /* "RIFF"                                  */
  uint32_t				flength;           /* file length in bytes                    */
  char					wave[4];           /* "WAVE"                                  */
  char					fmt[4];            /* "fmt "                                  */
  uint32_t				chunk_size;        /* size of FMT chunk in bytes (usually 16) */
  uint16_t				format_tag;        /* 1=PCM, 257=Mu-Law, 258=A-Law, 259=ADPCM */
  uint16_t				num_chans;         /* 1=mono, 2=stereo                        */
  uint32_t				srate;             /* Sampling rate in samples per second     */
  uint32_t				bytes_per_sec;     /* bytes per second = srate*bytes_per_samp */
  uint16_t				bytes_per_samp;    /* 2=16-bit mono, 4=16-bit stereo          */
  uint16_t				bits_per_samp;     /* Number of bits per sample               */
  char					data[4];           /* "data"                                  */
  uint32_t				dlength;           /* data length in bytes (filelength - 44)  */
}t_wav;

typedef struct LogicalDevice
{
	SDL_AudioDeviceID	logical_id;
	SDL_AudioDeviceID	physical_id;
	DeviceType			type;
	int					sample;
	SDL_AudioSpec		spec;
	SDL_AudioStream 	*stream;
	const char			*name; /* not guaranteed to get the device */
    /*
	 * void				*buffer;
	 * size_t				current_buff_size;
     */
}LogicalDevice;

typedef struct AudioData
{
	SDL_AudioSpec		spec;
	const char			*path;
    Uint8				*buffer;
    Uint32				length;
    Uint32				position;
	SDL_AudioStream 	*stream;
	int					sample_size;
	size_t				current_buff_size;
	t_wav				header;
	bool				paused;

	int					index;
	Uint8				is_pressed;
	SDL_AudioDeviceID	capture_id;
	SDL_AudioDeviceID	out_id;
} AudioData;

typedef struct inst
{
	SDL_Window			*window;
	SDL_Renderer		*renderer;
	SDL_Rect			rect;
	SDL_Event			e;
	SDL_Texture			*texture;

	SDL_AudioStream 	*stream;
	SDL_AudioDeviceID	capture_id;
	SDL_AudioDeviceID	out_id;

	FILE				*audio_file;
	char				*capture_name;
	char				*output_name;
	int					sample_size;
	size_t				current_buff_size;

	SDL_Cursor			*cursor;
}inst;


#endif
