// Tests for src/profiles, against the behaviour in docs/PROFILES.md.
// Profile files are created in a temporary folder that stands in for the SD
// card (tools/sdcard/sd_fs_posix.c, given the folder in SD_TEST_ROOT).

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
   CHECK_STR(af.bad_profile, "");
   CHECK_STR(af.bad_once, "");

   // Invalid values are kept, so start-up can say what was wrong.
   snprintf(text, sizeof(text), "profile=My Game\nonce=../x\n");
   active_file_parse(text, &af);
   CHECK_STR(af.profile, "main");
   CHECK_STR(af.once, "");
   CHECK_STR(af.bad_profile, "My Game");
   CHECK_STR(af.bad_once, "../x");
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
   // "main" is reserved.
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
   profiles_boot_init("C64/PAL/HDMI");
   // Main, and nothing created on the card.
   CHECK(profiles_running_is_main());
   CHECK(!profiles_in_use());
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

   // Main is always in the list.
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
   profiles_boot_init("C64/PAL/HDMI");
   CHECK(!profiles_running_is_main());
   CHECK_STR(profiles_running()->id, "geos");
   CHECK_STR(profiles_running()->name, "GEOS");
   CHECK_STR(profiles_boot_message(), "");
   CHECK_STR(profiles_vice_config(), "/profiles/geos/vice.ini");
   CHECK_STR(profiles_settings_file("/settings.txt"),
             "/profiles/geos/settings-c64.txt");
   // The machine name is matched without regard to case.
   profiles_boot_init("c64/pal/hdmi");
   CHECK_STR(profiles_running()->id, "geos");

   // A missing or invalid profile starts Main, with a message.
   write_file("/profiles/active.txt", "profile=gone\n");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK(profiles_running_is_main());
   CHECK(strstr(profiles_boot_message(), "gone") != NULL);
   make_profile("broken", "name=No machine\n");
   write_file("/profiles/active.txt", "profile=broken\n");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK(profiles_running_is_main());
   CHECK(profiles_boot_message()[0] != '\0');

   // A profile for another machine starts Main, with a message.
   make_profile("vic", "name=PAL 16K\nmachine=VIC20/PAL/HDMI\n");
   write_file("/profiles/active.txt", "profile=vic\n");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK(profiles_running_is_main());
   CHECK(strstr(profiles_boot_message(), "PAL 16K") != NULL);
   profiles_boot_init("VIC20/PAL/HDMI");
   CHECK_STR(profiles_running()->id, "vic");
   profiles_boot_init("VIC20/NTSC/HDMI");
   CHECK(profiles_running_is_main());

   // A typo in profile= starts Main, with a message naming it.
   write_file("/profiles/active.txt", "profile=GEOS\n");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK(profiles_running_is_main());
   CHECK(strstr(profiles_boot_message(), "profile=GEOS") != NULL);
   write_file("/profiles/active.txt", "profile=my game\n");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK(profiles_running_is_main());
   CHECK(strstr(profiles_boot_message(), "my game") != NULL);

   // A typo in once= is ignored with a message; the usual profile starts.
   write_file("/profiles/active.txt", "profile=geos\nonce=Elite!\n");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK_STR(profiles_running()->id, "geos");
   CHECK(strstr(profiles_boot_message(), "once=Elite!") != NULL);

   // A valid once wins over a typo in profile=, without a message.
   make_profile("elite", "name=Elite\nmachine=C64\n");
   write_file("/profiles/active.txt", "profile=GEOS\nonce=elite\n");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK_STR(profiles_running()->id, "elite");
   CHECK_STR(profiles_boot_message(), "");

   // An empty value is just "not set": no message.
   write_file("/profiles/active.txt", "profile=\n");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK(profiles_running_is_main());
   CHECK_STR(profiles_boot_message(), "");

   // profile=main is Main without a message, with profiles in use.
   write_file("/profiles/active.txt", "profile=main\n");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK(profiles_running_is_main());
   CHECK(profiles_in_use());
   CHECK_STR(profiles_boot_message(), "");
}

