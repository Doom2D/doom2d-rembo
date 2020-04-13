#ifndef CP866_H_INCLUDED
#define CP866_H_INCLUDED

int cp866_isalpha (int ch);
int cp866_isupper (int ch);
int cp866_islower (int ch);
int cp866_toupper (int ch);
int cp866_tolower (int ch);
int cp866_strcasecmp (const char *a, const char *b);
int cp866_strncasecmp (const char *a, const char *b, unsigned int n);

int cp866_ctou (int ch);
int cp866_ctoug (int ch);
int cp866_utoc (int ch);

#endif /* CP866_H_INCLUDED */
