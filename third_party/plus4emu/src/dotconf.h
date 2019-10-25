#ifndef DOTCONF_H
#define DOTCONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* stdio.h should be included by the application - as the manual page says */
#ifndef _STDIO_H
#include <stdio.h>		/* needed for FILE* */
#endif

#ifdef WIN32
# ifndef R_OK
#define R_OK 0
# endif
#endif

/* some buffersize definitions */
#define CFG_BUFSIZE 4096	/* max length of one line */
#define CFG_MAX_OPTION 32	/* max length of any option name */
#define CFG_MAX_VALUE 4064	/* max length of any options value */
#define CFG_MAX_FILENAME 256	/* max length of a filename */
#define CFG_VALUES 16		/* max # of arguments an option takes */

#define CFG_INCLUDEPATH_ENV "DC_INCLUDEPATH"
#define WILDCARDS	"*?"	/* list of supported wild-card characters */

/* constants for type of option */
#define ARG_TOGGLE    0		/* TOGGLE on,off; yes,no; 1, 0; */
#define ARG_INT       1		/* callback wants an integer */
#define ARG_DOUBLE    7		/* callback wants a float / double */
#define ARG_STR       2		/* callback expects a \0 terminated str */
#define ARG_LIST      3		/* wants list of strings */
#define ARG_NAME      4		/* wants option name plus ARG_LIST stuff */
#define ARG_RAW       5		/* wants raw argument data */
#define ARG_NONE      6		/* does not expect ANY args */

#define CTX_ALL       0		/* context: option can be used anywhere */

/* for convenience of terminating the dotconf_options list */
#define LAST_OPTION						{ "", 0, NULL, NULL    }
#define LAST_CONTEXT_OPTION				{ "", 0, NULL, NULL, 0 }

#define DOTCONF_CB(__name)     const char *__name(command_t *cmd,             \
                                                  context_t *ctx)
#define FUNC_ERRORHANDLER(_name) int _name(configfile_t * configfile,		  \
											int type, long dc_errno, const char *msg)

/* some flags that change the runtime behaviour of dotconf */
#define NONE 0
#define CASE_INSENSITIVE 1<<0	/* match option names case insensitive */
#define DONT_SUBSTITUTE 1<<1	/* do not call substitute_env after read_arg */
#define NO_INLINE_COMMENTS 1<<2	/* do not allow inline comments */
#define DUPLICATE_OPTION_NAMES 1<<3	/* allow for duplicate option names */

/* syslog style errors as suggested by Sander Steffann <sander@steffann.nl> */
#ifdef HAVE_SYSLOG
#include <syslog.h>

#define DCLOG_EMERG		LOG_EMERG	/* system is unusable */
#define DCLOG_ALERT		LOG_ALERT	/* action must be taken immediately */
#define DCLOG_CRIT		LOG_CRIT	/* critical conditions */
#define DCLOG_ERR		LOG_ERR	/* error conditions */
#define DCLOG_WARNING	LOG_WARNING	/* warning conditions */
#define DCLOG_NOTICE	LOG_NOTICE	/* normal but significant condition */
#define DCLOG_INFO		LOG_INFO	/* informational */
#define DCLOG_DEBUG		LOG_DEBUG	/* debug-level messages */

#define DCLOG_LEVELMASK	LOG_PRIMASK	/* mask off the level value */

#else				/* HAVE_SYSLOG */

#define DCLOG_EMERG     0	/* system is unusable */
#define DCLOG_ALERT     1	/* action must be taken immediately */
#define DCLOG_CRIT      2	/* critical conditions */
#define DCLOG_ERR       3	/* error conditions */
#define DCLOG_WARNING   4	/* warning conditions */
#define DCLOG_NOTICE    5	/* normal but significant condition */
#define DCLOG_INFO      6	/* informational */
#define DCLOG_DEBUG     7	/* debug-level messages */

#define DCLOG_LEVELMASK 7	/* mask off the level value */

#endif				/* HAVE_SYSLOG */

/* callback types for dotconf_callback */

/* error constants */
#define ERR_NOERROR                0x0000
#define ERR_PARSE_ERROR            0x0001
#define ERR_UNKNOWN_OPTION         0x0002
#define ERR_WRONG_ARG_COUNT        0x0003
#define ERR_INCLUDE_ERROR          0x0004
#define ERR_NOACCESS               0x0005
#define ERR_USER                   0x1000	/* base for userdefined errno's */

