#!/bin/bash

# Be sure that this file has execution permissions:
# Use the nautilus explorer or chmod +x run_vad.sh

# Se pueden pasar opciones al vad para probar parametros, por ejemplo:
#   scripts/run_vad.sh -1 4 -2 6
# Si no se pasa nada se usan los valores por defecto del programa.
OPCIONES="$@"

# Establecemos que el código de retorno de un pipeline sea el del último programa con código de retorno
# distinto de cero, o cero si todos devuelven cero.
set -o pipefail

# Write here the name and path of your program and database
DIR_P2=$HOME/PAV/P2
DB=$DIR_P2/db.v4
CMD="$DIR_P2/bin/vad $OPCIONES"

for filewav in $DB/*/*wav; do
#    echo
    echo "**************** $filewav ****************"
    if [[ ! -f $filewav ]]; then 
	    echo "Wav file not found: $filewav" >&2
	    exit 1
    fi

    filevad=${filewav/.wav/.vad}

    $CMD -i $filewav -o $filevad || exit 1

# Alternatively, uncomment to create output wave files
#    filewavOut=${filewav/.wav/.vad.wav}
#    $CMD -i $filewav -o $filevad -w $filewavOut || exit 1

done

$DIR_P2/scripts/vad_evaluation.pl $DB/*/*lab

exit 0
