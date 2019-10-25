/* dot.conf - configuration file parser library
 * Copyright (C) 1999,2000,2001,2002 Lukas Schroeder <lukas@azzit.de>,
 *   and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 *
 */

/* -- dotconf.c - this code is responsible for the input, parsing and dispatching of options  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Added by Stephen W. Boyer <sboyer@caldera.com>
 * for wildcard support in Include file paths
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>

/* -- AIX 4.3 compile time fix
 * by Eduardo Marcel Macan <macan@colband.com.br>
 *
 * modified by Stephen W. Boyer <sboyer@caldera.com>
 * for Unixware and OpenServer
 */

#if defined (_AIX43) || defined(UNIXWARE) || defined(OSR5)
#include <strings.h>
#endif

#include <stdarg.h>
#include <time.h>
#include <sys/stat.h>

#if 1 /* ndef WIN32 */          /* IstvanV: this also works on Win32 / MinGW */

#include <dirent.h>
#include <unistd.h>

#else /* ndef WIN32 */

#include "readdir.h"            /* WIN32 fix by Robert J. Buck */

#define strncasecmp strnicmp
typedef unsigned long ulong;
#define snprintf _snprintf
#define vsnprintf _vsnprintf
#endif /* !WIN32 */

#include <ctype.h>
#include "dotconf.h"
#include "dotconf_priv.h"

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifdef WIN32
/* defined in cfg_db.cpp */
extern FILE *Plus4Emu_fileOpen(const char *name, const char *mode);
#endif

static char name[CFG_MAX_OPTION + 1];   /* option name */

/*
 * some 'magic' options that are predefined by dot.conf itself for
 * advanced functionality
 */
static DOTCONF_CB(dotconf_cb_include);  /* internal 'Include'     */
static DOTCONF_CB(dotconf_cb_includepath);      /* internal 'IncludePath' */

static configoption_t dotconf_options[] = {
        {"Include", ARG_STR, dotconf_cb_include, NULL, CTX_ALL},
        {"IncludePath", ARG_STR, dotconf_cb_includepath, NULL, CTX_ALL},
        LAST_CONTEXT_OPTION
};

static void skip_whitespace(char **cp, int n, char term)
{
        char *cp1 = *cp;
        while (isspace((int)*cp1) && *cp1 != term && n--)
                cp1++;
        *cp = cp1;
}

static void copy_word(char **dest, char **src, int max, char term)
{
        char *cp1 = *src;
        char *cp2 = *dest;
        while (max-- && !isspace((int)*cp1) && *cp1 != term)
                *cp2++ = *cp1++;
        *cp2 = 0;

        *src = cp1;
        *dest = cp2;
}

static const configoption_t *get_argname_fallback(const configoption_t *
                                                  options)
{
        int i;

        for (i = 0; (options[i].name && options[i].name[0]); i++) ;
        if (options[i].type == ARG_NAME && options[i].callback)
                return &options[i];
        return NULL;
}

char *dotconf_substitute_env(configfile_t * configfile, char *str)
{
        char *cp1, *cp2, *cp3, *eos, *eob;
        char *env_value;
        char env_name[CFG_MAX_VALUE + 1];
        char env_default[CFG_MAX_VALUE + 1];
        char tmp_value[CFG_MAX_VALUE + 1];

        memset(env_name, 0, CFG_MAX_VALUE + 1);
        memset(env_default, 0, CFG_MAX_VALUE + 1);
        memset(tmp_value, 0, CFG_MAX_VALUE + 1);

        cp1 = str;
        eob = cp1 + strlen(str) + 1;
        cp2 = tmp_value;
        eos = cp2 + CFG_MAX_VALUE + 1;

        while ((cp1 < eob) && (cp2 < eos) && (*cp1 != '\0')) {
                /* substitution needed ?? */
                if (*cp1 == '$' && *(cp1 + 1) == '{') {
                        cp1 += 2;       /* skip ${ */
                        cp3 = env_name;
                        while ((cp1 < eob) && !(*cp1 == '}' || *cp1 == ':'))
                                *cp3++ = *cp1++;
                        *cp3 = '\0';    /* terminate */

                        /* default substitution */
                        if (*cp1 == ':' && *(cp1 + 1) == '-') {
                                cp1 += 2;       /* skip :- */
                                cp3 = env_default;
                                while ((cp1 < eob) && (*cp1 != '}'))
                                        *cp3++ = *cp1++;
                                *cp3 = '\0';    /* terminate */
                        } else {
                                while ((cp1 < eob) && (*cp1 != '}'))
                                        cp1++;
                        }

                        if (*cp1 != '}') {
                                dotconf_warning(configfile, DCLOG_WARNING,
                                                ERR_PARSE_ERROR,
                                                "Unbalanced '{'");
                        } else {
                                cp1++;  /* skip } */
                                if ((env_value = getenv(env_name)) != NULL) {
                                        strncat(cp2, env_value, eos - cp2);
                                        cp2 += strlen(env_value);
                                } else {
                                        strncat(cp2, env_default, eos - cp2);
                                        cp2 += strlen(env_default);
                                }
                        }

                }

                *cp2++ = *cp1++;
        }
        *cp2 = '\0';            /* terminate buffer */

        free(str);
        return strdup(tmp_value);
}

