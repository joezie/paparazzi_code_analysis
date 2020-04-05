TARGET_AP_LIST="Bixler Bumblebee_Quad Hexa_LisaL LadyLisa MentorEnergy Minion_Lia Quad_Elle0 Quad_LisaMX Quad_LisaM_2 Quad_Navstik ardrone2 bebop2 bebop2_opticflow krooz_quad quadshot Twinstar_energyadaptive Microjet Umarim_Lite"

#TARGET_AP_LIST="Bixler" # this is for single target test

TARGET_SIM_LIST="Microjet Twinstar_energyadaptive Umarim_Lite Bixler MentorEnergy"

TARGET_NPS_LIST="Bixler Bumblebee_Quad LadyLisa MentorEnergy Minion_Lia Quad_Elle0 Quad_LisaMX Quad_LisaM_2 Quad_Navstik ardrone2 bebop2 krooz_quad"

# CAUTION: change this to your paparazzi directory path!
PAPARAZZI_DIR="/home/joezie/paparazzi"

#Compile all autopilot targets
for ac in $TARGET_AP_LIST; do
	make -C $PAPARAZZI_DIR -f Makefile.ac PAPARAZZI_HOME=$PAPARAZZI_DIR PAPARAZZI_SRC=$PAPARAZZI_DIR AIRCRAFT=$ac ap.compile 
done

#Clean all autopilots targets
for ac in $TARGET_AP_LIST; do
	make -C $PAPARAZZI_DIR -f Makefile.ac AIRCRAFT=$ac clean_ac
done

#Compile all sim targets
#for ac in $TARGET_SIM_LIST; do
#	make -C $PAPARAZZI_DIR -f Makefile.ac PAPARAZZI_HOME=$PAPARAZZI_DIR PAPARAZZI_SRC=$PAPARAZZI_DIR AIRCRAFT=$ac nps.ARCHDIR=sim sim.compile 
#done

#Clean all sim targets
#for ac in $TARGET_SIM_LIST; do
#	make -C $PAPARAZZI_DIR -f Makefile.ac AIRCRAFT=$ac clean_ac
#done

#Compile all nps targets
#for ac in $TARGET_NPS_LIST; do
#	make -C $PAPARAZZI_DIR -f Makefile.ac PAPARAZZI_HOME=$PAPARAZZI_DIR PAPARAZZI_SRC=$PAPARAZZI_DIR AIRCRAFT=$ac nps.ARCHDIR=sim nps.compile 
#done

#Clean all nps targets
#for ac in $TARGET_NPS_LIST; do
#	make -C $PAPARAZZI_DIR -f Makefile.ac AIRCRAFT=$ac clean_ac
#done