#ifndef _VAD_H
#define _VAD_H
#include <stdio.h>

/* Estados del automata. MAYBE_SILENCE y MAYBE_VOICE son los estados
   "indecisos": vad() devuelve ST_UNDEF mientras estamos en ellos */
typedef enum {ST_UNDEF=0, ST_SILENCE, ST_VOICE, ST_INIT, ST_MAYBE_SILENCE, ST_MAYBE_VOICE} VAD_STATE;

/* Return a string label associated to each state */
const char *state2str(VAD_STATE st);

/* Variables para controlar el VAD */
typedef struct {
  VAD_STATE state;
  float sampling_rate;
  unsigned int frame_length;
  float last_feature; /* for debuggin purposes */

  float k0;           /* nivel del ruido de fondo (dB) */
  float k1;           /* umbral para pasar a maybe voice (dB) */
  float k2;           /* umbral para confirmar la voz (dB) */
  float alfa1;        /* k1 = k0 + alfa1 */
  float alfa2;        /* k2 = k1 + alfa2 */

  int n_init;         /* numero de tramas que se usan para calcular k0 */
  int min_silence;    /* tramas que hay que estar en maybe silence para que sea silencio */
  int min_voice;      /* tramas maximas en maybe voice esperando a llegar a k2 */

  int contador;       /* cuenta tramas en el estado actual (init, maybe...) */
  float suma_potencia; /* para hacer la media de la potencia en el init */
} VAD_DATA;

/* Call this function before using VAD: 
   It should return allocated and initialized values of vad_data

   sampling_rate: ... the sampling rate
   el resto son los parametros del automata */
VAD_DATA *vad_open(float sampling_rate, float alfa1, float alfa2,
                   int min_silence, int min_voice, int n_init);

/* vad works frame by frame.
   This function returns the frame size so that the program knows how
   many samples have to be provided */
unsigned int vad_frame_size(VAD_DATA *);

/* Main function. For each 'time', compute the new state 
   It returns:
    ST_UNDEF   (0) : undefined; it needs more frames to take decission
    ST_SILENCE (1) : silence
    ST_VOICE   (2) : voice

    x: input frame
       It is assumed the length is frame_length */
VAD_STATE vad(VAD_DATA *vad_data, float *x);

/* Free memory
   Returns the state of the last (undecided) states. */
VAD_STATE vad_close(VAD_DATA *vad_data);

/* Print actual state of vad, for debug purposes */
void vad_show_state(const VAD_DATA *, FILE *);

#endif
