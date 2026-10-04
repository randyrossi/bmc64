// Tests for src/profiles. Rule numbers ([P2], [F1], ...) refer to the
// behaviour reference in docs/PROFILES.md. Profile files are created in a
// temporary folder that stands in for the SD card
// (tools/sdcard/sd_fs_posix.c, given the folder in SD_TEST_ROOT).

#define _POSIX_C_SOURCE 200809L

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../../src/sdcard/sd_fs.h"
#include "profiles_internal.h"

// The temporary folder standing in for the SD card.
static char card_root[512];

static int failures;

#define CHECK(expr)                                                         \
   do {                                                                     \
      if (!(expr)) {                                                        \
         fprintf(stderr, "%s:%d: FAILED: %s\n", __FILE__, __LINE__, #expr); \
         failures++;                                                        \
      }                                                                     \
   } while (0)

#define CHECK_STR(actual, expected)                                         \
   do {                                                                     \
      const char *a_ = (actual);                                            \
      const char *e_ = (expected);                                          \
      if (a_ == NULL || strcmp(a_, e_) != 0) {                              \
         fprintf(stderr, "%s:%d: FAILED: %s is \"%s\", expected \"%s\"\n",  \
                 __FILE__, __LINE__, #actual, a_ ? a_ : "(null)", e_);      \
         failures++;                                                        \
      }                                                                     \
   } while (0)

// ---- Test card ----

static void remove_tree(const char *path) {
   DIR *dir = opendir(path);
   if (dir != NULL) {
      struct dirent *entry;
      while ((entry = readdir(dir)) != NULL) {
         if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
         }
         char child[1024];
         snprintf(child, sizeof(child), "%s/%s", path, entry->d_name);
         remove_tree(child);
      }
      closedir(dir);
      rmdir(path);
   } else {
      unlink(path);
   }
}

// Empties the test card.
static void fresh_card(void) {
   DIR *dir = opendir(card_root);
   if (dir != NULL) {
      struct dirent *entry;
      while ((entry = readdir(dir)) != NULL) {
         if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
         }
         char child[1024];
         snprintf(child, sizeof(child), "%s/%s", card_root, entry->d_name);
         remove_tree(child);
      }
      closedir(dir);
   }
}

static void write_file(const char *path, const char *text) {
   char dir[256];
   snprintf(dir, sizeof(dir), "%s", path);
   char *slash = strrchr(dir, '/');
   if (slash && slash != dir) {
      *slash = '\0';
      // Make the parent folders.
      for (char *c = dir + 1; *c; c++) {
         if (*c == '/') {
            *c = '\0';
            sd_mkdir(dir);
            *c = '/';
         }
      }
      sd_mkdir(dir);
   }
   CHECK(sd_write_file(path, text, (int)strlen(text)) == 0);
}

static const char *read_file(const char *path) {
   static char text[4096];
   if (sd_read_file(path, text, sizeof(text)) < 0) {
      return NULL;
   }
   return text;
}

static int exists(const char *path) {
   return sd_stat(path, NULL, NULL) == 0;
}

static void make_profile(const char *id, const char *text) {
   char path[256];
   snprintf(path, sizeof(path), "/profiles/%s/profile.txt", id);
   write_file(path, text);
}

// ---- key=value parser ----

typedef struct {
   int count;
   char keys[16][64];
   char values[16][256];
} Collected;

static void collect(void *ctx, const char *key, const char *value) {
   Collected *c = (Collected *)ctx;
   if (c->count < 16) {
      snprintf(c->keys[c->count], sizeof(c->keys[0]), "%s", key);
      snprintf(c->values[c->count], sizeof(c->values[0]), "%s", value);
      c->count++;
   }
}

static Collected parse(const char *text) {
   char copy[1024];
   Collected c;
   memset(&c, 0, sizeof(c));
   snprintf(copy, sizeof(copy), "%s", text);
   pkv_parse(copy, collect, &c);
   return c;
}