int dotconf_warning(configfile_t * configfile, int type, unsigned long errnum,
                    const char *fmt, ...)
{
        va_list args;
        int retval = 0;

        va_start(args, fmt);
        if (configfile->errorhandler != 0) {    /* an errorhandler is registered */
                char msg[CFG_BUFSIZE];
                vsnprintf(msg, CFG_BUFSIZE, fmt, args);
                retval =
                    configfile->errorhandler(configfile, type, errnum, msg);
        } else {                /* no errorhandler, do-it-yourself */

                retval = 0;
                fprintf(stderr, "%s:%ld: ", configfile->filename,
                        configfile->line);
                vfprintf(stderr, fmt, args);
                fprintf(stderr, "\n");
        }
        va_end(args);

        return retval;
}

int dotconf_register_options(configfile_t * configfile,
                             const configoption_t * options)
{
        int num = configfile->config_option_count;
        int ret = 0;
        configoption_t const **temp = configfile->config_options;

#define GROW_BY   10

        /* resize memoryblock for options blockwise */
        if (temp == NULL)
                temp = malloc(sizeof(configoption_t *) * (GROW_BY + 1));
        else {
                if (!(num % GROW_BY))
                        temp = realloc(temp,
                                       sizeof(configoption_t *) * (num +
                                                                   GROW_BY +
                                                                   1));
        }

#undef GROW_BY

        if (temp != NULL) {
                /* Allocation or reallocation was successful. */
                /* append new options */
                temp[configfile->config_option_count] = options;
                configfile->config_options = temp;
                configfile->config_options[++configfile->config_option_count] =
                    0;
                ret = 1;
        }

        return ret;
}

void dotconf_callback(configfile_t * configfile, callback_types type,
                      dotconf_callback_t callback)
{
        switch (type) {
        case ERROR_HANDLER:
                configfile->errorhandler = (dotconf_errorhandler_t) callback;
                break;
        case CONTEXT_CHECKER:
                configfile->contextchecker =
                    (dotconf_contextchecker_t) callback;
                break;
        default:
                break;
        }
}

int dotconf_continue_line(char *buffer, size_t length)
{
        /* ------ match [^\\]\\[\r]\n ------------------------------ */
        char *cp1 = buffer + length;

        if (length < 2 || *(--cp1) != '\n')
                return 0;

        if (*(--cp1) == '\r' && length >= 3)
                cp1--;

        if (*cp1 != '\\')
                return 0;

        *cp1 = '\0';            /* strip escape character and/or newline */
        return (cp1 <= buffer || *(cp1 - 1) != '\\');
}

int dotconf_get_next_line(char *buffer, size_t bufsize,
                          configfile_t * configfile)
{
        char *cp1, *cp2;
        size_t length;

        if (configfile->eof)
                return 1;

        cp1 = fgets(buffer, bufsize, configfile->stream);

        if (!cp1) {
                configfile->eof = 1;
                return 1;
        }

        configfile->line++;
        length = strlen(cp1);
        while (dotconf_continue_line(cp1, length)) {
                length = strlen(cp1);
                cp2 = fgets(cp1 + length, bufsize - length, configfile->stream);
                if (!cp2) {
                        fprintf(stderr,
                                "[dotconf] Parse error. Unexpected end of file at "
                                "line %ld in file %s\n", configfile->line,
                                configfile->filename);
                        configfile->eof = 1;
                        return 1;
                }
                configfile->line++;
                length = strlen(cp1);
        }

        return 0;
}

char *dotconf_get_here_document(configfile_t * configfile, const char *delimit)
{
        /* it's a here-document: yeah, what a cool feature ;) */
        unsigned int limit_len;
        char here_string;
        char buffer[CFG_BUFSIZE];
        char *here_doc = 0;
        char here_limit[9];     /* max length for here-document delimiter: 8 */
        struct stat finfo;
        int offset = 0;

        if (configfile->size <= 0) {
                if (stat(configfile->filename, &finfo)) {
                        dotconf_warning(configfile, DCLOG_EMERG, ERR_NOACCESS,
                                        "[emerg] could not stat currently read file (%s)\n",
                                        configfile->filename);
                        return NULL;
                }
                configfile->size = finfo.st_size;
        }

        /*
         * allocate a buffer of filesize bytes; should be enough to
         * prevent buffer overflows
         */
        here_doc = malloc(configfile->size);    /* allocate buffer memory */
        memset(here_doc, 0, configfile->size);

        here_string = 1;
        limit_len = snprintf(here_limit, 9, "%s", delimit);
        while (!dotconf_get_next_line(buffer, CFG_BUFSIZE, configfile)) {
                if (!strncmp(here_limit, buffer, limit_len - 1)) {
                        here_string = 0;
                        break;
                }
                offset +=
                    snprintf((here_doc + offset), configfile->size - offset - 1,
                             "%s", buffer);
        }
        if (here_string)
                dotconf_warning(configfile, DCLOG_WARNING, ERR_PARSE_ERROR,
                                "Unterminated here-document!");

        here_doc[offset - 1] = '\0';    /* strip newline */

        return (char *)realloc(here_doc, offset);
}

const char *dotconf_invoke_command(configfile_t * configfile, command_t * cmd)
{
        const char *error = 0;

        error = cmd->option->callback(cmd, configfile->context);
        return error;
}

