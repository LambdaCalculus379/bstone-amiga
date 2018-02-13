#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
//#include <findfirst.h>
#include "findfirst.c"


typedef enum
{
	MACRO_IF,
	MACRO_ELSE,
	MACRO_END,
	MACRO_NONE
} macrostate_t;

typedef enum
{
	STATE_TYPE,
	STATE_VARIABLE,
	STATE_NONE
} state_t;

typedef enum
{
	STAGE_EXTERN,
	STAGE_ARRAY,
	STAGE_DONE
} stage_t;

#define TYPE_NAME "statetype"
const char cset[] = "_abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
const char linebreak[] = {'\n','\r','\0'};
char line[1024];

int main(void)
{
	struct ffblk f;
	char str[256];
	char macro[256];
	FILE *in, *out;

	out = fopen("statlist.c", "wt");
	if (!out)
		return 1;

	//fprintf(out, "#include \"3d_def.h\"\n\n");

	stage_t stage = STAGE_EXTERN;

	while (stage < STAGE_DONE)
	{
		printf("starting stage %d\n", stage);
		if (stage == STAGE_ARRAY)
		{
			fprintf(out, "%s *states_list[] =\n{\n", TYPE_NAME);
			fprintf(out, "\tNULL,\n");
		}

		if (!findfirst("*.c", &f, 0))
		{
			do
			{
				in = fopen(f.ff_name, "rt");
				if (in)
				{
					macrostate_t macrostate = MACRO_NONE;
					state_t state = STATE_NONE;
					bool filename = false;
					int macrocount[MACRO_NONE];
					memset(&macrocount, 0, sizeof(macrocount));
					int linenum = 0;
					//int macroendline = 0;
					int macrostarted = 0;

					//printf("parsing %s...\n", f.ff_name);
					while (fgets(line,sizeof(line),in))
					{
						int tokencount = 0;
						char *token = strtok(line, " ");
						while (token)
						{
							if (state == STATE_TYPE && token[0] != '*' && token[0] != '*')
							{
								if (!filename)
								{
									printf("file: '%s'\n", f.ff_name);
									filename = true;
								}
								int i = strspn(token, cset);
								strncpy(str, token, i);
								str[i] = 0;
								//printf("variable: '%s' '%s'\n", token, str);
								printf("variable: '%s'\n", str);
								if (stage == STAGE_ARRAY)
									fprintf(out, "\t&%s,\n", str);
								else if (stage == STAGE_EXTERN)
									fprintf(out, "extern %s %s;\n", TYPE_NAME, str);
								state = STATE_NONE;
							}
							else if (tokencount == 0 && !strcmp(token, TYPE_NAME))
							{
								if (macrostate == MACRO_IF || macrostate == MACRO_ELSE ||
									(macrostate == MACRO_END && macrostarted))
								{
									if (macrostate == MACRO_IF)
										macrostarted++;
									else if (macrostate == MACRO_END)
										macrostarted--;
									printf("macro: '%s'\n", macro);
									//if (stage == STAGE_ARRAY)
										fprintf(out, "%s\n", macro);
									macrostate = MACRO_NONE;
								}
								state = STATE_TYPE;
								//printf("type: %s\n", token);
							}
							else if (tokencount == 0 && token[0] == '#')
							{
								// found some macro
								macro[0] = 0;
								if (!strncmp(token, "#if", 3))
								{
									macrostate = MACRO_IF;
									macrocount[MACRO_IF]++;
									strncpy(macro, token, sizeof(macro));
									// the rest of the line is a macro
									while ((token = strtok(NULL, " ")))
									{
										strncat(macro, " ", sizeof(macro));
										strncat(macro, token, sizeof(macro));
									}
								}
								else if (macrocount[MACRO_IF] > 0 && !strncmp(token, "#else", 5))
								{
									macrostate = MACRO_ELSE;
									macrocount[MACRO_ELSE]++;
									strncpy(macro, token, sizeof(macro));
								}
								else if (macrocount[MACRO_IF] > 0 && !strncmp(token, "#endif", 5))
								{
									macrostate = MACRO_END;
									macrocount[MACRO_IF]--;
									macrocount[MACRO_ELSE]--;
									strncpy(macro, token, sizeof(macro));
								}
								else
								{
									// DEBUG!!!
									macrostate = MACRO_NONE;
								}
								int i = strcspn(macro, linebreak);
								if (i < strlen(macro))
								{
									macro[i] = 0;
								}
								if (macrostate != MACRO_NONE)
								{
									//printf("macro: '%s'\n", macro);
								}
							}

							tokencount++;
							token = strtok(NULL, " ");
						}
						linenum++;
					}
					fclose(in);
				}
				else
				{
					printf("can't open source %s!\n", f.ff_name);
				}
			} while(!findnext(&f));
		}

		if (stage == STAGE_ARRAY)
			fprintf(out, "\tNULL\n};");

		fprintf(out, "\n");

		// next stage
		stage++;
	}

	fclose(out);

	return 0;
}
