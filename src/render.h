void R_draw (void);
void R_alloc (void);
void R_init (void);
void R_done (void);

void R_setgamma (int g);
int R_getgamma (void);
void R_toggle_fullscreen (void);

void R_get_name (int n, char s[8]);
int R_get_special_id (int n);
void R_begin_load (void);
void R_load (char s[8], int f);
void R_end_load (void);
void R_loadsky (int sky);