char *dotconf_read_arg(configfile_t * configfile, char **line)
{
        int sq = 0, dq = 0;     /* single quote, double quote */
        int done;
        char *cp1 = *line;
        char *cp2, *eos;
        char buf[CFG_MAX_VALUE];

        memset(buf, 0, CFG_MAX_VALUE);
        done = 0;
        cp2 = buf;
        eos = cp2 + CFG_MAX_VALUE - 1;

        if (*cp1 == '#' || !*cp1)
                return NULL;

        skip_whitespace(&cp1, CFG_MAX_VALUE, 0);

        while ((*cp1 != '\0') && (cp2 != eos) && !done) {
                switch (*cp1) {
                case '\'':      /* single quote */
                        if (dq)
                                break;  /* already double quoting, break out */
                        if (sq)
                                sq--;   /* already single quoting, clear state */
                        else if (!sq)
                                sq++;   /* set state for single quoting */
                        break;
                case '"':       /* double quote */
                        if (sq)
                                break;  /* already single quoting, break out */
                        if (dq)
                                dq--;   /* already double quoting, clear state */
                        else if (!dq)
                                dq++;   /* set state for double quoting */
                        break;
                case '\\':      /* protected chars */
                        if (!cp1[1])    /* dont protect NUL */
                                break;
                        *cp2++ = *(++cp1);
                        cp1++;  /* skip the protected one */
                        continue;
                        break;
                default:
                        break;
                }

                /* unquoted space: start a new option argument */
                if (isspace((int)*cp1) && !dq && !sq) {
                        *cp2 = '\0';
                        break;
                }
                /* unquoted, unescaped comment-hash ; break out, unless NO_INLINE_COMMENTS is set */
                else if (*cp1 == '#' && !dq && !sq
                         && !(configfile->flags & NO_INLINE_COMMENTS)) {
                        /*
                         * NOTE: 1.0.8a got the NO_INLINE_COMMENTS feature wrong: it
                         * skipped every argument starting with a #, instead of simply eating it!
                         */

                        *cp2 = 0;
                        *cp1 = 0;
                        *line = cp1;
                        return NULL;
                }
                /* not space or quoted: eat it; dont take quote if quoting */
                else if ((!isspace((int)*cp1) && !dq && !sq && *cp1 != '"'
                          && *cp1 != '\'')
                         || (dq && (*cp1 != '"')) || (sq && *cp1 != '\'')) {
                        *cp2++ = *cp1;
                }

                cp1++;
        }

        *line = cp1;

        /* FIXME: escaping substitutes does not work
           Subst ${HOME} \$\{HOME\}
           BOTH! will be substituted, which is somewhat wrong, ain't it ?? :-(
         */
        if ((configfile->flags & DONT_SUBSTITUTE) == DONT_SUBSTITUTE)
                return buf[0] ? strdup(buf) : NULL;
        return buf[0] ? dotconf_substitute_env(configfile, strdup(buf)) : NULL;
}

/* dotconf_find_command remains here for backwards compatability. it's
 * internally unused since dot.conf 1.0.9 because it cannot handle the
 * DUPLICATE_OPTION_NAMES flag
 */
#if 0
configoption_t *dotconf_find_command(configfile_t * configfile,
                                     const char *command)
{
        configoption_t *option;
        int i = 0, mod = 0, done = 0;

        for (option = 0, mod = 0; configfile->config_options[mod] && !done;
             mod++)
                for (i = 0; configfile->config_options[mod][i].name[0]; i++) {
                        if (!configfile->cmp_func(name,
                                                  configfile->
                                                  config_options[mod][i].name,
                                                  CFG_MAX_OPTION)) {
                                option =
                                    (configoption_t *) & configfile->
                                    config_options[mod][i];
                                /* TODO: this could be flagged: option overwriting by modules */
                                done = 1;
                                break;  /* found it; break out */
                        }
                }

        /* handle ARG_NAME fallback */
        if ((option && option->name[0] == 0)
            || configfile->config_options[mod - 1][i].type == ARG_NAME) {
                option =
                    (configoption_t *) & configfile->config_options[mod - 1][i];
        }

        return option;
}
#endif

void dotconf_set_command(configfile_t * configfile,
                         const configoption_t * option, char *args,
                         command_t * cmd)
{
        char *eob = args + strlen(args);

        /* fill in the command_t structure with values we already know */
        cmd->name = option->type == ARG_NAME ? name : option->name;
        cmd->option = (configoption_t *) option;
        cmd->context = configfile->context;
        cmd->configfile = configfile;
        cmd->data.list = (char **)calloc(CFG_VALUES, sizeof(char *));
        cmd->data.str = 0;

        if (option->type == ARG_RAW) {
                /* if it is an ARG_RAW type, save some time and call the
                   callback now */
                cmd->data.str = strdup(args);
        } else if (option->type == ARG_STR) {
                char *cp = args;

                /* check if it's a here-document and act accordingly */
                skip_whitespace(&cp, eob - cp, 0);

                if (!strncmp("<<", cp, 2)) {
                        cmd->data.str =
                            dotconf_get_here_document(configfile, cp + 2);
                        cmd->arg_count = 1;
                }
        }

        if (!(option->type == ARG_STR && cmd->data.str != 0)) {
                /* we only get here for non-heredocument lines */

                skip_whitespace(&args, eob - args, 0);

                cmd->arg_count = 0;
                while (cmd->arg_count < (CFG_VALUES - 1)
                       && (cmd->data.list[cmd->arg_count] =
                           dotconf_read_arg(configfile, &args))) {
                        cmd->arg_count++;
                }

                skip_whitespace(&args, eob - args, 0);

                if (cmd->arg_count && cmd->data.list[cmd->arg_count - 1]
                    && *args)
                        cmd->data.list[cmd->arg_count++] = strdup(args);

                /* has an option entry been found before or do we have to use a fallback? */
                if ((option->name && option->name[0] > 32)
                    || option->type == ARG_NAME) {
                        /* found it, now check the type of args it wants */
                        switch (option->type) {
                        case ARG_TOGGLE:
                                /* the value is true if the argument is Yes, On or 1 */
                                if (cmd->arg_count < 1) {
                                        dotconf_warning(configfile,
                                                        DCLOG_WARNING,
                                                        ERR_WRONG_ARG_COUNT,
                                                        "Missing argument to option '%s'",
                                                        name);
                                        return;
                                }

                                cmd->data.value =
                                    CFG_TOGGLED(cmd->data.list[0]);
                                break;
                        case ARG_INT:
                                if (cmd->arg_count < 1) {
                                        dotconf_warning(configfile,
                                                        DCLOG_WARNING,
                                                        ERR_WRONG_ARG_COUNT,
                                                        "Missing argument to option '%s'",
                                                        name);
                                        return;
                                }

                                sscanf(cmd->data.list[0], "%li",
                                       &cmd->data.value);
                                break;

                        case ARG_DOUBLE:
                                if (cmd->arg_count < 1) {
                                        dotconf_warning(configfile,
                                                        DCLOG_WARNING,
                                                        ERR_WRONG_ARG_COUNT,
                                                        "Missing argument to option '%s'",
                                                        name);
                                        return;
                                }

                                cmd->data.dvalue = strtod(cmd->data.list[0], 0);
                                break;

                        case ARG_STR:
                                if (cmd->arg_count < 1) {
                                        dotconf_warning(configfile,
                                                        DCLOG_WARNING,
                                                        ERR_WRONG_ARG_COUNT,
                                                        "Missing argument to option '%s'",
                                                        name);
                                        return;
                                }

                                cmd->data.str = strdup(cmd->data.list[0]);
                                break;
                        case ARG_NAME:  /* fall through */
                        case ARG_LIST:
                        case ARG_NONE:
                        case ARG_RAW:   /* this has been handled before */
                        default:
                                break;
                        }
                }
        }
}