static void test_start_once(void) {
   fresh_card();
   make_profile("geos", "name=GEOS\nmachine=C64\n");
   make_profile("elite", "name=Elite\nmachine=C64\n");
   write_file("/profiles/active.txt", "profile=geos\n");
   profiles_boot_init("C64/PAL/HDMI");

   // Once is added, profile is unchanged.
   CHECK(profiles_start_once("elite") == PROFILES_OK);
   CHECK_STR(read_file("/profiles/active.txt"), "profile=geos\nonce=elite\n");

   // The once profile starts; once stays until after boot.
   profiles_boot_init("C64/PAL/HDMI");
   CHECK_STR(profiles_running()->id, "elite");
   CHECK_STR(read_file("/profiles/active.txt"), "profile=geos\nonce=elite\n");
   profiles_after_boot();
   CHECK_STR(read_file("/profiles/active.txt"), "profile=geos\n");

   // A restart BMC64 asks for keeps the once profile.
   profiles_before_reboot();
   CHECK_STR(read_file("/profiles/active.txt"), "profile=geos\nonce=elite\n");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK_STR(profiles_running()->id, "elite");
   profiles_after_boot();

   // The next power-on is back on GEOS.
   profiles_boot_init("C64/PAL/HDMI");
   CHECK_STR(profiles_running()->id, "geos");
   // A switched-to profile isn't put back as once.
   profiles_before_reboot();
   CHECK_STR(read_file("/profiles/active.txt"), "profile=geos\n");

   // A once profile that can't start still has its entry removed.
   write_file("/profiles/active.txt", "profile=geos\nonce=gone\n");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK(profiles_running_is_main());
   profiles_after_boot();
   // Main ran, so it also remembers its machine.
   CHECK_STR(read_file("/profiles/active.txt"),
             "profile=geos\nmain_machine=C64/PAL/HDMI\n");

   // Start once from Main with no active.txt.
   fresh_card();
   make_profile("elite", "name=Elite\nmachine=C64\n");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK(profiles_start_once("elite") == PROFILES_OK);
   CHECK_STR(read_file("/profiles/active.txt"),
             "profile=main\nonce=elite\nmain_machine=C64/PAL/HDMI\n");
}

static void test_switch_to(void) {
   fresh_card();
   make_profile("geos", "name=GEOS\nmachine=C64\n");
   make_profile("vic", "name=PAL 16K\nmachine=VIC20\n");
   profiles_boot_init("C64/PAL/HDMI");

   // Leaving Main remembers its machine.
   CHECK(profiles_switch_to("geos") == PROFILES_OK);
   CHECK_STR(read_file("/profiles/active.txt"),
             "profile=geos\nmain_machine=C64/PAL/HDMI\n");
   CHECK(!exists("/profiles/active.new"));
   // Switching clears a pending once.
   write_file("/profiles/active.txt", "profile=geos\nonce=geos\n");
   CHECK(profiles_switch_to("main") == PROFILES_OK);
   CHECK_STR(read_file("/profiles/active.txt"),
             "profile=main\nmain_machine=C64/PAL/HDMI\n");

   // Missing and invalid profiles aren't started.
   CHECK(profiles_switch_to("gone") == PROFILES_ERROR);
   CHECK(profiles_switch_to("../x") == PROFILES_ERROR);
   CHECK(profiles_switch_to(NULL) == PROFILES_ERROR);
   CHECK(profiles_start_once("gone") == PROFILES_ERROR);
   CHECK_STR(read_file("/profiles/active.txt"),
             "profile=main\nmain_machine=C64/PAL/HDMI\n");

   // Other machines can be chosen; the caller switches machine first.
   CHECK_STR(profiles_machine_for("vic"), "VIC20");
   CHECK_STR(profiles_machine_for("geos"), "");
   CHECK_STR(profiles_machine_for("main"), "");
   CHECK_STR(profiles_machine_for("gone"), "");
   CHECK(profiles_switch_to("vic") == PROFILES_OK);
   CHECK_STR(read_file("/profiles/active.txt"),
             "profile=vic\nmain_machine=C64/PAL/HDMI\n");
}

