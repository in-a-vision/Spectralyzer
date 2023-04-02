#include <stdint.h>

float *sndbuf;
uint32_t bufsize, numsamples, rate, chan;

#ifdef __linux__

#include <alsa/asoundlib.h>

snd_pcm_t *pcm;
snd_pcm_format_t format;
void *params;

void InitSnd() {

	numsamples = 1000;
	chan = 2;
	format = SND_PCM_FORMAT_FLOAT;
	rate = 48000;
	bufsize = numsamples * chan * sizeof(float);
	sndbuf = malloc(bufsize);

	snd_pcm_open(&pcm, "default", SND_PCM_STREAM_CAPTURE, 0);
	snd_pcm_hw_params_malloc((snd_pcm_hw_params_t **)&params);
	snd_pcm_hw_params_any(pcm, params);
	snd_pcm_hw_params_set_access(pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(pcm, params, format);
	snd_pcm_hw_params_set_rate(pcm, params, rate, 0);
	snd_pcm_hw_params_set_channels(pcm, params, chan);
	snd_pcm_hw_params(pcm, params);
	snd_pcm_hw_params_free(params);
	snd_pcm_prepare(pcm);
}

int Rec() {
	snd_pcm_sframes_t sf = snd_pcm_readi(pcm, sndbuf, numsamples);

	float *f=sndbuf, max = .0f;
	for(int i=0; i<sf; i++)
		if(*f > max) max = *f;

	printf("\r%.4f", max); fflush(stdout);


	return sf;
}

#endif // __linux__

