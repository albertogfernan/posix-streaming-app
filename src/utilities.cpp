#include "utilities.h"

unsigned int count_lines(FILE * filedesc){
	char c;
	unsigned int newline_count = 0;
	long pos;

	pos = ftell(filedesc);
	rewind(filedesc);

	while ((c = fgetc(filedesc)) != EOF) {
		if (c == '\n')
			newline_count++;
	}

	fseek(filedesc, pos, SEEK_SET);
	return newline_count;
}


char *trim_whitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace(*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace(*end)) end--;

  // Write new null terminator
  *(end+1) = 0;

  return str;
}

int count_chars(char *s, char c)
{
	int i;
	for (i=0; s[i]; s[i]==c ? i++ : *s++);
	return i;
}


int parse_options(char* options, char** program, program_opt_t** options_found)
{
	int numops;
	int i=0, j=0;
	char *key, *value;

	numops = count_chars(options, ' ');

	char* tokens[numops];
	*options_found = (program_opt_t*)malloc(sizeof(program_opt_t) * numops);

	*program = strtok(options, " ");

	//Do de actual parse
	while(i < numops && options != NULL)
	{
		tokens[i] = strtok(NULL, " ");
		i++;
	}

	for(j = 0; j < i; j++)
	{
		key = strtok(tokens[j], "=");
		value = strtok(NULL, "=");

		strncpy((*options_found)[j].key, key, OPT_MAX_LENGTH);
		strncpy((*options_found)[j].value, value, OPT_MAX_LENGTH);
	}

	return i;

}

unsigned long long int then(struct timeval *t) {
	return (t->tv_sec * 1e6 + t->tv_usec);
}

unsigned long long int stamp(struct timeval *t) {
	gettimeofday(t, NULL);
	return (t->tv_sec * 1e6 + t->tv_usec);
}

unsigned long long int now() {
	struct timeval t;
	gettimeofday(&t, NULL);
	return (t.tv_sec * 1e6 + t.tv_usec);
}