static void test_kv_parse(void) {
   Collected c = parse("name=GEOS\nmachine=C64\n");
   CHECK(c.count == 2);
   CHECK_STR(c.keys[0], "name");
   CHECK_STR(c.values[0], "GEOS");
   CHECK_STR(c.keys[1], "machine");
   CHECK_STR(c.values[1], "C64");

   // Spaces around keys and values are ignored, spaces inside values kept.
   c = parse("  name =  My Game  \n\tmachine\t=\tC64/PAL/HDMI/VICE 720p@50Hz\t\n");
   CHECK(c.count == 2);
   CHECK_STR(c.keys[0], "name");
   CHECK_STR(c.values[0], "My Game");
   CHECK_STR(c.values[1], "C64/PAL/HDMI/VICE 720p@50Hz");

   // Windows line endings.
   c = parse("name=GEOS\r\nmachine=C64\r\n");
   CHECK(c.count == 2);
   CHECK_STR(c.values[0], "GEOS");
   CHECK_STR(c.values[1], "C64");

   // Comment lines, blank lines, and lines without '=' are skipped.
   c = parse("# a comment\n\n   \n  # indented comment\nno equals here\nname=x\n");
   CHECK(c.count == 1);
   CHECK_STR(c.keys[0], "name");

   // '#' only starts a comment at the start of a line.
   c = parse("name=Game #2\n");
   CHECK(c.count == 1);
   CHECK_STR(c.values[0], "Game #2");

   // Only the first '=' splits.
   c = parse("description=a=b=c\n");
   CHECK(c.count == 1);
   CHECK_STR(c.values[0], "a=b=c");

   // An empty key is skipped; an empty value is passed on.
   c = parse("=value\nname=\n");
   CHECK(c.count == 1);
   CHECK_STR(c.keys[0], "name");
   CHECK_STR(c.values[0], "");

   // No newline at the end.
   c = parse("name=last");
   CHECK(c.count == 1);
   CHECK_STR(c.values[0], "last");

   // Nothing at all.
   c = parse("");
   CHECK(c.count == 0);
}

static void test_kv_append(void) {
   char buf[64];
   int len = 0;
   buf[0] = '\0';
   CHECK(pkv_append(buf, sizeof(buf), &len, "name", "GEOS") == 0);
   CHECK(pkv_append(buf, sizeof(buf), &len, "empty", "") == 0);
   CHECK(pkv_append(buf, sizeof(buf), &len, "none", NULL) == 0);
   CHECK_STR(buf, "name=GEOS\n");
   CHECK(len == (int)strlen(buf));

   // Values never span lines.
   CHECK(pkv_append(buf, sizeof(buf), &len, "d", "a\nb\rc") == 0);
   CHECK_STR(buf, "name=GEOS\nd=a b c\n");

   // What doesn't fit isn't written.
   int before = len;
   CHECK(pkv_append(buf, sizeof(buf), &len, "long",
                    "0123456789012345678901234567890123456789") == -1);
   CHECK(len == before);
   CHECK_STR(buf, "name=GEOS\nd=a b c\n");

   char out[8];
   pkv_copy(out, sizeof(out), "abcdefghij");
   CHECK_STR(out, "abcdefg");
   pkv_copy(out, sizeof(out), "a\nb");
   CHECK_STR(out, "a b");
}

// ---- profile.txt ----