void dotconf_free_command(command_t * command)
{
        int i;

        if (command->data.str)
                free(command->data.str);

        for (i = 0; i < command->arg_count; i++)
                free(command->data.list[i]);
        free(command->data.list);
}

const char *dotconf_handle_command(configfile_t * configfile, char *buffer)
{
        char *cp1;
        char *cp2;
        /* generic char pointer      */
        char *eob;              /* end of buffer; end of string  */
        const char *error;      /* error message we'll return */
        const char *context_error;      /* error message returned by contextchecker */
        command_t command;      /* command structure */
        int mod = 0;
        int next_opt_idx = 0;

        memset(&command, 0, sizeof(command_t));
        name[0] = 0;
        error = 0;
        context_error = 0;

        cp1 = buffer;
        eob = cp1 + strlen(cp1);

        skip_whitespace(&cp1, eob - cp1, 0);

        /* ignore comments and empty lines */
        if (!cp1 || !*cp1 || *cp1 == '#' || *cp1 == '\n' || *cp1 == EOF)
                return NULL;

        /* skip line if it only contains whitespace */
        if (cp1 == eob)
                return NULL;

        /* get first token: read the name of a possible option */
        cp2 = name;
        copy_word(&cp2, &cp1, MIN(eob - cp1, CFG_MAX_OPTION), 0);

        while (1) {
                const configoption_t *option;
                int done = 0;
                int opt_idx = 0;

                for (option = 0; configfile->config_options[mod] && !done;
                     mod++) {
                        for (opt_idx = next_opt_idx;
                             configfile->config_options[mod][opt_idx].name[0];
                             opt_idx++) {
                                if (!configfile->
                                    cmp_func(name,
                                             configfile->
                                             config_options[mod][opt_idx].name,
                                             CFG_MAX_OPTION)) {
                                        /* TODO: this could be flagged: option overwriting by modules */
                                        option =
                                            (configoption_t *) & configfile->
                                            config_options[mod][opt_idx];
                                        done = 1;
                                        break;  /* found one; break out */
                                }
                        }
                }

                if (!option)
                        option =
                            get_argname_fallback(configfile->config_options[1]);

                if (!option || !option->callback) {
                        if (error)
                                return error;
                        dotconf_warning(configfile, DCLOG_INFO,
                                        ERR_UNKNOWN_OPTION,
                                        "Unknown Config-Option: '%s'", name);
                        return NULL;
                }

                /* set up the command structure (contextchecker wants this) */
                dotconf_set_command(configfile, option, cp1, &command);

                if (configfile->contextchecker)
                        context_error =
                            configfile->contextchecker(&command,
                                                       command.option->context);

                if (!context_error)
                        error = dotconf_invoke_command(configfile, &command);
                else {
                        if (!error) {
                                /* avoid returning another error then the first. This makes it easier to
                                   reproduce problems. */
                                error = context_error;
                        }
                }

                dotconf_free_command(&command);

                if (!context_error
                    || !(configfile->flags & DUPLICATE_OPTION_NAMES)) {
                        /* don't try more, just quit now. */
                        break;
                }
        }

        return error;
}

const char *dotconf_command_loop_until_error(configfile_t * configfile)
{
        char buffer[CFG_BUFSIZE];

        while (!(dotconf_get_next_line(buffer, CFG_BUFSIZE, configfile))) {
                const char *error = dotconf_handle_command(configfile, buffer);
                if (error)
                        return error;
        }
        return NULL;
}

int dotconf_command_loop(configfile_t * configfile)
{
        /* ------ returns: 0 for failure -- !0 for success ------------------------------------------ */
        char buffer[CFG_BUFSIZE];

        while (!(dotconf_get_next_line(buffer, CFG_BUFSIZE, configfile))) {
                const char *error = dotconf_handle_command(configfile, buffer);
                if (error != NULL) {
                        if (dotconf_warning(configfile, DCLOG_ERR, 0, error))
                                return 0;
                }
        }
        return 1;
}

