/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
// Relocated code to a separate file from 3D_ACT2.C,
// since these are a part of 3D_AGENT.C  in AOG v3.0.
//
// - Should be #include'd from the right location.

//==========================================================================
//
//								WALL SWITCH ACTIVATION
//
//==========================================================================

//--------------------------------------------------------------------------
// ActivateWallSwitch() - Updates the Map, Actors, and Tables for wall switchs
//--------------------------------------------------------------------------
void ActivateWallSwitch(uint16_t iconnum, int16_t x, int16_t y)
{
	#define UPDATE_OTHER_SWITCHES			1

	uint16_t states[] = {OFF_SWITCH,ON_SWITCH};
	uint16_t	mapx,mapy,newwall;
	uint16_t icon,num;
   uint8_t *tile;
   intptr_t *actor;
   barrier_type *barrier;


	if ((iconnum & 0xFF00) == 0xF800)
   {

		//
      // Update tile map & Display switch'd message
      //
      num = iconnum & 0xff;



      /*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
      barrier = GAMEVER_RESTORATION_BARRIER_PTR(num/16,num);
#else
      barrier = &gamestate.barrier_table[num];
#endif
      barrier->on ^= 1;
      newwall = tilemap[x][y] = states[barrier->on];
	  tilemap[x][y] = (uint8_t)states[barrier->on];

      DisplaySwitchOperateMsg(num);
      PlaySoundLocActor(SWITCHSND,player);

#if UPDATE_OTHER_SWITCHES

		tile = (uint8_t *)tilemap;
      actor = (intptr_t *)actorat;

		for (mapx=0;mapx<MAPSIZE;mapx++)
			for (mapy=0;mapy<MAPSIZE;mapy++)
			{
      		if (*tile == OFF_SWITCH || *tile == ON_SWITCH)
            {
            	icon = *(mapsegs[1]+farmapylookup[mapy]+mapx);

            	if (icon == iconnum)
               {
               	*tile = (uint8_t)newwall;
                  *actor = newwall;
               }
            }

            tile++;
            actor++;
      	}
#endif

   }
   else
   {
		DISPLAY_TIMED_MSG(SwitchNotActivateMsg,MP_WALLSWITCH_OPERATE,MT_GENERAL);
      SD_PlaySound(NOWAYSND);
   }
}

//--------------------------------------------------------------------------
// DisplaySwitchOperateMsg() - Displays the Operating Barrier Switch message
//										 for a particular level across the InfoArea.
//--------------------------------------------------------------------------
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
char OnSwitchMessage[] =  "\r\r  ACTIVATING BARRIER\r      ON FLOOR   ";

char OffSwitchMessage[] = "\r\r DEACTIVATING BARRIER\r      ON FLOOR   ";
#else
char OnSwitchMessage[] =  "\r\r  ACTIVATING BARRIER";

char OffSwitchMessage[] = "\r\r DEACTIVATING BARRIER";
#endif


void DisplaySwitchOperateMsg(uint16_t coords)
{
   barrier_type *Barrier;

	/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
	byte floor = (coords/16)&15;
	Barrier = GAMEVER_RESTORATION_BARRIER_PTR(floor,coords);
#else
	Barrier = &gamestate.barrier_table[coords];
#endif

  	if (Barrier->on)
   {
		/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
		itoa(floor,&OnSwitchMessage[38],10);
#endif
		DISPLAY_TIMED_MSG(OnSwitchMessage,MP_WALLSWITCH_OPERATE,MT_GENERAL);
   }
	else
   {
		/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
		itoa(floor,&OffSwitchMessage[39],10);
#endif
		DISPLAY_TIMED_MSG(OffSwitchMessage,MP_WALLSWITCH_OPERATE,MT_GENERAL);
   }
}


//--------------------------------------------------------------------------
// UpdateBarrierTable(x,y,level) - Finds/Inserts arc entry in arc list
//
// RETURNS: Offset into barrier_table[] for a particular arc
//
//--------------------------------------------------------------------------
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
uint16_t UpdateBarrierTable(uint8_t x, uint8_t y, uint8_t level)
#else
uint16_t UpdateBarrierTable(uint8_t x, uint8_t y, boolean OnOff)
#endif
{
   barrier_type *Barrier;
   int16_t num;
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG

   if (level == 0xFF)
      level = gamestate.mapon;
   if (level > MAPS_PER_EPISODE)
      AGENT_ERROR(UPDATE_SWITCH_BAD_LEVEL);
#endif

	//
   // Scan Table...
   //

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
  	Barrier = &gamestate.barrier_table[MAX_BARRIER_SWITCHES*level];
#else
  	Barrier = gamestate.barrier_table;
#endif

   for (num = 0;num < MAX_BARRIER_SWITCHES;num++,Barrier++)
   {
   	if (Barrier->coord.tilex == x && Barrier->coord.tiley == y)
      {
         /*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
         return GAMEVER_RESTORATION_BARRIER_CODE(level,num);
#else
         return (num);
#endif
      }
      else
   	if (Barrier->on == 0xff)				// Empty?
      {
      	// We have hit end of list - Add

  	      Barrier->coord.tilex = x;
     	   Barrier->coord.tiley = y;
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
         Barrier->on = 1;
         return GAMEVER_RESTORATION_BARRIER_CODE(level,num);
#else
         Barrier->on = OnOff;
         return (num);
#endif
      }
   }

  	AGENT_ERROR(SWITCH_TABLE_OVERFLOW);
	return(0);
}



//--------------------------------------------------------------------------
// ScanBarrierTable(x,y) - Scans a switch table for a arc in this level
//
// RETURNS :
//	   0xFFFF    - Not found in table
//		barrier   - barrier_table of the barrier for [num]
//
//--------------------------------------------------------------------------
uint16_t ScanBarrierTable(uint8_t x, uint8_t y)
{
   barrier_type *Barrier;
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
   int16_t level = gamestate.mapon;
#endif
   uint16_t num;

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
   Barrier = &gamestate.barrier_table[MAX_BARRIER_SWITCHES*level];
#else
   Barrier = gamestate.barrier_table;
#endif

   for (num=0;num<MAX_BARRIER_SWITCHES;num++,Barrier++)
   {
      if (Barrier->coord.tilex == x && Barrier->coord.tiley == y)
      {
      	// Found switch...

         /*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
         return GAMEVER_RESTORATION_BARRIER_CODE(level,num);
#else
         return (num);
#endif
		}
	}

	return(0xffff);		// Mark as EMPTY
}


//--------------------------------------------------------------------------
// Checks to see if the Barrier obj is free
//--------------------------------------------------------------------------
boolean CheckActor(objtype *actor,uint16_t code)
{
		if ((uint16_t)actor->temp2 == 0xffff)		// Is this actor free?
		{
      	//
         // Connect actor to barrier switch (code is index into barrier table)
         //

			actor->temp2 = code;		// This actor is NO longer a cycle actor.
			return(true);
		}

	return(false);
}


//--------------------------------------------------------------------------
// CheckAndConnect() -
//--------------------------------------------------------------------------
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
void CheckAndConnect(int8_t x,int8_t y, uint16_t code)
#else
int16_t CheckAndConnect(int8_t x,int8_t y, uint16_t code)
#endif
{
	objtype *ob;
	int8_t offsets[] = {-1,0,1,0};
	int16_t loop;

	ob = objlist;

	do
	{
		switch (ob->obclass)
		{
			case arc_barrierobj:
			case post_barrierobj:
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
			case vpost_barrierobj:
			case vspike_barrierobj:
#endif
			{
				for (loop = 0;loop<4;loop++)
				{
					if ((ob->tilex == x+offsets[loop]) && (ob->tiley == y+offsets[3-loop]))
					{
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
		         	bars_connected++;
#endif

						if (CheckActor(ob,code))
						   CheckAndConnect(x+offsets[loop],y+offsets[3-loop],code);
					}
				}
			}
			break;
		}
//#pragma warn -pia
	} while ((ob = ob->next));
//#pragma warn +pia

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
	return(bars_connected);
#endif
}

//--------------------------------------------------------------------------
// ConnectBarriers() - Scans the object list and finds the single barrier
//                     that is connected by switch and checks to see if
//							  there are any surrounding barriers that need to be
//							  connected also.
//--------------------------------------------------------------------------
void ConnectBarriers(void)
{
   barrier_type *Barrier;
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
   int16_t level = gamestate.mapon;
#endif
	uint16_t num;

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
   Barrier = &gamestate.barrier_table[MAX_BARRIER_SWITCHES*level];
#else
   Barrier = gamestate.barrier_table;
#endif

   for (num=0;num<MAX_BARRIER_SWITCHES;num++,Barrier++)
   {
      if (Barrier->on != 0xff)
      {
/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifndef GAMEVER_RESTORATION_AOG
		   bars_connected = 0;
#endif

/*** BLAKE STONE: ALIENS OF GOLD RESTORATION ***/
#ifdef GAMEVER_RESTORATION_AOG
		CheckAndConnect(Barrier->coord.tilex, Barrier->coord.tiley, GAMEVER_RESTORATION_BARRIER_CODE(level,num));
#else
			if (!CheckAndConnect(Barrier->coord.tilex, Barrier->coord.tiley, num))
         	AGENT_ERROR(BARRIER_SWITCH_NOT_CONNECTED);
#endif
      }
   }
}