static void test_profile_parse(void) {
   char text[1024];
   ProfileFile pf;

   snprintf(text, sizeof(text),
            "# GEOS\n"
            "name=GEOS 2.0\n"
            "description=GEOS with a 1764 REU\n"
            "category=Applications\n"
            "machine=C64/PAL/HDMI/VICE 720p@50Hz\n"
            "start=once\n"
            "disk_8=/disks/geos/GEOS64.D81\n"
            "disk_9=/disks/geos/APPS.D81\n"
            "disk_11=/disks/data.d64\n"
            "autostart=/disks/geos/GEOS64.D81\n"
            "unknown=ignored\n");
   CHECK(profile_file_parse(text, &pf) == PROFILES_OK);
   CHECK_STR(pf.info.name, "GEOS 2.0");
   CHECK_STR(pf.description, "GEOS with a 1764 REU");
   CHECK_STR(pf.info.category, "Applications");
   CHECK_STR(pf.info.machine, "C64/PAL/HDMI/VICE 720p@50Hz");
   CHECK(pf.info.start == PROFILE_START_ONCE);
   CHECK_STR(pf.disks[0], "/disks/geos/GEOS64.D81");
   CHECK_STR(pf.disks[1], "/disks/geos/APPS.D81");
   CHECK_STR(pf.disks[2], "");
   CHECK_STR(pf.disks[3], "/disks/data.d64");
   CHECK_STR(pf.autostart, "/disks/geos/GEOS64.D81");

   // name and machine are required.
   snprintf(text, sizeof(text), "machine=C64\n");
   CHECK(profile_file_parse(text, &pf) == PROFILES_ERROR);
   snprintf(text, sizeof(text), "name=No machine\n");
   CHECK(profile_file_parse(text, &pf) == PROFILES_ERROR);
   snprintf(text, sizeof(text), "name=\nmachine=C64\n");
   CHECK(profile_file_parse(text, &pf) == PROFILES_ERROR);
   snprintf(text, sizeof(text), "%s", "");
   CHECK(profile_file_parse(text, &pf) == PROFILES_ERROR);

   // start defaults to switch; only "once" means once.
   snprintf(text, sizeof(text), "name=a\nmachine=C64\n");
   CHECK(profile_file_parse(text, &pf) == PROFILES_OK);
   CHECK(pf.info.start == PROFILE_START_SWITCH);
   snprintf(text, sizeof(text), "name=a\nmachine=C64\nstart=Once\n");
   profile_file_parse(text, &pf);
   CHECK(pf.info.start == PROFILE_START_SWITCH);
   snprintf(text, sizeof(text), "name=a\nmachine=C64\nstart=switch\n");
   profile_file_parse(text, &pf);
   CHECK(pf.info.start == PROFILE_START_SWITCH);

   // Only drives 8 to 11, spelled exactly.
   snprintf(text, sizeof(text),
            "name=a\nmachine=C64\ndisk_7=x\ndisk_12=x\ndisk_08=x\ndisk_8x=x\n"
            "disk_=x\ndisk_10=/ten.d64\n");
   CHECK(profile_file_parse(text, &pf) == PROFILES_OK);
   CHECK_STR(pf.disks[0], "");
   CHECK_STR(pf.disks[1], "");
   CHECK_STR(pf.disks[2], "/ten.d64");
   CHECK_STR(pf.disks[3], "");

   // A long name is cut to 32 characters.
   snprintf(text, sizeof(text),
            "name=0123456789012345678901234567890123456789\nmachine=C64\n");
   CHECK(profile_file_parse(text, &pf) == PROFILES_OK);
   CHECK(strlen(pf.info.name) == PROFILES_MAX_NAME_LEN);

   // The last of a repeated key wins.
   snprintf(text, sizeof(text), "name=first\nname=second\nmachine=C64\n");
   profile_file_parse(text, &pf);
   CHECK_STR(pf.info.name, "second");

   // Windows line endings.
   snprintf(text, sizeof(text), "name=Win\r\nmachine=C64\r\n");
   CHECK(profile_file_parse(text, &pf) == PROFILES_OK);
   CHECK_STR(pf.info.name, "Win");
   CHECK_STR(pf.info.machine, "C64");
}

static void test_profile_format(void) {
   ProfileFile pf;
   memset(&pf, 0, sizeof(pf));
   strcpy(pf.info.name, "Elite");
   strcpy(pf.info.machine, "C64");
   char text[PROFILES_MAX_FILE_LEN];

   // Empty values and the default start aren't written.
   CHECK(profile_file_format(&pf, text, sizeof(text)) > 0);
   CHECK_STR(text, "name=Elite\nmachine=C64\n");

   strcpy(pf.description, "A game");
   strcpy(pf.info.category, "Games");
   pf.info.start = PROFILE_START_ONCE;
   strcpy(pf.disks[0], "/disks/elite.d64");
   strcpy(pf.disks[3], "/disks/save.d64");
   strcpy(pf.autostart, "/C64/snapshots/elite.vsf");
   CHECK(profile_file_format(&pf, text, sizeof(text)) > 0);
   CHECK_STR(text,
             "name=Elite\ndescription=A game\ncategory=Games\nmachine=C64\n"
             "start=once\ndisk_8=/disks/elite.d64\ndisk_11=/disks/save.d64\n"
             "autostart=/C64/snapshots/elite.vsf\n");

   // Writing and reading back gives the same profile.
   ProfileFile back;
   CHECK(profile_file_parse(text, &back) == PROFILES_OK);
   CHECK_STR(back.info.name, pf.info.name);
   CHECK_STR(back.description, pf.description);
   CHECK_STR(back.info.category, pf.info.category);
   CHECK_STR(back.info.machine, pf.info.machine);
   CHECK(back.info.start == pf.info.start);
   for (int i = 0; i < PROFILES_NUM_DRIVES; i++) {
      CHECK_STR(back.disks[i], pf.disks[i]);
   }
   CHECK_STR(back.autostart, pf.autostart);

   // A buffer that's too small is an error, not a cut-off file.
   CHECK(profile_file_format(&pf, text, 40) == PROFILES_ERROR);
}