/* i needed this to check an ARG_LIST entry if it's toggled in one of my apps; maybe you do too */
#define CFG_TOGGLED(_val) ( (_val[0] == 'Y'                                \
                             || _val[0] == 'y')                            \
                             || (_val[0] == '1')                           \
                             || ((_val[0] == 'o'                           \
                                 || _val[0] == 'O')                        \
                                && (_val[1] == 'n'                         \
                                    || _val[1] == 'N')))

	enum callback_types {
		ERROR_HANDLER = 1,
		CONTEXT_CHECKER
	};

	typedef enum callback_types callback_types;
	typedef struct configfile_t configfile_t;
	typedef struct configoption_t configoption_t;
	typedef struct configoption_t ConfigOption;
	typedef struct command_t command_t;
	typedef void context_t;
	typedef void info_t;

	typedef const char *(*dotconf_callback_t) (command_t *, context_t *);
	typedef int (*dotconf_errorhandler_t) (configfile_t *, int,
					       unsigned long, const char *);
	typedef const char *(*dotconf_contextchecker_t) (command_t *,
							 unsigned long);

	struct configfile_t {
		/* ------ the fields in configfile_t are provided to the app via command_t's ; READ ONLY! --- */

		FILE *stream;
		char eof;	/* end of file reached ? */
		size_t size;	/* file size; cached on-demand for here-documents */

		context_t *context;

		configoption_t const **config_options;
		int config_option_count;

		/* ------ misc read-only fields ------------------------------------------------------------- */
		char *filename;	/* name of file this option was found in */
		unsigned long line;	/* line number we're currently at */
		unsigned long flags;	/* runtime flags given to dotconf_open */

		char *includepath;

		/* ------ some callbacks for interactivity -------------------------------------------------- */
		dotconf_errorhandler_t errorhandler;
		dotconf_contextchecker_t contextchecker;

		int (*cmp_func) (const char *, const char *, size_t);
	};

	struct configoption_t {
		const char *name;	/* name of configuration option */
		int type;	/* for possible values, see above */
		dotconf_callback_t callback;	/* callback function */
		info_t *info;	/* additional info for multi-option callbacks */
		unsigned long context;	/* context sensitivity flags */
	};

	struct command_t {
		const char *name;	/* name of the command */
		configoption_t *option;	/* the option as given in the app; READ ONLY */

		/* ------ misc context information ---------------------------------------------------------- */
		configfile_t *configfile;
		context_t *context;

		/* ------ argument data filled in for each line / command ----------------------------------- */
		int arg_count;	/* number of arguments (in data.list) */
		struct {
			long value;	/* ARG_INT, ARG_TOGGLE */
			char *str;	/* ARG_STR */
			char **list;	/* ARG_LIST */
			double dvalue;	/* ARG_DOUBLE */
		} data;
	};

/* ------ dotconf_create() - create the configfile_t needed for further dot.conf fun ------------ */
	configfile_t *dotconf_create(char *, const configoption_t *,
				     context_t *, unsigned long);

/* ------ dotconf_cleanup() - tidy up behind dotconf_create and the parser dust ----------------- */
	void dotconf_cleanup(configfile_t * configfile);

/* ------ dotconf_command_loop() - iterate through each line of file and handle the commands ---- */
	int dotconf_command_loop(configfile_t * configfile);

/* ------ dotconf_command_loop_until_error() - like continue_line but return on the first error - */
	const char *dotconf_command_loop_until_error(configfile_t * configfile);

/* ------ dotconf_continue_line() - check if line continuation is to be handled ----------------- */
	int dotconf_continue_line(char *buffer, size_t length);

/* ------ dotconf_get_next_line() - read in the next line of the configfile_t ------------------- */
	int dotconf_get_next_line(char *buffer, size_t bufsize,
				  configfile_t * configfile);

/* ------ dotconf_get_here_document() - read the here document until delimit is found ----------- */
	char *dotconf_get_here_document(configfile_t * configfile,
					const char *delimit);

/* ------ dotconf_invoke_command() - call the callback for command_t ---------------------------- */
	const char *dotconf_invoke_command(configfile_t * configfile,
					   command_t * cmd);

/* ------ dotconf_find_command() - iterate through all registered options trying to match ------- */
#if 0
	configoption_t *dotconf_find_command(configfile_t * configfile,
					     const char *command);
#endif

/* ------ dotconf_read_arg() - read one argument from the line handling quoting and escaping ---- */
/*
	side effects: the char* returned by dotconf_read_arg is malloc() before, hence that pointer
                  will have to be free()ed later.
*/
	char *dotconf_read_arg(configfile_t * configfile, char **line);

/* ------ dotconf_handle_command() - parse, substitute, find, invoke the command found in buffer  */
	const char *dotconf_handle_command(configfile_t * configfile,
					   char *buffer);

/* ------ dotconf_register_option() - add a new option table to the list of commands ------------ */
	int dotconf_register_options(configfile_t * configfile,
				     const configoption_t * options);

/* ------ dotconf_warning() - handle the dispatch of error messages of various levels ----------- */
	int dotconf_warning(configfile_t * configfile, int level,
			    unsigned long errnum, const char *, ...);

/* ------ dotconf_callback() - register a special callback -------------------------------------- */
	void dotconf_callback(configfile_t * configfile, callback_types type,
			      dotconf_callback_t);

/* ------ dotconf_substitute_env() - handle the substitution on environment variables ----------- */
	char *dotconf_substitute_env(configfile_t *, char *);

#ifdef __cplusplus
}				/* extern "C" */
#endif
#endif				/* DOTCONF_H */