void dotconf_cleanup(configfile_t * configfile)
{
        if (configfile->stream)
                fclose(configfile->stream);

        if (configfile->filename)
                free(configfile->filename);

        if (configfile->config_options)
                free(configfile->config_options);

        if (configfile->includepath)
                free(configfile->includepath);

        free(configfile);
}

configfile_t *dotconf_create(char *fname, const configoption_t * options,
                             context_t * context, unsigned long flags)
{
        char *dc_env = NULL;
        int registered = 0;
        configfile_t *new_cfg = calloc(1, sizeof(configfile_t));
        char *path = NULL;
        char *cwd = NULL;

        if (!new_cfg)
                return NULL;

        /*
         * From here on, we can use dotconf_cleanup to free resources
         * when errors occur.  All of our pointers are NULL, because
         * we allocated with calloc.  When an error occurs, dotconf_cleanup
         * will free all of the resources that were allocated prior to the
         * error.
         */

        new_cfg->context = context;
        new_cfg->flags = flags;
        if (new_cfg->flags & CASE_INSENSITIVE)
                new_cfg->cmp_func = strncasecmp;
        else
                new_cfg->cmp_func = strncmp;

#ifndef WIN32
        new_cfg->stream = fopen(fname, "r");
#else
        /* use UTF-8 compatible wrapper on Windows */
        new_cfg->stream = Plus4Emu_fileOpen(fname, "r");
#endif
        if (new_cfg->stream == NULL) {
                fprintf(stderr, "Error opening configuration file '%s'\n",
                        fname);
                dotconf_cleanup(new_cfg);
                return NULL;
        }

        registered = dotconf_register_options(new_cfg, dotconf_options);
        if (!registered) {
                dotconf_cleanup(new_cfg);
                return NULL;
        }

        registered = dotconf_register_options(new_cfg, options);
        if (!registered) {
                dotconf_cleanup(new_cfg);
                return NULL;
        }

        new_cfg->filename = strdup(fname);
        if (!new_cfg->filename) {
                dotconf_cleanup(new_cfg);
                return NULL;
        }

        new_cfg->includepath = malloc(CFG_MAX_FILENAME);
        if (!new_cfg->includepath) {
                dotconf_cleanup(new_cfg);
                return NULL;
        }

        new_cfg->includepath[0] = 0x00;

        /*
         * take default includepath from environment if present
         * Otherwise, resolve the path of the configuration file and use that
         * as default includepath.
         */
        dc_env = getenv(CFG_INCLUDEPATH_ENV);
        if (dc_env != NULL) {
                snprintf(new_cfg->includepath, CFG_MAX_FILENAME, "%s", dc_env);
        } else {
                path = get_path(fname);
                if (path != NULL) {
                        if (path[0] == '/') {
                                snprintf(new_cfg->includepath, CFG_MAX_FILENAME, "%s", path);
                        } else {
                                cwd = get_cwd();
                                if (cwd != NULL) {
                                        snprintf(new_cfg->includepath, CFG_MAX_FILENAME,
                                                "%s/%s", cwd, path);
                                        free(cwd);
                                }
                        }
                        free(path);
                }
        }
        return new_cfg;
}

#ifndef WIN32

/* ------ internal utility function that verifies if a character is in the WILDCARDS list -- */
int dotconf_is_wild_card(char value)
{
        int retval = 0;
        int i;
        int wildcards_len = strlen(WILDCARDS);

        for (i = 0; i < wildcards_len; i++) {
                if (value == WILDCARDS[i]) {
                        retval = 1;
                        break;
                }
        }

        return retval;
}

/* ------ internal utility function that calls the appropriate routine for the wildcard passed in -- */
int dotconf_handle_wild_card(command_t * cmd, char wild_card, char *path,
                             char *pre, char *ext)
{
        int retval = 0;

        switch (wild_card) {
        case '*':

                retval = dotconf_handle_star(cmd, path, pre, ext);

                break;

        case '?':

                retval = dotconf_handle_question_mark(cmd, path, pre, ext);

                break;

        default:
                retval = -1;
        }

        return retval;
}

/* ------ internal utility function that frees allocated memory from dotcont_find_wild_card -- */
void dotconf_wild_card_cleanup(char *path, char *pre)
{

        if (path != NULL) {
                free(path);
        }

        if (pre != NULL) {
                free(pre);
        }

}

/* ------ internal utility function to check for wild cards in file path -- */
/* ------ path and pre must be freed by the developer ( dotconf_wild_card_cleanup) -- */
int dotconf_find_wild_card(char *filename, char *wildcard, char **path,
                           char **pre, char **ext)
{
        int retval = -1;
        int prefix_len = 0;
        int tmp_count = 0;
        char *tmp = 0;
        int found_path = 0;

        int len = strlen(filename);

        if (wildcard != NULL && len > 0 && path != NULL && pre != NULL
            && ext != NULL) {
                prefix_len = strcspn(filename, WILDCARDS);      /* find any wildcard in WILDCARDS */

                if (prefix_len < len) { /* Wild card found */
                        tmp = filename + prefix_len;
                        tmp_count = prefix_len + 1;

                        while (tmp != filename && *(tmp) != '/') {
                                tmp--;
                                tmp_count--;
                        }

                        if (*(tmp) == '/') {
                                *path = (char *)malloc(tmp_count + 1);
                                found_path = 1;

                        } else

                                *path = (char *)malloc(1);

                        *pre =
                            (char *)
                            malloc((prefix_len -
                                    (tmp_count - (found_path ? 0 : 1))) + 1);

                        if (*path && *pre) {
                                if (found_path)
                                        strncpy(*path, filename, tmp_count);
                                (*path)[tmp_count] = '\0';

                                strncpy(*pre, (tmp + (found_path ? 1 : 0)),
                                        (prefix_len -
                                         (tmp_count - (found_path ? 0 : 1))));
                                (*pre)[(prefix_len -
                                        (tmp_count - (found_path ? 0 : 1)))] =
                                    '\0';

                                *ext = filename + prefix_len;
                                *wildcard = (**ext);
                                (*ext)++;

                                retval = prefix_len;

                        }

                }

        }

        return retval;
}