// ---- active.txt ----

static void test_active_parse(void) {
   char text[256];
   ActiveFile af;

   snprintf(text, sizeof(text), "%s", "");
   active_file_parse(text, &af);
   CHECK_STR(af.profile, "main");
   CHECK_STR(af.once, "");

   snprintf(text, sizeof(text), "profile=geos\n");
   active_file_parse(text, &af);
   CHECK_STR(af.profile, "geos");
   CHECK_STR(af.once, "");

   snprintf(text, sizeof(text), "# comment\r\nprofile=geos\r\nonce=elite\r\n");
   active_file_parse(text, &af);
   CHECK_STR(af.profile, "geos");
   CHECK_STR(af.once, "elite");

   // Invalid ids are ignored: no paths, upper case or long names.
   snprintf(text, sizeof(text),
            "profile=../system\nonce=/profiles/x\n");
   active_file_parse(text, &af);
   CHECK_STR(af.profile, "main");
   CHECK_STR(af.once, "");
   snprintf(text, sizeof(text), "profile=GEOS\nonce=%s\n",
            "a23456789012345678901234567890123");
   active_file_parse(text, &af);
   CHECK_STR(af.profile, "main");
   CHECK_STR(af.once, "");
   snprintf(text, sizeof(text), "profile=\nonce=\n");
   active_file_parse(text, &af);
   CHECK_STR(af.profile, "main");
   CHECK_STR(af.once, "");
}

// ---- Ids ----

static void test_ids(void) {
   CHECK(profiles_id_valid("geos"));
   CHECK(profiles_id_valid("vic20-pal-16k"));
   CHECK(profiles_id_valid("a2345678901234567890123456789012"));
   CHECK(!profiles_id_valid("a23456789012345678901234567890123"));
   CHECK(!profiles_id_valid(""));
   CHECK(!profiles_id_valid(NULL));
   CHECK(!profiles_id_valid("GEOS"));
   CHECK(!profiles_id_valid("my game"));
   CHECK(!profiles_id_valid("../x"));
   CHECK(!profiles_id_valid("a/b"));

   fresh_card();
   char id[PROFILES_MAX_ID_LEN + 1];
   CHECK(profiles_make_id("GEOS 2.0", id, sizeof(id)) == PROFILES_OK);
   CHECK_STR(id, "geos-2-0");
   profiles_make_id("  Elite!! ", id, sizeof(id));
   CHECK_STR(id, "elite");
   profiles_make_id("PAL VIC-20 + 16K", id, sizeof(id));
   CHECK_STR(id, "pal-vic-20-16k");
   profiles_make_id("Über Spiel", id, sizeof(id));
   CHECK_STR(id, "ber-spiel");
   profiles_make_id("", id, sizeof(id));
   CHECK_STR(id, "profile");
   profiles_make_id("!!!", id, sizeof(id));
   CHECK_STR(id, "profile");
   // main is reserved ([P5]).
   profiles_make_id("Main", id, sizeof(id));
   CHECK_STR(id, "main-2");

   // Ids in use get -2, -3, ...
   make_profile("elite", "name=Elite\nmachine=C64\n");
   profiles_make_id("Elite", id, sizeof(id));
   CHECK_STR(id, "elite-2");
   make_profile("elite-2", "name=Elite\nmachine=C64\n");
   profiles_make_id("ELITE", id, sizeof(id));
   CHECK_STR(id, "elite-3");

   // Up to 32 characters, including the -2.
   const char *long_name = "A very long profile name for testing ids";
   profiles_make_id(long_name, id, sizeof(id));
   CHECK_STR(id, "a-very-long-profile-name-for-tes");
   make_profile(id, "name=x\nmachine=C64\n");
   profiles_make_id(long_name, id, sizeof(id));
   CHECK_STR(id, "a-very-long-profile-name-for-t-2");
   CHECK(strlen(id) == PROFILES_MAX_ID_LEN);
}

// ---- Start-up and the running profile ----