static void test_safe_mode(void) {
   fresh_card();
   make_profile("geos", "name=GEOS\nmachine=C64\n");
   write_file("/profiles/active.txt",
              "profile=geos\nonce=geos\nmain_machine=VIC20/PAL/HDMI\n");

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
   profiles_boot_init("C64/PAL/HDMI");

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
   profiles_boot_init("C64/PAL/HDMI");
   char id[PROFILES_MAX_ID_LEN + 1];
   // Main's new profiles are for what booted.
   CHECK(profiles_create("  My GEOS  ", id, sizeof(id)) == PROFILES_OK);
   CHECK_STR(id, "my-geos");
   CHECK_STR(read_file("/profiles/my-geos/profile.txt"),
             "name=My GEOS\nmachine=C64/PAL/HDMI\n");
   // No active.txt yet: creating doesn't switch.
   CHECK(!exists("/profiles/active.txt"));

   CHECK(profiles_create("My GEOS", id, sizeof(id)) == PROFILES_OK);
   CHECK_STR(id, "my-geos-2");
   CHECK(profiles_create("   ", id, sizeof(id)) == PROFILES_ERROR);
   CHECK(profiles_create("", id, sizeof(id)) == PROFILES_ERROR);

   // A new profile made in a profile copies its machine entry.
   make_profile("pal", "name=PAL\nmachine=C64/PAL/HDMI/VICE 720p@50Hz\n");
   write_file("/profiles/active.txt", "profile=pal\n");
   profiles_boot_init("C64/PAL/HDMI");
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
   profiles_boot_init("C64/PAL/HDMI");

   // Only the name changes; the id and other keys stay.
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

   // Main can't be renamed.
   fresh_card();
   profiles_boot_init("C64/PAL/HDMI");
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
   // Left by the web UI's editor.
   write_file("/profiles/elite/settings.txt.bak", "palette=0\n");
   write_file("/profiles/elite/profile.txt.bak", "name=Old\nmachine=C64\n");
   write_file("/profiles/elite/vice.ini.bak", "[C64]\n");
   write_file("/disks/elite.d64", "disk image");
   write_file("/profiles/active.txt", "profile=geos\n");
   profiles_boot_init("C64/PAL/HDMI");

   // Not Main, not the running profile, nothing invalid.
   CHECK(profiles_delete("main") == PROFILES_ERROR);
   CHECK(profiles_delete("geos") == PROFILES_ERROR);
   CHECK(exists("/profiles/geos/profile.txt"));
   CHECK(profiles_delete("../disks") == PROFILES_ERROR);
   CHECK(profiles_delete("") == PROFILES_ERROR);
   CHECK(profiles_delete(NULL) == PROFILES_ERROR);
   CHECK(profiles_delete("gone") == PROFILES_ERROR);

   // The profile's own files and folder go, nothing else.
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

// ---- Autostart ----

static void test_autostart_profile(void) {
   fresh_card();
   make_profile("elite", "name=Elite\nmachine=C64\ncategory=Games\n");
   write_file("/profiles/active.txt", "profile=elite\n");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK_STR(profiles_autostart(), "");

   // Saved straight away into profile.txt; other keys are kept.
   CHECK(profiles_set_autostart("SD:/games/elite.d64") == PROFILES_OK);
   CHECK_STR(profiles_autostart(), "SD:/games/elite.d64");
   CHECK_STR(read_file("/profiles/elite/profile.txt"),
             "name=Elite\ncategory=Games\nmachine=C64\n"
             "autostart=SD:/games/elite.d64\n");

   // Start-up picks it up.
   profiles_boot_init("C64/PAL/HDMI");
   CHECK_STR(profiles_autostart(), "SD:/games/elite.d64");

   // A rename made since start-up isn't lost by a later autostart change.
   CHECK(profiles_rename_running("Elite Plus") == PROFILES_OK);
   CHECK(profiles_set_autostart("SD:/games/elite2.d64") == PROFILES_OK);
   ProfileFile pf;
   CHECK(profile_file_read("elite", &pf) == PROFILES_OK);
   CHECK_STR(pf.info.name, "Elite Plus");
   CHECK_STR(pf.autostart, "SD:/games/elite2.d64");

   // Clearing removes the key.
   CHECK(profiles_clear_autostart() == PROFILES_OK);
   CHECK_STR(profiles_autostart(), "");
   CHECK(strstr(read_file("/profiles/elite/profile.txt"), "autostart") == NULL);

   // An empty or missing path isn't an autostart.
   CHECK(profiles_set_autostart("") == PROFILES_ERROR);
   CHECK(profiles_set_autostart(NULL) == PROFILES_ERROR);
   CHECK_STR(profiles_autostart(), "");
}

static void test_startup_disks(void) {
   fresh_card();
   make_profile("geos", "name=GEOS\nmachine=C64\n");
   write_file("/profiles/active.txt", "profile=geos\n");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK_STR(profiles_startup_disk(0), "");
   CHECK_STR(profiles_startup_disk(-1), "");
   CHECK_STR(profiles_startup_disk(PROFILES_NUM_DRIVES), "");

   // Saved straight away; empty drives are left out.
   const char *disks[PROFILES_NUM_DRIVES] = {"SD:/disks/geos.d64", "", NULL,
                                             "SD:/disks/data.d81"};
   CHECK(profiles_set_startup_disks(disks) == PROFILES_OK);
   CHECK_STR(profiles_startup_disk(0), "SD:/disks/geos.d64");
   CHECK_STR(profiles_startup_disk(1), "");
   CHECK_STR(profiles_startup_disk(3), "SD:/disks/data.d81");
   CHECK_STR(read_file("/profiles/geos/profile.txt"),
             "name=GEOS\nmachine=C64\ndisk_8=SD:/disks/geos.d64\n"
             "disk_11=SD:/disks/data.d81\n");

   // Start-up picks them up; the autostart is kept separately.
   CHECK(profiles_set_autostart("SD:/disks/geos.d64") == PROFILES_OK);
   profiles_boot_init("C64/PAL/HDMI");
   CHECK_STR(profiles_startup_disk(0), "SD:/disks/geos.d64");
   CHECK_STR(profiles_startup_disk(3), "SD:/disks/data.d81");
   CHECK_STR(profiles_autostart(), "SD:/disks/geos.d64");

   // Setting again replaces all four drives.
   const char *one[PROFILES_NUM_DRIVES] = {NULL, "SD:/disks/b.d64", NULL, NULL};
   CHECK(profiles_set_startup_disks(one) == PROFILES_OK);
   CHECK_STR(profiles_startup_disk(0), "");
   CHECK_STR(profiles_startup_disk(1), "SD:/disks/b.d64");

   // Clearing removes them and keeps the autostart.
   CHECK(profiles_clear_startup_disks() == PROFILES_OK);
   CHECK_STR(profiles_startup_disk(1), "");
   CHECK_STR(read_file("/profiles/geos/profile.txt"),
             "name=GEOS\nmachine=C64\nautostart=SD:/disks/geos.d64\n");

   // A path too long to keep isn't saved cut short.
   char long_path[PROFILES_MAX_PATH_LEN + 10];
   memset(long_path, 'a', sizeof(long_path) - 1);
   long_path[sizeof(long_path) - 1] = '\0';
   const char *too_long[PROFILES_NUM_DRIVES] = {long_path, NULL, NULL, NULL};
   CHECK(profiles_set_startup_disks(too_long) == PROFILES_ERROR);
   CHECK_STR(profiles_startup_disk(0), "");

   // Main keeps them in its own file per machine.
   fresh_card();
   profiles_boot_init("C64/PAL/HDMI");
   CHECK(profiles_set_startup_disks(disks) == PROFILES_OK);
   CHECK_STR(read_file("/profiles/main/c64.txt"),
             "disk_8=SD:/disks/geos.d64\ndisk_11=SD:/disks/data.d81\n");
   CHECK(exists("/profiles/active.txt"));
   profiles_boot_init("C64/PAL/HDMI");
   CHECK_STR(profiles_startup_disk(0), "SD:/disks/geos.d64");
   profiles_boot_init("VIC20/PAL/HDMI");
   CHECK_STR(profiles_startup_disk(0), "");
}

static void test_autostart_main(void) {
   // Main with no profiles: setting it creates Main's file and active.txt,
   // so start-up knows to read it.
   fresh_card();
   profiles_boot_init("C64/PAL/HDMI");
   CHECK(profiles_set_autostart("SD:/geos/GEOS64.D81") == PROFILES_OK);
   CHECK_STR(profiles_autostart(), "SD:/geos/GEOS64.D81");
   CHECK_STR(read_file("/profiles/main/c64.txt"),
             "autostart=SD:/geos/GEOS64.D81\n");
   CHECK_STR(read_file("/profiles/active.txt"), "profile=main\n");

   profiles_boot_init("C64/PAL/HDMI");
   CHECK(profiles_running_is_main());
   CHECK(profiles_in_use());
   CHECK_STR(profiles_autostart(), "SD:/geos/GEOS64.D81");

   // Each machine has its own file.
   profiles_boot_init("VIC20/PAL/HDMI");
   CHECK_STR(profiles_autostart(), "");
   CHECK(profiles_set_autostart("SD:/vic/game.prg") == PROFILES_OK);
   CHECK_STR(read_file("/profiles/main/vic20.txt"),
             "autostart=SD:/vic/game.prg\n");
   profiles_boot_init("Plus4Emu/PAL/HDMI");
   CHECK(profiles_set_autostart("SD:/p4/game.prg") == PROFILES_OK);
   CHECK(exists("/profiles/main/plus4emu.txt"));
   profiles_boot_init("C64/PAL/HDMI");
   CHECK_STR(profiles_autostart(), "SD:/geos/GEOS64.D81");

   // An existing active.txt is left alone.
   make_profile("elite", "name=Elite\nmachine=C64\n");
   write_file("/profiles/active.txt", "profile=main\nonce=elite\n");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK_STR(profiles_running()->id, "elite");
   CHECK_STR(profiles_autostart(), "");
   write_file("/profiles/active.txt", "profile=main\n");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK(profiles_clear_autostart() == PROFILES_OK);
   CHECK_STR(read_file("/profiles/active.txt"), "profile=main\n");
   CHECK_STR(read_file("/profiles/main/c64.txt"), "");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK_STR(profiles_autostart(), "");

   // Main's file is used when a profile can't start and Main runs instead.
   CHECK(profiles_set_autostart("SD:/x.prg") == PROFILES_OK);
   write_file("/profiles/active.txt", "profile=gone\n");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK(profiles_running_is_main());
   CHECK_STR(profiles_autostart(), "SD:/x.prg");

   // Main's file only holds startup actions, never a name or machine.
   write_file("/profiles/main/c64.txt",
              "name=Sneaky\nmachine=C64\nautostart=SD:/y.prg\n");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK(profiles_set_autostart("SD:/z.prg") == PROFILES_OK);
   CHECK_STR(read_file("/profiles/main/c64.txt"), "autostart=SD:/z.prg\n");

   // Without active.txt, nothing reads Main's file.
   fresh_card();
   write_file("/profiles/main/c64.txt", "autostart=SD:/x.prg\n");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK_STR(profiles_autostart(), "");
}

// ---- Machines ----

static void test_machine_switching(void) {
   // Main remembers the machine it runs on, once profiles are in use.
   fresh_card();
   profiles_boot_init("C64/PAL/HDMI");
   profiles_after_boot();
   CHECK(!exists("/profiles"));
   make_profile("elite", "name=Elite\nmachine=C64/PAL/HDMI\n");
   make_profile("vic", "name=PAL 16K\nmachine=VIC20/PAL/HDMI\n");
   write_file("/profiles/active.txt", "profile=main\n");
   profiles_boot_init("C64/PAL/HDMI");
   profiles_after_boot();
   CHECK_STR(read_file("/profiles/active.txt"),
             "profile=main\nmain_machine=C64/PAL/HDMI\n");
   // A Switch machine entry for the same machine is kept as it is.
   write_file("/profiles/active.txt",
              "profile=main\nmain_machine=C64/PAL/HDMI/VICE 1080p@50Hz\n");
   profiles_boot_init("C64/PAL/HDMI");
   profiles_after_boot();
   CHECK_STR(read_file("/profiles/active.txt"),
             "profile=main\nmain_machine=C64/PAL/HDMI/VICE 1080p@50Hz\n");

   // What each profile needs from here.
   CHECK_STR(profiles_machine_for("vic"), "VIC20/PAL/HDMI");
   CHECK_STR(profiles_machine_for("elite"), "");
   CHECK_STR(profiles_machine_for("main"), "");
   make_profile("ntsc", "name=NTSC\nmachine=C64/NTSC\n");
   CHECK_STR(profiles_machine_for("ntsc"), "C64/NTSC");

   // Back to Main from the VIC-20: Main's machine.
   CHECK(profiles_switch_to("vic") == PROFILES_OK);
   profiles_boot_init("VIC20/PAL/HDMI");
   CHECK_STR(profiles_running()->id, "vic");
   CHECK_STR(profiles_machine_for("main"), "C64/PAL/HDMI/VICE 1080p@50Hz");
   CHECK_STR(profiles_machine_for("elite"), "C64/PAL/HDMI");
   CHECK_STR(profiles_return_machine(), "");
   // Main without a remembered machine stays where it is.
   write_file("/profiles/active.txt", "profile=vic\n");
   profiles_boot_init("VIC20/PAL/HDMI");
   CHECK_STR(profiles_machine_for("main"), "");

   // Start once on another machine: the power-on profile's machine is put
   // back for the next power-on, and once is kept for BMC64's own restarts.
   write_file("/profiles/active.txt", "profile=elite\n");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK(profiles_start_once("vic") == PROFILES_OK);
   CHECK_STR(read_file("/profiles/active.txt"), "profile=elite\nonce=vic\n");
   profiles_boot_init("VIC20/PAL/HDMI");
   CHECK_STR(profiles_running()->id, "vic");
   CHECK_STR(profiles_return_machine(), "C64/PAL/HDMI");
   profiles_after_boot();
   CHECK_STR(read_file("/profiles/active.txt"), "profile=elite\n");
   profiles_before_reboot();
   CHECK_STR(read_file("/profiles/active.txt"), "profile=elite\nonce=vic\n");
   // From Main: back to Main's machine.
   write_file("/profiles/active.txt",
              "profile=main\nonce=vic\nmain_machine=C64/PAL/HDMI\n");
   profiles_boot_init("VIC20/PAL/HDMI");
   CHECK_STR(profiles_return_machine(), "C64/PAL/HDMI");
   // On the same machine there's nothing to put back.
   write_file("/profiles/active.txt", "profile=main\nonce=elite\n");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK_STR(profiles_running()->id, "elite");
   CHECK_STR(profiles_return_machine(), "");
   // Start once from Main remembers Main's machine first.
   fresh_card();
   make_profile("vic", "name=PAL 16K\nmachine=VIC20/PAL/HDMI\n");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK(profiles_start_once("vic") == PROFILES_OK);
   CHECK_STR(read_file("/profiles/active.txt"),
             "profile=main\nonce=vic\nmain_machine=C64/PAL/HDMI\n");

   // Switch machine: nothing without profiles.
   fresh_card();
   profiles_boot_init("C64/PAL/HDMI");
   profiles_machine_switched("VIC20/PAL/HDMI/VICE 720p@50Hz");
   CHECK(!exists("/profiles"));
   // In Main: Main remembers the entry.
   make_profile("hd", "name=HD\nmachine=C64/PAL/HDMI\n");
   write_file("/profiles/active.txt", "profile=main\nonce=hd\n");
   profiles_machine_switched("C64/NTSC/HDMI/VICE 720p@60Hz");
   CHECK_STR(read_file("/profiles/active.txt"),
             "profile=main\nmain_machine=C64/NTSC/HDMI/VICE 720p@60Hz\n");
   // In a profile, same machine: the profile follows the standard and output.
   write_file("/profiles/active.txt", "profile=hd\n");
   profiles_machine_switched("C64/NTSC/HDMI/VICE 720p@60Hz");
   CHECK_STR(read_file("/profiles/active.txt"), "profile=hd\n");
   CHECK_STR(read_file("/profiles/hd/profile.txt"),
             "name=HD\nmachine=C64/NTSC/HDMI\n");
   // Another machine: the profile follows too, and Main isn't changed.
   write_file("/profiles/active.txt",
              "profile=hd\nmain_machine=C64/PAL/HDMI\n");
   write_file("/profiles/hd/settings-c64.txt", "c64 settings\n");
   profiles_machine_switched("VIC20/PAL/HDMI/VICE 720p@50Hz");
   CHECK_STR(read_file("/profiles/active.txt"),
             "profile=hd\nmain_machine=C64/PAL/HDMI\n");
   CHECK_STR(read_file("/profiles/hd/profile.txt"),
             "name=HD\nmachine=VIC20/PAL/HDMI\n");
   // It starts on the VIC-20 with the VIC-20's own (new) settings file; the
   // C64's are kept for when it goes back.
   profiles_boot_init("VIC20/PAL/HDMI");
   CHECK_STR(profiles_running()->id, "hd");
   CHECK_STR(profiles_settings_file("/settings-vic20.txt"),
             "/profiles/hd/settings-vic20.txt");
   CHECK_STR(read_file("/profiles/hd/settings-c64.txt"), "c64 settings\n");
   // A profile that can't be read: Main.
   write_file("/profiles/active.txt", "profile=gone\n");
   profiles_machine_switched("VIC20/PAL/HDMI/VICE 720p@50Hz");
   CHECK_STR(read_file("/profiles/active.txt"),
             "profile=main\nmain_machine=VIC20/PAL/HDMI/VICE 720p@50Hz\n");

   // A full header in machine= works too.
   make_profile("full", "name=Full\nmachine=C64/PAL/HDMI/VICE 720p@50Hz\n");
   write_file("/profiles/active.txt", "profile=full\n");
   profiles_boot_init("C64/PAL/HDMI");
   CHECK_STR(profiles_running()->id, "full");

   char desc[64];
   profiles_machine_desc("C64 / PAL / HDMI / VICE 720p@50Hz", desc, sizeof(desc));
   CHECK_STR(desc, "C64/PAL/HDMI");
   profiles_machine_desc("C64", desc, sizeof(desc));
   CHECK_STR(desc, "C64");
}

static void test_settings_files(void) {
   // New profiles get the running machine's settings file.
   fresh_card();
   profiles_boot_init("Plus4Emu/PAL/HDMI");
   char path[PROFILES_MAX_PATH_LEN];
   profiles_new_settings_file("ted", path, sizeof(path));
   CHECK_STR(path, "/profiles/ted/settings-plus4emu.txt");

   // An older profile's settings.txt is read at boot, and renamed to the
   // machine's own name once boot is complete.
   make_profile("old", "name=Old\nmachine=C128/PAL/HDMI\n");
   write_file("/profiles/old/settings.txt", "c128 settings\n");
   write_file("/profiles/active.txt", "profile=old\n");
   profiles_boot_init("C128/PAL/HDMI");
   CHECK_STR(profiles_settings_file("/settings-c128.txt"),
             "/profiles/old/settings.txt");
   CHECK(exists("/profiles/old/settings.txt"));
   profiles_after_boot();
   CHECK_STR(profiles_settings_file("/settings-c128.txt"),
             "/profiles/old/settings-c128.txt");
   CHECK(!exists("/profiles/old/settings.txt"));
   CHECK_STR(read_file("/profiles/old/settings-c128.txt"), "c128 settings\n");
   profiles_boot_init("C128/PAL/HDMI");
   CHECK_STR(profiles_settings_file("/settings-c128.txt"),
             "/profiles/old/settings-c128.txt");

   // Moving an older profile to another machine first gives its
   // settings.txt the old machine's name, so the new machine never reads it.
   make_profile("move", "name=Move\nmachine=C64/PAL/HDMI\n");
   write_file("/profiles/move/settings.txt", "c64 settings\n");
   write_file("/profiles/active.txt", "profile=move\n");
   profiles_machine_switched("Plus4/PAL/HDMI/VICE 720p@50Hz");
   CHECK(!exists("/profiles/move/settings.txt"));
   CHECK_STR(read_file("/profiles/move/settings-c64.txt"), "c64 settings\n");
   profiles_boot_init("Plus4/PAL/HDMI");
   CHECK_STR(profiles_settings_file("/settings-plus4.txt"),
             "/profiles/move/settings-plus4.txt");

   // Delete removes every machine's settings.
   profiles_boot_init("C64/PAL/HDMI");
   write_file("/profiles/move/settings-plus4.txt", "x\n");
   write_file("/profiles/move/settings-plus4.txt.bak", "x\n");
   CHECK(profiles_delete("move") == PROFILES_OK);
   CHECK(!exists("/profiles/move"));
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
   CHECK(profiles_machine_matches("C64/PAL/HDMI", "c64 / pal / hdmi / VICE 720p@50Hz"));
   CHECK(!profiles_machine_matches("C64/PAL/HDMI/VICE 1080p@50Hz",
                                   "C64/PAL/HDMI/VICE 720p@50Hz"));

   CHECK(profiles_machine_covers("C64/PAL", "C64/PAL/HDMI/VICE 720p@50Hz"));
   CHECK(profiles_machine_covers("C64/PAL/HDMI/VICE 720p@50Hz",
                                 "C64/PAL/HDMI/VICE 720p@50Hz"));
   CHECK(!profiles_machine_covers("C64/PAL/HDMI", "C64/PAL"));
   CHECK(!profiles_machine_covers("C64/NTSC", "C64/PAL/HDMI/VICE 720p@50Hz"));
   CHECK(!profiles_machine_covers("", "C64"));
   CHECK(!profiles_machine_covers("C64", ""));
   // The description may hold '/'.
   CHECK(profiles_machine_covers("C64/PAL/DPI/Gert 666/RGB",
                                 "C64/PAL/DPI/Gert 666/RGB"));
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
   test_autostart_profile();
   test_autostart_main();
   test_startup_disks();
   test_machines();
   test_machine_switching();
   test_settings_files();

   remove_tree(root);

   if (failures) {
      fprintf(stderr, "profiles tests: %d failure(s)\n", failures);
      return 1;
   }
   printf("profiles tests passed\n");
   return 0;
}