/* ------ internal utility function that compares two stings from back to front -- */
int dotconf_strcmp_from_back(const char *s1, const char *s2)
{
        int retval = 0;
        int i, j;
        int len_1 = strlen(s1);
        int len_2 = strlen(s2);

        for (i = len_1, j = len_2; (i >= 0 && j >= 0); i--, j--) {
                if (s1[i] != s2[j]) {
                        retval = -1;
                        break;
                }
        }

        return retval;
}

/* ------ internal utility function that determins if a string matches the '?' criteria -- */
int dotconf_question_mark_match(char *dir_name, char *pre, char *ext)
{
        int retval = -1;
        int dir_name_len = strlen(dir_name);
        int pre_len = strlen(pre);
        int ext_len = strlen(ext);
        int w_card_check = strcspn(ext, WILDCARDS);

        if ((w_card_check < ext_len) && (strncmp(dir_name, pre, pre_len) == 0)
            && (strcmp(dir_name, ".") != 0) && (strcmp(dir_name, "..") != 0)) {
                retval = 1;     /* Another wildcard found */

        } else {

                if ((dir_name_len >= pre_len) &&
                    (strncmp(dir_name, pre, pre_len) == 0) &&
                    (strcmp(dir_name, ".") != 0) &&
                    (strcmp(dir_name, "..") != 0)) {
                        retval = 0;     /* Matches no other wildcards */
                }

        }

        return retval;
}

/* ------ internal utility function that determins if a string matches the '*' criteria -- */
int dotconf_star_match(char *dir_name, char *pre, char *ext)
{
        int retval = -1;
        int dir_name_len = strlen(dir_name);
        int pre_len = strlen(pre);
        int ext_len = strlen(ext);
        int w_card_check = strcspn(ext, WILDCARDS);

        if ((w_card_check < ext_len) && (strncmp(dir_name, pre, pre_len) == 0)
            && (strcmp(dir_name, ".") != 0) && (strcmp(dir_name, "..") != 0)) {
                retval = 1;     /* Another wildcard found */

        } else {

                if ((dir_name_len >= (ext_len + pre_len)) &&
                    (dotconf_strcmp_from_back(dir_name, ext) == 0) &&
                    (strncmp(dir_name, pre, pre_len) == 0) &&
                    (strcmp(dir_name, ".") != 0) &&
                    (strcmp(dir_name, "..") != 0)) {
                        retval = 0;     /* Matches no other wildcards */
                }

        }

        return retval;
}

/* ------ internal utility function that determins matches for filenames with   -- */
/* ------ a '?' in name and calls the Internal Include function on that filename -- */
int dotconf_handle_question_mark(command_t * cmd, char *path, char *pre,
                                 char *ext)
{
        configfile_t *included;
        DIR *dh = 0;
        struct dirent *dirptr = 0;
        int i;

        char new_pre[CFG_MAX_FILENAME];
        char already_matched[CFG_MAX_FILENAME];

        char wc = '\0';

        char *new_path = 0;
        char *wc_path = 0;
        char *wc_pre = 0;
        char *wc_ext = 0;
        char *temp = NULL;

        int pre_len;
        int new_path_len;
        int name_len = 0;
        int alloced = 0;
        int match_state = 0;

        pre_len = strlen(pre);

        if ((dh = opendir(path)) != NULL) {
                while ((dirptr = readdir(dh)) != NULL) {
                        match_state =
                            dotconf_question_mark_match(dirptr->d_name, pre,
                                                        ext);

                        if (match_state >= 0) {
                                name_len = strlen(dirptr->d_name);
                                new_path_len =
                                    strlen(path) + name_len + strlen(ext) + 1;

                                if (!alloced) {
                                        if ((new_path =
                                             (char *)malloc(new_path_len)) ==
                                            NULL) {
                                                return -1;
                                        }

                                        alloced = new_path_len;

                                } else {

                                        if (new_path_len > alloced) {
                                                temp =
                                                    realloc(new_path,
                                                            new_path_len);
                                                if (temp == NULL) {
                                                        free(new_path);
                                                        return -1;
                                                }
                                                new_path = temp;
                                                alloced = new_path_len;

                                        }

                                }

                                if (match_state == 1) {

                                        strncpy(new_pre, dirptr->d_name,
                                                (name_len >
                                                 pre_len) ? (pre_len +
                                                             1) : pre_len);
                                        new_pre[(name_len >
                                                 pre_len) ? (pre_len +
                                                             1) : pre_len] =
                                            '\0';

                                        sprintf(new_path, "%s%s%s", path,
                                                new_pre, ext);

                                        if (strcmp(new_path, already_matched) ==
                                            0) {
                                                continue;       /* Already searched this expression */

                                        } else {

                                                strcpy(already_matched,
                                                       new_path);

                                        }

                                        if (dotconf_find_wild_card
                                            (new_path, &wc, &wc_path, &wc_pre,
                                             &wc_ext) >= 0) {
                                                if (dotconf_handle_wild_card
                                                    (cmd, wc, wc_path, wc_pre,
                                                     wc_ext) < 0) {
                                                        dotconf_warning(cmd->
                                                                        configfile,
                                                                        DCLOG_WARNING,
                                                                        ERR_INCLUDE_ERROR,
                                                                        "Error occured while processing wildcard %c\n"
                                                                        "Filename is '%s'\n",
                                                                        wc,
                                                                        new_path);

                                                        free(new_path);
                                                        dotconf_wild_card_cleanup
                                                            (wc_path, wc_pre);
                                                        return -1;
                                                }

                                                dotconf_wild_card_cleanup
                                                    (wc_path, wc_pre);
                                                continue;
                                        }

                                }

                                sprintf(new_path, "%s%s", path, dirptr->d_name);

                                if (access(new_path, R_OK)) {
                                        dotconf_warning(cmd->configfile,
                                                        DCLOG_WARNING,
                                                        ERR_INCLUDE_ERROR,
                                                        "Cannot open %s for inclusion.\n"
                                                        "IncludePath is '%s'\n",
                                                        new_path,
                                                        cmd->configfile->
                                                        includepath);
                                        return -1;
                                }

                                included =
                                    dotconf_create(new_path,
                                                   cmd->configfile->
                                                   config_options[1],
                                                   cmd->configfile->context,
                                                   cmd->configfile->flags);
                                if (included) {
                                        for (i = 2;
                                             cmd->configfile->config_options[i];
                                             i++)
                                                dotconf_register_options
                                                    (included,
                                                     cmd->configfile->
                                                     config_options[i]);
                                        included->errorhandler =
                                            cmd->configfile->errorhandler;
                                        included->contextchecker =
                                            cmd->configfile->contextchecker;
                                        dotconf_command_loop(included);
                                        dotconf_cleanup(included);
                                }

                        }

                }

                closedir(dh);
                free(new_path);

        }

        return 0;
}