static void test_no_profiles(void) {
   fresh_card();
   profiles_boot_init("C64");
   // [P1], [F1]: Main, and nothing created on the card.
   CHECK(profiles_running_is_main());
   CHECK_STR(profiles_running()->id, "main");
   CHECK_STR(profiles_running()->name, "Main");
   CHECK_STR(profiles_running()->machine, "");
   CHECK_STR(profiles_boot_message(), "");
   CHECK(profiles_vice_config() == NULL);
   CHECK_STR(profiles_settings_file("/settings-vic20.txt"),
             "/settings-vic20.txt");
   CHECK_STR(profiles_autostart(), "");
   profiles_after_boot();
   profiles_before_reboot();
   profiles_reset_to_main();
   CHECK(!exists("/profiles"));

   // [P2]: Main is always in the list.
   CHECK(profiles_list_open() == 1);
   CHECK_STR(profiles_list_at(0)->id, "main");
   CHECK(profiles_list_at(1) == NULL);
   CHECK(profiles_list_at(-1) == NULL);
   profiles_list_close();
}

static void test_boot_profile(void) {
   fresh_card();
   make_profile("geos", "name=GEOS\nmachine=C64/PAL/HDMI/VICE 720p@50Hz\n");
   write_file("/profiles/active.txt", "profile=geos\n");
   profiles_boot_init("C64");
   CHECK(!profiles_running_is_main());
   CHECK_STR(profiles_running()->id, "geos");
   CHECK_STR(profiles_running()->name, "GEOS");
   CHECK_STR(profiles_boot_message(), "");
   CHECK_STR(profiles_vice_config(), "/profiles/geos/vice.ini");
   CHECK_STR(profiles_settings_file("/settings.txt"),
             "/profiles/geos/settings.txt");
   // The machine name is matched without regard to case.
   profiles_boot_init("c64");
   CHECK_STR(profiles_running()->id, "geos");

   // [S6]: a missing or invalid profile starts Main, with a message.
   write_file("/profiles/active.txt", "profile=gone\n");
   profiles_boot_init("C64");
   CHECK(profiles_running_is_main());
   CHECK(strstr(profiles_boot_message(), "gone") != NULL);
   make_profile("broken", "name=No machine\n");
   write_file("/profiles/active.txt", "profile=broken\n");
   profiles_boot_init("C64");
   CHECK(profiles_running_is_main());
   CHECK(profiles_boot_message()[0] != '\0');

   // A profile for another machine starts Main, with a message.
   make_profile("vic", "name=PAL 16K\nmachine=VIC20/PAL/HDMI/VICE 720p@50Hz\n");
   write_file("/profiles/active.txt", "profile=vic\n");
   profiles_boot_init("C64");
   CHECK(profiles_running_is_main());
   CHECK(strstr(profiles_boot_message(), "PAL 16K") != NULL);
   profiles_boot_init("VIC20");
   CHECK_STR(profiles_running()->id, "vic");

   // profile=main is Main without a message.
   write_file("/profiles/active.txt", "profile=main\n");
   profiles_boot_init("C64");
   CHECK(profiles_running_is_main());
   CHECK_STR(profiles_boot_message(), "");
}

static void test_start_once(void) {
   fresh_card();
   make_profile("geos", "name=GEOS\nmachine=C64\n");
   make_profile("elite", "name=Elite\nmachine=C64\n");
   write_file("/profiles/active.txt", "profile=geos\n");
   profiles_boot_init("C64");

   // [S2]: once is added, profile is unchanged.
   CHECK(profiles_start_once("elite") == PROFILES_OK);
   CHECK_STR(read_file("/profiles/active.txt"), "profile=geos\nonce=elite\n");

   // [S3]: the once profile starts; once stays until after boot ([F6]).
   profiles_boot_init("C64");
   CHECK_STR(profiles_running()->id, "elite");
   CHECK_STR(read_file("/profiles/active.txt"), "profile=geos\nonce=elite\n");
   profiles_after_boot();
   CHECK_STR(read_file("/profiles/active.txt"), "profile=geos\n");

   // [S4]: a restart BMC64 asks for keeps the once profile.
   profiles_before_reboot();
   CHECK_STR(read_file("/profiles/active.txt"), "profile=geos\nonce=elite\n");
   profiles_boot_init("C64");
   CHECK_STR(profiles_running()->id, "elite");
   profiles_after_boot();

   // The next power-on is back on GEOS.
   profiles_boot_init("C64");
   CHECK_STR(profiles_running()->id, "geos");
   // A switched-to profile isn't put back as once.
   profiles_before_reboot();
   CHECK_STR(read_file("/profiles/active.txt"), "profile=geos\n");

   // A once profile that can't start still has its entry removed.
   write_file("/profiles/active.txt", "profile=geos\nonce=gone\n");
   profiles_boot_init("C64");
   CHECK(profiles_running_is_main());
   profiles_after_boot();
   CHECK_STR(read_file("/profiles/active.txt"), "profile=geos\n");

   // Start once from Main with no active.txt.
   fresh_card();
   make_profile("elite", "name=Elite\nmachine=C64\n");
   profiles_boot_init("C64");
   CHECK(profiles_start_once("elite") == PROFILES_OK);
   CHECK_STR(read_file("/profiles/active.txt"), "profile=main\nonce=elite\n");
}

