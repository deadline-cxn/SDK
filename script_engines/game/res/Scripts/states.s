
//
// States
//

BEGIN STATEMAP
  NAME "Babe"

#STATE 1
	LOG "Entered state #1"
	GOTO "Entre" (60,70)
	PLAYANIM "Bow" WAIT
	FOLLOW "Hero"
	SWITCHSTATE 0

END STATEMAP
