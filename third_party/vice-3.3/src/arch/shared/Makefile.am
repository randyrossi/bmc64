# vim: set noet ts=8:
#
# Makefile.am for the shared archdep code


AM_CPPFLAGS = \
	@ARCH_INCLUDES@ \
	$(extra_includes) \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src


noinst_LIBRARIES = libarchdep.a

libarchdep_a_SOURCES = \
	archdep_atexit.c \
	archdep_boot_path.c \
	archdep_cbmfont.c \
	archdep_create_user_config_dir.c \
	archdep_default_autostart_disk_image_file_name.c \
	archdep_default_fliplist_file_name.c \
	archdep_default_resource_file_name.c \
	archdep_default_rtc_file_name.c \
	archdep_default_sysfile_pathlist.c \
	archdep_expand_path.c \
	archdep_extra_title_text.c \
	archdep_file_is_blockdev.c \
	archdep_file_is_chardev.c \
	archdep_filename_parameter.c \
	archdep_fix_permissions.c \
	archdep_get_vice_datadir.c \
	archdep_get_vice_docsdir.c \
	archdep_home_path.c \
	archdep_join_paths.c \
	archdep_make_backup_filename.c \
	archdep_mkdir.c \
	archdep_open_default_log_file.c \
	archdep_path_is_relative.c \
	archdep_program_name.c \
	archdep_program_path.c \
	archdep_quote_parameter.c \
	archdep_rename.c \
	archdep_rmdir.c \
	archdep_sanitize_filename.c \
	archdep_startup_log_error.c \
	archdep_stat.c \
	archdep_tmpnam.c \
	archdep_user_config_path.c \
	archdep_xdg.c

EXTRA_DIST = \
	archdep_defs.h \
	archdep_atexit.h \
	archdep_boot_path.h \
	archdep_cbmfont.h \
	archdep_create_user_config_dir.h \
	archdep_default_autostart_disk_image_file_name.h \
	archdep_default_fliplist_file_name.h \
	archdep_default_resource_file_name.h \
	archdep_default_rtc_file_name.h \
	archdep_default_sysfile_pathlist.h \
	archdep_expand_path.h \
	archdep_extra_title_text.h \
	archdep_file_is_blockdev.h \
	archdep_file_is_chardev.h \
	archdep_filename_parameter.h \
	archdep_fix_permissions.h \
	archdep_get_vice_datadir.h \
	archdep_get_vice_docsdir.h \
	archdep_home_path.h \
	archdep_join_paths.h \
	archdep_make_backup_filename.h \
	archdep_mkdir.h \
	archdep_open_default_log_file.h \
	archdep_path_is_relative.h \
	archdep_program_name.h \
	archdep_program_path.h \
	archdep_quote_parameter.h \
	archdep_rename.h \
	archdep_rmdir.h \
	archdep_sanitize_filename.h \
	archdep_startup_log_error.h \
	archdep_stat.h \
	archdep_tmpnam.h \
	archdep_user_config_path.h \
	archdep_xdg.h