static void test_switch_to(void) {
   fresh_card();
   make_profile("geos", "name=GEOS\nmachine=C64\n");
   make_profile("vic", "name=PAL 16K\nmachine=VIC20\n");
   profiles_boot_init("C64");

   // [S1]
   CHECK(profiles_switch_to("geos") == PROFILES_OK);
   CHECK_STR(read_file("/profiles/active.txt"), "profile=geos\n");
   CHECK(!exists("/profiles/active.new"));
   // Switching clears a pending once.
   write_file("/profiles/active.txt", "profile=geos\nonce=geos\n");
   CHECK(profiles_switch_to("main") == PROFILES_OK);
   CHECK_STR(read_file("/profiles/active.txt"), "profile=main\n");

   // Missing, invalid and other-machine profiles aren't started.
   CHECK(profiles_switch_to("gone") == PROFILES_ERROR);
   CHECK(profiles_switch_to("../x") == PROFILES_ERROR);
   CHECK(profiles_switch_to(NULL) == PROFILES_ERROR);
   CHECK(profiles_start_once("gone") == PROFILES_ERROR);
   CHECK(profiles_switch_to("vic") == PROFILES_NOT_IMPLEMENTED);
   CHECK(profiles_start_once("vic") == PROFILES_NOT_IMPLEMENTED);
   CHECK_STR(read_file("/profiles/active.txt"), "profile=main\n");
}

static void test_safe_mode(void) {
   fresh_card();
   make_profile("geos", "name=GEOS\nmachine=C64\n");
   write_file("/profiles/active.txt", "profile=geos\nonce=geos\n");
   // [S7]
   profiles_reset_to_main();
   CHECK_STR(read_file("/profiles/active.txt"), "profile=main\n");
}

// ---- The list ----

static void test_list(void) {
   fresh_card();
   make_profile("zork", "name=zork\nmachine=C64\ncategory=Games\n");
   make_profile("elite", "name=Elite\nmachine=C64\ncategory=Games\nstart=once\n");
   make_profile("vic", "name=PAL 16K\nmachine=VIC20/PAL/HDMI/VICE 720p@50Hz\n");
   make_profile("broken", "name=No machine\n");
   make_profile("UPPER", "name=Bad id\nmachine=C64\n");
   make_profile("main", "name=Not Main\nmachine=C64\n");
   write_file("/profiles/main/c64.txt", "disk_8=/x.d64\n");
   write_file("/profiles/notes.txt", "a file, not a profile\n");
   write_file("/profiles/active.txt", "profile=main\n");
   sd_mkdir("/profiles/empty");
   profiles_boot_init("C64");

   // Main first, then valid profiles of every machine sorted by name.
   CHECK(profiles_list_open() == 4);
   CHECK_STR(profiles_list_at(0)->id, "main");
   CHECK_STR(profiles_list_at(1)->id, "elite");
   CHECK_STR(profiles_list_at(2)->id, "vic");
   CHECK_STR(profiles_list_at(3)->id, "zork");
   CHECK(profiles_list_at(4) == NULL);
   CHECK_STR(profiles_list_at(1)->name, "Elite");
   CHECK_STR(profiles_list_at(1)->category, "Games");
   CHECK(profiles_list_at(1)->start == PROFILE_START_ONCE);
   CHECK_STR(profiles_list_at(2)->machine, "VIC20/PAL/HDMI/VICE 720p@50Hz");
   profiles_list_close();
   // Closing frees the list; Main is still there.
   CHECK_STR(profiles_list_at(0)->id, "main");
   CHECK(profiles_list_at(1) == NULL);
   profiles_list_close();

   // Lots of profiles.
   fresh_card();
   for (int i = 0; i < 40; i++) {
      char id[16];
      char text[64];
      snprintf(id, sizeof(id), "p%02d", i);
      snprintf(text, sizeof(text), "name=Profile %02d\nmachine=C64\n", i);
      make_profile(id, text);
   }
   CHECK(profiles_list_open() == 41);
   CHECK_STR(profiles_list_at(40)->id, "p39");
   profiles_list_close();
}

// ---- Create, rename, delete ----

