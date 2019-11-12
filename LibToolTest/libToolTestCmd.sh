#!/bin/bash

#header paths
AIRBORNE="/home/joezie/paparazzi/sw/airborne"
GENERATED_PATH="/home/joezie/paparazzi/sw/misc/attitude_reference/c_att_refs"
STD_PATH="/home/joezie/paparazzi/sw/include"

#original src code paths
NAV_ADDR="/home/joezie/paparazzi/sw/airborne/modules/nav/nav_gls.c"
GUIDANCE_ADDR="/home/joezie/paparazzi/sw/airborne/firmwares/rotorcraft/guidance/guidance_h_ref.c"


#LibToolTest for nav_gls.c:
#./LibToolTest/LibToolAnalysis ../mysrc/nav_gls.c -extra-arg=-I${AIRBORNE} -extra-arg=-I${GENERATED_PATH} -extra-arg=-include${STD}> output4.txt
#./LibToolTest/LibToolAnalysis ${NAV_ADDR} -extra-arg=-I${AIRBORNE} -extra-arg=-I${GENERATED_PATH} -extra-arg=-I${STD_PATH} > output4.txt
#./ctrlstick_dict ${NAV_ADDR} -I${AIRBORNE} -I${GENERATED_PATH} -I${STD_PATH} > output4.txt

#LibToolTest for guidance_h_ref.c:
#./LibToolTest/LibToolAnalysis ../mysrc/guidance_h_ref.c -I${AIRBORNE} -I${GENERATED_PATH} -I${STD_PATH}> output3.txt
./ctrlstick_dict ../mysrc/guidance_h_ref.c -I${AIRBORNE} -I${GENERATED_PATH} -I${STD_PATH}> output.txt