/* ------ internal utility function that determins matches for filenames with   --- */
/* ------ a '*' in name and calls the Internal Include function on that filename -- */
int dotconf_handle_star(command_t * cmd, char *path, char *pre, char *ext)
{
        configfile_t *included;
        DIR *dh = 0;
        struct dirent *dirptr = 0;

        char new_pre[CFG_MAX_FILENAME];
        char new_ext[CFG_MAX_FILENAME];
        char already_matched[CFG_MAX_FILENAME];

        char wc = '\0';

        char *new_path = 0;
        char *s_ext = 0;
        char *t_ext = 0;
        char *sub = 0;
        char *wc_path = 0;
        char *wc_pre = 0;
        char *wc_ext = 0;
        char *temp = NULL;

        int pre_len;
        int new_path_len;
        int name_len = 0;
        int alloced = 0;
        int match_state = 0;
        int t_ext_count = 0;
        int sub_count = 0;

        pre_len = strlen(pre);
        memset(already_matched, 0, CFG_MAX_FILENAME);
        s_ext = ext;

        while (dotconf_is_wild_card(*s_ext)) {  /* remove trailing wild-cards proceeded by * */
                s_ext++;
        }

        t_ext = s_ext;

        while (t_ext != NULL && !(dotconf_is_wild_card(*t_ext))
               && *t_ext != '\0') {
                t_ext++;        /* find non-wild-card string */
                t_ext_count++;
        }

        strncpy(new_ext, s_ext, t_ext_count);
        new_ext[t_ext_count] = '\0';

        if ((dh = opendir(path)) != NULL) {
                while ((dirptr = readdir(dh)) != NULL) {
                        sub_count = 0;
                        t_ext_count = 0;

                        match_state =
                            dotconf_star_match(dirptr->d_name, pre, s_ext);

                        if (match_state >= 0) {
                                name_len = strlen(dirptr->d_name);
                                new_path_len =
                                    strlen(path) + name_len + strlen(s_ext) + 1;

                                if (!alloced) {
                                        if ((new_path =
                                             (char *)malloc(new_path_len)) ==
                                            NULL) {
                                                return -1;
                                        }

                                        alloced = new_path_len;

                                } else {

                                        if (new_path_len > alloced) {
                                                temp =
                                                    realloc(new_path,
                                                            new_path_len);
                                                if (temp == NULL) {
                                                        free(new_path);
                                                        return -1;
                                                }
                                                new_path = temp;
                                                alloced = new_path_len;

                                        }

                                }

                                if (match_state == 1) {

                                        if ((sub =
                                             strstr((dirptr->d_name + pre_len),
                                                    new_ext)) == NULL) {
                                                continue;
                                        }

                                        while (sub != dirptr->d_name) {
                                                sub--;
                                                sub_count++;
                                        }

                                        if (sub_count + t_ext_count > name_len) {
                                                continue;
                                        }

                                        strncpy(new_pre, dirptr->d_name,
                                                (sub_count + t_ext_count));
                                        new_pre[sub_count + t_ext_count] = '\0';
                                        strcat(new_pre, new_ext);

                                        sprintf(new_path, "%s%s%s", path,
                                                new_pre, t_ext);

                                        if (strcmp(new_path, already_matched) ==
                                            0) {
                                                continue;       /* Already searched this expression */

                                        } else {

                                                strcpy(already_matched,
                                                       new_path);

                                        }

                                        if (dotconf_find_wild_card
                                            (new_path, &wc, &wc_path, &wc_pre,
                                             &wc_ext) >= 0) {
                                                if (dotconf_handle_wild_card
                                                    (cmd, wc, wc_path, wc_pre,
                                                     wc_ext) < 0) {
                                                        dotconf_warning(cmd->
                                                                        configfile,
                                                                        DCLOG_WARNING,
                                                                        ERR_INCLUDE_ERROR,
                                                                        "Error occured while processing wildcard %c\n"
                                                                        "Filename is '%s'\n",
                                                                        wc,
                                                                        new_path);

                                                        free(new_path);
                                                        dotconf_wild_card_cleanup
                                                            (wc_path, wc_pre);
                                                        return -1;
                                                }

                                                dotconf_wild_card_cleanup
                                                    (wc_path, wc_pre);
                                                continue;
                                        }

                                }

                                sprintf(new_path, "%s%s", path, dirptr->d_name);

                                if (access(new_path, R_OK)) {
                                        dotconf_warning(cmd->configfile,
                                                        DCLOG_WARNING,
                                                        ERR_INCLUDE_ERROR,
                                                        "Cannot open %s for inclusion.\n"
                                                        "IncludePath is '%s'\n",
                                                        new_path,
                                                        cmd->configfile->
                                                        includepath);
                                        return -1;
                                }

                                included =
                                    dotconf_create(new_path,
                                                   cmd->configfile->
                                                   config_options[1],
                                                   cmd->configfile->context,
                                                   cmd->configfile->flags);
                                if (included) {
                                        included->errorhandler =
                                            cmd->configfile->errorhandler;
                                        included->contextchecker =
                                            cmd->configfile->contextchecker;
                                        dotconf_command_loop(included);
                                        dotconf_cleanup(included);
                                }

                        }

                }

                closedir(dh);
                free(new_path);

        }

        return 0;
}