static void test_create(void) {
   fresh_card();
   profiles_boot_init("C64");
   char id[PROFILES_MAX_ID_LEN + 1];
   // [P6]: Main's new profiles are for the booted machine.
   CHECK(profiles_create("  My GEOS  ", id, sizeof(id)) == PROFILES_OK);
   CHECK_STR(id, "my-geos");
   CHECK_STR(read_file("/profiles/my-geos/profile.txt"),
             "name=My GEOS\nmachine=C64\n");
   // No active.txt yet: creating doesn't switch.
   CHECK(!exists("/profiles/active.txt"));

   CHECK(profiles_create("My GEOS", id, sizeof(id)) == PROFILES_OK);
   CHECK_STR(id, "my-geos-2");
   CHECK(profiles_create("   ", id, sizeof(id)) == PROFILES_ERROR);
   CHECK(profiles_create("", id, sizeof(id)) == PROFILES_ERROR);

   // A new profile made in a profile copies its machine entry.
   make_profile("pal", "name=PAL\nmachine=C64/PAL/HDMI/VICE 720p@50Hz\n");
   write_file("/profiles/active.txt", "profile=pal\n");
   profiles_boot_init("C64");
   CHECK(profiles_create("Copy", id, sizeof(id)) == PROFILES_OK);
   CHECK_STR(read_file("/profiles/copy/profile.txt"),
             "name=Copy\nmachine=C64/PAL/HDMI/VICE 720p@50Hz\n");

   // Names are cut to 32 characters.
   CHECK(profiles_create("0123456789012345678901234567890123456789", id,
                         sizeof(id)) == PROFILES_OK);
   ProfileFile pf;
   CHECK(profile_file_read(id, &pf) == PROFILES_OK);
   CHECK(strlen(pf.info.name) == PROFILES_MAX_NAME_LEN);
}

static void test_rename(void) {
   fresh_card();
   make_profile("geos", "name=GEOS\nmachine=C64\ncategory=Apps\n"
                        "disk_8=/disks/geos.d81\n");
   write_file("/profiles/active.txt", "profile=geos\n");
   profiles_boot_init("C64");

   // [P5]: only the name changes; the id and other keys stay.
   CHECK(profiles_rename_running(" GEOS 2.0 ") == PROFILES_OK);
   CHECK_STR(profiles_running()->name, "GEOS 2.0");
   CHECK_STR(profiles_running()->id, "geos");
   ProfileFile pf;
   CHECK(profile_file_read("geos", &pf) == PROFILES_OK);
   CHECK_STR(pf.info.name, "GEOS 2.0");
   CHECK_STR(pf.info.category, "Apps");
   CHECK_STR(pf.disks[0], "/disks/geos.d81");
   CHECK(!exists("/profiles/geos-2-0"));

   CHECK(profiles_rename_running("") == PROFILES_ERROR);
   CHECK(profiles_rename_running("  ") == PROFILES_ERROR);
   CHECK_STR(profiles_running()->name, "GEOS 2.0");

   // [P2]: Main can't be renamed.
   fresh_card();
   profiles_boot_init("C64");
   CHECK(profiles_rename_running("Other") == PROFILES_ERROR);
   CHECK_STR(profiles_running()->name, "Main");
}

static void test_delete(void) {
   fresh_card();
   make_profile("geos", "name=GEOS\nmachine=C64\n");
   make_profile("elite", "name=Elite\nmachine=C64\n");
   write_file("/profiles/elite/vice.ini", "[C64]\n");
   write_file("/profiles/elite/vice.in~", "[C64]\n");
   write_file("/profiles/elite/settings.txt", "palette=1\n");
   write_file("/disks/elite.d64", "disk image");
   write_file("/profiles/active.txt", "profile=geos\n");
   profiles_boot_init("C64");

   // [P2], [P3]: not Main, not the running profile, nothing invalid.
   CHECK(profiles_delete("main") == PROFILES_ERROR);
   CHECK(profiles_delete("geos") == PROFILES_ERROR);
   CHECK(exists("/profiles/geos/profile.txt"));
   CHECK(profiles_delete("../disks") == PROFILES_ERROR);
   CHECK(profiles_delete("") == PROFILES_ERROR);
   CHECK(profiles_delete(NULL) == PROFILES_ERROR);
   CHECK(profiles_delete("gone") == PROFILES_ERROR);

   // [P4]: the profile's own files and folder go, nothing else.
   CHECK(profiles_delete("elite") == PROFILES_OK);
   CHECK(!exists("/profiles/elite"));
   CHECK(exists("/disks/elite.d64"));
   CHECK(exists("/profiles/geos/profile.txt"));

   // A folder holding anything else is kept, with that file.
   make_profile("keep", "name=Keep\nmachine=C64\n");
   write_file("/profiles/keep/my-notes.txt", "mine");
   CHECK(profiles_delete("keep") == PROFILES_ERROR);
   CHECK(exists("/profiles/keep/my-notes.txt"));
}

