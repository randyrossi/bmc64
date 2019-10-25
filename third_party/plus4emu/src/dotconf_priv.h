#ifndef DOTCONF_PRIV_H
#define DOTCONF_PRIV_H

/* verify whether or not a character is in the WILDCARDS list */
	int dotconf_is_wild_card(char value);

/* call the appropriate routine for the wildcard passed in */
	int dotconf_handle_wild_card(command_t * cmd, char wild_card,
				     char *path, char *pre, char *ext);

/* free allocated memory from dotcont_find_wild_card */
	void dotconf_wild_card_cleanup(char *path, char *pre);

/* check for wild cards in file path */
/* path and pre must be freed by the developer ( dotconf_wild_card_cleanup) */
	int dotconf_find_wild_card(char *filename, char *wildcard, char **path,
				   char **pre, char **ext);

/* compare two strings from back to front */
	int dotconf_strcmp_from_back(const char *s1, const char *s2);

/* determine if a string matches the '?' criteria */
	int dotconf_question_mark_match(char *dir_name, char *pre, char *ext);

/* determine if a string matches the '*' criteria */
	int dotconf_star_match(char *dir_name, char *pre, char *ext);

/* determine matches for filenames with a '?' in name and call */
/* the Internal Include function on that filename */
	int dotconf_handle_question_mark(command_t * cmd, char *path, char *pre,
					 char *ext);

/* determine matches for filenames with a '*' in name and call */
/* the Internal Include function on that filename */
	int dotconf_handle_star(command_t * cmd, char *path, char *pre,
				char *ext);

char *get_cwd(void);

char *get_path(char *name);

#endif
