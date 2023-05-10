#include <stdint.h>

#define SMPRATE 48000 // 48 KHz sample rate
#define SMPNUM   1024 // 
#define SMPCHAN     2 // stereo

typedef struct Sound {
	float *data;
	unsigned bufsize;
} Sound;


#ifdef __linux__

#include <alsa/asoundlib.h>

snd_pcm_t *pcm;

void InitSnd() {

	void *params;

	snd_pcm_open(&pcm, "default", SND_PCM_STREAM_CAPTURE, 0);
	snd_pcm_hw_params_malloc((snd_pcm_hw_params_t **)&params);
	snd_pcm_hw_params_any(pcm, params);
	snd_pcm_hw_params_set_access(pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	snd_pcm_hw_params_set_format(pcm, params, SND_PCM_FORMAT_FLOAT);
	snd_pcm_hw_params_set_rate(pcm, params, SMPRATE, 0);
	snd_pcm_hw_params_set_channels(pcm, params, SMPCHAN);
	snd_pcm_hw_params(pcm, params);
	snd_pcm_hw_params_free(params);
	snd_pcm_prepare(pcm);
}

Sound *NewSound(float length) {
	Sound *snd = malloc(sizeof(Sound));
	snd->bufsize = ((int)(length * SMPRATE)) * SMPCHAN * sizeof(float);
	snd->data = malloc(snd->bufsize);
	return snd;
}

int Rec(void *buf, long num) {
	
	snd_pcm_sframes_t sf = snd_pcm_readi(pcm, buf, num);

	return sf;
}

#endif // __linux__

