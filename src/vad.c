#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "vad.h"
#include "pav_analysis.h"

const float FRAME_TIME = 10.0F; /* in ms. */

/* 
 * As the output state is only ST_VOICE, ST_SILENCE, or ST_UNDEF,
 * only this labels are needed. You need to add all labels, in case
 * you want to print the internal state in string format
 */

const char *state_str[] = {
  "UNDEF", "S", "V", "INIT", "MS", "MV"
};

const char *state2str(VAD_STATE st) {
  return state_str[st];
}

/* Define a datatype with interesting features */
typedef struct {
  float zcr;
  float p;
  float am;
} Features;

/* Calculamos las caracteristicas con las funciones de la practica 1 */
Features compute_features(const float *x, int N, float fm) {
  Features feat;
  feat.p = compute_power(x, N);
  feat.am = compute_am(x, N);
  feat.zcr = compute_zcr(x, N, fm);
  return feat;
}

VAD_DATA * vad_open(float rate, float alfa1, float alfa2,
                    int min_silence, int min_voice, int n_init) {
  VAD_DATA *vad_data = malloc(sizeof(VAD_DATA));
  vad_data->state = ST_INIT;
  vad_data->sampling_rate = rate;
  vad_data->frame_length = rate * FRAME_TIME * 1e-3;
  vad_data->last_feature = 0;

  vad_data->alfa1 = alfa1;
  vad_data->alfa2 = alfa2;
  vad_data->min_silence = min_silence;
  vad_data->min_voice = min_voice;
  vad_data->n_init = n_init;

  vad_data->k0 = 0;
  vad_data->k1 = 0;
  vad_data->k2 = 0;
  vad_data->contador = 0;
  vad_data->suma_potencia = 0;
  return vad_data;
}

VAD_STATE vad_close(VAD_DATA *vad_data) {
  /* Si acabamos en un estado indeciso, nos quedamos con el estado de antes:
     maybe silence todavia era voz y maybe voice todavia era silencio */
  VAD_STATE state = vad_data->state;

  if (state == ST_MAYBE_SILENCE)
    state = ST_VOICE;
  else if (state == ST_MAYBE_VOICE || state == ST_INIT)
    state = ST_SILENCE;

  free(vad_data);
  return state;
}

unsigned int vad_frame_size(VAD_DATA *vad_data) {
  return vad_data->frame_length;
}

/* 
 * Automata de estados finitos:
 *  INIT: las primeras n_init tramas son silencio y se usan para calcular k0
 *  SILENCE: si la potencia pasa de k1 vamos a MAYBE_VOICE
 *  MAYBE_VOICE: si pasa de k2 es voz, si baja de k1 o tardamos mucho es silencio
 *  VOICE: si la potencia baja de k1 vamos a MAYBE_SILENCE
 *  MAYBE_SILENCE: si vuelve a subir de k1 es voz, si estamos min_silence
 *                 tramas por debajo es silencio
 */

VAD_STATE vad(VAD_DATA *vad_data, float *x) {

  Features f = compute_features(x, vad_data->frame_length, vad_data->sampling_rate);
  vad_data->last_feature = f.p; /* save feature, in case you want to show */

  switch (vad_data->state) {
  case ST_INIT:
    /* Sumamos la potencia en lineal (formula 1 del enunciado) */
    vad_data->suma_potencia = vad_data->suma_potencia + pow(10, f.p / 10);
    vad_data->contador++;
    if (vad_data->contador >= vad_data->n_init) {
      vad_data->k0 = 10 * log10(vad_data->suma_potencia / vad_data->contador);
      vad_data->k1 = vad_data->k0 + vad_data->alfa1;
      vad_data->k2 = vad_data->k1 + vad_data->alfa2;
      vad_data->state = ST_SILENCE;
    }
    /* las tramas del init las consideramos silencio */
    return ST_SILENCE;

  case ST_SILENCE:
    if (f.p > vad_data->k2) {
      vad_data->state = ST_VOICE;
    }
    else if (f.p > vad_data->k1) {
      vad_data->state = ST_MAYBE_VOICE;
      vad_data->contador = 1;
    }
    break;

  case ST_MAYBE_VOICE:
    if (f.p > vad_data->k2) {
      vad_data->state = ST_VOICE;
    }
    else if (f.p < vad_data->k1) {
      vad_data->state = ST_SILENCE;
    }
    else {
      vad_data->contador++;
      /* si no llegamos a k2 en min_voice tramas era ruido */
      if (vad_data->contador >= vad_data->min_voice)
        vad_data->state = ST_SILENCE;
    }
    break;

  case ST_VOICE:
    if (f.p < vad_data->k1) {
      vad_data->state = ST_MAYBE_SILENCE;
      vad_data->contador = 1;
    }
    break;

  case ST_MAYBE_SILENCE:
    if (f.p > vad_data->k1) {
      vad_data->state = ST_VOICE;
    }
    else {
      vad_data->contador++;
      if (vad_data->contador >= vad_data->min_silence)
        vad_data->state = ST_SILENCE;
    }
    break;

  case ST_UNDEF:
    break;
  }

  if (vad_data->state == ST_SILENCE ||
      vad_data->state == ST_VOICE)
    return vad_data->state;
  else
    return ST_UNDEF;
}

void vad_show_state(const VAD_DATA *vad_data, FILE *out) {
  fprintf(out, "%s\t%f\n", state2str(vad_data->state), vad_data->last_feature);
}
