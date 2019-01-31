extern char *intl_speed_at_text;

extern char *intl_translate_text(int en_resource);
extern int intl_translate_res(int en_resource);

extern void intl_init(void);
extern void intl_shutdown(void);
extern char *intl_arch_language_init(void);
extern void intl_update_ui(void);
extern char *intl_convert_cp(char *text, int cp);

extern void intl_convert_mui_table(int x[], char *y[]);