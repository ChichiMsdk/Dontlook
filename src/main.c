#include "app.h"
#include <io.h>

int		WINDOW_WIDTH = 600;
int		WINDOW_HEIGHT = 400;
int		g_sending = 1;
int		g_retrieving = 1;
int		g_running = 1;

inst	g_inst = {};

void logExit(char *msg);

void 
init_sdl(void)
{
	/* is set for the capture device sample in set_capture_device */
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
	{ fprintf(stderr, "%s\n", SDL_GetError()); exit(1); }

	g_inst.window = SDL_CreateWindow("Audio_player", WINDOW_WIDTH, WINDOW_HEIGHT,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	if (g_inst.window == NULL)
	{ fprintf(stderr, "%s\n", SDL_GetError()); SDL_Quit(); exit(1); }

	g_inst.renderer = SDL_CreateRenderer(g_inst.window,NULL);
	if (g_inst.renderer == NULL)
		logExit("renderer failed to be created");
}

void
key_down(SDL_Keycode key)
{
	/* record_pressed(key); */
	switch (key)
	{
		case SDLK_ESCAPE:
			g_running = 0;
			break;
		case SDLK_UP:
			break;
		case SDLK_DOWN:
			break;
		case SDLK_LEFT:
			break;
		case SDLK_RIGHT:
			break;
	}
}

void
key_up(SDL_Keycode key, AudioData *a_data)
{
	char *filename = "audio2.wav";
	/* record_released(key); */
	switch (key)
	{
		case SDLK_l:
			g_sending = !g_sending;
			break;
		case SDLK_s:
			/*
			 * should be prompted to change the name of the file
			 * maybe a global? 
			 */
			break;
		case SDLK_r:
			{
				if (!SDL_AudioDevicePaused(g_inst.capture_id))
				{
					printf("Paused recording..\n");
					SDL_PauseAudioDevice(g_inst.capture_id);
					g_retrieving = 1;
				}
				else
				{
					printf("Resume recording!\n");
					SDL_ResumeAudioDevice(g_inst.capture_id);
					g_retrieving = 0;
				}
				break;
			}
		case SDLK_f:
			{
				if (!SDL_AudioDevicePaused(g_inst.capture_id))
				{
					printf("Stopped recording..\n");
					SDL_PauseAudioDevice(g_inst.capture_id);
				}
				printf("Clearing the stream!\n");
				SDL_ClearAudioStream(g_inst.stream);
				break;
			}
	}
}

void
Events(SDL_Event e, AudioData *a_data)
{
	while (SDL_PollEvent(&e) != 0)
	{
		switch (e.type)
		{
			case SDL_EVENT_QUIT:
				{
					g_running = 0;
					break;
				}
			case SDL_EVENT_MOUSE_BUTTON_UP:
				{
					/* button_check_released(get_mouse_state(), &g_inst.button); */
					break;
				}
			case SDL_EVENT_MOUSE_BUTTON_DOWN:
				{
					/* button_check_pressed(get_mouse_state(), &g_inst.button); */
					/* debug_mouse_state(get_mouse_state()); */
					break;
				}
			case SDL_EVENT_MOUSE_MOTION:
				{
					/* button_check_hover(get_mouse_state(), &g_inst.button); */
					break;
				}
			case SDL_EVENT_KEY_DOWN:
				{
					/* should I break here ? */
					key_down(e.key.keysym.sym);
					break;
				}
			case SDL_EVENT_KEY_UP:
				{
					key_up(e.key.keysym.sym, a_data);
					break;
				}
			case SDL_EVENT_MOUSE_WHEEL:
				return;
				/* mouse_wheel(e.wheel); */
				break;
		}
	}
}

#include <windows.h>

LARGE_INTEGER frequency;
LARGE_INTEGER start;
LARGE_INTEGER end;
double elapsedTime;

void
fn(float duration, int samples, void *buffer, SDL_AudioStream *stream)
{
    // Get the starting time
	size_t offset;
	static uint64_t count = 1;

    QueryPerformanceCounter(&end);
	offset = count * samples;

    elapsedTime = (double)(end.QuadPart - start.QuadPart) / frequency.QuadPart;
	elapsedTime *= 1000;
	if (elapsedTime >= duration)
	{
		void *tmp = (char *) buffer + offset;
		if (SDL_PutAudioStreamData(stream, tmp, samples) < 0)
		{
			fprintf(stderr, "Could not open audio: %s\n", SDL_GetError());
			SDL_free(tmp);
			SDL_Quit();
		}
		QueryPerformanceCounter(&start);
	}
}

void
print_wav_header(t_wav header)
{
	_write(1, "\n", 1);
	_write(1, (char *)header.riff, 4); _write(1, "|\n", 2);
	printf("flength: %d\n", header.flength);
	_write(1, (char *)header.wave, 4); _write(1, "|\n", 2);
	_write(1, (char *)header.fmt, 4); _write(1, "|\n", 2);
	printf("chunk_size: %d\n", header.chunk_size);
	printf("format_tag: %d\n", header.format_tag);
	printf("num_chans: %d\n", header.num_chans);
	printf("srate: %d\n", header.srate);
	printf("bytes_per_sec: %d\n", header.bytes_per_sec);
	printf("bytes_per_samp: %d\n", header.bytes_per_samp);
	printf("bits_per_samp: %d\n", header.bits_per_samp);
	_write(1, (char *)header.data, 4); _write(1, "|\n", 2);
	printf("dlength: %d\n", header.dlength);
}

AudioData
load_full_wav(const char *fpath)
{
	AudioData a_data = {};
	t_wav header = {};
	size_t real_size = 0;
	size_t total_size = 0;

	a_data.path = fpath;
	int error = 0;

	FILE *fd = fopen(fpath, "rb");
	if (fd == NULL)
		logExit("fopen failed");

	size_t count = fread(&header, sizeof(t_wav), 1, fd);
	if (count < 0)
		logExit("fread failed");
	a_data.header = header;
	print_wav_header(header);

	/* dont trust riff header since it can provide wrong size... */
	int offset = ftell(fd);
	fseek(fd, (offset * -1), SEEK_END);
	real_size = ftell(fd);
	fseek(fd, 0L, SEEK_END);
	total_size = ftell(fd);
	fseek(fd, offset, SEEK_SET);

	uint8_t *buffer = malloc(real_size);
	if (!buffer)
		logExit("malloc failed");

	count = fread(buffer, real_size, 1, fd);
	if (count < 0)
		logExit("fopen failed");

	size_t i = 0;

	/*
	 * idk what im doing honestly .. just saw a bunch of zero in HXD ..
	 * it works.. so .. ?..
	 * */
	while (buffer[i] == 0)
		i++;
	uint8_t *buffer2 = malloc(real_size - i);
	memcpy(buffer2, buffer+i, real_size - i);
	real_size -= i;

	free(buffer);
	buffer = buffer2;
	a_data.buffer = buffer;
	a_data.header.dlength = real_size;
	a_data.header.flength = total_size;

	fclose(fd);
	return a_data;
}

unsigned int array_of_shame[] = {
	SDL_AUDIO_U8, SDL_AUDIO_S8, SDL_AUDIO_S16, SDL_AUDIO_S32, SDL_AUDIO_F32};

int
main()
{
	init_sdl();
	SDL_AudioSpec wav_spec;
	Uint8 *wav_buffer = NULL;
	Uint32 wav_length;
	/* const char *audio_file = "EE_VictoryMusic.wav"; */
	const char *audio_file = "beethoven_third_movement_moonlight_sonata.wav";
    /*
	 * if (SDL_LoadWAV(audio_file, &wav_spec, &wav_buffer, &wav_length) == -1) {
	 * 	fprintf(stderr, "Could not open %s: %s\n", audio_file, SDL_GetError());
	 * 	SDL_Quit();
	 * 	return 1;
	 * }
     */

	SDL_free(wav_buffer);
	wav_buffer = NULL;
	AudioData data = load_full_wav(audio_file);
	wav_spec.freq = data.header.srate;
	wav_spec.channels = data.header.num_chans;
	wav_buffer = data.buffer;
	wav_length = data.header.dlength;
	wav_spec.format = SDL_AUDIO_S16; /* I guess it's the most common one ?.. */

	/* shame on me .. for not willing to parse that shit*/
	int i = 0;
	uint64_t idiot;
	while (1)
	{
		if (((idiot = wav_length % SDL_AUDIO_FRAMESIZE(wav_spec))) == 0)
		{
			printf("idiot: %u\n", wav_spec.format);
			printf("i: %d\n", i);
			if (wav_spec.format >= 3284)
				break;
		}
		if (i >= 5)
		{
			printf("LOSER AHAHA\n");
			exit(1);
		}
		wav_spec.format = array_of_shame[i];
		printf("HAHAHAHHAAH\n");
		i++;
	}
	/*
	 * either this works and I am genius, or it just miserably fails and either
	 * you get a clean superb lovely abort or ... an awful sound that will just
	 * destroy your ears .. so keep the volume low, very low.
	 */

	printf("format %d\n", wav_spec.format);

	printf("%d\n", wav_length);
	SDL_AudioStream *audio_stream = SDL_CreateAudioStream(&wav_spec, &wav_spec);
	if (audio_stream == NULL) 
	{
		fprintf(stderr, "Could not create audio stream: %s\n", SDL_GetError());
		SDL_free(wav_buffer);
		SDL_Quit();
		return 1;
	}
	if (SDL_PutAudioStreamData(audio_stream, wav_buffer, wav_length) < 0)
	{
		fprintf(stderr, "Error putting data to audio stream: %s\n", SDL_GetError());
		SDL_free(wav_buffer);
		SDL_Quit();
		return 1;
	}
	SDL_AudioDeviceID devid;
	if ((devid = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_OUTPUT, NULL)) < 0)
	{
		fprintf(stderr, "Could not open audio: %s\n", SDL_GetError());
		SDL_free(wav_buffer);
		SDL_Quit();
		return 1;
	}
	SDL_AudioSpec dev_spec;
	int	sample_frames;
	SDL_GetAudioDeviceFormat(devid, &dev_spec, &sample_frames);
	printf("dev_freq %d\n sample %d\n", dev_spec.freq, sample_frames);

	SDL_GetAudioStreamFormat(audio_stream, &dev_spec, &wav_spec);
	printf("%d\n %d\n", dev_spec.freq, wav_spec.freq);

	SDL_BindAudioStream(devid, audio_stream);

	SDL_PauseAudioDevice(devid);

	size_t bytes = SDL_GetAudioStreamAvailable(audio_stream);
	printf("bytes: %llu\n", bytes);

	g_inst.capture_id = devid;
	g_inst.stream = audio_stream;

    QueryPerformanceFrequency(&frequency);
	float sample = 4096;
	float duration = (sample / wav_spec.freq) * 1000;
	printf("%f ms of time to process\n", duration);
	while(g_running == 1)
	{
		/* if (g_retrieving == 0) */
			/* fn(duration, sample, wav_buffer, audio_stream); */
		Events(g_inst.e, NULL);
	}

	SDL_CloseAudioDevice(devid);
	free(data.buffer);
	/* SDL_free(wav_buffer); */
	SDL_DestroyRenderer(g_inst.renderer);
	SDL_DestroyWindow(g_inst.window);
	SDL_Quit();
	return 0;
}

void
logExit(char *msg)
{
	fprintf(stderr, "%s: %s\n", msg, SDL_GetError());
	SDL_DestroyWindow(g_inst.window);
	SDL_Quit();
	exit(1);
}
