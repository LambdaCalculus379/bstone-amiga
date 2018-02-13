/*** BLAKE STONE VERSIONS RESTORATION ***/
#if (defined GAMEVER_RESTORATION_BS1_300) || (defined GAMEVER_RESTORATION_BS6_300)
#define GAMEVER_RESTORATION_AOG_300
#define GAMEVER_RESTORATION_AOG
#elif (defined GAMEVER_RESTORATION_BS1_210) || (defined GAMEVER_RESTORATION_BS6_210)
#define GAMEVER_RESTORATION_AOG_210
#define GAMEVER_RESTORATION_AOG
#elif (defined GAMEVER_RESTORATION_BS1_200) || (defined GAMEVER_RESTORATION_BS6_200)
#define GAMEVER_RESTORATION_AOG_200
#define GAMEVER_RESTORATION_AOG
#elif (defined GAMEVER_RESTORATION_BS1_100) || (defined GAMEVER_RESTORATION_BS6_100)
#define GAMEVER_RESTORATION_AOG_100
#define GAMEVER_RESTORATION_AOG
#elif (defined GAMEVER_RESTORATION_VSI_100) || (defined GAMEVER_RESTORATION_VSI_101)
#define GAMEVER_RESTORATION_VSI
#endif

#if (defined GAMEVER_RESTORATION_BS1_100) || (defined GAMEVER_RESTORATION_BS1_200) || (defined GAMEVER_RESTORATION_BS1_210) || (defined GAMEVER_RESTORATION_BS1_300)
#define GAMEVER_RESTORATION_BS1
#endif

#if (defined GAMEVER_RESTORATION_BS6_100) || (defined GAMEVER_RESTORATION_BS6_200) || (defined GAMEVER_RESTORATION_BS6_210) || (defined GAMEVER_RESTORATION_BS6_300)
#define GAMEVER_RESTORATION_BS6
#endif

#if (defined GAMEVER_RESTORATION_AOG_100) || (defined GAMEVER_RESTORATION_AOG_200)
#define GAMEVER_RESTORATION_AOG_PRE210
#endif

#if (defined GAMEVER_RESTORATION_AOG_210) || (defined GAMEVER_RESTORATION_AOG_300)
#define GAMEVER_RESTORATION_AOG_POST200
#endif

#if (defined GAMEVER_RESTORATION_AOG_200) || (defined GAMEVER_RESTORATION_AOG_POST200)
#define GAMEVER_RESTORATION_AOG_POST100
#endif

#if (defined GAMEVER_RESTORATION_AOG_300) || (defined GAMEVER_RESTORATION_VSI_101)
#define GAMEVER_RESTORATION_ANY_LATE
#endif

// Hack: Replacements for __DATE__ and __TIME__ marcos (basically cheating here)
#ifdef GAMEVER_RESTORATION_BS1_100
#define GAMEVER_RESTORATION_DATE "Nov 01 1993"
#define GAMEVER_RESTORATION_TIME "10:52:52"
#elif defined GAMEVER_RESTORATION_BS6_100
#define GAMEVER_RESTORATION_DATE "Dec 01 1993"
#define GAMEVER_RESTORATION_TIME "14:58:23"
#elif defined GAMEVER_RESTORATION_BS1_200
#define GAMEVER_RESTORATION_DATE "Feb 06 1994"
#define GAMEVER_RESTORATION_TIME "14:56:29"
#elif defined GAMEVER_RESTORATION_BS6_200
#define GAMEVER_RESTORATION_DATE "Feb 06 1994"
#define GAMEVER_RESTORATION_TIME "15:13:38"
#elif defined GAMEVER_RESTORATION_BS1_210
#define GAMEVER_RESTORATION_DATE "Jun 13 1994"
#define GAMEVER_RESTORATION_TIME "14:48:05"
#elif defined GAMEVER_RESTORATION_BS6_210
#define GAMEVER_RESTORATION_DATE "Jun 13 1994"
#define GAMEVER_RESTORATION_TIME "15:00:37"
#elif defined GAMEVER_RESTORATION_VSI_100
#define GAMEVER_RESTORATION_DATE "Sep 07 1994"
#define GAMEVER_RESTORATION_TIME "15:15:24"
#elif defined GAMEVER_RESTORATION_VSI_101
#define GAMEVER_RESTORATION_DATE "Oct 24 1994"
#define GAMEVER_RESTORATION_TIME "18:20:35"
#elif defined GAMEVER_RESTORATION_BS1_300
#define GAMEVER_RESTORATION_DATE "Oct 31 1994"
#define GAMEVER_RESTORATION_TIME "11:33:04"
#elif defined GAMEVER_RESTORATION_BS6_300
#define GAMEVER_RESTORATION_DATE "Oct 31 1994"
#define GAMEVER_RESTORATION_TIME "18:03:26"
#endif