#endif  /* !WIN32 */

char *get_cwd(void)
{
        char *buf = calloc(1, CFG_MAX_FILENAME);

        if (buf == NULL)
                return NULL;
        getcwd(buf, CFG_MAX_FILENAME);
        return buf;
}

char *get_path(char *s)
{
        char *tmp;
        char *buf = NULL;
        int len = 0;

        tmp = strrchr(s, '/');
        if (tmp == NULL)
                return NULL;
        buf = calloc(1, CFG_MAX_FILENAME);
        if (buf == NULL)
                return NULL;
        if (tmp == s) {
                sprintf(buf, "/");
        } else {
                len = tmp - s + 1;
                if (len > CFG_MAX_FILENAME)
                        len = CFG_MAX_FILENAME;
        }
        snprintf(buf, len, "%s", s);
        return buf;
}

/* ------ callbacks of the internal option (Include, IncludePath) ------------------------------- */
DOTCONF_CB(dotconf_cb_include)
{
        char *filename = 0;
        configfile_t *included;
#ifndef WIN32
        char wild_card;
        char *path = 0;
        char *pre = 0;
        char *ext = 0;
#endif

        (void) ctx;

        if (cmd->configfile->includepath
            && cmd->data.str[0] != '/'
            && cmd->configfile->includepath[0] != '\0') {
                /* relative file AND include path is used */
                int len, inclen;
                char *sl;

                inclen = strlen(cmd->configfile->includepath);
                if ((len =
                     (strlen(cmd->data.str) + inclen + 1)) ==
                    CFG_MAX_FILENAME) {
                        dotconf_warning(cmd->configfile, DCLOG_WARNING,
                                        ERR_INCLUDE_ERROR,
                                        "Absolute filename too long (>%d)",
                                        CFG_MAX_FILENAME);
                        return NULL;
                }

                if (cmd->configfile->includepath[inclen - 1] == '/')
                        sl = "";
                else {
                        sl = "/";
                        len++;
                }

                filename = malloc(len);
                snprintf(filename, len, "%s%s%s",
                         cmd->configfile->includepath, sl, cmd->data.str);
        } else                  /* fully qualified, or no includepath */
                filename = strdup(cmd->data.str);

        /* Added wild card support here */
#ifndef WIN32
        if (dotconf_find_wild_card(filename, &wild_card, &path, &pre, &ext) >=
            0) {
                if (dotconf_handle_wild_card(cmd, wild_card, path, pre, ext) <
                    0) {
                        dotconf_warning(cmd->configfile, DCLOG_WARNING,
                                        ERR_INCLUDE_ERROR,
                                        "Error occured while attempting to process %s for inclusion.\n"
                                        "IncludePath is '%s'\n", filename,
                                        cmd->configfile->includepath);
                }

                dotconf_wild_card_cleanup(path, pre);
                free(filename);
                return NULL;
        }

        if (access(filename, R_OK)) {
                dotconf_warning(cmd->configfile, DCLOG_WARNING,
                                ERR_INCLUDE_ERROR,
                                "Cannot open %s for inclusion.\n"
                                "IncludePath is '%s'\n", filename,
                                cmd->configfile->includepath);
                free(filename);
                return NULL;
        }
#endif  /* !WIN32 */

        included = dotconf_create(filename, cmd->configfile->config_options[1],
                                  cmd->configfile->context,
                                  cmd->configfile->flags);
        if (included) {
                included->contextchecker =
                    (dotconf_contextchecker_t) cmd->configfile->contextchecker;
                included->errorhandler =
                    (dotconf_errorhandler_t) cmd->configfile->errorhandler;

                dotconf_command_loop(included);
                dotconf_cleanup(included);
        }

        free(filename);
        return NULL;
}

DOTCONF_CB(dotconf_cb_includepath)
{
        char *env = getenv(CFG_INCLUDEPATH_ENV);
        (void) ctx;
        /* environment overrides configuration file setting */
        if (!env)
                snprintf(cmd->configfile->includepath, CFG_MAX_FILENAME, "%s",
                         cmd->data.str);
        return NULL;
}
