#ifndef CP866_H_INCLUDED
#define CP866_H_INCLUDED

int cp866_isalpha (char ch);
int cp866_isupper (char ch);
int cp866_islower (char ch);
char cp866_toupper (char ch);
char cp866_tolower (char ch);
int cp866_strcasecmp (const char *a, const char *b);
int cp866_strncasecmp (const char *a, const char *b, unsigned int n);

int cp866_ctou (char ch);
int cp866_ctoug (char ch);
int cp866_utoc (int ch);

#endif /* CP866_H_INCLUDED */