// ---- Shared settings and machines ----

static void test_shared_settings(void) {
   static const char *const shared[] = {
      "network_device", "timezone_offset_minutes", "network_modem_address",
      "webui_enabled", "webui_pin", "hotkey_cf1", "hotkey_cf3", "hotkey_cf5",
      "hotkey_cf7", "hotkey_tf1", "hotkey_tf3", "hotkey_tf5", "hotkey_tf7",
      "volume", "overlay", "overlay_padding", "vkbd_trans", "reset_confirm",
      "dir_convention", "drive_flush",
   };
   static const char *const per_profile[] = {
      "port_1", "palette", "gpio_config", "custom_gpio", "keyboard_layout",
      "keyboard_layout_mapping", "drive_type_8", "usb_0", "key_binding_1",
      "h_center_0", "tapereset", "s_scanlines", "",
   };
   for (unsigned i = 0; i < sizeof(shared) / sizeof(shared[0]); i++) {
      CHECK(profiles_is_shared_setting(shared[i]));
   }
   for (unsigned i = 0; i < sizeof(per_profile) / sizeof(per_profile[0]); i++) {
      CHECK(!profiles_is_shared_setting(per_profile[i]));
   }
   CHECK(!profiles_is_shared_setting(NULL));
}

static void check_label(const char *machine, const char *expected) {
   char label[16];
   profiles_machine_label(machine, label, sizeof(label));
   CHECK_STR(label, expected);
}

static void test_machines(void) {
   check_label("C64/PAL/HDMI/VICE 720p@50Hz", "C64");
   check_label("C128/NTSC/HDMI/VICE 720p@60Hz", "C128");
   check_label("VIC20/PAL/HDMI/VICE 720p@50Hz", "VIC-20");
   check_label("Plus4/PAL/HDMI/VICE 720p@50Hz", "Plus/4");
   check_label("Plus4Emu/PAL/HDMI/720p@50Hz", "Plus/4");
   check_label("Pet/NTSC/HDMI/VICE 720p@60Hz", "PET");
   check_label("vic20", "VIC-20");
   check_label("C64", "C64");
   check_label("", "");
   check_label(NULL, "");
   char small[4];
   profiles_machine_label("VIC20/PAL", small, sizeof(small));
   CHECK_STR(small, "VIC");

   CHECK(profiles_machine_matches("C64/PAL/HDMI/VICE 720p@50Hz", "C64"));
   CHECK(profiles_machine_matches("C64", "C64"));
   CHECK(profiles_machine_matches("c64", "C64"));
   CHECK(profiles_machine_matches("Plus4Emu/PAL", "Plus4Emu"));
   CHECK(!profiles_machine_matches("Plus4/PAL", "Plus4Emu"));
   CHECK(!profiles_machine_matches("C128/NTSC", "C64"));
   CHECK(!profiles_machine_matches("C64", "C128"));
   CHECK(!profiles_machine_matches("", "C64"));
   CHECK(!profiles_machine_matches("/PAL", "C64"));
   CHECK(!profiles_machine_matches("C64", ""));
   CHECK(!profiles_machine_matches(NULL, "C64"));
}

int main(void) {
   char root[] = "/tmp/bmc64-profiles-test-XXXXXX";
   if (mkdtemp(root) == NULL) {
      perror("mkdtemp");
      return 1;
   }
   snprintf(card_root, sizeof(card_root), "%s", root);
   setenv("SD_TEST_ROOT", root, 1);

   test_kv_parse();
   test_kv_append();
   test_profile_parse();
   test_profile_format();
   test_active_parse();
   test_ids();
   test_no_profiles();
   test_boot_profile();
   test_start_once();
   test_switch_to();
   test_safe_mode();
   test_list();
   test_create();
   test_rename();
   test_delete();
   test_shared_settings();
   test_machines();

   remove_tree(root);

   if (failures) {
      fprintf(stderr, "profiles tests: %d failure(s)\n", failures);
      return 1;
   }
   printf("profiles tests passed\n");
   return 0;
}
