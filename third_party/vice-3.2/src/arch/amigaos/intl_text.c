/*
 * intl_text.c - Translation texts to be included in intl.c
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

intl_translate_t intl_string_table[] = {

/* en */ { IDMS_FILE,    "File" },
/* da */ { IDMS_FILE_DA, "Fil" },
/* de */ { IDMS_FILE_DE, "Datei" },
/* es */ { IDMS_FILE_ES, "Archivo" },
/* fr */ { IDMS_FILE_FR, "Fichier" },
/* hu */ { IDMS_FILE_HU, "Fájl" },
/* it */ { IDMS_FILE_IT, "File" },
/* ko */ { IDMS_FILE_KO, "ÆÄÀÏ" },
/* nl */ { IDMS_FILE_NL, "Bestand" },
/* pl */ { IDMS_FILE_PL, "Plik" },
/* ru */ { IDMS_FILE_RU, "ÄÐÙÛ" },
/* sv */ { IDMS_FILE_SV, "Arkiv" },
/* tr */ { IDMS_FILE_TR, "Dosya" },

/* en */ { IDMS_AUTOSTART_IMAGE,    "Autostart disk/tape image..." },
/* da */ { IDMS_AUTOSTART_IMAGE_DA, "Autostart disk-/bånd-image..." },
/* de */ { IDMS_AUTOSTART_IMAGE_DE, "Autostart von Disk/Band Image..." },
/* es */ { IDMS_AUTOSTART_IMAGE_ES, "Autoarranque imagen de disco/cinta..." },
/* fr */ { IDMS_AUTOSTART_IMAGE_FR, "Autodémarrage image disque/datassette..." },
/* hu */ { IDMS_AUTOSTART_IMAGE_HU, "Lemez/szalag képmás automatikus indítása..." },
/* it */ { IDMS_AUTOSTART_IMAGE_IT, "Avvia automaticamente immagine disco/cassetta..." },
/* ko */ { IDMS_AUTOSTART_IMAGE_KO, "ÀÚµ¿ ½ÃÀÛ µð½ºÅ©/Å×ÀÌÇÁ ÀÌ¹ÌÁö..." },
/* nl */ { IDMS_AUTOSTART_IMAGE_NL, "Autostart disk-/tapebestand..." },
/* pl */ { IDMS_AUTOSTART_IMAGE_PL, "Automatycznie startuj z obrazu dysku lub ta¶my..." },
/* ru */ { IDMS_AUTOSTART_IMAGE_RU, "°ÒâÞÜÐâØçÕáÚØ ×ÐßãáâØâì ÞÑàÐ× ÔØáÚÐ/ÚÐááÕâë..." },
/* sv */ { IDMS_AUTOSTART_IMAGE_SV, "Autostarta disk-/bandavbildningsfil..." },
/* tr */ { IDMS_AUTOSTART_IMAGE_TR, "Disk/Teyp imajýný otomatik baþlat..." },

/* en */ { IDMS_ATTACH_DISK_IMAGE,    "Attach disk image" },
/* da */ { IDMS_ATTACH_DISK_IMAGE_DA, "Tilslut disk-image" },
/* de */ { IDMS_ATTACH_DISK_IMAGE_DE, "Disk Image einlegen" },
/* es */ { IDMS_ATTACH_DISK_IMAGE_ES, "Insertar imagen de disco" },
/* fr */ { IDMS_ATTACH_DISK_IMAGE_FR, "Insérer une image de disque" },
/* hu */ { IDMS_ATTACH_DISK_IMAGE_HU, "Lemez képmás csatolása" },
/* it */ { IDMS_ATTACH_DISK_IMAGE_IT, "Seleziona immagine disco" },
/* ko */ { IDMS_ATTACH_DISK_IMAGE_KO, "µð½ºÅ© ÀÌ¹ÌÁö ºÙ¿©³Ö±â" },
/* nl */ { IDMS_ATTACH_DISK_IMAGE_NL, "Koppel schijfbestand" },
/* pl */ { IDMS_ATTACH_DISK_IMAGE_PL, "Zamontuj obraz dyskietki" },
/* ru */ { IDMS_ATTACH_DISK_IMAGE_RU, "´ÞÑÐÒØâì ÞÑàÐ× ÔØáÚÐ" },
/* sv */ { IDMS_ATTACH_DISK_IMAGE_SV, "Anslut diskettavbildningsfil" },
/* tr */ { IDMS_ATTACH_DISK_IMAGE_TR, "Disk imajýný yerleþtir" },

/* en */ {IDMS_DRIVE_8,    "Drive 8" },
/* da */ {IDMS_DRIVE_8_DA, "Drev 8" },
/* de */ {IDMS_DRIVE_8_DE, "Laufwerk 8" },
/* es */ {IDMS_DRIVE_8_ES, "Unidad disco 8" },
/* fr */ {IDMS_DRIVE_8_FR, "Lecteur #8" },
/* hu */ {IDMS_DRIVE_8_HU, "#8-as lemezegység" },
/* it */ {IDMS_DRIVE_8_IT, "Drive 8" },
/* ko */ {IDMS_DRIVE_8_KO, "µå¶óÀÌºê8" },
/* nl */ {IDMS_DRIVE_8_NL, "Drive 8" },
/* pl */ {IDMS_DRIVE_8_PL, "Napêd 8" },
/* ru */ {IDMS_DRIVE_8_RU, "ÃáâàÞÙáâÒÞ 8" },
/* sv */ {IDMS_DRIVE_8_SV, "Enhet 8" },
/* tr */ {IDMS_DRIVE_8_TR, "Sürücü 8" },

/* en */ { IDMS_DRIVE_9,    "Drive 9" },
/* da */ { IDMS_DRIVE_9_DA, "Drev 9" },
/* de */ { IDMS_DRIVE_9_DE, "Laufwerk 9" },
/* es */ { IDMS_DRIVE_9_ES, "Unidad disco 9" },
/* fr */ { IDMS_DRIVE_9_FR, "Lecteur #9" },
/* hu */ { IDMS_DRIVE_9_HU, "#9-es lemezegység" },
/* it */ { IDMS_DRIVE_9_IT, "Drive 9" },
/* ko */ { IDMS_DRIVE_9_KO, "µå¶óÀÌºê9" },
/* nl */ { IDMS_DRIVE_9_NL, "Drive 9" },
/* pl */ { IDMS_DRIVE_9_PL, "Napêd 9" },
/* ru */ { IDMS_DRIVE_9_RU, "ÃáâàÞÙáâÒÞ 9" },
/* sv */ { IDMS_DRIVE_9_SV, "Enhet 9" },
/* tr */ { IDMS_DRIVE_9_TR, "Sürücü 9" },

/* en */ { IDMS_DRIVE_10,    "Drive 10" },
/* da */ { IDMS_DRIVE_10_DA, "Drev 10" },
/* de */ { IDMS_DRIVE_10_DE, "Laufwerk 10" },
/* es */ { IDMS_DRIVE_10_ES, "Unidad disco 10" },
/* fr */ { IDMS_DRIVE_10_FR, "Lecteur #10" },
/* hu */ { IDMS_DRIVE_10_HU, "#10-es lemezegység" },
/* it */ { IDMS_DRIVE_10_IT, "Drive 10" },
/* ko */ { IDMS_DRIVE_10_KO, "µå¶óÀÌºê10" },
/* nl */ { IDMS_DRIVE_10_NL, "Drive 10" },
/* pl */ { IDMS_DRIVE_10_PL, "Napêd 10" },
/* ru */ { IDMS_DRIVE_10_RU, "ÃáâàÞÙáâÒÞ 10" },
/* sv */ { IDMS_DRIVE_10_SV, "Enhet 10" },
/* tr */ { IDMS_DRIVE_10_TR, "Sürücü 10" },

/* en */ { IDMS_DRIVE_11,    "Drive 11" },
/* da */ { IDMS_DRIVE_11_DA, "Drev 11" },
/* de */ { IDMS_DRIVE_11_DE, "Laufwerk 11" },
/* es */ { IDMS_DRIVE_11_ES, "Unidad disco 11" },
/* fr */ { IDMS_DRIVE_11_FR, "Lecteur #11" },
/* hu */ { IDMS_DRIVE_11_HU, "#11-es lemezegység" },
/* it */ { IDMS_DRIVE_11_IT, "Drive 11" },
/* ko */ { IDMS_DRIVE_11_KO, "µå¶óÀÌºê11" },
/* nl */ { IDMS_DRIVE_11_NL, "Drive 11" },
/* pl */ { IDMS_DRIVE_11_PL, "Napêd 11" },
/* ru */ { IDMS_DRIVE_11_RU, "ÃáâàÞÙáâÒÞ 11" },
/* sv */ { IDMS_DRIVE_11_SV, "Enhet 11" },
/* tr */ { IDMS_DRIVE_11_TR, "Sürücü 11" },

/* en */ { IDMS_DETACH_DISK_IMAGE,    "Detach disk image" },
/* da */ { IDMS_DETACH_DISK_IMAGE_DA, "Frakobl disk-image" },
/* de */ { IDMS_DETACH_DISK_IMAGE_DE, "Disk Image entfernen" },
/* es */ { IDMS_DETACH_DISK_IMAGE_ES, "Extraer imagen de disco" },
/* fr */ { IDMS_DETACH_DISK_IMAGE_FR, "Détacher une image de disque" },
/* hu */ { IDMS_DETACH_DISK_IMAGE_HU, "Lemezképmás leválasztása" },
/* it */ { IDMS_DETACH_DISK_IMAGE_IT, "Rimuovi immagine disco" },
/* ko */ { IDMS_DETACH_DISK_IMAGE_KO, "µð½ºÅ© ÀÌ¹ÌÁö ¶¼¾î³»±â" },
/* nl */ { IDMS_DETACH_DISK_IMAGE_NL, "Ontkoppel schijfbestand" },
/* pl */ { IDMS_DETACH_DISK_IMAGE_PL, "Wyjmij obraz dyskietki" },
/* ru */ { IDMS_DETACH_DISK_IMAGE_RU, "ÃÑàÐâì ÞÑàÐ× ÔØáÚÐ" },
/* sv */ { IDMS_DETACH_DISK_IMAGE_SV, "Koppla från diskettavbildningsfil" },
/* tr */ { IDMS_DETACH_DISK_IMAGE_TR, "Disk imajýný çýkart" },

/* en */ { IDMS_ALL,    "All" },
/* da */ { IDMS_ALL_DA, "Alle" },
/* de */ { IDMS_ALL_DE, "Alle" },
/* es */ { IDMS_ALL_ES, "Todo" },
/* fr */ { IDMS_ALL_FR, "Tous" },
/* hu */ { IDMS_ALL_HU, "Összes" },
/* it */ { IDMS_ALL_IT, "Tutti" },
/* ko */ { IDMS_ALL_KO, "ÀüºÎ" },
/* nl */ { IDMS_ALL_NL, "Alles" },
/* pl */ { IDMS_ALL_PL, "Wszystkie" },
/* ru */ { IDMS_ALL_RU, "²áÕ" },
/* sv */ { IDMS_ALL_SV, "Alla" },
/* tr */ { IDMS_ALL_TR, "Tümü" },

/* en */ { IDMS_JAM_ACTION_SETTINGS,    "CPU JAM action settings" },
/* da */ { IDMS_JAM_ACTION_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDMS_JAM_ACTION_SETTINGS_DE, "CPU JAM Aktion Einstellungen" },
/* es */ { IDMS_JAM_ACTION_SETTINGS_ES, "Ajustes acción CPU JAM" },
/* fr */ { IDMS_JAM_ACTION_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_JAM_ACTION_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_JAM_ACTION_SETTINGS_IT, "Impostazioni azione al JAM della CPU" },
/* ko */ { IDMS_JAM_ACTION_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_JAM_ACTION_SETTINGS_NL, "CPU JAM actie instellingen" },
/* pl */ { IDMS_JAM_ACTION_SETTINGS_PL, "Ustawienia dzia³ania CPU JAM" },
/* ru */ { IDMS_JAM_ACTION_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_JAM_ACTION_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDMS_JAM_ACTION_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDMS_FLIP_LIST,    "Flip list" },
/* da */ { IDMS_FLIP_LIST_DA, "Bladreliste" },
/* de */ { IDMS_FLIP_LIST_DE, "Flipliste" },
/* es */ { IDMS_FLIP_LIST_ES, "Lista agrupación discos" },
/* fr */ { IDMS_FLIP_LIST_FR, "Groupement de disques" },
/* hu */ { IDMS_FLIP_LIST_HU, "Lemezlista" },
/* it */ { IDMS_FLIP_LIST_IT, "Flip list" },
/* ko */ { IDMS_FLIP_LIST_KO, "" },  /* fuzzy */
/* nl */ { IDMS_FLIP_LIST_NL, "Wissellijst" },
/* pl */ { IDMS_FLIP_LIST_PL, "Lista podmiany no¶nika" },
/* ru */ { IDMS_FLIP_LIST_RU, "Flip list" },
/* sv */ { IDMS_FLIP_LIST_SV, "Vallista" },
/* tr */ { IDMS_FLIP_LIST_TR, "Sýralama listesi" },

/* en */ { IDMS_ADD_CURRENT_IMAGE,    "Add current image (Unit 8)" },
/* da */ { IDMS_ADD_CURRENT_IMAGE_DA, "Tilføj nuværende image (Enhed 8)" },
/* de */ { IDMS_ADD_CURRENT_IMAGE_DE, "Aktuelles Image hinzufügen (Gerät 8)" },
/* es */ { IDMS_ADD_CURRENT_IMAGE_ES, "Añadir imagen actual (Unidad 8)" },
/* fr */ { IDMS_ADD_CURRENT_IMAGE_FR, "Ajouter l'image de disque courante (#8) au groupement" },
/* hu */ { IDMS_ADD_CURRENT_IMAGE_HU, "Az aktuális képmás hozzáadása (8-as egység)" },
/* it */ { IDMS_ADD_CURRENT_IMAGE_IT, "Aggiungi immagine attuale (Unità 8)" },
/* ko */ { IDMS_ADD_CURRENT_IMAGE_KO, "ÇöÁ¦ ÀÌ¹ÌÁö (Unit 8) ´õÇÏ±â" },
/* nl */ { IDMS_ADD_CURRENT_IMAGE_NL, "Voeg huidig bestand toe (Drive 8)" },
/* pl */ { IDMS_ADD_CURRENT_IMAGE_PL, "Dodaj aktualny obraz (jednostka 8)" },
/* ru */ { IDMS_ADD_CURRENT_IMAGE_RU, "Add current image (Unit 8)" },
/* sv */ { IDMS_ADD_CURRENT_IMAGE_SV, "Lägg till aktuell avbildning (enhet 8)" },
/* tr */ { IDMS_ADD_CURRENT_IMAGE_TR, "Geçerli imajý ekle (Birim 8)" },

/* en */ { IDMS_REMOVE_CURRENT_IMAGE,    "Remove current image (Unit 8)" },
/* da */ { IDMS_REMOVE_CURRENT_IMAGE_DA, "Fjern nuværende image (Enhed 8)" },
/* de */ { IDMS_REMOVE_CURRENT_IMAGE_DE, "Aktuelles Image entfernen (Gerät 8)" },
/* es */ { IDMS_REMOVE_CURRENT_IMAGE_ES, "Quitar imagen actual (Unidad 8)" },
/* fr */ { IDMS_REMOVE_CURRENT_IMAGE_FR, "Retirer l'image de disque courante (#8) au groupement" },
/* hu */ { IDMS_REMOVE_CURRENT_IMAGE_HU, "Az aktuális képmás eltávolítása (8-as egység)" },
/* it */ { IDMS_REMOVE_CURRENT_IMAGE_IT, "Rimuovi immagine attuale (Unità 8)" },
/* ko */ { IDMS_REMOVE_CURRENT_IMAGE_KO, "ÇöÁ¦ ÀÌ¹ÌÁö (Unit 8) »èÁ¦ÇÏ±â" },
/* nl */ { IDMS_REMOVE_CURRENT_IMAGE_NL, "Verwijder huidig bestand (Drive 8)" },
/* pl */ { IDMS_REMOVE_CURRENT_IMAGE_PL, "Usuñ aktualny obraz (jednostka 8)" },
/* ru */ { IDMS_REMOVE_CURRENT_IMAGE_RU, "Remove current image (Unit 8)" },
/* sv */ { IDMS_REMOVE_CURRENT_IMAGE_SV, "Ta bort aktuell avbildning (enhet 8)" },
/* tr */ { IDMS_REMOVE_CURRENT_IMAGE_TR, "Geçerli imajý kaldýr (Birim 8)" },

/* en */ { IDMS_ATTACH_NEXT_IMAGE,    "Attach next image (Unit 8)" },
/* da */ { IDMS_ATTACH_NEXT_IMAGE_DA, "Tilslut næste image (Enhed 8)" },
/* de */ { IDMS_ATTACH_NEXT_IMAGE_DE, "Nächstes Image (Gerät 8)" },
/* es */ { IDMS_ATTACH_NEXT_IMAGE_ES, "Insertar imagen siguiente (Unidad 8)" },
/* fr */ { IDMS_ATTACH_NEXT_IMAGE_FR, "Insérer le prochain disque du groupement dans le lecteur #8" },
/* hu */ { IDMS_ATTACH_NEXT_IMAGE_HU, "Következõ képmás csatolása (8-as egység)" },
/* it */ { IDMS_ATTACH_NEXT_IMAGE_IT, "Seleziona immagine successiva (Unità 8)" },
/* ko */ { IDMS_ATTACH_NEXT_IMAGE_KO, "´ÙÀ½ ÀÌ¹ÌÁö (Unit 8) ºÙÀÌ±â" },
/* nl */ { IDMS_ATTACH_NEXT_IMAGE_NL, "Koppel volgend bestand (Drive 8)" },
/* pl */ { IDMS_ATTACH_NEXT_IMAGE_PL, "Zamontuj nastêpny obraz (jednostka 8)" },
/* ru */ { IDMS_ATTACH_NEXT_IMAGE_RU, "Attach next image (Unit 8)" },
/* sv */ { IDMS_ATTACH_NEXT_IMAGE_SV, "Anslut nästa avbildning (enhet 8)" },
/* tr */ { IDMS_ATTACH_NEXT_IMAGE_TR, "Sonraki imajý yerleþtir (Birim 8)" },

/* en */ { IDMS_ATTACH_PREVIOUS_IMAGE,    "Attach previous image (Unit 8)" },
/* da */ { IDMS_ATTACH_PREVIOUS_IMAGE_DA, "Tilslut forrige image (Enhed 8)" },
/* de */ { IDMS_ATTACH_PREVIOUS_IMAGE_DE, "Voriges Image (Gerät 8)" },
/* es */ { IDMS_ATTACH_PREVIOUS_IMAGE_ES, "Insertar imagen previa (Unidad 8)" },
/* fr */ { IDMS_ATTACH_PREVIOUS_IMAGE_FR, "Insérer le disque précédent du groupement dans le lecteur #8" },
/* hu */ { IDMS_ATTACH_PREVIOUS_IMAGE_HU, "Elõzõ képmás csatolása (8-as egység)" },
/* it */ { IDMS_ATTACH_PREVIOUS_IMAGE_IT, "Seleziona immagine precedente (Unità 8)" },
/* ko */ { IDMS_ATTACH_PREVIOUS_IMAGE_KO, "ÀÌÀü ÀÌ¹ÌÁö (Unit 8) ºÙÀÌ±â" },
/* nl */ { IDMS_ATTACH_PREVIOUS_IMAGE_NL, "Koppel vorig bestand (Drive 8)" },
/* pl */ { IDMS_ATTACH_PREVIOUS_IMAGE_PL, "Zamontuj poprzedni obraz (jednostka 8)" },
/* ru */ { IDMS_ATTACH_PREVIOUS_IMAGE_RU, "Attach previous image (Unit 8)" },
/* sv */ { IDMS_ATTACH_PREVIOUS_IMAGE_SV, "Anslut föregående avbildning (enhet 8)" },
/* tr */ { IDMS_ATTACH_PREVIOUS_IMAGE_TR, "Önceki imajý yerleþtir (Birim 8)" },

/* en */ { IDMS_LOAD_FLIP_LIST,    "Load flip list file" },
/* da */ { IDMS_LOAD_FLIP_LIST_DA, "Indlæs bladrelistefil" },
/* de */ { IDMS_LOAD_FLIP_LIST_DE, "Fliplist Datei laden" },
/* es */ { IDMS_LOAD_FLIP_LIST_ES, "Cargar lista de ficheros a conmutar" },
/* fr */ { IDMS_LOAD_FLIP_LIST_FR, "Charger un fichier de groupement de disques" },
/* hu */ { IDMS_LOAD_FLIP_LIST_HU, "Lemezlista betöltése" },
/* it */ { IDMS_LOAD_FLIP_LIST_IT, "Carica file fliplist" },
/* ko */ { IDMS_LOAD_FLIP_LIST_KO, "ÇÃ¸³ ¸®½ºÆ® ÆÄÀÏ ºÒ·¯¿À±â" },
/* nl */ { IDMS_LOAD_FLIP_LIST_NL, "Laad fliplijstbestand" },
/* pl */ { IDMS_LOAD_FLIP_LIST_PL, "Wczytaj plik listy podmiany no¶ników" },
/* ru */ { IDMS_LOAD_FLIP_LIST_RU, "Load flip list file" },
/* sv */ { IDMS_LOAD_FLIP_LIST_SV, "Läs vallistefil" },
/* tr */ { IDMS_LOAD_FLIP_LIST_TR, "Sýralama listesi dosyasýný yükle" },

/* en */ { IDMS_SAVE_FLIP_LIST,    "Save flip list file" },
/* da */ { IDMS_SAVE_FLIP_LIST_DA, "Gem bladrelistefil" },
/* de */ { IDMS_SAVE_FLIP_LIST_DE, "Fliplist Datei speichern" },
/* es */ { IDMS_SAVE_FLIP_LIST_ES, "Grabar lista de ficheros a conmutar" },
/* fr */ { IDMS_SAVE_FLIP_LIST_FR, "Enregistrer le fichier de groupement de disques actuel" },
/* hu */ { IDMS_SAVE_FLIP_LIST_HU, "Lemezlista mentése fájlba" },
/* it */ { IDMS_SAVE_FLIP_LIST_IT, "Salva file fliplist" },
/* ko */ { IDMS_SAVE_FLIP_LIST_KO, "ÇÃ¸³ ¸®½ºÆ® ÆÄÀÏ ÀúÀåÇÏ±â" },
/* nl */ { IDMS_SAVE_FLIP_LIST_NL, "Fliplijstbestand opslaan" },
/* pl */ { IDMS_SAVE_FLIP_LIST_PL, "Zapisz plik listy podmiany no¶ników" },
/* ru */ { IDMS_SAVE_FLIP_LIST_RU, "Save flip list file" },
/* sv */ { IDMS_SAVE_FLIP_LIST_SV, "Spara vallistefil" },
/* tr */ { IDMS_SAVE_FLIP_LIST_TR, "Sýralama listesi dosyasýný kaydet" },

/* en */ { IDMS_ATTACH_TAPE_IMAGE,    "Attach tape image..." },
/* da */ { IDMS_ATTACH_TAPE_IMAGE_DA, "Tilslut bånd-image..." },
/* de */ { IDMS_ATTACH_TAPE_IMAGE_DE, "Band Image einlegen..." },
/* es */ { IDMS_ATTACH_TAPE_IMAGE_ES, "Insertar imagen de cinta..." },
/* fr */ { IDMS_ATTACH_TAPE_IMAGE_FR, "Insérer une image de datassette..." },
/* hu */ { IDMS_ATTACH_TAPE_IMAGE_HU, "Szalag képmás csatolása..." },
/* it */ { IDMS_ATTACH_TAPE_IMAGE_IT, "Seleziona immagine cassetta..." },
/* ko */ { IDMS_ATTACH_TAPE_IMAGE_KO, "Å×ÀÌÇÁ ÀÌ¹ÌÁö ºÙ¿©³Ö±â..." },
/* nl */ { IDMS_ATTACH_TAPE_IMAGE_NL, "Koppel tapebestand..." },
/* pl */ { IDMS_ATTACH_TAPE_IMAGE_PL, "Zamontuj obraz ta¶my..." },
/* ru */ { IDMS_ATTACH_TAPE_IMAGE_RU, "´ÞÑÐÒØâì ÞÑàÐ× ×ÐßØáØ..." },
/* sv */ { IDMS_ATTACH_TAPE_IMAGE_SV, "Anslut bandavbildningsfil..." },
/* tr */ { IDMS_ATTACH_TAPE_IMAGE_TR, "Teyp imajýný yerleþtir..." },

/* en */ { IDMS_DETACH_TAPE_IMAGE,    "Detach tape image" },
/* da */ { IDMS_DETACH_TAPE_IMAGE_DA, "Frakobl bånd-image" },
/* de */ { IDMS_DETACH_TAPE_IMAGE_DE, "Band Image entfernen" },
/* es */ { IDMS_DETACH_TAPE_IMAGE_ES, "Extraer imagen de cinta" },
/* fr */ { IDMS_DETACH_TAPE_IMAGE_FR, "Détacher une image datassette..." },
/* hu */ { IDMS_DETACH_TAPE_IMAGE_HU, "Szalag képmás leválasztása" },
/* it */ { IDMS_DETACH_TAPE_IMAGE_IT, "Rimuovi immagine cassetta" },
/* ko */ { IDMS_DETACH_TAPE_IMAGE_KO, "Å×ÀÌÇÁ ÀÌ¹ÌÁö ¶§¾î³»±â" },
/* nl */ { IDMS_DETACH_TAPE_IMAGE_NL, "Ontkoppel tapebestand" },
/* pl */ { IDMS_DETACH_TAPE_IMAGE_PL, "Wyjmij obraz ta¶my" },
/* ru */ { IDMS_DETACH_TAPE_IMAGE_RU, "ÃÑàÐâì ÞÑàÐ× ×ÐßØáØ" },
/* sv */ { IDMS_DETACH_TAPE_IMAGE_SV, "Koppla från bandavbildningsfil" },
/* tr */ { IDMS_DETACH_TAPE_IMAGE_TR, "Teyp imajýný çýkart" },

/* en */ { IDMS_DATASSETTE_CONTROL,    "Datassette control" },
/* da */ { IDMS_DATASSETTE_CONTROL_DA, "Datasettestyring" },
/* de */ { IDMS_DATASSETTE_CONTROL_DE, "Bandlaufwerk Kontrolle" },
/* es */ { IDMS_DATASSETTE_CONTROL_ES, "Control del datasette" },
/* fr */ { IDMS_DATASSETTE_CONTROL_FR, "Contrôle datassette" },
/* hu */ { IDMS_DATASSETTE_CONTROL_HU, "Magnó vezérlés" },
/* it */ { IDMS_DATASSETTE_CONTROL_IT, "Comandi registratore" },
/* ko */ { IDMS_DATASSETTE_CONTROL_KO, "µ¥ÀÌÅ¸¼Â Ä¿Æ®·Ñ" },
/* nl */ { IDMS_DATASSETTE_CONTROL_NL, "Datasette regeling" },
/* pl */ { IDMS_DATASSETTE_CONTROL_PL, "Zarz±dzanie magnetofonem" },
/* ru */ { IDMS_DATASSETTE_CONTROL_RU, "Datassette control" },
/* sv */ { IDMS_DATASSETTE_CONTROL_SV, "Datasettestyrning" },
/* tr */ { IDMS_DATASSETTE_CONTROL_TR, "Teyp kontrolü" },

/* en */ { IDMS_STOP,    "Stop" },
/* da */ { IDMS_STOP_DA, "Stop" },
/* de */ { IDMS_STOP_DE, "Stopp" },
/* es */ { IDMS_STOP_ES, "Detener" },
/* fr */ { IDMS_STOP_FR, "Arrêt" },
/* hu */ { IDMS_STOP_HU, "Leállítás" },
/* it */ { IDMS_STOP_IT, "Stop" },
/* ko */ { IDMS_STOP_KO, "¸ØÃã" },
/* nl */ { IDMS_STOP_NL, "Stop" },
/* pl */ { IDMS_STOP_PL, "Zatrzymaj" },
/* ru */ { IDMS_STOP_RU, "¾áâÐÝÞÒØâì" },
/* sv */ { IDMS_STOP_SV, "Stoppa" },
/* tr */ { IDMS_STOP_TR, "Dur" },

/* en */ { IDMS_START,    "Start" },
/* da */ { IDMS_START_DA, "Start" },
/* de */ { IDMS_START_DE, "Start" },
/* es */ { IDMS_START_ES, "Arrancar" },
/* fr */ { IDMS_START_FR, "Démarrer" },
/* hu */ { IDMS_START_HU, "Indítás" },
/* it */ { IDMS_START_IT, "Avvia" },
/* ko */ { IDMS_START_KO, "½ÃÀÛ" },
/* nl */ { IDMS_START_NL, "Start" },
/* pl */ { IDMS_START_PL, "Start" },
/* ru */ { IDMS_START_RU, "Start" },
/* sv */ { IDMS_START_SV, "Starta" },
/* tr */ { IDMS_START_TR, "Baþlat" },

/* en */ { IDMS_FORWARD,    "Forward" },
/* da */ { IDMS_FORWARD_DA, "Spol frem" },
/* de */ { IDMS_FORWARD_DE, "Vorspulen" },
/* es */ { IDMS_FORWARD_ES, "Avance rápido" },
/* fr */ { IDMS_FORWARD_FR, "En avant" },
/* hu */ { IDMS_FORWARD_HU, "Elõrecsévélés" },
/* it */ { IDMS_FORWARD_IT, "Avanti" },
/* ko */ { IDMS_FORWARD_KO, "»¡¸®°¨±â" },
/* nl */ { IDMS_FORWARD_NL, "Vooruit" },
/* pl */ { IDMS_FORWARD_PL, "Przewiñ do przodu" },
/* ru */ { IDMS_FORWARD_RU, "¿ÕàÕÜÞâÚÐ ÒßÕàÕÔ" },
/* sv */ { IDMS_FORWARD_SV, "Spola framåt" },
/* tr */ { IDMS_FORWARD_TR, "Ýleri" },

/* en */ { IDMS_REWIND,    "Rewind" },
/* da */ { IDMS_REWIND_DA, "Spol tilbage" },
/* de */ { IDMS_REWIND_DE, "Rückspulen" },
/* es */ { IDMS_REWIND_ES, "Rebobinar" },
/* fr */ { IDMS_REWIND_FR, "En arrière" },
/* hu */ { IDMS_REWIND_HU, "Visszacsévélés" },
/* it */ { IDMS_REWIND_IT, "Indietro" },
/* ko */ { IDMS_REWIND_KO, "µÇ°¨±â" },
/* nl */ { IDMS_REWIND_NL, "Terug" },
/* pl */ { IDMS_REWIND_PL, "Przewiñ do ty³u" },
/* ru */ { IDMS_REWIND_RU, "¿ÕàÕÜÞâÚÐ ÝÐ×ÐÔ" },
/* sv */ { IDMS_REWIND_SV, "Spola bakåt" },
/* tr */ { IDMS_REWIND_TR, "Geri" },

/* en */ { IDMS_RECORD,    "Record" },
/* da */ { IDMS_RECORD_DA, "Optag" },
/* de */ { IDMS_RECORD_DE, "Aufnahme" },
/* es */ { IDMS_RECORD_ES, "Grabar" },
/* fr */ { IDMS_RECORD_FR, "Enregistrer" },
/* hu */ { IDMS_RECORD_HU, "Felvétel" },
/* it */ { IDMS_RECORD_IT, "Registra" },
/* ko */ { IDMS_RECORD_KO, "³ìÈ­" },
/* nl */ { IDMS_RECORD_NL, "Opname" },
/* pl */ { IDMS_RECORD_PL, "Nagrywaj" },
/* ru */ { IDMS_RECORD_RU, "·ÐßØáì" },
/* sv */ { IDMS_RECORD_SV, "Spela in" },
/* tr */ { IDMS_RECORD_TR, "Kayýt" },

/* en */ { IDMS_RESET,    "Reset" },
/* da */ { IDMS_RESET_DA, "Reset" },
/* de */ { IDMS_RESET_DE, "Reset" },
/* es */ { IDMS_RESET_ES, "Reiniciar" },
/* fr */ { IDMS_RESET_FR, "Réinitialiser" },
/* hu */ { IDMS_RESET_HU, "Reset" },
/* it */ { IDMS_RESET_IT, "Reset" },
/* ko */ { IDMS_RESET_KO, "¸®¼Â" },
/* nl */ { IDMS_RESET_NL, "Reset" },
/* pl */ { IDMS_RESET_PL, "Reset" },
/* ru */ { IDMS_RESET_RU, "Reset" },
/* sv */ { IDMS_RESET_SV, "Återställ" },
/* tr */ { IDMS_RESET_TR, "Reset" },

/* en */ { IDMS_RESET_COUNTER,    "Reset Counter" },
/* da */ { IDMS_RESET_COUNTER_DA, "Nulstil tæller" },
/* de */ { IDMS_RESET_COUNTER_DE, "Zähler zurücksetzen" },
/* es */ { IDMS_RESET_COUNTER_ES, "Reiniciar contador" },
/* fr */ { IDMS_RESET_COUNTER_FR, "Réinialiser le compteur" },
/* hu */ { IDMS_RESET_COUNTER_HU, "Számláló nullázása" },
/* it */ { IDMS_RESET_COUNTER_IT, "Reset contantore" },
/* ko */ { IDMS_RESET_COUNTER_KO, "¸®¼Â Ä«¿îÅÍ" },
/* nl */ { IDMS_RESET_COUNTER_NL, "Reset Teller" },
/* pl */ { IDMS_RESET_COUNTER_PL, "Skasuj licznik" },
/* ru */ { IDMS_RESET_COUNTER_RU, "Reset Counter" },
/* sv */ { IDMS_RESET_COUNTER_SV, "Nollställ räknare" },
/* tr */ { IDMS_RESET_COUNTER_TR, "Sayacý Resetle" },

/* en */ { IDMS_ATTACH_CART_IMAGE,    "Attach cartridge image..." },
/* da */ { IDMS_ATTACH_CART_IMAGE_DA, "Tilslut cartridge-image..." },
/* de */ { IDMS_ATTACH_CART_IMAGE_DE, "Erweiterungsmodul einlegen..." },
/* es */ { IDMS_ATTACH_CART_IMAGE_ES, "Insertar imagen de cartucho..." },
/* fr */ { IDMS_ATTACH_CART_IMAGE_FR, "Insérer une cartouche..." },
/* hu */ { IDMS_ATTACH_CART_IMAGE_HU, "Cartridge képmás csatolása..." },
/* it */ { IDMS_ATTACH_CART_IMAGE_IT, "Seleziona immagine cartuccia..." },
/* ko */ { IDMS_ATTACH_CART_IMAGE_KO, "Ä«Æ®¸®Áö ÀÌ¹ÌÁö ºÙÀÌ±â..." },
/* nl */ { IDMS_ATTACH_CART_IMAGE_NL, "Koppel cartridge bestand..." },
/* pl */ { IDMS_ATTACH_CART_IMAGE_PL, "Zamontuj obraz kartrid¿a..." },
/* ru */ { IDMS_ATTACH_CART_IMAGE_RU, "´ÞÑÐÒØâì ÞÑàÐ× ÚÐàâàØÔÖÐ..." },
/* sv */ { IDMS_ATTACH_CART_IMAGE_SV, "Anslut insticksmodulfil..." },
/* tr */ { IDMS_ATTACH_CART_IMAGE_TR, "Kartuþ imajý yerleþtir..." },

/* en */ { IDMS_GENERIC_IMAGE,    "Generic image..." },
/* da */ { IDMS_GENERIC_IMAGE_DA, "Standard-image..." },
/* de */ { IDMS_GENERIC_IMAGE_DE, "Generisches Image..." },
/* es */ { IDMS_GENERIC_IMAGE_ES, "Imagen generica..." },
/* fr */ { IDMS_GENERIC_IMAGE_FR, "Image générique..." },
/* hu */ { IDMS_GENERIC_IMAGE_HU, "" },  /* fuzzy */
/* it */ { IDMS_GENERIC_IMAGE_IT, "Immagine generica..." },
/* ko */ { IDMS_GENERIC_IMAGE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_GENERIC_IMAGE_NL, "Algemeen bestand..." },
/* pl */ { IDMS_GENERIC_IMAGE_PL, "Standardowy obraz..." },
/* ru */ { IDMS_GENERIC_IMAGE_RU, "Generic image..." },
/* sv */ { IDMS_GENERIC_IMAGE_SV, "Generisk avbild..." },
/* tr */ { IDMS_GENERIC_IMAGE_TR, "Jenerik imaj..." },

/* en */ { IDMS_BEHR_BONZ_IMAGE,    "Behr Bonz image..." },
/* da */ { IDMS_BEHR_BONZ_IMAGE_DA, "" },  /* fuzzy */
/* de */ { IDMS_BEHR_BONZ_IMAGE_DE, "Behr Bonz Image..." },
/* es */ { IDMS_BEHR_BONZ_IMAGE_ES, "" },  /* fuzzy */
/* fr */ { IDMS_BEHR_BONZ_IMAGE_FR, "" },  /* fuzzy */
/* hu */ { IDMS_BEHR_BONZ_IMAGE_HU, "" },  /* fuzzy */
/* it */ { IDMS_BEHR_BONZ_IMAGE_IT, "" },  /* fuzzy */
/* ko */ { IDMS_BEHR_BONZ_IMAGE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_BEHR_BONZ_IMAGE_NL, "Behr Bonz bestand..." },
/* pl */ { IDMS_BEHR_BONZ_IMAGE_PL, "" },  /* fuzzy */
/* ru */ { IDMS_BEHR_BONZ_IMAGE_RU, "" },  /* fuzzy */
/* sv */ { IDMS_BEHR_BONZ_IMAGE_SV, "" },  /* fuzzy */
/* tr */ { IDMS_BEHR_BONZ_IMAGE_TR, "" },  /* fuzzy */

/* en */ { IDMS_MEGA_CART_IMAGE,    "Mega-Cart image..." },
/* da */ { IDMS_MEGA_CART_IMAGE_DA, "Mega-Cart image..." },
/* de */ { IDMS_MEGA_CART_IMAGE_DE, "Mega-Cart Image..." },
/* es */ { IDMS_MEGA_CART_IMAGE_ES, "Imagen Mega-Cart..." },
/* fr */ { IDMS_MEGA_CART_IMAGE_FR, "Image Mega-Cart..." },
/* hu */ { IDMS_MEGA_CART_IMAGE_HU, "" },  /* fuzzy */
/* it */ { IDMS_MEGA_CART_IMAGE_IT, "Immagine Mega-Cart..." },
/* ko */ { IDMS_MEGA_CART_IMAGE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_MEGA_CART_IMAGE_NL, "Mega-Cart bestand..." },
/* pl */ { IDMS_MEGA_CART_IMAGE_PL, "Obraz Mega-Cart..." },
/* ru */ { IDMS_MEGA_CART_IMAGE_RU, "Mega-Cart image..." },
/* sv */ { IDMS_MEGA_CART_IMAGE_SV, "Mega-Cart-avbildning..." },
/* tr */ { IDMS_MEGA_CART_IMAGE_TR, "Mega-Cart imajý..." },

/* en */ { IDMS_FINAL_EXPANSION_IMAGE,    "Final Expansion image..." },
/* da */ { IDMS_FINAL_EXPANSION_IMAGE_DA, "Final Expansion-image..." },
/* de */ { IDMS_FINAL_EXPANSION_IMAGE_DE, "Final Expansion Image..." },
/* es */ { IDMS_FINAL_EXPANSION_IMAGE_ES, "Imagen Final Expansion..." },
/* fr */ { IDMS_FINAL_EXPANSION_IMAGE_FR, "Image Final Expansion..." },
/* hu */ { IDMS_FINAL_EXPANSION_IMAGE_HU, "" },  /* fuzzy */
/* it */ { IDMS_FINAL_EXPANSION_IMAGE_IT, "Immagine Final Expansion..." },
/* ko */ { IDMS_FINAL_EXPANSION_IMAGE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_FINAL_EXPANSION_IMAGE_NL, "Final Expansion bestand..." },
/* pl */ { IDMS_FINAL_EXPANSION_IMAGE_PL, "Obraz Final Expansion..." },
/* ru */ { IDMS_FINAL_EXPANSION_IMAGE_RU, "" },  /* fuzzy */
/* sv */ { IDMS_FINAL_EXPANSION_IMAGE_SV, "Final Expansion-fil..." },
/* tr */ { IDMS_FINAL_EXPANSION_IMAGE_TR, "Final Expansion imajý..." },

/* en */ { IDMS_FP_IMAGE,    "Vic Flash Plugin image..." },
/* da */ { IDMS_FP_IMAGE_DA, "Vic Flash Plugin-image..." },
/* de */ { IDMS_FP_IMAGE_DE, "Vic Flash Plugin Image..." },
/* es */ { IDMS_FP_IMAGE_ES, "Imagen Vic Flash Plugin..." },
/* fr */ { IDMS_FP_IMAGE_FR, "Image Vic Flash Plugin..." },
/* hu */ { IDMS_FP_IMAGE_HU, "" },  /* fuzzy */
/* it */ { IDMS_FP_IMAGE_IT, "Immagine Vic Flash Plugin..." },
/* ko */ { IDMS_FP_IMAGE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_FP_IMAGE_NL, "Vic Flash Plugin bestand..." },
/* pl */ { IDMS_FP_IMAGE_PL, "Obraz Vic Flash Plugin..." },
/* ru */ { IDMS_FP_IMAGE_RU, "Vic Flash Plugin image..." },
/* sv */ { IDMS_FP_IMAGE_SV, "Vic Flash insticksmodul-fil..." },
/* tr */ { IDMS_FP_IMAGE_TR, "Vic Flash Plugin imajý..." },

/* en */ { IDMS_UM_IMAGE,    "UltiMem image..." },
/* da */ { IDMS_UM_IMAGE_DA, "" },  /* fuzzy */
/* de */ { IDMS_UM_IMAGE_DE, "UltiMem Image..." },
/* es */ { IDMS_UM_IMAGE_ES, "" },  /* fuzzy */
/* fr */ { IDMS_UM_IMAGE_FR, "" },  /* fuzzy */
/* hu */ { IDMS_UM_IMAGE_HU, "" },  /* fuzzy */
/* it */ { IDMS_UM_IMAGE_IT, "" },  /* fuzzy */
/* ko */ { IDMS_UM_IMAGE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_UM_IMAGE_NL, "UltiMem bestand..." },
/* pl */ { IDMS_UM_IMAGE_PL, "" },  /* fuzzy */
/* ru */ { IDMS_UM_IMAGE_RU, "" },  /* fuzzy */
/* sv */ { IDMS_UM_IMAGE_SV, "" },  /* fuzzy */
/* tr */ { IDMS_UM_IMAGE_TR, "" },  /* fuzzy */

/* en */ { IDMS_ADD_TO_GENERIC,    "Add to generic cartridge..." },
/* da */ { IDMS_ADD_TO_GENERIC_DA, "Tilføj til standardcartridge..." },
/* de */ { IDMS_ADD_TO_GENERIC_DE, "Zu universellem Erweiterungsmodul hinzufügen..." },
/* es */ { IDMS_ADD_TO_GENERIC_ES, "Añadir al cartucho generico..." },
/* fr */ { IDMS_ADD_TO_GENERIC_FR, "Ajouter à une cartouche générique..." },
/* hu */ { IDMS_ADD_TO_GENERIC_HU, "Hozzáadás általános cartridge-hoz..." },
/* it */ { IDMS_ADD_TO_GENERIC_IT, "Aggiungi a cartuccia generica..." },
/* ko */ { IDMS_ADD_TO_GENERIC_KO, "Á¦³Ê¸¯ Ä«Æ®¸®Áö¿¡ ´õÇÏ±â..." },
/* nl */ { IDMS_ADD_TO_GENERIC_NL, "Voeg toe aan algemene cartridge..." },
/* pl */ { IDMS_ADD_TO_GENERIC_PL, "Dodaj do standardowego kartrid¿a..." },
/* ru */ { IDMS_ADD_TO_GENERIC_RU, "Add to generic cartridge..." },
/* sv */ { IDMS_ADD_TO_GENERIC_SV, "Lägg till generisk insticksmodul..." },
/* tr */ { IDMS_ADD_TO_GENERIC_TR, "Jenerik kartuþa ekle..." },

/* en */ { IDMS_SMART_ATTACH_IMAGE,    "Smart-attach image..." },
/* da */ { IDMS_SMART_ATTACH_IMAGE_DA, "Smart-tilslut image..." },
/* de */ { IDMS_SMART_ATTACH_IMAGE_DE, "Smart-Einlegen Image Date..." },
/* es */ { IDMS_SMART_ATTACH_IMAGE_ES, "Imagen Smart-attach..." },
/* fr */ { IDMS_SMART_ATTACH_IMAGE_FR, "Insertion intelligente d'une image..." },
/* hu */ { IDMS_SMART_ATTACH_IMAGE_HU, "" },  /* fuzzy */
/* it */ { IDMS_SMART_ATTACH_IMAGE_IT, "Immagine Smart-attach..." },
/* ko */ { IDMS_SMART_ATTACH_IMAGE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_SMART_ATTACH_IMAGE_NL, "Slimme bestandskoppeling..." },
/* pl */ { IDMS_SMART_ATTACH_IMAGE_PL, "Inteligentne montowanie obrazu..." },
/* ru */ { IDMS_SMART_ATTACH_IMAGE_RU, "Smart-attach image..." },
/* sv */ { IDMS_SMART_ATTACH_IMAGE_SV, "Smartanslut avbildning..." },
/* tr */ { IDMS_SMART_ATTACH_IMAGE_TR, "Smart-attach imajý..." },

/* en */ { IDMS_4_8_16KB_AT_2000,    "4/8/16KB image at $2000..." },
/* da */ { IDMS_4_8_16KB_AT_2000_DA, "4/8/16KB-image på $2000..." },
/* de */ { IDMS_4_8_16KB_AT_2000_DE, "4/8/16KB Image bei $2000..." },
/* es */ { IDMS_4_8_16KB_AT_2000_ES, "4/8/16KB imagen en $2000..." },
/* fr */ { IDMS_4_8_16KB_AT_2000_FR, "image 4/8/16 ko à $2000..." },
/* hu */ { IDMS_4_8_16KB_AT_2000_HU, "" },  /* fuzzy */
/* it */ { IDMS_4_8_16KB_AT_2000_IT, "Immagine di 4/8/16KB a $2000..." },
/* ko */ { IDMS_4_8_16KB_AT_2000_KO, "" },  /* fuzzy */
/* nl */ { IDMS_4_8_16KB_AT_2000_NL, "4/8/16KB bestand in $2000..." },
/* pl */ { IDMS_4_8_16KB_AT_2000_PL, "4/8/16KB obraz w $2000..." },
/* ru */ { IDMS_4_8_16KB_AT_2000_RU, "4/8/16KB image at $2000..." },
/* sv */ { IDMS_4_8_16KB_AT_2000_SV, "4/8/16KB-fil vid $2000..." },
/* tr */ { IDMS_4_8_16KB_AT_2000_TR, "4/8/16KB imajý $2000 adresinde..." },

/* en */ { IDMS_4_8_16KB_AT_4000,    "4/8/16KB image at $4000..." },
/* da */ { IDMS_4_8_16KB_AT_4000_DA, "4/8/16KB-image på $4000..." },
/* de */ { IDMS_4_8_16KB_AT_4000_DE, "4/8/16KB Image bei $4000..." },
/* es */ { IDMS_4_8_16KB_AT_4000_ES, "4/8/16KB imagen en $4000..." },
/* fr */ { IDMS_4_8_16KB_AT_4000_FR, "image 4/8/16 ko à $4000..." },
/* hu */ { IDMS_4_8_16KB_AT_4000_HU, "" },  /* fuzzy */
/* it */ { IDMS_4_8_16KB_AT_4000_IT, "Immagine di 4/8/16KB a $4000..." },
/* ko */ { IDMS_4_8_16KB_AT_4000_KO, "" },  /* fuzzy */
/* nl */ { IDMS_4_8_16KB_AT_4000_NL, "4/8/16KB bestand in $4000..." },
/* pl */ { IDMS_4_8_16KB_AT_4000_PL, "4/8/16KB obraz w $4000..." },
/* ru */ { IDMS_4_8_16KB_AT_4000_RU, "4/8/16KB image at $4000..." },
/* sv */ { IDMS_4_8_16KB_AT_4000_SV, "4/8/16KB-fil vid $4000..." },
/* tr */ { IDMS_4_8_16KB_AT_4000_TR, "4/8/16KB imajý $4000 adresinde..." },

/* en */ { IDMS_4_8_16KB_AT_6000,    "4/8/16KB image at $6000..." },
/* da */ { IDMS_4_8_16KB_AT_6000_DA, "4/8/16KB-image på $6000..." },
/* de */ { IDMS_4_8_16KB_AT_6000_DE, "4/8/16KB Image bei $6000..." },
/* es */ { IDMS_4_8_16KB_AT_6000_ES, "4/8/16KB imagen en $6000..." },
/* fr */ { IDMS_4_8_16KB_AT_6000_FR, "image 4/8/16 ko à $6000..." },
/* hu */ { IDMS_4_8_16KB_AT_6000_HU, "" },  /* fuzzy */
/* it */ { IDMS_4_8_16KB_AT_6000_IT, "Immagine di 4/8/16KB a $6000..." },
/* ko */ { IDMS_4_8_16KB_AT_6000_KO, "" },  /* fuzzy */
/* nl */ { IDMS_4_8_16KB_AT_6000_NL, "4/8/16KB bestand in $6000..." },
/* pl */ { IDMS_4_8_16KB_AT_6000_PL, "4/8/16KB obraz w $6000..." },
/* ru */ { IDMS_4_8_16KB_AT_6000_RU, "4/8/16KB image at $6000..." },
/* sv */ { IDMS_4_8_16KB_AT_6000_SV, "4/8/16KB-fil vid $6000..." },
/* tr */ { IDMS_4_8_16KB_AT_6000_TR, "4/8/16KB imajý $6000 adresinde..." },

/* en */ { IDMS_4_8KB_AT_A000,    "4/8KB image at $A000..." },
/* da */ { IDMS_4_8KB_AT_A000_DA, "4/8KB-image på $A000..." },
/* de */ { IDMS_4_8KB_AT_A000_DE, "4/8KB Image bei $A000..." },
/* es */ { IDMS_4_8KB_AT_A000_ES, "4/8KB imagen en $A000..." },
/* fr */ { IDMS_4_8KB_AT_A000_FR, "image 4/8 ko à $A000..." },
/* hu */ { IDMS_4_8KB_AT_A000_HU, "" },  /* fuzzy */
/* it */ { IDMS_4_8KB_AT_A000_IT, "Immagine di 4/8KB a $A000..." },
/* ko */ { IDMS_4_8KB_AT_A000_KO, "" },  /* fuzzy */
/* nl */ { IDMS_4_8KB_AT_A000_NL, "4/8KB bestand in $A000..." },
/* pl */ { IDMS_4_8KB_AT_A000_PL, "4/8KB obraz w $A000..." },
/* ru */ { IDMS_4_8KB_AT_A000_RU, "4/8KB image at $A000..." },
/* sv */ { IDMS_4_8KB_AT_A000_SV, "4/8KB-fil vid $A000..." },
/* tr */ { IDMS_4_8KB_AT_A000_TR, "4/8KB imajý $A000 adresinde..." },

/* en */ { IDMS_4KB_AT_B000,    "4KB image at $B000..." },
/* da */ { IDMS_4KB_AT_B000_DA, "4KB-image på $B000..." },
/* de */ { IDMS_4KB_AT_B000_DE, "4KB Image bei $B000..." },
/* es */ { IDMS_4KB_AT_B000_ES, "4KB imagen en $B000..." },
/* fr */ { IDMS_4KB_AT_B000_FR, "image 4 ko à $B000..." },
/* hu */ { IDMS_4KB_AT_B000_HU, "" },  /* fuzzy */
/* it */ { IDMS_4KB_AT_B000_IT, "Immagine di 4KB a $B000..." },
/* ko */ { IDMS_4KB_AT_B000_KO, "" },  /* fuzzy */
/* nl */ { IDMS_4KB_AT_B000_NL, "4KB bestand in $B000..." },
/* pl */ { IDMS_4KB_AT_B000_PL, "4KB obraz w $B000..." },
/* ru */ { IDMS_4KB_AT_B000_RU, "4KB image at $B000..." },
/* sv */ { IDMS_4KB_AT_B000_SV, "4KB-fil vid $B000..." },
/* tr */ { IDMS_4KB_AT_B000_TR, "4KB imajý $B000 adresinde..." },

/* en */ { IDMS_DETACH_CART_IMAGE,    "Detach cartridge image(s)" },
/* da */ { IDMS_DETACH_CART_IMAGE_DA, "Frakobl cartridge-image" },
/* de */ { IDMS_DETACH_CART_IMAGE_DE, "Erweiterungsmodul Image(s) entfernen" },
/* es */ { IDMS_DETACH_CART_IMAGE_ES, "Extraer imagen(es) cartucho" },
/* fr */ { IDMS_DETACH_CART_IMAGE_FR, "Détacher une cartouche" },
/* hu */ { IDMS_DETACH_CART_IMAGE_HU, "Cartridge képmás(ok) leválasztása" },
/* it */ { IDMS_DETACH_CART_IMAGE_IT, "Rimuovi le immagini delle cartucce" },
/* ko */ { IDMS_DETACH_CART_IMAGE_KO, "Ä«Æ®¸®Áö ÀÌ¹ÌÁö ¶¼±â" },
/* nl */ { IDMS_DETACH_CART_IMAGE_NL, "Ontkoppel cartridge bestand(en)" },
/* pl */ { IDMS_DETACH_CART_IMAGE_PL, "Wysuñ obraz(y) kartrid¿a" },
/* ru */ { IDMS_DETACH_CART_IMAGE_RU, "ÃÑàÐâì ÞÑàÐ×ë ÚÐàâàØÔÖÕÙ" },
/* sv */ { IDMS_DETACH_CART_IMAGE_SV, "Koppla från insticksmodulfil(er)" },
/* tr */ { IDMS_DETACH_CART_IMAGE_TR, "Kartuþ imaj(lar)ýný çýkart" },

/* en */ { IDMS_C0_LOW_IMAGE,    "C0 low  image..." },
/* da */ { IDMS_C0_LOW_IMAGE_DA, "C0 lav-image..." },
/* de */ { IDMS_C0_LOW_IMAGE_DE, "C0 low Imagedatei..." },
/* es */ { IDMS_C0_LOW_IMAGE_ES, "Imagen C0 bajo..." },
/* fr */ { IDMS_C0_LOW_IMAGE_FR, "Image C0 basse..." },
/* hu */ { IDMS_C0_LOW_IMAGE_HU, "" },  /* fuzzy */
/* it */ { IDMS_C0_LOW_IMAGE_IT, "Immagine C0 bassa..." },
/* ko */ { IDMS_C0_LOW_IMAGE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C0_LOW_IMAGE_NL, "C0 low bestand..." },
/* pl */ { IDMS_C0_LOW_IMAGE_PL, "Obraz C0 low..." },
/* ru */ { IDMS_C0_LOW_IMAGE_RU, "" },  /* fuzzy */
/* sv */ { IDMS_C0_LOW_IMAGE_SV, "C0 låg-avbildning..." },
/* tr */ { IDMS_C0_LOW_IMAGE_TR, "C0 low  imajý..." },

/* en */ { IDMS_C0_HIGH_IMAGE,    "C0 high image..." },
/* da */ { IDMS_C0_HIGH_IMAGE_DA, "C0 høj-image..." },
/* de */ { IDMS_C0_HIGH_IMAGE_DE, "C0 high Imagedatei..." },
/* es */ { IDMS_C0_HIGH_IMAGE_ES, "Imagen C0 alto..." },
/* fr */ { IDMS_C0_HIGH_IMAGE_FR, "Image C0 haute..." },
/* hu */ { IDMS_C0_HIGH_IMAGE_HU, "" },  /* fuzzy */
/* it */ { IDMS_C0_HIGH_IMAGE_IT, "Immagine C0 alta..." },
/* ko */ { IDMS_C0_HIGH_IMAGE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C0_HIGH_IMAGE_NL, "C0 high bestand..." },
/* pl */ { IDMS_C0_HIGH_IMAGE_PL, "Obraz C0 high..." },
/* ru */ { IDMS_C0_HIGH_IMAGE_RU, "C0 high image..." },
/* sv */ { IDMS_C0_HIGH_IMAGE_SV, "C0 hög-avbildning..." },
/* tr */ { IDMS_C0_HIGH_IMAGE_TR, "C0 high imajý..." },

/* en */ { IDMS_C1_LOW_IMAGE,    "C1 low  image..." },
/* da */ { IDMS_C1_LOW_IMAGE_DA, "C1 lav-image..." },
/* de */ { IDMS_C1_LOW_IMAGE_DE, "C1 low Imagedatei..." },
/* es */ { IDMS_C1_LOW_IMAGE_ES, "C1 imagen bajo..." },
/* fr */ { IDMS_C1_LOW_IMAGE_FR, "Image C1 basse..." },
/* hu */ { IDMS_C1_LOW_IMAGE_HU, "" },  /* fuzzy */
/* it */ { IDMS_C1_LOW_IMAGE_IT, "Immagine C1 bassa..." },
/* ko */ { IDMS_C1_LOW_IMAGE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C1_LOW_IMAGE_NL, "C1 low bestand..." },
/* pl */ { IDMS_C1_LOW_IMAGE_PL, "Obraz C1 low..." },
/* ru */ { IDMS_C1_LOW_IMAGE_RU, "C1 low  image..." },
/* sv */ { IDMS_C1_LOW_IMAGE_SV, "C1 låg-avbildning..." },
/* tr */ { IDMS_C1_LOW_IMAGE_TR, "C1 low  imajý..." },

/* en */ { IDMS_C1_HIGH_IMAGE,    "C1 high image..." },
/* da */ { IDMS_C1_HIGH_IMAGE_DA, "C1 høj-image..." },
/* de */ { IDMS_C1_HIGH_IMAGE_DE, "C1 high Imagedatei..." },
/* es */ { IDMS_C1_HIGH_IMAGE_ES, "C1 imagen alto..." },
/* fr */ { IDMS_C1_HIGH_IMAGE_FR, "Image C1 haute..." },
/* hu */ { IDMS_C1_HIGH_IMAGE_HU, "" },  /* fuzzy */
/* it */ { IDMS_C1_HIGH_IMAGE_IT, "Immagine C1 alta..." },
/* ko */ { IDMS_C1_HIGH_IMAGE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C1_HIGH_IMAGE_NL, "C1 high bestand..." },
/* pl */ { IDMS_C1_HIGH_IMAGE_PL, "Obraz C1 high..." },
/* ru */ { IDMS_C1_HIGH_IMAGE_RU, "C1 high image..." },
/* sv */ { IDMS_C1_HIGH_IMAGE_SV, "C1 hög-avbildning..." },
/* tr */ { IDMS_C1_HIGH_IMAGE_TR, "C1 high imajý..." },

/* en */ { IDMS_C2_LOW_IMAGE,    "C2 low  image..." },
/* da */ { IDMS_C2_LOW_IMAGE_DA, "C2 lav-image..." },
/* de */ { IDMS_C2_LOW_IMAGE_DE, "C2 low  Imagedatei..." },
/* es */ { IDMS_C2_LOW_IMAGE_ES, "C2 imagen bajo..." },
/* fr */ { IDMS_C2_LOW_IMAGE_FR, "Image C2 basse..." },
/* hu */ { IDMS_C2_LOW_IMAGE_HU, "" },  /* fuzzy */
/* it */ { IDMS_C2_LOW_IMAGE_IT, "Immagine C2 bassa..." },
/* ko */ { IDMS_C2_LOW_IMAGE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C2_LOW_IMAGE_NL, "C2 low bestand..." },
/* pl */ { IDMS_C2_LOW_IMAGE_PL, "Obraz C2 low..." },
/* ru */ { IDMS_C2_LOW_IMAGE_RU, "C2 low  image..." },
/* sv */ { IDMS_C2_LOW_IMAGE_SV, "C2 låg-avbildning..." },
/* tr */ { IDMS_C2_LOW_IMAGE_TR, "C2 low  imajý..." },

/* en */ { IDMS_C2_HIGH_IMAGE,    "C2 high image..." },
/* da */ { IDMS_C2_HIGH_IMAGE_DA, "C2 høj-image..." },
/* de */ { IDMS_C2_HIGH_IMAGE_DE, "C2 high Imagedatei..." },
/* es */ { IDMS_C2_HIGH_IMAGE_ES, "C2 imagen alto..." },
/* fr */ { IDMS_C2_HIGH_IMAGE_FR, "Image C2 haute..." },
/* hu */ { IDMS_C2_HIGH_IMAGE_HU, "" },  /* fuzzy */
/* it */ { IDMS_C2_HIGH_IMAGE_IT, "Immagine C2 alta..." },
/* ko */ { IDMS_C2_HIGH_IMAGE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C2_HIGH_IMAGE_NL, "C2 high bestand..." },
/* pl */ { IDMS_C2_HIGH_IMAGE_PL, "Obraz C2 low..." },
/* ru */ { IDMS_C2_HIGH_IMAGE_RU, "C2 high image..." },
/* sv */ { IDMS_C2_HIGH_IMAGE_SV, "C2 hög-avbildning..." },
/* tr */ { IDMS_C2_HIGH_IMAGE_TR, "C2 high imajý..." },

/* en */ { IDMS_FUNCTION_LOW_3PLUS1,    "Function low  image (3plus1)..." },
/* da */ { IDMS_FUNCTION_LOW_3PLUS1_DA, "Funktion lav-image (3plus1)..." },
/* de */ { IDMS_FUNCTION_LOW_3PLUS1_DE, "Funktions ROM Imagedatei low (3plus1)..." },
/* es */ { IDMS_FUNCTION_LOW_3PLUS1_ES, "Función imagen bajo (3plus1)..." },
/* fr */ { IDMS_FUNCTION_LOW_3PLUS1_FR, "Image Fonction basse (3plus1)..." },
/* hu */ { IDMS_FUNCTION_LOW_3PLUS1_HU, "" },  /* fuzzy */
/* it */ { IDMS_FUNCTION_LOW_3PLUS1_IT, "Immagine Function low (3plus1)..." },
/* ko */ { IDMS_FUNCTION_LOW_3PLUS1_KO, "" },  /* fuzzy */
/* nl */ { IDMS_FUNCTION_LOW_3PLUS1_NL, "Function low bestand (3plus1)..." },
/* pl */ { IDMS_FUNCTION_LOW_3PLUS1_PL, "Obraz Function low (3plus1)..." },
/* ru */ { IDMS_FUNCTION_LOW_3PLUS1_RU, "" },  /* fuzzy */
/* sv */ { IDMS_FUNCTION_LOW_3PLUS1_SV, "Funktion låg-avbildning (3plus1)..." },
/* tr */ { IDMS_FUNCTION_LOW_3PLUS1_TR, "Function low  imajý (3plus1)..." },

/* en */ { IDMS_FUNCTION_HIGH_3PLUS1,    "Function high image (3plus1)..." },
/* da */ { IDMS_FUNCTION_HIGH_3PLUS1_DA, "Funktion høj-image (3plus1)..." },
/* de */ { IDMS_FUNCTION_HIGH_3PLUS1_DE, "Funktions ROM Imagedatei high (3plus1)..." },
/* es */ { IDMS_FUNCTION_HIGH_3PLUS1_ES, "Función imagen alto (3plus1)..." },
/* fr */ { IDMS_FUNCTION_HIGH_3PLUS1_FR, "Image Fonction haute (3plus1)..." },
/* hu */ { IDMS_FUNCTION_HIGH_3PLUS1_HU, "" },  /* fuzzy */
/* it */ { IDMS_FUNCTION_HIGH_3PLUS1_IT, "Immagine Function high (3plus1)..." },
/* ko */ { IDMS_FUNCTION_HIGH_3PLUS1_KO, "" },  /* fuzzy */
/* nl */ { IDMS_FUNCTION_HIGH_3PLUS1_NL, "Function high bestand (3plus1)..." },
/* pl */ { IDMS_FUNCTION_HIGH_3PLUS1_PL, "Obraz Function high (3plus1)..." },
/* ru */ { IDMS_FUNCTION_HIGH_3PLUS1_RU, "" },  /* fuzzy */
/* sv */ { IDMS_FUNCTION_HIGH_3PLUS1_SV, "Funktion hög-avbildning (3plus1)..." },
/* tr */ { IDMS_FUNCTION_HIGH_3PLUS1_TR, "Function high imajý (3plus1)..." },

/* en */ { IDMS_LOAD_CART_1XXX,    "Load new Cart $1***" },
/* da */ { IDMS_LOAD_CART_1XXX_DA, "Indlæs nyt cartridge $1***" },
/* de */ { IDMS_LOAD_CART_1XXX_DE, "Modul bei $1*** laden" },
/* es */ { IDMS_LOAD_CART_1XXX_ES, "Cargar nuevo Cart $1***" },
/* fr */ { IDMS_LOAD_CART_1XXX_FR, "Charger la cartouche $1***" },
/* hu */ { IDMS_LOAD_CART_1XXX_HU, "Új Cart. betöltése a $1*** címre" },
/* it */ { IDMS_LOAD_CART_1XXX_IT, "Carica nuova cartuccia $1***" },
/* ko */ { IDMS_LOAD_CART_1XXX_KO, "" },  /* fuzzy */
/* nl */ { IDMS_LOAD_CART_1XXX_NL, "Laad nieuw Cart $1***" },
/* pl */ { IDMS_LOAD_CART_1XXX_PL, "Wczytaj nowy kartrid¿ $1***" },
/* ru */ { IDMS_LOAD_CART_1XXX_RU, "·ÐÓàã×Øâì ÝÞÒëÙ ÚÐàâàØÔÖ $1***" },
/* sv */ { IDMS_LOAD_CART_1XXX_SV, "Läs ny insticksmodul $1***" },
/* tr */ { IDMS_LOAD_CART_1XXX_TR, "Yeni Kartuþ $1*** yükle" },

/* en */ { IDMS_UNLOAD_CART_1XXX,    "Unload Cart $1***" },
/* da */ { IDMS_UNLOAD_CART_1XXX_DA, "Afbryd cartridge $1***" },
/* de */ { IDMS_UNLOAD_CART_1XXX_DE, "Modul bei $1*** entfernen" },
/* es */ { IDMS_UNLOAD_CART_1XXX_ES, "Extraer Cart $1***" },
/* fr */ { IDMS_UNLOAD_CART_1XXX_FR, "Décharger la cassette $1***" },
/* hu */ { IDMS_UNLOAD_CART_1XXX_HU, "Cart. leválasztása a $1*** címrõl" },
/* it */ { IDMS_UNLOAD_CART_1XXX_IT, "Scarica cartuccia $1***" },
/* ko */ { IDMS_UNLOAD_CART_1XXX_KO, "" },  /* fuzzy */
/* nl */ { IDMS_UNLOAD_CART_1XXX_NL, "Verwijder Cart $1***" },
/* pl */ { IDMS_UNLOAD_CART_1XXX_PL, "Usuñ kartrid¿ $1***" },
/* ru */ { IDMS_UNLOAD_CART_1XXX_RU, "¸×ÒÛÕçì ÚÐàâàØÔÖ $1***" },
/* sv */ { IDMS_UNLOAD_CART_1XXX_SV, "Läs ur insticksmodul $1***" },
/* tr */ { IDMS_UNLOAD_CART_1XXX_TR, "Kartuþ $1*** kaldýr" },

/* en */ { IDMS_LOAD_CART_2_3XXX,    "Load new Cart $2-3***" },
/* da */ { IDMS_LOAD_CART_2_3XXX_DA, "Indlæs nyt cartridge $2-3***" },
/* de */ { IDMS_LOAD_CART_2_3XXX_DE, "Modul bei $2-3*** laden" },
/* es */ { IDMS_LOAD_CART_2_3XXX_ES, "Cargar nuevo Cart $2-3***" },
/* fr */ { IDMS_LOAD_CART_2_3XXX_FR, "Charger la cartouche $2-3***" },
/* hu */ { IDMS_LOAD_CART_2_3XXX_HU, "Új Cart. betöltése a $2-$3*** címre" },
/* it */ { IDMS_LOAD_CART_2_3XXX_IT, "Carica nuova cartuccia $2-3***" },
/* ko */ { IDMS_LOAD_CART_2_3XXX_KO, "" },  /* fuzzy */
/* nl */ { IDMS_LOAD_CART_2_3XXX_NL, "Laad nieuw Cart $2-3***" },
/* pl */ { IDMS_LOAD_CART_2_3XXX_PL, "Wczytaj nowy kartrid¿ $2-3***" },
/* ru */ { IDMS_LOAD_CART_2_3XXX_RU, "·ÐÓàã×Øâì ÝÞÒëÙ ÚÐàâàØÔÖ $2-3***" },
/* sv */ { IDMS_LOAD_CART_2_3XXX_SV, "Läs ny insticksmodul $2-3***" },
/* tr */ { IDMS_LOAD_CART_2_3XXX_TR, "Yeni Kartuþ $2-3*** yükle" },

/* en */ { IDMS_UNLOAD_CART_2_3XXX,    "Unload Cart $2-3***" },
/* da */ { IDMS_UNLOAD_CART_2_3XXX_DA, "Afbryd cartridge $2-3***" },
/* de */ { IDMS_UNLOAD_CART_2_3XXX_DE, "Modul bei $2-3*** entfernen" },
/* es */ { IDMS_UNLOAD_CART_2_3XXX_ES, "Extraer Cart $2-3***" },
/* fr */ { IDMS_UNLOAD_CART_2_3XXX_FR, "Décharger la cassette $2-3***" },
/* hu */ { IDMS_UNLOAD_CART_2_3XXX_HU, "Cart. leválasztása a $2-3*** címrõl" },
/* it */ { IDMS_UNLOAD_CART_2_3XXX_IT, "Scarica cartuccia $2-3***" },
/* ko */ { IDMS_UNLOAD_CART_2_3XXX_KO, "" },  /* fuzzy */
/* nl */ { IDMS_UNLOAD_CART_2_3XXX_NL, "Verwijder Cart $2-3***" },
/* pl */ { IDMS_UNLOAD_CART_2_3XXX_PL, "Usuñ kartrid¿ $2-3***" },
/* ru */ { IDMS_UNLOAD_CART_2_3XXX_RU, "¸×ÒÛÕçì ÚÐàâàØÔÖ $2-3***" },
/* sv */ { IDMS_UNLOAD_CART_2_3XXX_SV, "Läs ur insticksmodul $2-3***" },
/* tr */ { IDMS_UNLOAD_CART_2_3XXX_TR, "Kartuþ $2-3*** kaldýr" },

/* en */ { IDMS_LOAD_CART_4_5XXX,    "Load new Cart $4-5***" },
/* da */ { IDMS_LOAD_CART_4_5XXX_DA, "Indlæs nyt cartridge $4-5***" },
/* de */ { IDMS_LOAD_CART_4_5XXX_DE, "Modul bei $4-5*** laden" },
/* es */ { IDMS_LOAD_CART_4_5XXX_ES, "Cargar nuevo Cart $4-5***" },
/* fr */ { IDMS_LOAD_CART_4_5XXX_FR, "Charger la cartouche $4-5***" },
/* hu */ { IDMS_LOAD_CART_4_5XXX_HU, "Új Cart. betöltése a $4-$5*** címre" },
/* it */ { IDMS_LOAD_CART_4_5XXX_IT, "Carica nuova cartuccia $4-5***" },
/* ko */ { IDMS_LOAD_CART_4_5XXX_KO, "" },  /* fuzzy */
/* nl */ { IDMS_LOAD_CART_4_5XXX_NL, "Laad nieuw Cart $4-5***" },
/* pl */ { IDMS_LOAD_CART_4_5XXX_PL, "Wczytaj nowy kartrid¿ $4-5***" },
/* ru */ { IDMS_LOAD_CART_4_5XXX_RU, "·ÐÓàã×Øâì ÝÞÒëÙ ÚÐàâàØÔÖ $4-5***" },
/* sv */ { IDMS_LOAD_CART_4_5XXX_SV, "Läs ny insticksmodul $4-5***" },
/* tr */ { IDMS_LOAD_CART_4_5XXX_TR, "Yeni Kartuþ $4-5*** yükle" },

/* en */ { IDMS_UNLOAD_CART_4_5XXX,    "Unload Cart $4-5***" },
/* da */ { IDMS_UNLOAD_CART_4_5XXX_DA, "Afbryd cartridge $4-5***" },
/* de */ { IDMS_UNLOAD_CART_4_5XXX_DE, "Modul bei $4-5*** entfernen" },
/* es */ { IDMS_UNLOAD_CART_4_5XXX_ES, "Extraer Cart $4-5***" },
/* fr */ { IDMS_UNLOAD_CART_4_5XXX_FR, "Décharger la cassette $4-5***" },
/* hu */ { IDMS_UNLOAD_CART_4_5XXX_HU, "Cart. leválasztása a $4-5*** címrõl" },
/* it */ { IDMS_UNLOAD_CART_4_5XXX_IT, "Scarica cartuccia $4-5***" },
/* ko */ { IDMS_UNLOAD_CART_4_5XXX_KO, "" },  /* fuzzy */
/* nl */ { IDMS_UNLOAD_CART_4_5XXX_NL, "Verwijder Cart $4-5***" },
/* pl */ { IDMS_UNLOAD_CART_4_5XXX_PL, "Usuñ kartrid¿ $4-5***" },
/* ru */ { IDMS_UNLOAD_CART_4_5XXX_RU, "¸×ÒÛÕçì ÚÐàâàØÔÖ $4-5***" },
/* sv */ { IDMS_UNLOAD_CART_4_5XXX_SV, "Läs ur insticksmodul $4-5***" },
/* tr */ { IDMS_UNLOAD_CART_4_5XXX_TR, "Kartuþ $4-5*** kaldýr" },

/* en */ { IDMS_LOAD_CART_6_7XXX,    "Load new Cart $6-7***" },
/* da */ { IDMS_LOAD_CART_6_7XXX_DA, "Indlæs nyt cartridge $6-7***" },
/* de */ { IDMS_LOAD_CART_6_7XXX_DE, "Modul bei $6-7*** laden" },
/* es */ { IDMS_LOAD_CART_6_7XXX_ES, "Cargar nuevo Cart $6-7***" },
/* fr */ { IDMS_LOAD_CART_6_7XXX_FR, "Charger la cartouche $6-7***" },
/* hu */ { IDMS_LOAD_CART_6_7XXX_HU, "Új Cart. betöltése a $6-$7*** címre" },
/* it */ { IDMS_LOAD_CART_6_7XXX_IT, "Carica nuova cartuccia $6-7***" },
/* ko */ { IDMS_LOAD_CART_6_7XXX_KO, "" },  /* fuzzy */
/* nl */ { IDMS_LOAD_CART_6_7XXX_NL, "Laad nieuw Cart $6-7***" },
/* pl */ { IDMS_LOAD_CART_6_7XXX_PL, "Wczytaj nowy kartrid¿ $6-7***" },
/* ru */ { IDMS_LOAD_CART_6_7XXX_RU, "·ÐÓàã×Øâì ÝÞÒëÙ ÚÐàâàØÔÖ $6-7***" },
/* sv */ { IDMS_LOAD_CART_6_7XXX_SV, "Läs ny insticksmodul $6-7***" },
/* tr */ { IDMS_LOAD_CART_6_7XXX_TR, "Yeni Kartuþ $6-7*** yükle" },

/* en */ { IDMS_UNLOAD_CART_6_7XXX,    "Unload Cart $6-7***" },
/* da */ { IDMS_UNLOAD_CART_6_7XXX_DA, "Afbryd cartridge $6-7***" },
/* de */ { IDMS_UNLOAD_CART_6_7XXX_DE, "Modul bei $6-7*** entfernen" },
/* es */ { IDMS_UNLOAD_CART_6_7XXX_ES, "Extraer Cart $6-7***" },
/* fr */ { IDMS_UNLOAD_CART_6_7XXX_FR, "Décharger la cassette $6-7***" },
/* hu */ { IDMS_UNLOAD_CART_6_7XXX_HU, "Cart. leválasztása a $6-7*** címrõl" },
/* it */ { IDMS_UNLOAD_CART_6_7XXX_IT, "Scarica cartuccia $6-7***" },
/* ko */ { IDMS_UNLOAD_CART_6_7XXX_KO, "" },  /* fuzzy */
/* nl */ { IDMS_UNLOAD_CART_6_7XXX_NL, "Verwijder Cart $6-7***" },
/* pl */ { IDMS_UNLOAD_CART_6_7XXX_PL, "Usuñ kartrid¿ $6-7***" },
/* ru */ { IDMS_UNLOAD_CART_6_7XXX_RU, "¸×ÒÛÕçì ÚÐàâàØÔÖ $6-7***" },
/* sv */ { IDMS_UNLOAD_CART_6_7XXX_SV, "Läs ur insticksmodul $6-7***" },
/* tr */ { IDMS_UNLOAD_CART_6_7XXX_TR, "Kartuþ $6-7*** kaldýr" },

/* en */ { IDMS_FUNCTION_HIGH_3PLUS1,    "Function high image (3plus1)..." },
/* da */ { IDMS_FUNCTION_HIGH_3PLUS1_DA, "Funktion høj-image (3plus1)..." },
/* de */ { IDMS_FUNCTION_HIGH_3PLUS1_DE, "Funktions ROM Imagedatei high (3plus1)..." },
/* es */ { IDMS_FUNCTION_HIGH_3PLUS1_ES, "Función imagen alto (3plus1)..." },
/* fr */ { IDMS_FUNCTION_HIGH_3PLUS1_FR, "Image Fonction haute (3plus1)..." },
/* hu */ { IDMS_FUNCTION_HIGH_3PLUS1_HU, "" },  /* fuzzy */
/* it */ { IDMS_FUNCTION_HIGH_3PLUS1_IT, "Immagine Function high (3plus1)..." },
/* ko */ { IDMS_FUNCTION_HIGH_3PLUS1_KO, "" },  /* fuzzy */
/* nl */ { IDMS_FUNCTION_HIGH_3PLUS1_NL, "Function high bestand (3plus1)..." },
/* pl */ { IDMS_FUNCTION_HIGH_3PLUS1_PL, "Obraz Function high (3plus1)..." },
/* ru */ { IDMS_FUNCTION_HIGH_3PLUS1_RU, "" },  /* fuzzy */
/* sv */ { IDMS_FUNCTION_HIGH_3PLUS1_SV, "Funktion hög-avbildning (3plus1)..." },
/* tr */ { IDMS_FUNCTION_HIGH_3PLUS1_TR, "Function high imajý (3plus1)..." },

/* en */ { IDMS_CRT_IMAGE,    "CRT image..." },
/* da */ { IDMS_CRT_IMAGE_DA, "CRT-fil..." },
/* de */ { IDMS_CRT_IMAGE_DE, "CRT Image..." },
/* es */ { IDMS_CRT_IMAGE_ES, "Imagen CRT..." },
/* fr */ { IDMS_CRT_IMAGE_FR, "Image CRT..." },
/* hu */ { IDMS_CRT_IMAGE_HU, "" },  /* fuzzy */
/* it */ { IDMS_CRT_IMAGE_IT, "Immagine CRT..." },
/* ko */ { IDMS_CRT_IMAGE_KO, "CRT ÀÌ¹ÌÁö..." },
/* nl */ { IDMS_CRT_IMAGE_NL, "CRT bestand..." },
/* pl */ { IDMS_CRT_IMAGE_PL, "Obraz CRT..." },
/* ru */ { IDMS_CRT_IMAGE_RU, "CRT image..." },
/* sv */ { IDMS_CRT_IMAGE_SV, "CRT-fil..." },
/* tr */ { IDMS_CRT_IMAGE_TR, "CRT imajý..." },

/* en */ { IDMS_GENERIC_CARTS,    "Generic Cartridges..." },
/* da */ { IDMS_GENERIC_CARTS_DA, "" },  /* fuzzy */
/* de */ { IDMS_GENERIC_CARTS_DE, "Universelle Erweiterungsmodule..." },
/* es */ { IDMS_GENERIC_CARTS_ES, "" },  /* fuzzy */
/* fr */ { IDMS_GENERIC_CARTS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_GENERIC_CARTS_HU, "" },  /* fuzzy */
/* it */ { IDMS_GENERIC_CARTS_IT, "" },  /* fuzzy */
/* ko */ { IDMS_GENERIC_CARTS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_GENERIC_CARTS_NL, "Algemene Cartridges..." },
/* pl */ { IDMS_GENERIC_CARTS_PL, "" },  /* fuzzy */
/* ru */ { IDMS_GENERIC_CARTS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_GENERIC_CARTS_SV, "" },  /* fuzzy */
/* tr */ { IDMS_GENERIC_CARTS_TR, "" },  /* fuzzy */

/* en */ { IDMS_FREEZER_CARTS,    "Freezer Cartridges..." },
/* da */ { IDMS_FREEZER_CARTS_DA, "" },  /* fuzzy */
/* de */ { IDMS_FREEZER_CARTS_DE, "Freezer Erweiterungsmodule..." },
/* es */ { IDMS_FREEZER_CARTS_ES, "" },  /* fuzzy */
/* fr */ { IDMS_FREEZER_CARTS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_FREEZER_CARTS_HU, "" },  /* fuzzy */
/* it */ { IDMS_FREEZER_CARTS_IT, "" },  /* fuzzy */
/* ko */ { IDMS_FREEZER_CARTS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_FREEZER_CARTS_NL, "Freezer Cartridges..." },
/* pl */ { IDMS_FREEZER_CARTS_PL, "" },  /* fuzzy */
/* ru */ { IDMS_FREEZER_CARTS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_FREEZER_CARTS_SV, "" },  /* fuzzy */
/* tr */ { IDMS_FREEZER_CARTS_TR, "" },  /* fuzzy */

/* en */ { IDMS_UTIL_CARTS,    "Utility Cartridges..." },
/* da */ { IDMS_UTIL_CARTS_DA, "" },  /* fuzzy */
/* de */ { IDMS_UTIL_CARTS_DE, "Utility Erweiterungsmodule..." },
/* es */ { IDMS_UTIL_CARTS_ES, "" },  /* fuzzy */
/* fr */ { IDMS_UTIL_CARTS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_UTIL_CARTS_HU, "" },  /* fuzzy */
/* it */ { IDMS_UTIL_CARTS_IT, "" },  /* fuzzy */
/* ko */ { IDMS_UTIL_CARTS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_UTIL_CARTS_NL, "Gebruiksprogramma Cartridges..." },
/* pl */ { IDMS_UTIL_CARTS_PL, "" },  /* fuzzy */
/* ru */ { IDMS_UTIL_CARTS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_UTIL_CARTS_SV, "" },  /* fuzzy */
/* tr */ { IDMS_UTIL_CARTS_TR, "" },  /* fuzzy */

/* en */ { IDMS_GAME_CARTS,    "Game Cartridges..." },
/* da */ { IDMS_GAME_CARTS_DA, "" },  /* fuzzy */
/* de */ { IDMS_GAME_CARTS_DE, "Spiel Erweiterungsmodule..." },
/* es */ { IDMS_GAME_CARTS_ES, "" },  /* fuzzy */
/* fr */ { IDMS_GAME_CARTS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_GAME_CARTS_HU, "" },  /* fuzzy */
/* it */ { IDMS_GAME_CARTS_IT, "" },  /* fuzzy */
/* ko */ { IDMS_GAME_CARTS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_GAME_CARTS_NL, "Spel Cartridges..." },
/* pl */ { IDMS_GAME_CARTS_PL, "" },  /* fuzzy */
/* ru */ { IDMS_GAME_CARTS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_GAME_CARTS_SV, "" },  /* fuzzy */
/* tr */ { IDMS_GAME_CARTS_TR, "" },  /* fuzzy */

/* en */ { IDMS_RAMEX_CARTS,    "RAM Expansion Cartridges..." },
/* da */ { IDMS_RAMEX_CARTS_DA, "" },  /* fuzzy */
/* de */ { IDMS_RAMEX_CARTS_DE, "RAM Erweiterungsmodule..." },
/* es */ { IDMS_RAMEX_CARTS_ES, "" },  /* fuzzy */
/* fr */ { IDMS_RAMEX_CARTS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_RAMEX_CARTS_HU, "" },  /* fuzzy */
/* it */ { IDMS_RAMEX_CARTS_IT, "" },  /* fuzzy */
/* ko */ { IDMS_RAMEX_CARTS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_RAMEX_CARTS_NL, "RAM Uitbreiding Cartridges..." },
/* pl */ { IDMS_RAMEX_CARTS_PL, "" },  /* fuzzy */
/* ru */ { IDMS_RAMEX_CARTS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_RAMEX_CARTS_SV, "" },  /* fuzzy */
/* tr */ { IDMS_RAMEX_CARTS_TR, "" },  /* fuzzy */

/* en */ { IDMS_ENABLE,    "Enable" },
/* da */ { IDMS_ENABLE_DA, "Aktivér" },
/* de */ { IDMS_ENABLE_DE, "Aktivieren" },
/* es */ { IDMS_ENABLE_ES, "Permitir" },
/* fr */ { IDMS_ENABLE_FR, "Activer" },
/* hu */ { IDMS_ENABLE_HU, "Engedélyezés" },
/* it */ { IDMS_ENABLE_IT, "Attiva" },
/* ko */ { IDMS_ENABLE_KO, "ÀÛµ¿½ÃÅ°´Ù" },
/* nl */ { IDMS_ENABLE_NL, "Activeer" },
/* pl */ { IDMS_ENABLE_PL, "W³±cz" },
/* ru */ { IDMS_ENABLE_RU, "²ÚÛîçØâì" },
/* sv */ { IDMS_ENABLE_SV, "Aktivera" },
/* tr */ { IDMS_ENABLE_TR, "Aktif Et" },

/* en */ { IDMS_SET_CART_AS_DEFAULT,    "Set cartridge as default" },
/* da */ { IDMS_SET_CART_AS_DEFAULT_DA, "Vælg cartridge som standard" },
/* de */ { IDMS_SET_CART_AS_DEFAULT_DE, "Aktuelles Erweiterungsmodul als Standard aktivieren" },
/* es */ { IDMS_SET_CART_AS_DEFAULT_ES, "Usar cartucho por defecto" },
/* fr */ { IDMS_SET_CART_AS_DEFAULT_FR, "Définir cette cartouche par défaut" },
/* hu */ { IDMS_SET_CART_AS_DEFAULT_HU, "Cartridge alapértelmezetté tétele" },
/* it */ { IDMS_SET_CART_AS_DEFAULT_IT, "Imposta la cartuccia come predefinita" },
/* ko */ { IDMS_SET_CART_AS_DEFAULT_KO, "Ä«Æ®¸®Áö¸¦ µðÆúÆ® ¼Â ÇÏ±â" },
/* nl */ { IDMS_SET_CART_AS_DEFAULT_NL, "Stel cartridge in als standaard" },
/* pl */ { IDMS_SET_CART_AS_DEFAULT_PL, "Ustaw kartrid¿ jako domy¶lny" },
/* ru */ { IDMS_SET_CART_AS_DEFAULT_RU, "ÃáâÐÝÞÒØâì ÚÐàâàØÔÖ ßÞ ãÜÞÛçÐÝØî" },
/* sv */ { IDMS_SET_CART_AS_DEFAULT_SV, "Välj insticksmodul som förval" },
/* tr */ { IDMS_SET_CART_AS_DEFAULT_TR, "Kartuþu varsayýlan yap" },

/* en */ { IDMS_RESET_ON_CART_CHANGE,    "Reset on cart change" },
/* da */ { IDMS_RESET_ON_CART_CHANGE_DA, "Reset ved cartridge-skift" },
/* de */ { IDMS_RESET_ON_CART_CHANGE_DE, "Reset bei Wechsel von Erweiterungsmodulen" },
/* es */ { IDMS_RESET_ON_CART_CHANGE_ES, "Reiniciar al cambiar cartucho" },
/* fr */ { IDMS_RESET_ON_CART_CHANGE_FR, "Réinialiser sur changement de cartouche" },
/* hu */ { IDMS_RESET_ON_CART_CHANGE_HU, "Újraindítás cartridge cserénél" },
/* it */ { IDMS_RESET_ON_CART_CHANGE_IT, "Reset al cambio di cartuccia" },
/* ko */ { IDMS_RESET_ON_CART_CHANGE_KO, "Ä«Æ®¸®Áö º¯°æ ¸®¼Â" },
/* nl */ { IDMS_RESET_ON_CART_CHANGE_NL, "Reset bij cart-wisseling" },
/* pl */ { IDMS_RESET_ON_CART_CHANGE_PL, "Restart przy zmianie kartrid¿a" },
/* ru */ { IDMS_RESET_ON_CART_CHANGE_RU, "Reset on cart change" },
/* sv */ { IDMS_RESET_ON_CART_CHANGE_SV, "Nollställ vid modulbyte" },
/* tr */ { IDMS_RESET_ON_CART_CHANGE_TR, "Kartuþ deðiþiminde resetle" },

/* en */ { IDMS_CART_FREEZE,    "Cartridge freeze" },
/* da */ { IDMS_CART_FREEZE_DA, "Cartridgets frys-funktion" },
/* de */ { IDMS_CART_FREEZE_DE, "Erweiterungsmodul Freeze" },
/* es */ { IDMS_CART_FREEZE_ES, "Congelar cartucho" },
/* fr */ { IDMS_CART_FREEZE_FR, "Geler la cartouche" },
/* hu */ { IDMS_CART_FREEZE_HU, "Fagyasztás Cartridge-dzsel" },
/* it */ { IDMS_CART_FREEZE_IT, "Freeze cartuccia" },
/* ko */ { IDMS_CART_FREEZE_KO, "Ä«Æ®¸®Áö ÇÁ¸®Áî" },
/* nl */ { IDMS_CART_FREEZE_NL, "Cartridge freeze" },
/* pl */ { IDMS_CART_FREEZE_PL, "Zamro¿enie kartrid¿a" },
/* ru */ { IDMS_CART_FREEZE_RU, "Cartridge freeze" },
/* sv */ { IDMS_CART_FREEZE_SV, "Insticksmodulens frysfunktion" },
/* tr */ { IDMS_CART_FREEZE_TR, "Kartuþ freeze" },

/* en */ { IDMS_PAUSE,    "Pause" },
/* da */ { IDMS_PAUSE_DA, "Pause" },
/* de */ { IDMS_PAUSE_DE, "Pause" },
/* es */ { IDMS_PAUSE_ES, "Pausa" },
/* fr */ { IDMS_PAUSE_FR, "Pause" },
/* hu */ { IDMS_PAUSE_HU, "Szünet" },
/* it */ { IDMS_PAUSE_IT, "Pausa" },
/* ko */ { IDMS_PAUSE_KO, "ÀÏ½ÃÁ¤Áö" },
/* nl */ { IDMS_PAUSE_NL, "Pauze" },
/* pl */ { IDMS_PAUSE_PL, "Pauza" },
/* ru */ { IDMS_PAUSE_RU, "¿Ðã×Ð" },
/* sv */ { IDMS_PAUSE_SV, "Paus" },
/* tr */ { IDMS_PAUSE_TR, "Duraklat" },

/* en */ { IDMS_SINGLE_FRAME_ADVANCE,    "Single frame advance" },
/* da */ { IDMS_SINGLE_FRAME_ADVANCE_DA, "" },  /* fuzzy */
/* de */ { IDMS_SINGLE_FRAME_ADVANCE_DE, "Ein Frame Fortschritt" },
/* es */ { IDMS_SINGLE_FRAME_ADVANCE_ES, "" },  /* fuzzy */
/* fr */ { IDMS_SINGLE_FRAME_ADVANCE_FR, "" },  /* fuzzy */
/* hu */ { IDMS_SINGLE_FRAME_ADVANCE_HU, "" },  /* fuzzy */
/* it */ { IDMS_SINGLE_FRAME_ADVANCE_IT, "" },  /* fuzzy */
/* ko */ { IDMS_SINGLE_FRAME_ADVANCE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_SINGLE_FRAME_ADVANCE_NL, "Een frame vooruit" },
/* pl */ { IDMS_SINGLE_FRAME_ADVANCE_PL, "" },  /* fuzzy */
/* ru */ { IDMS_SINGLE_FRAME_ADVANCE_RU, "" },  /* fuzzy */
/* sv */ { IDMS_SINGLE_FRAME_ADVANCE_SV, "" },  /* fuzzy */
/* tr */ { IDMS_SINGLE_FRAME_ADVANCE_TR, "" },  /* fuzzy */

/* en */ { IDMS_IO_COLLISION_SETTINGS,    "I/O collision settings" },
/* da */ { IDMS_IO_COLLISION_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDMS_IO_COLLISION_SETTINGS_DE, "I/O Kollisionsbehandlung" },
/* es */ { IDMS_IO_COLLISION_SETTINGS_ES, "" },  /* fuzzy */
/* fr */ { IDMS_IO_COLLISION_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_IO_COLLISION_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_IO_COLLISION_SETTINGS_IT, "" },  /* fuzzy */
/* ko */ { IDMS_IO_COLLISION_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_IO_COLLISION_SETTINGS_NL, "I/O conflict instellingen" },
/* pl */ { IDMS_IO_COLLISION_SETTINGS_PL, "" },  /* fuzzy */
/* ru */ { IDMS_IO_COLLISION_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_IO_COLLISION_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDMS_IO_COLLISION_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDMS_MONITOR,    "Monitor" },
/* da */ { IDMS_MONITOR_DA, "Monitor" },
/* de */ { IDMS_MONITOR_DE, "Monitor" },
/* es */ { IDMS_MONITOR_ES, "Monitor" },
/* fr */ { IDMS_MONITOR_FR, "Moniteur" },
/* hu */ { IDMS_MONITOR_HU, "Monitor" },
/* it */ { IDMS_MONITOR_IT, "Monitor" },
/* ko */ { IDMS_MONITOR_KO, "¸ð´ÏÅÍ" },
/* nl */ { IDMS_MONITOR_NL, "Monitor" },
/* pl */ { IDMS_MONITOR_PL, "Monitor" },
/* ru */ { IDMS_MONITOR_RU, "¼ÞÝØâÞà" },
/* sv */ { IDMS_MONITOR_SV, "Monitor" },
/* tr */ { IDMS_MONITOR_TR, "Monitör" },

/* en */ { IDMS_HARD,    "Hard" },
/* da */ { IDMS_HARD_DA, "Hård" },
/* de */ { IDMS_HARD_DE, "Hart" },
/* es */ { IDMS_HARD_ES, "Hard" },
/* fr */ { IDMS_HARD_FR, "à froid" },
/* hu */ { IDMS_HARD_HU, "Hideg" },
/* it */ { IDMS_HARD_IT, "Hard" },
/* ko */ { IDMS_HARD_KO, "´Ü´ÜÇÑ" },
/* nl */ { IDMS_HARD_NL, "Hard" },
/* pl */ { IDMS_HARD_PL, "Twardy" },
/* ru */ { IDMS_HARD_RU, "Hard" },
/* sv */ { IDMS_HARD_SV, "Hård" },
/* tr */ { IDMS_HARD_TR, "Hard" },

/* en */ { IDMS_SOFT,    "Soft" },
/* da */ { IDMS_SOFT_DA, "Blød" },
/* de */ { IDMS_SOFT_DE, "Weich" },
/* es */ { IDMS_SOFT_ES, "Soft" },
/* fr */ { IDMS_SOFT_FR, "à chaud" },
/* hu */ { IDMS_SOFT_HU, "Meleg" },
/* it */ { IDMS_SOFT_IT, "Soft" },
/* ko */ { IDMS_SOFT_KO, "ºÎµå·¯¿î" },
/* nl */ { IDMS_SOFT_NL, "Zacht" },
/* pl */ { IDMS_SOFT_PL, "Miêkki" },
/* ru */ { IDMS_SOFT_RU, "Soft" },
/* sv */ { IDMS_SOFT_SV, "Mjuk" },
/* tr */ { IDMS_SOFT_TR, "Soft" },

/* en */ { IDMS_EXIT,    "Exit" },
/* da */ { IDMS_EXIT_DA, "Afslut" },
/* de */ { IDMS_EXIT_DE, "Exit" },
/* es */ { IDMS_EXIT_ES, "Salir" },
/* fr */ { IDMS_EXIT_FR, "Sortie" },
/* hu */ { IDMS_EXIT_HU, "Kilépés" },
/* it */ { IDMS_EXIT_IT, "Esci" },
/* ko */ { IDMS_EXIT_KO, "³ª°¡±â" },
/* nl */ { IDMS_EXIT_NL, "Afsluiten" },
/* pl */ { IDMS_EXIT_PL, "Wyj¶cie" },
/* ru */ { IDMS_EXIT_RU, "²ëåÞÔ" },
/* sv */ { IDMS_EXIT_SV, "Avsluta" },
/* tr */ { IDMS_EXIT_TR, "Çýkýþ" },

/* en */ { IDMS_EDIT,    "Edit" },
/* da */ { IDMS_EDIT_DA, "Redigér" },
/* de */ { IDMS_EDIT_DE, "Bearbeiten" },
/* es */ { IDMS_EDIT_ES, "Editar" },
/* fr */ { IDMS_EDIT_FR, "Édition" },
/* hu */ { IDMS_EDIT_HU, "Szerkeszt" },
/* it */ { IDMS_EDIT_IT, "Modifica" },
/* ko */ { IDMS_EDIT_KO, "ÆíÁý" },
/* nl */ { IDMS_EDIT_NL, "Bewerken" },
/* pl */ { IDMS_EDIT_PL, "Edycja" },
/* ru */ { IDMS_EDIT_RU, "¿àÐÒÚÐ" },
/* sv */ { IDMS_EDIT_SV, "Redigera" },
/* tr */ { IDMS_EDIT_TR, "Düzenle" },

/* en */ { IDMS_COPY,    "Copy" },
/* da */ { IDMS_COPY_DA, "Kopiér" },
/* de */ { IDMS_COPY_DE, "Kopie" },
/* es */ { IDMS_COPY_ES, "Copiar" },
/* fr */ { IDMS_COPY_FR, "Copier" },
/* hu */ { IDMS_COPY_HU, "Másol" },
/* it */ { IDMS_COPY_IT, "Copia" },
/* ko */ { IDMS_COPY_KO, "º¹»ç" },
/* nl */ { IDMS_COPY_NL, "Kopie" },
/* pl */ { IDMS_COPY_PL, "Kopiuj" },
/* ru */ { IDMS_COPY_RU, "ºÞßØàÞÒÐâì" },
/* sv */ { IDMS_COPY_SV, "Kopiera" },
/* tr */ { IDMS_COPY_TR, "Kopyala" },

/* en */ { IDMS_PASTE,    "Paste" },
/* da */ { IDMS_PASTE_DA, "Indsæt" },
/* de */ { IDMS_PASTE_DE, "Einfügen" },
/* es */ { IDMS_PASTE_ES, "Pegar" },
/* fr */ { IDMS_PASTE_FR, "Coller" },
/* hu */ { IDMS_PASTE_HU, "Beilleszt" },
/* it */ { IDMS_PASTE_IT, "Incolla" },
/* ko */ { IDMS_PASTE_KO, "ºÙ¿©³Ö±â" },
/* nl */ { IDMS_PASTE_NL, "Plakken" },
/* pl */ { IDMS_PASTE_PL, "Wklej" },
/* ru */ { IDMS_PASTE_RU, "²áâÐÒØâì" },
/* sv */ { IDMS_PASTE_SV, "Klistra in" },
/* tr */ { IDMS_PASTE_TR, "Yapýþtýr" },

/* en */ { IDMS_SNAPSHOT,    "Snapshot" },
/* da */ { IDMS_SNAPSHOT_DA, "Snapshot" },
/* de */ { IDMS_SNAPSHOT_DE, "Snapshot" },
/* es */ { IDMS_SNAPSHOT_ES, "Instantánea" },
/* fr */ { IDMS_SNAPSHOT_FR, "Instantanés" },
/* hu */ { IDMS_SNAPSHOT_HU, "Pillanatkép" },
/* it */ { IDMS_SNAPSHOT_IT, "Snapshot" },
/* ko */ { IDMS_SNAPSHOT_KO, "½º³À¼¦" },
/* nl */ { IDMS_SNAPSHOT_NL, "Momentopname" },
/* pl */ { IDMS_SNAPSHOT_PL, "Zrzut" },
/* ru */ { IDMS_SNAPSHOT_RU, "ÁÝíßèÞâ" },
/* sv */ { IDMS_SNAPSHOT_SV, "Ögonblicksbild" },
/* tr */ { IDMS_SNAPSHOT_TR, "Anlýk Görüntü" },

/* en */ { IDMS_LOAD_SNAPSHOT_IMAGE,    "Load snapshot image..." },
/* da */ { IDMS_LOAD_SNAPSHOT_IMAGE_DA, "Indlæs snapshot..." },
/* de */ { IDMS_LOAD_SNAPSHOT_IMAGE_DE, "Snapshot laden..." },
/* es */ { IDMS_LOAD_SNAPSHOT_IMAGE_ES, "Cargar Instantánea..." },
/* fr */ { IDMS_LOAD_SNAPSHOT_IMAGE_FR, "Charger le fichier de sauvegarde..." },
/* hu */ { IDMS_LOAD_SNAPSHOT_IMAGE_HU, "Pillanatkép betöltése..." },
/* it */ { IDMS_LOAD_SNAPSHOT_IMAGE_IT, "Carica immagine snapshot..." },
/* ko */ { IDMS_LOAD_SNAPSHOT_IMAGE_KO, "½º³À¼¦ ÀÌ¹ÌÁö ºÒ·¯¿À±â..." },
/* nl */ { IDMS_LOAD_SNAPSHOT_IMAGE_NL, "Laad momentopnamebestand..." },
/* pl */ { IDMS_LOAD_SNAPSHOT_IMAGE_PL, "Wczytaj obraz zrzutu..." },
/* ru */ { IDMS_LOAD_SNAPSHOT_IMAGE_RU, "·ÐÓàã×Øâì áÝÕßèÞâ..." },
/* sv */ { IDMS_LOAD_SNAPSHOT_IMAGE_SV, "Läser in ögonblicksbild..." },
/* tr */ { IDMS_LOAD_SNAPSHOT_IMAGE_TR, "Anlýk görüntü imajýný yükle..." },

/* en */ { IDMS_SAVE_SNAPSHOT_IMAGE,    "Save snapshot image..." },
/* da */ { IDMS_SAVE_SNAPSHOT_IMAGE_DA, "Gem snapshot..." },
/* de */ { IDMS_SAVE_SNAPSHOT_IMAGE_DE, "Snapshot Datei speichern..." },
/* es */ { IDMS_SAVE_SNAPSHOT_IMAGE_ES, "Grabar Instantánea..." },
/* fr */ { IDMS_SAVE_SNAPSHOT_IMAGE_FR, "Enregistrer le fichier de sauvegarde..." },
/* hu */ { IDMS_SAVE_SNAPSHOT_IMAGE_HU, "Pillanatkép fájl mentése..." },
/* it */ { IDMS_SAVE_SNAPSHOT_IMAGE_IT, "Salva immagine snapshot..." },
/* ko */ { IDMS_SAVE_SNAPSHOT_IMAGE_KO, "½º³À¼¦ ÀÌ¹ÌÁö ÀúÀå..." },
/* nl */ { IDMS_SAVE_SNAPSHOT_IMAGE_NL, "Momentopnamebestand opslaan..." },
/* pl */ { IDMS_SAVE_SNAPSHOT_IMAGE_PL, "Zapisz obraz zrzutu..." },
/* ru */ { IDMS_SAVE_SNAPSHOT_IMAGE_RU, "ÁÞåàÐÝØâì áÝÕßèÞâ..." },
/* sv */ { IDMS_SAVE_SNAPSHOT_IMAGE_SV, "Spara ögonblicksbildfil..." },
/* tr */ { IDMS_SAVE_SNAPSHOT_IMAGE_TR, "Anlýk görüntü imajýný kaydet..." },

/* en */ { IDMS_START_STOP_RECORDING,    "Start/Stop Recording History" },
/* da */ { IDMS_START_STOP_RECORDING_DA, "Start/stop optagelse af historik" },
/* de */ { IDMS_START_STOP_RECORDING_DE, "Start/Stopp Aufnahme History" },
/* es */ { IDMS_START_STOP_RECORDING_ES, "Iniciar/finalizar grabación historial" },
/* fr */ { IDMS_START_STOP_RECORDING_FR, "Démarrer/Arrêter l'historique de l'enregistrement" },
/* hu */ { IDMS_START_STOP_RECORDING_HU, "Esemény felvétel indítása/megállítása" },
/* it */ { IDMS_START_STOP_RECORDING_IT, "Avvia/termina registrazione cronologia" },
/* ko */ { IDMS_START_STOP_RECORDING_KO, "½ÃÀÛ/Á¤Áö ·¹ÄÚµù È÷½ºÅä¸®" },
/* nl */ { IDMS_START_STOP_RECORDING_NL, "Start/Stop Opnamegeschiedenis" },
/* pl */ { IDMS_START_STOP_RECORDING_PL, "Odtwarzaj/Zatrzymaj historiê zapisu" },
/* ru */ { IDMS_START_STOP_RECORDING_RU, "Start/Stop Recording History" },
/* sv */ { IDMS_START_STOP_RECORDING_SV, "Börja/sluta spela in händelser" },
/* tr */ { IDMS_START_STOP_RECORDING_TR, "Kayýt Tarihçesini Baþlat/Durdur" },

/* en */ { IDMS_START_STOP_PLAYBACK,    "Start/Stop Playback History" },
/* da */ { IDMS_START_STOP_PLAYBACK_DA, "Start/stop afspilning af historik" },
/* de */ { IDMS_START_STOP_PLAYBACK_DE, "Start/Stopp Wiedergabe History" },
/* es */ { IDMS_START_STOP_PLAYBACK_ES, "Iniciar/finalizar reproducción historial" },
/* fr */ { IDMS_START_STOP_PLAYBACK_FR, "Démarrer/Arrêter l'historique de la lecture" },
/* hu */ { IDMS_START_STOP_PLAYBACK_HU, "Esemény visszajátszás indítása/megállítása" },
/* it */ { IDMS_START_STOP_PLAYBACK_IT, "Avvia/termina riproduzione cronologia" },
/* ko */ { IDMS_START_STOP_PLAYBACK_KO, "½ÃÀÛ/Á¤Áö Àç»ý È÷½ºÅä¸®" },
/* nl */ { IDMS_START_STOP_PLAYBACK_NL, "Start/Stop Afspeelgeschiedenis" },
/* pl */ { IDMS_START_STOP_PLAYBACK_PL, "Odtwarzaj/Zatrzymaj historiê odtwarzania" },
/* ru */ { IDMS_START_STOP_PLAYBACK_RU, "Start/Stop Playback History" },
/* sv */ { IDMS_START_STOP_PLAYBACK_SV, "Börja/sluta spela upp händelser" },
/* tr */ { IDMS_START_STOP_PLAYBACK_TR, "Kayýttan Yürütme Tarihçesini Baþlat/Durdur" },

/* en */ { IDMS_SET_MILESTONE,    "Set recording milestone" },
/* da */ { IDMS_SET_MILESTONE_DA, "Angiv optagelses-bogmærke" },
/* de */ { IDMS_SET_MILESTONE_DE, "Setze Aufnahme Meilenstein" },
/* es */ { IDMS_SET_MILESTONE_ES, "Insertar marca de grabación" },
/* fr */ { IDMS_SET_MILESTONE_FR, "Lever le signet d'enregistrement" },
/* hu */ { IDMS_SET_MILESTONE_HU, "Kilóméterkõ elhelyezése a felvételben" },
/* it */ { IDMS_SET_MILESTONE_IT, "Imposta segnalibro" },
/* ko */ { IDMS_SET_MILESTONE_KO, "·¹ÄÚµù ¸¶ÀÏ½ºÅæ ¼³Á¤" },
/* nl */ { IDMS_SET_MILESTONE_NL, "Zet opname mijlpaal" },
/* pl */ { IDMS_SET_MILESTONE_PL, "Ustaw kamieñ milowy zapisu" },
/* ru */ { IDMS_SET_MILESTONE_RU, "ÃáâÐÝÞÒØâì âÞçÚã ×ÐßØáØ" },
/* sv */ { IDMS_SET_MILESTONE_SV, "Ange inspelningsbokmärke" },
/* tr */ { IDMS_SET_MILESTONE_TR, "Kayýt kilometre taþý koy" },

/* en */ { IDMS_RETURN_TO_MILESTONE,    "Return to milestone" },
/* da */ { IDMS_RETURN_TO_MILESTONE_DA, "Gå til bogmærke" },
/* de */ { IDMS_RETURN_TO_MILESTONE_DE, "Rückkehr zum Meilenstein" },
/* es */ { IDMS_RETURN_TO_MILESTONE_ES, "Volver a la marca" },
/* fr */ { IDMS_RETURN_TO_MILESTONE_FR, "Retourner au signet d'enregistrement" },
/* hu */ { IDMS_RETURN_TO_MILESTONE_HU, "Visszaugrás az elõzõ kilóméterkõhöz" },
/* it */ { IDMS_RETURN_TO_MILESTONE_IT, "Vai al segnalibro" },
/* ko */ { IDMS_RETURN_TO_MILESTONE_KO, "¸¶ÀÏ½ºÅæ À¸·Î µ¹¾Æ°¡±â" },
/* nl */ { IDMS_RETURN_TO_MILESTONE_NL, "Ga terug naar mijlpaal" },
/* pl */ { IDMS_RETURN_TO_MILESTONE_PL, "Powróæ do kamienia milowego" },
/* ru */ { IDMS_RETURN_TO_MILESTONE_RU, "Return to milestone" },
/* sv */ { IDMS_RETURN_TO_MILESTONE_SV, "Återgå till bokmärke" },
/* tr */ { IDMS_RETURN_TO_MILESTONE_TR, "Kilometre taþýna geri dön" },

/* en */ { IDMS_RECORDING_START_MODE,    "Recording start mode" },
/* da */ { IDMS_RECORDING_START_MODE_DA, "Start-tilstand for optagelse" },
/* de */ { IDMS_RECORDING_START_MODE_DE, "Aufnahme Start Modus" },
/* es */ { IDMS_RECORDING_START_MODE_ES, "Grabar modo de inicio" },
/* fr */ { IDMS_RECORDING_START_MODE_FR, "Mode de départ d'enregistrement" },
/* hu */ { IDMS_RECORDING_START_MODE_HU, "Rögzítés indításának módja" },
/* it */ { IDMS_RECORDING_START_MODE_IT, "Modalità di avvio registrazione" },
/* ko */ { IDMS_RECORDING_START_MODE_KO, "·¹ÄÚµù ½ÃÀÛ ¸ðµå" },
/* nl */ { IDMS_RECORDING_START_MODE_NL, "Opname start modus" },
/* pl */ { IDMS_RECORDING_START_MODE_PL, "Rozpocznij tryb zapisu" },
/* ru */ { IDMS_RECORDING_START_MODE_RU, "Recording start mode" },
/* sv */ { IDMS_RECORDING_START_MODE_SV, "Spelar in startläge" },
/* tr */ { IDMS_RECORDING_START_MODE_TR, "Kayýt baþlangýç modu" },

/* en */ { IDMS_SAVE_NEW_SNAPSHOT,    "Save new snapshot" },
/* da */ { IDMS_SAVE_NEW_SNAPSHOT_DA, "Gem nyt snapshot" },
/* de */ { IDMS_SAVE_NEW_SNAPSHOT_DE, "Speichere neuen Snapshot" },
/* es */ { IDMS_SAVE_NEW_SNAPSHOT_ES, "Grabar nueva Instantánea" },
/* fr */ { IDMS_SAVE_NEW_SNAPSHOT_FR, "Enregistrer une nouvelle sauvegarde" },
/* hu */ { IDMS_SAVE_NEW_SNAPSHOT_HU, "Új pillanatkép mentése" },
/* it */ { IDMS_SAVE_NEW_SNAPSHOT_IT, "Salva nuovo snapshot" },
/* ko */ { IDMS_SAVE_NEW_SNAPSHOT_KO, "¼¼·Î¿î ½º³À¼¦ ÀúÀå" },
/* nl */ { IDMS_SAVE_NEW_SNAPSHOT_NL, "Opslaan nieuwe momentopname" },
/* pl */ { IDMS_SAVE_NEW_SNAPSHOT_PL, "Zapisz nowy zrzut" },
/* ru */ { IDMS_SAVE_NEW_SNAPSHOT_RU, "ÁÞåàÐÝØâì ÝÞÒëÙ áÝÕßèÞâ" },
/* sv */ { IDMS_SAVE_NEW_SNAPSHOT_SV, "Spara ny ögonblicksbild" },
/* tr */ { IDMS_SAVE_NEW_SNAPSHOT_TR, "Yeni anlýk görüntü kaydet" },

/* en */ { IDMS_LOAD_EXISTING_SNAPSHOT,    "Load existing snapshot" },
/* da */ { IDMS_LOAD_EXISTING_SNAPSHOT_DA, "Indlæs eksisterende snapshot" },
/* de */ { IDMS_LOAD_EXISTING_SNAPSHOT_DE, "Lade existierenden Snapshot" },
/* es */ { IDMS_LOAD_EXISTING_SNAPSHOT_ES, "Cargar Instantánea existente" },
/* fr */ { IDMS_LOAD_EXISTING_SNAPSHOT_FR, "Charger un instantané" },
/* hu */ { IDMS_LOAD_EXISTING_SNAPSHOT_HU, "Létezõ pillanatkép betöltése" },
/* it */ { IDMS_LOAD_EXISTING_SNAPSHOT_IT, "Carica snapshot esistente" },
/* ko */ { IDMS_LOAD_EXISTING_SNAPSHOT_KO, "ÇöÁ¸ÇÏ´Â ½º³À¼¦ ºÒ·¯¿À±â" },
/* nl */ { IDMS_LOAD_EXISTING_SNAPSHOT_NL, "Laad bestaande momentopname" },
/* pl */ { IDMS_LOAD_EXISTING_SNAPSHOT_PL, "Wczytaj istniej±cy zrzut" },
/* ru */ { IDMS_LOAD_EXISTING_SNAPSHOT_RU, "·ÐÓàã×Øâì áãéÕáâÒãîéØÙ áÝÕßèÞâ" },
/* sv */ { IDMS_LOAD_EXISTING_SNAPSHOT_SV, "Läs in existerande ögonblicksbild" },
/* tr */ { IDMS_LOAD_EXISTING_SNAPSHOT_TR, "Varolan anlýk görüntüyü yükle" },

/* en */ { IDMS_START_WITH_RESET,    "Start with reset" },
/* da */ { IDMS_START_WITH_RESET_DA, "Start med reset" },
/* de */ { IDMS_START_WITH_RESET_DE, "Mit Reset starten" },
/* es */ { IDMS_START_WITH_RESET_ES, "Comenzar con reinicio" },
/* fr */ { IDMS_START_WITH_RESET_FR, "Réinitialiser au démarrage" },
/* hu */ { IDMS_START_WITH_RESET_HU, "Indulás RESET-tel" },
/* it */ { IDMS_START_WITH_RESET_IT, "Avvia con reset" },
/* ko */ { IDMS_START_WITH_RESET_KO, "¸®¼ÂÀ¸·Î »çÀÛÇÏ±â" },
/* nl */ { IDMS_START_WITH_RESET_NL, "Start met reset" },
/* pl */ { IDMS_START_WITH_RESET_PL, "Rozpocznij z resetem" },
/* ru */ { IDMS_START_WITH_RESET_RU, "Start with reset" },
/* sv */ { IDMS_START_WITH_RESET_SV, "Starta med nollställning" },
/* tr */ { IDMS_START_WITH_RESET_TR, "Reset ile baþlat" },

/* en */ { IDMS_OVERWRITE_PLAYBACK,    "Overwrite Playback" },
/* da */ { IDMS_OVERWRITE_PLAYBACK_DA, "Overskriv afspilning" },
/* de */ { IDMS_OVERWRITE_PLAYBACK_DE, "Laufende Aufnahme überschreiben" },
/* es */ { IDMS_OVERWRITE_PLAYBACK_ES, "Sobrescribir reproducción" },
/* fr */ { IDMS_OVERWRITE_PLAYBACK_FR, "Écraser la lecture actuelle" },
/* hu */ { IDMS_OVERWRITE_PLAYBACK_HU, "Visszajátszás felülírása" },
/* it */ { IDMS_OVERWRITE_PLAYBACK_IT, "Sovrascrivi riproduzione" },
/* ko */ { IDMS_OVERWRITE_PLAYBACK_KO, "µ¤¾î¾²±â Àç»ý" },
/* nl */ { IDMS_OVERWRITE_PLAYBACK_NL, "Overschrijf afspelen" },
/* pl */ { IDMS_OVERWRITE_PLAYBACK_PL, "Nadpisuj wyj¶ciowy sygna³" },
/* ru */ { IDMS_OVERWRITE_PLAYBACK_RU, "Overwrite Playback" },
/* sv */ { IDMS_OVERWRITE_PLAYBACK_SV, "Skriv över aktiv uppspelning" },
/* tr */ { IDMS_OVERWRITE_PLAYBACK_TR, "Kayýttan Yürütmenin Üzerine Yaz" },

/* en */ { IDMS_SELECT_HISTORY_DIR,    "Select History files/directory" },
/* da */ { IDMS_SELECT_HISTORY_DIR_DA, "Vælg historikfiler/-katalog" },
/* de */ { IDMS_SELECT_HISTORY_DIR_DE, "Verzeichnis für Event Aufnahme" },
/* es */ { IDMS_SELECT_HISTORY_DIR_ES, "Seleccionar historial ficheros/directorios" },
/* fr */ { IDMS_SELECT_HISTORY_DIR_FR, "Sélectionnez le répertoire des captures" },
/* hu */ { IDMS_SELECT_HISTORY_DIR_HU, "Válassza ki a felvételeket/könyvtárat" },
/* it */ { IDMS_SELECT_HISTORY_DIR_IT, "Seleziona file/directory della cronologia" },
/* ko */ { IDMS_SELECT_HISTORY_DIR_KO, "¼±ÅÃ È÷½ºÅä¸® ÆÄÀÏ/µð·ºÅä¸®" },
/* nl */ { IDMS_SELECT_HISTORY_DIR_NL, "Selecteer geschiedenisbestanden/-directory" },
/* pl */ { IDMS_SELECT_HISTORY_DIR_PL, "Wybierz pliki lub katalog historii" },
/* ru */ { IDMS_SELECT_HISTORY_DIR_RU, "Select History files/directory" },
/* sv */ { IDMS_SELECT_HISTORY_DIR_SV, "Välj historikfiler/-katalog" },
/* tr */ { IDMS_SELECT_HISTORY_DIR_TR, "Tarihçe dosyalarý/dizin seç" },

/* en */ { IDMS_SAVE_STOP_MEDIA_FILE,    "Save screenshot file..." },
/* da */ { IDMS_SAVE_STOP_MEDIA_FILE_DA, "Gem screenshot..." },
/* de */ { IDMS_SAVE_STOP_MEDIA_FILE_DE, "Screenshot speichern..." },
/* es */ { IDMS_SAVE_STOP_MEDIA_FILE_ES, "Grabar fichero de captura de pantalla..." },
/* fr */ { IDMS_SAVE_STOP_MEDIA_FILE_FR, "Enregistrer une capture d'écran..." },
/* hu */ { IDMS_SAVE_STOP_MEDIA_FILE_HU, "Fénykép fájl mentése..." },
/* it */ { IDMS_SAVE_STOP_MEDIA_FILE_IT, "Salva schermata su file..." },
/* ko */ { IDMS_SAVE_STOP_MEDIA_FILE_KO, "½ºÅ©¸°¼¦ ÆÄÀÏ ÀúÀå..." },
/* nl */ { IDMS_SAVE_STOP_MEDIA_FILE_NL, "Opslaan schermafdrukbestand..." },
/* pl */ { IDMS_SAVE_STOP_MEDIA_FILE_PL, "Zapisz plik zrzutu ekranu..." },
/* ru */ { IDMS_SAVE_STOP_MEDIA_FILE_RU, "ÁÞåàÐÝØâì äÐÙÛ áÚàØÝèÞâÐ..." },
/* sv */ { IDMS_SAVE_STOP_MEDIA_FILE_SV, "Spara skärmdump..." },
/* tr */ { IDMS_SAVE_STOP_MEDIA_FILE_TR, "Ekran görüntüsü dosyasý kaydet..." },

/* en */ { IDMS_NATIVE_SCREENSHOT_SETTINGS,    "Native screenshot settings..." },
/* da */ { IDMS_NATIVE_SCREENSHOT_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDMS_NATIVE_SCREENSHOT_SETTINGS_DE, "Native Screenshot Einstellungen..." },
/* es */ { IDMS_NATIVE_SCREENSHOT_SETTINGS_ES, "" },  /* fuzzy */
/* fr */ { IDMS_NATIVE_SCREENSHOT_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_NATIVE_SCREENSHOT_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_NATIVE_SCREENSHOT_SETTINGS_IT, "" },  /* fuzzy */
/* ko */ { IDMS_NATIVE_SCREENSHOT_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_NATIVE_SCREENSHOT_SETTINGS_NL, "Koala/doodle schermafdruk instellingen..." },
/* pl */ { IDMS_NATIVE_SCREENSHOT_SETTINGS_PL, "" },  /* fuzzy */
/* ru */ { IDMS_NATIVE_SCREENSHOT_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_NATIVE_SCREENSHOT_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDMS_NATIVE_SCREENSHOT_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDMS_NETPLAY,    "Netplay (experimental)..." },
/* da */ { IDMS_NETPLAY_DA, "Netværksspil (eksperimentelt)..." },
/* de */ { IDMS_NETPLAY_DE, "Netplay (experimentell)..." },
/* es */ { IDMS_NETPLAY_ES, "Juego en red (experimental)..." },
/* fr */ { IDMS_NETPLAY_FR, "Jeu en réseau (experimental)..." },
/* hu */ { IDMS_NETPLAY_HU, "Hálózati játék (kísérleti)..." },
/* it */ { IDMS_NETPLAY_IT, "Netplay (sperimentale)..." },
/* ko */ { IDMS_NETPLAY_KO, "³ÝÇÃ·¹ÀÌ (½ÇÇèÀû)..." },
/* nl */ { IDMS_NETPLAY_NL, "Netplay (experimenteel)..." },
/* pl */ { IDMS_NETPLAY_PL, "Gra sieciowa (eksperymentalne)..." },
/* ru */ { IDMS_NETPLAY_RU, "Netplay (experimental)..." },
/* sv */ { IDMS_NETPLAY_SV, "Nätverksspel (experimentellt)..." },
/* tr */ { IDMS_NETPLAY_TR, "Netplay (deneysel)..." },

/* en */ { IDMS_OPTIONS,    "Options" },
/* da */ { IDMS_OPTIONS_DA, "Valg" },
/* de */ { IDMS_OPTIONS_DE, "Optionen" },
/* es */ { IDMS_OPTIONS_ES, "Opciones" },
/* fr */ { IDMS_OPTIONS_FR, "Options" },
/* hu */ { IDMS_OPTIONS_HU, "Opciók" },
/* it */ { IDMS_OPTIONS_IT, "Opzioni" },
/* ko */ { IDMS_OPTIONS_KO, "¼±ÅÃ" },
/* nl */ { IDMS_OPTIONS_NL, "Opties" },
/* pl */ { IDMS_OPTIONS_PL, "Opcje" },
/* ru */ { IDMS_OPTIONS_RU, "¾ßæØØ" },
/* sv */ { IDMS_OPTIONS_SV, "Alternativ" },
/* tr */ { IDMS_OPTIONS_TR, "Seçenekler" },

/* en */ { IDMS_REFRESH_RATE,    "Refresh rate" },
/* da */ { IDMS_REFRESH_RATE_DA, "Opdateringsfrekvens" },
/* de */ { IDMS_REFRESH_RATE_DE, "Wiederholrate" },
/* es */ { IDMS_REFRESH_RATE_ES, "Velocidad de refresco" },
/* fr */ { IDMS_REFRESH_RATE_FR, "Taux de rafraîchissement" },
/* hu */ { IDMS_REFRESH_RATE_HU, "Frissítési ráta" },
/* it */ { IDMS_REFRESH_RATE_IT, "Frequenza di aggiornamento" },
/* ko */ { IDMS_REFRESH_RATE_KO, "Àç»ý·ü" },
/* nl */ { IDMS_REFRESH_RATE_NL, "Vernieuwing snelheid" },
/* pl */ { IDMS_REFRESH_RATE_PL, "Czêstotliwo¶æ od¶wie¿ania" },
/* ru */ { IDMS_REFRESH_RATE_RU, "ÇÐáâÞâÐ ÞÑÝÞÒÛÕÝØï" },
/* sv */ { IDMS_REFRESH_RATE_SV, "Uppdateringshastighet" },
/* tr */ { IDMS_REFRESH_RATE_TR, "Yenileme hýzý" },

/* en */ { IDMS_AUTO,    "Auto" },
/* da */ { IDMS_AUTO_DA, "Auto" },
/* de */ { IDMS_AUTO_DE, "Automatisch" },
/* es */ { IDMS_AUTO_ES, "Auto" },
/* fr */ { IDMS_AUTO_FR, "Automatique" },
/* hu */ { IDMS_AUTO_HU, "Automatikus" },
/* it */ { IDMS_AUTO_IT, "Auto" },
/* ko */ { IDMS_AUTO_KO, "ÀÚµ¿" },
/* nl */ { IDMS_AUTO_NL, "Auto" },
/* pl */ { IDMS_AUTO_PL, "Automatyczna" },
/* ru */ { IDMS_AUTO_RU, "°ÒâÞ" },
/* sv */ { IDMS_AUTO_SV, "Auto" },
/* tr */ { IDMS_AUTO_TR, "Otomatik" },

/* en */ { IDMS_1_1,    "1/1" },
/* da */ { IDMS_1_1_DA, "1/1" },
/* de */ { IDMS_1_1_DE, "1/1" },
/* es */ { IDMS_1_1_ES, "1/1" },
/* fr */ { IDMS_1_1_FR, "1/1" },
/* hu */ { IDMS_1_1_HU, "" },  /* fuzzy */
/* it */ { IDMS_1_1_IT, "1/1" },
/* ko */ { IDMS_1_1_KO, "1/1" },
/* nl */ { IDMS_1_1_NL, "1/1" },
/* pl */ { IDMS_1_1_PL, "1/1" },
/* ru */ { IDMS_1_1_RU, "1/1" },
/* sv */ { IDMS_1_1_SV, "1/1" },
/* tr */ { IDMS_1_1_TR, "1/1" },

/* en */ { IDMS_1_2,    "1/2" },
/* da */ { IDMS_1_2_DA, "1/2" },
/* de */ { IDMS_1_2_DE, "1/2" },
/* es */ { IDMS_1_2_ES, "1/2" },
/* fr */ { IDMS_1_2_FR, "1/2" },
/* hu */ { IDMS_1_2_HU, "" },  /* fuzzy */
/* it */ { IDMS_1_2_IT, "1/2" },
/* ko */ { IDMS_1_2_KO, "1/2" },
/* nl */ { IDMS_1_2_NL, "1/2" },
/* pl */ { IDMS_1_2_PL, "1/2" },
/* ru */ { IDMS_1_2_RU, "1/2" },
/* sv */ { IDMS_1_2_SV, "1/2" },
/* tr */ { IDMS_1_2_TR, "1/2" },

/* en */ { IDMS_1_3,    "1/3" },
/* da */ { IDMS_1_3_DA, "1/3" },
/* de */ { IDMS_1_3_DE, "1/3" },
/* es */ { IDMS_1_3_ES, "1/3" },
/* fr */ { IDMS_1_3_FR, "1/3" },
/* hu */ { IDMS_1_3_HU, "" },  /* fuzzy */
/* it */ { IDMS_1_3_IT, "1/3" },
/* ko */ { IDMS_1_3_KO, "1/3" },
/* nl */ { IDMS_1_3_NL, "1/3" },
/* pl */ { IDMS_1_3_PL, "1/3" },
/* ru */ { IDMS_1_3_RU, "1/3" },
/* sv */ { IDMS_1_3_SV, "1/3" },
/* tr */ { IDMS_1_3_TR, "1/3" },

/* en */ { IDMS_1_4,    "1/4" },
/* da */ { IDMS_1_4_DA, "1/4" },
/* de */ { IDMS_1_4_DE, "1/4" },
/* es */ { IDMS_1_4_ES, "1/4" },
/* fr */ { IDMS_1_4_FR, "1/4" },
/* hu */ { IDMS_1_4_HU, "" },  /* fuzzy */
/* it */ { IDMS_1_4_IT, "1/4" },
/* ko */ { IDMS_1_4_KO, "1/4" },
/* nl */ { IDMS_1_4_NL, "1/4" },
/* pl */ { IDMS_1_4_PL, "1/4" },
/* ru */ { IDMS_1_4_RU, "1/4" },
/* sv */ { IDMS_1_4_SV, "1/4" },
/* tr */ { IDMS_1_4_TR, "1/4" },

/* en */ { IDMS_1_5,    "1/5" },
/* da */ { IDMS_1_5_DA, "1/5" },
/* de */ { IDMS_1_5_DE, "1/5" },
/* es */ { IDMS_1_5_ES, "1/5" },
/* fr */ { IDMS_1_5_FR, "1/5" },
/* hu */ { IDMS_1_5_HU, "" },  /* fuzzy */
/* it */ { IDMS_1_5_IT, "1/5" },
/* ko */ { IDMS_1_5_KO, "1/5" },
/* nl */ { IDMS_1_5_NL, "1/5" },
/* pl */ { IDMS_1_5_PL, "1/5" },
/* ru */ { IDMS_1_5_RU, "1/5" },
/* sv */ { IDMS_1_5_SV, "1/5" },
/* tr */ { IDMS_1_5_TR, "1/5" },

/* en */ { IDMS_1_6,    "1/6" },
/* da */ { IDMS_1_6_DA, "1/6" },
/* de */ { IDMS_1_6_DE, "1/6" },
/* es */ { IDMS_1_6_ES, "1/6" },
/* fr */ { IDMS_1_6_FR, "1/6" },
/* hu */ { IDMS_1_6_HU, "" },  /* fuzzy */
/* it */ { IDMS_1_6_IT, "1/6" },
/* ko */ { IDMS_1_6_KO, "1/6" },
/* nl */ { IDMS_1_6_NL, "1/6" },
/* pl */ { IDMS_1_6_PL, "1/6" },
/* ru */ { IDMS_1_6_RU, "1/6" },
/* sv */ { IDMS_1_6_SV, "1/6" },
/* tr */ { IDMS_1_6_TR, "1/6" },

/* en */ { IDMS_1_7,    "1/7" },
/* da */ { IDMS_1_7_DA, "1/7" },
/* de */ { IDMS_1_7_DE, "1/7" },
/* es */ { IDMS_1_7_ES, "1/7" },
/* fr */ { IDMS_1_7_FR, "1/7" },
/* hu */ { IDMS_1_7_HU, "" },  /* fuzzy */
/* it */ { IDMS_1_7_IT, "1/7" },
/* ko */ { IDMS_1_7_KO, "1/7" },
/* nl */ { IDMS_1_7_NL, "1/7" },
/* pl */ { IDMS_1_7_PL, "1/7" },
/* ru */ { IDMS_1_7_RU, "1/7" },
/* sv */ { IDMS_1_7_SV, "1/7" },
/* tr */ { IDMS_1_7_TR, "1/7" },

/* en */ { IDMS_1_8,    "1/8" },
/* da */ { IDMS_1_8_DA, "1/8" },
/* de */ { IDMS_1_8_DE, "1/8" },
/* es */ { IDMS_1_8_ES, "1/8" },
/* fr */ { IDMS_1_8_FR, "1/8" },
/* hu */ { IDMS_1_8_HU, "" },  /* fuzzy */
/* it */ { IDMS_1_8_IT, "1/8" },
/* ko */ { IDMS_1_8_KO, "1/8" },
/* nl */ { IDMS_1_8_NL, "1/8" },
/* pl */ { IDMS_1_8_PL, "1/8" },
/* ru */ { IDMS_1_8_RU, "1/8" },
/* sv */ { IDMS_1_8_SV, "1/8" },
/* tr */ { IDMS_1_8_TR, "1/8" },

/* en */ { IDMS_1_9,    "1/9" },
/* da */ { IDMS_1_9_DA, "1/9" },
/* de */ { IDMS_1_9_DE, "1/9" },
/* es */ { IDMS_1_9_ES, "1/9" },
/* fr */ { IDMS_1_9_FR, "1/9" },
/* hu */ { IDMS_1_9_HU, "" },  /* fuzzy */
/* it */ { IDMS_1_9_IT, "1/9" },
/* ko */ { IDMS_1_9_KO, "1/9" },
/* nl */ { IDMS_1_9_NL, "1/9" },
/* pl */ { IDMS_1_9_PL, "1/9" },
/* ru */ { IDMS_1_9_RU, "1/9" },
/* sv */ { IDMS_1_9_SV, "1/9" },
/* tr */ { IDMS_1_9_TR, "1/9" },

/* en */ { IDMS_1_10,    "1/10" },
/* da */ { IDMS_1_10_DA, "1/10" },
/* de */ { IDMS_1_10_DE, "1/10" },
/* es */ { IDMS_1_10_ES, "1/10" },
/* fr */ { IDMS_1_10_FR, "1/10" },
/* hu */ { IDMS_1_10_HU, "" },  /* fuzzy */
/* it */ { IDMS_1_10_IT, "1/10" },
/* ko */ { IDMS_1_10_KO, "1/10" },
/* nl */ { IDMS_1_10_NL, "1/10" },
/* pl */ { IDMS_1_10_PL, "1/10" },
/* ru */ { IDMS_1_10_RU, "1/10" },
/* sv */ { IDMS_1_10_SV, "1/10" },
/* tr */ { IDMS_1_10_TR, "1/10" },

/* en */ { IDMS_MAXIMUM_SPEED,    "Maximum speed" },
/* da */ { IDMS_MAXIMUM_SPEED_DA, "Maksimal hastighed" },
/* de */ { IDMS_MAXIMUM_SPEED_DE, "Maximale Geschwindigkeit" },
/* es */ { IDMS_MAXIMUM_SPEED_ES, "Velocidad máxima" },
/* fr */ { IDMS_MAXIMUM_SPEED_FR, "Vitesse maximale" },
/* hu */ { IDMS_MAXIMUM_SPEED_HU, "Maximális sebesség" },
/* it */ { IDMS_MAXIMUM_SPEED_IT, "Velocità massima" },
/* ko */ { IDMS_MAXIMUM_SPEED_KO, "" },  /* fuzzy */
/* nl */ { IDMS_MAXIMUM_SPEED_NL, "Maximale snelheid" },
/* pl */ { IDMS_MAXIMUM_SPEED_PL, "Maksymalna prêdko¶æ" },
/* ru */ { IDMS_MAXIMUM_SPEED_RU, "¼ÐÚáØÜÐÛìÝÐï áÚÞàÞáâì" },
/* sv */ { IDMS_MAXIMUM_SPEED_SV, "Maximal hastighet" },
/* tr */ { IDMS_MAXIMUM_SPEED_TR, "Maksimum hýz" },

/* en */ { IDMS_200_PERCENT,    "200%" },
/* da */ { IDMS_200_PERCENT_DA, "200%" },
/* de */ { IDMS_200_PERCENT_DE, "200%" },
/* es */ { IDMS_200_PERCENT_ES, "200%" },
/* fr */ { IDMS_200_PERCENT_FR, "200%" },
/* hu */ { IDMS_200_PERCENT_HU, "" },  /* fuzzy */
/* it */ { IDMS_200_PERCENT_IT, "200%" },
/* ko */ { IDMS_200_PERCENT_KO, "200%" },
/* nl */ { IDMS_200_PERCENT_NL, "200%" },
/* pl */ { IDMS_200_PERCENT_PL, "200%" },
/* ru */ { IDMS_200_PERCENT_RU, "200%" },
/* sv */ { IDMS_200_PERCENT_SV, "200%" },
/* tr */ { IDMS_200_PERCENT_TR, "200%" },

/* en */ { IDMS_100_PERCENT,    "100%" },
/* da */ { IDMS_100_PERCENT_DA, "100%" },
/* de */ { IDMS_100_PERCENT_DE, "100%" },
/* es */ { IDMS_100_PERCENT_ES, "100%" },
/* fr */ { IDMS_100_PERCENT_FR, "100%" },
/* hu */ { IDMS_100_PERCENT_HU, "" },  /* fuzzy */
/* it */ { IDMS_100_PERCENT_IT, "100%" },
/* ko */ { IDMS_100_PERCENT_KO, "100%" },
/* nl */ { IDMS_100_PERCENT_NL, "100%" },
/* pl */ { IDMS_100_PERCENT_PL, "100%" },
/* ru */ { IDMS_100_PERCENT_RU, "100%" },
/* sv */ { IDMS_100_PERCENT_SV, "100%" },
/* tr */ { IDMS_100_PERCENT_TR, "100%" },

/* en */ { IDMS_50_PERCENT,    "50%" },
/* da */ { IDMS_50_PERCENT_DA, "50%" },
/* de */ { IDMS_50_PERCENT_DE, "50%" },
/* es */ { IDMS_50_PERCENT_ES, "50%" },
/* fr */ { IDMS_50_PERCENT_FR, "50%" },
/* hu */ { IDMS_50_PERCENT_HU, "" },  /* fuzzy */
/* it */ { IDMS_50_PERCENT_IT, "50%" },
/* ko */ { IDMS_50_PERCENT_KO, "50%" },
/* nl */ { IDMS_50_PERCENT_NL, "50%" },
/* pl */ { IDMS_50_PERCENT_PL, "50%" },
/* ru */ { IDMS_50_PERCENT_RU, "50%" },
/* sv */ { IDMS_50_PERCENT_SV, "50%" },
/* tr */ { IDMS_50_PERCENT_TR, "50%" },

/* en */ { IDMS_20_PERCENT,    "20%" },
/* da */ { IDMS_20_PERCENT_DA, "20%" },
/* de */ { IDMS_20_PERCENT_DE, "20%" },
/* es */ { IDMS_20_PERCENT_ES, "20%" },
/* fr */ { IDMS_20_PERCENT_FR, "20%" },
/* hu */ { IDMS_20_PERCENT_HU, "" },  /* fuzzy */
/* it */ { IDMS_20_PERCENT_IT, "20%" },
/* ko */ { IDMS_20_PERCENT_KO, "20%" },
/* nl */ { IDMS_20_PERCENT_NL, "20%" },
/* pl */ { IDMS_20_PERCENT_PL, "20%" },
/* ru */ { IDMS_20_PERCENT_RU, "20%" },
/* sv */ { IDMS_20_PERCENT_SV, "20%" },
/* tr */ { IDMS_20_PERCENT_TR, "20%" },

/* en */ { IDMS_10_PERCENT,    "10%" },
/* da */ { IDMS_10_PERCENT_DA, "10%" },
/* de */ { IDMS_10_PERCENT_DE, "10%" },
/* es */ { IDMS_10_PERCENT_ES, "10%" },
/* fr */ { IDMS_10_PERCENT_FR, "10%" },
/* hu */ { IDMS_10_PERCENT_HU, "" },  /* fuzzy */
/* it */ { IDMS_10_PERCENT_IT, "10%" },
/* ko */ { IDMS_10_PERCENT_KO, "10%" },
/* nl */ { IDMS_10_PERCENT_NL, "10%" },
/* pl */ { IDMS_10_PERCENT_PL, "10%" },
/* ru */ { IDMS_10_PERCENT_RU, "10%" },
/* sv */ { IDMS_10_PERCENT_SV, "10%" },
/* tr */ { IDMS_10_PERCENT_TR, "10%" },

/* en */ { IDMS_NO_LIMIT,    "No limit" },
/* da */ { IDMS_NO_LIMIT_DA, "Ubegrænset" },
/* de */ { IDMS_NO_LIMIT_DE, "Kein Limit" },
/* es */ { IDMS_NO_LIMIT_ES, "Sin límite" },
/* fr */ { IDMS_NO_LIMIT_FR, "Aucune limite" },
/* hu */ { IDMS_NO_LIMIT_HU, "Nincs határ" },
/* it */ { IDMS_NO_LIMIT_IT, "Nessun limite" },
/* ko */ { IDMS_NO_LIMIT_KO, "ÇÑ°è¾øÀ½" },
/* nl */ { IDMS_NO_LIMIT_NL, "Geen limiet" },
/* pl */ { IDMS_NO_LIMIT_PL, "Bez limitu" },
/* ru */ { IDMS_NO_LIMIT_RU, "No limit" },
/* sv */ { IDMS_NO_LIMIT_SV, "Ingen gräns" },
/* tr */ { IDMS_NO_LIMIT_TR, "Limitsiz" },

/* en */ { IDMS_CUSTOM,    "Custom" },
/* da */ { IDMS_CUSTOM_DA, "Egen" },
/* de */ { IDMS_CUSTOM_DE, "Benutzerdefiniert" },
/* es */ { IDMS_CUSTOM_ES, "Personalizado" },
/* fr */ { IDMS_CUSTOM_FR, "Personnalisé" },
/* hu */ { IDMS_CUSTOM_HU, "Egyéni" },
/* it */ { IDMS_CUSTOM_IT, "Personalizzato" },
/* ko */ { IDMS_CUSTOM_KO, "Ä¿½ºÅÒ" },
/* nl */ { IDMS_CUSTOM_NL, "Aangepast" },
/* pl */ { IDMS_CUSTOM_PL, "W³asna" },
/* ru */ { IDMS_CUSTOM_RU, "Custom" },
/* sv */ { IDMS_CUSTOM_SV, "Egen" },
/* tr */ { IDMS_CUSTOM_TR, "Özel" },

/* en */ { IDMS_WARP_MODE,    "Warp mode" },
/* da */ { IDMS_WARP_MODE_DA, "Warp-tilstand" },
/* de */ { IDMS_WARP_MODE_DE, "Warp modus" },
/* es */ { IDMS_WARP_MODE_ES, "Modo máxima velocidad" },
/* fr */ { IDMS_WARP_MODE_FR, "Mode Turbo " },
/* hu */ { IDMS_WARP_MODE_HU, "Hipergyors mód" },
/* it */ { IDMS_WARP_MODE_IT, "Modalità turbo" },
/* ko */ { IDMS_WARP_MODE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_WARP_MODE_NL, "Warpmodus" },
/* pl */ { IDMS_WARP_MODE_PL, "Tryb przyspieszony" },
/* ru */ { IDMS_WARP_MODE_RU, "Warp mode" },
/* sv */ { IDMS_WARP_MODE_SV, "Warpläge" },
/* tr */ { IDMS_WARP_MODE_TR, "Warp modu" },

/* en */ { IDMS_FULLSCREEN,    "Fullscreen" },
/* da */ { IDMS_FULLSCREEN_DA, "Fuldskærm" },
/* de */ { IDMS_FULLSCREEN_DE, "Vollbild" },
/* es */ { IDMS_FULLSCREEN_ES, "Pantalla completa" },
/* fr */ { IDMS_FULLSCREEN_FR, "Plein écran" },
/* hu */ { IDMS_FULLSCREEN_HU, "Teljes képernyõ" },
/* it */ { IDMS_FULLSCREEN_IT, "Schermo intero" },
/* ko */ { IDMS_FULLSCREEN_KO, "ÀüÃ¼È­¸é" },
/* nl */ { IDMS_FULLSCREEN_NL, "Volledig Scherm" },
/* pl */ { IDMS_FULLSCREEN_PL, "Pe³ny ekran" },
/* ru */ { IDMS_FULLSCREEN_RU, "²Þ ÒÕáì íÚàÐÝ" },
/* sv */ { IDMS_FULLSCREEN_SV, "Fullskärm" },
/* tr */ { IDMS_FULLSCREEN_TR, "Tam ekran" },

/* en */ { IDMS_VIDEO_CACHE,    "Video cache" },
/* da */ { IDMS_VIDEO_CACHE_DA, "Grafikcache" },
/* de */ { IDMS_VIDEO_CACHE_DE, "Video Cache" },
/* es */ { IDMS_VIDEO_CACHE_ES, "Caché de video" },
/* fr */ { IDMS_VIDEO_CACHE_FR, "Cache vidéo" },
/* hu */ { IDMS_VIDEO_CACHE_HU, "Kép gyorsítótár" },
/* it */ { IDMS_VIDEO_CACHE_IT, "Cache video" },
/* ko */ { IDMS_VIDEO_CACHE_KO, "ºñµð¿À Ä³½Ã" },
/* nl */ { IDMS_VIDEO_CACHE_NL, "Videocache" },
/* pl */ { IDMS_VIDEO_CACHE_PL, "Cache wideo" },
/* ru */ { IDMS_VIDEO_CACHE_RU, "²ØÔÕÞ ÚÕè" },
/* sv */ { IDMS_VIDEO_CACHE_SV, "Grafikcache" },
/* tr */ { IDMS_VIDEO_CACHE_TR, "Görüntü önbelleði" },

/* en */ { IDMS_DOUBLE_SIZE,    "Double size" },
/* da */ { IDMS_DOUBLE_SIZE_DA, "Dobbelt størrelse" },
/* de */ { IDMS_DOUBLE_SIZE_DE, "Doppelte Größe" },
/* es */ { IDMS_DOUBLE_SIZE_ES, "Tamaño doble" },
/* fr */ { IDMS_DOUBLE_SIZE_FR, "Double taille" },
/* hu */ { IDMS_DOUBLE_SIZE_HU, "Dupla méret" },
/* it */ { IDMS_DOUBLE_SIZE_IT, "Dimensione doppia" },
/* ko */ { IDMS_DOUBLE_SIZE_KO, "´õºí »çÀÌÁî" },
/* nl */ { IDMS_DOUBLE_SIZE_NL, "Dubbele grootte" },
/* pl */ { IDMS_DOUBLE_SIZE_PL, "Podwójny rozmiar" },
/* ru */ { IDMS_DOUBLE_SIZE_RU, "´ÒÞÙÝÞÙ àÐ×ÜÕà" },
/* sv */ { IDMS_DOUBLE_SIZE_SV, "Dubbel storlek" },
/* tr */ { IDMS_DOUBLE_SIZE_TR, "Çift boyut" },

/* en */ { IDMS_DOUBLE_SCAN,    "Double scan" },
/* da */ { IDMS_DOUBLE_SCAN_DA, "Dobbelt-skan" },
/* de */ { IDMS_DOUBLE_SCAN_DE, "Doppelscan" },
/* es */ { IDMS_DOUBLE_SCAN_ES, "Doble escaneo" },
/* fr */ { IDMS_DOUBLE_SCAN_FR, "Double scan" },
/* hu */ { IDMS_DOUBLE_SCAN_HU, "Dupla pásztázás" },
/* it */ { IDMS_DOUBLE_SCAN_IT, "Scansione doppia" },
/* ko */ { IDMS_DOUBLE_SCAN_KO, "´õºí ½ºÄµ" },
/* nl */ { IDMS_DOUBLE_SCAN_NL, "Dubbele scan" },
/* pl */ { IDMS_DOUBLE_SCAN_PL, "Podwójne skanowanie" },
/* ru */ { IDMS_DOUBLE_SCAN_RU, "Double scan" },
/* sv */ { IDMS_DOUBLE_SCAN_SV, "Dubbelskanning" },
/* tr */ { IDMS_DOUBLE_SCAN_TR, "Çift tarama" },

/* en */ { IDMS_VERTICAL_STRETCH,    "Vertical stretch" },
/* da */ { IDMS_VERTICAL_STRETCH_DA, "Vertikal stræk" },
/* de */ { IDMS_VERTICAL_STRETCH_DE, "Vertikale Dehnung" },
/* es */ { IDMS_VERTICAL_STRETCH_ES, "Extensión vertical" },
/* fr */ { IDMS_VERTICAL_STRETCH_FR, "Étirement vertical" },
/* hu */ { IDMS_VERTICAL_STRETCH_HU, "" },  /* fuzzy */
/* it */ { IDMS_VERTICAL_STRETCH_IT, "Estensione verticale" },
/* ko */ { IDMS_VERTICAL_STRETCH_KO, "" },  /* fuzzy */
/* nl */ { IDMS_VERTICAL_STRETCH_NL, "Verticaal uitrekken" },
/* pl */ { IDMS_VERTICAL_STRETCH_PL, "Rozci±gniêcie w pionie" },
/* ru */ { IDMS_VERTICAL_STRETCH_RU, "ÀÐáâïÝãâì ßÞ ÒÕàâØÚÐÛØ" },
/* sv */ { IDMS_VERTICAL_STRETCH_SV, "Vertikal sträckning" },
/* tr */ { IDMS_VERTICAL_STRETCH_TR, "Dikey uzatma" },

/* en */ { IDMS_VDC_SETTINGS,    "VDC settings" },
/* da */ { IDMS_VDC_SETTINGS_DA, "VDC-indstillinger" },
/* de */ { IDMS_VDC_SETTINGS_DE, "VDC Einstellungen" },
/* es */ { IDMS_VDC_SETTINGS_ES, "Ajustes VDC" },
/* fr */ { IDMS_VDC_SETTINGS_FR, "Paramètres VDC" },
/* hu */ { IDMS_VDC_SETTINGS_HU, "VDC beállításai" },
/* it */ { IDMS_VDC_SETTINGS_IT, "Impostazioni VDC" },
/* ko */ { IDMS_VDC_SETTINGS_KO, "VDC ¼³Á¤" },
/* nl */ { IDMS_VDC_SETTINGS_NL, "VDC-instellingen" },
/* pl */ { IDMS_VDC_SETTINGS_PL, "Ustawienia VDC" },
/* ru */ { IDMS_VDC_SETTINGS_RU, "½ÐáâàÞÙÚØ VDC" },
/* sv */ { IDMS_VDC_SETTINGS_SV, "VDC-inställningar" },
/* tr */ { IDMS_VDC_SETTINGS_TR, "VDC ayarlarý" },

/* en */ { IDMS_64KB_VIDEO_MEMORY,    "64KB video memory" },
/* da */ { IDMS_64KB_VIDEO_MEMORY_DA, "64KB grafikhukommelse" },
/* de */ { IDMS_64KB_VIDEO_MEMORY_DE, "64KB Bildschirm Speicher" },
/* es */ { IDMS_64KB_VIDEO_MEMORY_ES, "Memoria de vídeo 64KB" },
/* fr */ { IDMS_64KB_VIDEO_MEMORY_FR, "Mémoire vidéo de 64Ko" },
/* hu */ { IDMS_64KB_VIDEO_MEMORY_HU, "64KB képernyõ memória" },
/* it */ { IDMS_64KB_VIDEO_MEMORY_IT, "Memoria video di 64KB" },
/* ko */ { IDMS_64KB_VIDEO_MEMORY_KO, "64KB ºñµð¿À ¸Þ¸ð¸®" },
/* nl */ { IDMS_64KB_VIDEO_MEMORY_NL, "64KB videogeheugen" },
/* pl */ { IDMS_64KB_VIDEO_MEMORY_PL, "Pamiêæ wideo 64KB" },
/* ru */ { IDMS_64KB_VIDEO_MEMORY_RU, "64KB video memory" },
/* sv */ { IDMS_64KB_VIDEO_MEMORY_SV, "64KB grafikminne" },
/* tr */ { IDMS_64KB_VIDEO_MEMORY_TR, "64KB görüntü belleði" },

/* en */ { IDMS_VDC_REVISION,    "VDC revision" },
/* da */ { IDMS_VDC_REVISION_DA, "VDC-revision" },
/* de */ { IDMS_VDC_REVISION_DE, "VDC Revision" },
/* es */ { IDMS_VDC_REVISION_ES, "Revisión VDC" },
/* fr */ { IDMS_VDC_REVISION_FR, "Révision VDC" },
/* hu */ { IDMS_VDC_REVISION_HU, "VDC változat" },
/* it */ { IDMS_VDC_REVISION_IT, "Revisione VDC" },
/* ko */ { IDMS_VDC_REVISION_KO, "" },  /* fuzzy */
/* nl */ { IDMS_VDC_REVISION_NL, "VDC revisie" },
/* pl */ { IDMS_VDC_REVISION_PL, "Wersja VDC" },
/* ru */ { IDMS_VDC_REVISION_RU, "VDC revision" },
/* sv */ { IDMS_VDC_REVISION_SV, "VDC-revision" },
/* tr */ { IDMS_VDC_REVISION_TR, "VDC revizyonu" },

/* en */ { IDMS_VDC_REV_0,    "Rev 0" },
/* da */ { IDMS_VDC_REV_0_DA, "Udgave 0" },
/* de */ { IDMS_VDC_REV_0_DE, "Rev 0" },
/* es */ { IDMS_VDC_REV_0_ES, "Rev 0" },
/* fr */ { IDMS_VDC_REV_0_FR, "Rév 0" },
/* hu */ { IDMS_VDC_REV_0_HU, "0-ás változat" },
/* it */ { IDMS_VDC_REV_0_IT, "Rev 0" },
/* ko */ { IDMS_VDC_REV_0_KO, "¼öÁ¤ 0" },
/* nl */ { IDMS_VDC_REV_0_NL, "Revisie 0" },
/* pl */ { IDMS_VDC_REV_0_PL, "Rev 0" },
/* ru */ { IDMS_VDC_REV_0_RU, "Rev 0" },
/* sv */ { IDMS_VDC_REV_0_SV, "Utgåva 0" },
/* tr */ { IDMS_VDC_REV_0_TR, "Rev 0" },

/* en */ { IDMS_VDC_REV_1,    "Rev 1" },
/* da */ { IDMS_VDC_REV_1_DA, "Udgave 1" },
/* de */ { IDMS_VDC_REV_1_DE, "Rev 1" },
/* es */ { IDMS_VDC_REV_1_ES, "Rev 1" },
/* fr */ { IDMS_VDC_REV_1_FR, "Rév 1" },
/* hu */ { IDMS_VDC_REV_1_HU, "1-es változat" },
/* it */ { IDMS_VDC_REV_1_IT, "Rev 1" },
/* ko */ { IDMS_VDC_REV_1_KO, "¼öÁ¤ 1" },
/* nl */ { IDMS_VDC_REV_1_NL, "Revisie 1" },
/* pl */ { IDMS_VDC_REV_1_PL, "Rev 1" },
/* ru */ { IDMS_VDC_REV_1_RU, "Rev 1" },
/* sv */ { IDMS_VDC_REV_1_SV, "Utgåva 1" },
/* tr */ { IDMS_VDC_REV_1_TR, "Rev 1" },

/* en */ { IDMS_VDC_REV_2,    "Rev 2" },
/* da */ { IDMS_VDC_REV_2_DA, "Udgave 2" },
/* de */ { IDMS_VDC_REV_2_DE, "Rev 2" },
/* es */ { IDMS_VDC_REV_2_ES, "Rev 2" },
/* fr */ { IDMS_VDC_REV_2_FR, "Rév 2" },
/* hu */ { IDMS_VDC_REV_2_HU, "2-es változat" },
/* it */ { IDMS_VDC_REV_2_IT, "Rev 2" },
/* ko */ { IDMS_VDC_REV_2_KO, "¼öÁ¤ 2" },
/* nl */ { IDMS_VDC_REV_2_NL, "Revisie 2" },
/* pl */ { IDMS_VDC_REV_2_PL, "Rev 2" },
/* ru */ { IDMS_VDC_REV_2_RU, "Rev 2" },
/* sv */ { IDMS_VDC_REV_2_SV, "Utgåva 2" },
/* tr */ { IDMS_VDC_REV_2_TR, "Rev 2" },

/* en */ { IDMS_SWAP_JOYSTICKS,    "Swap joysticks" },
/* da */ { IDMS_SWAP_JOYSTICKS_DA, "Byt om på joystickportene" },
/* de */ { IDMS_SWAP_JOYSTICKS_DE, "Joystick ports austauschen" },
/* es */ { IDMS_SWAP_JOYSTICKS_ES, "Intercambiar joysticks" },
/* fr */ { IDMS_SWAP_JOYSTICKS_FR, "Permuter les ports Joysticks" },
/* hu */ { IDMS_SWAP_JOYSTICKS_HU, "" },  /* fuzzy */
/* it */ { IDMS_SWAP_JOYSTICKS_IT, "Scambia joystick" },
/* ko */ { IDMS_SWAP_JOYSTICKS_KO, "Á¶ÀÌ½ºÆ½À» ¹Ù²Ù¼¼¿ä" },
/* nl */ { IDMS_SWAP_JOYSTICKS_NL, "Verwissel joysticks" },
/* pl */ { IDMS_SWAP_JOYSTICKS_PL, "Zamieñ joysticki" },
/* ru */ { IDMS_SWAP_JOYSTICKS_RU, "Swap joysticks" },
/* sv */ { IDMS_SWAP_JOYSTICKS_SV, "Växla spelportarna" },
/* tr */ { IDMS_SWAP_JOYSTICKS_TR, "Joystickleri deðiþtir" },

/* en */ { IDMS_SWAP_USERPORT_JOYSTICKS,    "Swap userport joysticks" },
/* da */ { IDMS_SWAP_USERPORT_JOYSTICKS_DA, "Byt om på brugerportjoysticks" },
/* de */ { IDMS_SWAP_USERPORT_JOYSTICKS_DE, "Userport Joystick ports austauschen" },
/* es */ { IDMS_SWAP_USERPORT_JOYSTICKS_ES, "Intercambiar puertos joysticks" },
/* fr */ { IDMS_SWAP_USERPORT_JOYSTICKS_FR, "Permuter les joysticks de port" },
/* hu */ { IDMS_SWAP_USERPORT_JOYSTICKS_HU, "" },  /* fuzzy */
/* it */ { IDMS_SWAP_USERPORT_JOYSTICKS_IT, "Scambia joystick su userport" },
/* ko */ { IDMS_SWAP_USERPORT_JOYSTICKS_KO, "À¯Á®Æ÷Æ® Á¶ÀÌ½ºÆ½À» ¹Ù²Ù¼¼¿ä" },
/* nl */ { IDMS_SWAP_USERPORT_JOYSTICKS_NL, "Verwissel userport joysticks" },
/* pl */ { IDMS_SWAP_USERPORT_JOYSTICKS_PL, "Zamieñ joysticki w userportach" },
/* ru */ { IDMS_SWAP_USERPORT_JOYSTICKS_RU, "Swap userport joysticks" },
/* sv */ { IDMS_SWAP_USERPORT_JOYSTICKS_SV, "Växla användarportstyrspakar" },
/* tr */ { IDMS_SWAP_USERPORT_JOYSTICKS_TR, "Userport joysticklerini yer deðiþtir" },

/* en */ { IDMS_CGA_USERPORT_JOY_ADAPTER,    "CGA userport joy adapter" },
/* da */ { IDMS_CGA_USERPORT_JOY_ADAPTER_DA, "CGA brugerportjoystickadapter" },
/* de */ { IDMS_CGA_USERPORT_JOY_ADAPTER_DE, "CGA Userport Joystick Adapter" },
/* es */ { IDMS_CGA_USERPORT_JOY_ADAPTER_ES, "Adaptador de puerto de juegos CGA" },
/* fr */ { IDMS_CGA_USERPORT_JOY_ADAPTER_FR, "Adaptateur joystick port utilisateur CGA" },
/* hu */ { IDMS_CGA_USERPORT_JOY_ADAPTER_HU, "CGA userport botkormány adapter" },
/* it */ { IDMS_CGA_USERPORT_JOY_ADAPTER_IT, "Adattatore joystick CGA su userport" },
/* ko */ { IDMS_CGA_USERPORT_JOY_ADAPTER_KO, "CGA À¯Á®Æ÷Æ® Á¶ÀÌ ¾Æ´äÅÍ" },
/* nl */ { IDMS_CGA_USERPORT_JOY_ADAPTER_NL, "CGA userport joy adapter" },
/* pl */ { IDMS_CGA_USERPORT_JOY_ADAPTER_PL, "Z³±cze CGA userportu joysticka" },
/* ru */ { IDMS_CGA_USERPORT_JOY_ADAPTER_RU, "CGA userport joy adapter" },
/* sv */ { IDMS_CGA_USERPORT_JOY_ADAPTER_SV, "CGA-användarportsspakadapter" },
/* tr */ { IDMS_CGA_USERPORT_JOY_ADAPTER_TR, "CGA userport joy dönüþtürücü" },

/* en */ { IDMS_PET_USERPORT_JOY_ADAPTER,    "PET userport joy adapter" },
/* da */ { IDMS_PET_USERPORT_JOY_ADAPTER_DA, "PET brugerportjoystickadapter" },
/* de */ { IDMS_PET_USERPORT_JOY_ADAPTER_DE, "PET Userport Joystick Adapter" },
/* es */ { IDMS_PET_USERPORT_JOY_ADAPTER_ES, "Adaptador de puerto de juegos PET" },
/* fr */ { IDMS_PET_USERPORT_JOY_ADAPTER_FR, "Adaptateur joystick port utilisateur PET" },
/* hu */ { IDMS_PET_USERPORT_JOY_ADAPTER_HU, "PET userport botkormány adapter" },
/* it */ { IDMS_PET_USERPORT_JOY_ADAPTER_IT, "Adattatore joystick PET su userport" },
/* ko */ { IDMS_PET_USERPORT_JOY_ADAPTER_KO, "PET À¯Á®Æ÷Æ® Á¶ÀÌ ¾Æ´äÅÍ" },
/* nl */ { IDMS_PET_USERPORT_JOY_ADAPTER_NL, "PET userport joy adapter" },
/* pl */ { IDMS_PET_USERPORT_JOY_ADAPTER_PL, "Z³±cze PET userportu joysticka" },
/* ru */ { IDMS_PET_USERPORT_JOY_ADAPTER_RU, "PET userport joy adapter" },
/* sv */ { IDMS_PET_USERPORT_JOY_ADAPTER_SV, "PET-användarportsspakadapter" },
/* tr */ { IDMS_PET_USERPORT_JOY_ADAPTER_TR, "PET userport joy dönüþtürücü" },

/* en */ { IDMS_HUMMER_USERPORT_JOY_ADAPTER,    "Hummer userport joy adapter" },
/* da */ { IDMS_HUMMER_USERPORT_JOY_ADAPTER_DA, "Hummer brugerport joystick adapter" },
/* de */ { IDMS_HUMMER_USERPORT_JOY_ADAPTER_DE, "Hummer Userport Joystick Adapter" },
/* es */ { IDMS_HUMMER_USERPORT_JOY_ADAPTER_ES, "Adaptador de puerto de juegos Hummer" },
/* fr */ { IDMS_HUMMER_USERPORT_JOY_ADAPTER_FR, "Adaptateur joystick port utilisateur Hummer" },
/* hu */ { IDMS_HUMMER_USERPORT_JOY_ADAPTER_HU, "" },  /* fuzzy */
/* it */ { IDMS_HUMMER_USERPORT_JOY_ADAPTER_IT, "Adattatore joystick HUMMER su userport" },
/* ko */ { IDMS_HUMMER_USERPORT_JOY_ADAPTER_KO, "Hummer À¯Á®Æ÷Æ® Á¶ÀÌ ¾Æ´äÅÍ" },
/* nl */ { IDMS_HUMMER_USERPORT_JOY_ADAPTER_NL, "Hummer userport joystick adapter" },
/* pl */ { IDMS_HUMMER_USERPORT_JOY_ADAPTER_PL, "Z³±cze Hummer userportu joysticka" },
/* ru */ { IDMS_HUMMER_USERPORT_JOY_ADAPTER_RU, "Hummer userport joy adapter" },
/* sv */ { IDMS_HUMMER_USERPORT_JOY_ADAPTER_SV, "Hummer-användarportsspakadapter" },
/* tr */ { IDMS_HUMMER_USERPORT_JOY_ADAPTER_TR, "Hummer userport joy dönüþtürücü" },

/* en */ { IDMS_OEM_USERPORT_JOY_ADAPTER,    "OEM userport joy adapter" },
/* da */ { IDMS_OEM_USERPORT_JOY_ADAPTER_DA, "OEM brugerportjoystickadapter" },
/* de */ { IDMS_OEM_USERPORT_JOY_ADAPTER_DE, "OEM Userport Joystick Adapter" },
/* es */ { IDMS_OEM_USERPORT_JOY_ADAPTER_ES, "Adaptador de puerto de juegos OEM" },
/* fr */ { IDMS_OEM_USERPORT_JOY_ADAPTER_FR, "Adaptateur joystick port utilisateur OEM" },
/* hu */ { IDMS_OEM_USERPORT_JOY_ADAPTER_HU, "OEM userport botkormány adapter" },
/* it */ { IDMS_OEM_USERPORT_JOY_ADAPTER_IT, "Adattatore joystick OEM su userport" },
/* ko */ { IDMS_OEM_USERPORT_JOY_ADAPTER_KO, "OEM À¯Á®Æ÷Æ® Á¶ÀÌ ¾Æ´äÅÍ" },
/* nl */ { IDMS_OEM_USERPORT_JOY_ADAPTER_NL, "OEM userport joy adapter" },
/* pl */ { IDMS_OEM_USERPORT_JOY_ADAPTER_PL, "Z³±cze OEM userportu joysticka" },
/* ru */ { IDMS_OEM_USERPORT_JOY_ADAPTER_RU, "OEM userport joy adapter" },
/* sv */ { IDMS_OEM_USERPORT_JOY_ADAPTER_SV, "OEM-användarportsspakadapter" },
/* tr */ { IDMS_OEM_USERPORT_JOY_ADAPTER_TR, "OEM userport joy dönüþtürücü" },

/* en */ { IDMS_HIT_USERPORT_JOY_ADAPTER,    "HIT userport joy adapter" },
/* da */ { IDMS_HIT_USERPORT_JOY_ADAPTER_DA, "HIT brugerportjoystickadapter" },
/* de */ { IDMS_HIT_USERPORT_JOY_ADAPTER_DE, "HIT Userport Joystick Adapter" },
/* es */ { IDMS_HIT_USERPORT_JOY_ADAPTER_ES, "Adaptador de puerto de juegos HIT" },
/* fr */ { IDMS_HIT_USERPORT_JOY_ADAPTER_FR, "Adaptateur joystick port utilisateur HIT" },
/* hu */ { IDMS_HIT_USERPORT_JOY_ADAPTER_HU, "HIT userport botkormány adapter" },
/* it */ { IDMS_HIT_USERPORT_JOY_ADAPTER_IT, "Adattatore joystick HIT su userport" },
/* ko */ { IDMS_HIT_USERPORT_JOY_ADAPTER_KO, "HIT À¯Á®Æ÷Æ® Á¶ÀÌ ¾Æ´äÅÍ" },
/* nl */ { IDMS_HIT_USERPORT_JOY_ADAPTER_NL, "HIT userport joy adapter" },
/* pl */ { IDMS_HIT_USERPORT_JOY_ADAPTER_PL, "Z³±cze HIT userportu joysticka" },
/* ru */ { IDMS_HIT_USERPORT_JOY_ADAPTER_RU, "HIT userport joy adapter" },
/* sv */ { IDMS_HIT_USERPORT_JOY_ADAPTER_SV, "HIT-användarportsspakadapter" },
/* tr */ { IDMS_HIT_USERPORT_JOY_ADAPTER_TR, "HIT userport joy dönüþtürücü" },

/* en */ { IDMS_KINGSOFT_USERPORT_JOY_ADAPTER,    "Kingsoft userport joy adapter" },
/* da */ { IDMS_KINGSOFT_USERPORT_JOY_ADAPTER_DA, "Kingsoft brugerportjoystickadapter" },
/* de */ { IDMS_KINGSOFT_USERPORT_JOY_ADAPTER_DE, "Kingsoft Userport Joystick Adapter" },
/* es */ { IDMS_KINGSOFT_USERPORT_JOY_ADAPTER_ES, "Adaptador puerto joy Kingsoft" },
/* fr */ { IDMS_KINGSOFT_USERPORT_JOY_ADAPTER_FR, "Adaptateur joystick port utilisateur Kingsoft" },
/* hu */ { IDMS_KINGSOFT_USERPORT_JOY_ADAPTER_HU, "" },  /* fuzzy */
/* it */ { IDMS_KINGSOFT_USERPORT_JOY_ADAPTER_IT, "Adattatore joystick Kingsoft su userport" },
/* ko */ { IDMS_KINGSOFT_USERPORT_JOY_ADAPTER_KO, "" },  /* fuzzy */
/* nl */ { IDMS_KINGSOFT_USERPORT_JOY_ADAPTER_NL, "Kingsoft userport joy adapter" },
/* pl */ { IDMS_KINGSOFT_USERPORT_JOY_ADAPTER_PL, "Z³±cze userportu joysticka Kingsoft" },
/* ru */ { IDMS_KINGSOFT_USERPORT_JOY_ADAPTER_RU, "" },  /* fuzzy */
/* sv */ { IDMS_KINGSOFT_USERPORT_JOY_ADAPTER_SV, "Kingsoft användarportsspakadapter" },
/* tr */ { IDMS_KINGSOFT_USERPORT_JOY_ADAPTER_TR, "Kingsoft userport joy dönüþtürücü" },

/* en */ { IDMS_STARBYTE_USERPORT_JOY_ADAPTER,    "Starbyte userport joy adapter" },
/* da */ { IDMS_STARBYTE_USERPORT_JOY_ADAPTER_DA, "Starbyte brugerportjoystickadapter" },
/* de */ { IDMS_STARBYTE_USERPORT_JOY_ADAPTER_DE, "Starbyte Userport Joystick Adapter" },
/* es */ { IDMS_STARBYTE_USERPORT_JOY_ADAPTER_ES, "Adaptador puerto joy Starbyte" },
/* fr */ { IDMS_STARBYTE_USERPORT_JOY_ADAPTER_FR, "Adaptateur joystick port utilisateur Starbyte" },
/* hu */ { IDMS_STARBYTE_USERPORT_JOY_ADAPTER_HU, "" },  /* fuzzy */
/* it */ { IDMS_STARBYTE_USERPORT_JOY_ADAPTER_IT, "Adattatore joystick Starbyte su userport " },
/* ko */ { IDMS_STARBYTE_USERPORT_JOY_ADAPTER_KO, "" },  /* fuzzy */
/* nl */ { IDMS_STARBYTE_USERPORT_JOY_ADAPTER_NL, "Starbyte userport joy adapter" },
/* pl */ { IDMS_STARBYTE_USERPORT_JOY_ADAPTER_PL, "Z³±cze userportu joysticka Starbyte" },
/* ru */ { IDMS_STARBYTE_USERPORT_JOY_ADAPTER_RU, "" },  /* fuzzy */
/* sv */ { IDMS_STARBYTE_USERPORT_JOY_ADAPTER_SV, "Starbyte användarportsspakadapter" },
/* tr */ { IDMS_STARBYTE_USERPORT_JOY_ADAPTER_TR, "Starbyte userport joy dönüþtürücü" },

/* en */ { IDMS_ENABLE_JOY_KEYS,    "Enable Joy Keys" },
/* da */ { IDMS_ENABLE_JOY_KEYS_DA, "Aktivér joystick via keyboard" },
/* de */ { IDMS_ENABLE_JOY_KEYS_DE, "Joystick Tasten aktivieren" },
/* es */ { IDMS_ENABLE_JOY_KEYS_ES, "Permitir Teclas en joy" },
/* fr */ { IDMS_ENABLE_JOY_KEYS_FR, "Activer les touches joystick" },
/* hu */ { IDMS_ENABLE_JOY_KEYS_HU, "Botkormány gombok engedélyezése" },
/* it */ { IDMS_ENABLE_JOY_KEYS_IT, "Attiva tasti joystick" },
/* ko */ { IDMS_ENABLE_JOY_KEYS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_ENABLE_JOY_KEYS_NL, "Activeer Joy Keys" },
/* pl */ { IDMS_ENABLE_JOY_KEYS_PL, "W³±cz klawisze joysticka" },
/* ru */ { IDMS_ENABLE_JOY_KEYS_RU, "Enable Joy Keys" },
/* sv */ { IDMS_ENABLE_JOY_KEYS_SV, "Aktivera styrspak via tangenter" },
/* tr */ { IDMS_ENABLE_JOY_KEYS_TR, "Joy Tuþlarýný Aktif Et" },

/* en */ { IDMS_OPPOSITE_JOY_DIR,    "Allow opposite directions" },
/* da */ { IDMS_OPPOSITE_JOY_DIR_DA, "Tillad modsatte retninger" },
/* de */ { IDMS_OPPOSITE_JOY_DIR_DE, "Gegenüberliegende Richtungen zulassen" },
/* es */ { IDMS_OPPOSITE_JOY_DIR_ES, "Permitir direcciones opuestas" },
/* fr */ { IDMS_OPPOSITE_JOY_DIR_FR, "Permettre des directions opposées simultanément" },
/* hu */ { IDMS_OPPOSITE_JOY_DIR_HU, "" },  /* fuzzy */
/* it */ { IDMS_OPPOSITE_JOY_DIR_IT, "Consenti direzioni opposte" },
/* ko */ { IDMS_OPPOSITE_JOY_DIR_KO, "¹Ý´ë ¹æÇâ Çã°¡ÇÏ±â" },
/* nl */ { IDMS_OPPOSITE_JOY_DIR_NL, "Tegenovergestelde richtingen toestaan" },
/* pl */ { IDMS_OPPOSITE_JOY_DIR_PL, "Pozwól na przeciwstawne kierunki" },
/* ru */ { IDMS_OPPOSITE_JOY_DIR_RU, "Allow opposite directions" },
/* sv */ { IDMS_OPPOSITE_JOY_DIR_SV, "Tillåt motsatta riktningar" },
/* tr */ { IDMS_OPPOSITE_JOY_DIR_TR, "Zýt yönlere izin ver" },

/* en */ { IDMS_SOUND_PLAYBACK,    "Sound playback" },
/* da */ { IDMS_SOUND_PLAYBACK_DA, "Slå lydemulering til" },
/* de */ { IDMS_SOUND_PLAYBACK_DE, "Sound Wiedergabe" },
/* es */ { IDMS_SOUND_PLAYBACK_ES, "Reproducción sonido" },
/* fr */ { IDMS_SOUND_PLAYBACK_FR, "Lecture audio" },
/* hu */ { IDMS_SOUND_PLAYBACK_HU, "Hangok engedélyezése" },
/* it */ { IDMS_SOUND_PLAYBACK_IT, "Riproduzione audio" },
/* ko */ { IDMS_SOUND_PLAYBACK_KO, "¼Ò¸® Àç»ý" },
/* nl */ { IDMS_SOUND_PLAYBACK_NL, "Geluid afspelen" },
/* pl */ { IDMS_SOUND_PLAYBACK_PL, "Odtwarzanie d¼wiêku" },
/* ru */ { IDMS_SOUND_PLAYBACK_RU, "Sound playback" },
/* sv */ { IDMS_SOUND_PLAYBACK_SV, "Aktivera ljudåtergivning" },
/* tr */ { IDMS_SOUND_PLAYBACK_TR, "Ses" },

/* en */ { IDMS_TRUE_DRIVE_EMU,    "True drive emulation" },
/* da */ { IDMS_TRUE_DRIVE_EMU_DA, "Ægte drev-emulering" },
/* de */ { IDMS_TRUE_DRIVE_EMU_DE, "Präzise Floppy Emulation" },
/* es */ { IDMS_TRUE_DRIVE_EMU_ES, "Emulación unidad disco verdadera" },
/* fr */ { IDMS_TRUE_DRIVE_EMU_FR, "Activer l'émulation réelle des lecteurs" },
/* hu */ { IDMS_TRUE_DRIVE_EMU_HU, "Valós lemezegység emuláció" },
/* it */ { IDMS_TRUE_DRIVE_EMU_IT, "Attiva emulazione hardware dei drive" },
/* ko */ { IDMS_TRUE_DRIVE_EMU_KO, "Æ®·ç µå¶óÀÌºê ¿¡¹Ä·¹ÀÌÅÍ" },
/* nl */ { IDMS_TRUE_DRIVE_EMU_NL, "Exacte driveemulatie" },
/* pl */ { IDMS_TRUE_DRIVE_EMU_PL, "Rzeczywista emulacja napêdu" },
/* ru */ { IDMS_TRUE_DRIVE_EMU_RU, "True drive emulation" },
/* sv */ { IDMS_TRUE_DRIVE_EMU_SV, "Äkta diskettenhetsemulering" },
/* tr */ { IDMS_TRUE_DRIVE_EMU_TR, "Gerçek sürücü emülasyonu" },

/* en */ { IDMS_DRIVE_SOUND_SETTINGS,    "Drive sound emulation settings" },
/* da */ { IDMS_DRIVE_SOUND_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDMS_DRIVE_SOUND_SETTINGS_DE, "Laufwerk Geräuschemulation Einstellungen" },
/* es */ { IDMS_DRIVE_SOUND_SETTINGS_ES, "Ajustes Emulación controlador de sonido" },
/* fr */ { IDMS_DRIVE_SOUND_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_DRIVE_SOUND_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_DRIVE_SOUND_SETTINGS_IT, "Impostazione emulazione suoni del drive" },
/* ko */ { IDMS_DRIVE_SOUND_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_DRIVE_SOUND_SETTINGS_NL, "Drive geluiden emulatie instellingen" },
/* pl */ { IDMS_DRIVE_SOUND_SETTINGS_PL, "Ustawienia emulacji d¼wiêków napêdu" },
/* ru */ { IDMS_DRIVE_SOUND_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_DRIVE_SOUND_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDMS_DRIVE_SOUND_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDMS_AUTOSTART_HANDLE_TDE,    "Handle True Drive Emulation on autostart" },
/* da */ { IDMS_AUTOSTART_HANDLE_TDE_DA, "Anvend ægte drev-emulering ved autostart" },
/* de */ { IDMS_AUTOSTART_HANDLE_TDE_DE, "Präzise Floppy Emulation bei Autostart beachten" },
/* es */ { IDMS_AUTOSTART_HANDLE_TDE_ES, "Manejar en autoarranque emulación de disco verdadera" },
/* fr */ { IDMS_AUTOSTART_HANDLE_TDE_FR, "Activer l'Émulation Réelle de Lecteur à l'autodémarrage" },
/* hu */ { IDMS_AUTOSTART_HANDLE_TDE_HU, "Valós lemezegység emuláció kezelése autostartnál" },
/* it */ { IDMS_AUTOSTART_HANDLE_TDE_IT, "Emulazione hardware dei drive all'avvio automatico" },
/* ko */ { IDMS_AUTOSTART_HANDLE_TDE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_AUTOSTART_HANDLE_TDE_NL, "Hanteer Hardwarematige Drive Emulatie bij autostarten" },
/* pl */ { IDMS_AUTOSTART_HANDLE_TDE_PL, "W³±cz rzeczywist± emulacjê napêdu przy autostarcie" },
/* ru */ { IDMS_AUTOSTART_HANDLE_TDE_RU, "Handle True Drive Emulation on autostart" },
/* sv */ { IDMS_AUTOSTART_HANDLE_TDE_SV, "Hantera äkta diskettenhetsemulering vid autostart" },
/* tr */ { IDMS_AUTOSTART_HANDLE_TDE_TR, "Otomatik baþlatmada Gerçek Sürücü Emülasyonu'nu denetle" },

/* en */ { IDMS_VIRTUAL_DEVICE_TRAPS,    "Virtual device traps" },
/* da */ { IDMS_VIRTUAL_DEVICE_TRAPS_DA, "Virtuelle enheds-traps" },
/* de */ { IDMS_VIRTUAL_DEVICE_TRAPS_DE, "Virtuelle Geräte Traps" },
/* es */ { IDMS_VIRTUAL_DEVICE_TRAPS_ES, "Interceptar periféricos virtuales" },
/* fr */ { IDMS_VIRTUAL_DEVICE_TRAPS_FR, "Activer les périphériques virtuels" },
/* hu */ { IDMS_VIRTUAL_DEVICE_TRAPS_HU, "Virtuális eszközök ciklus kihagyása" },
/* it */ { IDMS_VIRTUAL_DEVICE_TRAPS_IT, "Attiva periferiche virtuali" },
/* ko */ { IDMS_VIRTUAL_DEVICE_TRAPS_KO, "°¡»ó µð¹ÙÀÌ½º Æ®·¦" },
/* nl */ { IDMS_VIRTUAL_DEVICE_TRAPS_NL, "Virtuele apparaattraps" },
/* pl */ { IDMS_VIRTUAL_DEVICE_TRAPS_PL, "Traps urz±dzenia wirtualnego." },
/* ru */ { IDMS_VIRTUAL_DEVICE_TRAPS_RU, "Virtual device traps" },
/* sv */ { IDMS_VIRTUAL_DEVICE_TRAPS_SV, "Virtuella enhetstrap:ar" },
/* tr */ { IDMS_VIRTUAL_DEVICE_TRAPS_TR, "Sanal aygýt trapleri" },

/* en */ { IDMS_DRIVE_SYNC_FACTOR,    "Drive sync factor" },
/* da */ { IDMS_DRIVE_SYNC_FACTOR_DA, "Synkfaktor for diskettedrev" },
/* de */ { IDMS_DRIVE_SYNC_FACTOR_DE, "Laufwerkt Sync Faktor" },
/* es */ { IDMS_DRIVE_SYNC_FACTOR_ES, "Factor sincronización unidad disco" },
/* fr */ { IDMS_DRIVE_SYNC_FACTOR_FR, "Facteur de synchro du lecteur" },
/* hu */ { IDMS_DRIVE_SYNC_FACTOR_HU, "Lemezegység szinkron faktor" },
/* it */ { IDMS_DRIVE_SYNC_FACTOR_IT, "Fattore di sincronizzazione del drive" },
/* ko */ { IDMS_DRIVE_SYNC_FACTOR_KO, "" },  /* fuzzy */
/* nl */ { IDMS_DRIVE_SYNC_FACTOR_NL, "Drive synchronisatiefactor" },
/* pl */ { IDMS_DRIVE_SYNC_FACTOR_PL, "Czynnik synchronizacji stacji" },
/* ru */ { IDMS_DRIVE_SYNC_FACTOR_RU, "Drive sync factor" },
/* sv */ { IDMS_DRIVE_SYNC_FACTOR_SV, "Synkfaktor för diskettstation" },
/* tr */ { IDMS_DRIVE_SYNC_FACTOR_TR, "Sürücü senkron faktörü" },

/* en */ { IDMS_PAL,    "PAL" },
/* da */ { IDMS_PAL_DA, "PAL-G" },
/* de */ { IDMS_PAL_DE, "PAL" },
/* es */ { IDMS_PAL_ES, "PAL" },
/* fr */ { IDMS_PAL_FR, "PAL" },
/* hu */ { IDMS_PAL_HU, "" },  /* fuzzy */
/* it */ { IDMS_PAL_IT, "PAL" },
/* ko */ { IDMS_PAL_KO, "" },  /* fuzzy */
/* nl */ { IDMS_PAL_NL, "PAL" },
/* pl */ { IDMS_PAL_PL, "PAL" },
/* ru */ { IDMS_PAL_RU, "PAL" },
/* sv */ { IDMS_PAL_SV, "PAL" },
/* tr */ { IDMS_PAL_TR, "PAL" },

/* en */ { IDMS_NTSC,    "NTSC" },
/* da */ { IDMS_NTSC_DA, "NTSC-M" },
/* de */ { IDMS_NTSC_DE, "NTSC" },
/* es */ { IDMS_NTSC_ES, "NTSC" },
/* fr */ { IDMS_NTSC_FR, "NTSC" },
/* hu */ { IDMS_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDMS_NTSC_IT, "NTSC" },
/* ko */ { IDMS_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDMS_NTSC_NL, "NTSC" },
/* pl */ { IDMS_NTSC_PL, "NTSC" },
/* ru */ { IDMS_NTSC_RU, "NTSC" },
/* sv */ { IDMS_NTSC_SV, "NTSC-M" },
/* tr */ { IDMS_NTSC_TR, "NTSC" },

/* en */ { IDMS_VIDEO_STANDARD,    "Video standard" },
/* da */ { IDMS_VIDEO_STANDARD_DA, "TV-standard" },
/* de */ { IDMS_VIDEO_STANDARD_DE, "Video Standard" },
/* es */ { IDMS_VIDEO_STANDARD_ES, "Vídeo estandar" },
/* fr */ { IDMS_VIDEO_STANDARD_FR, "Standard vidéo" },
/* hu */ { IDMS_VIDEO_STANDARD_HU, "Videó szabvány" },
/* it */ { IDMS_VIDEO_STANDARD_IT, "Standard video" },
/* ko */ { IDMS_VIDEO_STANDARD_KO, "Ç¥ÁØ ºñµð¿À" },
/* nl */ { IDMS_VIDEO_STANDARD_NL, "Videostandaard" },
/* pl */ { IDMS_VIDEO_STANDARD_PL, "Standard wideo" },
/* ru */ { IDMS_VIDEO_STANDARD_RU, "²ØÔÕÞ áâÐÝÔÐàâ" },
/* sv */ { IDMS_VIDEO_STANDARD_SV, "TV-standard" },
/* tr */ { IDMS_VIDEO_STANDARD_TR, "Görüntü standardý" },

/* en */ { IDMS_PAL_G,    "PAL-G" },
/* da */ { IDMS_PAL_G_DA, "PAL-G" },
/* de */ { IDMS_PAL_G_DE, "PAL-G" },
/* es */ { IDMS_PAL_G_ES, "PAL-G" },
/* fr */ { IDMS_PAL_G_FR, "PAL-G" },
/* hu */ { IDMS_PAL_G_HU, "" },  /* fuzzy */
/* it */ { IDMS_PAL_G_IT, "PAL-G" },
/* ko */ { IDMS_PAL_G_KO, "" },  /* fuzzy */
/* nl */ { IDMS_PAL_G_NL, "PAL-G" },
/* pl */ { IDMS_PAL_G_PL, "PAL-G" },
/* ru */ { IDMS_PAL_G_RU, "PAL-G" },
/* sv */ { IDMS_PAL_G_SV, "PAL-G" },
/* tr */ { IDMS_PAL_G_TR, "PAL-G" },

/* en */ { IDMS_NTSC_M,    "NTSC-M" },
/* da */ { IDMS_NTSC_M_DA, "NTSC-M" },
/* de */ { IDMS_NTSC_M_DE, "NTSC-M" },
/* es */ { IDMS_NTSC_M_ES, "NTSC-M" },
/* fr */ { IDMS_NTSC_M_FR, "NTSC-M" },
/* hu */ { IDMS_NTSC_M_HU, "" },  /* fuzzy */
/* it */ { IDMS_NTSC_M_IT, "NTSC-M" },
/* ko */ { IDMS_NTSC_M_KO, "" },  /* fuzzy */
/* nl */ { IDMS_NTSC_M_NL, "NTSC-M" },
/* pl */ { IDMS_NTSC_M_PL, "NTSC-M" },
/* ru */ { IDMS_NTSC_M_RU, "NTSC-M" },
/* sv */ { IDMS_NTSC_M_SV, "NTSC-M" },
/* tr */ { IDMS_NTSC_M_TR, "NTSC-M" },

/* en */ { IDMS_OLD_NTSC_M,    "Old NTSC-M" },
/* da */ { IDMS_OLD_NTSC_M_DA, "Gammel NTSC-M" },
/* de */ { IDMS_OLD_NTSC_M_DE, "NTSC-M alt" },
/* es */ { IDMS_OLD_NTSC_M_ES, "Antiguo NTSC-M" },
/* fr */ { IDMS_OLD_NTSC_M_FR, "Ancien NTSC-M" },
/* hu */ { IDMS_OLD_NTSC_M_HU, "Régi NTSC-M" },
/* it */ { IDMS_OLD_NTSC_M_IT, "NTSC-M vecchio" },
/* ko */ { IDMS_OLD_NTSC_M_KO, "±¸Çü NTSC-M" },
/* nl */ { IDMS_OLD_NTSC_M_NL, "Oude NTSC-M" },
/* pl */ { IDMS_OLD_NTSC_M_PL, "Stary NTSC-M" },
/* ru */ { IDMS_OLD_NTSC_M_RU, "Old NTSC-M" },
/* sv */ { IDMS_OLD_NTSC_M_SV, "Gammal NTSC-M" },
/* tr */ { IDMS_OLD_NTSC_M_TR, "Eski NTSC-M" },

/* en */ { IDMS_VIC_1112_IEEE_488,    "VIC-1112 IEEE-488 module" },
/* da */ { IDMS_VIC_1112_IEEE_488_DA, "VIC-1112 IEEE-488-modul" },
/* de */ { IDMS_VIC_1112_IEEE_488_DE, "VIC-1112 IEEE-488 Modul" },
/* es */ { IDMS_VIC_1112_IEEE_488_ES, "Módulo VIC-1112 IEEE-488" },
/* fr */ { IDMS_VIC_1112_IEEE_488_FR, "Module VIC-1112 IEEE 488" },
/* hu */ { IDMS_VIC_1112_IEEE_488_HU, "" },  /* fuzzy */
/* it */ { IDMS_VIC_1112_IEEE_488_IT, "Modulo VIC-1112 IEEE-488" },
/* ko */ { IDMS_VIC_1112_IEEE_488_KO, "" },  /* fuzzy */
/* nl */ { IDMS_VIC_1112_IEEE_488_NL, "VIC-1112 IEEE-488 module" },
/* pl */ { IDMS_VIC_1112_IEEE_488_PL, "Modu³ IEEE-488 VIC-1112" },
/* ru */ { IDMS_VIC_1112_IEEE_488_RU, "VIC-1112 IEEE-488 module" },
/* sv */ { IDMS_VIC_1112_IEEE_488_SV, "VIC-1112 IEEE-488-modul" },
/* tr */ { IDMS_VIC_1112_IEEE_488_TR, "VIC-1112 IEEE-488 modülü" },

/* en */ { IDMS_IEEE_488_INTERFACE,    "IEEE488 Interface emulation" },
/* da */ { IDMS_IEEE_488_INTERFACE_DA, "Emulering af IEEE488-interface" },
/* de */ { IDMS_IEEE_488_INTERFACE_DE, "IEEE488 Schnittstellen-Emulation" },
/* es */ { IDMS_IEEE_488_INTERFACE_ES, "Emulación de interface IEEE488" },
/* fr */ { IDMS_IEEE_488_INTERFACE_FR, "Interface d'émulation IEEE488" },
/* hu */ { IDMS_IEEE_488_INTERFACE_HU, "" },  /* fuzzy */
/* it */ { IDMS_IEEE_488_INTERFACE_IT, "Emulazione interfaccia IEEE488" },
/* ko */ { IDMS_IEEE_488_INTERFACE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_IEEE_488_INTERFACE_NL, "IEEE488 Interface emulatie" },
/* pl */ { IDMS_IEEE_488_INTERFACE_PL, "Emulacja interfejsu IEEE488" },
/* ru */ { IDMS_IEEE_488_INTERFACE_RU, "" },  /* fuzzy */
/* sv */ { IDMS_IEEE_488_INTERFACE_SV, "Emulering av IEEE488-gränssnitt" },
/* tr */ { IDMS_IEEE_488_INTERFACE_TR, "IEEE488 Arabirimi emülasyonu" },

/* en */ { IDMS_GRAB_MOUSE,    "Grab mouse events" },
/* da */ { IDMS_GRAB_MOUSE_DA, "Fang mussehændelser" },
/* de */ { IDMS_GRAB_MOUSE_DE, "Maus Ereignisse abfangen" },
/* es */ { IDMS_GRAB_MOUSE_ES, "Capturar eventos del ratón" },
/* fr */ { IDMS_GRAB_MOUSE_FR, "Attraper les événements souris" },
/* hu */ { IDMS_GRAB_MOUSE_HU, "Egér események elkapása" },
/* it */ { IDMS_GRAB_MOUSE_IT, "Cattura eventi mouse" },
/* ko */ { IDMS_GRAB_MOUSE_KO, "±×·¦ ¸¶¿ì½º ÀÌº¥Æ®" },
/* nl */ { IDMS_GRAB_MOUSE_NL, "Gebruik de muis" },
/* pl */ { IDMS_GRAB_MOUSE_PL, "Przechwytuj zdarzenia myszy" },
/* ru */ { IDMS_GRAB_MOUSE_RU, "Grab mouse events" },
/* sv */ { IDMS_GRAB_MOUSE_SV, "Fånga mushändelser" },
/* tr */ { IDMS_GRAB_MOUSE_TR, "Mouse olaylarýný yakala" },

/* en */ { IDMS_PS2_MOUSE,    "PS/2 mouse on Userport" },
/* da */ { IDMS_PS2_MOUSE_DA, "PS/2-mus på brugerporten" },
/* de */ { IDMS_PS2_MOUSE_DE, "PS/2 Userport Mausemulation" },
/* es */ { IDMS_PS2_MOUSE_ES, "Ratón PS/2 en puerto usuario" },
/* fr */ { IDMS_PS2_MOUSE_FR, "Souris PS/2 sur port utilisateur" },
/* hu */ { IDMS_PS2_MOUSE_HU, "PS/2 egér a userporton" },
/* it */ { IDMS_PS2_MOUSE_IT, "Mouse PS/2 su userport" },
/* ko */ { IDMS_PS2_MOUSE_KO, "À¯ÀúÆ÷Æ®¿¡ PS/S ¸¶¿ì½º " },
/* nl */ { IDMS_PS2_MOUSE_NL, "Aktiveer emulatie van een PS/2 muis op de userport" },
/* pl */ { IDMS_PS2_MOUSE_PL, "Mysz PS/2 w userporcie" },
/* ru */ { IDMS_PS2_MOUSE_RU, "PS/2 mouse on Userport" },
/* sv */ { IDMS_PS2_MOUSE_SV, "PS/2-mus på användarport" },
/* tr */ { IDMS_PS2_MOUSE_TR, "Userport üzerinde PS/2 mouse" },

/* en */ { IDMS_SETTINGS,    "Settings" },
/* da */ { IDMS_SETTINGS_DA, "Indstillinger" },
/* de */ { IDMS_SETTINGS_DE, "Einstellungen" },
/* es */ { IDMS_SETTINGS_ES, "Ajustes" },
/* fr */ { IDMS_SETTINGS_FR, "Paramètres" },
/* hu */ { IDMS_SETTINGS_HU, "Beállítások" },
/* it */ { IDMS_SETTINGS_IT, "Impostazioni" },
/* ko */ { IDMS_SETTINGS_KO, "¼³Á¤" },
/* nl */ { IDMS_SETTINGS_NL, "Instellingen" },
/* pl */ { IDMS_SETTINGS_PL, "Ustawienia" },
/* ru */ { IDMS_SETTINGS_RU, "½ÐáâàÞÙÚØ" },
/* sv */ { IDMS_SETTINGS_SV, "Inställningar" },
/* tr */ { IDMS_SETTINGS_TR, "Ayarlar" },

/* en */ { IDMS_C64_MODEL_SETTINGS,    "C64 model settings..." },
/* da */ { IDMS_C64_MODEL_SETTINGS_DA, "C64-model indstillinger..." },
/* de */ { IDMS_C64_MODEL_SETTINGS_DE, "C64 Modell Einstellungen..." },
/* es */ { IDMS_C64_MODEL_SETTINGS_ES, "Ajustes modelo C64..." },
/* fr */ { IDMS_C64_MODEL_SETTINGS_FR, "Paramètres du modèle C64..." },
/* hu */ { IDMS_C64_MODEL_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_C64_MODEL_SETTINGS_IT, "Impostazioni modello C64..." },
/* ko */ { IDMS_C64_MODEL_SETTINGS_KO, "C64 ¸ðµ¨ ¼ÂÆÃ..." },
/* nl */ { IDMS_C64_MODEL_SETTINGS_NL, "C64 model instellingen..." },
/* pl */ { IDMS_C64_MODEL_SETTINGS_PL, "Ustawienia modelu C64..." },
/* ru */ { IDMS_C64_MODEL_SETTINGS_RU, "C64 model settings..." },
/* sv */ { IDMS_C64_MODEL_SETTINGS_SV, "C64-modellinställningar..." },
/* tr */ { IDMS_C64_MODEL_SETTINGS_TR, "C64 model ayarlarý..." },

/* en */ { IDMS_C64_PAL,    "C64 PAL" },
/* da */ { IDMS_C64_PAL_DA, "C64 PAL" },
/* de */ { IDMS_C64_PAL_DE, "C64 PAL" },
/* es */ { IDMS_C64_PAL_ES, "C64 PAL" },
/* fr */ { IDMS_C64_PAL_FR, "RTC baseC64 PAL" },
/* hu */ { IDMS_C64_PAL_HU, "" },  /* fuzzy */
/* it */ { IDMS_C64_PAL_IT, "C64 PAL" },
/* ko */ { IDMS_C64_PAL_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C64_PAL_NL, "C64 PAL" },
/* pl */ { IDMS_C64_PAL_PL, "C64 PAL" },
/* ru */ { IDMS_C64_PAL_RU, "C64 PAL" },
/* sv */ { IDMS_C64_PAL_SV, "C64 PAL" },
/* tr */ { IDMS_C64_PAL_TR, "C64 PAL" },

/* en */ { IDMS_C64C_PAL,    "C64C PAL" },
/* da */ { IDMS_C64C_PAL_DA, "C64C PAL" },
/* de */ { IDMS_C64C_PAL_DE, "C64C PAL" },
/* es */ { IDMS_C64C_PAL_ES, "C64C PAL" },
/* fr */ { IDMS_C64C_PAL_FR, "C64C PAL" },
/* hu */ { IDMS_C64C_PAL_HU, "" },  /* fuzzy */
/* it */ { IDMS_C64C_PAL_IT, "C64C PAL" },
/* ko */ { IDMS_C64C_PAL_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C64C_PAL_NL, "C64C PAL" },
/* pl */ { IDMS_C64C_PAL_PL, "C64C PAL" },
/* ru */ { IDMS_C64C_PAL_RU, "C64C PAL" },
/* sv */ { IDMS_C64C_PAL_SV, "C64C PAL" },
/* tr */ { IDMS_C64C_PAL_TR, "C64C PAL" },

/* en */ { IDMS_C64_OLD_PAL,    "C64 old PAL" },
/* da */ { IDMS_C64_OLD_PAL_DA, "C64 gammel PAL" },
/* de */ { IDMS_C64_OLD_PAL_DE, "C64 alt PAL" },
/* es */ { IDMS_C64_OLD_PAL_ES, "C64 PAL antiguo" },
/* fr */ { IDMS_C64_OLD_PAL_FR, "Ancien PAL C64" },
/* hu */ { IDMS_C64_OLD_PAL_HU, "" },  /* fuzzy */
/* it */ { IDMS_C64_OLD_PAL_IT, "C64 PAL vecchio" },
/* ko */ { IDMS_C64_OLD_PAL_KO, "C64 ±¸Çü PAL" },
/* nl */ { IDMS_C64_OLD_PAL_NL, "C64 oude PAL" },
/* pl */ { IDMS_C64_OLD_PAL_PL, "C64 stary PAL" },
/* ru */ { IDMS_C64_OLD_PAL_RU, "" },  /* fuzzy */
/* sv */ { IDMS_C64_OLD_PAL_SV, "C64 gammal PAL" },
/* tr */ { IDMS_C64_OLD_PAL_TR, "C64 eski PAL" },

/* en */ { IDMS_C64_NTSC,    "C64 NTSC" },
/* da */ { IDMS_C64_NTSC_DA, "C64 NTSC" },
/* de */ { IDMS_C64_NTSC_DE, "C64 NTSC" },
/* es */ { IDMS_C64_NTSC_ES, "C64 NTSC" },
/* fr */ { IDMS_C64_NTSC_FR, "NTSC C64" },
/* hu */ { IDMS_C64_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDMS_C64_NTSC_IT, "C64 NTSC" },
/* ko */ { IDMS_C64_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C64_NTSC_NL, "C64 NTSC" },
/* pl */ { IDMS_C64_NTSC_PL, "C64 NTSC" },
/* ru */ { IDMS_C64_NTSC_RU, "C64 NTSC" },
/* sv */ { IDMS_C64_NTSC_SV, "C64 NTSC" },
/* tr */ { IDMS_C64_NTSC_TR, "C64 NTSC" },

/* en */ { IDMS_C64C_NTSC,    "C64C NTSC" },
/* da */ { IDMS_C64C_NTSC_DA, "C64C NTSC" },
/* de */ { IDMS_C64C_NTSC_DE, "C64C NTSC" },
/* es */ { IDMS_C64C_NTSC_ES, "C64C NTSC" },
/* fr */ { IDMS_C64C_NTSC_FR, "NTSC C64C" },
/* hu */ { IDMS_C64C_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDMS_C64C_NTSC_IT, "C64C NTSC" },
/* ko */ { IDMS_C64C_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C64C_NTSC_NL, "C64C NTSC" },
/* pl */ { IDMS_C64C_NTSC_PL, "C64C NTSC" },
/* ru */ { IDMS_C64C_NTSC_RU, "C64C NTSC" },
/* sv */ { IDMS_C64C_NTSC_SV, "C64C NTSC" },
/* tr */ { IDMS_C64C_NTSC_TR, "C64C NTSC" },

/* en */ { IDMS_C64_OLD_NTSC,    "C64 old NTSC" },
/* da */ { IDMS_C64_OLD_NTSC_DA, "C64 gammel NTSC" },
/* de */ { IDMS_C64_OLD_NTSC_DE, "C64 alt NTSC" },
/* es */ { IDMS_C64_OLD_NTSC_ES, "C64 NTSC antiguo" },
/* fr */ { IDMS_C64_OLD_NTSC_FR, "Ancien NTSC C64" },
/* hu */ { IDMS_C64_OLD_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDMS_C64_OLD_NTSC_IT, "C64 NTSC vecchio" },
/* ko */ { IDMS_C64_OLD_NTSC_KO, "C64 ±¸Çü NTSC" },
/* nl */ { IDMS_C64_OLD_NTSC_NL, "C64 oude NTSC" },
/* pl */ { IDMS_C64_OLD_NTSC_PL, "C64 stary NTSC" },
/* ru */ { IDMS_C64_OLD_NTSC_RU, "C64 old NTSC" },
/* sv */ { IDMS_C64_OLD_NTSC_SV, "C64 gammal NTSC" },
/* tr */ { IDMS_C64_OLD_NTSC_TR, "C64 eski NTSC" },

/* en */ { IDMS_DREAN,    "Drean" },
/* da */ { IDMS_DREAN_DA, "Drean" },
/* de */ { IDMS_DREAN_DE, "Drean" },
/* es */ { IDMS_DREAN_ES, "Drean" },
/* fr */ { IDMS_DREAN_FR, "Rêve" },
/* hu */ { IDMS_DREAN_HU, "" },  /* fuzzy */
/* it */ { IDMS_DREAN_IT, "Drean" },
/* ko */ { IDMS_DREAN_KO, "µå¸°" },
/* nl */ { IDMS_DREAN_NL, "Drean" },
/* pl */ { IDMS_DREAN_PL, "Drean" },
/* ru */ { IDMS_DREAN_RU, "Drean" },
/* sv */ { IDMS_DREAN_SV, "Drean" },
/* tr */ { IDMS_DREAN_TR, "Drean" },

/* en */ { IDMS_C64SX_PAL,    "C64 SX PAL" },
/* da */ { IDMS_C64SX_PAL_DA, "" },  /* fuzzy */
/* de */ { IDMS_C64SX_PAL_DE, "C64 SX PAL" },
/* es */ { IDMS_C64SX_PAL_ES, "C64 SX PAL" },
/* fr */ { IDMS_C64SX_PAL_FR, "" },  /* fuzzy */
/* hu */ { IDMS_C64SX_PAL_HU, "" },  /* fuzzy */
/* it */ { IDMS_C64SX_PAL_IT, "C64 SX PAL" },
/* ko */ { IDMS_C64SX_PAL_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C64SX_PAL_NL, "C64 SX PAL" },
/* pl */ { IDMS_C64SX_PAL_PL, "C64 SX PAL" },
/* ru */ { IDMS_C64SX_PAL_RU, "" },  /* fuzzy */
/* sv */ { IDMS_C64SX_PAL_SV, "C64 SX PAL" },
/* tr */ { IDMS_C64SX_PAL_TR, "" },  /* fuzzy */

/* en */ { IDMS_C64SX_NTSC,    "C64 SX NTSC" },
/* da */ { IDMS_C64SX_NTSC_DA, "" },  /* fuzzy */
/* de */ { IDMS_C64SX_NTSC_DE, "C64 SX NTSC" },
/* es */ { IDMS_C64SX_NTSC_ES, "C64 SX NTSC" },
/* fr */ { IDMS_C64SX_NTSC_FR, "" },  /* fuzzy */
/* hu */ { IDMS_C64SX_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDMS_C64SX_NTSC_IT, "C64 SX NTSC" },
/* ko */ { IDMS_C64SX_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C64SX_NTSC_NL, "C64 SX NTSC" },
/* pl */ { IDMS_C64SX_NTSC_PL, "C64 SX NTSC" },
/* ru */ { IDMS_C64SX_NTSC_RU, "" },  /* fuzzy */
/* sv */ { IDMS_C64SX_NTSC_SV, "C64 SX NTSC" },
/* tr */ { IDMS_C64SX_NTSC_TR, "" },  /* fuzzy */

/* en */ { IDMS_C64_JAP,    "Japanese" },
/* da */ { IDMS_C64_JAP_DA, "" },  /* fuzzy */
/* de */ { IDMS_C64_JAP_DE, "Japanisch" },
/* es */ { IDMS_C64_JAP_ES, "Japonés" },
/* fr */ { IDMS_C64_JAP_FR, "" },  /* fuzzy */
/* hu */ { IDMS_C64_JAP_HU, "" },  /* fuzzy */
/* it */ { IDMS_C64_JAP_IT, "Giapponese" },
/* ko */ { IDMS_C64_JAP_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C64_JAP_NL, "Japans" },
/* pl */ { IDMS_C64_JAP_PL, "Japoñski" },
/* ru */ { IDMS_C64_JAP_RU, "" },  /* fuzzy */
/* sv */ { IDMS_C64_JAP_SV, "japansk" },
/* tr */ { IDMS_C64_JAP_TR, "" },  /* fuzzy */

/* en */ { IDMS_C64_GS,    "C64 GS" },
/* da */ { IDMS_C64_GS_DA, "" },  /* fuzzy */
/* de */ { IDMS_C64_GS_DE, "C64 GS" },
/* es */ { IDMS_C64_GS_ES, "C64 GS" },
/* fr */ { IDMS_C64_GS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_C64_GS_HU, "" },  /* fuzzy */
/* it */ { IDMS_C64_GS_IT, "C64 GS" },
/* ko */ { IDMS_C64_GS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C64_GS_NL, "C64 GS" },
/* pl */ { IDMS_C64_GS_PL, "C64 GS" },
/* ru */ { IDMS_C64_GS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_C64_GS_SV, "C64 GS" },
/* tr */ { IDMS_C64_GS_TR, "" },  /* fuzzy */

/* en */ { IDMS_PET64_PAL,    "PET64 PAL" },
/* da */ { IDMS_PET64_PAL_DA, "" },  /* fuzzy */
/* de */ { IDMS_PET64_PAL_DE, "PET64 PAL" },
/* es */ { IDMS_PET64_PAL_ES, "PET64 PAL" },
/* fr */ { IDMS_PET64_PAL_FR, "" },  /* fuzzy */
/* hu */ { IDMS_PET64_PAL_HU, "" },  /* fuzzy */
/* it */ { IDMS_PET64_PAL_IT, "PET64 PAL" },
/* ko */ { IDMS_PET64_PAL_KO, "" },  /* fuzzy */
/* nl */ { IDMS_PET64_PAL_NL, "PET64 PAL" },
/* pl */ { IDMS_PET64_PAL_PL, "PET64 PAL" },
/* ru */ { IDMS_PET64_PAL_RU, "" },  /* fuzzy */
/* sv */ { IDMS_PET64_PAL_SV, "PET64 PAL" },
/* tr */ { IDMS_PET64_PAL_TR, "" },  /* fuzzy */

/* en */ { IDMS_PET64_NTSC,    "PET64 NTSC" },
/* da */ { IDMS_PET64_NTSC_DA, "" },  /* fuzzy */
/* de */ { IDMS_PET64_NTSC_DE, "PET64 NTSC" },
/* es */ { IDMS_PET64_NTSC_ES, "PET64 NTSC" },
/* fr */ { IDMS_PET64_NTSC_FR, "" },  /* fuzzy */
/* hu */ { IDMS_PET64_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDMS_PET64_NTSC_IT, "PET64 NTSC" },
/* ko */ { IDMS_PET64_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDMS_PET64_NTSC_NL, "PET64 NTSC" },
/* pl */ { IDMS_PET64_NTSC_PL, "PET64 NTSC" },
/* ru */ { IDMS_PET64_NTSC_RU, "" },  /* fuzzy */
/* sv */ { IDMS_PET64_NTSC_SV, "PET64 NTSC" },
/* tr */ { IDMS_PET64_NTSC_TR, "" },  /* fuzzy */

/* en */ { IDMS_ULTIMAX,    "MAX Machine" },
/* da */ { IDMS_ULTIMAX_DA, "" },  /* fuzzy */
/* de */ { IDMS_ULTIMAX_DE, "MAX Machine" },
/* es */ { IDMS_ULTIMAX_ES, "Máquina MAX" },
/* fr */ { IDMS_ULTIMAX_FR, "" },  /* fuzzy */
/* hu */ { IDMS_ULTIMAX_HU, "" },  /* fuzzy */
/* it */ { IDMS_ULTIMAX_IT, "MAX Machine" },
/* ko */ { IDMS_ULTIMAX_KO, "" },  /* fuzzy */
/* nl */ { IDMS_ULTIMAX_NL, "MAX Machine" },
/* pl */ { IDMS_ULTIMAX_PL, "MAX Machine" },
/* ru */ { IDMS_ULTIMAX_RU, "" },  /* fuzzy */
/* sv */ { IDMS_ULTIMAX_SV, "MAX-maskin" },
/* tr */ { IDMS_ULTIMAX_TR, "" },  /* fuzzy */

/* en */ { IDMS_CUSTOM_C64_MODEL,    "Custom C64 model" },
/* da */ { IDMS_CUSTOM_C64_MODEL_DA, "Speciel C64 model" },
/* de */ { IDMS_CUSTOM_C64_MODEL_DE, "Modifiziertes C64 Modell" },
/* es */ { IDMS_CUSTOM_C64_MODEL_ES, "Modelo C64 a medida" },
/* fr */ { IDMS_CUSTOM_C64_MODEL_FR, "Modèle personnalisé C64" },
/* hu */ { IDMS_CUSTOM_C64_MODEL_HU, "" },  /* fuzzy */
/* it */ { IDMS_CUSTOM_C64_MODEL_IT, "Modello C64 personalizzato" },
/* ko */ { IDMS_CUSTOM_C64_MODEL_KO, "" },  /* fuzzy */
/* nl */ { IDMS_CUSTOM_C64_MODEL_NL, "Aangepast C64 model" },
/* pl */ { IDMS_CUSTOM_C64_MODEL_PL, "W³asny model C64" },
/* ru */ { IDMS_CUSTOM_C64_MODEL_RU, "" },  /* fuzzy */
/* sv */ { IDMS_CUSTOM_C64_MODEL_SV, "Speciell C64-modell" },
/* tr */ { IDMS_CUSTOM_C64_MODEL_TR, "Özel C64 modeli" },

/* en */ { IDMS_PLUS4_MODEL_SETTINGS,    "Plus4 model settings..." },
/* da */ { IDMS_PLUS4_MODEL_SETTINGS_DA, "Plus4-model indstillinger..." },
/* de */ { IDMS_PLUS4_MODEL_SETTINGS_DE, "Plus4 Modell Einstellungen..." },
/* es */ { IDMS_PLUS4_MODEL_SETTINGS_ES, "Ajustes modelo Plus4..." },
/* fr */ { IDMS_PLUS4_MODEL_SETTINGS_FR, "Paramètres du modèle Plus4..." },
/* hu */ { IDMS_PLUS4_MODEL_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_PLUS4_MODEL_SETTINGS_IT, "Impostazioni modello Plus4..." },
/* ko */ { IDMS_PLUS4_MODEL_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_PLUS4_MODEL_SETTINGS_NL, "Plus4 model instellingen..." },
/* pl */ { IDMS_PLUS4_MODEL_SETTINGS_PL, "Ustawienia modelu Plus4..." },
/* ru */ { IDMS_PLUS4_MODEL_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_PLUS4_MODEL_SETTINGS_SV, "Plus4-modellinställningar..." },
/* tr */ { IDMS_PLUS4_MODEL_SETTINGS_TR, "Plus4 model ayarlarý..." },

/* en */ { IDMS_C16_PAL,    "C16 PAL" },
/* da */ { IDMS_C16_PAL_DA, "C16 PAL" },
/* de */ { IDMS_C16_PAL_DE, "C16 PAL" },
/* es */ { IDMS_C16_PAL_ES, "C16 PAL" },
/* fr */ { IDMS_C16_PAL_FR, "PAL C16" },
/* hu */ { IDMS_C16_PAL_HU, "" },  /* fuzzy */
/* it */ { IDMS_C16_PAL_IT, "C16 PAL" },
/* ko */ { IDMS_C16_PAL_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C16_PAL_NL, "C16 PAL" },
/* pl */ { IDMS_C16_PAL_PL, "C16 PAL" },
/* ru */ { IDMS_C16_PAL_RU, "" },  /* fuzzy */
/* sv */ { IDMS_C16_PAL_SV, "C16 PAL" },
/* tr */ { IDMS_C16_PAL_TR, "C16 PAL" },

/* en */ { IDMS_C16_NTSC,    "C16 NTSC" },
/* da */ { IDMS_C16_NTSC_DA, "C16 NTSC" },
/* de */ { IDMS_C16_NTSC_DE, "C16 NTSC" },
/* es */ { IDMS_C16_NTSC_ES, "C16 NTSC" },
/* fr */ { IDMS_C16_NTSC_FR, "NTSC C16" },
/* hu */ { IDMS_C16_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDMS_C16_NTSC_IT, "C16 NTSC" },
/* ko */ { IDMS_C16_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C16_NTSC_NL, "C16 NTSC" },
/* pl */ { IDMS_C16_NTSC_PL, "C16 NTSC" },
/* ru */ { IDMS_C16_NTSC_RU, "" },  /* fuzzy */
/* sv */ { IDMS_C16_NTSC_SV, "C16 NTSC" },
/* tr */ { IDMS_C16_NTSC_TR, "C16 NTSC" },

/* en */ { IDMS_PLUS4_PAL,    "Plus4 PAL" },
/* da */ { IDMS_PLUS4_PAL_DA, "Plus4 PAL" },
/* de */ { IDMS_PLUS4_PAL_DE, "Plus4 PAL" },
/* es */ { IDMS_PLUS4_PAL_ES, "Plus4 PAL" },
/* fr */ { IDMS_PLUS4_PAL_FR, "Plus4 PAL" },
/* hu */ { IDMS_PLUS4_PAL_HU, "" },  /* fuzzy */
/* it */ { IDMS_PLUS4_PAL_IT, "Plus4 PAL" },
/* ko */ { IDMS_PLUS4_PAL_KO, "" },  /* fuzzy */
/* nl */ { IDMS_PLUS4_PAL_NL, "Plus4 PAL" },
/* pl */ { IDMS_PLUS4_PAL_PL, "Plus4 PAL" },
/* ru */ { IDMS_PLUS4_PAL_RU, "" },  /* fuzzy */
/* sv */ { IDMS_PLUS4_PAL_SV, "Plus4 PAL" },
/* tr */ { IDMS_PLUS4_PAL_TR, "Plus4 PAL" },

/* en */ { IDMS_PLUS4_NTSC,    "Plus4 NTSC" },
/* da */ { IDMS_PLUS4_NTSC_DA, "Plus4 NTSC" },
/* de */ { IDMS_PLUS4_NTSC_DE, "Plus4 NTSC" },
/* es */ { IDMS_PLUS4_NTSC_ES, "Plus4 NTSC" },
/* fr */ { IDMS_PLUS4_NTSC_FR, "NTSC Plus4" },
/* hu */ { IDMS_PLUS4_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDMS_PLUS4_NTSC_IT, "Plus4 NTSC" },
/* ko */ { IDMS_PLUS4_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDMS_PLUS4_NTSC_NL, "Plus4 NTSC" },
/* pl */ { IDMS_PLUS4_NTSC_PL, "Plus4 NTSC" },
/* ru */ { IDMS_PLUS4_NTSC_RU, "" },  /* fuzzy */
/* sv */ { IDMS_PLUS4_NTSC_SV, "Plus4 NTSC" },
/* tr */ { IDMS_PLUS4_NTSC_TR, "Plus4 NTSC" },

/* en */ { IDMS_V364_NTSC,    "V364 NTSC" },
/* da */ { IDMS_V364_NTSC_DA, "V364 NTSC" },
/* de */ { IDMS_V364_NTSC_DE, "V364 NTSC" },
/* es */ { IDMS_V364_NTSC_ES, "V364 NTSC" },
/* fr */ { IDMS_V364_NTSC_FR, "NTSC-M V364" },
/* hu */ { IDMS_V364_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDMS_V364_NTSC_IT, "V364 NTSC" },
/* ko */ { IDMS_V364_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDMS_V364_NTSC_NL, "V364 NTSC" },
/* pl */ { IDMS_V364_NTSC_PL, "V364 NTSC" },
/* ru */ { IDMS_V364_NTSC_RU, "" },  /* fuzzy */
/* sv */ { IDMS_V364_NTSC_SV, "V364 NTSC" },
/* tr */ { IDMS_V364_NTSC_TR, "V364 NTSC" },

/* en */ { IDMS_C232_NTSC,    "C232 NTSC" },
/* da */ { IDMS_C232_NTSC_DA, "C232 NTSC" },
/* de */ { IDMS_C232_NTSC_DE, "C232 NTSC" },
/* es */ { IDMS_C232_NTSC_ES, "C232 NTSC" },
/* fr */ { IDMS_C232_NTSC_FR, "NTSC C232" },
/* hu */ { IDMS_C232_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDMS_C232_NTSC_IT, "C232 NTSC" },
/* ko */ { IDMS_C232_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C232_NTSC_NL, "C232 NTSC" },
/* pl */ { IDMS_C232_NTSC_PL, "C232 NTSC" },
/* ru */ { IDMS_C232_NTSC_RU, "" },  /* fuzzy */
/* sv */ { IDMS_C232_NTSC_SV, "C232 NTSC" },
/* tr */ { IDMS_C232_NTSC_TR, "C232 NTSC" },

/* en */ { IDMS_VIC20_MODEL_SETTINGS,    "VIC20 model settings..." },
/* da */ { IDMS_VIC20_MODEL_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDMS_VIC20_MODEL_SETTINGS_DE, "VIC20 Modell Einstellungen..." },
/* es */ { IDMS_VIC20_MODEL_SETTINGS_ES, "Ajustes modelo VIC20..." },
/* fr */ { IDMS_VIC20_MODEL_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_VIC20_MODEL_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_VIC20_MODEL_SETTINGS_IT, "Impostazioni modello VIC20..." },
/* ko */ { IDMS_VIC20_MODEL_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_VIC20_MODEL_SETTINGS_NL, "VIC20 model instellingen..." },
/* pl */ { IDMS_VIC20_MODEL_SETTINGS_PL, "Ustawienia modelu VIC20..." },
/* ru */ { IDMS_VIC20_MODEL_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_VIC20_MODEL_SETTINGS_SV, "VIC20-modellinställningar..." },
/* tr */ { IDMS_VIC20_MODEL_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDMS_VIC20_PAL,    "VIC20 PAL" },
/* da */ { IDMS_VIC20_PAL_DA, "" },  /* fuzzy */
/* de */ { IDMS_VIC20_PAL_DE, "VIC20 PAL" },
/* es */ { IDMS_VIC20_PAL_ES, "VIC20 PAL" },
/* fr */ { IDMS_VIC20_PAL_FR, "" },  /* fuzzy */
/* hu */ { IDMS_VIC20_PAL_HU, "" },  /* fuzzy */
/* it */ { IDMS_VIC20_PAL_IT, "VIC20 PAL" },
/* ko */ { IDMS_VIC20_PAL_KO, "" },  /* fuzzy */
/* nl */ { IDMS_VIC20_PAL_NL, "VIC20 PAL" },
/* pl */ { IDMS_VIC20_PAL_PL, "VIC20 PAL" },
/* ru */ { IDMS_VIC20_PAL_RU, "" },  /* fuzzy */
/* sv */ { IDMS_VIC20_PAL_SV, "VIC20 PAL" },
/* tr */ { IDMS_VIC20_PAL_TR, "" },  /* fuzzy */

/* en */ { IDMS_VIC20_NTSC,    "VIC20 NTSC" },
/* da */ { IDMS_VIC20_NTSC_DA, "" },  /* fuzzy */
/* de */ { IDMS_VIC20_NTSC_DE, "VIC20 NTSC" },
/* es */ { IDMS_VIC20_NTSC_ES, "VIC20 NTSC" },
/* fr */ { IDMS_VIC20_NTSC_FR, "" },  /* fuzzy */
/* hu */ { IDMS_VIC20_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDMS_VIC20_NTSC_IT, "VIC20 NTSC" },
/* ko */ { IDMS_VIC20_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDMS_VIC20_NTSC_NL, "VIC20 NTSC" },
/* pl */ { IDMS_VIC20_NTSC_PL, "VIC20 NTSC" },
/* ru */ { IDMS_VIC20_NTSC_RU, "" },  /* fuzzy */
/* sv */ { IDMS_VIC20_NTSC_SV, "VIC20 NTSC" },
/* tr */ { IDMS_VIC20_NTSC_TR, "" },  /* fuzzy */

/* en */ { IDMS_VIC21,    "VIC21" },
/* da */ { IDMS_VIC21_DA, "" },  /* fuzzy */
/* de */ { IDMS_VIC21_DE, "VIC21" },
/* es */ { IDMS_VIC21_ES, "VIC21" },
/* fr */ { IDMS_VIC21_FR, "" },  /* fuzzy */
/* hu */ { IDMS_VIC21_HU, "" },  /* fuzzy */
/* it */ { IDMS_VIC21_IT, "VIC21" },
/* ko */ { IDMS_VIC21_KO, "" },  /* fuzzy */
/* nl */ { IDMS_VIC21_NL, "VIC21" },
/* pl */ { IDMS_VIC21_PL, "VIC21" },
/* ru */ { IDMS_VIC21_RU, "" },  /* fuzzy */
/* sv */ { IDMS_VIC21_SV, "VIC21" },
/* tr */ { IDMS_VIC21_TR, "" },  /* fuzzy */

/* en */ { IDMS_C64DTV_MODEL_SETTINGS,    "C64DTV model settings..." },
/* da */ { IDMS_C64DTV_MODEL_SETTINGS_DA, "C64DTV-modelindstillinger..." },
/* de */ { IDMS_C64DTV_MODEL_SETTINGS_DE, "C64DTV Modell Einstellungen..." },
/* es */ { IDMS_C64DTV_MODEL_SETTINGS_ES, "Ajustes modelo C64DTV..." },
/* fr */ { IDMS_C64DTV_MODEL_SETTINGS_FR, "Paramètres du modèle C64DTV..." },
/* hu */ { IDMS_C64DTV_MODEL_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_C64DTV_MODEL_SETTINGS_IT, "Impostazioni modello C64DTV..." },
/* ko */ { IDMS_C64DTV_MODEL_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C64DTV_MODEL_SETTINGS_NL, "C64DTV model instellingen..." },
/* pl */ { IDMS_C64DTV_MODEL_SETTINGS_PL, "Ustawienia modelu C64DTV..." },
/* ru */ { IDMS_C64DTV_MODEL_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_C64DTV_MODEL_SETTINGS_SV, "C64DTV-modellinställningar..." },
/* tr */ { IDMS_C64DTV_MODEL_SETTINGS_TR, "C64DTV model ayarlarý..." },

/* en */ { IDMS_C64DTV2_PAL,    "C64DTV V2 PAL" },
/* da */ { IDMS_C64DTV2_PAL_DA, "C64DTV V2 PAL" },
/* de */ { IDMS_C64DTV2_PAL_DE, "C64DTV V2 PAL" },
/* es */ { IDMS_C64DTV2_PAL_ES, "C64DTV V2 PAL" },
/* fr */ { IDMS_C64DTV2_PAL_FR, "PAL C64DTV V2" },
/* hu */ { IDMS_C64DTV2_PAL_HU, "" },  /* fuzzy */
/* it */ { IDMS_C64DTV2_PAL_IT, "C64DTV V2 PAL" },
/* ko */ { IDMS_C64DTV2_PAL_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C64DTV2_PAL_NL, "C64DTV V2 PAL" },
/* pl */ { IDMS_C64DTV2_PAL_PL, "C64DTV V2 PAL" },
/* ru */ { IDMS_C64DTV2_PAL_RU, "" },  /* fuzzy */
/* sv */ { IDMS_C64DTV2_PAL_SV, "C64DTV V2 PAL" },
/* tr */ { IDMS_C64DTV2_PAL_TR, "C64DTV V2 PAL" },

/* en */ { IDMS_C64DTV2_NTSC,    "C64DTV V2 NTSC" },
/* da */ { IDMS_C64DTV2_NTSC_DA, "C64DTV V2 NTSC" },
/* de */ { IDMS_C64DTV2_NTSC_DE, "C64DTV V2 NTSC" },
/* es */ { IDMS_C64DTV2_NTSC_ES, "C64DTV V2 PAL" },
/* fr */ { IDMS_C64DTV2_NTSC_FR, "NTSC C64DTV V2" },
/* hu */ { IDMS_C64DTV2_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDMS_C64DTV2_NTSC_IT, "C64DTV V2 NTSC" },
/* ko */ { IDMS_C64DTV2_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C64DTV2_NTSC_NL, "C64DTV V2 NTSC" },
/* pl */ { IDMS_C64DTV2_NTSC_PL, "C64DTV V2 NTSC" },
/* ru */ { IDMS_C64DTV2_NTSC_RU, "" },  /* fuzzy */
/* sv */ { IDMS_C64DTV2_NTSC_SV, "C64DTV V2 NTSC" },
/* tr */ { IDMS_C64DTV2_NTSC_TR, "C64DTV V2 NTSC" },

/* en */ { IDMS_C64DTV3_PAL,    "C64DTV V3 PAL" },
/* da */ { IDMS_C64DTV3_PAL_DA, "C64DTV V3 PAL" },
/* de */ { IDMS_C64DTV3_PAL_DE, "C64DTV V3 PAL" },
/* es */ { IDMS_C64DTV3_PAL_ES, "C64DTV V2 PAL" },
/* fr */ { IDMS_C64DTV3_PAL_FR, "PAL C64DTV V3" },
/* hu */ { IDMS_C64DTV3_PAL_HU, "" },  /* fuzzy */
/* it */ { IDMS_C64DTV3_PAL_IT, "C64DTV V3 PAL" },
/* ko */ { IDMS_C64DTV3_PAL_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C64DTV3_PAL_NL, "C64DTV V3 PAL" },
/* pl */ { IDMS_C64DTV3_PAL_PL, "C64DTV V3 PAL" },
/* ru */ { IDMS_C64DTV3_PAL_RU, "" },  /* fuzzy */
/* sv */ { IDMS_C64DTV3_PAL_SV, "C64DTV V3 PAL" },
/* tr */ { IDMS_C64DTV3_PAL_TR, "C64DTV V3 PAL" },

/* en */ { IDMS_C64DTV3_NTSC,    "C64DTV V3 NTSC" },
/* da */ { IDMS_C64DTV3_NTSC_DA, "C64DTV V3 NTSC" },
/* de */ { IDMS_C64DTV3_NTSC_DE, "C64DTV V3 NTSC" },
/* es */ { IDMS_C64DTV3_NTSC_ES, "C64DTV V3 NTSC" },
/* fr */ { IDMS_C64DTV3_NTSC_FR, "NTSC C64DTV V3" },
/* hu */ { IDMS_C64DTV3_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDMS_C64DTV3_NTSC_IT, "C64DTV V3 NTSC" },
/* ko */ { IDMS_C64DTV3_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C64DTV3_NTSC_NL, "C64DTV V3 NTSC" },
/* pl */ { IDMS_C64DTV3_NTSC_PL, "C64DTV V3 NTSC" },
/* ru */ { IDMS_C64DTV3_NTSC_RU, "" },  /* fuzzy */
/* sv */ { IDMS_C64DTV3_NTSC_SV, "C64DTV V3 NTSC" },
/* tr */ { IDMS_C64DTV3_NTSC_TR, "C64DTV V3 NTSC" },

/* en */ { IDMS_HUMMER_NTSC,    "Hummer NTSC" },
/* da */ { IDMS_HUMMER_NTSC_DA, "Hummer NTSC" },
/* de */ { IDMS_HUMMER_NTSC_DE, "Hummer NTSC" },
/* es */ { IDMS_HUMMER_NTSC_ES, "Hummer NTSC" },
/* fr */ { IDMS_HUMMER_NTSC_FR, "NTSC Hummer" },
/* hu */ { IDMS_HUMMER_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDMS_HUMMER_NTSC_IT, "Hummer NTSC" },
/* ko */ { IDMS_HUMMER_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDMS_HUMMER_NTSC_NL, "Hummer NTSC" },
/* pl */ { IDMS_HUMMER_NTSC_PL, "Hummer NTSC" },
/* ru */ { IDMS_HUMMER_NTSC_RU, "" },  /* fuzzy */
/* sv */ { IDMS_HUMMER_NTSC_SV, "Hummer NTSC" },
/* tr */ { IDMS_HUMMER_NTSC_TR, "Hummer NTSC" },

/* en */ { IDMS_C128_MODEL_SETTINGS,    "C128 model settings..." },
/* da */ { IDMS_C128_MODEL_SETTINGS_DA, "C128-modelindstillinger..." },
/* de */ { IDMS_C128_MODEL_SETTINGS_DE, "C128 Modell Einstellungen..." },
/* es */ { IDMS_C128_MODEL_SETTINGS_ES, "Ajustes modelo C128..." },
/* fr */ { IDMS_C128_MODEL_SETTINGS_FR, "Paramètres du modèle C128..." },
/* hu */ { IDMS_C128_MODEL_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_C128_MODEL_SETTINGS_IT, "Impostazioni modello C128..." },
/* ko */ { IDMS_C128_MODEL_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C128_MODEL_SETTINGS_NL, "C128 model instellingen..." },
/* pl */ { IDMS_C128_MODEL_SETTINGS_PL, "Ustawienia modelu C128..." },
/* ru */ { IDMS_C128_MODEL_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_C128_MODEL_SETTINGS_SV, "C128-modellinställningar..." },
/* tr */ { IDMS_C128_MODEL_SETTINGS_TR, "C128 model ayarlarý..." },

/* en */ { IDMS_C128_PAL,    "C128 PAL" },
/* da */ { IDMS_C128_PAL_DA, "C128 PAL" },
/* de */ { IDMS_C128_PAL_DE, "C128 PAL" },
/* es */ { IDMS_C128_PAL_ES, "C128 PAL" },
/* fr */ { IDMS_C128_PAL_FR, "C128 PAL" },
/* hu */ { IDMS_C128_PAL_HU, "" },  /* fuzzy */
/* it */ { IDMS_C128_PAL_IT, "C128 PAL" },
/* ko */ { IDMS_C128_PAL_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C128_PAL_NL, "C128 PAL" },
/* pl */ { IDMS_C128_PAL_PL, "C128 PAL" },
/* ru */ { IDMS_C128_PAL_RU, "" },  /* fuzzy */
/* sv */ { IDMS_C128_PAL_SV, "C128 PAL" },
/* tr */ { IDMS_C128_PAL_TR, "C128 PAL" },

/* en */ { IDMS_C128DCR_PAL,    "C128DCR PAL" },
/* da */ { IDMS_C128DCR_PAL_DA, "C128DCR PAL" },
/* de */ { IDMS_C128DCR_PAL_DE, "C128DCR PAL" },
/* es */ { IDMS_C128DCR_PAL_ES, "C128DCR PAL" },
/* fr */ { IDMS_C128DCR_PAL_FR, "C128DCR PAL" },
/* hu */ { IDMS_C128DCR_PAL_HU, "" },  /* fuzzy */
/* it */ { IDMS_C128DCR_PAL_IT, "C128DCR PAL" },
/* ko */ { IDMS_C128DCR_PAL_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C128DCR_PAL_NL, "C128DCR PAL" },
/* pl */ { IDMS_C128DCR_PAL_PL, "C128DCR PAL" },
/* ru */ { IDMS_C128DCR_PAL_RU, "" },  /* fuzzy */
/* sv */ { IDMS_C128DCR_PAL_SV, "C128DCR PAL" },
/* tr */ { IDMS_C128DCR_PAL_TR, "C128DCR PAL" },

/* en */ { IDMS_C128_NTSC,    "C128 NTSC" },
/* da */ { IDMS_C128_NTSC_DA, "C128 NTSC" },
/* de */ { IDMS_C128_NTSC_DE, "C128 NTSC" },
/* es */ { IDMS_C128_NTSC_ES, "C128 NTSC" },
/* fr */ { IDMS_C128_NTSC_FR, "NTSC C128" },
/* hu */ { IDMS_C128_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDMS_C128_NTSC_IT, "C128 NTSC" },
/* ko */ { IDMS_C128_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C128_NTSC_NL, "C128 NTSC" },
/* pl */ { IDMS_C128_NTSC_PL, "C128 NTSC" },
/* ru */ { IDMS_C128_NTSC_RU, "" },  /* fuzzy */
/* sv */ { IDMS_C128_NTSC_SV, "C128 NTSC" },
/* tr */ { IDMS_C128_NTSC_TR, "C128 NTSC" },

/* en */ { IDMS_C128DCR_NTSC,    "C128DCR NTSC" },
/* da */ { IDMS_C128DCR_NTSC_DA, "C128DCR NTSC" },
/* de */ { IDMS_C128DCR_NTSC_DE, "C128DCR NTSC" },
/* es */ { IDMS_C128DCR_NTSC_ES, "C128DCR NTSC" },
/* fr */ { IDMS_C128DCR_NTSC_FR, "NTSC C128DCR" },
/* hu */ { IDMS_C128DCR_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDMS_C128DCR_NTSC_IT, "C128DCR NTSC" },
/* ko */ { IDMS_C128DCR_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C128DCR_NTSC_NL, "C128DCR NTSC" },
/* pl */ { IDMS_C128DCR_NTSC_PL, "C128DCR NTSC" },
/* ru */ { IDMS_C128DCR_NTSC_RU, "" },  /* fuzzy */
/* sv */ { IDMS_C128DCR_NTSC_SV, "C128DCR NTSC" },
/* tr */ { IDMS_C128DCR_NTSC_TR, "C128DCR NTSC" },

/* en */ { IDMS_PET_MODEL_SETTINGS,    "PET model settings..." },
/* da */ { IDMS_PET_MODEL_SETTINGS_DA, "PET-model indstillinger..." },
/* de */ { IDMS_PET_MODEL_SETTINGS_DE, "PET Modell Einstellungen..." },
/* es */ { IDMS_PET_MODEL_SETTINGS_ES, "Ajustes modelo PET..." },
/* fr */ { IDMS_PET_MODEL_SETTINGS_FR, "Paramètres du modèle PET..." },
/* hu */ { IDMS_PET_MODEL_SETTINGS_HU, "PET modell beállításai..." },
/* it */ { IDMS_PET_MODEL_SETTINGS_IT, "Impostazioni modello PET..." },
/* ko */ { IDMS_PET_MODEL_SETTINGS_KO, "PET ¸ðµ¨ ¼ÂÆÃ..." },
/* nl */ { IDMS_PET_MODEL_SETTINGS_NL, "PET model instellingen..." },
/* pl */ { IDMS_PET_MODEL_SETTINGS_PL, "Ustawienia modelu PET..." },
/* ru */ { IDMS_PET_MODEL_SETTINGS_RU, "½ÐáâàÞÙÚØ ÜÞÔÕÛØ PET..." },
/* sv */ { IDMS_PET_MODEL_SETTINGS_SV, "Inställningar för PET-modell..." },
/* tr */ { IDMS_PET_MODEL_SETTINGS_TR, "PET model ayarlarý..." },

/* en */ { IDMS_2001,    "2001" },
/* da */ { IDMS_2001_DA, "2001" },
/* de */ { IDMS_2001_DE, "2001" },
/* es */ { IDMS_2001_ES, "2001" },
/* fr */ { IDMS_2001_FR, "2001" },
/* hu */ { IDMS_2001_HU, "" },  /* fuzzy */
/* it */ { IDMS_2001_IT, "2001" },
/* ko */ { IDMS_2001_KO, "" },  /* fuzzy */
/* nl */ { IDMS_2001_NL, "2001" },
/* pl */ { IDMS_2001_PL, "2001" },
/* ru */ { IDMS_2001_RU, "2001" },
/* sv */ { IDMS_2001_SV, "2001" },
/* tr */ { IDMS_2001_TR, "2001" },

/* en */ { IDMS_3008,    "3008" },
/* da */ { IDMS_3008_DA, "3008" },
/* de */ { IDMS_3008_DE, "3008" },
/* es */ { IDMS_3008_ES, "3008" },
/* fr */ { IDMS_3008_FR, "3008" },
/* hu */ { IDMS_3008_HU, "" },  /* fuzzy */
/* it */ { IDMS_3008_IT, "3008" },
/* ko */ { IDMS_3008_KO, "" },  /* fuzzy */
/* nl */ { IDMS_3008_NL, "3008" },
/* pl */ { IDMS_3008_PL, "3008" },
/* ru */ { IDMS_3008_RU, "3008" },
/* sv */ { IDMS_3008_SV, "3008" },
/* tr */ { IDMS_3008_TR, "3008" },

/* en */ { IDMS_3016,    "3016" },
/* da */ { IDMS_3016_DA, "3016" },
/* de */ { IDMS_3016_DE, "3016" },
/* es */ { IDMS_3016_ES, "3016" },
/* fr */ { IDMS_3016_FR, "3016" },
/* hu */ { IDMS_3016_HU, "" },  /* fuzzy */
/* it */ { IDMS_3016_IT, "3016" },
/* ko */ { IDMS_3016_KO, "" },  /* fuzzy */
/* nl */ { IDMS_3016_NL, "3016" },
/* pl */ { IDMS_3016_PL, "3016" },
/* ru */ { IDMS_3016_RU, "3016" },
/* sv */ { IDMS_3016_SV, "3016" },
/* tr */ { IDMS_3016_TR, "3016" },

/* en */ { IDMS_3032,    "3032" },
/* da */ { IDMS_3032_DA, "3032" },
/* de */ { IDMS_3032_DE, "3032" },
/* es */ { IDMS_3032_ES, "3032" },
/* fr */ { IDMS_3032_FR, "3032" },
/* hu */ { IDMS_3032_HU, "" },  /* fuzzy */
/* it */ { IDMS_3032_IT, "3032" },
/* ko */ { IDMS_3032_KO, "" },  /* fuzzy */
/* nl */ { IDMS_3032_NL, "3032" },
/* pl */ { IDMS_3032_PL, "3032" },
/* ru */ { IDMS_3032_RU, "3032" },
/* sv */ { IDMS_3032_SV, "3032" },
/* tr */ { IDMS_3032_TR, "3032" },

/* en */ { IDMS_3032B,    "3032B" },
/* da */ { IDMS_3032B_DA, "3032B" },
/* de */ { IDMS_3032B_DE, "3032B" },
/* es */ { IDMS_3032B_ES, "3032B" },
/* fr */ { IDMS_3032B_FR, "3032B" },
/* hu */ { IDMS_3032B_HU, "" },  /* fuzzy */
/* it */ { IDMS_3032B_IT, "3032B" },
/* ko */ { IDMS_3032B_KO, "" },  /* fuzzy */
/* nl */ { IDMS_3032B_NL, "3032B" },
/* pl */ { IDMS_3032B_PL, "3032B" },
/* ru */ { IDMS_3032B_RU, "3032B" },
/* sv */ { IDMS_3032B_SV, "3032B" },
/* tr */ { IDMS_3032B_TR, "3032B" },

/* en */ { IDMS_4016,    "4016" },
/* da */ { IDMS_4016_DA, "4016" },
/* de */ { IDMS_4016_DE, "4016" },
/* es */ { IDMS_4016_ES, "4016" },
/* fr */ { IDMS_4016_FR, "4016" },
/* hu */ { IDMS_4016_HU, "" },  /* fuzzy */
/* it */ { IDMS_4016_IT, "4016" },
/* ko */ { IDMS_4016_KO, "" },  /* fuzzy */
/* nl */ { IDMS_4016_NL, "4016" },
/* pl */ { IDMS_4016_PL, "4016" },
/* ru */ { IDMS_4016_RU, "4016" },
/* sv */ { IDMS_4016_SV, "4016" },
/* tr */ { IDMS_4016_TR, "4016" },

/* en */ { IDMS_4032,    "4032" },
/* da */ { IDMS_4032_DA, "4032" },
/* de */ { IDMS_4032_DE, "4032" },
/* es */ { IDMS_4032_ES, "4032" },
/* fr */ { IDMS_4032_FR, "4032" },
/* hu */ { IDMS_4032_HU, "" },  /* fuzzy */
/* it */ { IDMS_4032_IT, "4032" },
/* ko */ { IDMS_4032_KO, "" },  /* fuzzy */
/* nl */ { IDMS_4032_NL, "4032" },
/* pl */ { IDMS_4032_PL, "4032" },
/* ru */ { IDMS_4032_RU, "4032" },
/* sv */ { IDMS_4032_SV, "4032" },
/* tr */ { IDMS_4032_TR, "4032" },

/* en */ { IDMS_4032B,    "4032B" },
/* da */ { IDMS_4032B_DA, "4032B" },
/* de */ { IDMS_4032B_DE, "4032B" },
/* es */ { IDMS_4032B_ES, "4032B" },
/* fr */ { IDMS_4032B_FR, "4032B" },
/* hu */ { IDMS_4032B_HU, "" },  /* fuzzy */
/* it */ { IDMS_4032B_IT, "4032B" },
/* ko */ { IDMS_4032B_KO, "" },  /* fuzzy */
/* nl */ { IDMS_4032B_NL, "4032B" },
/* pl */ { IDMS_4032B_PL, "4032B" },
/* ru */ { IDMS_4032B_RU, "4032B" },
/* sv */ { IDMS_4032B_SV, "4032B" },
/* tr */ { IDMS_4032B_TR, "4032B" },

/* en */ { IDMS_8032,    "8032" },
/* da */ { IDMS_8032_DA, "8032" },
/* de */ { IDMS_8032_DE, "8032" },
/* es */ { IDMS_8032_ES, "8032" },
/* fr */ { IDMS_8032_FR, "8032" },
/* hu */ { IDMS_8032_HU, "" },  /* fuzzy */
/* it */ { IDMS_8032_IT, "8032" },
/* ko */ { IDMS_8032_KO, "" },  /* fuzzy */
/* nl */ { IDMS_8032_NL, "8032" },
/* pl */ { IDMS_8032_PL, "8032" },
/* ru */ { IDMS_8032_RU, "8032" },
/* sv */ { IDMS_8032_SV, "8032" },
/* tr */ { IDMS_8032_TR, "8032" },

/* en */ { IDMS_8096,    "8096" },
/* da */ { IDMS_8096_DA, "8096" },
/* de */ { IDMS_8096_DE, "8096" },
/* es */ { IDMS_8096_ES, "8096" },
/* fr */ { IDMS_8096_FR, "8096" },
/* hu */ { IDMS_8096_HU, "" },  /* fuzzy */
/* it */ { IDMS_8096_IT, "8096" },
/* ko */ { IDMS_8096_KO, "" },  /* fuzzy */
/* nl */ { IDMS_8096_NL, "8096" },
/* pl */ { IDMS_8096_PL, "8096" },
/* ru */ { IDMS_8096_RU, "8096" },
/* sv */ { IDMS_8096_SV, "8096" },
/* tr */ { IDMS_8096_TR, "8096" },

/* en */ { IDMS_8296,    "8296" },
/* da */ { IDMS_8296_DA, "8296" },
/* de */ { IDMS_8296_DE, "8296" },
/* es */ { IDMS_8296_ES, "8293" },
/* fr */ { IDMS_8296_FR, "8296" },
/* hu */ { IDMS_8296_HU, "" },  /* fuzzy */
/* it */ { IDMS_8296_IT, "8296" },
/* ko */ { IDMS_8296_KO, "" },  /* fuzzy */
/* nl */ { IDMS_8296_NL, "8296" },
/* pl */ { IDMS_8296_PL, "8296" },
/* ru */ { IDMS_8296_RU, "8296" },
/* sv */ { IDMS_8296_SV, "8296" },
/* tr */ { IDMS_8296_TR, "8296" },

/* en */ { IDMS_SUPERPET,    "SuperPET" },
/* da */ { IDMS_SUPERPET_DA, "SuperPET" },
/* de */ { IDMS_SUPERPET_DE, "SuperPET" },
/* es */ { IDMS_SUPERPET_ES, "SuperPET" },
/* fr */ { IDMS_SUPERPET_FR, "SuperPET" },
/* hu */ { IDMS_SUPERPET_HU, "" },  /* fuzzy */
/* it */ { IDMS_SUPERPET_IT, "SuperPET" },
/* ko */ { IDMS_SUPERPET_KO, "" },  /* fuzzy */
/* nl */ { IDMS_SUPERPET_NL, "SuperPET" },
/* pl */ { IDMS_SUPERPET_PL, "SuperPET" },
/* ru */ { IDMS_SUPERPET_RU, "" },  /* fuzzy */
/* sv */ { IDMS_SUPERPET_SV, "SuperPET" },
/* tr */ { IDMS_SUPERPET_TR, "SuperPET" },

/* en */ { IDMS_CBM2_MODEL_SETTINGS,    "CBM2 model settings..." },
/* da */ { IDMS_CBM2_MODEL_SETTINGS_DA, "CBM2-modelindstillinger..." },
/* de */ { IDMS_CBM2_MODEL_SETTINGS_DE, "CBM2 Modell Einstellungen..." },
/* es */ { IDMS_CBM2_MODEL_SETTINGS_ES, "Ajustes modelo CBM2..." },
/* fr */ { IDMS_CBM2_MODEL_SETTINGS_FR, "Paramètres du modèle CBM2..." },
/* hu */ { IDMS_CBM2_MODEL_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_CBM2_MODEL_SETTINGS_IT, "Impostazioni modello CBM2..." },
/* ko */ { IDMS_CBM2_MODEL_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_CBM2_MODEL_SETTINGS_NL, "CBM2 model instellingen..." },
/* pl */ { IDMS_CBM2_MODEL_SETTINGS_PL, "Ustawienia modelu CBM2..." },
/* ru */ { IDMS_CBM2_MODEL_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_CBM2_MODEL_SETTINGS_SV, "CBM2-modellinställningar..." },
/* tr */ { IDMS_CBM2_MODEL_SETTINGS_TR, "CBM2 model ayarlarý..." },

/* en */ { IDMS_610_PAL,    "CBM610 PAL" },
/* da */ { IDMS_610_PAL_DA, "CBM610 PAL" },
/* de */ { IDMS_610_PAL_DE, "CBM610 PAL" },
/* es */ { IDMS_610_PAL_ES, "CBM610 PAL" },
/* fr */ { IDMS_610_PAL_FR, "CBM610 PAL" },
/* hu */ { IDMS_610_PAL_HU, "" },  /* fuzzy */
/* it */ { IDMS_610_PAL_IT, "CBM610 PAL" },
/* ko */ { IDMS_610_PAL_KO, "" },  /* fuzzy */
/* nl */ { IDMS_610_PAL_NL, "CBM610 PAL" },
/* pl */ { IDMS_610_PAL_PL, "CBM610 PAL" },
/* ru */ { IDMS_610_PAL_RU, "" },  /* fuzzy */
/* sv */ { IDMS_610_PAL_SV, "CBM610 PAL" },
/* tr */ { IDMS_610_PAL_TR, "CBM610 PAL" },

/* en */ { IDMS_610_NTSC,    "CBM610 NTSC" },
/* da */ { IDMS_610_NTSC_DA, "CBM610 NTSC" },
/* de */ { IDMS_610_NTSC_DE, "CBM610 NTSC" },
/* es */ { IDMS_610_NTSC_ES, "CBM610 NTSC" },
/* fr */ { IDMS_610_NTSC_FR, "NTSC CBM610" },
/* hu */ { IDMS_610_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDMS_610_NTSC_IT, "CBM610 NTSC" },
/* ko */ { IDMS_610_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDMS_610_NTSC_NL, "CBM610 NTSC" },
/* pl */ { IDMS_610_NTSC_PL, "CBM610 NTSC" },
/* ru */ { IDMS_610_NTSC_RU, "" },  /* fuzzy */
/* sv */ { IDMS_610_NTSC_SV, "CBM610 NTSC" },
/* tr */ { IDMS_610_NTSC_TR, "CBM610 NTSC" },

/* en */ { IDMS_620_PAL,    "CBM620 PAL" },
/* da */ { IDMS_620_PAL_DA, "CBM620 PAL" },
/* de */ { IDMS_620_PAL_DE, "CBM620 PAL" },
/* es */ { IDMS_620_PAL_ES, "CBM620 PAL" },
/* fr */ { IDMS_620_PAL_FR, "CBM620 PAL" },
/* hu */ { IDMS_620_PAL_HU, "" },  /* fuzzy */
/* it */ { IDMS_620_PAL_IT, "CBM620 PAL" },
/* ko */ { IDMS_620_PAL_KO, "" },  /* fuzzy */
/* nl */ { IDMS_620_PAL_NL, "CBM620 PAL" },
/* pl */ { IDMS_620_PAL_PL, "CBM620 PAL" },
/* ru */ { IDMS_620_PAL_RU, "" },  /* fuzzy */
/* sv */ { IDMS_620_PAL_SV, "CBM620 PAL" },
/* tr */ { IDMS_620_PAL_TR, "CBM620 PAL" },

/* en */ { IDMS_620_NTSC,    "CBM620 NTSC" },
/* da */ { IDMS_620_NTSC_DA, "CBM620 NTSC" },
/* de */ { IDMS_620_NTSC_DE, "CBM620 NTSC" },
/* es */ { IDMS_620_NTSC_ES, "CBM620 NTSC" },
/* fr */ { IDMS_620_NTSC_FR, "NTSC CBM620" },
/* hu */ { IDMS_620_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDMS_620_NTSC_IT, "CBM620 NTSC" },
/* ko */ { IDMS_620_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDMS_620_NTSC_NL, "CBM620 NTSC" },
/* pl */ { IDMS_620_NTSC_PL, "CBM620 NTSC" },
/* ru */ { IDMS_620_NTSC_RU, "" },  /* fuzzy */
/* sv */ { IDMS_620_NTSC_SV, "CBM620 NTSC" },
/* tr */ { IDMS_620_NTSC_TR, "CBM620 NTSC" },

/* en */ { IDMS_620PLUS_PAL,    "CBM620+ PAL" },
/* da */ { IDMS_620PLUS_PAL_DA, "CBM620+ PAL" },
/* de */ { IDMS_620PLUS_PAL_DE, "CBM620+ PAL" },
/* es */ { IDMS_620PLUS_PAL_ES, "CBM620+ PAL" },
/* fr */ { IDMS_620PLUS_PAL_FR, "CBM620+ PAL" },
/* hu */ { IDMS_620PLUS_PAL_HU, "" },  /* fuzzy */
/* it */ { IDMS_620PLUS_PAL_IT, "CBM620+ PAL" },
/* ko */ { IDMS_620PLUS_PAL_KO, "" },  /* fuzzy */
/* nl */ { IDMS_620PLUS_PAL_NL, "CBM620+ PAL" },
/* pl */ { IDMS_620PLUS_PAL_PL, "CBM620+ PAL" },
/* ru */ { IDMS_620PLUS_PAL_RU, "" },  /* fuzzy */
/* sv */ { IDMS_620PLUS_PAL_SV, "CBM620+ PAL" },
/* tr */ { IDMS_620PLUS_PAL_TR, "CBM620+ PAL" },

/* en */ { IDMS_620PLUS_NTSC,    "CBM620+ NTSC" },
/* da */ { IDMS_620PLUS_NTSC_DA, "CBM620+ NTSC" },
/* de */ { IDMS_620PLUS_NTSC_DE, "CBM620+ NTSC" },
/* es */ { IDMS_620PLUS_NTSC_ES, "CBM620+ NTSC" },
/* fr */ { IDMS_620PLUS_NTSC_FR, "NTSC CBM620+" },
/* hu */ { IDMS_620PLUS_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDMS_620PLUS_NTSC_IT, "CBM620+ NTSC" },
/* ko */ { IDMS_620PLUS_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDMS_620PLUS_NTSC_NL, "CBM620+ NTSC" },
/* pl */ { IDMS_620PLUS_NTSC_PL, "CBM620+ NTSC" },
/* ru */ { IDMS_620PLUS_NTSC_RU, "" },  /* fuzzy */
/* sv */ { IDMS_620PLUS_NTSC_SV, "CBM620+ NTSC" },
/* tr */ { IDMS_620PLUS_NTSC_TR, "CBM620+ NTSC" },

/* en */ { IDMS_710_NTSC,    "CBM710 NTSC" },
/* da */ { IDMS_710_NTSC_DA, "CBM710 NTSC" },
/* de */ { IDMS_710_NTSC_DE, "CBM710 NTSC" },
/* es */ { IDMS_710_NTSC_ES, "CBM710 NTSC" },
/* fr */ { IDMS_710_NTSC_FR, "NTSC CBM710" },
/* hu */ { IDMS_710_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDMS_710_NTSC_IT, "CBM710 NTSC" },
/* ko */ { IDMS_710_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDMS_710_NTSC_NL, "CBM710 NTSC" },
/* pl */ { IDMS_710_NTSC_PL, "CBM710 NTSC" },
/* ru */ { IDMS_710_NTSC_RU, "" },  /* fuzzy */
/* sv */ { IDMS_710_NTSC_SV, "CBM710 NTSC" },
/* tr */ { IDMS_710_NTSC_TR, "CBM710 NTSC" },

/* en */ { IDMS_720_NTSC,    "CBM720 NTSC" },
/* da */ { IDMS_720_NTSC_DA, "CBM720 NTSC" },
/* de */ { IDMS_720_NTSC_DE, "CBM720 NTSC" },
/* es */ { IDMS_720_NTSC_ES, "CBM720 NTSC" },
/* fr */ { IDMS_720_NTSC_FR, "NTSC CBM720" },
/* hu */ { IDMS_720_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDMS_720_NTSC_IT, "CBM720 NTSC" },
/* ko */ { IDMS_720_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDMS_720_NTSC_NL, "CBM720 NTSC" },
/* pl */ { IDMS_720_NTSC_PL, "CBM720 NTSC" },
/* ru */ { IDMS_720_NTSC_RU, "" },  /* fuzzy */
/* sv */ { IDMS_720_NTSC_SV, "CBM720 NTSC" },
/* tr */ { IDMS_720_NTSC_TR, "CBM720 NTSC" },

/* en */ { IDMS_720PLUS_NTSC,    "CBM720+ NTSC" },
/* da */ { IDMS_720PLUS_NTSC_DA, "CBM720+ NTSC" },
/* de */ { IDMS_720PLUS_NTSC_DE, "CBM720+ NTSC" },
/* es */ { IDMS_720PLUS_NTSC_ES, "CBM720+ NTSC" },
/* fr */ { IDMS_720PLUS_NTSC_FR, "NTSC CBM720+" },
/* hu */ { IDMS_720PLUS_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDMS_720PLUS_NTSC_IT, "CBM720+ NTSC" },
/* ko */ { IDMS_720PLUS_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDMS_720PLUS_NTSC_NL, "CBM720+ NTSC" },
/* pl */ { IDMS_720PLUS_NTSC_PL, "CBM720+ NTSC" },
/* ru */ { IDMS_720PLUS_NTSC_RU, "" },  /* fuzzy */
/* sv */ { IDMS_720PLUS_NTSC_SV, "CBM720+ NTSC" },
/* tr */ { IDMS_720PLUS_NTSC_TR, "CBM720+ NTSC" },

/* en */ { IDMS_CBM5X0_MODEL_SETTINGS,    "CBM5x0 model settings..." },
/* da */ { IDMS_CBM5X0_MODEL_SETTINGS_DA, "CBM5x0-modelindstillinger..." },
/* de */ { IDMS_CBM5X0_MODEL_SETTINGS_DE, "CBM5x0 Modell Einstellungen..." },
/* es */ { IDMS_CBM5X0_MODEL_SETTINGS_ES, "Ajustes modelo CBM5x0..." },
/* fr */ { IDMS_CBM5X0_MODEL_SETTINGS_FR, "Paramètres du modèle CBM5x0..." },
/* hu */ { IDMS_CBM5X0_MODEL_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_CBM5X0_MODEL_SETTINGS_IT, "Impostazioni modello CBM5x0..." },
/* ko */ { IDMS_CBM5X0_MODEL_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_CBM5X0_MODEL_SETTINGS_NL, "CBM5x0 model instellingen..." },
/* pl */ { IDMS_CBM5X0_MODEL_SETTINGS_PL, "Ustawienia modelu CBM5x0..." },
/* ru */ { IDMS_CBM5X0_MODEL_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_CBM5X0_MODEL_SETTINGS_SV, "CBM5x0-modellinställningar..." },
/* tr */ { IDMS_CBM5X0_MODEL_SETTINGS_TR, "CBM5x0 model ayarlarý..." },

/* en */ { IDMS_510_PAL,    "CBM510 PAL" },
/* da */ { IDMS_510_PAL_DA, "CBM510 PAL" },
/* de */ { IDMS_510_PAL_DE, "CBM510 PAL" },
/* es */ { IDMS_510_PAL_ES, "CBM510 PAL" },
/* fr */ { IDMS_510_PAL_FR, "CBM510 PAL" },
/* hu */ { IDMS_510_PAL_HU, "" },  /* fuzzy */
/* it */ { IDMS_510_PAL_IT, "CBM510 PAL" },
/* ko */ { IDMS_510_PAL_KO, "" },  /* fuzzy */
/* nl */ { IDMS_510_PAL_NL, "CBM510 PAL" },
/* pl */ { IDMS_510_PAL_PL, "CBM510 PAL" },
/* ru */ { IDMS_510_PAL_RU, "CBM510 PAL" },
/* sv */ { IDMS_510_PAL_SV, "CBM510 PAL" },
/* tr */ { IDMS_510_PAL_TR, "CBM510 PAL" },

/* en */ { IDMS_510_NTSC,    "CBM510 NTSC" },
/* da */ { IDMS_510_NTSC_DA, "CBM510 NTSC" },
/* de */ { IDMS_510_NTSC_DE, "CBM510 NTSC" },
/* es */ { IDMS_510_NTSC_ES, "CBM510 NTSC" },
/* fr */ { IDMS_510_NTSC_FR, "NTSC CBM510" },
/* hu */ { IDMS_510_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDMS_510_NTSC_IT, "CBM510 NTSC" },
/* ko */ { IDMS_510_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDMS_510_NTSC_NL, "CBM510 NTSC" },
/* pl */ { IDMS_510_NTSC_PL, "CBM510 NTSC" },
/* ru */ { IDMS_510_NTSC_RU, "" },  /* fuzzy */
/* sv */ { IDMS_510_NTSC_SV, "CBM510 NTSC" },
/* tr */ { IDMS_510_NTSC_TR, "CBM510 NTSC" },

/* en */ { IDMS_AUTOSTART_SETTINGS,    "Autostart settings..." },
/* da */ { IDMS_AUTOSTART_SETTINGS_DA, "Autostart-indstillinger..." },
/* de */ { IDMS_AUTOSTART_SETTINGS_DE, "Autostart Einstellungen..." },
/* es */ { IDMS_AUTOSTART_SETTINGS_ES, "Ajustes de autoinicio..." },
/* fr */ { IDMS_AUTOSTART_SETTINGS_FR, "Paramètres Autodémarrage..." },
/* hu */ { IDMS_AUTOSTART_SETTINGS_HU, "Autostart beállításai..." },
/* it */ { IDMS_AUTOSTART_SETTINGS_IT, "Impostazioni avvio automatico..." },
/* ko */ { IDMS_AUTOSTART_SETTINGS_KO, "ÀÚµ¿½ÃÀÛ ¼ÂÆÃ..." },
/* nl */ { IDMS_AUTOSTART_SETTINGS_NL, "Autostart instellingen..." },
/* pl */ { IDMS_AUTOSTART_SETTINGS_PL, "Ustawienia autostartu..." },
/* ru */ { IDMS_AUTOSTART_SETTINGS_RU, "½ÐáâàÞÙÚØ ÐÒâÞ×ÐßãáÚÐ..." },
/* sv */ { IDMS_AUTOSTART_SETTINGS_SV, "Autostartinställningar..." },
/* tr */ { IDMS_AUTOSTART_SETTINGS_TR, "Otomatik baþlatma ayarlarý..." },

/* en */ { IDMS_VICII_VIDEO_SETTINGS,    "VICII video settings..." },
/* da */ { IDMS_VICII_VIDEO_SETTINGS_DA, "VICII grafikindstillinger..." },
/* de */ { IDMS_VICII_VIDEO_SETTINGS_DE, "VIC-II Video Einstellungen..." },
/* es */ { IDMS_VICII_VIDEO_SETTINGS_ES, "Ajustes de vídeo VIC II..." },
/* fr */ { IDMS_VICII_VIDEO_SETTINGS_FR, "Paramètres vidéo VICII..." },
/* hu */ { IDMS_VICII_VIDEO_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_VICII_VIDEO_SETTINGS_IT, "Impostazioni video VICII..." },
/* ko */ { IDMS_VICII_VIDEO_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_VICII_VIDEO_SETTINGS_NL, "VICII video instellingen..." },
/* pl */ { IDMS_VICII_VIDEO_SETTINGS_PL, "Ustawienia wideo VICII..." },
/* ru */ { IDMS_VICII_VIDEO_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_VICII_VIDEO_SETTINGS_SV, "Grafikinställningar för VICII..." },
/* tr */ { IDMS_VICII_VIDEO_SETTINGS_TR, "VICII görüntü ayarlarý..." },

/* en */ { IDMS_VIC_VIDEO_SETTINGS,    "VIC video settings..." },
/* da */ { IDMS_VIC_VIDEO_SETTINGS_DA, "VIC grafikindstillinger..." },
/* de */ { IDMS_VIC_VIDEO_SETTINGS_DE, "VIC Video Einstellungen..." },
/* es */ { IDMS_VIC_VIDEO_SETTINGS_ES, "Ajustes de vídeo VIC..." },
/* fr */ { IDMS_VIC_VIDEO_SETTINGS_FR, "Paramètres vidéo VIC..." },
/* hu */ { IDMS_VIC_VIDEO_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_VIC_VIDEO_SETTINGS_IT, "Impostazioni video VIC..." },
/* ko */ { IDMS_VIC_VIDEO_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_VIC_VIDEO_SETTINGS_NL, "VIC video instellingen..." },
/* pl */ { IDMS_VIC_VIDEO_SETTINGS_PL, "Ustawienia wideo VIC..." },
/* ru */ { IDMS_VIC_VIDEO_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_VIC_VIDEO_SETTINGS_SV, "Grafikinställningar för VIC..." },
/* tr */ { IDMS_VIC_VIDEO_SETTINGS_TR, "VIC görüntü ayarlarý..." },

/* en */ { IDMS_VDC_VIDEO_SETTINGS,    "VDC video settings..." },
/* da */ { IDMS_VDC_VIDEO_SETTINGS_DA, "VDC grafikindstillinger..." },
/* de */ { IDMS_VDC_VIDEO_SETTINGS_DE, "VDC Video Einstellungen..." },
/* es */ { IDMS_VDC_VIDEO_SETTINGS_ES, "Ajustes de vídeo VDC..." },
/* fr */ { IDMS_VDC_VIDEO_SETTINGS_FR, "Paramètres vidéo VDC..." },
/* hu */ { IDMS_VDC_VIDEO_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_VDC_VIDEO_SETTINGS_IT, "Impostazioni video VDC..." },
/* ko */ { IDMS_VDC_VIDEO_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_VDC_VIDEO_SETTINGS_NL, "VDC video instellingen..." },
/* pl */ { IDMS_VDC_VIDEO_SETTINGS_PL, "Ustawienia wideo VDC..." },
/* ru */ { IDMS_VDC_VIDEO_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_VDC_VIDEO_SETTINGS_SV, "Grafikinställningar för VDC..." },
/* tr */ { IDMS_VDC_VIDEO_SETTINGS_TR, "VDC görüntü ayarlarý..." },

/* en */ { IDMS_CRTC_VIDEO_SETTINGS,    "CRTC video settings..." },
/* da */ { IDMS_CRTC_VIDEO_SETTINGS_DA, "CRTC grafikindstillinger..." },
/* de */ { IDMS_CRTC_VIDEO_SETTINGS_DE, "CRTC Video Einstellungen..." },
/* es */ { IDMS_CRTC_VIDEO_SETTINGS_ES, "Ajustes vídeo CRTC..." },
/* fr */ { IDMS_CRTC_VIDEO_SETTINGS_FR, "Paramètres vidéo CRTC..." },
/* hu */ { IDMS_CRTC_VIDEO_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_CRTC_VIDEO_SETTINGS_IT, "Impostazioni video  CRTC..." },
/* ko */ { IDMS_CRTC_VIDEO_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_CRTC_VIDEO_SETTINGS_NL, "CRTC video instellingen..." },
/* pl */ { IDMS_CRTC_VIDEO_SETTINGS_PL, "Ustawienia wideo CRTC..." },
/* ru */ { IDMS_CRTC_VIDEO_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_CRTC_VIDEO_SETTINGS_SV, "Grafikinställningar för CRTC..." },
/* tr */ { IDMS_CRTC_VIDEO_SETTINGS_TR, "CRTC görüntü ayarlarý..." },

/* en */ { IDMS_TED_VIDEO_SETTINGS,    "TED video settings..." },
/* da */ { IDMS_TED_VIDEO_SETTINGS_DA, "TED grafikindstillinger..." },
/* de */ { IDMS_TED_VIDEO_SETTINGS_DE, "TED Video Einstellungen..." },
/* es */ { IDMS_TED_VIDEO_SETTINGS_ES, "Ajustes de vídeo TED..." },
/* fr */ { IDMS_TED_VIDEO_SETTINGS_FR, "Paramètres vidéo TED..." },
/* hu */ { IDMS_TED_VIDEO_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_TED_VIDEO_SETTINGS_IT, "Impostazioni video TED..." },
/* ko */ { IDMS_TED_VIDEO_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_TED_VIDEO_SETTINGS_NL, "TED video instellingen..." },
/* pl */ { IDMS_TED_VIDEO_SETTINGS_PL, "Ustawienia wideo TED..." },
/* ru */ { IDMS_TED_VIDEO_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_TED_VIDEO_SETTINGS_SV, "Grafikinställningar för TED..." },
/* tr */ { IDMS_TED_VIDEO_SETTINGS_TR, "TED görüntü ayarlarý..." },

/* en */ { IDMS_AUDIO_LEAK,    "Audio leak" },
/* da */ { IDMS_AUDIO_LEAK_DA, "Audiolækage" },
/* de */ { IDMS_AUDIO_LEAK_DE, "Audio Leak" },
/* es */ { IDMS_AUDIO_LEAK_ES, "Filtro audio" },
/* fr */ { IDMS_AUDIO_LEAK_FR, "Fuite audio" },
/* hu */ { IDMS_AUDIO_LEAK_HU, "" },  /* fuzzy */
/* it */ { IDMS_AUDIO_LEAK_IT, "Audio leak" },
/* ko */ { IDMS_AUDIO_LEAK_KO, "" },  /* fuzzy */
/* nl */ { IDMS_AUDIO_LEAK_NL, "Audio lek" },
/* pl */ { IDMS_AUDIO_LEAK_PL, "Audio leak" },
/* ru */ { IDMS_AUDIO_LEAK_RU, "" },  /* fuzzy */
/* sv */ { IDMS_AUDIO_LEAK_SV, "Audioläckage" },
/* tr */ { IDMS_AUDIO_LEAK_TR, "Ses kaçaðý" },

/* en */ { IDMS_PALETTE_SETTINGS,    "Palette settings..." },
/* da */ { IDMS_PALETTE_SETTINGS_DA, "Paletteindstillinger..." },
/* de */ { IDMS_PALETTE_SETTINGS_DE, "Paletteneinstellungen..." },
/* es */ { IDMS_PALETTE_SETTINGS_ES, "Ajustes Paleta..." },
/* fr */ { IDMS_PALETTE_SETTINGS_FR, "Paramètres Palette..." },
/* hu */ { IDMS_PALETTE_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_PALETTE_SETTINGS_IT, "Impostazioni palette..." },
/* ko */ { IDMS_PALETTE_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_PALETTE_SETTINGS_NL, "Palet instellingen..." },
/* pl */ { IDMS_PALETTE_SETTINGS_PL, "Ustawienia palety barw..." },
/* ru */ { IDMS_PALETTE_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_PALETTE_SETTINGS_SV, "Palettinställningar..." },
/* tr */ { IDMS_PALETTE_SETTINGS_TR, "Palet ayarlarý..." },

/* en */ { IDMS_COLOR_SETTINGS,    "Color settings..." },
/* da */ { IDMS_COLOR_SETTINGS_DA, "Farveindstillinger..." },
/* de */ { IDMS_COLOR_SETTINGS_DE, "Farbeinstellungen..." },
/* es */ { IDMS_COLOR_SETTINGS_ES, "Ajustes color..." },
/* fr */ { IDMS_COLOR_SETTINGS_FR, "Paramètres de couleurs..." },
/* hu */ { IDMS_COLOR_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_COLOR_SETTINGS_IT, "Impostazioni colori..." },
/* ko */ { IDMS_COLOR_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_COLOR_SETTINGS_NL, "Kleurinstellingen..." },
/* pl */ { IDMS_COLOR_SETTINGS_PL, "Ustawienia koloru..." },
/* ru */ { IDMS_COLOR_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_COLOR_SETTINGS_SV, "Färginställningar..." },
/* tr */ { IDMS_COLOR_SETTINGS_TR, "Renk ayarlarý..." },

/* en */ { IDMS_RENDER_FILTER,    "Render filter settings..." },
/* da */ { IDMS_RENDER_FILTER_DA, "Renderingsfilterindstillinger..." },
/* de */ { IDMS_RENDER_FILTER_DE, "Render Filter Einstellungen..." },
/* es */ { IDMS_RENDER_FILTER_ES, "Ajuste filtro de renderizado..." },
/* fr */ { IDMS_RENDER_FILTER_FR, "Paramètres filtres de rendu..." },
/* hu */ { IDMS_RENDER_FILTER_HU, "" },  /* fuzzy */
/* it */ { IDMS_RENDER_FILTER_IT, "Impostazioni filtro di render..." },
/* ko */ { IDMS_RENDER_FILTER_KO, "" },  /* fuzzy */
/* nl */ { IDMS_RENDER_FILTER_NL, "Render filter instellingen..." },
/* pl */ { IDMS_RENDER_FILTER_PL, "Ustawienia filtra renderera..." },
/* ru */ { IDMS_RENDER_FILTER_RU, "" },  /* fuzzy */
/* sv */ { IDMS_RENDER_FILTER_SV, "Inställningar för rendrerarfilter..." },
/* tr */ { IDMS_RENDER_FILTER_TR, "Ýmge oluþturma filtresi ayarlarý..." },

/* en */ { IDMS_CRT_EMULATION_SETTINGS,    "CRT emulation settings..." },
/* da */ { IDMS_CRT_EMULATION_SETTINGS_DA, "Indstillinger for CRT-emulering..." },
/* de */ { IDMS_CRT_EMULATION_SETTINGS_DE, "CRT Emulator Einstellungen..." },
/* es */ { IDMS_CRT_EMULATION_SETTINGS_ES, "Ajustes de emulación CRT..." },
/* fr */ { IDMS_CRT_EMULATION_SETTINGS_FR, "Paramètres d'émulation CRT..." },
/* hu */ { IDMS_CRT_EMULATION_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_CRT_EMULATION_SETTINGS_IT, "Impostazioni emulazione CRT..." },
/* ko */ { IDMS_CRT_EMULATION_SETTINGS_KO, "ÄÉÅäµå ·¹ÀÌ ÅÍ¹Ì³Î ¼³Á¤..." },
/* nl */ { IDMS_CRT_EMULATION_SETTINGS_NL, "CRT emulatie instellingen..." },
/* pl */ { IDMS_CRT_EMULATION_SETTINGS_PL, "Ustawienia emulacji CRT..." },
/* ru */ { IDMS_CRT_EMULATION_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_CRT_EMULATION_SETTINGS_SV, "Inställningar för CRT-emulering..." },
/* tr */ { IDMS_CRT_EMULATION_SETTINGS_TR, "CRT emülasyonu ayarlarý..." },

/* en */ { IDMS_VIC20_SETTINGS,    "VIC20 settings..." },
/* da */ { IDMS_VIC20_SETTINGS_DA, "VIC20-indstillinger..." },
/* de */ { IDMS_VIC20_SETTINGS_DE, "VIC20 Einstellungen..." },
/* es */ { IDMS_VIC20_SETTINGS_ES, "Ajustes VIC20..." },
/* fr */ { IDMS_VIC20_SETTINGS_FR, "Paramètres VIC20..." },
/* hu */ { IDMS_VIC20_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_VIC20_SETTINGS_IT, "Impostazioni VIC20..." },
/* ko */ { IDMS_VIC20_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_VIC20_SETTINGS_NL, "VIC20 instellingen..." },
/* pl */ { IDMS_VIC20_SETTINGS_PL, "Ustawienia VIC20..." },
/* ru */ { IDMS_VIC20_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_VIC20_SETTINGS_SV, "VIC20-inställningar..." },
/* tr */ { IDMS_VIC20_SETTINGS_TR, "VIC20 ayarlarý..." },

/* en */ { IDMS_CBM2_SETTINGS,    "CBM2 settings..." },
/* da */ { IDMS_CBM2_SETTINGS_DA, "CBM2-indstillinger..." },
/* de */ { IDMS_CBM2_SETTINGS_DE, "CBM2 Einstellungen..." },
/* es */ { IDMS_CBM2_SETTINGS_ES, "Ajustes CBM2..." },
/* fr */ { IDMS_CBM2_SETTINGS_FR, "Paramètres CBM2..." },
/* hu */ { IDMS_CBM2_SETTINGS_HU, "CBM2 beállításai..." },
/* it */ { IDMS_CBM2_SETTINGS_IT, "Impostazioni CBM2..." },
/* ko */ { IDMS_CBM2_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_CBM2_SETTINGS_NL, "CBM2 instellingen..." },
/* pl */ { IDMS_CBM2_SETTINGS_PL, "Ustawienia CBM2..." },
/* ru */ { IDMS_CBM2_SETTINGS_RU, "CBM2 settings..." },
/* sv */ { IDMS_CBM2_SETTINGS_SV, "CBM2-inställningar..." },
/* tr */ { IDMS_CBM2_SETTINGS_TR, "CBM2 ayarlarý..." },

/* en */ { IDMS_CBM5X0_SETTINGS,    "CBM5x0 settings..." },
/* da */ { IDMS_CBM5X0_SETTINGS_DA, "CBM5x0-indstillinger..." },
/* de */ { IDMS_CBM5X0_SETTINGS_DE, "CBM5x0 Einstellungen..." },
/* es */ { IDMS_CBM5X0_SETTINGS_ES, "Ajustes CBM5x0..." },
/* fr */ { IDMS_CBM5X0_SETTINGS_FR, "Paramètres CBM5x0..." },
/* hu */ { IDMS_CBM5X0_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_CBM5X0_SETTINGS_IT, "Impostazioni CBM5x0..." },
/* ko */ { IDMS_CBM5X0_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_CBM5X0_SETTINGS_NL, "CBM5x0 instellingen..." },
/* pl */ { IDMS_CBM5X0_SETTINGS_PL, "Ustawienia CBM5x0..." },
/* ru */ { IDMS_CBM5X0_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_CBM5X0_SETTINGS_SV, "CBM5x0-inställningar..." },
/* tr */ { IDMS_CBM5X0_SETTINGS_TR, "CBM5x0 ayarlarý..." },

/* en */ { IDMS_PET_SETTINGS,    "PET settings..." },
/* da */ { IDMS_PET_SETTINGS_DA, "PET-indstillinger..." },
/* de */ { IDMS_PET_SETTINGS_DE, "PET Einstellungen..." },
/* es */ { IDMS_PET_SETTINGS_ES, "Ajustes PET..." },
/* fr */ { IDMS_PET_SETTINGS_FR, "Paramètres PET..." },
/* hu */ { IDMS_PET_SETTINGS_HU, "PET beállításai..." },
/* it */ { IDMS_PET_SETTINGS_IT, "Impostazioni PET..." },
/* ko */ { IDMS_PET_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_PET_SETTINGS_NL, "PET instellingen..." },
/* pl */ { IDMS_PET_SETTINGS_PL, "Ustawienia PET..." },
/* ru */ { IDMS_PET_SETTINGS_RU, "PET settings..." },
/* sv */ { IDMS_PET_SETTINGS_SV, "PET-inställningar..." },
/* tr */ { IDMS_PET_SETTINGS_TR, "PET ayarlarý..." },

/* en */ { IDMS_PERIPHERAL_SETTINGS,    "Peripheral settings..." },
/* da */ { IDMS_PERIPHERAL_SETTINGS_DA, "Eksterne enhedsindstillinger..." },
/* de */ { IDMS_PERIPHERAL_SETTINGS_DE, "Peripherie Einstellungen..." },
/* es */ { IDMS_PERIPHERAL_SETTINGS_ES, "Ajustes de periféricos..." },
/* fr */ { IDMS_PERIPHERAL_SETTINGS_FR, "Paramètres des périphériques..." },
/* hu */ { IDMS_PERIPHERAL_SETTINGS_HU, "Perifériák beállításai..." },
/* it */ { IDMS_PERIPHERAL_SETTINGS_IT, "Impostazioni periferiche..." },
/* ko */ { IDMS_PERIPHERAL_SETTINGS_KO, "ÁÖº¯ÀåÄ¡ ¼ÂÆÃ..." },
/* nl */ { IDMS_PERIPHERAL_SETTINGS_NL, "Randapparaten instellingen..." },
/* pl */ { IDMS_PERIPHERAL_SETTINGS_PL, "Ustawienia zewnêtrzne..." },
/* ru */ { IDMS_PERIPHERAL_SETTINGS_RU, "Peripheral settings..." },
/* sv */ { IDMS_PERIPHERAL_SETTINGS_SV, "Inställningar för kringutrustning..." },
/* tr */ { IDMS_PERIPHERAL_SETTINGS_TR, "Çevresel ayarlar..." },

/* en */ { IDMS_DRIVE_SETTINGS,    "Drive settings..." },
/* da */ { IDMS_DRIVE_SETTINGS_DA, "Diskettedrevsindstillinger..." },
/* de */ { IDMS_DRIVE_SETTINGS_DE, "Floppy Einstellungen..." },
/* es */ { IDMS_DRIVE_SETTINGS_ES, "Ajustes unidad disco..." },
/* fr */ { IDMS_DRIVE_SETTINGS_FR, "Paramètres des lecteurs..." },
/* hu */ { IDMS_DRIVE_SETTINGS_HU, "Lemezegység beállításai..." },
/* it */ { IDMS_DRIVE_SETTINGS_IT, "Impostazioni drive..." },
/* ko */ { IDMS_DRIVE_SETTINGS_KO, "µå¶óÀÌºê ¼ÂÆÃ..." },
/* nl */ { IDMS_DRIVE_SETTINGS_NL, "Driveinstellingen..." },
/* pl */ { IDMS_DRIVE_SETTINGS_PL, "Ustawienia napêdu..." },
/* ru */ { IDMS_DRIVE_SETTINGS_RU, "Drive settings..." },
/* sv */ { IDMS_DRIVE_SETTINGS_SV, "Diskettenhetsinställningar..." },
/* tr */ { IDMS_DRIVE_SETTINGS_TR, "Sürücü ayarlarý..." },

/* en */ { IDMS_PRINTER_SETTINGS,    "Printer settings..." },
/* da */ { IDMS_PRINTER_SETTINGS_DA, "Printerindstillinger..." },
/* de */ { IDMS_PRINTER_SETTINGS_DE, "Drucker Einstellungen..." },
/* es */ { IDMS_PRINTER_SETTINGS_ES, "Ajustes de impresora..." },
/* fr */ { IDMS_PRINTER_SETTINGS_FR, "Paramètres d'imprimante..." },
/* hu */ { IDMS_PRINTER_SETTINGS_HU, "Nyomtató beállításai..." },
/* it */ { IDMS_PRINTER_SETTINGS_IT, "Impostazioni stampanti..." },
/* ko */ { IDMS_PRINTER_SETTINGS_KO, "ÇÁ¸°ÅÍ ¼ÂÆÃ..." },
/* nl */ { IDMS_PRINTER_SETTINGS_NL, "Printer instellingen..." },
/* pl */ { IDMS_PRINTER_SETTINGS_PL, "Ustawienia drukarki..." },
/* ru */ { IDMS_PRINTER_SETTINGS_RU, "½ÐáâàÞÙÚØ ßàØÝâÕàÐ..." },
/* sv */ { IDMS_PRINTER_SETTINGS_SV, "Skrivarinställningar..." },
/* tr */ { IDMS_PRINTER_SETTINGS_TR, "Yazýcý ayarlarý..." },

/* en */ { IDMS_DATASETTE_SETTINGS,    "Datasette settings..." },
/* da */ { IDMS_DATASETTE_SETTINGS_DA, "Datasetteindstillinger..." },
/* de */ { IDMS_DATASETTE_SETTINGS_DE, "Datasette Einstellungen..." },
/* es */ { IDMS_DATASETTE_SETTINGS_ES, "Ajustes Datasette..." },
/* fr */ { IDMS_DATASETTE_SETTINGS_FR, "Paramètres datassette..." },
/* hu */ { IDMS_DATASETTE_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_DATASETTE_SETTINGS_IT, "Impostazioni registratore..." },
/* ko */ { IDMS_DATASETTE_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_DATASETTE_SETTINGS_NL, "Datasette instellingen..." },
/* pl */ { IDMS_DATASETTE_SETTINGS_PL, "Ustawienia magnetofonu..." },
/* ru */ { IDMS_DATASETTE_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_DATASETTE_SETTINGS_SV, "Datasetteinställningar..." },
/* tr */ { IDMS_DATASETTE_SETTINGS_TR, "Teyp Ayarlarý..." },

/* en */ { IDMS_PLUS4_SETTINGS,    "Plus4 settings..." },
/* da */ { IDMS_PLUS4_SETTINGS_DA, "Plus4-indstillinger..." },
/* de */ { IDMS_PLUS4_SETTINGS_DE, "Plus4 Einstellungen..." },
/* es */ { IDMS_PLUS4_SETTINGS_ES, "Ajustes Plus4..." },
/* fr */ { IDMS_PLUS4_SETTINGS_FR, "Paramètres Plus4..." },
/* hu */ { IDMS_PLUS4_SETTINGS_HU, "Plus4 beállításai..." },
/* it */ { IDMS_PLUS4_SETTINGS_IT, "Impostazioni Plus4..." },
/* ko */ { IDMS_PLUS4_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_PLUS4_SETTINGS_NL, "Plus4 instellingen..." },
/* pl */ { IDMS_PLUS4_SETTINGS_PL, "Ustawienia Plus4..." },
/* ru */ { IDMS_PLUS4_SETTINGS_RU, "Plus4 settings..." },
/* sv */ { IDMS_PLUS4_SETTINGS_SV, "Plus4-inställningar..." },
/* tr */ { IDMS_PLUS4_SETTINGS_TR, "Plus4 ayarlarý..." },

/* en */ { IDMS_VICII_SETTINGS,    "VIC-II settings..." },
/* da */ { IDMS_VICII_SETTINGS_DA, "VIC-II-indstillinger..." },
/* de */ { IDMS_VICII_SETTINGS_DE, "VIC-II Einstellungen..." },
/* es */ { IDMS_VICII_SETTINGS_ES, "Ajustes VIC-II..." },
/* fr */ { IDMS_VICII_SETTINGS_FR, "Paramètres VIC-II..." },
/* hu */ { IDMS_VICII_SETTINGS_HU, "VIC-II beállításai..." },
/* it */ { IDMS_VICII_SETTINGS_IT, "Impostazioni VIC-II..." },
/* ko */ { IDMS_VICII_SETTINGS_KO, "VIC-II ¼³Á¤..." },
/* nl */ { IDMS_VICII_SETTINGS_NL, "VIC-II-instellingen..." },
/* pl */ { IDMS_VICII_SETTINGS_PL, "Ustawienia VIC-II..." },
/* ru */ { IDMS_VICII_SETTINGS_RU, "½ÐáâàÞÙÚØ VIC-II..." },
/* sv */ { IDMS_VICII_SETTINGS_SV, "VIC-II-inställningar..." },
/* tr */ { IDMS_VICII_SETTINGS_TR, "VIC-II ayarlarý..." },

/* en */ { IDMS_SCPU64_SETTINGS,    "SuperCPU64 settings..." },
/* da */ { IDMS_SCPU64_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDMS_SCPU64_SETTINGS_DE, "SuperCPU64 Einstellungen..." },
/* es */ { IDMS_SCPU64_SETTINGS_ES, "Ajustes SuperCPU64..." },
/* fr */ { IDMS_SCPU64_SETTINGS_FR, "Paramètres SuperCPU64..." },
/* hu */ { IDMS_SCPU64_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_SCPU64_SETTINGS_IT, "Impostazioni SuperCPU64..." },
/* ko */ { IDMS_SCPU64_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_SCPU64_SETTINGS_NL, "SuperCPU64 instellingen..." },
/* pl */ { IDMS_SCPU64_SETTINGS_PL, "Ustawienia SuperCPU64..." },
/* ru */ { IDMS_SCPU64_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_SCPU64_SETTINGS_SV, "SuperCPU64-inställningar..." },
/* tr */ { IDMS_SCPU64_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDMS_VIC_SETTINGS,    "VIC settings..." },
/* da */ { IDMS_VIC_SETTINGS_DA, "VIC-indstillinger..." },
/* de */ { IDMS_VIC_SETTINGS_DE, "VIC Einstellungen..." },
/* es */ { IDMS_VIC_SETTINGS_ES, "Ajustes VIC..." },
/* fr */ { IDMS_VIC_SETTINGS_FR, "Paramètres VIC..." },
/* hu */ { IDMS_VIC_SETTINGS_HU, "VIC beállításai..." },
/* it */ { IDMS_VIC_SETTINGS_IT, "Impostazioni VIC..." },
/* ko */ { IDMS_VIC_SETTINGS_KO, "VIC ¼ÂÆÃ..." },
/* nl */ { IDMS_VIC_SETTINGS_NL, "VIC instellingen..." },
/* pl */ { IDMS_VIC_SETTINGS_PL, "Ustawienia VIC..." },
/* ru */ { IDMS_VIC_SETTINGS_RU, "VIC settings..." },
/* sv */ { IDMS_VIC_SETTINGS_SV, "VIC-inställningar..." },
/* tr */ { IDMS_VIC_SETTINGS_TR, "VIC ayarlarý..." },

/* en */ { IDMS_TED_SETTINGS,    "TED settings..." },
/* da */ { IDMS_TED_SETTINGS_DA, "TED-indstillinger..." },
/* de */ { IDMS_TED_SETTINGS_DE, "TED Einstellungen..." },
/* es */ { IDMS_TED_SETTINGS_ES, "Ajustes TED..." },
/* fr */ { IDMS_TED_SETTINGS_FR, "Paramètres TED..." },
/* hu */ { IDMS_TED_SETTINGS_HU, "TED beállításai..." },
/* it */ { IDMS_TED_SETTINGS_IT, "Impostazioni TED..." },
/* ko */ { IDMS_TED_SETTINGS_KO, "TED ¼ÂÆÃ..." },
/* nl */ { IDMS_TED_SETTINGS_NL, "TED-instellingen..." },
/* pl */ { IDMS_TED_SETTINGS_PL, "Ustawienia TED..." },
/* ru */ { IDMS_TED_SETTINGS_RU, "½ÐáâàÞÙÚØ TED..." },
/* sv */ { IDMS_TED_SETTINGS_SV, "TED-inställningar..." },
/* tr */ { IDMS_TED_SETTINGS_TR, "TED ayarlarý..." },

/* en */ { IDMS_JOYPORT_SETTINGS,    "Control port settings..." },
/* da */ { IDMS_JOYPORT_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDMS_JOYPORT_SETTINGS_DE, "Control Port Einstellungen..." },
/* es */ { IDMS_JOYPORT_SETTINGS_ES, "" },  /* fuzzy */
/* fr */ { IDMS_JOYPORT_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_JOYPORT_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_JOYPORT_SETTINGS_IT, "" },  /* fuzzy */
/* ko */ { IDMS_JOYPORT_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_JOYPORT_SETTINGS_NL, "Spel connector instellingen..." },
/* pl */ { IDMS_JOYPORT_SETTINGS_PL, "" },  /* fuzzy */
/* ru */ { IDMS_JOYPORT_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_JOYPORT_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDMS_JOYPORT_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDMS_JOYSTICK_SETTINGS,    "Joystick settings..." },
/* da */ { IDMS_JOYSTICK_SETTINGS_DA, "Joystick indstillinger..." },
/* de */ { IDMS_JOYSTICK_SETTINGS_DE, "Joystick Einstellungen..." },
/* es */ { IDMS_JOYSTICK_SETTINGS_ES, "Ajustes de joystick..." },
/* fr */ { IDMS_JOYSTICK_SETTINGS_FR, "Paramètres des joysticks..." },
/* hu */ { IDMS_JOYSTICK_SETTINGS_HU, "Botkormány beállításai..." },
/* it */ { IDMS_JOYSTICK_SETTINGS_IT, "Impostazioni joystick..." },
/* ko */ { IDMS_JOYSTICK_SETTINGS_KO, "Á¶ÀÌ½ºÆ½ ¼³Á¤..." },
/* nl */ { IDMS_JOYSTICK_SETTINGS_NL, "Joystick instellingen..." },
/* pl */ { IDMS_JOYSTICK_SETTINGS_PL, "Ustawienia joysticka..." },
/* ru */ { IDMS_JOYSTICK_SETTINGS_RU, "½ÐáâàÞÙÚØ ÔÖÞÙáâØÚÐ..." },
/* sv */ { IDMS_JOYSTICK_SETTINGS_SV, "Inställningar för styrspak..." },
/* tr */ { IDMS_JOYSTICK_SETTINGS_TR, "Joystick ayarlarý..." },

/* en */ { IDMS_JOYSTICK_DEVICE_SELECT,    "Joystick device selection" },
/* da */ { IDMS_JOYSTICK_DEVICE_SELECT_DA, "Joystick enheds-valg" },
/* de */ { IDMS_JOYSTICK_DEVICE_SELECT_DE, "Joystick Geräte Auswahl" },
/* es */ { IDMS_JOYSTICK_DEVICE_SELECT_ES, "Selección periférico joystick" },
/* fr */ { IDMS_JOYSTICK_DEVICE_SELECT_FR, "Sélection des périphériques Joystick" },
/* hu */ { IDMS_JOYSTICK_DEVICE_SELECT_HU, "Botkormány eszköz kiválasztása" },
/* it */ { IDMS_JOYSTICK_DEVICE_SELECT_IT, "Selezione dispositivo joystick" },
/* ko */ { IDMS_JOYSTICK_DEVICE_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDMS_JOYSTICK_DEVICE_SELECT_NL, "Joystickapparaatselectie" },
/* pl */ { IDMS_JOYSTICK_DEVICE_SELECT_PL, "Wybór urz±dzenia joysticka" },
/* ru */ { IDMS_JOYSTICK_DEVICE_SELECT_RU, "Joystick device selection" },
/* sv */ { IDMS_JOYSTICK_DEVICE_SELECT_SV, "Välj styrspakenhet" },
/* tr */ { IDMS_JOYSTICK_DEVICE_SELECT_TR, "Joystick aygýt seçimi" },

/* en */ { IDMS_JOYSTICK_FIRE_SELECT,    "Joystick fire button selection" },
/* da */ { IDMS_JOYSTICK_FIRE_SELECT_DA, "Vælg fireknap for joystick" },
/* de */ { IDMS_JOYSTICK_FIRE_SELECT_DE, "Joystick Feuerknopf Wahl" },
/* es */ { IDMS_JOYSTICK_FIRE_SELECT_ES, "Selección botón disparo joystick" },
/* fr */ { IDMS_JOYSTICK_FIRE_SELECT_FR, "Sélection du bouton FEU du Joystick" },
/* hu */ { IDMS_JOYSTICK_FIRE_SELECT_HU, "Botkormány tûzgomb kiválasztás" },
/* it */ { IDMS_JOYSTICK_FIRE_SELECT_IT, "Selezione pulsante di fuoco del joystick" },
/* ko */ { IDMS_JOYSTICK_FIRE_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDMS_JOYSTICK_FIRE_SELECT_NL, "Joystickvuurknopselectie" },
/* pl */ { IDMS_JOYSTICK_FIRE_SELECT_PL, "Wybór przycisku fire joysticka" },
/* ru */ { IDMS_JOYSTICK_FIRE_SELECT_RU, "Joystick fire button selection" },
/* sv */ { IDMS_JOYSTICK_FIRE_SELECT_SV, "Välj fireknapp för styrspak" },
/* tr */ { IDMS_JOYSTICK_FIRE_SELECT_TR, "Joystick ateþ düþmesi seçimi" },

/* en */ { IDMS_KEYBOARD_SETTINGS,    "Keyboard settings..." },
/* da */ { IDMS_KEYBOARD_SETTINGS_DA, "Tastaturindstillinger..." },
/* de */ { IDMS_KEYBOARD_SETTINGS_DE, "Tastatur Einstellungen..." },
/* es */ { IDMS_KEYBOARD_SETTINGS_ES, "Ajustes de teclado..." },
/* fr */ { IDMS_KEYBOARD_SETTINGS_FR, "Paramètres du clavier..." },
/* hu */ { IDMS_KEYBOARD_SETTINGS_HU, "Billentyûzet beállításai..." },
/* it */ { IDMS_KEYBOARD_SETTINGS_IT, "Impostazioni tastiera..." },
/* ko */ { IDMS_KEYBOARD_SETTINGS_KO, "Å°º¸µå ¼³Á¤..." },
/* nl */ { IDMS_KEYBOARD_SETTINGS_NL, "Toetsenbord instellingen..." },
/* pl */ { IDMS_KEYBOARD_SETTINGS_PL, "Ustawienia klawiatury..." },
/* ru */ { IDMS_KEYBOARD_SETTINGS_RU, "½ÐáâàÞÙÚØ ÚÛÐÒØÐâãàë..." },
/* sv */ { IDMS_KEYBOARD_SETTINGS_SV, "Tangentbordsinställningar..." },
/* tr */ { IDMS_KEYBOARD_SETTINGS_TR, "Klavye ayarlarý..." },

/* en */ { IDMS_MOUSE_SETTINGS,    "Mouse settings..." },
/* da */ { IDMS_MOUSE_SETTINGS_DA, "Musseindstillinger..." },
/* de */ { IDMS_MOUSE_SETTINGS_DE, "Maus Einstellungen..." },
/* es */ { IDMS_MOUSE_SETTINGS_ES, "Ajustes ratón..." },
/* fr */ { IDMS_MOUSE_SETTINGS_FR, "Paramètres de souris..." },
/* hu */ { IDMS_MOUSE_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_MOUSE_SETTINGS_IT, "Impostazioni mouse..." },
/* ko */ { IDMS_MOUSE_SETTINGS_KO, "¸¶¿ì½º ¼ÂÆÃ..." },
/* nl */ { IDMS_MOUSE_SETTINGS_NL, "Muis instellingen..." },
/* pl */ { IDMS_MOUSE_SETTINGS_PL, "Ustawienia myszy..." },
/* ru */ { IDMS_MOUSE_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_MOUSE_SETTINGS_SV, "Musinställningar..." },
/* tr */ { IDMS_MOUSE_SETTINGS_TR, "Mouse Ayarlarý..." },

/* en */ { IDMS_SOUND_SETTINGS,    "Sound settings..." },
/* da */ { IDMS_SOUND_SETTINGS_DA, "Lydindstillinger..." },
/* de */ { IDMS_SOUND_SETTINGS_DE, "Sound Einstellungen..." },
/* es */ { IDMS_SOUND_SETTINGS_ES, "Ajustes de sonido..." },
/* fr */ { IDMS_SOUND_SETTINGS_FR, "Paramètres son..." },
/* hu */ { IDMS_SOUND_SETTINGS_HU, "Hang beállításai..." },
/* it */ { IDMS_SOUND_SETTINGS_IT, "Impostazioni audio..." },
/* ko */ { IDMS_SOUND_SETTINGS_KO, "¼Ò¸® ¼³Á¤..." },
/* nl */ { IDMS_SOUND_SETTINGS_NL, "Geluidsinstellingen..." },
/* pl */ { IDMS_SOUND_SETTINGS_PL, "Ustawienia d¼wiêku..." },
/* ru */ { IDMS_SOUND_SETTINGS_RU, "½ÐáâàÞÙÚØ ×ÒãÚÐ..." },
/* sv */ { IDMS_SOUND_SETTINGS_SV, "Ljudinställningar..." },
/* tr */ { IDMS_SOUND_SETTINGS_TR, "Ses ayarlarý..." },

/* en */ { IDMS_SAMPLER_SETTINGS,    "Sampler settings..." },
/* da */ { IDMS_SAMPLER_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDMS_SAMPLER_SETTINGS_DE, "Sampler Einstellungen..." },
/* es */ { IDMS_SAMPLER_SETTINGS_ES, "" },  /* fuzzy */
/* fr */ { IDMS_SAMPLER_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_SAMPLER_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_SAMPLER_SETTINGS_IT, "" },  /* fuzzy */
/* ko */ { IDMS_SAMPLER_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_SAMPLER_SETTINGS_NL, "Sampler instellingen..." },
/* pl */ { IDMS_SAMPLER_SETTINGS_PL, "" },  /* fuzzy */
/* ru */ { IDMS_SAMPLER_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_SAMPLER_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDMS_SAMPLER_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDMS_SID_SETTINGS,    "SID settings..." },
/* da */ { IDMS_SID_SETTINGS_DA, "SID-indstillinger..." },
/* de */ { IDMS_SID_SETTINGS_DE, "SID Einstellungen..." },
/* es */ { IDMS_SID_SETTINGS_ES, "Ajustes SID..." },
/* fr */ { IDMS_SID_SETTINGS_FR, "Paramètres SID..." },
/* hu */ { IDMS_SID_SETTINGS_HU, "SID beállításai..." },
/* it */ { IDMS_SID_SETTINGS_IT, "Impostazioni SID..." },
/* ko */ { IDMS_SID_SETTINGS_KO, "SID ¼³Á¤..." },
/* nl */ { IDMS_SID_SETTINGS_NL, "SID-instellingen..." },
/* pl */ { IDMS_SID_SETTINGS_PL, "Ustawienia SID..." },
/* ru */ { IDMS_SID_SETTINGS_RU, "½ÐáâàÞÙÚØ SID..." },
/* sv */ { IDMS_SID_SETTINGS_SV, "SID-inställningar..." },
/* tr */ { IDMS_SID_SETTINGS_TR, "SID ayarlarý..." },

/* en */ { IDMS_CIA_SETTINGS,    "CIA settings..." },
/* da */ { IDMS_CIA_SETTINGS_DA, "CIA-indstillinger..." },
/* de */ { IDMS_CIA_SETTINGS_DE, "CIA Einstellungen..." },
/* es */ { IDMS_CIA_SETTINGS_ES, "Ajustes CIA..." },
/* fr */ { IDMS_CIA_SETTINGS_FR, "Paramètres CIA..." },
/* hu */ { IDMS_CIA_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_CIA_SETTINGS_IT, "Impostazioni CIA..." },
/* ko */ { IDMS_CIA_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_CIA_SETTINGS_NL, "CIA instellingen..." },
/* pl */ { IDMS_CIA_SETTINGS_PL, "Ustawienia CIA..." },
/* ru */ { IDMS_CIA_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_CIA_SETTINGS_SV, "CIA-inställningar..." },
/* tr */ { IDMS_CIA_SETTINGS_TR, "CIA ayarlarý..." },

/* en */ { IDMS_COMPUTER_ROM_SETTINGS,    "Computer ROM settings..." },
/* da */ { IDMS_COMPUTER_ROM_SETTINGS_DA, "Computer ROM-indstillinger..." },
/* de */ { IDMS_COMPUTER_ROM_SETTINGS_DE, "Computer ROM Einstellungen..." },
/* es */ { IDMS_COMPUTER_ROM_SETTINGS_ES, "Ajustes ROM ordenador..." },
/* fr */ { IDMS_COMPUTER_ROM_SETTINGS_FR, "Paramètres ROM ordinateur..." },
/* hu */ { IDMS_COMPUTER_ROM_SETTINGS_HU, "Számítógép ROM beállításai..." },
/* it */ { IDMS_COMPUTER_ROM_SETTINGS_IT, "Impostazioni ROM del computer..." },
/* ko */ { IDMS_COMPUTER_ROM_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_COMPUTER_ROM_SETTINGS_NL, "Computer ROM instellingen..." },
/* pl */ { IDMS_COMPUTER_ROM_SETTINGS_PL, "Ustawienia ROM-u komputera..." },
/* ru */ { IDMS_COMPUTER_ROM_SETTINGS_RU, "Computer ROM settings..." },
/* sv */ { IDMS_COMPUTER_ROM_SETTINGS_SV, "ROM-inställningar för dator..." },
/* tr */ { IDMS_COMPUTER_ROM_SETTINGS_TR, "Bilgisayar ROM ayarlarý..." },

/* en */ { IDMS_DRIVE_ROM_SETTINGS,    "Drive ROM settings..." },
/* da */ { IDMS_DRIVE_ROM_SETTINGS_DA, "Drev ROM-indstillinger..." },
/* de */ { IDMS_DRIVE_ROM_SETTINGS_DE, "Floppy ROM Einstellungen..." },
/* es */ { IDMS_DRIVE_ROM_SETTINGS_ES, "Ajustes ROM unidad disco..." },
/* fr */ { IDMS_DRIVE_ROM_SETTINGS_FR, "Paramètres ROM Lecteur..." },
/* hu */ { IDMS_DRIVE_ROM_SETTINGS_HU, "Lemezegység ROM beállításai..." },
/* it */ { IDMS_DRIVE_ROM_SETTINGS_IT, "Impostazioni ROM del drive..." },
/* ko */ { IDMS_DRIVE_ROM_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_DRIVE_ROM_SETTINGS_NL, "Drive ROM instellingen..." },
/* pl */ { IDMS_DRIVE_ROM_SETTINGS_PL, "Ustawienia ROM-u napêdu..." },
/* ru */ { IDMS_DRIVE_ROM_SETTINGS_RU, "Drive ROM settings..." },
/* sv */ { IDMS_DRIVE_ROM_SETTINGS_SV, "ROM-inställningar för diskettenhet..." },
/* tr */ { IDMS_DRIVE_ROM_SETTINGS_TR, "Sürücü ROM ayarlarý..." },

/* en */ { IDMS_RAM_SETTINGS,    "RAM settings..." },
/* da */ { IDMS_RAM_SETTINGS_DA, "RAM-indstillinger..." },
/* de */ { IDMS_RAM_SETTINGS_DE, "RAM Einstellungen..." },
/* es */ { IDMS_RAM_SETTINGS_ES, "Ajustes RAM..." },
/* fr */ { IDMS_RAM_SETTINGS_FR, "Paramètres de la RAM..." },
/* hu */ { IDMS_RAM_SETTINGS_HU, "RAM beállításai..." },
/* it */ { IDMS_RAM_SETTINGS_IT, "Impostazioni RAM..." },
/* ko */ { IDMS_RAM_SETTINGS_KO, "RAM ¼ÂÆÃ..." },
/* nl */ { IDMS_RAM_SETTINGS_NL, "RAM-instellingen..." },
/* pl */ { IDMS_RAM_SETTINGS_PL, "Ustawienia RAM..." },
/* ru */ { IDMS_RAM_SETTINGS_RU, "½ÐáâàÞÙÚØ RAM..." },
/* sv */ { IDMS_RAM_SETTINGS_SV, "RAM-inställningar..." },
/* tr */ { IDMS_RAM_SETTINGS_TR, "RAM ayarlarý..." },

/* en */ { IDMS_RS232_SETTINGS,    "RS232 settings..." },
/* da */ { IDMS_RS232_SETTINGS_DA, "RS232-indstillinger..." },
/* de */ { IDMS_RS232_SETTINGS_DE, "RS232 Einstellungen..." },
/* es */ { IDMS_RS232_SETTINGS_ES, "Ajustes RS232..." },
/* fr */ { IDMS_RS232_SETTINGS_FR, "Paramètres RS232..." },
/* hu */ { IDMS_RS232_SETTINGS_HU, "RS232 beállításai..." },
/* it */ { IDMS_RS232_SETTINGS_IT, "Impostazioni RS232..." },
/* ko */ { IDMS_RS232_SETTINGS_KO, "RS232 ¼³Á¤..." },
/* nl */ { IDMS_RS232_SETTINGS_NL, "RS232 instellingen..." },
/* pl */ { IDMS_RS232_SETTINGS_PL, "Ustawienia RS232..." },
/* ru */ { IDMS_RS232_SETTINGS_RU, "½ÐáâàÞÙÚØ RS232..." },
/* sv */ { IDMS_RS232_SETTINGS_SV, "RS232-inställningar..." },
/* tr */ { IDMS_RS232_SETTINGS_TR, "RS232 ayarlarý..." },

/* en */ { IDMS_C128_SETTINGS,    "C128 settings..." },
/* da */ { IDMS_C128_SETTINGS_DA, "C128-indstillinger..." },
/* de */ { IDMS_C128_SETTINGS_DE, "C128 Einstellungen..." },
/* es */ { IDMS_C128_SETTINGS_ES, "Ajustes C128..." },
/* fr */ { IDMS_C128_SETTINGS_FR, "Paramètres C128..." },
/* hu */ { IDMS_C128_SETTINGS_HU, "C128 beállításai..." },
/* it */ { IDMS_C128_SETTINGS_IT, "Impostazioni C128..." },
/* ko */ { IDMS_C128_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C128_SETTINGS_NL, "C128 instellingen..." },
/* pl */ { IDMS_C128_SETTINGS_PL, "Ustawienia C128..." },
/* ru */ { IDMS_C128_SETTINGS_RU, "½ÐáâàÞÙÚØ C128..." },
/* sv */ { IDMS_C128_SETTINGS_SV, "C128-inställningar..." },
/* tr */ { IDMS_C128_SETTINGS_TR, "C128 ayarlarý..." },

/* en */ { IDMS_USERPORT_DEVICE_SETTINGS,    "Userport device settings" },
/* da */ { IDMS_USERPORT_DEVICE_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDMS_USERPORT_DEVICE_SETTINGS_DE, "Userport Geräteeinstellungen" },
/* es */ { IDMS_USERPORT_DEVICE_SETTINGS_ES, "" },  /* fuzzy */
/* fr */ { IDMS_USERPORT_DEVICE_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_USERPORT_DEVICE_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_USERPORT_DEVICE_SETTINGS_IT, "" },  /* fuzzy */
/* ko */ { IDMS_USERPORT_DEVICE_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_USERPORT_DEVICE_SETTINGS_NL, "Userport apparaat instellingen" },
/* pl */ { IDMS_USERPORT_DEVICE_SETTINGS_PL, "" },  /* fuzzy */
/* ru */ { IDMS_USERPORT_DEVICE_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_USERPORT_DEVICE_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDMS_USERPORT_DEVICE_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDMS_TAPEPORT_DEVICE_SETTINGS,    "Tapeport device settings" },
/* da */ { IDMS_TAPEPORT_DEVICE_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDMS_TAPEPORT_DEVICE_SETTINGS_DE, "Bandport Geräteeinstellungen" },
/* es */ { IDMS_TAPEPORT_DEVICE_SETTINGS_ES, "" },  /* fuzzy */
/* fr */ { IDMS_TAPEPORT_DEVICE_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_TAPEPORT_DEVICE_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_TAPEPORT_DEVICE_SETTINGS_IT, "" },  /* fuzzy */
/* ko */ { IDMS_TAPEPORT_DEVICE_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_TAPEPORT_DEVICE_SETTINGS_NL, "Tapeport apparaat instellingen" },
/* pl */ { IDMS_TAPEPORT_DEVICE_SETTINGS_PL, "" },  /* fuzzy */
/* ru */ { IDMS_TAPEPORT_DEVICE_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_TAPEPORT_DEVICE_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDMS_TAPEPORT_DEVICE_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDMS_TAPELOG_SETTINGS,    "TapeLog device settings..." },
/* da */ { IDMS_TAPELOG_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDMS_TAPELOG_SETTINGS_DE, "Bandlog Geräteeinstellungen..." },
/* es */ { IDMS_TAPELOG_SETTINGS_ES, "" },  /* fuzzy */
/* fr */ { IDMS_TAPELOG_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_TAPELOG_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_TAPELOG_SETTINGS_IT, "" },  /* fuzzy */
/* ko */ { IDMS_TAPELOG_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_TAPELOG_SETTINGS_NL, "TapeLog apparaat instellingen..." },
/* pl */ { IDMS_TAPELOG_SETTINGS_PL, "" },  /* fuzzy */
/* ru */ { IDMS_TAPELOG_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_TAPELOG_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDMS_TAPELOG_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDMS_CPCLOCKF83_SETTINGS,    "CP CLock F83 RTC device settings..." },
/* da */ { IDMS_CPCLOCKF83_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDMS_CPCLOCKF83_SETTINGS_DE, "CP Clock F83 RTC Geräteeinstellungen..." },
/* es */ { IDMS_CPCLOCKF83_SETTINGS_ES, "" },  /* fuzzy */
/* fr */ { IDMS_CPCLOCKF83_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_CPCLOCKF83_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_CPCLOCKF83_SETTINGS_IT, "" },  /* fuzzy */
/* ko */ { IDMS_CPCLOCKF83_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_CPCLOCKF83_SETTINGS_NL, "CP CLock F83 RTC apparaat instellingen..." },
/* pl */ { IDMS_CPCLOCKF83_SETTINGS_PL, "" },  /* fuzzy */
/* ru */ { IDMS_CPCLOCKF83_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_CPCLOCKF83_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDMS_CPCLOCKF83_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDMS_ENABLE_TAPE_SENSE_DONGLE,    "Enable tape sense dongle" },
/* da */ { IDMS_ENABLE_TAPE_SENSE_DONGLE_DA, "" },  /* fuzzy */
/* de */ { IDMS_ENABLE_TAPE_SENSE_DONGLE_DE, "Band Sense Dongle aktivieren" },
/* es */ { IDMS_ENABLE_TAPE_SENSE_DONGLE_ES, "" },  /* fuzzy */
/* fr */ { IDMS_ENABLE_TAPE_SENSE_DONGLE_FR, "" },  /* fuzzy */
/* hu */ { IDMS_ENABLE_TAPE_SENSE_DONGLE_HU, "" },  /* fuzzy */
/* it */ { IDMS_ENABLE_TAPE_SENSE_DONGLE_IT, "" },  /* fuzzy */
/* ko */ { IDMS_ENABLE_TAPE_SENSE_DONGLE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_ENABLE_TAPE_SENSE_DONGLE_NL, "Tape sense dongle inschakelen" },
/* pl */ { IDMS_ENABLE_TAPE_SENSE_DONGLE_PL, "" },  /* fuzzy */
/* ru */ { IDMS_ENABLE_TAPE_SENSE_DONGLE_RU, "" },  /* fuzzy */
/* sv */ { IDMS_ENABLE_TAPE_SENSE_DONGLE_SV, "" },  /* fuzzy */
/* tr */ { IDMS_ENABLE_TAPE_SENSE_DONGLE_TR, "" },  /* fuzzy */

/* en */ { IDMS_ENABLE_DTL_BASIC_DONGLE,    "Enable DTL Basic dongle" },
/* da */ { IDMS_ENABLE_DTL_BASIC_DONGLE_DA, "" },  /* fuzzy */
/* de */ { IDMS_ENABLE_DTL_BASIC_DONGLE_DE, "DTL Basic Dongle aktivieren" },
/* es */ { IDMS_ENABLE_DTL_BASIC_DONGLE_ES, "" },  /* fuzzy */
/* fr */ { IDMS_ENABLE_DTL_BASIC_DONGLE_FR, "" },  /* fuzzy */
/* hu */ { IDMS_ENABLE_DTL_BASIC_DONGLE_HU, "" },  /* fuzzy */
/* it */ { IDMS_ENABLE_DTL_BASIC_DONGLE_IT, "" },  /* fuzzy */
/* ko */ { IDMS_ENABLE_DTL_BASIC_DONGLE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_ENABLE_DTL_BASIC_DONGLE_NL, "DTL Basic dongle inschakelen" },
/* pl */ { IDMS_ENABLE_DTL_BASIC_DONGLE_PL, "" },  /* fuzzy */
/* ru */ { IDMS_ENABLE_DTL_BASIC_DONGLE_RU, "" },  /* fuzzy */
/* sv */ { IDMS_ENABLE_DTL_BASIC_DONGLE_SV, "" },  /* fuzzy */
/* tr */ { IDMS_ENABLE_DTL_BASIC_DONGLE_TR, "" },  /* fuzzy */

/* en */ { IDMS_CART_IO_SETTINGS,    "Cartridge/IO settings" },
/* da */ { IDMS_CART_IO_SETTINGS_DA, "Modul/IO-indstillinger" },
/* de */ { IDMS_CART_IO_SETTINGS_DE, "Erweiterungsmodul I/O Einstellungen" },
/* es */ { IDMS_CART_IO_SETTINGS_ES, "Ajustes entrada/salida cartucho" },
/* fr */ { IDMS_CART_IO_SETTINGS_FR, "Paramètres E/S cartouche" },
/* hu */ { IDMS_CART_IO_SETTINGS_HU, "Cartridge/IO beállításai" },
/* it */ { IDMS_CART_IO_SETTINGS_IT, "Impostazioni cartuccia/IO" },
/* ko */ { IDMS_CART_IO_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_CART_IO_SETTINGS_NL, "Cartridge/IO instellingen" },
/* pl */ { IDMS_CART_IO_SETTINGS_PL, "Ustawienia kartrid¿a i we/wy" },
/* ru */ { IDMS_CART_IO_SETTINGS_RU, "Cartridge/IO settings" },
/* sv */ { IDMS_CART_IO_SETTINGS_SV, "Modul/IO-inställningar" },
/* tr */ { IDMS_CART_IO_SETTINGS_TR, "Kartuþ/GÇ ayarlarý" },

/* en */ { IDMS_REU_SETTINGS,    "REU settings..." },
/* da */ { IDMS_REU_SETTINGS_DA, "REU-indstillinger..." },
/* de */ { IDMS_REU_SETTINGS_DE, "REU Einstellungen..." },
/* es */ { IDMS_REU_SETTINGS_ES, "Ajustes REU..." },
/* fr */ { IDMS_REU_SETTINGS_FR, "Paramètres REU..." },
/* hu */ { IDMS_REU_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_REU_SETTINGS_IT, "Impostazioni REU..." },
/* ko */ { IDMS_REU_SETTINGS_KO, "REU ¼ÂÆÃ..." },
/* nl */ { IDMS_REU_SETTINGS_NL, "REU instellingen..." },
/* pl */ { IDMS_REU_SETTINGS_PL, "Ustawienia REU..." },
/* ru */ { IDMS_REU_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_REU_SETTINGS_SV, "REU-Inställningar..." },
/* tr */ { IDMS_REU_SETTINGS_TR, "REU Ayarlarý..." },

/* en */ { IDMS_MAGIC_VOICE_SETTINGS,    "Magic Voice settings..." },
/* da */ { IDMS_MAGIC_VOICE_SETTINGS_DA, "Magic Voice indstillinger..." },
/* de */ { IDMS_MAGIC_VOICE_SETTINGS_DE, "Magic Voice Einstellungen..." },
/* es */ { IDMS_MAGIC_VOICE_SETTINGS_ES, "Ajustes Magic Voice..." },
/* fr */ { IDMS_MAGIC_VOICE_SETTINGS_FR, "Paramètres Magic Voice..." },
/* hu */ { IDMS_MAGIC_VOICE_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_MAGIC_VOICE_SETTINGS_IT, "Impostazioni Magic Voice..." },
/* ko */ { IDMS_MAGIC_VOICE_SETTINGS_KO, "Magic Voice ¼³Á¤..." },
/* nl */ { IDMS_MAGIC_VOICE_SETTINGS_NL, "Magic Voice instellingen..." },
/* pl */ { IDMS_MAGIC_VOICE_SETTINGS_PL, "Ustawienia Magic Voice..." },
/* ru */ { IDMS_MAGIC_VOICE_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_MAGIC_VOICE_SETTINGS_SV, "Magic Voice-inställningar..." },
/* tr */ { IDMS_MAGIC_VOICE_SETTINGS_TR, "Magic Voice ayarlarý..." },

/* en */ { IDMS_GEORAM_SETTINGS,    "GEO-RAM settings..." },
/* da */ { IDMS_GEORAM_SETTINGS_DA, "GEO-RAM-indstillinger..." },
/* de */ { IDMS_GEORAM_SETTINGS_DE, "GEO-RAM Einstellungen..." },
/* es */ { IDMS_GEORAM_SETTINGS_ES, "Ajustes GEO-RAM..." },
/* fr */ { IDMS_GEORAM_SETTINGS_FR, "Paramètres GEO-RAM..." },
/* hu */ { IDMS_GEORAM_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_GEORAM_SETTINGS_IT, "Impostazioni GEO-RAM..." },
/* ko */ { IDMS_GEORAM_SETTINGS_KO, "GEO-RAM ¼ÂÆÃ..." },
/* nl */ { IDMS_GEORAM_SETTINGS_NL, "GEO-RAM instellingen..." },
/* pl */ { IDMS_GEORAM_SETTINGS_PL, "Ustawienia GEO-RAM..." },
/* ru */ { IDMS_GEORAM_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_GEORAM_SETTINGS_SV, "GEO-RAM-inställningar..." },
/* tr */ { IDMS_GEORAM_SETTINGS_TR, "GEO-RAM ayarlarý..." },

/* en */ { IDMS_GEORAM_SETTINGS_MASCUERADE,    "GEO-RAM settings (MasC=uerade)..." },
/* da */ { IDMS_GEORAM_SETTINGS_MASCUERADE_DA, "GEO-RAM-indstillinger (MasC=uerade)..." },
/* de */ { IDMS_GEORAM_SETTINGS_MASCUERADE_DE, "GEO-RAM Einstellungen (MasC=uerade)..." },
/* es */ { IDMS_GEORAM_SETTINGS_MASCUERADE_ES, "Ajustes GEO-RAM (MasC=uerade)..." },
/* fr */ { IDMS_GEORAM_SETTINGS_MASCUERADE_FR, "Paramètres GEO-RAM (MasC=uerade)..." },
/* hu */ { IDMS_GEORAM_SETTINGS_MASCUERADE_HU, "" },  /* fuzzy */
/* it */ { IDMS_GEORAM_SETTINGS_MASCUERADE_IT, "Impostazioni GEO-RAM (MasC=uerade)..." },
/* ko */ { IDMS_GEORAM_SETTINGS_MASCUERADE_KO, "GEO-RAM ¼ÂÆÃ (MasC=uerade)..." },
/* nl */ { IDMS_GEORAM_SETTINGS_MASCUERADE_NL, "GEO-RAM instellingen (MasC=uerade)..." },
/* pl */ { IDMS_GEORAM_SETTINGS_MASCUERADE_PL, "Ustawienia GEO-RAM (MasC=uerade)..." },
/* ru */ { IDMS_GEORAM_SETTINGS_MASCUERADE_RU, "" },  /* fuzzy */
/* sv */ { IDMS_GEORAM_SETTINGS_MASCUERADE_SV, "GEO-RAM-inställningar (MasC=uerade)..." },
/* tr */ { IDMS_GEORAM_SETTINGS_MASCUERADE_TR, "GEO-RAM ayarlarý (MasC=uerade)..." },

/* en */ { IDMS_RAMCART_SETTINGS,    "RamCart settings..." },
/* da */ { IDMS_RAMCART_SETTINGS_DA, "RamCart-indstillinger..." },
/* de */ { IDMS_RAMCART_SETTINGS_DE, "RamCart Einstellungen..." },
/* es */ { IDMS_RAMCART_SETTINGS_ES, "Ajustes RamCart..." },
/* fr */ { IDMS_RAMCART_SETTINGS_FR, "Paramètres RamCart..." },
/* hu */ { IDMS_RAMCART_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_RAMCART_SETTINGS_IT, "Impostazioni RamCart ..." },
/* ko */ { IDMS_RAMCART_SETTINGS_KO, "RamCart ¼ÂÆÃ..." },
/* nl */ { IDMS_RAMCART_SETTINGS_NL, "RamCart instellingen..." },
/* pl */ { IDMS_RAMCART_SETTINGS_PL, "Ustawienia RamCart..." },
/* ru */ { IDMS_RAMCART_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_RAMCART_SETTINGS_SV, "RamCart-inställningar..." },
/* tr */ { IDMS_RAMCART_SETTINGS_TR, "RamCart ayarlarý..." },

/* en */ { IDMS_DQBB_SETTINGS,    "Double Quick Brown Box settings..." },
/* da */ { IDMS_DQBB_SETTINGS_DA, "Double Quick Brown Box indstillinger..." },
/* de */ { IDMS_DQBB_SETTINGS_DE, "Double Quick Brown Box Einstellungen..." },
/* es */ { IDMS_DQBB_SETTINGS_ES, "Ajustes Double Quick Brown Box..." },
/* fr */ { IDMS_DQBB_SETTINGS_FR, "Paramètres Double Quick Brown Box..." },
/* hu */ { IDMS_DQBB_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_DQBB_SETTINGS_IT, "Impostazioni Double Quick Brown Box..." },
/* ko */ { IDMS_DQBB_SETTINGS_KO, "Double Quick Brown Box ¼ÂÆÃ..." },
/* nl */ { IDMS_DQBB_SETTINGS_NL, "Double Quick Brown Box instellingen..." },
/* pl */ { IDMS_DQBB_SETTINGS_PL, "Ustawienia Double Quick Brown Box..." },
/* ru */ { IDMS_DQBB_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_DQBB_SETTINGS_SV, "Inställninga för dubbel Quick Brown Box..." },
/* tr */ { IDMS_DQBB_SETTINGS_TR, "Double Quick Brown Box ayarlarý..." },

/* en */ { IDMS_ISEPIC_SETTINGS,    "ISEPIC settings..." },
/* da */ { IDMS_ISEPIC_SETTINGS_DA, "ISEPIC-indstillinger..." },
/* de */ { IDMS_ISEPIC_SETTINGS_DE, "ISEPIC Einstellungen..." },
/* es */ { IDMS_ISEPIC_SETTINGS_ES, "Ajustes ISEPIC..." },
/* fr */ { IDMS_ISEPIC_SETTINGS_FR, "Paramètres ISEPIC..." },
/* hu */ { IDMS_ISEPIC_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_ISEPIC_SETTINGS_IT, "Impostazioni ISEPIC..." },
/* ko */ { IDMS_ISEPIC_SETTINGS_KO, "ISEPIC ¼ÂÆÃ..." },
/* nl */ { IDMS_ISEPIC_SETTINGS_NL, "ISEPIC instellingen..." },
/* pl */ { IDMS_ISEPIC_SETTINGS_PL, "Ustawienia ISEPIC..." },
/* ru */ { IDMS_ISEPIC_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_ISEPIC_SETTINGS_SV, "ISEPIC-inställningar..." },
/* tr */ { IDMS_ISEPIC_SETTINGS_TR, "ISEPIC ayarlarý..." },

/* en */ { IDMS_EXPERT_SETTINGS,    "Expert Cartridge settings..." },
/* da */ { IDMS_EXPERT_SETTINGS_DA, "Expert Cartridge-indstillinger..." },
/* de */ { IDMS_EXPERT_SETTINGS_DE, "Expert Cartridge Einstellungen..." },
/* es */ { IDMS_EXPERT_SETTINGS_ES, "Ajustes Expert Cartrige..." },
/* fr */ { IDMS_EXPERT_SETTINGS_FR, "Paramètres Cartouche Expert..." },
/* hu */ { IDMS_EXPERT_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_EXPERT_SETTINGS_IT, "Impostazioni Expert Cartridge..." },
/* ko */ { IDMS_EXPERT_SETTINGS_KO, "Expert Ä«Æ®¸®Áö ¼ÂÆÃ..." },
/* nl */ { IDMS_EXPERT_SETTINGS_NL, "Expert Cartridge instellingen..." },
/* pl */ { IDMS_EXPERT_SETTINGS_PL, "Ustawienia Expert Cartridge..." },
/* ru */ { IDMS_EXPERT_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_EXPERT_SETTINGS_SV, "Inställningar för Expert Cartridge..." },
/* tr */ { IDMS_EXPERT_SETTINGS_TR, "Expert Kartuþu ayarlarý..." },

/* en */ { IDMS_C64_MEMORY_HACKS_SETTINGS,    "Memory expansion hacks settings..." },
/* da */ { IDMS_C64_MEMORY_HACKS_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDMS_C64_MEMORY_HACKS_SETTINGS_DE, "Speicher Erweiterungshack Einstellungen..." },
/* es */ { IDMS_C64_MEMORY_HACKS_SETTINGS_ES, "" },  /* fuzzy */
/* fr */ { IDMS_C64_MEMORY_HACKS_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_C64_MEMORY_HACKS_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_C64_MEMORY_HACKS_SETTINGS_IT, "" },  /* fuzzy */
/* ko */ { IDMS_C64_MEMORY_HACKS_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C64_MEMORY_HACKS_SETTINGS_NL, "Geheugen uitbreiding hacks instellingen..." },
/* pl */ { IDMS_C64_MEMORY_HACKS_SETTINGS_PL, "Ustawienia hacków rozszerzenia pamiêci..." },
/* ru */ { IDMS_C64_MEMORY_HACKS_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_C64_MEMORY_HACKS_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDMS_C64_MEMORY_HACKS_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDS_PLUS256K_FILENAME,    "PLUS256K file" },
/* da */ { IDS_PLUS256K_FILENAME_DA, "PLUS256K-fil" },
/* de */ { IDS_PLUS256K_FILENAME_DE, "PLUS256K Image Name" },
/* es */ { IDS_PLUS256K_FILENAME_ES, "Fichero PLUS256K" },
/* fr */ { IDS_PLUS256K_FILENAME_FR, "Fichier PLUS256K" },
/* hu */ { IDS_PLUS256K_FILENAME_HU, "PLUS256K fájl" },
/* it */ { IDS_PLUS256K_FILENAME_IT, "File PLUS256K" },
/* ko */ { IDS_PLUS256K_FILENAME_KO, "PLUS256K ÆÄÀÏ" },
/* nl */ { IDS_PLUS256K_FILENAME_NL, "PLUS256K bestand" },
/* pl */ { IDS_PLUS256K_FILENAME_PL, "Plik PLUS256K" },
/* ru */ { IDS_PLUS256K_FILENAME_RU, "PLUS256K file" },
/* sv */ { IDS_PLUS256K_FILENAME_SV, "PLUS256K-fil" },
/* tr */ { IDS_PLUS256K_FILENAME_TR, "PLUS256K dosyasý" },

/* en */ { IDS_PLUS256K_FILENAME_SELECT,    "Select file for PLUS256K" },
/* da */ { IDS_PLUS256K_FILENAME_SELECT_DA, "Vælg fil for PLUS256K" },
/* de */ { IDS_PLUS256K_FILENAME_SELECT_DE, "Datei für PLUS256K auswählen" },
/* es */ { IDS_PLUS256K_FILENAME_SELECT_ES, "Seleccionar fichero para PLUS256K" },
/* fr */ { IDS_PLUS256K_FILENAME_SELECT_FR, "Sélectionner fichier pour PLUS256K" },
/* hu */ { IDS_PLUS256K_FILENAME_SELECT_HU, "Válasszon fájlt a PLUS256K-hoz" },
/* it */ { IDS_PLUS256K_FILENAME_SELECT_IT, "Seleziona file per PLUS256K" },
/* ko */ { IDS_PLUS256K_FILENAME_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDS_PLUS256K_FILENAME_SELECT_NL, "Selecteer bestand voor PLUS256K" },
/* pl */ { IDS_PLUS256K_FILENAME_SELECT_PL, "Wybierz plik PLUS256K" },
/* ru */ { IDS_PLUS256K_FILENAME_SELECT_RU, "Select file for PLUS256K" },
/* sv */ { IDS_PLUS256K_FILENAME_SELECT_SV, "Ange fil för PLUS256K" },
/* tr */ { IDS_PLUS256K_FILENAME_SELECT_TR, "PLUS256K için dosya seçin" },

/* en */ { IDMS_IDE64_SETTINGS,    "IDE64 settings..." },
/* da */ { IDMS_IDE64_SETTINGS_DA, "IDE64-indstillinger..." },
/* de */ { IDMS_IDE64_SETTINGS_DE, "IDE64 Einstellungen..." },
/* es */ { IDMS_IDE64_SETTINGS_ES, "Ajustes IDE64..." },
/* fr */ { IDMS_IDE64_SETTINGS_FR, "Paramètres IDE64..." },
/* hu */ { IDMS_IDE64_SETTINGS_HU, "IDE64 beállításai..." },
/* it */ { IDMS_IDE64_SETTINGS_IT, "Impostazioni IDE64..." },
/* ko */ { IDMS_IDE64_SETTINGS_KO, "IDE64 ¼³Á¤..." },
/* nl */ { IDMS_IDE64_SETTINGS_NL, "IDE64 instellingen..." },
/* pl */ { IDMS_IDE64_SETTINGS_PL, "Ustawienia IDE64..." },
/* ru */ { IDMS_IDE64_SETTINGS_RU, "IDE64 settings..." },
/* sv */ { IDMS_IDE64_SETTINGS_SV, "IDE64-inställningar..." },
/* tr */ { IDMS_IDE64_SETTINGS_TR, "IDE64 ayarlarý..." },

/* en */ { IDMS_MMC64_SETTINGS,    "MMC64 settings..." },
/* da */ { IDMS_MMC64_SETTINGS_DA, "MMC64-indstillinger..." },
/* de */ { IDMS_MMC64_SETTINGS_DE, "MMC64 Einstellungen..." },
/* es */ { IDMS_MMC64_SETTINGS_ES, "Ajustes MMC64..." },
/* fr */ { IDMS_MMC64_SETTINGS_FR, "Paramètres MMC64......" },
/* hu */ { IDMS_MMC64_SETTINGS_HU, "MMC64 beállításai..." },
/* it */ { IDMS_MMC64_SETTINGS_IT, "Impostazioni MMC64..." },
/* ko */ { IDMS_MMC64_SETTINGS_KO, "MMC64 ¼³Á¤..." },
/* nl */ { IDMS_MMC64_SETTINGS_NL, "MMC64 instellingen..." },
/* pl */ { IDMS_MMC64_SETTINGS_PL, "Ustawienia MMC64..." },
/* ru */ { IDMS_MMC64_SETTINGS_RU, "MMC64 settings..." },
/* sv */ { IDMS_MMC64_SETTINGS_SV, "MMC64-inställningar......" },
/* tr */ { IDMS_MMC64_SETTINGS_TR, "MMC64 ayarlarý..." },

/* en */ { IDMS_GMOD2_SETTINGS,    "GMod2 settings..." },
/* da */ { IDMS_GMOD2_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDMS_GMOD2_SETTINGS_DE, "GMod2 Einstellungen..." },
/* es */ { IDMS_GMOD2_SETTINGS_ES, "" },  /* fuzzy */
/* fr */ { IDMS_GMOD2_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_GMOD2_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_GMOD2_SETTINGS_IT, "" },  /* fuzzy */
/* ko */ { IDMS_GMOD2_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_GMOD2_SETTINGS_NL, "GMod2 instellingen..." },
/* pl */ { IDMS_GMOD2_SETTINGS_PL, "" },  /* fuzzy */
/* ru */ { IDMS_GMOD2_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_GMOD2_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDMS_GMOD2_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDMS_MMCREPLAY_SETTINGS,    "MMC Replay settings..." },
/* da */ { IDMS_MMCREPLAY_SETTINGS_DA, "MMC Replay-indstillinger..." },
/* de */ { IDMS_MMCREPLAY_SETTINGS_DE, "MMC Replay Einstellungen..." },
/* es */ { IDMS_MMCREPLAY_SETTINGS_ES, "Ajustes MMC Replay..." },
/* fr */ { IDMS_MMCREPLAY_SETTINGS_FR, "Paramètres MMC Replay..." },
/* hu */ { IDMS_MMCREPLAY_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_MMCREPLAY_SETTINGS_IT, "Impostazioni MMC Replay..." },
/* ko */ { IDMS_MMCREPLAY_SETTINGS_KO, "MMC Replay ¼³Á¤..." },
/* nl */ { IDMS_MMCREPLAY_SETTINGS_NL, "MMC Replay instellingen..." },
/* pl */ { IDMS_MMCREPLAY_SETTINGS_PL, "Ustawienia MMC Replay..." },
/* ru */ { IDMS_MMCREPLAY_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_MMCREPLAY_SETTINGS_SV, "MMC Replay-inställningar..." },
/* tr */ { IDMS_MMCREPLAY_SETTINGS_TR, "MMC Replay ayarlarý..." },

/* en */ { IDMS_RETROREPLAY_SETTINGS,    "Retro Replay settings..." },
/* da */ { IDMS_RETROREPLAY_SETTINGS_DA, "Retro Replay-indstillinger..." },
/* de */ { IDMS_RETROREPLAY_SETTINGS_DE, "Retro Replay Einstellungen..." },
/* es */ { IDMS_RETROREPLAY_SETTINGS_ES, "Ajustes Retro Replay..." },
/* fr */ { IDMS_RETROREPLAY_SETTINGS_FR, "Paramètres Retro Replay..." },
/* hu */ { IDMS_RETROREPLAY_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_RETROREPLAY_SETTINGS_IT, "Impostazioni Retro Replay..." },
/* ko */ { IDMS_RETROREPLAY_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_RETROREPLAY_SETTINGS_NL, "Retro Replay instellingen..." },
/* pl */ { IDMS_RETROREPLAY_SETTINGS_PL, "Ustawienia Retro Replay..." },
/* ru */ { IDMS_RETROREPLAY_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_RETROREPLAY_SETTINGS_SV, "Retro Replay-inställningar..." },
/* tr */ { IDMS_RETROREPLAY_SETTINGS_TR, "Retro Replay ayarlarý..." },

/* en */ { IDMS_DIGIMAX_SETTINGS,    "DigiMAX settings..." },
/* da */ { IDMS_DIGIMAX_SETTINGS_DA, "DigiMAX-indstillinger..." },
/* de */ { IDMS_DIGIMAX_SETTINGS_DE, "DigiMAX Einstellungen..." },
/* es */ { IDMS_DIGIMAX_SETTINGS_ES, "Ajustes DigiMAX..." },
/* fr */ { IDMS_DIGIMAX_SETTINGS_FR, "Paramètres DigiMAX..." },
/* hu */ { IDMS_DIGIMAX_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_DIGIMAX_SETTINGS_IT, "Impostazioni DigiMAX..." },
/* ko */ { IDMS_DIGIMAX_SETTINGS_KO, "DigiMAX ¼³Á¤..." },
/* nl */ { IDMS_DIGIMAX_SETTINGS_NL, "DigiMAX instellingen..." },
/* pl */ { IDMS_DIGIMAX_SETTINGS_PL, "Ustawienia DigiMAX..." },
/* ru */ { IDMS_DIGIMAX_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_DIGIMAX_SETTINGS_SV, "DigiMAX-inställningar..." },
/* tr */ { IDMS_DIGIMAX_SETTINGS_TR, "DigiMAX ayarlarý..." },

/* en */ { IDMS_DIGIMAX_SETTINGS_MASCUERADE,    "DigiMAX settings (MasC=uerade)..." },
/* da */ { IDMS_DIGIMAX_SETTINGS_MASCUERADE_DA, "DigiMAX-indstillinger (MasC=uerade)..." },
/* de */ { IDMS_DIGIMAX_SETTINGS_MASCUERADE_DE, "DigiMAX Einstellungen (MasC=uerade)..." },
/* es */ { IDMS_DIGIMAX_SETTINGS_MASCUERADE_ES, "Ajustes DigiMAX (MasC=uerade)..." },
/* fr */ { IDMS_DIGIMAX_SETTINGS_MASCUERADE_FR, "Paramètres DigiMAX (MasC=uerade)..." },
/* hu */ { IDMS_DIGIMAX_SETTINGS_MASCUERADE_HU, "" },  /* fuzzy */
/* it */ { IDMS_DIGIMAX_SETTINGS_MASCUERADE_IT, "Impostazioni DigiMAX (MasC=uerade)..." },
/* ko */ { IDMS_DIGIMAX_SETTINGS_MASCUERADE_KO, "DigiMAX ¼³Á¤ (MasC=uerade)..." },
/* nl */ { IDMS_DIGIMAX_SETTINGS_MASCUERADE_NL, "DigiMAX instellingen (MasC=uerade)..." },
/* pl */ { IDMS_DIGIMAX_SETTINGS_MASCUERADE_PL, "Ustawienia DigiMAX (MasC=uerade)..." },
/* ru */ { IDMS_DIGIMAX_SETTINGS_MASCUERADE_RU, "" },  /* fuzzy */
/* sv */ { IDMS_DIGIMAX_SETTINGS_MASCUERADE_SV, "DigiMAX-inställningar (MasC=uerade)..." },
/* tr */ { IDMS_DIGIMAX_SETTINGS_MASCUERADE_TR, "DigiMAX ayarlarý (MasC=uerade)..." },

/* en */ { IDMS_DS12C887RTC_SETTINGS,    "DS12C887 RTC settings..." },
/* da */ { IDMS_DS12C887RTC_SETTINGS_DA, "DS12C887 RTC-indstillinger..." },
/* de */ { IDMS_DS12C887RTC_SETTINGS_DE, "DS12C887 RTC Einstellungen..." },
/* es */ { IDMS_DS12C887RTC_SETTINGS_ES, "Ajustes DS12C887 RTC..." },
/* fr */ { IDMS_DS12C887RTC_SETTINGS_FR, "Paramètres DS12C887 RTC..." },
/* hu */ { IDMS_DS12C887RTC_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_DS12C887RTC_SETTINGS_IT, "Impostazioni RTC DS12C887..." },
/* ko */ { IDMS_DS12C887RTC_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_DS12C887RTC_SETTINGS_NL, "DS12C887 RTC instellingen..." },
/* pl */ { IDMS_DS12C887RTC_SETTINGS_PL, "Ustawienia DS12C887 RTC..." },
/* ru */ { IDMS_DS12C887RTC_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_DS12C887RTC_SETTINGS_SV, "DS12C887 RTC-inställningar..." },
/* tr */ { IDMS_DS12C887RTC_SETTINGS_TR, "DS12C887 RTC ayarlarý..." },

/* en */ { IDMS_DS12C887RTC_SETTINGS_MASCUERADE,    "DS12C887 RTC settings (MasC=uerade)..." },
/* da */ { IDMS_DS12C887RTC_SETTINGS_MASCUERADE_DA, "DS12C887 RTC-indstillinger (MasC=uerade)..." },
/* de */ { IDMS_DS12C887RTC_SETTINGS_MASCUERADE_DE, "DS12C887 RTC Einstellungen (MasC=uerade)..." },
/* es */ { IDMS_DS12C887RTC_SETTINGS_MASCUERADE_ES, "Ajustes DS12C887 RTC (MasC=uerade)..." },
/* fr */ { IDMS_DS12C887RTC_SETTINGS_MASCUERADE_FR, "Paramètres DS12C887 RTC (MasC=uerade)..." },
/* hu */ { IDMS_DS12C887RTC_SETTINGS_MASCUERADE_HU, "" },  /* fuzzy */
/* it */ { IDMS_DS12C887RTC_SETTINGS_MASCUERADE_IT, "Impostazioni RTC DS12C887 (MasC=uerade)..." },
/* ko */ { IDMS_DS12C887RTC_SETTINGS_MASCUERADE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_DS12C887RTC_SETTINGS_MASCUERADE_NL, "DS12C887 RTC instellingen (MasC=uerade)..." },
/* pl */ { IDMS_DS12C887RTC_SETTINGS_MASCUERADE_PL, "Ustawienia DS12C887 RTC (MasC=uerade)..." },
/* ru */ { IDMS_DS12C887RTC_SETTINGS_MASCUERADE_RU, "" },  /* fuzzy */
/* sv */ { IDMS_DS12C887RTC_SETTINGS_MASCUERADE_SV, "DS12C887 RTC-inställningar (MasC=uerade)..." },
/* tr */ { IDMS_DS12C887RTC_SETTINGS_MASCUERADE_TR, "DS12C887 RTC ayarlarý (MasC=uerade)..." },

/* en */ { IDMS_SFX_SE_SETTINGS,    "SFX Sound Expander settings..." },
/* da */ { IDMS_SFX_SE_SETTINGS_DA, "SFX Sound Expander-indstillinger..." },
/* de */ { IDMS_SFX_SE_SETTINGS_DE, "SFX Sound Expander Einstellungen..." },
/* es */ { IDMS_SFX_SE_SETTINGS_ES, "Ajustes SFX Sound Expander..." },
/* fr */ { IDMS_SFX_SE_SETTINGS_FR, "Paramètres du SFX Sound Expander..." },
/* hu */ { IDMS_SFX_SE_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_SFX_SE_SETTINGS_IT, "Impostazioni SFX Sound Expander..." },
/* ko */ { IDMS_SFX_SE_SETTINGS_KO, "SFX Sound Expander ¼³Á¤..." },
/* nl */ { IDMS_SFX_SE_SETTINGS_NL, "SFX Sound Expander instellingen..." },
/* pl */ { IDMS_SFX_SE_SETTINGS_PL, "Ustawienia SFX Sound Expander..." },
/* ru */ { IDMS_SFX_SE_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_SFX_SE_SETTINGS_SV, "Inställningar för SFX Sound Expander..." },
/* tr */ { IDMS_SFX_SE_SETTINGS_TR, "SFX Sound Expander Ayarlarý..." },

/* en */ { IDMS_SFX_SE_SETTINGS_MASCUERADE,    "SFX Sound Expander settings (MasC=uerade)..." },
/* da */ { IDMS_SFX_SE_SETTINGS_MASCUERADE_DA, "SFX Sound Expander-indstillinger (MasC=uerade)..." },
/* de */ { IDMS_SFX_SE_SETTINGS_MASCUERADE_DE, "SFX Sound Expander Einstellungen (MasC=uerade)..." },
/* es */ { IDMS_SFX_SE_SETTINGS_MASCUERADE_ES, "Ajustes SFX Sound Expander (MasC=uerade)..." },
/* fr */ { IDMS_SFX_SE_SETTINGS_MASCUERADE_FR, "Paramètres du SFX Sound Expander (MasC=uerade)..." },
/* hu */ { IDMS_SFX_SE_SETTINGS_MASCUERADE_HU, "" },  /* fuzzy */
/* it */ { IDMS_SFX_SE_SETTINGS_MASCUERADE_IT, "Impostazioni SFX Sound Expander (MasC=uerade)..." },
/* ko */ { IDMS_SFX_SE_SETTINGS_MASCUERADE_KO, "SFX Sound Expander ¼³Á¤ (MasC=uerade)..." },
/* nl */ { IDMS_SFX_SE_SETTINGS_MASCUERADE_NL, "SFX Sound Expander instellingen (MasC=uerade)..." },
/* pl */ { IDMS_SFX_SE_SETTINGS_MASCUERADE_PL, "Ustawienia SFX Sound Expander (MasC=uerade)..." },
/* ru */ { IDMS_SFX_SE_SETTINGS_MASCUERADE_RU, "" },  /* fuzzy */
/* sv */ { IDMS_SFX_SE_SETTINGS_MASCUERADE_SV, "Inställningar för SFX Sound Expander (MasC=uerade)..." },
/* tr */ { IDMS_SFX_SE_SETTINGS_MASCUERADE_TR, "SFX Sound Expander Ayarlarý (MasC=uerade)..." },

/* en */ { IDMS_SFX_SS_SETTINGS_MASCUERADE,    "SFX Sound Sampler settings (MasC=uerade)..." },
/* da */ { IDMS_SFX_SS_SETTINGS_MASCUERADE_DA, "SFX Sound Sampler-indstillinger (MasC=uerade)..." },
/* de */ { IDMS_SFX_SS_SETTINGS_MASCUERADE_DE, "SFX Sound Sampler Einstellungen (MasC=uerade)..." },
/* es */ { IDMS_SFX_SS_SETTINGS_MASCUERADE_ES, "Ajustes SFX Sound Sampler (MasC=uerade)..." },
/* fr */ { IDMS_SFX_SS_SETTINGS_MASCUERADE_FR, "Paramètres du SFX Sound Sampler (MasC=uerade)..." },
/* hu */ { IDMS_SFX_SS_SETTINGS_MASCUERADE_HU, "" },  /* fuzzy */
/* it */ { IDMS_SFX_SS_SETTINGS_MASCUERADE_IT, "Impostazioni SFX Sound Sampler (MasC=uerade)..." },
/* ko */ { IDMS_SFX_SS_SETTINGS_MASCUERADE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_SFX_SS_SETTINGS_MASCUERADE_NL, "SFX Sound Sampler instellingen (MasC=uerade)..." },
/* pl */ { IDMS_SFX_SS_SETTINGS_MASCUERADE_PL, "Ustawienia samplera d¼wiêku SFX (MasC=uerade)..." },
/* ru */ { IDMS_SFX_SS_SETTINGS_MASCUERADE_RU, "" },  /* fuzzy */
/* sv */ { IDMS_SFX_SS_SETTINGS_MASCUERADE_SV, "Inställningar för SFX Sound Sampler (MasC=uerade)..." },
/* tr */ { IDMS_SFX_SS_SETTINGS_MASCUERADE_TR, "SFX Sound Sampler ayarlarý (MasC=uerade)..." },

/* en */ { IDMS_EASYFLASH_SETTINGS,    "EasyFlash settings..." },
/* da */ { IDMS_EASYFLASH_SETTINGS_DA, "EasyFlash-indstillinger..." },
/* de */ { IDMS_EASYFLASH_SETTINGS_DE, "EasyFlash Einstellungen..." },
/* es */ { IDMS_EASYFLASH_SETTINGS_ES, "Ajustes EasyFlash..." },
/* fr */ { IDMS_EASYFLASH_SETTINGS_FR, "Paramètres datassette..." },
/* hu */ { IDMS_EASYFLASH_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_EASYFLASH_SETTINGS_IT, "Impostazioni EasyFlash..." },
/* ko */ { IDMS_EASYFLASH_SETTINGS_KO, "EasyFlash ¼³Á¤..." },
/* nl */ { IDMS_EASYFLASH_SETTINGS_NL, "EasyFlash instellingen..." },
/* pl */ { IDMS_EASYFLASH_SETTINGS_PL, "Ustawienia EasyFlash..." },
/* ru */ { IDMS_EASYFLASH_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_EASYFLASH_SETTINGS_SV, "EasyFlash-inställningar..." },
/* tr */ { IDMS_EASYFLASH_SETTINGS_TR, "EasyFlash ayarlarý..." },

/* en */ { IDMS_ENABLE_SFX_SS,    "Enable the SFX Sound Sampler cartridge" },
/* da */ { IDMS_ENABLE_SFX_SS_DA, "Aktivér SFX Sound Sampler cartridge" },
/* de */ { IDMS_ENABLE_SFX_SS_DE, "SFX Sound Sampler Modul aktivieren" },
/* es */ { IDMS_ENABLE_SFX_SS_ES, "Permitir cartucho SFX Sound Sampler" },
/* fr */ { IDMS_ENABLE_SFX_SS_FR, "Activer la cartouche SFX Sound Sampler" },
/* hu */ { IDMS_ENABLE_SFX_SS_HU, "SFX Sound Sampler engedélyezése" },
/* it */ { IDMS_ENABLE_SFX_SS_IT, "Attiva cartuccia SFX Sound Sampler" },
/* ko */ { IDMS_ENABLE_SFX_SS_KO, "SFX Sound Sampler Ä«Æ®¸®Áö ¸¦ ÀÛµ¿½ÃÅ²´Ù" },
/* nl */ { IDMS_ENABLE_SFX_SS_NL, "SFX Sound Sampler cartridge inschakelen" },
/* pl */ { IDMS_ENABLE_SFX_SS_PL, "W³±cz kartrid¿ SFX Sound Sampler" },
/* ru */ { IDMS_ENABLE_SFX_SS_RU, "Enable the SFX Sound Sampler cartridge" },
/* sv */ { IDMS_ENABLE_SFX_SS_SV, "Aktivera SFX Sound Sampler-insticksmodul" },
/* tr */ { IDMS_ENABLE_SFX_SS_TR, "SFX Sound Sampler kartuþunu aktif et" },

/* en */ { IDMS_ENABLE_SS5_32K_ADDON,    "Enable the Super Snapshot 5 32KB addon" },
/* da */ { IDMS_ENABLE_SS5_32K_ADDON_DA, "" },  /* fuzzy */
/* de */ { IDMS_ENABLE_SS5_32K_ADDON_DE, "Super Snapshot 5 32KB Erweiterung aktivieren" },
/* es */ { IDMS_ENABLE_SS5_32K_ADDON_ES, "" },  /* fuzzy */
/* fr */ { IDMS_ENABLE_SS5_32K_ADDON_FR, "" },  /* fuzzy */
/* hu */ { IDMS_ENABLE_SS5_32K_ADDON_HU, "" },  /* fuzzy */
/* it */ { IDMS_ENABLE_SS5_32K_ADDON_IT, "" },  /* fuzzy */
/* ko */ { IDMS_ENABLE_SS5_32K_ADDON_KO, "" },  /* fuzzy */
/* nl */ { IDMS_ENABLE_SS5_32K_ADDON_NL, "Activeer de Super Snapshot 5 32KB uitbreiding" },
/* pl */ { IDMS_ENABLE_SS5_32K_ADDON_PL, "" },  /* fuzzy */
/* ru */ { IDMS_ENABLE_SS5_32K_ADDON_RU, "" },  /* fuzzy */
/* sv */ { IDMS_ENABLE_SS5_32K_ADDON_SV, "" },  /* fuzzy */
/* tr */ { IDMS_ENABLE_SS5_32K_ADDON_TR, "" },  /* fuzzy */

/* en */ { IDMS_ENABLE_CPM_CART,    "Enable the CP/M cartridge" },
/* da */ { IDMS_ENABLE_CPM_CART_DA, "" },  /* fuzzy */
/* de */ { IDMS_ENABLE_CPM_CART_DE, "CP/M Modul aktivieren" },
/* es */ { IDMS_ENABLE_CPM_CART_ES, "" },  /* fuzzy */
/* fr */ { IDMS_ENABLE_CPM_CART_FR, "" },  /* fuzzy */
/* hu */ { IDMS_ENABLE_CPM_CART_HU, "" },  /* fuzzy */
/* it */ { IDMS_ENABLE_CPM_CART_IT, "" },  /* fuzzy */
/* ko */ { IDMS_ENABLE_CPM_CART_KO, "" },  /* fuzzy */
/* nl */ { IDMS_ENABLE_CPM_CART_NL, "CP/M cartridge inschakelen" },
/* pl */ { IDMS_ENABLE_CPM_CART_PL, "" },  /* fuzzy */
/* ru */ { IDMS_ENABLE_CPM_CART_RU, "" },  /* fuzzy */
/* sv */ { IDMS_ENABLE_CPM_CART_SV, "" },  /* fuzzy */
/* tr */ { IDMS_ENABLE_CPM_CART_TR, "" },  /* fuzzy */

/* en */ { IDMS_BURST_MOD_SETTINGS,    "Drive burst modification settings..." },
/* da */ { IDMS_BURST_MOD_SETTINGS_DA, "Drev burst-modifikations-indstillinger..." },
/* de */ { IDMS_BURST_MOD_SETTINGS_DE, "Laufwerk Burstmodifkation Einstellungen..." },
/* es */ { IDMS_BURST_MOD_SETTINGS_ES, "Ajuste modificación Burst en controlador..." },
/* fr */ { IDMS_BURST_MOD_SETTINGS_FR, "Activer l'émulation de Burst Lecteur..." },
/* hu */ { IDMS_BURST_MOD_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_BURST_MOD_SETTINGS_IT, "Impostazioni modifica burst del drive..." },
/* ko */ { IDMS_BURST_MOD_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_BURST_MOD_SETTINGS_NL, "Drive burst modificatie instellingen..." },
/* pl */ { IDMS_BURST_MOD_SETTINGS_PL, "Ustawienia trybu synchronicznego napêdu..." },
/* ru */ { IDMS_BURST_MOD_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_BURST_MOD_SETTINGS_SV, "Inställningar för diskettburstmodifiering..." },
/* tr */ { IDMS_BURST_MOD_SETTINGS_TR, "Sürücü atlama modifikasyon ayarlarý..." },

/* en */ { IDMS_ENABLE_PETHRE,    "Enable PET High Res Emulator board" },
/* da */ { IDMS_ENABLE_PETHRE_DA, "" },  /* fuzzy */
/* de */ { IDMS_ENABLE_PETHRE_DE, "PET Hi-Res Emulator Karte aktivieren" },
/* es */ { IDMS_ENABLE_PETHRE_ES, "Permitir tableta gráfica PET alta resolución" },
/* fr */ { IDMS_ENABLE_PETHRE_FR, "Activer la carte haute-rés PET" },
/* hu */ { IDMS_ENABLE_PETHRE_HU, "" },  /* fuzzy */
/* it */ { IDMS_ENABLE_PETHRE_IT, "Attiva scheda alta risoluzione del PET" },
/* ko */ { IDMS_ENABLE_PETHRE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_ENABLE_PETHRE_NL, "PET High Res Emulator bord inschakelen" },
/* pl */ { IDMS_ENABLE_PETHRE_PL, "W³±cz uk³ad emulatora PET hi-res" },
/* ru */ { IDMS_ENABLE_PETHRE_RU, "" },  /* fuzzy */
/* sv */ { IDMS_ENABLE_PETHRE_SV, "Aktivera PET-högupplösningsemuleringskort" },
/* tr */ { IDMS_ENABLE_PETHRE_TR, "" },  /* fuzzy */

/* en */ { IDMS_ENABLE_USERPORT_DAC,    "Enable Userport DAC for sound output" },
/* da */ { IDMS_ENABLE_USERPORT_DAC_DA, "Aktivér brugerport DAC for lydoutput" },
/* de */ { IDMS_ENABLE_USERPORT_DAC_DE, "Userport DAC Sound aktivieren" },
/* es */ { IDMS_ENABLE_USERPORT_DAC_ES, "Permitir puerto DAC para salida de sonido" },
/* fr */ { IDMS_ENABLE_USERPORT_DAC_FR, "Activer DAC port utilisateur comme sortie de son" },
/* hu */ { IDMS_ENABLE_USERPORT_DAC_HU, "" },  /* fuzzy */
/* it */ { IDMS_ENABLE_USERPORT_DAC_IT, "Attiva DAC su userport per la riproduzione audio" },
/* ko */ { IDMS_ENABLE_USERPORT_DAC_KO, "" },  /* fuzzy */
/* nl */ { IDMS_ENABLE_USERPORT_DAC_NL, "Userport DAC voor geluidsuitvoer inschakelen" },
/* pl */ { IDMS_ENABLE_USERPORT_DAC_PL, "W³±cz userport DAC jako wyj¶cie d¼wiêku" },
/* ru */ { IDMS_ENABLE_USERPORT_DAC_RU, "Enable Userport DAC for sound output" },
/* sv */ { IDMS_ENABLE_USERPORT_DAC_SV, "Aktivera D/A-omvandlare på användarporten för ljudutdata" },
/* tr */ { IDMS_ENABLE_USERPORT_DAC_TR, "Ses çýkýþý için Userport DAC'ý aktif et" },

/* en */ { IDMS_ENABLE_USERPORT_DIGIMAX,    "Enable Userport DigiMAX device" },
/* da */ { IDMS_ENABLE_USERPORT_DIGIMAX_DA, "" },  /* fuzzy */
/* de */ { IDMS_ENABLE_USERPORT_DIGIMAX_DE, "Userport DigiMAX Gerät aktivieren" },
/* es */ { IDMS_ENABLE_USERPORT_DIGIMAX_ES, "" },  /* fuzzy */
/* fr */ { IDMS_ENABLE_USERPORT_DIGIMAX_FR, "" },  /* fuzzy */
/* hu */ { IDMS_ENABLE_USERPORT_DIGIMAX_HU, "" },  /* fuzzy */
/* it */ { IDMS_ENABLE_USERPORT_DIGIMAX_IT, "" },  /* fuzzy */
/* ko */ { IDMS_ENABLE_USERPORT_DIGIMAX_KO, "" },  /* fuzzy */
/* nl */ { IDMS_ENABLE_USERPORT_DIGIMAX_NL, "Userport DigiMAX apparaat inschakelen" },
/* pl */ { IDMS_ENABLE_USERPORT_DIGIMAX_PL, "" },  /* fuzzy */
/* ru */ { IDMS_ENABLE_USERPORT_DIGIMAX_RU, "" },  /* fuzzy */
/* sv */ { IDMS_ENABLE_USERPORT_DIGIMAX_SV, "" },  /* fuzzy */
/* tr */ { IDMS_ENABLE_USERPORT_DIGIMAX_TR, "" },  /* fuzzy */

/* en */ { IDMS_ENABLE_USERPORT_4BIT_SAMPLER,    "Enable Userport 4bit sampler" },
/* da */ { IDMS_ENABLE_USERPORT_4BIT_SAMPLER_DA, "" },  /* fuzzy */
/* de */ { IDMS_ENABLE_USERPORT_4BIT_SAMPLER_DE, "Userport 4bit Sampler aktivieren" },
/* es */ { IDMS_ENABLE_USERPORT_4BIT_SAMPLER_ES, "" },  /* fuzzy */
/* fr */ { IDMS_ENABLE_USERPORT_4BIT_SAMPLER_FR, "" },  /* fuzzy */
/* hu */ { IDMS_ENABLE_USERPORT_4BIT_SAMPLER_HU, "" },  /* fuzzy */
/* it */ { IDMS_ENABLE_USERPORT_4BIT_SAMPLER_IT, "" },  /* fuzzy */
/* ko */ { IDMS_ENABLE_USERPORT_4BIT_SAMPLER_KO, "" },  /* fuzzy */
/* nl */ { IDMS_ENABLE_USERPORT_4BIT_SAMPLER_NL, "Userport 4bit sampler inschakelen" },
/* pl */ { IDMS_ENABLE_USERPORT_4BIT_SAMPLER_PL, "" },  /* fuzzy */
/* ru */ { IDMS_ENABLE_USERPORT_4BIT_SAMPLER_RU, "" },  /* fuzzy */
/* sv */ { IDMS_ENABLE_USERPORT_4BIT_SAMPLER_SV, "" },  /* fuzzy */
/* tr */ { IDMS_ENABLE_USERPORT_4BIT_SAMPLER_TR, "" },  /* fuzzy */

/* en */ { IDMS_ENABLE_USERPORT_8BSS,    "Enable Userport 8bit stereo sampler" },
/* da */ { IDMS_ENABLE_USERPORT_8BSS_DA, "" },  /* fuzzy */
/* de */ { IDMS_ENABLE_USERPORT_8BSS_DE, "Userport 8bit Sampler aktivieren" },
/* es */ { IDMS_ENABLE_USERPORT_8BSS_ES, "" },  /* fuzzy */
/* fr */ { IDMS_ENABLE_USERPORT_8BSS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_ENABLE_USERPORT_8BSS_HU, "" },  /* fuzzy */
/* it */ { IDMS_ENABLE_USERPORT_8BSS_IT, "" },  /* fuzzy */
/* ko */ { IDMS_ENABLE_USERPORT_8BSS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_ENABLE_USERPORT_8BSS_NL, "Userport 8bit stereo sampler inschakelen" },
/* pl */ { IDMS_ENABLE_USERPORT_8BSS_PL, "" },  /* fuzzy */
/* ru */ { IDMS_ENABLE_USERPORT_8BSS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_ENABLE_USERPORT_8BSS_SV, "" },  /* fuzzy */
/* tr */ { IDMS_ENABLE_USERPORT_8BSS_TR, "" },  /* fuzzy */

/* en */ { IDMS_ETHERNET_SETTINGS,    "Ethernet settings..." },
/* da */ { IDMS_ETHERNET_SETTINGS_DA, "Ethernetindstillinger..." },
/* de */ { IDMS_ETHERNET_SETTINGS_DE, "Ethernet Einstellungen..." },
/* es */ { IDMS_ETHERNET_SETTINGS_ES, "Ajustes Ethernet..." },
/* fr */ { IDMS_ETHERNET_SETTINGS_FR, "Paramètres Ethernet..." },
/* hu */ { IDMS_ETHERNET_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_ETHERNET_SETTINGS_IT, "Impostazioni Ethernet..." },
/* ko */ { IDMS_ETHERNET_SETTINGS_KO, "ÀÌ´õ³Ý ¼³Á¤..." },
/* nl */ { IDMS_ETHERNET_SETTINGS_NL, "Ethernet instellingen..." },
/* pl */ { IDMS_ETHERNET_SETTINGS_PL, "Ustawienia Ethernet..." },
/* ru */ { IDMS_ETHERNET_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_ETHERNET_SETTINGS_SV, "Ethernetinställningar..." },
/* tr */ { IDMS_ETHERNET_SETTINGS_TR, "Ethernet Ayarlarý..." },

/* en */ { IDMS_ACIA_SETTINGS,    "ACIA settings..." },
/* da */ { IDMS_ACIA_SETTINGS_DA, "ACIA-indstillinger..." },
/* de */ { IDMS_ACIA_SETTINGS_DE, "ACIA Einstellungen..." },
/* es */ { IDMS_ACIA_SETTINGS_ES, "Ajustes ACIA..." },
/* fr */ { IDMS_ACIA_SETTINGS_FR, "Paramètres ACIA..." },
/* hu */ { IDMS_ACIA_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_ACIA_SETTINGS_IT, "Impostazioni ACIA..." },
/* ko */ { IDMS_ACIA_SETTINGS_KO, "ACIA ¼³Á¤..." },
/* nl */ { IDMS_ACIA_SETTINGS_NL, "ACIA instellingen..." },
/* pl */ { IDMS_ACIA_SETTINGS_PL, "Ustawienia ACIA..." },
/* ru */ { IDMS_ACIA_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_ACIA_SETTINGS_SV, "ACIA-inställningar..." },
/* tr */ { IDMS_ACIA_SETTINGS_TR, "ACIA Ayarlarý..." },

/* en */ { IDMS_ACIA_SETTINGS_MASCUERADE,    "ACIA settings (MasC=uerade)..." },
/* da */ { IDMS_ACIA_SETTINGS_MASCUERADE_DA, "ACIA-indstillinger (MasC=uerade)..." },
/* de */ { IDMS_ACIA_SETTINGS_MASCUERADE_DE, "ACIA Einstellungen (MasC=uerade)..." },
/* es */ { IDMS_ACIA_SETTINGS_MASCUERADE_ES, "Ajustes ACIA (MasC=uerade)..." },
/* fr */ { IDMS_ACIA_SETTINGS_MASCUERADE_FR, "Paramètres ACIA (MasC=uerade)..." },
/* hu */ { IDMS_ACIA_SETTINGS_MASCUERADE_HU, "" },  /* fuzzy */
/* it */ { IDMS_ACIA_SETTINGS_MASCUERADE_IT, "Impostazioni ACIA (MasC=uerade)..." },
/* ko */ { IDMS_ACIA_SETTINGS_MASCUERADE_KO, "ACIA ¼³Á¤ (MasC=uerade)..." },
/* nl */ { IDMS_ACIA_SETTINGS_MASCUERADE_NL, "ACIA instellingen (MasC=uerade)..." },
/* pl */ { IDMS_ACIA_SETTINGS_MASCUERADE_PL, "Ustawienia ACIA (MasC=uerade)..." },
/* ru */ { IDMS_ACIA_SETTINGS_MASCUERADE_RU, "" },  /* fuzzy */
/* sv */ { IDMS_ACIA_SETTINGS_MASCUERADE_SV, "ACIA-inställningar (MasC=uerade)..." },
/* tr */ { IDMS_ACIA_SETTINGS_MASCUERADE_TR, "ACIA Ayarlarý (MasC=uerade)..." },

/* en */ { IDMS_PETREU_SETTINGS,    "PET REU settings..." },
/* da */ { IDMS_PETREU_SETTINGS_DA, "PET REU-indstillinger..." },
/* de */ { IDMS_PETREU_SETTINGS_DE, "PET REU Einstellungen..." },
/* es */ { IDMS_PETREU_SETTINGS_ES, "Ajustes PET REU..." },
/* fr */ { IDMS_PETREU_SETTINGS_FR, "Paramètres PET REU..." },
/* hu */ { IDMS_PETREU_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_PETREU_SETTINGS_IT, "Impostazioni PET REU..." },
/* ko */ { IDMS_PETREU_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_PETREU_SETTINGS_NL, "PET REU instellingen..." },
/* pl */ { IDMS_PETREU_SETTINGS_PL, "Ustawienia PET REU..." },
/* ru */ { IDMS_PETREU_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_PETREU_SETTINGS_SV, "PET REU-Inställningar..." },
/* tr */ { IDMS_PETREU_SETTINGS_TR, "PET REU Ayarlarý..." },

/* en */ { IDMS_V364_SPEECH_SETTINGS,    "V364 Speech settings..." },
/* da */ { IDMS_V364_SPEECH_SETTINGS_DA, "V364 tale-indstillinger..." },
/* de */ { IDMS_V364_SPEECH_SETTINGS_DE, "V364 Stimme Einstellungen..." },
/* es */ { IDMS_V364_SPEECH_SETTINGS_ES, "Ajustes V364 Speech..." },
/* fr */ { IDMS_V364_SPEECH_SETTINGS_FR, "Paramètres du V364 Speech..." },
/* hu */ { IDMS_V364_SPEECH_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_V364_SPEECH_SETTINGS_IT, "Impostazioni V364 Speech..." },
/* ko */ { IDMS_V364_SPEECH_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_V364_SPEECH_SETTINGS_NL, "V364 Spraak instellingen..." },
/* pl */ { IDMS_V364_SPEECH_SETTINGS_PL, "Ustawienia mowy V364..." },
/* ru */ { IDMS_V364_SPEECH_SETTINGS_RU, "¿ÐàÐÜÕâàë àÕçØ V364..." },
/* sv */ { IDMS_V364_SPEECH_SETTINGS_SV, "V364-röstinställningar..." },
/* tr */ { IDMS_V364_SPEECH_SETTINGS_TR, "V364 Speech ayarlarý..." },

/* en */ { IDMS_SIDCART_SETTINGS,    "SID cart settings..." },
/* da */ { IDMS_SIDCART_SETTINGS_DA, "SID-cartridgeindstillinger..." },
/* de */ { IDMS_SIDCART_SETTINGS_DE, "SID Modul Einstellungen..." },
/* es */ { IDMS_SIDCART_SETTINGS_ES, "Ajustes SID Cart..." },
/* fr */ { IDMS_SIDCART_SETTINGS_FR, "Paramètres SID Cart..." },
/* hu */ { IDMS_SIDCART_SETTINGS_HU, "SID kártya beállításai......" },
/* it */ { IDMS_SIDCART_SETTINGS_IT, "Impostazioni cartuccia SID..." },
/* ko */ { IDMS_SIDCART_SETTINGS_KO, "SID Ä«Æ® ¼³Á¤..." },
/* nl */ { IDMS_SIDCART_SETTINGS_NL, "SID cart instellingen..." },
/* pl */ { IDMS_SIDCART_SETTINGS_PL, "Ustawienia karty SID..." },
/* ru */ { IDMS_SIDCART_SETTINGS_RU, "SID cart settings..." },
/* sv */ { IDMS_SIDCART_SETTINGS_SV, "SID-modulinställningarg..." },
/* tr */ { IDMS_SIDCART_SETTINGS_TR, "SID kartuþu ayarlarý..." },

/* en */ { IDMS_PETDWW_SETTINGS,    "PET DWW settings..." },
/* da */ { IDMS_PETDWW_SETTINGS_DA, "PET DWW-indstillinger..." },
/* de */ { IDMS_PETDWW_SETTINGS_DE, "PET DWW Einstellungen..." },
/* es */ { IDMS_PETDWW_SETTINGS_ES, "Ajustes PET DWW..." },
/* fr */ { IDMS_PETDWW_SETTINGS_FR, "Paramètres PET DWW..." },
/* hu */ { IDMS_PETDWW_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_PETDWW_SETTINGS_IT, "Impostazioni PET DWW..." },
/* ko */ { IDMS_PETDWW_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_PETDWW_SETTINGS_NL, "PET DWW instellingen..." },
/* pl */ { IDMS_PETDWW_SETTINGS_PL, "Ustawienia PET DWW..." },
/* ru */ { IDMS_PETDWW_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_PETDWW_SETTINGS_SV, "PET DWW-Inställningar..." },
/* tr */ { IDMS_PETDWW_SETTINGS_TR, "PET DWW Ayarlarý..." },

/* en */ { IDMS_PETCOLOUR_SETTINGS,    "PET Colour board settings..." },
/* da */ { IDMS_PETCOLOUR_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDMS_PETCOLOUR_SETTINGS_DE, "PET Farbkarte Einstellungen..." },
/* es */ { IDMS_PETCOLOUR_SETTINGS_ES, "Ajustes color PET..." },
/* fr */ { IDMS_PETCOLOUR_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_PETCOLOUR_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_PETCOLOUR_SETTINGS_IT, "Impostazioni scheda PET Colour..." },
/* ko */ { IDMS_PETCOLOUR_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_PETCOLOUR_SETTINGS_NL, "PET Colour bord instellingen..." },
/* pl */ { IDMS_PETCOLOUR_SETTINGS_PL, "Ustawienia koloru karty PET..." },
/* ru */ { IDMS_PETCOLOUR_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_PETCOLOUR_SETTINGS_SV, "Inställningar för PET-färgkort..." },
/* tr */ { IDMS_PETCOLOUR_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDMS_FINAL_EXPANSION_WRITEBACK,    "Enable Final Expansion write back to ROM file" },
/* da */ { IDMS_FINAL_EXPANSION_WRITEBACK_DA, "Aktivér Final Expansion tilbageskrivning til ROM-fil" },
/* de */ { IDMS_FINAL_EXPANSION_WRITEBACK_DE, "Final Expansion ROM Schreibzugriff aktivieren" },
/* es */ { IDMS_FINAL_EXPANSION_WRITEBACK_ES, "Permitir grabación en fichero ROM Final Expansion" },
/* fr */ { IDMS_FINAL_EXPANSION_WRITEBACK_FR, "Activer la réécriture du fichier ROM Final Expansion" },
/* hu */ { IDMS_FINAL_EXPANSION_WRITEBACK_HU, "Final Expansion ROM fájlba visszaírás engedélyezése" },
/* it */ { IDMS_FINAL_EXPANSION_WRITEBACK_IT, "Attiva write back per Final Expansion su file ROM" },
/* ko */ { IDMS_FINAL_EXPANSION_WRITEBACK_KO, "" },  /* fuzzy */
/* nl */ { IDMS_FINAL_EXPANSION_WRITEBACK_NL, "Terugschrijven naar ROM bestand voor Final Expansion inschakelen" },
/* pl */ { IDMS_FINAL_EXPANSION_WRITEBACK_PL, "W³±cz zapis do pliku ROM Final Expansion" },
/* ru */ { IDMS_FINAL_EXPANSION_WRITEBACK_RU, "Enable Final Expansion write back to ROM file" },
/* sv */ { IDMS_FINAL_EXPANSION_WRITEBACK_SV, "Aktivera återskrivning till ROM-fil för Final Expansion" },
/* tr */ { IDMS_FINAL_EXPANSION_WRITEBACK_TR, "Final Expansion ROM dosyasýna geri yazmayý aktif et" },

/* en */ { IDMS_FP_WRITEBACK,    "Enable Vic Flash Plugin write back to ROM file" },
/* da */ { IDMS_FP_WRITEBACK_DA, "Aktivér Vic Flash Plugin tilbageskrivning til ROM-fil" },
/* de */ { IDMS_FP_WRITEBACK_DE, "Vic Flash Plugin Rückschreiben auf ROM Datei aktivieren" },
/* es */ { IDMS_FP_WRITEBACK_ES, "Permitir grabación en ROM Vic Flash Plugin" },
/* fr */ { IDMS_FP_WRITEBACK_FR, "Activer la réécriture du fichier ROM Vic Flash Plugin" },
/* hu */ { IDMS_FP_WRITEBACK_HU, "" },  /* fuzzy */
/* it */ { IDMS_FP_WRITEBACK_IT, "Attiva write back per Vic Flash Plugin su file ROM" },
/* ko */ { IDMS_FP_WRITEBACK_KO, "" },  /* fuzzy */
/* nl */ { IDMS_FP_WRITEBACK_NL, "Terugschrijven naar ROM bestand voor Vic Flash Plugin inschakelen" },
/* pl */ { IDMS_FP_WRITEBACK_PL, "W³±cz zapis do pliku ROM Vic Flash Plugin" },
/* ru */ { IDMS_FP_WRITEBACK_RU, "Enable Vic Flash Plugin write back to ROM file" },
/* sv */ { IDMS_FP_WRITEBACK_SV, "Aktivera återskrivning till ROM-fil för Vic Flash-insticksmodul" },
/* tr */ { IDMS_FP_WRITEBACK_TR, "Flash Plugin ROM dosyasýna geri yazmayý aktif et" },

/* en */ { IDMS_UM_WRITEBACK,    "Enable UltiMem write back to ROM file" },
/* da */ { IDMS_UM_WRITEBACK_DA, "" },  /* fuzzy */
/* de */ { IDMS_UM_WRITEBACK_DE, "UltiMem ROM Schreibzugriff aktivieren" },
/* es */ { IDMS_UM_WRITEBACK_ES, "" },  /* fuzzy */
/* fr */ { IDMS_UM_WRITEBACK_FR, "" },  /* fuzzy */
/* hu */ { IDMS_UM_WRITEBACK_HU, "" },  /* fuzzy */
/* it */ { IDMS_UM_WRITEBACK_IT, "" },  /* fuzzy */
/* ko */ { IDMS_UM_WRITEBACK_KO, "" },  /* fuzzy */
/* nl */ { IDMS_UM_WRITEBACK_NL, "Terugschrijven naar ROM bestand voor UltiMem inschakelen" },
/* pl */ { IDMS_UM_WRITEBACK_PL, "" },  /* fuzzy */
/* ru */ { IDMS_UM_WRITEBACK_RU, "" },  /* fuzzy */
/* sv */ { IDMS_UM_WRITEBACK_SV, "" },  /* fuzzy */
/* tr */ { IDMS_UM_WRITEBACK_TR, "" },  /* fuzzy */

/* en */ { IDMS_MEGACART_WRITEBACK,    "Enable Mega-Cart NvRAM writeback" },
/* da */ { IDMS_MEGACART_WRITEBACK_DA, "Aktivér Mega-Cart NvRAM tilbageskrivning" },
/* de */ { IDMS_MEGACART_WRITEBACK_DE, "Mega-Cart NvRAM Rückschreiben aktivieren" },
/* es */ { IDMS_MEGACART_WRITEBACK_ES, "Permitir respuesta Mega-Cart NvRAM" },
/* fr */ { IDMS_MEGACART_WRITEBACK_FR, "Permettre la réécriture de l'image Mega-Cart" },
/* hu */ { IDMS_MEGACART_WRITEBACK_HU, "" },  /* fuzzy */
/* it */ { IDMS_MEGACART_WRITEBACK_IT, "Write back NvRAM Mega-Cart" },
/* ko */ { IDMS_MEGACART_WRITEBACK_KO, "" },  /* fuzzy */
/* nl */ { IDMS_MEGACART_WRITEBACK_NL, "Mega-Cart NvRAM terugschrijven inschakelem" },
/* pl */ { IDMS_MEGACART_WRITEBACK_PL, "W³±cz zapis do NvRAM Mega-Cart" },
/* ru */ { IDMS_MEGACART_WRITEBACK_RU, "Enable Mega-Cart NvRAM writeback" },
/* sv */ { IDMS_MEGACART_WRITEBACK_SV, "Aktivera Mega-Cart NvRAM-återskrivning" },
/* tr */ { IDMS_MEGACART_WRITEBACK_TR, "Mega-Cart NvRAM geri yazmayý aktif et" },

/* en */ { IDMS_MEGACART_WRITEBACK_FILE,    "Select Mega-Cart non-volatile RAM file" },
/* da */ { IDMS_MEGACART_WRITEBACK_FILE_DA, "Vælg Mega-Cart NVRAM-fil" },
/* de */ { IDMS_MEGACART_WRITEBACK_FILE_DE, "Mega-Cart nicht-flüchtiges RAM Datei wählen" },
/* es */ { IDMS_MEGACART_WRITEBACK_FILE_ES, "Seleccionar fichero RAM Mega-Cart no volatil" },
/* fr */ { IDMS_MEGACART_WRITEBACK_FILE_FR, "Sélectionner le fichier de RAM non-volatile Mega-Cart" },
/* hu */ { IDMS_MEGACART_WRITEBACK_FILE_HU, "Válassza ki az NvRAM képfájlt" },
/* it */ { IDMS_MEGACART_WRITEBACK_FILE_IT, "Seleziona file non-volatile RAM Mega-Cart" },
/* ko */ { IDMS_MEGACART_WRITEBACK_FILE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_MEGACART_WRITEBACK_FILE_NL, "Selecteer Mega-Cart niet-vluchtig RAM bestand" },
/* pl */ { IDMS_MEGACART_WRITEBACK_FILE_PL, "Wybierz sta³y plik RAM Mega-Cart" },
/* ru */ { IDMS_MEGACART_WRITEBACK_FILE_RU, "Select Mega-Cart non-volatile RAM file" },
/* sv */ { IDMS_MEGACART_WRITEBACK_FILE_SV, "Välj NvRAM-fil för Mega-Cart" },
/* tr */ { IDMS_MEGACART_WRITEBACK_FILE_TR, "Mega-Cart non-volatile RAM dosyasýný seç" },

/* en */ { IDMS_IO2_RAM_ENABLE,    "Enable I/O-2 RAM" },
/* da */ { IDMS_IO2_RAM_ENABLE_DA, "" },  /* fuzzy */
/* de */ { IDMS_IO2_RAM_ENABLE_DE, "I/O-2 RAM aktivieren" },
/* es */ { IDMS_IO2_RAM_ENABLE_ES, "" },  /* fuzzy */
/* fr */ { IDMS_IO2_RAM_ENABLE_FR, "" },  /* fuzzy */
/* hu */ { IDMS_IO2_RAM_ENABLE_HU, "" },  /* fuzzy */
/* it */ { IDMS_IO2_RAM_ENABLE_IT, "" },  /* fuzzy */
/* ko */ { IDMS_IO2_RAM_ENABLE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_IO2_RAM_ENABLE_NL, "I/O-2 RAM inschakelen" },
/* pl */ { IDMS_IO2_RAM_ENABLE_PL, "" },  /* fuzzy */
/* ru */ { IDMS_IO2_RAM_ENABLE_RU, "" },  /* fuzzy */
/* sv */ { IDMS_IO2_RAM_ENABLE_SV, "" },  /* fuzzy */
/* tr */ { IDMS_IO2_RAM_ENABLE_TR, "" },  /* fuzzy */

/* en */ { IDMS_IO3_RAM_ENABLE,    "Enable I/O-3 RAM" },
/* da */ { IDMS_IO3_RAM_ENABLE_DA, "" },  /* fuzzy */
/* de */ { IDMS_IO3_RAM_ENABLE_DE, "I/O-3 RAM aktivieren" },
/* es */ { IDMS_IO3_RAM_ENABLE_ES, "" },  /* fuzzy */
/* fr */ { IDMS_IO3_RAM_ENABLE_FR, "" },  /* fuzzy */
/* hu */ { IDMS_IO3_RAM_ENABLE_HU, "" },  /* fuzzy */
/* it */ { IDMS_IO3_RAM_ENABLE_IT, "" },  /* fuzzy */
/* ko */ { IDMS_IO3_RAM_ENABLE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_IO3_RAM_ENABLE_NL, "I/O-3 RAM inschakelen" },
/* pl */ { IDMS_IO3_RAM_ENABLE_PL, "" },  /* fuzzy */
/* ru */ { IDMS_IO3_RAM_ENABLE_RU, "" },  /* fuzzy */
/* sv */ { IDMS_IO3_RAM_ENABLE_SV, "" },  /* fuzzy */
/* tr */ { IDMS_IO3_RAM_ENABLE_TR, "" },  /* fuzzy */

/* en */ { IDMS_VFLI_MOD_ENABLE,    "Enable VFLI modification" },
/* da */ { IDMS_VFLI_MOD_ENABLE_DA, "" },  /* fuzzy */
/* de */ { IDMS_VFLI_MOD_ENABLE_DE, "VFLI Emulation aktivieren" },
/* es */ { IDMS_VFLI_MOD_ENABLE_ES, "" },  /* fuzzy */
/* fr */ { IDMS_VFLI_MOD_ENABLE_FR, "" },  /* fuzzy */
/* hu */ { IDMS_VFLI_MOD_ENABLE_HU, "" },  /* fuzzy */
/* it */ { IDMS_VFLI_MOD_ENABLE_IT, "" },  /* fuzzy */
/* ko */ { IDMS_VFLI_MOD_ENABLE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_VFLI_MOD_ENABLE_NL, "VFLI modificatie inschakelen" },
/* pl */ { IDMS_VFLI_MOD_ENABLE_PL, "" },  /* fuzzy */
/* ru */ { IDMS_VFLI_MOD_ENABLE_RU, "" },  /* fuzzy */
/* sv */ { IDMS_VFLI_MOD_ENABLE_SV, "" },  /* fuzzy */
/* tr */ { IDMS_VFLI_MOD_ENABLE_TR, "" },  /* fuzzy */

/* en */ { IDMS_C64DTV_SETTINGS,    "C64DTV settings..." },
/* da */ { IDMS_C64DTV_SETTINGS_DA, "C64DTV-indstillinger..." },
/* de */ { IDMS_C64DTV_SETTINGS_DE, "C64DTV Einstellungen..." },
/* es */ { IDMS_C64DTV_SETTINGS_ES, "Ajustes C64DTV..." },
/* fr */ { IDMS_C64DTV_SETTINGS_FR, "Paramètres C64DTV..." },
/* hu */ { IDMS_C64DTV_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_C64DTV_SETTINGS_IT, "Impostazioni C64DTV..." },
/* ko */ { IDMS_C64DTV_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_C64DTV_SETTINGS_NL, "C64DTV instellingen..." },
/* pl */ { IDMS_C64DTV_SETTINGS_PL, "Ustawienia C64DTV..." },
/* ru */ { IDMS_C64DTV_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_C64DTV_SETTINGS_SV, "C64DTV-inställningar..." },
/* tr */ { IDMS_C64DTV_SETTINGS_TR, "C64DTV Ayarlarý..." },

/* en */ { IDMS_RS232_USERPORT_SETTINGS,    "RS232 userport settings..." },
/* da */ { IDMS_RS232_USERPORT_SETTINGS_DA, "RS232-brugerportindstillinger..." },
/* de */ { IDMS_RS232_USERPORT_SETTINGS_DE, "RS232 Userport Einstellungen..." },
/* es */ { IDMS_RS232_USERPORT_SETTINGS_ES, "Ajustes puerto RS232..." },
/* fr */ { IDMS_RS232_USERPORT_SETTINGS_FR, "Paramètres port utilisateur RS232..." },
/* hu */ { IDMS_RS232_USERPORT_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_RS232_USERPORT_SETTINGS_IT, "Impostazioni RS232 su userport..." },
/* ko */ { IDMS_RS232_USERPORT_SETTINGS_KO, "RS232 À¯Á®Æ÷Æ® ¼³Á¤..." },
/* nl */ { IDMS_RS232_USERPORT_SETTINGS_NL, "RS232 userport instellingen..." },
/* pl */ { IDMS_RS232_USERPORT_SETTINGS_PL, "Ustawienia userportu RS232..." },
/* ru */ { IDMS_RS232_USERPORT_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_RS232_USERPORT_SETTINGS_SV, "RS232-användarportinställningar..." },
/* tr */ { IDMS_RS232_USERPORT_SETTINGS_TR, "RS232 userport ayarlarý..." },

/* en */ { IDMS_USERPORT_RTC58321A_SETTINGS,    "Userport RTC58321a settings..." },
/* da */ { IDMS_USERPORT_RTC58321A_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDMS_USERPORT_RTC58321A_SETTINGS_DE, "Userport RTC58321a Einstellungen..." },
/* es */ { IDMS_USERPORT_RTC58321A_SETTINGS_ES, "" },  /* fuzzy */
/* fr */ { IDMS_USERPORT_RTC58321A_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_USERPORT_RTC58321A_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_USERPORT_RTC58321A_SETTINGS_IT, "" },  /* fuzzy */
/* ko */ { IDMS_USERPORT_RTC58321A_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_USERPORT_RTC58321A_SETTINGS_NL, "Userport RTC58321a instellingen..." },
/* pl */ { IDMS_USERPORT_RTC58321A_SETTINGS_PL, "" },  /* fuzzy */
/* ru */ { IDMS_USERPORT_RTC58321A_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_USERPORT_RTC58321A_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDMS_USERPORT_RTC58321A_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDMS_USERPORT_DS1307_RTC_SETTINGS,    "Userport DS1307 RTC settings..." },
/* da */ { IDMS_USERPORT_DS1307_RTC_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDMS_USERPORT_DS1307_RTC_SETTINGS_DE, "Userport DS1307 RTC Einstellungen..." },
/* es */ { IDMS_USERPORT_DS1307_RTC_SETTINGS_ES, "" },  /* fuzzy */
/* fr */ { IDMS_USERPORT_DS1307_RTC_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_USERPORT_DS1307_RTC_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_USERPORT_DS1307_RTC_SETTINGS_IT, "" },  /* fuzzy */
/* ko */ { IDMS_USERPORT_DS1307_RTC_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_USERPORT_DS1307_RTC_SETTINGS_NL, "Userport DS1307 RTC instellingen..." },
/* pl */ { IDMS_USERPORT_DS1307_RTC_SETTINGS_PL, "" },  /* fuzzy */
/* ru */ { IDMS_USERPORT_DS1307_RTC_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_USERPORT_DS1307_RTC_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDMS_USERPORT_DS1307_RTC_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDMS_SAVE_CURRENT_SETTINGS_FILE,    "Save current settings to specified file" },
/* da */ { IDMS_SAVE_CURRENT_SETTINGS_FILE_DA, "Gem indstillinger som" },
/* de */ { IDMS_SAVE_CURRENT_SETTINGS_FILE_DE, "Speichere Einstellungen in angegebene Datei" },
/* es */ { IDMS_SAVE_CURRENT_SETTINGS_FILE_ES, "Grabar ajustes actuales en el fichero especificado" },
/* fr */ { IDMS_SAVE_CURRENT_SETTINGS_FILE_FR, "Enregistrer les paramètres courants" },
/* hu */ { IDMS_SAVE_CURRENT_SETTINGS_FILE_HU, "Jelenlegi beállítások mentése adott fájlba" },
/* it */ { IDMS_SAVE_CURRENT_SETTINGS_FILE_IT, "Salva impostazioni attuali sul file specificato" },
/* ko */ { IDMS_SAVE_CURRENT_SETTINGS_FILE_KO, "ÇöÁ¦ ¼³Á¤ À» ¼±ÅÃµÈ ÆÄÀÏ¿¡ ÀúÀå ÇÕ´Ï´Ù" },
/* nl */ { IDMS_SAVE_CURRENT_SETTINGS_FILE_NL, "Huidige instellingen opslaan naar opgegeven bestand" },
/* pl */ { IDMS_SAVE_CURRENT_SETTINGS_FILE_PL, "Zapisz obecne ustawienia do danego pliku" },
/* ru */ { IDMS_SAVE_CURRENT_SETTINGS_FILE_RU, "ÁÞåàÐÝØâì âÕÚãéØÕ ÝÐáâàÞÙÚØ Ò ãÚÐ×ÐÝÝÞÜ äÐÙÛÕ" },
/* sv */ { IDMS_SAVE_CURRENT_SETTINGS_FILE_SV, "Spara nuvarande inställningar till den angivna filen" },
/* tr */ { IDMS_SAVE_CURRENT_SETTINGS_FILE_TR, "Belirtilen dosyaya geçerli ayarlarý kaydet" },

/* en */ { IDMS_LOAD_SAVED_SETTINGS_FILE,    "Load settings from specified file" },
/* da */ { IDMS_LOAD_SAVED_SETTINGS_FILE_DA, "Indlæs indstillinger fra" },
/* de */ { IDMS_LOAD_SAVED_SETTINGS_FILE_DE, "Lade Einstellungen aus Datei" },
/* es */ { IDMS_LOAD_SAVED_SETTINGS_FILE_ES, "Cargar ajustes desde el fichero especificado" },
/* fr */ { IDMS_LOAD_SAVED_SETTINGS_FILE_FR, "Charger les paramètres depuis le fichier spécifié" },
/* hu */ { IDMS_LOAD_SAVED_SETTINGS_FILE_HU, "Beállítások betöltése a megadott fájlból" },
/* it */ { IDMS_LOAD_SAVED_SETTINGS_FILE_IT, "Carica impostazioni dal file specificato" },
/* ko */ { IDMS_LOAD_SAVED_SETTINGS_FILE_KO, "¼±ÅÃµÈ ÆÄÀÏ¿¡¼­ ¼³Á¤À» ºÒ·¯¿É´Ï´Ù" },
/* nl */ { IDMS_LOAD_SAVED_SETTINGS_FILE_NL, "Instellingen laden van opgegeven bestand" },
/* pl */ { IDMS_LOAD_SAVED_SETTINGS_FILE_PL, "Wczytaj ustawienia z danego pliku" },
/* ru */ { IDMS_LOAD_SAVED_SETTINGS_FILE_RU, "·ÐÓàã×Øâì ÝÐáâàÞÙÚØ Ø× ãÚÐ×ÐÝÝÞÓÞ äÐÙÛÐ" },
/* sv */ { IDMS_LOAD_SAVED_SETTINGS_FILE_SV, "Läs inställningar från den angivna filen" },
/* tr */ { IDMS_LOAD_SAVED_SETTINGS_FILE_TR, "Belirtilen dosyadan ayarlarý yükle" },

/* en */ { IDMS_SAVE_CURRENT_SETTINGS,    "Save current settings" },
/* da */ { IDMS_SAVE_CURRENT_SETTINGS_DA, "Gen aktuelle indstillinger" },
/* de */ { IDMS_SAVE_CURRENT_SETTINGS_DE, "Einstellungen speichern" },
/* es */ { IDMS_SAVE_CURRENT_SETTINGS_ES, "Grabar ajustes actuales" },
/* fr */ { IDMS_SAVE_CURRENT_SETTINGS_FR, "Enregistrer les paramètres courants" },
/* hu */ { IDMS_SAVE_CURRENT_SETTINGS_HU, "Jelenlegi beállítások mentése" },
/* it */ { IDMS_SAVE_CURRENT_SETTINGS_IT, "Salva impostazioni attuali" },
/* ko */ { IDMS_SAVE_CURRENT_SETTINGS_KO, "ÇöÁ¦ ¼³Á¤ ÀúÀå" },
/* nl */ { IDMS_SAVE_CURRENT_SETTINGS_NL, "Huidige instellingen opslaan" },
/* pl */ { IDMS_SAVE_CURRENT_SETTINGS_PL, "Zapisz obecne ustawienia" },
/* ru */ { IDMS_SAVE_CURRENT_SETTINGS_RU, "ÁÞåàÐÝØâì âÕÚãéØÕ ÝÐáâàÞÙÚØ" },
/* sv */ { IDMS_SAVE_CURRENT_SETTINGS_SV, "Spara nuvarande inställningar" },
/* tr */ { IDMS_SAVE_CURRENT_SETTINGS_TR, "Geçerli ayarlarý kaydet" },

/* en */ { IDMS_LOAD_SAVED_SETTINGS,    "Load saved settings" },
/* da */ { IDMS_LOAD_SAVED_SETTINGS_DA, "Indlæs gemte indstillinger" },
/* de */ { IDMS_LOAD_SAVED_SETTINGS_DE, "Gespeicherte Einstellungen Laden" },
/* es */ { IDMS_LOAD_SAVED_SETTINGS_ES, "Cargar ajustes grabados" },
/* fr */ { IDMS_LOAD_SAVED_SETTINGS_FR, "Charger les paramètres enregistrés" },
/* hu */ { IDMS_LOAD_SAVED_SETTINGS_HU, "Beállítások betöltése" },
/* it */ { IDMS_LOAD_SAVED_SETTINGS_IT, "Carica impostazioni salvate" },
/* ko */ { IDMS_LOAD_SAVED_SETTINGS_KO, "ÀúÀåµÈ ¼³Á¤ ºÒ·¯¿À±â" },
/* nl */ { IDMS_LOAD_SAVED_SETTINGS_NL, "Opgeslagen instellingen laden" },
/* pl */ { IDMS_LOAD_SAVED_SETTINGS_PL, "Wczytaj zapisane ustawienia" },
/* ru */ { IDMS_LOAD_SAVED_SETTINGS_RU, "·ÐÓàã×Øâì áÞåàÐÝÕÝÝëÕ ÝÐáâàÞÙÚØ" },
/* sv */ { IDMS_LOAD_SAVED_SETTINGS_SV, "Läs sparade inställningar" },
/* tr */ { IDMS_LOAD_SAVED_SETTINGS_TR, "Kaydedilmiþ ayarlarý yükle" },

/* en */ { IDMS_SET_DEFAULT_SETTINGS,    "Set default settings" },
/* da */ { IDMS_SET_DEFAULT_SETTINGS_DA, "Gendan standard indstillinger" },
/* de */ { IDMS_SET_DEFAULT_SETTINGS_DE, "Standard Einstellungen wiederherstellen" },
/* es */ { IDMS_SET_DEFAULT_SETTINGS_ES, "Usar ajustes por defecto" },
/* fr */ { IDMS_SET_DEFAULT_SETTINGS_FR, "Rétablir les paramètres par défaut" },
/* hu */ { IDMS_SET_DEFAULT_SETTINGS_HU, "Alapértelmezés beállítása" },
/* it */ { IDMS_SET_DEFAULT_SETTINGS_IT, "Ripristina impostazioni originarie" },
/* ko */ { IDMS_SET_DEFAULT_SETTINGS_KO, "¼³Á¤ ÃÊ±âÈ­" },
/* nl */ { IDMS_SET_DEFAULT_SETTINGS_NL, "Herstel standaard instellingen" },
/* pl */ { IDMS_SET_DEFAULT_SETTINGS_PL, "Ustaw domy¶lne ustawienia" },
/* ru */ { IDMS_SET_DEFAULT_SETTINGS_RU, "ÃáâÐÝÞÒØâì ÝÐáâàÞÙÚØ ßÞ ãÜÞÛçÐÝØî" },
/* sv */ { IDMS_SET_DEFAULT_SETTINGS_SV, "Återställ förvalda inställningar" },
/* tr */ { IDMS_SET_DEFAULT_SETTINGS_TR, "Mevcut ayarlarý varsayýlan olarak kaydet" },

/* en */ { IDMS_SAVE_SETTING_ON_EXIT,    "Save settings on exit" },
/* da */ { IDMS_SAVE_SETTING_ON_EXIT_DA, "Gem indstillinger når programmet afsluttes" },
/* de */ { IDMS_SAVE_SETTING_ON_EXIT_DE, "Einstellungen beim Beenden speichern" },
/* es */ { IDMS_SAVE_SETTING_ON_EXIT_ES, "Grabar ajustes al salir" },
/* fr */ { IDMS_SAVE_SETTING_ON_EXIT_FR, "Enregistrer les paramètres à la sortie" },
/* hu */ { IDMS_SAVE_SETTING_ON_EXIT_HU, "Beállítások mentése kilépéskor" },
/* it */ { IDMS_SAVE_SETTING_ON_EXIT_IT, "Salva impostazioni in uscita" },
/* ko */ { IDMS_SAVE_SETTING_ON_EXIT_KO, "Á¾·á½Ã ¼³Á¤ ÀúÀåÇÏ±â" },
/* nl */ { IDMS_SAVE_SETTING_ON_EXIT_NL, "Sla instellingen op bij afsluiten" },
/* pl */ { IDMS_SAVE_SETTING_ON_EXIT_PL, "Zapisz ustawienia przy wyj¶ciu" },
/* ru */ { IDMS_SAVE_SETTING_ON_EXIT_RU, "ÁÞåàÐÝØâì ÝÐáâàÞÙÚØ ßÞáÛÕ ÒëåÞÔÐ" },
/* sv */ { IDMS_SAVE_SETTING_ON_EXIT_SV, "Spara inställningar vid avslut" },
/* tr */ { IDMS_SAVE_SETTING_ON_EXIT_TR, "Çýkýþta ayarlarý kaydet" },

/* en */ { IDMS_CONFIRM_ON_EXIT,    "Confirm quitting VICE" },
/* da */ { IDMS_CONFIRM_ON_EXIT_DA, "Bekræft programafslutning" },
/* de */ { IDMS_CONFIRM_ON_EXIT_DE, "Bestätige Beenden von VICE" },
/* es */ { IDMS_CONFIRM_ON_EXIT_ES, "Confirmar cierre de VICE" },
/* fr */ { IDMS_CONFIRM_ON_EXIT_FR, "Confirmer avant de quitter VICE" },
/* hu */ { IDMS_CONFIRM_ON_EXIT_HU, "" },  /* fuzzy */
/* it */ { IDMS_CONFIRM_ON_EXIT_IT, "Conferma l'uscita da VICE" },
/* ko */ { IDMS_CONFIRM_ON_EXIT_KO, "VICE È®ÀÎ Á¾·á" },
/* nl */ { IDMS_CONFIRM_ON_EXIT_NL, "Bevestigen van het afsluiten van VICE" },
/* pl */ { IDMS_CONFIRM_ON_EXIT_PL, "Potwierdzaj zamkniêcie VICE" },
/* ru */ { IDMS_CONFIRM_ON_EXIT_RU, "¿ÞÔâÒÕàÔØâì ×ÐÚàëâØÕ VICE" },
/* sv */ { IDMS_CONFIRM_ON_EXIT_SV, "Bekräfta för att avsluta VICE" },
/* tr */ { IDMS_CONFIRM_ON_EXIT_TR, "VICE'dan çýkýþý teyit et" },

/* en */ { IDMS_HELP,    "Help" },
/* da */ { IDMS_HELP_DA, "Hjælp" },
/* de */ { IDMS_HELP_DE, "Hilfe" },
/* es */ { IDMS_HELP_ES, "Ayuda" },
/* fr */ { IDMS_HELP_FR, "Aide" },
/* hu */ { IDMS_HELP_HU, "Segítség" },
/* it */ { IDMS_HELP_IT, "Aiuto" },
/* ko */ { IDMS_HELP_KO, "µµ¿ò" },
/* nl */ { IDMS_HELP_NL, "Help" },
/* pl */ { IDMS_HELP_PL, "Pomoc" },
/* ru */ { IDMS_HELP_RU, "¿ÞÜÞéì" },
/* sv */ { IDMS_HELP_SV, "Hjälp" },
/* tr */ { IDMS_HELP_TR, "Yardým" },

/* en */ { IDMS_ABOUT,    "About VICE..." },
/* da */ { IDMS_ABOUT_DA, "Om VICE..." },
/* de */ { IDMS_ABOUT_DE, "Über VICE..." },
/* es */ { IDMS_ABOUT_ES, "Acerca de VICE..." },
/* fr */ { IDMS_ABOUT_FR, "À propos de VICE..." },
/* hu */ { IDMS_ABOUT_HU, "A VICE névjegye..." },
/* it */ { IDMS_ABOUT_IT, "Informazioni su VICE..." },
/* ko */ { IDMS_ABOUT_KO, "µµ¿ò VICE..." },
/* nl */ { IDMS_ABOUT_NL, "Over VICE..." },
/* pl */ { IDMS_ABOUT_PL, "O VICE..." },
/* ru */ { IDMS_ABOUT_RU, "¿ÞÔàÞÑÝÕÕ Þ VICE..." },
/* sv */ { IDMS_ABOUT_SV, "Om VICE..." },
/* tr */ { IDMS_ABOUT_TR, "VICE Hakkýnda..." },

/* en */ { IDMS_COMMAND_LINE_OPTIONS,    "Command line options" },
/* da */ { IDMS_COMMAND_LINE_OPTIONS_DA, "Kommandolinjeparametre" },
/* de */ { IDMS_COMMAND_LINE_OPTIONS_DE, "Kommandozeilen Parameter" },
/* es */ { IDMS_COMMAND_LINE_OPTIONS_ES, "Opciones de la linea de comandos" },
/* fr */ { IDMS_COMMAND_LINE_OPTIONS_FR, "Options de ligne de commande" },
/* hu */ { IDMS_COMMAND_LINE_OPTIONS_HU, "Parancssori opciók" },
/* it */ { IDMS_COMMAND_LINE_OPTIONS_IT, "Parametri della riga di comando" },
/* ko */ { IDMS_COMMAND_LINE_OPTIONS_KO, "Ä¿¸Çµå ¶óÀÎ ¿É¼Ç" },
/* nl */ { IDMS_COMMAND_LINE_OPTIONS_NL, "Opdrachtregelopties" },
/* pl */ { IDMS_COMMAND_LINE_OPTIONS_PL, "Opcje wiersza poleceñ" },
/* ru */ { IDMS_COMMAND_LINE_OPTIONS_RU, "¾ßæØØ ÚÞÜÐÝÔÝÞÙ áâàÞÚØ" },
/* sv */ { IDMS_COMMAND_LINE_OPTIONS_SV, "Kommandoradsflaggor" },
/* tr */ { IDMS_COMMAND_LINE_OPTIONS_TR, "Komut satýrý seçenekleri" },

/* en */ { IDMS_COMPILE_FEATURES,    "Compile time features" },
/* da */ { IDMS_COMPILE_FEATURES_DA, "" },  /* fuzzy */
/* de */ { IDMS_COMPILE_FEATURES_DE, "Kompilierte Funktionen" },
/* es */ { IDMS_COMPILE_FEATURES_ES, "Utilidades tiempo de compilación" },
/* fr */ { IDMS_COMPILE_FEATURES_FR, "" },  /* fuzzy */
/* hu */ { IDMS_COMPILE_FEATURES_HU, "" },  /* fuzzy */
/* it */ { IDMS_COMPILE_FEATURES_IT, "Opzioni di compilazione" },
/* ko */ { IDMS_COMPILE_FEATURES_KO, "" },  /* fuzzy */
/* nl */ { IDMS_COMPILE_FEATURES_NL, "Speciale compile-time opties" },
/* pl */ { IDMS_COMPILE_FEATURES_PL, "Opcje kompilacji" },
/* ru */ { IDMS_COMPILE_FEATURES_RU, "" },  /* fuzzy */
/* sv */ { IDMS_COMPILE_FEATURES_SV, "" },  /* fuzzy */
/* tr */ { IDMS_COMPILE_FEATURES_TR, "" },  /* fuzzy */

/* en */ { IDMS_CONTRIBUTORS,    "Contributors" },
/* da */ { IDMS_CONTRIBUTORS_DA, "Bidragsydere" },
/* de */ { IDMS_CONTRIBUTORS_DE, "VICE Hackers" },
/* es */ { IDMS_CONTRIBUTORS_ES, "Contribuyentes" },
/* fr */ { IDMS_CONTRIBUTORS_FR, "Contributeurs" },
/* hu */ { IDMS_CONTRIBUTORS_HU, "Közremûködõk" },
/* it */ { IDMS_CONTRIBUTORS_IT, "Collaboratori" },
/* ko */ { IDMS_CONTRIBUTORS_KO, "°øÇåÀÚ" },
/* nl */ { IDMS_CONTRIBUTORS_NL, "Medewerkers" },
/* pl */ { IDMS_CONTRIBUTORS_PL, "Wspó³pracownicy" },
/* ru */ { IDMS_CONTRIBUTORS_RU, "Contributors" },
/* sv */ { IDMS_CONTRIBUTORS_SV, "Bidragslämnare" },
/* tr */ { IDMS_CONTRIBUTORS_TR, "Katýlýmcýlar" },

/* en */ { IDMS_LICENSE,    "License" },
/* da */ { IDMS_LICENSE_DA, "Licens" },
/* de */ { IDMS_LICENSE_DE, "Lizenz" },
/* es */ { IDMS_LICENSE_ES, "Licencia" },
/* fr */ { IDMS_LICENSE_FR, "License" },
/* hu */ { IDMS_LICENSE_HU, "Licensz" },
/* it */ { IDMS_LICENSE_IT, "Licenza" },
/* ko */ { IDMS_LICENSE_KO, "¸éÇã" },
/* nl */ { IDMS_LICENSE_NL, "Licentie" },
/* pl */ { IDMS_LICENSE_PL, "Licencja" },
/* ru */ { IDMS_LICENSE_RU, "»ØæÕÝ×Øï" },
/* sv */ { IDMS_LICENSE_SV, "Licens" },
/* tr */ { IDMS_LICENSE_TR, "Lisans" },

/* en */ { IDMS_NO_WARRANTY,    "No warranty!" },
/* da */ { IDMS_NO_WARRANTY_DA, "Ingen garanti!" },
/* de */ { IDMS_NO_WARRANTY_DE, "Keine Garantie!" },
/* es */ { IDMS_NO_WARRANTY_ES, "¡Sin garantías!" },
/* fr */ { IDMS_NO_WARRANTY_FR, "Aucune garantie!" },
/* hu */ { IDMS_NO_WARRANTY_HU, "Nincs garancia!" },
/* it */ { IDMS_NO_WARRANTY_IT, "Nessuna garanzia!" },
/* ko */ { IDMS_NO_WARRANTY_KO, "¿ö·±Æ¼ ¾ø½À´Ï´Ù" },
/* nl */ { IDMS_NO_WARRANTY_NL, "Geen garantie!" },
/* pl */ { IDMS_NO_WARRANTY_PL, "Bez gwarancji!" },
/* ru */ { IDMS_NO_WARRANTY_RU, "±Õ× ÓÐàÐÝâØØ!" },
/* sv */ { IDMS_NO_WARRANTY_SV, "Ingen garanti!" },
/* tr */ { IDMS_NO_WARRANTY_TR, "Garanti verilmez!" },

/* en */ { IDMS_LANGUAGE_INTERNATIONAL,    "International" },
/* da */ { IDMS_LANGUAGE_INTERNATIONAL_DA, "International" },
/* de */ { IDMS_LANGUAGE_INTERNATIONAL_DE, "International" },
/* es */ { IDMS_LANGUAGE_INTERNATIONAL_ES, "Internacional" },
/* fr */ { IDMS_LANGUAGE_INTERNATIONAL_FR, "International" },
/* hu */ { IDMS_LANGUAGE_INTERNATIONAL_HU, "Nemzetközi" },
/* it */ { IDMS_LANGUAGE_INTERNATIONAL_IT, "Internazionale" },
/* ko */ { IDMS_LANGUAGE_INTERNATIONAL_KO, "±¹Á¦" },
/* nl */ { IDMS_LANGUAGE_INTERNATIONAL_NL, "Internationaal" },
/* pl */ { IDMS_LANGUAGE_INTERNATIONAL_PL, "Miêdzynarodowy" },
/* ru */ { IDMS_LANGUAGE_INTERNATIONAL_RU, "ÜÕÖÔãÝÐàÞÔÝëÙ" },
/* sv */ { IDMS_LANGUAGE_INTERNATIONAL_SV, "Internationell" },
/* tr */ { IDMS_LANGUAGE_INTERNATIONAL_TR, "Uluslararasý" },

/* en */ { IDMS_LANGUAGE_ENGLISH,    "English" },
/* da */ { IDMS_LANGUAGE_ENGLISH_DA, "Engelsk" },
/* de */ { IDMS_LANGUAGE_ENGLISH_DE, "Englisch" },
/* es */ { IDMS_LANGUAGE_ENGLISH_ES, "Inglés" },
/* fr */ { IDMS_LANGUAGE_ENGLISH_FR, "Anglais" },
/* hu */ { IDMS_LANGUAGE_ENGLISH_HU, "Angol" },
/* it */ { IDMS_LANGUAGE_ENGLISH_IT, "Inglese" },
/* ko */ { IDMS_LANGUAGE_ENGLISH_KO, "¿µ¾î" },
/* nl */ { IDMS_LANGUAGE_ENGLISH_NL, "Engels" },
/* pl */ { IDMS_LANGUAGE_ENGLISH_PL, "Angielski" },
/* ru */ { IDMS_LANGUAGE_ENGLISH_RU, "ÐÝÓÛØÙáÚØÙ" },
/* sv */ { IDMS_LANGUAGE_ENGLISH_SV, "Engelska" },
/* tr */ { IDMS_LANGUAGE_ENGLISH_TR, "Ýngilizce" },

/* en */ { IDMS_LANGUAGE_DANISH,    "Danish" },
/* da */ { IDMS_LANGUAGE_DANISH_DA, "Dansk" },
/* de */ { IDMS_LANGUAGE_DANISH_DE, "Dänisch" },
/* es */ { IDMS_LANGUAGE_DANISH_ES, "Danés" },
/* fr */ { IDMS_LANGUAGE_DANISH_FR, "Danois" },
/* hu */ { IDMS_LANGUAGE_DANISH_HU, "Dán" },
/* it */ { IDMS_LANGUAGE_DANISH_IT, "Danese" },
/* ko */ { IDMS_LANGUAGE_DANISH_KO, "µ§¸¶Å©¾î" },
/* nl */ { IDMS_LANGUAGE_DANISH_NL, "Deens" },
/* pl */ { IDMS_LANGUAGE_DANISH_PL, "Duñski" },
/* ru */ { IDMS_LANGUAGE_DANISH_RU, "ÔÐâáÚØÙ" },
/* sv */ { IDMS_LANGUAGE_DANISH_SV, "Danska" },
/* tr */ { IDMS_LANGUAGE_DANISH_TR, "Danimarkaca" },

/* en */ { IDMS_LANGUAGE_GERMAN,    "German" },
/* da */ { IDMS_LANGUAGE_GERMAN_DA, "Tysk" },
/* de */ { IDMS_LANGUAGE_GERMAN_DE, "Deutsch" },
/* es */ { IDMS_LANGUAGE_GERMAN_ES, "Alemán" },
/* fr */ { IDMS_LANGUAGE_GERMAN_FR, "Allemand" },
/* hu */ { IDMS_LANGUAGE_GERMAN_HU, "Német" },
/* it */ { IDMS_LANGUAGE_GERMAN_IT, "Tedesco" },
/* ko */ { IDMS_LANGUAGE_GERMAN_KO, "µ¶ÀÏ¾î" },
/* nl */ { IDMS_LANGUAGE_GERMAN_NL, "Duits" },
/* pl */ { IDMS_LANGUAGE_GERMAN_PL, "Niemiecki" },
/* ru */ { IDMS_LANGUAGE_GERMAN_RU, "ÝÕÜÕæÚØÙ" },
/* sv */ { IDMS_LANGUAGE_GERMAN_SV, "Tyska" },
/* tr */ { IDMS_LANGUAGE_GERMAN_TR, "Almanca" },

/* en */ { IDMS_LANGUAGE_SPANISH,    "Spanish" },
/* da */ { IDMS_LANGUAGE_SPANISH_DA, "Spansk" },
/* de */ { IDMS_LANGUAGE_SPANISH_DE, "Spanisch" },
/* es */ { IDMS_LANGUAGE_SPANISH_ES, "Español" },
/* fr */ { IDMS_LANGUAGE_SPANISH_FR, "Espagnol" },
/* hu */ { IDMS_LANGUAGE_SPANISH_HU, "" },  /* fuzzy */
/* it */ { IDMS_LANGUAGE_SPANISH_IT, "Spagnolo" },
/* ko */ { IDMS_LANGUAGE_SPANISH_KO, "" },  /* fuzzy */
/* nl */ { IDMS_LANGUAGE_SPANISH_NL, "Spaans" },
/* pl */ { IDMS_LANGUAGE_SPANISH_PL, "Hiszpañski" },
/* ru */ { IDMS_LANGUAGE_SPANISH_RU, "ØáßÐÝáÚØÙ" },
/* sv */ { IDMS_LANGUAGE_SPANISH_SV, "Spanska" },
/* tr */ { IDMS_LANGUAGE_SPANISH_TR, "Ýspanyolca" },

/* en */ { IDMS_LANGUAGE_FINNISH,    "Finnish" },
/* da */ { IDMS_LANGUAGE_FINNISH_DA, "Finsk" },
/* de */ { IDMS_LANGUAGE_FINNISH_DE, "Finnisch" },
/* es */ { IDMS_LANGUAGE_FINNISH_ES, "Finlandés" },
/* fr */ { IDMS_LANGUAGE_FINNISH_FR, "Finlandais" },
/* hu */ { IDMS_LANGUAGE_FINNISH_HU, "Finn" },
/* it */ { IDMS_LANGUAGE_FINNISH_IT, "Finlandese" },
/* ko */ { IDMS_LANGUAGE_FINNISH_KO, "ÇÉ¶õµå¾î" },
/* nl */ { IDMS_LANGUAGE_FINNISH_NL, "Fins" },
/* pl */ { IDMS_LANGUAGE_FINNISH_PL, "Fiñski" },
/* ru */ { IDMS_LANGUAGE_FINNISH_RU, "äØÝáÚØÙ" },
/* sv */ { IDMS_LANGUAGE_FINNISH_SV, "Finska" },
/* tr */ { IDMS_LANGUAGE_FINNISH_TR, "Fince" },

/* en */ { IDMS_LANGUAGE_FRENCH,    "French" },
/* da */ { IDMS_LANGUAGE_FRENCH_DA, "Fransk" },
/* de */ { IDMS_LANGUAGE_FRENCH_DE, "Französisch" },
/* es */ { IDMS_LANGUAGE_FRENCH_ES, "Francés" },
/* fr */ { IDMS_LANGUAGE_FRENCH_FR, "Français" },
/* hu */ { IDMS_LANGUAGE_FRENCH_HU, "Francia" },
/* it */ { IDMS_LANGUAGE_FRENCH_IT, "Francese" },
/* ko */ { IDMS_LANGUAGE_FRENCH_KO, "ÇÁ¶û½º¾î" },
/* nl */ { IDMS_LANGUAGE_FRENCH_NL, "Frans" },
/* pl */ { IDMS_LANGUAGE_FRENCH_PL, "Francuski" },
/* ru */ { IDMS_LANGUAGE_FRENCH_RU, "äàÐÝæã×áÚØÙ" },
/* sv */ { IDMS_LANGUAGE_FRENCH_SV, "Franska" },
/* tr */ { IDMS_LANGUAGE_FRENCH_TR, "Fransýzca" },

/* en */ { IDMS_LANGUAGE_HUNGARIAN,    "Hungarian" },
/* da */ { IDMS_LANGUAGE_HUNGARIAN_DA, "Ungarsk" },
/* de */ { IDMS_LANGUAGE_HUNGARIAN_DE, "Ungarisch" },
/* es */ { IDMS_LANGUAGE_HUNGARIAN_ES, "Húngaro" },
/* fr */ { IDMS_LANGUAGE_HUNGARIAN_FR, "Hungarian" },
/* hu */ { IDMS_LANGUAGE_HUNGARIAN_HU, "Magyar" },
/* it */ { IDMS_LANGUAGE_HUNGARIAN_IT, "Ungherese" },
/* ko */ { IDMS_LANGUAGE_HUNGARIAN_KO, "Çë°¡¸®¾î" },
/* nl */ { IDMS_LANGUAGE_HUNGARIAN_NL, "Hongaars" },
/* pl */ { IDMS_LANGUAGE_HUNGARIAN_PL, "Wêgierski" },
/* ru */ { IDMS_LANGUAGE_HUNGARIAN_RU, "ÒÕÝÓÕàáÚØÙ" },
/* sv */ { IDMS_LANGUAGE_HUNGARIAN_SV, "Ungerska" },
/* tr */ { IDMS_LANGUAGE_HUNGARIAN_TR, "Macarca" },

/* en */ { IDMS_LANGUAGE_ITALIAN,    "Italian" },
/* da */ { IDMS_LANGUAGE_ITALIAN_DA, "Italiensk" },
/* de */ { IDMS_LANGUAGE_ITALIAN_DE, "Italienisch" },
/* es */ { IDMS_LANGUAGE_ITALIAN_ES, "Italiano" },
/* fr */ { IDMS_LANGUAGE_ITALIAN_FR, "Italien" },
/* hu */ { IDMS_LANGUAGE_ITALIAN_HU, "Olasz" },
/* it */ { IDMS_LANGUAGE_ITALIAN_IT, "Italiano" },
/* ko */ { IDMS_LANGUAGE_ITALIAN_KO, "ÀÌÅ»¸®¾Æ¾î" },
/* nl */ { IDMS_LANGUAGE_ITALIAN_NL, "Italiaans" },
/* pl */ { IDMS_LANGUAGE_ITALIAN_PL, "W³oski" },
/* ru */ { IDMS_LANGUAGE_ITALIAN_RU, "ØâÐÛìïÝáÚØÙ" },
/* sv */ { IDMS_LANGUAGE_ITALIAN_SV, "Italienska" },
/* tr */ { IDMS_LANGUAGE_ITALIAN_TR, "Ýtalyanca" },

/* en */ { IDMS_LANGUAGE_KOREAN,    "Korean" },
/* da */ { IDMS_LANGUAGE_KOREAN_DA, "Koreansk" },
/* de */ { IDMS_LANGUAGE_KOREAN_DE, "Koreanisch" },
/* es */ { IDMS_LANGUAGE_KOREAN_ES, "Coreano" },
/* fr */ { IDMS_LANGUAGE_KOREAN_FR, "Koréen" },
/* hu */ { IDMS_LANGUAGE_KOREAN_HU, "" },  /* fuzzy */
/* it */ { IDMS_LANGUAGE_KOREAN_IT, "Coreano" },
/* ko */ { IDMS_LANGUAGE_KOREAN_KO, "ÇÑ±¹¾î" },
/* nl */ { IDMS_LANGUAGE_KOREAN_NL, "Koreaans" },
/* pl */ { IDMS_LANGUAGE_KOREAN_PL, "Koreañski" },
/* ru */ { IDMS_LANGUAGE_KOREAN_RU, "" },  /* fuzzy */
/* sv */ { IDMS_LANGUAGE_KOREAN_SV, "Koreanska" },
/* tr */ { IDMS_LANGUAGE_KOREAN_TR, "Korece" },

/* en */ { IDMS_LANGUAGE_DUTCH,    "Dutch" },
/* da */ { IDMS_LANGUAGE_DUTCH_DA, "Hollandsk" },
/* de */ { IDMS_LANGUAGE_DUTCH_DE, "Holländisch" },
/* es */ { IDMS_LANGUAGE_DUTCH_ES, "Holandés" },
/* fr */ { IDMS_LANGUAGE_DUTCH_FR, "Hollandais" },
/* hu */ { IDMS_LANGUAGE_DUTCH_HU, "Holland" },
/* it */ { IDMS_LANGUAGE_DUTCH_IT, "Olandese" },
/* ko */ { IDMS_LANGUAGE_DUTCH_KO, "³×´ú¶õµå¾î" },
/* nl */ { IDMS_LANGUAGE_DUTCH_NL, "Nederlands" },
/* pl */ { IDMS_LANGUAGE_DUTCH_PL, "Holenderski" },
/* ru */ { IDMS_LANGUAGE_DUTCH_RU, "ÓÞÛÛÐÝÔáÚØÙ" },
/* sv */ { IDMS_LANGUAGE_DUTCH_SV, "Nederländska" },
/* tr */ { IDMS_LANGUAGE_DUTCH_TR, "Hollandaca" },

/* en */ { IDMS_LANGUAGE_POLISH,    "Polish" },
/* da */ { IDMS_LANGUAGE_POLISH_DA, "Polsk" },
/* de */ { IDMS_LANGUAGE_POLISH_DE, "Polnisch" },
/* es */ { IDMS_LANGUAGE_POLISH_ES, "Polaco" },
/* fr */ { IDMS_LANGUAGE_POLISH_FR, "Polonais" },
/* hu */ { IDMS_LANGUAGE_POLISH_HU, "Lengyel" },
/* it */ { IDMS_LANGUAGE_POLISH_IT, "Polacco" },
/* ko */ { IDMS_LANGUAGE_POLISH_KO, "Æú¶õµå¾î" },
/* nl */ { IDMS_LANGUAGE_POLISH_NL, "Pools" },
/* pl */ { IDMS_LANGUAGE_POLISH_PL, "Polski" },
/* ru */ { IDMS_LANGUAGE_POLISH_RU, "ßÞÛìáÚØÙ" },
/* sv */ { IDMS_LANGUAGE_POLISH_SV, "Polska" },
/* tr */ { IDMS_LANGUAGE_POLISH_TR, "Polonyaca" },

/* en */ { IDMS_LANGUAGE_NORWEGIAN,    "Norwegian" },
/* da */ { IDMS_LANGUAGE_NORWEGIAN_DA, "Norsk" },
/* de */ { IDMS_LANGUAGE_NORWEGIAN_DE, "Norwegisch" },
/* es */ { IDMS_LANGUAGE_NORWEGIAN_ES, "Noruego" },
/* fr */ { IDMS_LANGUAGE_NORWEGIAN_FR, "Norvégien" },
/* hu */ { IDMS_LANGUAGE_NORWEGIAN_HU, "Norvég" },
/* it */ { IDMS_LANGUAGE_NORWEGIAN_IT, "Norvegese" },
/* ko */ { IDMS_LANGUAGE_NORWEGIAN_KO, "³ë¸£¿þÀÌ¾î" },
/* nl */ { IDMS_LANGUAGE_NORWEGIAN_NL, "Noors" },
/* pl */ { IDMS_LANGUAGE_NORWEGIAN_PL, "Norweski" },
/* ru */ { IDMS_LANGUAGE_NORWEGIAN_RU, "ÝÞàÒÕÖáÚØÙ" },
/* sv */ { IDMS_LANGUAGE_NORWEGIAN_SV, "Norska" },
/* tr */ { IDMS_LANGUAGE_NORWEGIAN_TR, "Norveççe" },

/* en */ { IDMS_LANGUAGE_RUSSIAN,    "Russian" },
/* da */ { IDMS_LANGUAGE_RUSSIAN_DA, "Russisk" },
/* de */ { IDMS_LANGUAGE_RUSSIAN_DE, "Russisch" },
/* es */ { IDMS_LANGUAGE_RUSSIAN_ES, "Ruso" },
/* fr */ { IDMS_LANGUAGE_RUSSIAN_FR, "Russe" },
/* hu */ { IDMS_LANGUAGE_RUSSIAN_HU, "" },  /* fuzzy */
/* it */ { IDMS_LANGUAGE_RUSSIAN_IT, "Russo" },
/* ko */ { IDMS_LANGUAGE_RUSSIAN_KO, "·¯½Ã¾Æ¾î" },
/* nl */ { IDMS_LANGUAGE_RUSSIAN_NL, "Russisch" },
/* pl */ { IDMS_LANGUAGE_RUSSIAN_PL, "Rosyjski" },
/* ru */ { IDMS_LANGUAGE_RUSSIAN_RU, "àãááÚØÙ" },
/* sv */ { IDMS_LANGUAGE_RUSSIAN_SV, "Ryska" },
/* tr */ { IDMS_LANGUAGE_RUSSIAN_TR, "Rusça" },

/* en */ { IDMS_LANGUAGE_SWEDISH,    "Swedish" },
/* da */ { IDMS_LANGUAGE_SWEDISH_DA, "Svensk" },
/* de */ { IDMS_LANGUAGE_SWEDISH_DE, "Schwedisch" },
/* es */ { IDMS_LANGUAGE_SWEDISH_ES, "Sueco" },
/* fr */ { IDMS_LANGUAGE_SWEDISH_FR, "Suédois" },
/* hu */ { IDMS_LANGUAGE_SWEDISH_HU, "Svéd" },
/* it */ { IDMS_LANGUAGE_SWEDISH_IT, "Svedese" },
/* ko */ { IDMS_LANGUAGE_SWEDISH_KO, "½º¿þµ§¾î" },
/* nl */ { IDMS_LANGUAGE_SWEDISH_NL, "Zweeds" },
/* pl */ { IDMS_LANGUAGE_SWEDISH_PL, "Szwedzki" },
/* ru */ { IDMS_LANGUAGE_SWEDISH_RU, "èÒÕÔáÚØÙ" },
/* sv */ { IDMS_LANGUAGE_SWEDISH_SV, "Svenska" },
/* tr */ { IDMS_LANGUAGE_SWEDISH_TR, "Ýsveççe" },

/* en */ { IDMS_LANGUAGE_SWISS,    "Swiss" },
/* da */ { IDMS_LANGUAGE_SWISS_DA, "" },  /* fuzzy */
/* de */ { IDMS_LANGUAGE_SWISS_DE, "Schweiz" },
/* es */ { IDMS_LANGUAGE_SWISS_ES, "Suizo" },
/* fr */ { IDMS_LANGUAGE_SWISS_FR, "" },  /* fuzzy */
/* hu */ { IDMS_LANGUAGE_SWISS_HU, "" },  /* fuzzy */
/* it */ { IDMS_LANGUAGE_SWISS_IT, "Svizzero" },
/* ko */ { IDMS_LANGUAGE_SWISS_KO, "" },  /* fuzzy */
/* nl */ { IDMS_LANGUAGE_SWISS_NL, "Zwitsers" },
/* pl */ { IDMS_LANGUAGE_SWISS_PL, "Szwajcarski" },
/* ru */ { IDMS_LANGUAGE_SWISS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_LANGUAGE_SWISS_SV, "" },  /* fuzzy */
/* tr */ { IDMS_LANGUAGE_SWISS_TR, "" },  /* fuzzy */

/* en */ { IDMS_LANGUAGE_TURKISH,    "Turkish" },
/* da */ { IDMS_LANGUAGE_TURKISH_DA, "Tyrkisk" },
/* de */ { IDMS_LANGUAGE_TURKISH_DE, "Türkisch" },
/* es */ { IDMS_LANGUAGE_TURKISH_ES, "Turco" },
/* fr */ { IDMS_LANGUAGE_TURKISH_FR, "Turc" },
/* hu */ { IDMS_LANGUAGE_TURKISH_HU, "Török" },
/* it */ { IDMS_LANGUAGE_TURKISH_IT, "Turco" },
/* ko */ { IDMS_LANGUAGE_TURKISH_KO, "ÅÍÅ°¾î" },
/* nl */ { IDMS_LANGUAGE_TURKISH_NL, "Turks" },
/* pl */ { IDMS_LANGUAGE_TURKISH_PL, "Turecki" },
/* ru */ { IDMS_LANGUAGE_TURKISH_RU, "âãàÕæÚØÙ" },
/* sv */ { IDMS_LANGUAGE_TURKISH_SV, "Turkiska" },
/* tr */ { IDMS_LANGUAGE_TURKISH_TR, "Türkçe" },

/* en */ { IDMS_LANGUAGE,    "Language" },
/* da */ { IDMS_LANGUAGE_DA, "Sprog" },
/* de */ { IDMS_LANGUAGE_DE, "Sprache" },
/* es */ { IDMS_LANGUAGE_ES, "Idioma" },
/* fr */ { IDMS_LANGUAGE_FR, "Langage" },
/* hu */ { IDMS_LANGUAGE_HU, "Nyelv" },
/* it */ { IDMS_LANGUAGE_IT, "Lingua" },
/* ko */ { IDMS_LANGUAGE_KO, "¾ð¾î" },
/* nl */ { IDMS_LANGUAGE_NL, "Taal" },
/* pl */ { IDMS_LANGUAGE_PL, "Jêzyk" },
/* ru */ { IDMS_LANGUAGE_RU, "Ï×ëÚ" },
/* sv */ { IDMS_LANGUAGE_SV, "Språk" },
/* tr */ { IDMS_LANGUAGE_TR, "Dil" },

/* en */ { IDMS_FULLSCREEN_STATUSBAR,    "Fullscreen Statusbar" },
/* da */ { IDMS_FULLSCREEN_STATUSBAR_DA, "Statusbar i fullscreen" },
/* de */ { IDMS_FULLSCREEN_STATUSBAR_DE, "Vollbild Statusleiste" },
/* es */ { IDMS_FULLSCREEN_STATUSBAR_ES, "Barra estado pantalla completa" },
/* fr */ { IDMS_FULLSCREEN_STATUSBAR_FR, "Barre de statut Plein écran" },
/* hu */ { IDMS_FULLSCREEN_STATUSBAR_HU, "Állapotsor teljes képernyõnél" },
/* it */ { IDMS_FULLSCREEN_STATUSBAR_IT, "Barra di stato nella modalità a schermo intero" },
/* ko */ { IDMS_FULLSCREEN_STATUSBAR_KO, "" },  /* fuzzy */
/* nl */ { IDMS_FULLSCREEN_STATUSBAR_NL, "Volledig Scherm Statusbalk" },
/* pl */ { IDMS_FULLSCREEN_STATUSBAR_PL, "Pasek stanu w trybie pe³nego ekranu" },
/* ru */ { IDMS_FULLSCREEN_STATUSBAR_RU, "Fullscreen Statusbar" },
/* sv */ { IDMS_FULLSCREEN_STATUSBAR_SV, "Statusrad i fullskärm" },
/* tr */ { IDMS_FULLSCREEN_STATUSBAR_TR, "Tam ekran durum çubuðu" },

/* en */ { IDMS_VIDEO_OVERLAY,    "Video overlay" },
/* da */ { IDMS_VIDEO_OVERLAY_DA, "Video-overlay" },
/* de */ { IDMS_VIDEO_OVERLAY_DE, "Video Overlay" },
/* es */ { IDMS_VIDEO_OVERLAY_ES, "Vídeo overlay" },
/* fr */ { IDMS_VIDEO_OVERLAY_FR, "Superposition vidéo" },
/* hu */ { IDMS_VIDEO_OVERLAY_HU, "Videó overlay" },
/* it */ { IDMS_VIDEO_OVERLAY_IT, "Overlay video" },
/* ko */ { IDMS_VIDEO_OVERLAY_KO, "" },  /* fuzzy */
/* nl */ { IDMS_VIDEO_OVERLAY_NL, "Video overlay" },
/* pl */ { IDMS_VIDEO_OVERLAY_PL, "Video overlay" },
/* ru */ { IDMS_VIDEO_OVERLAY_RU, "Video overlay" },
/* sv */ { IDMS_VIDEO_OVERLAY_SV, "Video-overlay" },
/* tr */ { IDMS_VIDEO_OVERLAY_TR, "Görüntü bindirme" },

/* en */ { IDMS_START_SOUND_RECORD,    "Start Sound Record..." },
/* da */ { IDMS_START_SOUND_RECORD_DA, "Start lydoptagelse..." },
/* de */ { IDMS_START_SOUND_RECORD_DE, "Tonaufnahme starten..." },
/* es */ { IDMS_START_SOUND_RECORD_ES, "Iniciar grabación sonido..." },
/* fr */ { IDMS_START_SOUND_RECORD_FR, "Démarrer l'enregistrement du son..." },
/* hu */ { IDMS_START_SOUND_RECORD_HU, "" },  /* fuzzy */
/* it */ { IDMS_START_SOUND_RECORD_IT, "Avvia la registrazione audio..." },
/* ko */ { IDMS_START_SOUND_RECORD_KO, "" },  /* fuzzy */
/* nl */ { IDMS_START_SOUND_RECORD_NL, "Start Geluidsopname..." },
/* pl */ { IDMS_START_SOUND_RECORD_PL, "Rozpocznij zapis d¼wiêku..." },
/* ru */ { IDMS_START_SOUND_RECORD_RU, "½ÐçÐâì ×ÐßØáì ×ÒãÚÐ..." },
/* sv */ { IDMS_START_SOUND_RECORD_SV, "Starta ljudinspelning..." },
/* tr */ { IDMS_START_SOUND_RECORD_TR, "Ses Kaydýný Baþlat..." },

/* en */ { IDMS_STOP_SOUND_RECORD,    "Stop Sound record" },
/* da */ { IDMS_STOP_SOUND_RECORD_DA, "Stop lydoptagelse" },
/* de */ { IDMS_STOP_SOUND_RECORD_DE, "Tonaufnahme stoppen" },
/* es */ { IDMS_STOP_SOUND_RECORD_ES, "Finalizar grabación de sonido" },
/* fr */ { IDMS_STOP_SOUND_RECORD_FR, "Arrêter l'enregistrement du son" },
/* hu */ { IDMS_STOP_SOUND_RECORD_HU, "Hangfelvétel leállítása" },
/* it */ { IDMS_STOP_SOUND_RECORD_IT, "Interrompi registrazione audio" },
/* ko */ { IDMS_STOP_SOUND_RECORD_KO, "»ç¿îµå ·¹ÄÚµå ¸ØÃã" },
/* nl */ { IDMS_STOP_SOUND_RECORD_NL, "Stop geluidsopname" },
/* pl */ { IDMS_STOP_SOUND_RECORD_PL, "Zatrzymaj zapis d¼wiêku" },
/* ru */ { IDMS_STOP_SOUND_RECORD_RU, "¾áâÐÝÞÒØâì ×ÐßØáì ×ÒãÚÐ" },
/* sv */ { IDMS_STOP_SOUND_RECORD_SV, "Stoppa ljudinspelning" },
/* tr */ { IDMS_STOP_SOUND_RECORD_TR, "Ses kaydýný durdur" },

/* en */ { IDMS_LOAD_PSID_FILE,    "Load PSID file" },
/* da */ { IDMS_LOAD_PSID_FILE_DA, "Indlæs PSID-fil" },
/* de */ { IDMS_LOAD_PSID_FILE_DE, "PSID Datei laden" },
/* es */ { IDMS_LOAD_PSID_FILE_ES, "Cargar fichero PSID" },
/* fr */ { IDMS_LOAD_PSID_FILE_FR, "Charger le fichier PSID" },
/* hu */ { IDMS_LOAD_PSID_FILE_HU, "PSID fájl betöltése" },
/* it */ { IDMS_LOAD_PSID_FILE_IT, "Carica file PSID" },
/* ko */ { IDMS_LOAD_PSID_FILE_KO, "PSID ÆÄÀÏ ºÒ·¯¿À±â" },
/* nl */ { IDMS_LOAD_PSID_FILE_NL, "Laad PSID bestand" },
/* pl */ { IDMS_LOAD_PSID_FILE_PL, "Wczytaj plik PSID" },
/* ru */ { IDMS_LOAD_PSID_FILE_RU, "Load PSID file" },
/* sv */ { IDMS_LOAD_PSID_FILE_SV, "Läs PSID-fil" },
/* tr */ { IDMS_LOAD_PSID_FILE_TR, "PSID dosyasý yükle" },

/* en */ { IDMS_NEXT_TUNE,    "Next tune" },
/* da */ { IDMS_NEXT_TUNE_DA, "Næste sang" },
/* de */ { IDMS_NEXT_TUNE_DE, "Nächster Titel " },
/* es */ { IDMS_NEXT_TUNE_ES, "Siguiente melodia" },
/* fr */ { IDMS_NEXT_TUNE_FR, "Clip suivant" },
/* hu */ { IDMS_NEXT_TUNE_HU, "" },  /* fuzzy */
/* it */ { IDMS_NEXT_TUNE_IT, "Melodia successiva" },
/* ko */ { IDMS_NEXT_TUNE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_NEXT_TUNE_NL, "Volgende melodie" },
/* pl */ { IDMS_NEXT_TUNE_PL, "Nastêpny ton" },
/* ru */ { IDMS_NEXT_TUNE_RU, "" },  /* fuzzy */
/* sv */ { IDMS_NEXT_TUNE_SV, "Nästa låt" },
/* tr */ { IDMS_NEXT_TUNE_TR, "Sonraki þarký" },

/* en */ { IDMS_PREVIOUS_TUNE,    "Previous tune" },
/* da */ { IDMS_PREVIOUS_TUNE_DA, "Forrige sang" },
/* de */ { IDMS_PREVIOUS_TUNE_DE, "Vorheriger Titel " },
/* es */ { IDMS_PREVIOUS_TUNE_ES, "Melodía previa" },
/* fr */ { IDMS_PREVIOUS_TUNE_FR, "Clip précédent" },
/* hu */ { IDMS_PREVIOUS_TUNE_HU, "" },  /* fuzzy */
/* it */ { IDMS_PREVIOUS_TUNE_IT, "Melodia precedente" },
/* ko */ { IDMS_PREVIOUS_TUNE_KO, "" },  /* fuzzy */
/* nl */ { IDMS_PREVIOUS_TUNE_NL, "Vorige melodie" },
/* pl */ { IDMS_PREVIOUS_TUNE_PL, "Poprzedni ton" },
/* ru */ { IDMS_PREVIOUS_TUNE_RU, "" },  /* fuzzy */
/* sv */ { IDMS_PREVIOUS_TUNE_SV, "Föregående låt" },
/* tr */ { IDMS_PREVIOUS_TUNE_TR, "Önceki þarký" },

/* en */ { IDMS_MEDIA,    "Media" },
/* da */ { IDMS_MEDIA_DA, "Medie" },
/* de */ { IDMS_MEDIA_DE, "Medien" },
/* es */ { IDMS_MEDIA_ES, "Medio" },
/* fr */ { IDMS_MEDIA_FR, "Média" },
/* hu */ { IDMS_MEDIA_HU, "" },  /* fuzzy */
/* it */ { IDMS_MEDIA_IT, "Media" },
/* ko */ { IDMS_MEDIA_KO, "" },  /* fuzzy */
/* nl */ { IDMS_MEDIA_NL, "Media" },
/* pl */ { IDMS_MEDIA_PL, "Media" },
/* ru */ { IDMS_MEDIA_RU, "" },  /* fuzzy */
/* sv */ { IDMS_MEDIA_SV, "Media" },
/* tr */ { IDMS_MEDIA_TR, "Ortam" },

/* en */ { IDMS_OVERRIDE_PSID_SETTINGS,    "Override PSID settings" },
/* da */ { IDMS_OVERRIDE_PSID_SETTINGS_DA, "Overstyr PSID-indstillinger" },
/* de */ { IDMS_OVERRIDE_PSID_SETTINGS_DE, "PSID Einstellungen ignorieren" },
/* es */ { IDMS_OVERRIDE_PSID_SETTINGS_ES, "No utilizar ajustes PSID" },
/* fr */ { IDMS_OVERRIDE_PSID_SETTINGS_FR, "Écraser les paramètres PSID" },
/* hu */ { IDMS_OVERRIDE_PSID_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDMS_OVERRIDE_PSID_SETTINGS_IT, "Sovrascrivi impostazioni PSID" },
/* ko */ { IDMS_OVERRIDE_PSID_SETTINGS_KO, "PSID ¼ÂÆÃ ¿À¹ö¶óÀÌµå ÇÏ±â" },
/* nl */ { IDMS_OVERRIDE_PSID_SETTINGS_NL, "Overschrijf PSID instellingen" },
/* pl */ { IDMS_OVERRIDE_PSID_SETTINGS_PL, "Nadpisz ustawienia PSID" },
/* ru */ { IDMS_OVERRIDE_PSID_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDMS_OVERRIDE_PSID_SETTINGS_SV, "Överskugga PSID-inställningar" },
/* tr */ { IDMS_OVERRIDE_PSID_SETTINGS_TR, "PSID ayarlarýný geçersiz kýl" },

/* en */ { IDMES_VALUE_D_OUT_OF_RANGE_USING_D_INSTEAD,    "Value %d out of range, range is from %d to %d, using %d instead" },
/* da */ { IDMES_VALUE_D_OUT_OF_RANGE_USING_D_INSTEAD_DA, "Værdi %d ugyldig. Tilladt værdi fra %d til %d. Bruger %d istedet" },
/* de */ { IDMES_VALUE_D_OUT_OF_RANGE_USING_D_INSTEAD_DE, "Wert %d ist unzulässig, Bereich muss zwischen %d und %d liegen, benutze %d" },
/* es */ { IDMES_VALUE_D_OUT_OF_RANGE_USING_D_INSTEAD_ES, "Valor %d fuera de rango, rango de %d a %d, usando %d en su lugar" },
/* fr */ { IDMES_VALUE_D_OUT_OF_RANGE_USING_D_INSTEAD_FR, "Valeur %d en dehors de la plage %d to %d, utilisation de %d à la place" },
/* hu */ { IDMES_VALUE_D_OUT_OF_RANGE_USING_D_INSTEAD_HU, "" },  /* fuzzy */
/* it */ { IDMES_VALUE_D_OUT_OF_RANGE_USING_D_INSTEAD_IT, "Valore %d fuori dai limiti, l' intervallo è da %d a %d, uso %d al suo posto" },
/* ko */ { IDMES_VALUE_D_OUT_OF_RANGE_USING_D_INSTEAD_KO, "" },  /* fuzzy */
/* nl */ { IDMES_VALUE_D_OUT_OF_RANGE_USING_D_INSTEAD_NL, "Waarde %d is buiten het bereik, het bereik is van %d tot %d, %d zal gebruikt worden" },
/* pl */ { IDMES_VALUE_D_OUT_OF_RANGE_USING_D_INSTEAD_PL, "Warto¶æ %d jest poza zakresem, zakres to %d do %d, u¿ywam %d" },
/* ru */ { IDMES_VALUE_D_OUT_OF_RANGE_USING_D_INSTEAD_RU, "Value %d out of range, range is from %d to %d, using %d instead" },
/* sv */ { IDMES_VALUE_D_OUT_OF_RANGE_USING_D_INSTEAD_SV, "Värdet %d är utanför gränserna, giltiga värden mellan %d och %d, använder %d istället" },
/* tr */ { IDMES_VALUE_D_OUT_OF_RANGE_USING_D_INSTEAD_TR, "%d deðeri alan dýþýna çýktý, alan %d - %d aralýðýnda, yerine %d kullanýlýyor" },

/* en */ { IDMES_VALUE_F_OUT_OF_RANGE_USING_F_INSTEAD,    "Value %.3f out of range, range is from %.3f to %.3f, using %.3f instead" },
/* da */ { IDMES_VALUE_F_OUT_OF_RANGE_USING_F_INSTEAD_DA, "Værdi %.3f ugyldig. Tilladt værdi fra %.3f til %.3f. Bruger %.3f istedet" },
/* de */ { IDMES_VALUE_F_OUT_OF_RANGE_USING_F_INSTEAD_DE, "Wert %.3f ist unzulässig, Bereich muss zwischen %.3f und %.3f liegen, benutze %.3f" },
/* es */ { IDMES_VALUE_F_OUT_OF_RANGE_USING_F_INSTEAD_ES, "Valor %.3f fuera de rango, rango de %.3f a %.3f, usando %.3f en su lugar" },
/* fr */ { IDMES_VALUE_F_OUT_OF_RANGE_USING_F_INSTEAD_FR, "Valeur %.3f en dehors de la plage %.3f to %.3f, utilisation de %.3f à la place" },
/* hu */ { IDMES_VALUE_F_OUT_OF_RANGE_USING_F_INSTEAD_HU, "" },  /* fuzzy */
/* it */ { IDMES_VALUE_F_OUT_OF_RANGE_USING_F_INSTEAD_IT, "Valore %.3f fuori dai limiti, l' intervallo è da %.3f a %.3f, uso %.3f al suo posto" },
/* ko */ { IDMES_VALUE_F_OUT_OF_RANGE_USING_F_INSTEAD_KO, "" },  /* fuzzy */
/* nl */ { IDMES_VALUE_F_OUT_OF_RANGE_USING_F_INSTEAD_NL, "Waarde %.3f is buiten het bereik, het bereik is van %.3f tot %.3f, %.3f zal gebruikt worden" },
/* pl */ { IDMES_VALUE_F_OUT_OF_RANGE_USING_F_INSTEAD_PL, "Warto¶æ %.3f jest poza zakresem, zakres to %.3f do %.3f, u¿ywam %.3f" },
/* ru */ { IDMES_VALUE_F_OUT_OF_RANGE_USING_F_INSTEAD_RU, "Value %.3f out of range, range is from %.3f to %.3f, using %.3f instead" },
/* sv */ { IDMES_VALUE_F_OUT_OF_RANGE_USING_F_INSTEAD_SV, "Värdet %.3f är utanför gränserna, giltiga värden mellan %.3f och %.3f, använder %.3f istället" },
/* tr */ { IDMES_VALUE_F_OUT_OF_RANGE_USING_F_INSTEAD_TR, "%.3f deðeri alan dýþýna çýktý, alan %.3f - %.3f aralýðýnda, yerine %.3f kullanýlýyor" },

/* en */ { IDMES_SETTINGS_SAVED_SUCCESS,    "Settings saved successfully." },
/* da */ { IDMES_SETTINGS_SAVED_SUCCESS_DA, "Indstillingerne er gemt." },
/* de */ { IDMES_SETTINGS_SAVED_SUCCESS_DE, "Einstellungen erfolgreich gespeichert" },
/* es */ { IDMES_SETTINGS_SAVED_SUCCESS_ES, "Ajustes grabados correctamente" },
/* fr */ { IDMES_SETTINGS_SAVED_SUCCESS_FR, "Sauvegarde des paramètres effectuée correctement." },
/* hu */ { IDMES_SETTINGS_SAVED_SUCCESS_HU, "Beállítások sikeresen elmentve." },
/* it */ { IDMES_SETTINGS_SAVED_SUCCESS_IT, "Impostazioni salvate con successo." },
/* ko */ { IDMES_SETTINGS_SAVED_SUCCESS_KO, "" },  /* fuzzy */
/* nl */ { IDMES_SETTINGS_SAVED_SUCCESS_NL, "Instellingen met succes opgeslagen." },
/* pl */ { IDMES_SETTINGS_SAVED_SUCCESS_PL, "Zapisywanie ustawieñ powiod³o siê." },
/* ru */ { IDMES_SETTINGS_SAVED_SUCCESS_RU, "½ÐáâàÞÙÚØ ãáßÕèÝÞ áÞåàÐÝÕÝë" },
/* sv */ { IDMES_SETTINGS_SAVED_SUCCESS_SV, "Inställningarna har sparats." },
/* tr */ { IDMES_SETTINGS_SAVED_SUCCESS_TR, "Ayarlar baþarýyla kaydedildi." },

/* en */ { IDMES_SETTINGS_LOAD_SUCCESS,    "Settings loaded successfully." },
/* da */ { IDMES_SETTINGS_LOAD_SUCCESS_DA, "Indstillinger indlæst med success." },
/* de */ { IDMES_SETTINGS_LOAD_SUCCESS_DE, "Einstellungen erfolgreich geladen." },
/* es */ { IDMES_SETTINGS_LOAD_SUCCESS_ES, "Ajustes cargados con éxito." },
/* fr */ { IDMES_SETTINGS_LOAD_SUCCESS_FR, "Chargement des paramètres réussi." },
/* hu */ { IDMES_SETTINGS_LOAD_SUCCESS_HU, "Beállítások sikeresen elmentve." },
/* it */ { IDMES_SETTINGS_LOAD_SUCCESS_IT, "Impostazioni caricate con successo." },
/* ko */ { IDMES_SETTINGS_LOAD_SUCCESS_KO, "¼ÂÆÃÀÌ ¼º°øÀûÀ¸·Î ºÒ·¯¿À±â ‰ç½À´Ï´Ù" },
/* nl */ { IDMES_SETTINGS_LOAD_SUCCESS_NL, "Instellingen met succes geladen." },
/* pl */ { IDMES_SETTINGS_LOAD_SUCCESS_PL, "Wczytywanie ustawieñ powiod³o siê." },
/* ru */ { IDMES_SETTINGS_LOAD_SUCCESS_RU, "½ÐáâàÞÙÚØ ãáßÕèÝÞ ×ÐÓàãÖÕÝë." },
/* sv */ { IDMES_SETTINGS_LOAD_SUCCESS_SV, "Inställningarna har lästs in." },
/* tr */ { IDMES_SETTINGS_LOAD_SUCCESS_TR, "Ayarlar baþarýyla yüklendi." },

/* en */ { IDMES_DFLT_SETTINGS_RESTORED,    "Default settings restored." },
/* da */ { IDMES_DFLT_SETTINGS_RESTORED_DA, "Standardindstillinger gendannet." },
/* de */ { IDMES_DFLT_SETTINGS_RESTORED_DE, "Standard wiederhergestellt" },
/* es */ { IDMES_DFLT_SETTINGS_RESTORED_ES, "Ajustes por defecto restaurados." },
/* fr */ { IDMES_DFLT_SETTINGS_RESTORED_FR, "Paramètres par défaut restaurés." },
/* hu */ { IDMES_DFLT_SETTINGS_RESTORED_HU, "Alapértelmezett beállítások visszaállítva." },
/* it */ { IDMES_DFLT_SETTINGS_RESTORED_IT, "Impostazioni originarie ripristinate." },
/* ko */ { IDMES_DFLT_SETTINGS_RESTORED_KO, "" },  /* fuzzy */
/* nl */ { IDMES_DFLT_SETTINGS_RESTORED_NL, "Standaard instellingen hersteld." },
/* pl */ { IDMES_DFLT_SETTINGS_RESTORED_PL, "Przywróono domy¶lne ustawienia." },
/* ru */ { IDMES_DFLT_SETTINGS_RESTORED_RU, "½ÐáâàÞÙÚØ ßÞ ãÜÞÛçÐÝØî ÒÞááâÐÝÞÒÛÕÝë" },
/* sv */ { IDMES_DFLT_SETTINGS_RESTORED_SV, "Förvalda inställningar återställda." },
/* tr */ { IDMES_DFLT_SETTINGS_RESTORED_TR, "Varsayýlan ayarlara geri dönüldü." },

/* en */ { IDMES_VICE_MESSAGE,    "VICE Message" },
/* da */ { IDMES_VICE_MESSAGE_DA, "VICE-besked" },
/* de */ { IDMES_VICE_MESSAGE_DE, "VICE Nachricht" },
/* es */ { IDMES_VICE_MESSAGE_ES, "Mensaje VICE" },
/* fr */ { IDMES_VICE_MESSAGE_FR, "Message VICE" },
/* hu */ { IDMES_VICE_MESSAGE_HU, "VICE üzenet" },
/* it */ { IDMES_VICE_MESSAGE_IT, "Messaggio di VICE" },
/* ko */ { IDMES_VICE_MESSAGE_KO, "VICE ¸Þ¼¼Áö" },
/* nl */ { IDMES_VICE_MESSAGE_NL, "VICE Bericht" },
/* pl */ { IDMES_VICE_MESSAGE_PL, "Komunikat VICE" },
/* ru */ { IDMES_VICE_MESSAGE_RU, "ÁÞÞÑéÕÝØÕ VICE" },
/* sv */ { IDMES_VICE_MESSAGE_SV, "VICE-meddelande" },
/* tr */ { IDMES_VICE_MESSAGE_TR, "VICE Mesajý" },

/* en */ { IDMES_OK,    "OK" },
/* da */ { IDMES_OK_DA, "OK" },
/* de */ { IDMES_OK_DE, "OK" },
/* es */ { IDMES_OK_ES, "OK" },
/* fr */ { IDMES_OK_FR, "OK" },
/* hu */ { IDMES_OK_HU, "OK" },
/* it */ { IDMES_OK_IT, "OK" },
/* ko */ { IDMES_OK_KO, "ok" },
/* nl */ { IDMES_OK_NL, "OK" },
/* pl */ { IDMES_OK_PL, "OK" },
/* ru */ { IDMES_OK_RU, "OK" },
/* sv */ { IDMES_OK_SV, "OK" },
/* tr */ { IDMES_OK_TR, "Tamam" },

/* en */ { IDMES_VICE_CONTRIBUTORS,    "VICE contributors" },
/* da */ { IDMES_VICE_CONTRIBUTORS_DA, "VICE bidragsydere" },
/* de */ { IDMES_VICE_CONTRIBUTORS_DE, "VICE Hackers" },
/* es */ { IDMES_VICE_CONTRIBUTORS_ES, "Colaboradores VICE" },
/* fr */ { IDMES_VICE_CONTRIBUTORS_FR, "Contributeurs à VICE" },
/* hu */ { IDMES_VICE_CONTRIBUTORS_HU, "VICE - közremûködõk" },
/* it */ { IDMES_VICE_CONTRIBUTORS_IT, "Collaboratori" },
/* ko */ { IDMES_VICE_CONTRIBUTORS_KO, "VICE °ü°èÀÚ" },
/* nl */ { IDMES_VICE_CONTRIBUTORS_NL, "VICE medewerkers" },
/* pl */ { IDMES_VICE_CONTRIBUTORS_PL, "Wspó³pracownicy VICE" },
/* ru */ { IDMES_VICE_CONTRIBUTORS_RU, "VICE contributors" },
/* sv */ { IDMES_VICE_CONTRIBUTORS_SV, "Bidragslämnare till VICE" },
/* tr */ { IDMES_VICE_CONTRIBUTORS_TR, "VICE katýlýmcýlarý" },

/* en */ { IDMES_WHO_MADE_WHAT,    "Who made what?" },
/* da */ { IDMES_WHO_MADE_WHAT_DA, "Hvem har lavet hvad?" },
/* de */ { IDMES_WHO_MADE_WHAT_DE, "Wer hat was gemacht?" },
/* es */ { IDMES_WHO_MADE_WHAT_ES, "¿Quien hizo qué?" },
/* fr */ { IDMES_WHO_MADE_WHAT_FR, "Qui fait quoi?" },
/* hu */ { IDMES_WHO_MADE_WHAT_HU, "Ki mit csinált?" },
/* it */ { IDMES_WHO_MADE_WHAT_IT, "Chi ha fatto cosa?" },
/* ko */ { IDMES_WHO_MADE_WHAT_KO, "´©°¡ ¹«¾úÀ» ¸¸µé¾ú³ª¿ä?" },
/* nl */ { IDMES_WHO_MADE_WHAT_NL, "Wie heeft wat gemaakt?" },
/* pl */ { IDMES_WHO_MADE_WHAT_PL, "Kto co zrobi³?" },
/* ru */ { IDMES_WHO_MADE_WHAT_RU, "Who made what?" },
/* sv */ { IDMES_WHO_MADE_WHAT_SV, "Vem gjorde vad?" },
/* tr */ { IDMES_WHO_MADE_WHAT_TR, "Kim ne yaptý?" },

/* en */ { IDMES_VICE_DIST_NO_WARRANTY,    "VICE is distributed WITHOUT ANY WARRANTY!" },
/* da */ { IDMES_VICE_DIST_NO_WARRANTY_DA, "VICE distribueres UDEN NOGEN SOM HELST GARANTI!" },
/* de */ { IDMES_VICE_DIST_NO_WARRANTY_DE, "VICE ist ohne Garantie" },
/* es */ { IDMES_VICE_DIST_NO_WARRANTY_ES, "¡VICE se distribuye SIN NINGUNA GARANTÍA!" },
/* fr */ { IDMES_VICE_DIST_NO_WARRANTY_FR, "VICE est distribué SANS AUCUN GARANTIE!" },
/* hu */ { IDMES_VICE_DIST_NO_WARRANTY_HU, "A VICE-t garancia nélkül terjesztjük!" },
/* it */ { IDMES_VICE_DIST_NO_WARRANTY_IT, "VICE è distribuito SENZA NESSUNA GARANZIA!" },
/* ko */ { IDMES_VICE_DIST_NO_WARRANTY_KO, "VICE ´Â ¾î¶°ÇÑ ¿ö·±Æ¼ ¾øÀÌ Á¦°øµÇ°í ÀÖ½À´Ï´Ù!" },
/* nl */ { IDMES_VICE_DIST_NO_WARRANTY_NL, "VICE wordt verspreid ZONDER ENIGE GARANTIE!" },
/* pl */ { IDMES_VICE_DIST_NO_WARRANTY_PL, "VICE jest rozpowszechniany BEZ ¯ADNEJ GWARANCJI!" },
/* ru */ { IDMES_VICE_DIST_NO_WARRANTY_RU, "VICE àÐáßàÞáâàÐÝïÕâáï ±µ· ³°À°½Â¸¸!" },
/* sv */ { IDMES_VICE_DIST_NO_WARRANTY_SV, "VICE distribueras UTAN NÅGON SOM HELST GARANTI!" },
/* tr */ { IDMES_VICE_DIST_NO_WARRANTY_TR, "VICE HÝÇBÝR GARANTÝ VERÝLMEDEN daðýtýlmýþtýr!" },

/* en */ { IDMES_WHICH_COMMANDS_AVAILABLE,    "Which command line options are available?" },
/* da */ { IDMES_WHICH_COMMANDS_AVAILABLE_DA, "Hvilke kommandolinjeparametre er tilgængelige?" },
/* de */ { IDMES_WHICH_COMMANDS_AVAILABLE_DE, "Welche Kommandozeilen Parameter sind verfügbar?" },
/* es */ { IDMES_WHICH_COMMANDS_AVAILABLE_ES, "¿Qué opciones L.comandos están disponibles?" },
/* fr */ { IDMES_WHICH_COMMANDS_AVAILABLE_FR, "Quelles sont les lignes de commandes disponibles?" },
/* hu */ { IDMES_WHICH_COMMANDS_AVAILABLE_HU, "Milyen parancssori opciók lehetségesek?" },
/* it */ { IDMES_WHICH_COMMANDS_AVAILABLE_IT, "Quali parametri della riga di comando sono disponibili?" },
/* ko */ { IDMES_WHICH_COMMANDS_AVAILABLE_KO, "¾î¶² Ä¿¸Çµå ¶óÀÎ ¿É¼ÇÀÌ °¡´ÉÇÕ´Ï±î?" },
/* nl */ { IDMES_WHICH_COMMANDS_AVAILABLE_NL, "Welke opdrachtregelopties zijn beschikbaar?" },
/* pl */ { IDMES_WHICH_COMMANDS_AVAILABLE_PL, "Które opcje wiersza poleceñ s± dostêpne?" },
/* ru */ { IDMES_WHICH_COMMANDS_AVAILABLE_RU, "ºÐÚØÕ ÞßæØØ ÚÞÜÐÝÔÝÞÙ áâàÞÚØ Õáâì Ò ÝÐÛØçØØ?" },
/* sv */ { IDMES_WHICH_COMMANDS_AVAILABLE_SV, "Vilka kommandoradsflaggor är tillgängliga?" },
/* tr */ { IDMES_WHICH_COMMANDS_AVAILABLE_TR, "Hangi komut satýrý seçenekleri mevcut?" },

/* en */ { IDMES_WHICH_COMPILE_FEATURES_AVAILABLE,    "Which compile time features are available?" },
/* da */ { IDMES_WHICH_COMPILE_FEATURES_AVAILABLE_DA, "" },  /* fuzzy */
/* de */ { IDMES_WHICH_COMPILE_FEATURES_AVAILABLE_DE, "Welche kompilierten Funktionen sind verfügbar?" },
/* es */ { IDMES_WHICH_COMPILE_FEATURES_AVAILABLE_ES, "¿Qué opciones en tiempo de compilación están disponibles?" },
/* fr */ { IDMES_WHICH_COMPILE_FEATURES_AVAILABLE_FR, "" },  /* fuzzy */
/* hu */ { IDMES_WHICH_COMPILE_FEATURES_AVAILABLE_HU, "" },  /* fuzzy */
/* it */ { IDMES_WHICH_COMPILE_FEATURES_AVAILABLE_IT, "" },  /* fuzzy */
/* ko */ { IDMES_WHICH_COMPILE_FEATURES_AVAILABLE_KO, "" },  /* fuzzy */
/* nl */ { IDMES_WHICH_COMPILE_FEATURES_AVAILABLE_NL, "Welke speciale compileer opties zijn beschikbaar?" },
/* pl */ { IDMES_WHICH_COMPILE_FEATURES_AVAILABLE_PL, "Które opcje kompilacji s± dostêpne?" },
/* ru */ { IDMES_WHICH_COMPILE_FEATURES_AVAILABLE_RU, "" },  /* fuzzy */
/* sv */ { IDMES_WHICH_COMPILE_FEATURES_AVAILABLE_SV, "" },  /* fuzzy */
/* tr */ { IDMES_WHICH_COMPILE_FEATURES_AVAILABLE_TR, "" },  /* fuzzy */

/* en */ { IDMES_CANNOT_SAVE_SETTINGS,    "Cannot save settings." },
/* da */ { IDMES_CANNOT_SAVE_SETTINGS_DA, "Kunne ikke gemme indstillinger." },
/* de */ { IDMES_CANNOT_SAVE_SETTINGS_DE, "Fehler beim Speichern der Einstellungen." },
/* es */ { IDMES_CANNOT_SAVE_SETTINGS_ES, "No puedo grabar ajustes" },
/* fr */ { IDMES_CANNOT_SAVE_SETTINGS_FR, "Impossible d'enregistrer les paramètres." },
/* hu */ { IDMES_CANNOT_SAVE_SETTINGS_HU, "A beállítások mentése nem sikerült." },
/* it */ { IDMES_CANNOT_SAVE_SETTINGS_IT, "Non è possibile salvare le impostazioni." },
/* ko */ { IDMES_CANNOT_SAVE_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMES_CANNOT_SAVE_SETTINGS_NL, "Kan de instellingen niet opslaan." },
/* pl */ { IDMES_CANNOT_SAVE_SETTINGS_PL, "Nie mo¿na zapisaæ ustawieñ." },
/* ru */ { IDMES_CANNOT_SAVE_SETTINGS_RU, "½Õ ãÔÐÕâáï áÞåàÐÝØâì ÝÐáâàÞÙÚØ" },
/* sv */ { IDMES_CANNOT_SAVE_SETTINGS_SV, "Kan inte spara inställningar." },
/* tr */ { IDMES_CANNOT_SAVE_SETTINGS_TR, "Ayarlar kaydedilemedi." },

/* en */ { IDMES_CANNOT_LOAD_SETTINGS,    "Cannot load settings." },
/* da */ { IDMES_CANNOT_LOAD_SETTINGS_DA, "Kunne ikke indlæse indstillinger." },
/* de */ { IDMES_CANNOT_LOAD_SETTINGS_DE, "Fehler beim Laden der Einstellungen." },
/* es */ { IDMES_CANNOT_LOAD_SETTINGS_ES, "No puedo cargar ajustes." },
/* fr */ { IDMES_CANNOT_LOAD_SETTINGS_FR, "Impossible de charger les paramètres." },
/* hu */ { IDMES_CANNOT_LOAD_SETTINGS_HU, "A beállítások betöltése nem sikerült." },
/* it */ { IDMES_CANNOT_LOAD_SETTINGS_IT, "Non è possibile caricare le impostazioni." },
/* ko */ { IDMES_CANNOT_LOAD_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDMES_CANNOT_LOAD_SETTINGS_NL, "Kan de instellingen niet laden." },
/* pl */ { IDMES_CANNOT_LOAD_SETTINGS_PL, "Nie mo¿na wczytaæ ustawieñ." },
/* ru */ { IDMES_CANNOT_LOAD_SETTINGS_RU, "½Õ ãÔÐÕâáï ×ÐÓàã×Øâì ÝÐáâàÞÙÚØ." },
/* sv */ { IDMES_CANNOT_LOAD_SETTINGS_SV, "Kan inte läsa inställningar." },
/* tr */ { IDMES_CANNOT_LOAD_SETTINGS_TR, "Ayarlar yüklenemedi." },

/* en */ { IDMES_VICE_ERROR,    "VICE Error!" },
/* da */ { IDMES_VICE_ERROR_DA, "VICE-fejl!" },
/* de */ { IDMES_VICE_ERROR_DE, "VICE Fehler!" },
/* es */ { IDMES_VICE_ERROR_ES, "¡Error VICE!" },
/* fr */ { IDMES_VICE_ERROR_FR, "Erreur VICE!" },
/* hu */ { IDMES_VICE_ERROR_HU, "VICE hiba!" },
/* it */ { IDMES_VICE_ERROR_IT, "Errore di VICE!" },
/* ko */ { IDMES_VICE_ERROR_KO, "VICE ¿À·ù!" },
/* nl */ { IDMES_VICE_ERROR_NL, "VICE Fout!" },
/* pl */ { IDMES_VICE_ERROR_PL, "B³±d VICE!" },
/* ru */ { IDMES_VICE_ERROR_RU, "¾èØÑÚÐ VICE!" },
/* sv */ { IDMES_VICE_ERROR_SV, "VICE-fel!" },
/* tr */ { IDMES_VICE_ERROR_TR, "VICE Hatasý!" },

/* en */ { IDMES_DEVICE_NOT_GAMEPAD,    "The device on this port is not a gamepad" },
/* da */ { IDMES_DEVICE_NOT_GAMEPAD_DA, "Enheden på denne port er ikke en joypad" },
/* de */ { IDMES_DEVICE_NOT_GAMEPAD_DE, "Das Gerät in diesem Port ist kein Gamepad" },
/* es */ { IDMES_DEVICE_NOT_GAMEPAD_ES, "El periférico en este puerto no es un gamepad" },
/* fr */ { IDMES_DEVICE_NOT_GAMEPAD_FR, "Le périphérique sur ce port n’est pas une manette" },
/* hu */ { IDMES_DEVICE_NOT_GAMEPAD_HU, "Az eszköz ezen a porton nem gamepad" },
/* it */ { IDMES_DEVICE_NOT_GAMEPAD_IT, "Il dispositivo su questa porta non è un gamepad" },
/* ko */ { IDMES_DEVICE_NOT_GAMEPAD_KO, "" },  /* fuzzy */
/* nl */ { IDMES_DEVICE_NOT_GAMEPAD_NL, "Het apparaat op deze poort is geen gamepad" },
/* pl */ { IDMES_DEVICE_NOT_GAMEPAD_PL, "Urz±dzenie w tym porcie nie jest padem" },
/* ru */ { IDMES_DEVICE_NOT_GAMEPAD_RU, "The device on this port is not a gamepad" },
/* sv */ { IDMES_DEVICE_NOT_GAMEPAD_SV, "Enheten på denna port är inte en styrplatta" },
/* tr */ { IDMES_DEVICE_NOT_GAMEPAD_TR, "Bu port üzerindeki aygýt gamepad deðil" },

/* en */ { IDMES_NOT_MAPPED_TO_AMIGA_PORT,    "The port is not mapped to an Amiga port" },
/* da */ { IDMES_NOT_MAPPED_TO_AMIGA_PORT_DA, "Denne port er ikke koblet til en Amigaport" },
/* de */ { IDMES_NOT_MAPPED_TO_AMIGA_PORT_DE, "Dieses Port ist keinem Amiga Port zugeordnet" },
/* es */ { IDMES_NOT_MAPPED_TO_AMIGA_PORT_ES, "El puerto no está mapeado como puerto Amiga" },
/* fr */ { IDMES_NOT_MAPPED_TO_AMIGA_PORT_FR, "Le port n’est pas mappé sur un port Amiga" },
/* hu */ { IDMES_NOT_MAPPED_TO_AMIGA_PORT_HU, "A port nincs leképezve egy Amiga portra" },
/* it */ { IDMES_NOT_MAPPED_TO_AMIGA_PORT_IT, "Questa porta non è mappata su una porta dell'Amiga" },
/* ko */ { IDMES_NOT_MAPPED_TO_AMIGA_PORT_KO, "" },  /* fuzzy */
/* nl */ { IDMES_NOT_MAPPED_TO_AMIGA_PORT_NL, "De poort is niet verbonden met een Amigapoort" },
/* pl */ { IDMES_NOT_MAPPED_TO_AMIGA_PORT_PL, "Port nie jest zmapowany do portu Amigi" },
/* ru */ { IDMES_NOT_MAPPED_TO_AMIGA_PORT_RU, "The port is not mapped to an Amiga port" },
/* sv */ { IDMES_NOT_MAPPED_TO_AMIGA_PORT_SV, "Porten är inte kopplad till en Amigaport" },
/* tr */ { IDMES_NOT_MAPPED_TO_AMIGA_PORT_TR, "Port bir Amiga portuyla eþleþtirilemedi" },

/* en */ { IDMES_CANNOT_AUTOSTART_FILE,    "Cannot autostart specified file." },
/* da */ { IDMES_CANNOT_AUTOSTART_FILE_DA, "Kan ikke autostarte valgt fil" },
/* de */ { IDMES_CANNOT_AUTOSTART_FILE_DE, "Autostart von angeforderter Datei fehlgeschlagen." },
/* es */ { IDMES_CANNOT_AUTOSTART_FILE_ES, "No puedo autoarrancar el fichero especificado." },
/* fr */ { IDMES_CANNOT_AUTOSTART_FILE_FR, "Impossible d'autocharger le fichier spécifié" },
/* hu */ { IDMES_CANNOT_AUTOSTART_FILE_HU, "Nem lehet automatikusan elindítani a megadott fájlt." },
/* it */ { IDMES_CANNOT_AUTOSTART_FILE_IT, "Non è possibile avviare automaticamente il file specificato." },
/* ko */ { IDMES_CANNOT_AUTOSTART_FILE_KO, "" },  /* fuzzy */
/* nl */ { IDMES_CANNOT_AUTOSTART_FILE_NL, "Kan opgegeven bestand niet autostarten." },
/* pl */ { IDMES_CANNOT_AUTOSTART_FILE_PL, "Nie mo¿na automatycznie wystartowaæ z danego pliku." },
/* ru */ { IDMES_CANNOT_AUTOSTART_FILE_RU, "Cannot autostart specified file." },
/* sv */ { IDMES_CANNOT_AUTOSTART_FILE_SV, "Kan inte autostarta angiven fil." },
/* tr */ { IDMES_CANNOT_AUTOSTART_FILE_TR, "Belirtilen dosya otomatik baþlatýlamadý." },

/* en */ { IDMES_CANNOT_ATTACH_FILE,    "Cannot attach specified file" },
/* da */ { IDMES_CANNOT_ATTACH_FILE_DA, "Kan ikke tilslutte den valgte fil" },
/* de */ { IDMES_CANNOT_ATTACH_FILE_DE, "Kann Imagedatei nicht einlegen" },
/* es */ { IDMES_CANNOT_ATTACH_FILE_ES, "No puedo insertar el fichero especificado" },
/* fr */ { IDMES_CANNOT_ATTACH_FILE_FR, "Impossible d'attacher le fichier spécifié" },
/* hu */ { IDMES_CANNOT_ATTACH_FILE_HU, "Nem lehet csatolni a megadott fájlt" },
/* it */ { IDMES_CANNOT_ATTACH_FILE_IT, "Non è possibile selezionare il file specificato" },
/* ko */ { IDMES_CANNOT_ATTACH_FILE_KO, "" },  /* fuzzy */
/* nl */ { IDMES_CANNOT_ATTACH_FILE_NL, "Kan het opgegeven bestand niet koppelen" },
/* pl */ { IDMES_CANNOT_ATTACH_FILE_PL, "Nie mo¿na zamontowaæ danego pliku" },
/* ru */ { IDMES_CANNOT_ATTACH_FILE_RU, "½Õ ãÔÐÕâáï ßàØÚàÕßØâì ãÚÐ×ÐÝÝëÙ äÐÙÛ" },
/* sv */ { IDMES_CANNOT_ATTACH_FILE_SV, "Kan inte ansluta filen" },
/* tr */ { IDMES_CANNOT_ATTACH_FILE_TR, "Belirtilen dosya yerleþtirilemedi" },

/* en */ { IDMES_INVALID_CART,    "Invalid cartridge image" },
/* da */ { IDMES_INVALID_CART_DA, "Ugyldigt cartridge-image" },
/* de */ { IDMES_INVALID_CART_DE, "Ungültiges Erweiterungsmodul Image" },
/* es */ { IDMES_INVALID_CART_ES, "Imagen de cartucho incorrecta" },
/* fr */ { IDMES_INVALID_CART_FR, "Image de cartouche invalide" },
/* hu */ { IDMES_INVALID_CART_HU, "Érvénytelen cartridge képmás" },
/* it */ { IDMES_INVALID_CART_IT, "Immagine cartuccia non valida" },
/* ko */ { IDMES_INVALID_CART_KO, "Ä«Æ®¸®Áö ÀÌ¹ÌÁö »ç¿ëºÒ°¡" },
/* nl */ { IDMES_INVALID_CART_NL, "Ongeldig cartridge bestand" },
/* pl */ { IDMES_INVALID_CART_PL, "Nieprawid³owy obraz kartrid¿a" },
/* ru */ { IDMES_INVALID_CART_RU, "½ÕÒÕàÝëÙ ÞÑàÐ× ÚÐàâàØÔÖÐ" },
/* sv */ { IDMES_INVALID_CART_SV, "Ogiltig insticksmodulfil" },
/* tr */ { IDMES_INVALID_CART_TR, "Geçersiz kartuþ imajý" },

/* en */ { IDMES_BAD_CART_CONFIG_IN_UI,    "Bad cartridge config in UI!" },
/* da */ { IDMES_BAD_CART_CONFIG_IN_UI_DA, "Ugyldig cartridgekonfiguration i brugergrænseflade!" },
/* de */ { IDMES_BAD_CART_CONFIG_IN_UI_DE, "Fehlerhafte Modul Konfiguration im UI!" },
/* es */ { IDMES_BAD_CART_CONFIG_IN_UI_ES, "¡Configuración errónea del cartucho en UI!" },
/* fr */ { IDMES_BAD_CART_CONFIG_IN_UI_FR, "Mauvaise configuration de la cartouche dans l'interface utilisateur!" },
/* hu */ { IDMES_BAD_CART_CONFIG_IN_UI_HU, "Rossz cartridge konfiguráció!" },
/* it */ { IDMES_BAD_CART_CONFIG_IN_UI_IT, "Configurazione della cartuccia errata nell'IU!" },
/* ko */ { IDMES_BAD_CART_CONFIG_IN_UI_KO, "UI ¿¡ Àß¸øµÈ Ä«Æ®¸®Áö°¡ ÀÖ½À´Ï´Ù" },
/* nl */ { IDMES_BAD_CART_CONFIG_IN_UI_NL, "Verkeerde cartridge configuratie in UI!" },
/* pl */ { IDMES_BAD_CART_CONFIG_IN_UI_PL, "Z³a konfiguracja kartrid¿a w UI!" },
/* ru */ { IDMES_BAD_CART_CONFIG_IN_UI_RU, "Bad cartridge config in UI!" },
/* sv */ { IDMES_BAD_CART_CONFIG_IN_UI_SV, "Felaktiga insticksmodulsinställningar i UI!" },
/* tr */ { IDMES_BAD_CART_CONFIG_IN_UI_TR, "UI'de kötü kartuþ konfigürasyonu!" },

/* en */ { IDMES_INVALID_CART_IMAGE,    "Invalid cartridge image" },
/* da */ { IDMES_INVALID_CART_IMAGE_DA, "Ugyldigt cartridge-image" },
/* de */ { IDMES_INVALID_CART_IMAGE_DE, "Ungültiges Erweiterungsmodul Image" },
/* es */ { IDMES_INVALID_CART_IMAGE_ES, "Imagen de cartucho incorrecta" },
/* fr */ { IDMES_INVALID_CART_IMAGE_FR, "Image de cartouche invalide" },
/* hu */ { IDMES_INVALID_CART_IMAGE_HU, "Érvénytelen cartridge képmás" },
/* it */ { IDMES_INVALID_CART_IMAGE_IT, "Immagine cartuccia non valida" },
/* ko */ { IDMES_INVALID_CART_IMAGE_KO, "Ä«Æ®¸®Áö ÀÌ¹ÌÁö »ç¿ëºÒ°¡" },
/* nl */ { IDMES_INVALID_CART_IMAGE_NL, "Ongeldig cartridge bestand" },
/* pl */ { IDMES_INVALID_CART_IMAGE_PL, "Nieprawid³owy obraz kartrid¿a" },
/* ru */ { IDMES_INVALID_CART_IMAGE_RU, "½ÕÒÕàÝëÙ ÞÑàÐ× ÚÐàâàØÔÖÐ" },
/* sv */ { IDMES_INVALID_CART_IMAGE_SV, "Ogiltig insticksmodulfil" },
/* tr */ { IDMES_INVALID_CART_IMAGE_TR, "Geçersiz kartuþ imajý" },

/* en */ { IDMES_CANNOT_CREATE_IMAGE,    "Cannot create image file!" },
/* da */ { IDMES_CANNOT_CREATE_IMAGE_DA, "Kunne ikke oprette image-fil!" },
/* de */ { IDMES_CANNOT_CREATE_IMAGE_DE, "Kann Imagedatei nicht erzeugen!" },
/* es */ { IDMES_CANNOT_CREATE_IMAGE_ES, "¡No puedo crear fichero imagen!" },
/* fr */ { IDMES_CANNOT_CREATE_IMAGE_FR, "Impossible de créer le fichier image!" },
/* hu */ { IDMES_CANNOT_CREATE_IMAGE_HU, "Nem sikerült a képmás fájlt létrehozni!" },
/* it */ { IDMES_CANNOT_CREATE_IMAGE_IT, "Non è possibile creare il file immagine!" },
/* ko */ { IDMES_CANNOT_CREATE_IMAGE_KO, "ÀÌ¹ÌÁö ÆÄÀÏÀ» »ý¼ºÇÒ¼ö ¾ø½À´Ï´Ù!" },
/* nl */ { IDMES_CANNOT_CREATE_IMAGE_NL, "Kan bestand niet aanmaken!" },
/* pl */ { IDMES_CANNOT_CREATE_IMAGE_PL, "Nie mo¿na utworzyæ pliku obrazu!" },
/* ru */ { IDMES_CANNOT_CREATE_IMAGE_RU, "½Õ ãÔÐÕâáï áÞ×ÔÐâì äÐÙÛ ÞÑàÐ×Ð!" },
/* sv */ { IDMES_CANNOT_CREATE_IMAGE_SV, "Kan inte skapa avbildningsfil!" },
/* tr */ { IDMES_CANNOT_CREATE_IMAGE_TR, "Imaj dosyasý yaratýlamadý!" },

/* en */ { IDMES_ERROR_STARTING_SERVER,    "An error occured starting the server." },
/* da */ { IDMES_ERROR_STARTING_SERVER_DA, "Fejl under start af serveren." },
/* de */ { IDMES_ERROR_STARTING_SERVER_DE, "Fehler beim Starten des Netplay Servers." },
/* es */ { IDMES_ERROR_STARTING_SERVER_ES, "Ha ocurrido error al iniciar el servidor" },
/* fr */ { IDMES_ERROR_STARTING_SERVER_FR, "Il y a eu une erreur au démarrage du serveur." },
/* hu */ { IDMES_ERROR_STARTING_SERVER_HU, "Hiba történt a játék kiszolgáló indításakor." },
/* it */ { IDMES_ERROR_STARTING_SERVER_IT, "Si è verificato un errore all'avvio del server." },
/* ko */ { IDMES_ERROR_STARTING_SERVER_KO, "¼­¹ö¸¦ ½ÃÀÛÇÏ´øÁß ¿¡·¯°¡ ¹ß»ýÇß½À´Ï´Ù" },
/* nl */ { IDMES_ERROR_STARTING_SERVER_NL, "Een fout is opgetreden bij het starten van de server." },
/* pl */ { IDMES_ERROR_STARTING_SERVER_PL, "Wyst±pi³ b³±d poczas uruchamiania serwera." },
/* ru */ { IDMES_ERROR_STARTING_SERVER_RU, "An error occured starting the server." },
/* sv */ { IDMES_ERROR_STARTING_SERVER_SV, "Fel vid start av servern." },
/* tr */ { IDMES_ERROR_STARTING_SERVER_TR, "Sunucu baþlatýlýrken bir hata oluþtu." },

/* en */ { IDMES_ERROR_CONNECTING_CLIENT,    "An error occured connecting the client." },
/* da */ { IDMES_ERROR_CONNECTING_CLIENT_DA, "Fejl under tilslutning af klienten." },
/* de */ { IDMES_ERROR_CONNECTING_CLIENT_DE, "Fehler beim Verbindungsaufbau zum Client" },
/* es */ { IDMES_ERROR_CONNECTING_CLIENT_ES, "Ha ocurrido error al conectar el cliente" },
/* fr */ { IDMES_ERROR_CONNECTING_CLIENT_FR, "Il y a eu une erreur de connexion avec le client." },
/* hu */ { IDMES_ERROR_CONNECTING_CLIENT_HU, "Hiba történt a kapcsolódáskor." },
/* it */ { IDMES_ERROR_CONNECTING_CLIENT_IT, "Si è verificato un errore durante il collegamento col client." },
/* ko */ { IDMES_ERROR_CONNECTING_CLIENT_KO, "Å©¶óÀÌ¾ðÆ®¿¡ Á¢¼ÓÁß ¿¡·¯°¡ ¹ß»ýÇß½À´Ï´Ù" },
/* nl */ { IDMES_ERROR_CONNECTING_CLIENT_NL, "Een fout is opgetreden bij het verbinden met de client." },
/* pl */ { IDMES_ERROR_CONNECTING_CLIENT_PL, "Wydarzy³ siê b³±d podczas ³±czenia z klientem." },
/* ru */ { IDMES_ERROR_CONNECTING_CLIENT_RU, "An error occured connecting the client." },
/* sv */ { IDMES_ERROR_CONNECTING_CLIENT_SV, "Fel vid anslutning til klienten." },
/* tr */ { IDMES_ERROR_CONNECTING_CLIENT_TR, "Ýstemciye baðlanýrken bir hata oluþtu." },

/* en */ { IDMES_INVALID_PORT_NUMBER,    "Invalid port number" },
/* da */ { IDMES_INVALID_PORT_NUMBER_DA, "Ugyldigt portnummer" },
/* de */ { IDMES_INVALID_PORT_NUMBER_DE, "Ungültige Port Nummer" },
/* es */ { IDMES_INVALID_PORT_NUMBER_ES, "Número de puerto incorrecto" },
/* fr */ { IDMES_INVALID_PORT_NUMBER_FR, "Numéro de port invalide" },
/* hu */ { IDMES_INVALID_PORT_NUMBER_HU, "Érvénytelen port szám" },
/* it */ { IDMES_INVALID_PORT_NUMBER_IT, "Numero di porta non valido" },
/* ko */ { IDMES_INVALID_PORT_NUMBER_KO, "»ç¿ëºÒ°¡´ÉÇÑ Æ÷Æ® ³Ñ¹ö" },
/* nl */ { IDMES_INVALID_PORT_NUMBER_NL, "Ongeldig poort nummer" },
/* pl */ { IDMES_INVALID_PORT_NUMBER_PL, "B³êdny numer portu" },
/* ru */ { IDMES_INVALID_PORT_NUMBER_RU, "½ÕÒÕàÝëÙ ßÞàâ" },
/* sv */ { IDMES_INVALID_PORT_NUMBER_SV, "Ogiltigt portnummer" },
/* tr */ { IDMES_INVALID_PORT_NUMBER_TR, "Geçersiz port numarasý" },

/* en */ { IDMES_THIS_MACHINE_NO_SID,    "This machine may not have a SID" },
/* da */ { IDMES_THIS_MACHINE_NO_SID_DA, "Denne maskine kan ikke have en SID" },
/* de */ { IDMES_THIS_MACHINE_NO_SID_DE, "Diese Maschine hat möglicherweise keinen SID" },
/* es */ { IDMES_THIS_MACHINE_NO_SID_ES, "Esta máquina dispone de SID" },
/* fr */ { IDMES_THIS_MACHINE_NO_SID_FR, "Cette machine n'a peut-être pas de SID" },
/* hu */ { IDMES_THIS_MACHINE_NO_SID_HU, "Ennek a gépnek nem lehet SIDje" },
/* it */ { IDMES_THIS_MACHINE_NO_SID_IT, "Questo computer potrebbe non avere un SID" },
/* ko */ { IDMES_THIS_MACHINE_NO_SID_KO, "" },  /* fuzzy */
/* nl */ { IDMES_THIS_MACHINE_NO_SID_NL, "Deze machine heeft mogelijk geen SID" },
/* pl */ { IDMES_THIS_MACHINE_NO_SID_PL, "W tej maszynie mo¿e nie byæ SID" },
/* ru */ { IDMES_THIS_MACHINE_NO_SID_RU, "This machine may not have a SID" },
/* sv */ { IDMES_THIS_MACHINE_NO_SID_SV, "Denna maskin kan inte ha en SID" },
/* tr */ { IDMES_THIS_MACHINE_NO_SID_TR, "Bu makinede SID olmayabilir" },

/* en */ { IDMES_SCREENSHOT_SAVE_S_FAILED,    "Screenshot save of %s failed" },
/* da */ { IDMES_SCREENSHOT_SAVE_S_FAILED_DA, "Kunne ikke gemme screenshot %s" },
/* de */ { IDMES_SCREENSHOT_SAVE_S_FAILED_DE, "Screenshot Speicherung von %s ist fehlgeschlagen" },
/* es */ { IDMES_SCREENSHOT_SAVE_S_FAILED_ES, "Error en la grabación Captura de pantalla %s" },
/* fr */ { IDMES_SCREENSHOT_SAVE_S_FAILED_FR, "L'enregistrement de la capture d'écran %s à échoué" },
/* hu */ { IDMES_SCREENSHOT_SAVE_S_FAILED_HU, "Fénykép fájl mentése %s fájlba nem sikerült" },
/* it */ { IDMES_SCREENSHOT_SAVE_S_FAILED_IT, "Il salvataggio della schermata %s è fallito" },
/* ko */ { IDMES_SCREENSHOT_SAVE_S_FAILED_KO, "" },  /* fuzzy */
/* nl */ { IDMES_SCREENSHOT_SAVE_S_FAILED_NL, "Opslaan schermafdrukbestand %s is mislukt" },
/* pl */ { IDMES_SCREENSHOT_SAVE_S_FAILED_PL, "Zapis zrzutu ekranu %s nie powiód³ siê" },
/* ru */ { IDMES_SCREENSHOT_SAVE_S_FAILED_RU, "Screenshot save of %s failed" },
/* sv */ { IDMES_SCREENSHOT_SAVE_S_FAILED_SV, "Kunde inte spara skärmdump %s" },
/* tr */ { IDMES_SCREENSHOT_SAVE_S_FAILED_TR, "%s ekran görüntüsü kaydedilemedi" },

/* en */ { IDMES_SCREENSHOT_S_SAVED,    "Screenshot %s saved." },
/* da */ { IDMES_SCREENSHOT_S_SAVED_DA, "Screenshot %s gemt." },
/* de */ { IDMES_SCREENSHOT_S_SAVED_DE, "Screenshots %s gespeichert." },
/* es */ { IDMES_SCREENSHOT_S_SAVED_ES, "Captura de pantalla %s grabada." },
/* fr */ { IDMES_SCREENSHOT_S_SAVED_FR, "Capture d'écran %s enregistrée." },
/* hu */ { IDMES_SCREENSHOT_S_SAVED_HU, "Fénykép %s fájlba elmentve." },
/* it */ { IDMES_SCREENSHOT_S_SAVED_IT, "Schermata %s salvata." },
/* ko */ { IDMES_SCREENSHOT_S_SAVED_KO, "" },  /* fuzzy */
/* nl */ { IDMES_SCREENSHOT_S_SAVED_NL, "Schermafdruk %s opgeslagen." },
/* pl */ { IDMES_SCREENSHOT_S_SAVED_PL, "Zapisano zrzut ekranu %s." },
/* ru */ { IDMES_SCREENSHOT_S_SAVED_RU, "Screenshot %s saved." },
/* sv */ { IDMES_SCREENSHOT_S_SAVED_SV, "Skärmdump %s sparad." },
/* tr */ { IDMES_SCREENSHOT_S_SAVED_TR, "%s ekran görüntüsü kaydedildi." },

/* en */ { IDS_PRESS_KEY_BUTTON,    "Press desired key/button, move stick or press ESC for no key." },
/* da */ { IDS_PRESS_KEY_BUTTON_DA, "Tryk ønsket tast, bevæg joystick eller tryk ESC for ingen binding." },
/* de */ { IDS_PRESS_KEY_BUTTON_DE, "Gewünschte Taste/Knopf drücken, Knüppel bewegen oder ESC für keine Taste drücken." },
/* es */ { IDS_PRESS_KEY_BUTTON_ES, "Pulsa la tecla deseada/botón, mueve stick o pulsa ESC para ninguna tecla." },
/* fr */ { IDS_PRESS_KEY_BUTTON_FR, "Appuyez sur la touche/bouton desire(e), déplacez le stick ou appuyez sur ESC pour effacer cette touche." },
/* hu */ { IDS_PRESS_KEY_BUTTON_HU, "Nyomja meg a választott gombot, mozdítsa a botkormányt, vagy nyomjon ESC-et üreshez" },
/* it */ { IDS_PRESS_KEY_BUTTON_IT, "Premi il tasto/bottone desiderato, muovi la leva o premi ESC per non selezionare alcun tasto." },
/* ko */ { IDS_PRESS_KEY_BUTTON_KO, "" },  /* fuzzy */
/* nl */ { IDS_PRESS_KEY_BUTTON_NL, "Druk de toets/knop, beweeg de joystick of druk op ESC voor geen toets." },
/* pl */ { IDS_PRESS_KEY_BUTTON_PL, "Wci¶nij po¿±dany klawisz, rusz dr±¿kiem, albo wci¶nij ESC by nie ustawiaæ klawisza." },
/* ru */ { IDS_PRESS_KEY_BUTTON_RU, "Press desired key/button, move stick or press ESC for no key." },
/* sv */ { IDS_PRESS_KEY_BUTTON_SV, "Tryck önskad tangent/knapp, flytta styrspaken eller tryck ESC för att inte koppla någon tangent." },
/* tr */ { IDS_PRESS_KEY_BUTTON_TR, "Ýstenilen tuþ/düðmeye basýn, çubuðu hareket ettirin veya tuþ seçmemek için ESC'e basýn." },

/* en */ { IDS_SAVE,    "Save" },
/* da */ { IDS_SAVE_DA, "Gem" },
/* de */ { IDS_SAVE_DE, "Speichern" },
/* es */ { IDS_SAVE_ES, "Grabación" },
/* fr */ { IDS_SAVE_FR, "Enregistrer" },
/* hu */ { IDS_SAVE_HU, "Mentés" },
/* it */ { IDS_SAVE_IT, "Salva" },
/* ko */ { IDS_SAVE_KO, "ÀúÀå" },
/* nl */ { IDS_SAVE_NL, "Opslaan" },
/* pl */ { IDS_SAVE_PL, "Zapisz" },
/* ru */ { IDS_SAVE_RU, "ÁÞåàÐÝØâì" },
/* sv */ { IDS_SAVE_SV, "Spara" },
/* tr */ { IDS_SAVE_TR, "Kaydet" },

/* en */ { IDS_BROWSE,    "Browse" },
/* da */ { IDS_BROWSE_DA, "Gennemse" },
/* de */ { IDS_BROWSE_DE, "Stöbern" },
/* es */ { IDS_BROWSE_ES, "Explora" },
/* fr */ { IDS_BROWSE_FR, "Parcourir" },
/* hu */ { IDS_BROWSE_HU, "Tallóz" },
/* it */ { IDS_BROWSE_IT, "Sfoglia" },
/* ko */ { IDS_BROWSE_KO, "ºê¶ó¿ì½º" },
/* nl */ { IDS_BROWSE_NL, "Bladeren" },
/* pl */ { IDS_BROWSE_PL, "Przegl±daj" },
/* ru */ { IDS_BROWSE_RU, "¾Ñ×Þà" },
/* sv */ { IDS_BROWSE_SV, "Bläddra" },
/* tr */ { IDS_BROWSE_TR, "Göz at" },

/* en */ { IDS_CURRENT_MODE,    "Current mode" },
/* da */ { IDS_CURRENT_MODE_DA, "Nuværende tilstand" },
/* de */ { IDS_CURRENT_MODE_DE, "Aktueller Modus" },
/* es */ { IDS_CURRENT_MODE_ES, "Modo actual" },
/* fr */ { IDS_CURRENT_MODE_FR, "Mode actuel" },
/* hu */ { IDS_CURRENT_MODE_HU, "Jelenlegi mód" },
/* it */ { IDS_CURRENT_MODE_IT, "Modalità attuale" },
/* ko */ { IDS_CURRENT_MODE_KO, "ÇöÁ¦ ¸ðµå" },
/* nl */ { IDS_CURRENT_MODE_NL, "Huidige modus" },
/* pl */ { IDS_CURRENT_MODE_PL, "Obecny tryb" },
/* ru */ { IDS_CURRENT_MODE_RU, "ÂÕÚãéØÙ àÕÖØÜ" },
/* sv */ { IDS_CURRENT_MODE_SV, "Nuvarande läge" },
/* tr */ { IDS_CURRENT_MODE_TR, "Geçerli mod" },

/* en */ { IDS_TCP_PORT,    "TCP port" },
/* da */ { IDS_TCP_PORT_DA, "TCP-port" },
/* de */ { IDS_TCP_PORT_DE, "TCP Port: " },
/* es */ { IDS_TCP_PORT_ES, "Puerto TCP" },
/* fr */ { IDS_TCP_PORT_FR, "Port TCP" },
/* hu */ { IDS_TCP_PORT_HU, "TCP port" },
/* it */ { IDS_TCP_PORT_IT, "Porta TCP" },
/* ko */ { IDS_TCP_PORT_KO, "TCP Æ÷Æ®" },
/* nl */ { IDS_TCP_PORT_NL, "TCP poort" },
/* pl */ { IDS_TCP_PORT_PL, "Port TCP" },
/* ru */ { IDS_TCP_PORT_RU, "TCP ßÞàâ" },
/* sv */ { IDS_TCP_PORT_SV, "TCP-port" },
/* tr */ { IDS_TCP_PORT_TR, "TCP portu" },

/* en */ { IDS_START_SERVER,    "Start server" },
/* da */ { IDS_START_SERVER_DA, "Start server" },
/* de */ { IDS_START_SERVER_DE, "Server starten" },
/* es */ { IDS_START_SERVER_ES, "Iniciar servidor" },
/* fr */ { IDS_START_SERVER_FR, "Démarrer le serveur" },
/* hu */ { IDS_START_SERVER_HU, "Kiszolgáló elindítása" },
/* it */ { IDS_START_SERVER_IT, "Avvia server" },
/* ko */ { IDS_START_SERVER_KO, "¼­¹ö ½ÃÀÛ" },
/* nl */ { IDS_START_SERVER_NL, "Start server" },
/* pl */ { IDS_START_SERVER_PL, "Uruchom serwer" },
/* ru */ { IDS_START_SERVER_RU, "Start server" },
/* sv */ { IDS_START_SERVER_SV, "Starta server" },
/* tr */ { IDS_START_SERVER_TR, "Sunucuyu baþlat" },

/* en */ { IDS_SERVER_BIND,    "Server Bind" },
/* da */ { IDS_SERVER_BIND_DA, "Serverbinding" },
/* de */ { IDS_SERVER_BIND_DE, "Server Bind" },
/* es */ { IDS_SERVER_BIND_ES, "Server Bind" },
/* fr */ { IDS_SERVER_BIND_FR, "Lien Serveur" },
/* hu */ { IDS_SERVER_BIND_HU, "Szerver cím+port" },
/* it */ { IDS_SERVER_BIND_IT, "Server Bind" },
/* ko */ { IDS_SERVER_BIND_KO, "¼­¹ö ¹ÙÀÎµå" },
/* nl */ { IDS_SERVER_BIND_NL, "Server binding" },
/* pl */ { IDS_SERVER_BIND_PL, "Po³±czenie serwera" },
/* ru */ { IDS_SERVER_BIND_RU, "Server Bind" },
/* sv */ { IDS_SERVER_BIND_SV, "Serverbindning" },
/* tr */ { IDS_SERVER_BIND_TR, "Sunucu Baðlantýsý" },

/* en */ { IDS_CONNECT_TO,    "Connect to" },
/* da */ { IDS_CONNECT_TO_DA, "Tilslut til" },
/* de */ { IDS_CONNECT_TO_DE, "Verbinden mit" },
/* es */ { IDS_CONNECT_TO_ES, "Conectar a" },
/* fr */ { IDS_CONNECT_TO_FR, "Se connecter à" },
/* hu */ { IDS_CONNECT_TO_HU, "Kapcsolódás ehhez:" },
/* it */ { IDS_CONNECT_TO_IT, "Connetti a" },
/* ko */ { IDS_CONNECT_TO_KO, "¿¬°á" },
/* nl */ { IDS_CONNECT_TO_NL, "Maak verbinding met" },
/* pl */ { IDS_CONNECT_TO_PL, "Po³±cz z" },
/* ru */ { IDS_CONNECT_TO_RU, "¿ÞÔÚÛîçØâìáï Ú" },
/* sv */ { IDS_CONNECT_TO_SV, "Anslut till" },
/* tr */ { IDS_CONNECT_TO_TR, "Baðlanýlan" },

/* en */ { IDS_DISCONNECT,    "Disconnect" },
/* da */ { IDS_DISCONNECT_DA, "Afbryd forbindelse" },
/* de */ { IDS_DISCONNECT_DE, "Verbindung Trennen" },
/* es */ { IDS_DISCONNECT_ES, "Desconectar" },
/* fr */ { IDS_DISCONNECT_FR, "Se déconnecter du serveur" },
/* hu */ { IDS_DISCONNECT_HU, "Szétkapcsolódás" },
/* it */ { IDS_DISCONNECT_IT, "Disconnetti" },
/* ko */ { IDS_DISCONNECT_KO, "¿¬°áÀ» ²÷´Ù" },
/* nl */ { IDS_DISCONNECT_NL, "Verbreek verbinding" },
/* pl */ { IDS_DISCONNECT_PL, "Roz³±cz" },
/* ru */ { IDS_DISCONNECT_RU, "Disconnect" },
/* sv */ { IDS_DISCONNECT_SV, "Koppla från" },
/* tr */ { IDS_DISCONNECT_TR, "Baðlantýyý kes" },

/* en */ { IDS_IDLE,    "Idle" },
/* da */ { IDS_IDLE_DA, "Venter" },
/* de */ { IDS_IDLE_DE, "Idle" },
/* es */ { IDS_IDLE_ES, "Inactivo" },
/* fr */ { IDS_IDLE_FR, "Inactif" },
/* hu */ { IDS_IDLE_HU, "Tétlen" },
/* it */ { IDS_IDLE_IT, "Inattivo" },
/* ko */ { IDS_IDLE_KO, "°¡µ¿ÇÏÁö¾Ê´Â" },
/* nl */ { IDS_IDLE_NL, "Niet actief" },
/* pl */ { IDS_IDLE_PL, "Bezczynny" },
/* ru */ { IDS_IDLE_RU, "Idle" },
/* sv */ { IDS_IDLE_SV, "Väntar" },
/* tr */ { IDS_IDLE_TR, "Boþa vakit geçirme" },

/* en */ { IDS_SERVER_LISTENING,    "Server listening" },
/* da */ { IDS_SERVER_LISTENING_DA, "Server lytter" },
/* de */ { IDS_SERVER_LISTENING_DE, "Server wartet auf Verbindung" },
/* es */ { IDS_SERVER_LISTENING_ES, "Servidor en escucha" },
/* fr */ { IDS_SERVER_LISTENING_FR, "Le serveur écoute" },
/* hu */ { IDS_SERVER_LISTENING_HU, "Várakozás kliensre" },
/* it */ { IDS_SERVER_LISTENING_IT, "Server in ascolto" },
/* ko */ { IDS_SERVER_LISTENING_KO, "¼­¹ö ¸®½º´×" },
/* nl */ { IDS_SERVER_LISTENING_NL, "Server wacht op verbinding" },
/* pl */ { IDS_SERVER_LISTENING_PL, "Serwer nas³uchuje" },
/* ru */ { IDS_SERVER_LISTENING_RU, "Server listening" },
/* sv */ { IDS_SERVER_LISTENING_SV, "Servern lyssnar" },
/* tr */ { IDS_SERVER_LISTENING_TR, "Sunucu dinleniyor" },

/* en */ { IDS_CONNECTED_SERVER,    "Connected server" },
/* da */ { IDS_CONNECTED_SERVER_DA, "Tilsluttet server" },
/* de */ { IDS_CONNECTED_SERVER_DE, "Server verbunden" },
/* es */ { IDS_CONNECTED_SERVER_ES, "Servidor conectado" },
/* fr */ { IDS_CONNECTED_SERVER_FR, "Connecté au serveur" },
/* hu */ { IDS_CONNECTED_SERVER_HU, "Kapcsolódva kiszolgálóhoz" },
/* it */ { IDS_CONNECTED_SERVER_IT, "Server connesso" },
/* ko */ { IDS_CONNECTED_SERVER_KO, "¿¬°áµÈ ¼­¹ö" },
/* nl */ { IDS_CONNECTED_SERVER_NL, "Verbonden server" },
/* pl */ { IDS_CONNECTED_SERVER_PL, "Po³±czony serwer" },
/* ru */ { IDS_CONNECTED_SERVER_RU, "Connected server" },
/* sv */ { IDS_CONNECTED_SERVER_SV, "Ansluten server" },
/* tr */ { IDS_CONNECTED_SERVER_TR, "Sunucuya baðlanýldý" },

/* en */ { IDS_CONNECTED_CLIENT,    "Connected client" },
/* da */ { IDS_CONNECTED_CLIENT_DA, "Tilsluttet klient" },
/* de */ { IDS_CONNECTED_CLIENT_DE, "Verbunden, Client" },
/* es */ { IDS_CONNECTED_CLIENT_ES, "Cliente conectado" },
/* fr */ { IDS_CONNECTED_CLIENT_FR, "Client connecté" },
/* hu */ { IDS_CONNECTED_CLIENT_HU, "Kapcsolódó kliens" },
/* it */ { IDS_CONNECTED_CLIENT_IT, "Client connesso" },
/* ko */ { IDS_CONNECTED_CLIENT_KO, "¿¬°áµÈ Å¬¶óÀÌ¾ðÆ®" },
/* nl */ { IDS_CONNECTED_CLIENT_NL, "Verbonden client" },
/* pl */ { IDS_CONNECTED_CLIENT_PL, "Po³±czony klient" },
/* ru */ { IDS_CONNECTED_CLIENT_RU, "Connected client" },
/* sv */ { IDS_CONNECTED_CLIENT_SV, "Ansluten klient" },
/* tr */ { IDS_CONNECTED_CLIENT_TR, "Baðlý istemci" },

/* en */ { IDS_NETPLAY_SETTINGS,    "Netplay Settings" },
/* da */ { IDS_NETPLAY_SETTINGS_DA, "Nætværksspilindstillinger" },
/* de */ { IDS_NETPLAY_SETTINGS_DE, "Netplay Einstellungen" },
/* es */ { IDS_NETPLAY_SETTINGS_ES, "Ajustes Netplay" },
/* fr */ { IDS_NETPLAY_SETTINGS_FR, "Paramètres de jeu en réseau" },
/* hu */ { IDS_NETPLAY_SETTINGS_HU, "Hálózati játék beállításai" },
/* it */ { IDS_NETPLAY_SETTINGS_IT, "Impostazioni Netplay" },
/* ko */ { IDS_NETPLAY_SETTINGS_KO, "³ÝÇÃ·¹ÀÌ ¼³Á¤" },
/* nl */ { IDS_NETPLAY_SETTINGS_NL, "Netplay Instellingen" },
/* pl */ { IDS_NETPLAY_SETTINGS_PL, "Ustawienia gry sieciowej" },
/* ru */ { IDS_NETPLAY_SETTINGS_RU, "½ÐáâàÞÙÚØ áÕâÕÒÞÙ ØÓàë" },
/* sv */ { IDS_NETPLAY_SETTINGS_SV, "Inställningar för nätverksspel" },
/* tr */ { IDS_NETPLAY_SETTINGS_TR, "Netplay Ayarlarý" },

/* en */ { IDS_SAVE_SCREENSHOT,    "Save screenshot file" },
/* da */ { IDS_SAVE_SCREENSHOT_DA, "Gem screenshot" },
/* de */ { IDS_SAVE_SCREENSHOT_DE, "Screenshot speichern" },
/* es */ { IDS_SAVE_SCREENSHOT_ES, "Grabar fichero de captura de pantalla" },
/* fr */ { IDS_SAVE_SCREENSHOT_FR, "Enregistrer une capture d'écran" },
/* hu */ { IDS_SAVE_SCREENSHOT_HU, "Fénykép fájl mentése" },
/* it */ { IDS_SAVE_SCREENSHOT_IT, "Salva schermata su file" },
/* ko */ { IDS_SAVE_SCREENSHOT_KO, "½ºÅ©¸°¼¦ ÆÄÀÏ ÀúÀå" },
/* nl */ { IDS_SAVE_SCREENSHOT_NL, "Opslaan schermafdrukbestand" },
/* pl */ { IDS_SAVE_SCREENSHOT_PL, "Zapisz plik zrzutu ekranu" },
/* ru */ { IDS_SAVE_SCREENSHOT_RU, "ÁÞåàÐÝØâì äÐÙÛ áÚàØÝèÞâÐ" },
/* sv */ { IDS_SAVE_SCREENSHOT_SV, "Spara skärmdump" },
/* tr */ { IDS_SAVE_SCREENSHOT_TR, "Ekran görüntüsü dosyasý kaydet" },

/* en */ { IDS_CANCEL,    "Cancel" },
/* da */ { IDS_CANCEL_DA, "Annullér" },
/* de */ { IDS_CANCEL_DE, "Abbruch" },
/* es */ { IDS_CANCEL_ES, "Cancelar" },
/* fr */ { IDS_CANCEL_FR, "Annuler" },
/* hu */ { IDS_CANCEL_HU, "Mégsem" },
/* it */ { IDS_CANCEL_IT, "Annulla" },
/* ko */ { IDS_CANCEL_KO, "Ãë¼Ò" },
/* nl */ { IDS_CANCEL_NL, "Annuleren" },
/* pl */ { IDS_CANCEL_PL, "Anuluj" },
/* ru */ { IDS_CANCEL_RU, "¾âÜÕÝØâì" },
/* sv */ { IDS_CANCEL_SV, "Avbryt" },
/* tr */ { IDS_CANCEL_TR, "Ýptal" },

/* en */ { IDS_CHOOSE_SCREENSHOT_FORMAT,    "Choose screenshot format" },
/* da */ { IDS_CHOOSE_SCREENSHOT_FORMAT_DA, "Vælg screenshot format" },
/* de */ { IDS_CHOOSE_SCREENSHOT_FORMAT_DE, "Wähle Screenshot Format" },
/* es */ { IDS_CHOOSE_SCREENSHOT_FORMAT_ES, "Elige formato Captura de pantalla" },
/* fr */ { IDS_CHOOSE_SCREENSHOT_FORMAT_FR, "Chosir le format de capture d'écran" },
/* hu */ { IDS_CHOOSE_SCREENSHOT_FORMAT_HU, "Válasszon fénykép formátumot" },
/* it */ { IDS_CHOOSE_SCREENSHOT_FORMAT_IT, "Seleziona il formato per il salvataggio della schermata" },
/* ko */ { IDS_CHOOSE_SCREENSHOT_FORMAT_KO, "" },  /* fuzzy */
/* nl */ { IDS_CHOOSE_SCREENSHOT_FORMAT_NL, "Kies schermafdrukformaat" },
/* pl */ { IDS_CHOOSE_SCREENSHOT_FORMAT_PL, "Wybierz format zrzutu ekranu" },
/* ru */ { IDS_CHOOSE_SCREENSHOT_FORMAT_RU, "²ëÑàÐâì äÞàÜÐâ áÚàØÝèÞâÐ" },
/* sv */ { IDS_CHOOSE_SCREENSHOT_FORMAT_SV, "Välj format på skärmdump" },
/* tr */ { IDS_CHOOSE_SCREENSHOT_FORMAT_TR, "Ekran görüntüsü biçimi seçin" },

/* en */ { IDS_SAVE_S_SCREENSHOT,    "Save %s screenshot" },
/* da */ { IDS_SAVE_S_SCREENSHOT_DA, "Gem %s-screenshot" },
/* de */ { IDS_SAVE_S_SCREENSHOT_DE, "Screenshot %s speichern" },
/* es */ { IDS_SAVE_S_SCREENSHOT_ES, "Graba %s Captura de pantalla" },
/* fr */ { IDS_SAVE_S_SCREENSHOT_FR, "Enregistrer une capture d'écran %s" },
/* hu */ { IDS_SAVE_S_SCREENSHOT_HU, "%s fénykép mentése" },
/* it */ { IDS_SAVE_S_SCREENSHOT_IT, "Salva schermata %s" },
/* ko */ { IDS_SAVE_S_SCREENSHOT_KO, "" },  /* fuzzy */
/* nl */ { IDS_SAVE_S_SCREENSHOT_NL, "Schermafdruk %s opslaan" },
/* pl */ { IDS_SAVE_S_SCREENSHOT_PL, "Zapisz zrzut ekranu %s" },
/* ru */ { IDS_SAVE_S_SCREENSHOT_RU, "Save %s screenshot" },
/* sv */ { IDS_SAVE_S_SCREENSHOT_SV, "Spara %s-skärmdump" },
/* tr */ { IDS_SAVE_S_SCREENSHOT_TR, "%s ekran görüntüsünü kaydet" },

/* en */ { IDS_CHOOSE_VICII_MODEL,    "Choose VICII model" },
/* da */ { IDS_CHOOSE_VICII_MODEL_DA, "" },  /* fuzzy */
/* de */ { IDS_CHOOSE_VICII_MODEL_DE, "VIC-II Modell wählen" },
/* es */ { IDS_CHOOSE_VICII_MODEL_ES, "" },  /* fuzzy */
/* fr */ { IDS_CHOOSE_VICII_MODEL_FR, "" },  /* fuzzy */
/* hu */ { IDS_CHOOSE_VICII_MODEL_HU, "" },  /* fuzzy */
/* it */ { IDS_CHOOSE_VICII_MODEL_IT, "" },  /* fuzzy */
/* ko */ { IDS_CHOOSE_VICII_MODEL_KO, "" },  /* fuzzy */
/* nl */ { IDS_CHOOSE_VICII_MODEL_NL, "Kies VIC-II model" },
/* pl */ { IDS_CHOOSE_VICII_MODEL_PL, "" },  /* fuzzy */
/* ru */ { IDS_CHOOSE_VICII_MODEL_RU, "" },  /* fuzzy */
/* sv */ { IDS_CHOOSE_VICII_MODEL_SV, "" },  /* fuzzy */
/* tr */ { IDS_CHOOSE_VICII_MODEL_TR, "" },  /* fuzzy */

/* en */ { IDS_DETACHED_DEVICE_D,    "Detached device %d" },
/* da */ { IDS_DETACHED_DEVICE_D_DA, "Frakoblet enhed %d" },
/* de */ { IDS_DETACHED_DEVICE_D_DE, "Gerät %d entfernt" },
/* es */ { IDS_DETACHED_DEVICE_D_ES, "Periférico extraído %d" },
/* fr */ { IDS_DETACHED_DEVICE_D_FR, "Périphérique détaché %d" },
/* hu */ { IDS_DETACHED_DEVICE_D_HU, "%d eszköz leválasztva" },
/* it */ { IDS_DETACHED_DEVICE_D_IT, "Rimosso dispositivo %d" },
/* ko */ { IDS_DETACHED_DEVICE_D_KO, "" },  /* fuzzy */
/* nl */ { IDS_DETACHED_DEVICE_D_NL, "Apparaat %d ontkoppeld" },
/* pl */ { IDS_DETACHED_DEVICE_D_PL, "Od³±czono urz±dzenie %d" },
/* ru */ { IDS_DETACHED_DEVICE_D_RU, "Detached device %d" },
/* sv */ { IDS_DETACHED_DEVICE_D_SV, "Kopplade från enhet %d" },
/* tr */ { IDS_DETACHED_DEVICE_D_TR, "Çýkartýlmýþ aygýt %d" },

/* en */ { IDS_ATTACHED_S_TO_DEVICE_D,    "Attached %s to device#%d" },
/* da */ { IDS_ATTACHED_S_TO_DEVICE_D_DA, "Tilsluttet %s til enhed #%d" },
/* de */ { IDS_ATTACHED_S_TO_DEVICE_D_DE, "%s mit Gerät#%d verbunden" },
/* es */ { IDS_ATTACHED_S_TO_DEVICE_D_ES, "Insertado %s en unidad#%d" },
/* fr */ { IDS_ATTACHED_S_TO_DEVICE_D_FR, "%s attaché au périphérique #%d" },
/* hu */ { IDS_ATTACHED_S_TO_DEVICE_D_HU, "%s csatolva #%d eszközhöz" },
/* it */ { IDS_ATTACHED_S_TO_DEVICE_D_IT, "Selezionato %s per la periferica #%d" },
/* ko */ { IDS_ATTACHED_S_TO_DEVICE_D_KO, "" },  /* fuzzy */
/* nl */ { IDS_ATTACHED_S_TO_DEVICE_D_NL, "%s aan apparaat#%d gekoppeld" },
/* pl */ { IDS_ATTACHED_S_TO_DEVICE_D_PL, "Pod³±czono %s pod urz±dzenie#%d" },
/* ru */ { IDS_ATTACHED_S_TO_DEVICE_D_RU, "Attached %s to device#%d" },
/* sv */ { IDS_ATTACHED_S_TO_DEVICE_D_SV, "Anslöt %s till enhet %d" },
/* tr */ { IDS_ATTACHED_S_TO_DEVICE_D_TR, "%s aygýt#%d'e baðlanmýþ" },

/* en */ { IDS_VICE_QUESTION,    "VICE Question" },
/* da */ { IDS_VICE_QUESTION_DA, "VICE-spørgsmål" },
/* de */ { IDS_VICE_QUESTION_DE, "VICE Frage" },
/* es */ { IDS_VICE_QUESTION_ES, "VICE cuestion" },
/* fr */ { IDS_VICE_QUESTION_FR, "Question VICE" },
/* hu */ { IDS_VICE_QUESTION_HU, "VICE kérdés" },
/* it */ { IDS_VICE_QUESTION_IT, "Domanda di VICE" },
/* ko */ { IDS_VICE_QUESTION_KO, "" },  /* fuzzy */
/* nl */ { IDS_VICE_QUESTION_NL, "VICE vraag" },
/* pl */ { IDS_VICE_QUESTION_PL, "Pytanie VICE" },
/* ru */ { IDS_VICE_QUESTION_RU, "VICE Question" },
/* sv */ { IDS_VICE_QUESTION_SV, "VICE-fråga" },
/* tr */ { IDS_VICE_QUESTION_TR, "VICE Sorusu" },

/* en */ { IDS_EXTEND_TO_40_TRACK,    "Do you want to extend the image?" },
/* da */ { IDS_EXTEND_TO_40_TRACK_DA, "" },  /* fuzzy */
/* de */ { IDS_EXTEND_TO_40_TRACK_DE, "Soll das Diskimage erweitert werden?" },
/* es */ { IDS_EXTEND_TO_40_TRACK_ES, "¿Quieres extender la imagen?" },
/* fr */ { IDS_EXTEND_TO_40_TRACK_FR, "" },  /* fuzzy */
/* hu */ { IDS_EXTEND_TO_40_TRACK_HU, "" },  /* fuzzy */
/* it */ { IDS_EXTEND_TO_40_TRACK_IT, "Vuoi estendere l'immagine?" },
/* ko */ { IDS_EXTEND_TO_40_TRACK_KO, "" },  /* fuzzy */
/* nl */ { IDS_EXTEND_TO_40_TRACK_NL, "Wilt u het bestand uitbreiden?" },
/* pl */ { IDS_EXTEND_TO_40_TRACK_PL, "Czy chcesz rozszerzyæ obraz ?" },
/* ru */ { IDS_EXTEND_TO_40_TRACK_RU, "" },  /* fuzzy */
/* sv */ { IDS_EXTEND_TO_40_TRACK_SV, "" },  /* fuzzy */
/* tr */ { IDS_EXTEND_TO_40_TRACK_TR, "" },  /* fuzzy */

/* en */ { IDS_YES_NO,    "Yes|No" },
/* da */ { IDS_YES_NO_DA, "Ja|Nej" },
/* de */ { IDS_YES_NO_DE, "Ja|Nein" },
/* es */ { IDS_YES_NO_ES, "Sí/No" },
/* fr */ { IDS_YES_NO_FR, "Oui|Non" },
/* hu */ { IDS_YES_NO_HU, "Igen|Nem" },
/* it */ { IDS_YES_NO_IT, "Sì|No" },
/* ko */ { IDS_YES_NO_KO, "³×|¾Æ´Ï¿ä" },
/* nl */ { IDS_YES_NO_NL, "Ja|Nee" },
/* pl */ { IDS_YES_NO_PL, "Tak|Nie" },
/* ru */ { IDS_YES_NO_RU, "´Ð|½Õâ" },
/* sv */ { IDS_YES_NO_SV, "Ja|Nej" },
/* tr */ { IDS_YES_NO_TR, "Evet|Hayýr" },

/* en */ { IDS_DETACHED_TAPE,    "Detached tape" },
/* da */ { IDS_DETACHED_TAPE_DA, "Frakoblet bånd" },
/* de */ { IDS_DETACHED_TAPE_DE, "Band Image entfernt" },
/* es */ { IDS_DETACHED_TAPE_ES, "Cinta extraida" },
/* fr */ { IDS_DETACHED_TAPE_FR, "Datassette détaché" },
/* hu */ { IDS_DETACHED_TAPE_HU, "Szalag leválasztva" },
/* it */ { IDS_DETACHED_TAPE_IT, "Cassetta rimossa" },
/* ko */ { IDS_DETACHED_TAPE_KO, "" },  /* fuzzy */
/* nl */ { IDS_DETACHED_TAPE_NL, "Tape ontkoppeld" },
/* pl */ { IDS_DETACHED_TAPE_PL, "Od³±czono ta¶mê" },
/* ru */ { IDS_DETACHED_TAPE_RU, "Detached tape" },
/* sv */ { IDS_DETACHED_TAPE_SV, "Kopplade från band" },
/* tr */ { IDS_DETACHED_TAPE_TR, "Teyp çýkartýldý" },

/* en */ { IDS_ATTACHED_TAPE_S,    "Attached tape %s" },
/* da */ { IDS_ATTACHED_TAPE_S_DA, "Tilsluttet bånd %s" },
/* de */ { IDS_ATTACHED_TAPE_S_DE, "Band Image %s eingelegt" },
/* es */ { IDS_ATTACHED_TAPE_S_ES, "Cinta insertada %s" },
/* fr */ { IDS_ATTACHED_TAPE_S_FR, "Datassette %s attaché" },
/* hu */ { IDS_ATTACHED_TAPE_S_HU, "%s szalag csatolva" },
/* it */ { IDS_ATTACHED_TAPE_S_IT, "Cassetta %s selezionata" },
/* ko */ { IDS_ATTACHED_TAPE_S_KO, "" },  /* fuzzy */
/* nl */ { IDS_ATTACHED_TAPE_S_NL, "Tape %s gekoppeld" },
/* pl */ { IDS_ATTACHED_TAPE_S_PL, "Zamontowana ta¶ma %s" },
/* ru */ { IDS_ATTACHED_TAPE_S_RU, "Attached tape %s" },
/* sv */ { IDS_ATTACHED_TAPE_S_SV, "Anslöt band %s" },
/* tr */ { IDS_ATTACHED_TAPE_S_TR, "Yerleþtirilmiþ teyp %s" },

/* en */ { IDS_SELECT_START_SNAPSHOT,    "Select start snapshot for event history" },
/* da */ { IDS_SELECT_START_SNAPSHOT_DA, "Vælg start-snapshot for hændelseshistorik" },
/* de */ { IDS_SELECT_START_SNAPSHOT_DE, "Startsnapshot für Ereignishistory festlegen" },
/* es */ { IDS_SELECT_START_SNAPSHOT_ES, "Seleccionar inicio volcado para historial de eventos" },
/* fr */ { IDS_SELECT_START_SNAPSHOT_FR, "Sélectionner l'instantané de départ pour l'historique des événements" },
/* hu */ { IDS_SELECT_START_SNAPSHOT_HU, "Válasszon kezdeti pillanatképet az esemény rögzítéshez" },
/* it */ { IDS_SELECT_START_SNAPSHOT_IT, "Seleziona snapshot iniziale per la cronologia degli eventi" },
/* ko */ { IDS_SELECT_START_SNAPSHOT_KO, "ÀÌº¥Æ® È÷½ºÅä¸® ½º³À¼¦À» ¼±ÅÃ ½ÃÀÛ ÇÏ¼¼¿ä" },
/* nl */ { IDS_SELECT_START_SNAPSHOT_NL, "Selecteer start momentopname voor gebeurtenis geschiedenis" },
/* pl */ { IDS_SELECT_START_SNAPSHOT_PL, "Wybierz pocz±tkowy zrzut historii zdarzeñ" },
/* ru */ { IDS_SELECT_START_SNAPSHOT_RU, "Select start snapshot for event history" },
/* sv */ { IDS_SELECT_START_SNAPSHOT_SV, "Välj startögonblicksmodul för händelsehistorik" },
/* tr */ { IDS_SELECT_START_SNAPSHOT_TR, "Olay tarihçesi için baþlangýç anlýk görüntüsünü seç" },

/* en */ { IDS_SELECT_END_SNAPSHOT,    "Select end snapshot for event history" },
/* da */ { IDS_SELECT_END_SNAPSHOT_DA, "Vælg slut-snapshot for hændelseshistorik" },
/* de */ { IDS_SELECT_END_SNAPSHOT_DE, "Startsnapshot für Ereignishistory festlegen" },
/* es */ { IDS_SELECT_END_SNAPSHOT_ES, "Seleccionar fin volcado para historial de eventos" },
/* fr */ { IDS_SELECT_END_SNAPSHOT_FR, "Sélectionner l'instantané de départ pour l'historique des événements" },
/* hu */ { IDS_SELECT_END_SNAPSHOT_HU, "Válasszon végsõ pillanatképet az esemény rögzítéshez" },
/* it */ { IDS_SELECT_END_SNAPSHOT_IT, "Seleziona snapshot finale per la cronologia degli eventi" },
/* ko */ { IDS_SELECT_END_SNAPSHOT_KO, "ÀÌº¥Æ® È÷½ºÅä¸® ½º³À¼¦À» ¼±ÅÃ Á¾·á ÇÏ¼¼¿ä" },
/* nl */ { IDS_SELECT_END_SNAPSHOT_NL, "Selecteer eind momentopname voor gebeurtenis geschiedenis" },
/* pl */ { IDS_SELECT_END_SNAPSHOT_PL, "Wybierz koñcowy zrzut historii zdarzeñ" },
/* ru */ { IDS_SELECT_END_SNAPSHOT_RU, "Select end snapshot for event history" },
/* sv */ { IDS_SELECT_END_SNAPSHOT_SV, "Välj slutögonblicksbild för händelsehistorik" },
/* tr */ { IDS_SELECT_END_SNAPSHOT_TR, "Olay tarihçesi için bitiþ anlýk görüntüsünü seç" },

/* en */ { IDS_REALLY_EXIT,    "Do you really want to exit?\n\nAll the data present in the emulated RAM will be lost." },
/* da */ { IDS_REALLY_EXIT_DA, "Vil du virkelig afslutte?\n\nAlle data i emuleret RAM går tabt." },
/* de */ { IDS_REALLY_EXIT_DE, "Willst Du wirklich beenden?\nDaten im emulierten Speicher RAM gehen verloren." },
/* es */ { IDS_REALLY_EXIT_ES, "¿Realmente quieres salir?\n\nSe perderán todos los datos presentes en la RAM emulada." },
/* fr */ { IDS_REALLY_EXIT_FR, "Désirez-vous vraiment quitter?\n\nToutes les données présentes dans la mémoire de l'émulateur seront perdues." },
/* hu */ { IDS_REALLY_EXIT_HU, "Biztosan kilép?\n\nAz emulált memória teljes tartalma el fog veszni." },
/* it */ { IDS_REALLY_EXIT_IT, "Uscire davvero?\n\nTutti i dati presenti nella RAM emulata saranno persi." },
/* ko */ { IDS_REALLY_EXIT_KO, "" },  /* fuzzy */
/* nl */ { IDS_REALLY_EXIT_NL, "Wilt u echt afsluiten?\n\nAlle data in geëmuleerd geheugen zal verloren gaan." },
/* pl */ { IDS_REALLY_EXIT_PL, "Na pewno wyj¶æ?\n\nWszystkie dane w emulowanym RAM-ie zostan± utracone." },
/* ru */ { IDS_REALLY_EXIT_RU, "Do you really want to exit?\n\nAll the data present in the emulated RAM will be lost." },
/* sv */ { IDS_REALLY_EXIT_SV, "Vill du verkligen avsluta?\n\nAlla data i emulerat RAM kommer förloras." },
/* tr */ { IDS_REALLY_EXIT_TR, "Çýkmak istediðinizden emin misiniz?\n\nEmüle edilmiþ RAM'de bulunan mevcut tüm veri kaybedilecektir." },

/* en */ { IDS_ATTACH_CRT_IMAGE,    "Attach CRT cartridge image" },
/* da */ { IDS_ATTACH_CRT_IMAGE_DA, "Tilslut CRT-cartridge image" },
/* de */ { IDS_ATTACH_CRT_IMAGE_DE, "CRT Erweiterungsmodul Image einlegen" },
/* es */ { IDS_ATTACH_CRT_IMAGE_ES, "Insertar imagen de cartucho CRT" },
/* fr */ { IDS_ATTACH_CRT_IMAGE_FR, "Insérer une cartouche CRT" },
/* hu */ { IDS_ATTACH_CRT_IMAGE_HU, "CRT cartridge képmás csatolása" },
/* it */ { IDS_ATTACH_CRT_IMAGE_IT, "Seleziona immagine cartuccia CRT" },
/* ko */ { IDS_ATTACH_CRT_IMAGE_KO, "CRT Ä«Æ®¸®Áö ÀÌ¹ÌÁö¸¦ ºÙ¿©³ÖÀ¸½Ã¿À" },
/* nl */ { IDS_ATTACH_CRT_IMAGE_NL, "Koppel CRT cartridge bestand" },
/* pl */ { IDS_ATTACH_CRT_IMAGE_PL, "Zamontuj obraz kartrid¿a CRT" },
/* ru */ { IDS_ATTACH_CRT_IMAGE_RU, "Attach CRT cartridge image" },
/* sv */ { IDS_ATTACH_CRT_IMAGE_SV, "Anslut CRT-insticksmodulfil" },
/* tr */ { IDS_ATTACH_CRT_IMAGE_TR, "CRT kartuþ imajý yerleþtir" },

/* en */ { IDS_ATTACH_RAW_8KB_IMAGE,    "Attach raw 8KB cartridge image" },
/* da */ { IDS_ATTACH_RAW_8KB_IMAGE_DA, "Tilslut råt 8kB-cartridge image" },
/* de */ { IDS_ATTACH_RAW_8KB_IMAGE_DE, "8KB Erweiterungsmodul (raw) Image einlegen" },
/* es */ { IDS_ATTACH_RAW_8KB_IMAGE_ES, "Insertar imagen de cartucho 8KB" },
/* fr */ { IDS_ATTACH_RAW_8KB_IMAGE_FR, "Insérer une cartouche 8KO" },
/* hu */ { IDS_ATTACH_RAW_8KB_IMAGE_HU, "Nyers 8KB-os cartridge képmás csatolása" },
/* it */ { IDS_ATTACH_RAW_8KB_IMAGE_IT, "Seleziona immagine cartuccia di 8KB" },
/* ko */ { IDS_ATTACH_RAW_8KB_IMAGE_KO, "8kb Ä«Æ®¸®Áö ÀÌ¹ÌÁö¸¦ ºÙÀÌ¼¼¿ä" },
/* nl */ { IDS_ATTACH_RAW_8KB_IMAGE_NL, "Koppel binair 8KB cartridge bestand" },
/* pl */ { IDS_ATTACH_RAW_8KB_IMAGE_PL, "Zamontuj surowy obraz kartrid¿a 8KB" },
/* ru */ { IDS_ATTACH_RAW_8KB_IMAGE_RU, "¿àØÚàÕßØâì ßàÞÕÚâ ÞÑàÐ×Ð 8ºÑ ÚÐàâàØÔÖÐ" },
/* sv */ { IDS_ATTACH_RAW_8KB_IMAGE_SV, "Anslut rå 8KB-insticksmodulavbildning" },
/* tr */ { IDS_ATTACH_RAW_8KB_IMAGE_TR, "Düz 8KB kartuþ imajý yerleþtir" },

/* en */ { IDS_ATTACH_RAW_16KB_IMAGE,    "Attach raw 16KB cartridge image" },
/* da */ { IDS_ATTACH_RAW_16KB_IMAGE_DA, "Tilslut råt 16kB-cartridge image" },
/* de */ { IDS_ATTACH_RAW_16KB_IMAGE_DE, "16KB Erweiterungsmodul (raw) Image einlegen" },
/* es */ { IDS_ATTACH_RAW_16KB_IMAGE_ES, "Insertar imagen de cartucho 16KB" },
/* fr */ { IDS_ATTACH_RAW_16KB_IMAGE_FR, "Insérer une cartouche 16KO" },
/* hu */ { IDS_ATTACH_RAW_16KB_IMAGE_HU, "Nyers 16KB-os cartridge képmás csatolása" },
/* it */ { IDS_ATTACH_RAW_16KB_IMAGE_IT, "Seleziona immagine cartuccia di 16KB" },
/* ko */ { IDS_ATTACH_RAW_16KB_IMAGE_KO, "16kb Ä«Æ®¸®Áö ÀÌ¹ÌÁö¸¦ ºÙÀÌ¼¼¿ä" },
/* nl */ { IDS_ATTACH_RAW_16KB_IMAGE_NL, "Koppel binair 16KB cartridge bestand" },
/* pl */ { IDS_ATTACH_RAW_16KB_IMAGE_PL, "Zamontuj surowy obraz kartrid¿a 16KB" },
/* ru */ { IDS_ATTACH_RAW_16KB_IMAGE_RU, "¿àØÚàÕßØâì ßàÞÕÚâ ÞÑàÐ×Ð 16ºÑ ÚÐàâàØÔÖÐ" },
/* sv */ { IDS_ATTACH_RAW_16KB_IMAGE_SV, "Anslut rå 16KB-insticksmodulavbildning" },
/* tr */ { IDS_ATTACH_RAW_16KB_IMAGE_TR, "Düz 16KB kartuþ imajý yerleþtir" },

/* en */ { IDS_ATTACH_ACTION_REPLAY_IMAGE,    "Attach raw 32KB Action Replay cartridge image" },
/* da */ { IDS_ATTACH_ACTION_REPLAY_IMAGE_DA, "Tilslut råt 32KB Action Replay cartridge image" },
/* de */ { IDS_ATTACH_ACTION_REPLAY_IMAGE_DE, "32kB Action Replay Image (raw) einlegen" },
/* es */ { IDS_ATTACH_ACTION_REPLAY_IMAGE_ES, "Insertar imagen cartucho Action Replay 32KB" },
/* fr */ { IDS_ATTACH_ACTION_REPLAY_IMAGE_FR, "Insérer une cartouche Action Replay 32KO" },
/* hu */ { IDS_ATTACH_ACTION_REPLAY_IMAGE_HU, "32KB-os Action Replay cartridge képmás csatolása" },
/* it */ { IDS_ATTACH_ACTION_REPLAY_IMAGE_IT, "Seleziona immagine cartuccia Action Replay di 32KB" },
/* ko */ { IDS_ATTACH_ACTION_REPLAY_IMAGE_KO, "" },  /* fuzzy */
/* nl */ { IDS_ATTACH_ACTION_REPLAY_IMAGE_NL, "Koppel binair 32KB Action Replay cartridge bestand" },
/* pl */ { IDS_ATTACH_ACTION_REPLAY_IMAGE_PL, "Zamontuj surowy obraz kartid¿a 32KB Action Replay" },
/* ru */ { IDS_ATTACH_ACTION_REPLAY_IMAGE_RU, "Attach raw 32KB Action Replay cartridge image" },
/* sv */ { IDS_ATTACH_ACTION_REPLAY_IMAGE_SV, "Anslut rå 32KB-Action Replay-insticksmodulfil" },
/* tr */ { IDS_ATTACH_ACTION_REPLAY_IMAGE_TR, "Düz 32KB Action Replay kartuþ imajý yerleþtir" },

/* en */ { IDS_ATTACH_ACTION_REPLAY3_IMAGE,    "Attach raw 16KB Action Replay MK3 cartridge image" },
/* da */ { IDS_ATTACH_ACTION_REPLAY3_IMAGE_DA, "Tilslut råt 16KB Action Replay MK3 cartridge image" },
/* de */ { IDS_ATTACH_ACTION_REPLAY3_IMAGE_DE, "16kB Action Replay MK3 Image (raw) einlegen" },
/* es */ { IDS_ATTACH_ACTION_REPLAY3_IMAGE_ES, "Insertar imagen cartucho Action Replay MK3 16KB" },
/* fr */ { IDS_ATTACH_ACTION_REPLAY3_IMAGE_FR, "Attacher une image de cartouche 16 ko Action Replay MK3" },
/* hu */ { IDS_ATTACH_ACTION_REPLAY3_IMAGE_HU, "" },  /* fuzzy */
/* it */ { IDS_ATTACH_ACTION_REPLAY3_IMAGE_IT, "Seleziona immagine cartuccia Action Replay MK3 di 16KB" },
/* ko */ { IDS_ATTACH_ACTION_REPLAY3_IMAGE_KO, "" },  /* fuzzy */
/* nl */ { IDS_ATTACH_ACTION_REPLAY3_IMAGE_NL, "Koppel binair 16KB Action Replay MK3 cartridge bestand" },
/* pl */ { IDS_ATTACH_ACTION_REPLAY3_IMAGE_PL, "Zamontuj surowy obraz kartrid¿a 16KB Action Replay MK3" },
/* ru */ { IDS_ATTACH_ACTION_REPLAY3_IMAGE_RU, "Attach raw 16KB Action Replay MK3 cartridge image" },
/* sv */ { IDS_ATTACH_ACTION_REPLAY3_IMAGE_SV, "Anslut rå 16KB-Action Replay MK3-insticksmodulfil" },
/* tr */ { IDS_ATTACH_ACTION_REPLAY3_IMAGE_TR, "Düz 16KB Action Replay MK3 kartuþ imajý yerleþtir" },

/* en */ { IDS_ATTACH_ACTION_REPLAY4_IMAGE,    "Attach raw 32KB Action Replay MK4 cartridge image" },
/* da */ { IDS_ATTACH_ACTION_REPLAY4_IMAGE_DA, "Tilslut råt 32KB Action Replay MK4 cartridge image" },
/* de */ { IDS_ATTACH_ACTION_REPLAY4_IMAGE_DE, "32kB Action Replay MK4 Image einlegen" },
/* es */ { IDS_ATTACH_ACTION_REPLAY4_IMAGE_ES, "Insertar imagen cartucho Action Replay MK4 32KB" },
/* fr */ { IDS_ATTACH_ACTION_REPLAY4_IMAGE_FR, "Attacher une image de cartouche 2 ko Action Replay MK4" },
/* hu */ { IDS_ATTACH_ACTION_REPLAY4_IMAGE_HU, "" },  /* fuzzy */
/* it */ { IDS_ATTACH_ACTION_REPLAY4_IMAGE_IT, "Seleziona immagine cartuccia Action Replay MK4 di 326KB" },
/* ko */ { IDS_ATTACH_ACTION_REPLAY4_IMAGE_KO, "" },  /* fuzzy */
/* nl */ { IDS_ATTACH_ACTION_REPLAY4_IMAGE_NL, "Koppel binair 32KB Action Replay MK4 cartridge bestand" },
/* pl */ { IDS_ATTACH_ACTION_REPLAY4_IMAGE_PL, "Zamontuj surowy obraz kartrid¿a 32KB Action Replay MK4" },
/* ru */ { IDS_ATTACH_ACTION_REPLAY4_IMAGE_RU, "Attach raw 32KB Action Replay MK4 cartridge image" },
/* sv */ { IDS_ATTACH_ACTION_REPLAY4_IMAGE_SV, "Anslut rå 32KB-Action Replay MK4-insticksmodulfil" },
/* tr */ { IDS_ATTACH_ACTION_REPLAY4_IMAGE_TR, "Düz 32KB Action Replay MK4 kartuþ imajý yerleþtir" },

/* en */ { IDS_ATTACH_STARDOS_IMAGE,    "Attach raw 16KB Stardos cartridge image" },
/* da */ { IDS_ATTACH_STARDOS_IMAGE_DA, "Tilslut råt 16KB StarDOS cartridge image" },
/* de */ { IDS_ATTACH_STARDOS_IMAGE_DE, "16kB Stardos Erweiterungsmodul Image (raw) einlegen" },
/* es */ { IDS_ATTACH_STARDOS_IMAGE_ES, "Insertar imagen cartucho Stardos 16KB" },
/* fr */ { IDS_ATTACH_STARDOS_IMAGE_FR, "Attacher une image de cartouche 16 ko Stardos" },
/* hu */ { IDS_ATTACH_STARDOS_IMAGE_HU, "" },  /* fuzzy */
/* it */ { IDS_ATTACH_STARDOS_IMAGE_IT, "Seleziona immagine cartuccia Stardos di 16KB" },
/* ko */ { IDS_ATTACH_STARDOS_IMAGE_KO, "" },  /* fuzzy */
/* nl */ { IDS_ATTACH_STARDOS_IMAGE_NL, "Koppel binair 16KB Stardos cartridge bestand" },
/* pl */ { IDS_ATTACH_STARDOS_IMAGE_PL, "Zamontuj surowy obraz kartrid¿a 16KB Stardos" },
/* ru */ { IDS_ATTACH_STARDOS_IMAGE_RU, "Attach raw 16KB Stardos cartridge image" },
/* sv */ { IDS_ATTACH_STARDOS_IMAGE_SV, "Anslut rå 16KB Stardos-insticksmodulfil" },
/* tr */ { IDS_ATTACH_STARDOS_IMAGE_TR, "Düz 16KB Stardos kartuþ imajý yerleþtir" },

/* en */ { IDS_ATTACH_ATOMIC_POWER_IMAGE,    "Attach raw 32KB Atomic Power cartridge image" },
/* da */ { IDS_ATTACH_ATOMIC_POWER_IMAGE_DA, "Tilslut råt 32kB Atomic Power-cartridge image" },
/* de */ { IDS_ATTACH_ATOMIC_POWER_IMAGE_DE, "32kB Atomic Power Image einlegen" },
/* es */ { IDS_ATTACH_ATOMIC_POWER_IMAGE_ES, "Insertar imagen cartucho Atomic Power 32KB" },
/* fr */ { IDS_ATTACH_ATOMIC_POWER_IMAGE_FR, "Insérer une cartouche Atomic Power 32KO" },
/* hu */ { IDS_ATTACH_ATOMIC_POWER_IMAGE_HU, "32KB-os Atomic Power cartridge képmás csatolása" },
/* it */ { IDS_ATTACH_ATOMIC_POWER_IMAGE_IT, "Seleziona immagine cartuccia Atomic Power di 32KB" },
/* ko */ { IDS_ATTACH_ATOMIC_POWER_IMAGE_KO, "32kb Atomic Power Ä«Æ®¸®Áö ÀÌ¹ÌÁö¸¦ ºÙÀÌ¼¼¿ä" },
/* nl */ { IDS_ATTACH_ATOMIC_POWER_IMAGE_NL, "Koppel binair 32KB Atomic Power cartridge bestand" },
/* pl */ { IDS_ATTACH_ATOMIC_POWER_IMAGE_PL, "Zamontuj surowy obraz kartrid¿a 32KB Atomic Power" },
/* ru */ { IDS_ATTACH_ATOMIC_POWER_IMAGE_RU, "Attach raw 32KB Atomic Power cartridge image" },
/* sv */ { IDS_ATTACH_ATOMIC_POWER_IMAGE_SV, "Anslut rå 32KB Atomic Power-insticksmodulfil" },
/* tr */ { IDS_ATTACH_ATOMIC_POWER_IMAGE_TR, "Düz 32KB Atomic Power kartuþ imajý yerleþtir" },

/* en */ { IDS_ATTACH_EPYX_FASTLOAD_IMAGE,    "Attach raw 8KB Epyx FastLoad cartridge image" },
/* da */ { IDS_ATTACH_EPYX_FASTLOAD_IMAGE_DA, "Tilslut råt 8kB Epyx FastLoad cartridge image" },
/* de */ { IDS_ATTACH_EPYX_FASTLOAD_IMAGE_DE, "8kB Epyx Fastload Image (raw) einlegen" },
/* es */ { IDS_ATTACH_EPYX_FASTLOAD_IMAGE_ES, "Insertar imagen cartucho Epyx Fastload 8KB" },
/* fr */ { IDS_ATTACH_EPYX_FASTLOAD_IMAGE_FR, "Attacher une image de cartouche 8 ko Epyx FastLoad" },
/* hu */ { IDS_ATTACH_EPYX_FASTLOAD_IMAGE_HU, "" },  /* fuzzy */
/* it */ { IDS_ATTACH_EPYX_FASTLOAD_IMAGE_IT, "Seleziona immagine cartuccia Epyx FastLoad di 8KB" },
/* ko */ { IDS_ATTACH_EPYX_FASTLOAD_IMAGE_KO, "8kb Epyx FastLoad Ä«Æ®¸®Áö ÀÌ¹ÌÁö¸¦ ºÙÀÌ¼¼¿ä" },
/* nl */ { IDS_ATTACH_EPYX_FASTLOAD_IMAGE_NL, "Koppel binair 8KB Epyx FastLoad cartridge bestand" },
/* pl */ { IDS_ATTACH_EPYX_FASTLOAD_IMAGE_PL, "Zamontuj surowy obraz kartrid¿a 8KB Epyx FastLoad" },
/* ru */ { IDS_ATTACH_EPYX_FASTLOAD_IMAGE_RU, "Attach raw 8KB Epyx FastLoad cartridge image" },
/* sv */ { IDS_ATTACH_EPYX_FASTLOAD_IMAGE_SV, "Anslut rå 8KB Epyx FastLoad-insticksmodulfil" },
/* tr */ { IDS_ATTACH_EPYX_FASTLOAD_IMAGE_TR, "Düz 8KB Epyx FastLoad kartuþ imajý yerleþtir" },

/* en */ { IDS_ATTACH_IEEE488_IMAGE,    "Attach IEEE-488 Interface cartridge image" },
/* da */ { IDS_ATTACH_IEEE488_IMAGE_DA, "Tilslut IEEE-488 interface cartridge image" },
/* de */ { IDS_ATTACH_IEEE488_IMAGE_DE, "IEEE Schnittstellen Erweiterungsmodul Image einlegen" },
/* es */ { IDS_ATTACH_IEEE488_IMAGE_ES, "Insertar imagen de cartucho Interface IEEE-488" },
/* fr */ { IDS_ATTACH_IEEE488_IMAGE_FR, "Attacher une image de cartouche Interface IEEE-488" },
/* hu */ { IDS_ATTACH_IEEE488_IMAGE_HU, "" },  /* fuzzy */
/* it */ { IDS_ATTACH_IEEE488_IMAGE_IT, "Seleziona immagine cartuccia interfaccia IEEE" },
/* ko */ { IDS_ATTACH_IEEE488_IMAGE_KO, "IEEE-488 ÀÎÅÍÆÐÀÌ½º Ä«Æ®¸®Áö ÀÌ¹ÌÁö¸¦ ºÙ¿©³ÖÀ¸¼¼¿ä" },
/* nl */ { IDS_ATTACH_IEEE488_IMAGE_NL, "Koppel IEEE-488 Interface cartridge bestand" },
/* pl */ { IDS_ATTACH_IEEE488_IMAGE_PL, "Zamontuj obraz kartrid¿a IEEE-488 Interface" },
/* ru */ { IDS_ATTACH_IEEE488_IMAGE_RU, "Attach IEEE-488 Interface cartridge image" },
/* sv */ { IDS_ATTACH_IEEE488_IMAGE_SV, "Anslut IEEE-488-gränssnittsavbildningsfil" },
/* tr */ { IDS_ATTACH_IEEE488_IMAGE_TR, "IEEE-488 Arabirimi kartuþ imajý yerleþtir" },

/* en */ { IDS_ATTACH_RETRO_REPLAY_IMAGE,    "Attach raw 64KB Retro Replay cartridge image" },
/* da */ { IDS_ATTACH_RETRO_REPLAY_IMAGE_DA, "Tilslut råt 64kB-Retro Replay-cartridge image" },
/* de */ { IDS_ATTACH_RETRO_REPLAY_IMAGE_DE, "64kB Retro Replay Image einlegen" },
/* es */ { IDS_ATTACH_RETRO_REPLAY_IMAGE_ES, "Insertar imagen cartucho Retro Replay 64KB" },
/* fr */ { IDS_ATTACH_RETRO_REPLAY_IMAGE_FR, "Insérer une cartouche Retro Replay 64KO" },
/* hu */ { IDS_ATTACH_RETRO_REPLAY_IMAGE_HU, "64KB-os Retro Replay cartridge képmás csatolása" },
/* it */ { IDS_ATTACH_RETRO_REPLAY_IMAGE_IT, "Seleziona immagine cartuccia Retro Replay di 64KB" },
/* ko */ { IDS_ATTACH_RETRO_REPLAY_IMAGE_KO, "64KB Retro Replay Ä«Æ®¸®Áö ÀÌ¹ÌÁö¸¦ ºÙÀÌ¼¼¿ä" },
/* nl */ { IDS_ATTACH_RETRO_REPLAY_IMAGE_NL, "Koppel binair 64KB Retro Replay cartridge bestand" },
/* pl */ { IDS_ATTACH_RETRO_REPLAY_IMAGE_PL, "Zamontuj surowy obraz kartrid¿a 64KB Retro Replay" },
/* ru */ { IDS_ATTACH_RETRO_REPLAY_IMAGE_RU, "Attach raw 64KB Retro Replay cartridge image" },
/* sv */ { IDS_ATTACH_RETRO_REPLAY_IMAGE_SV, "Anslut rå 64KB-Retro Replay-insticksmodulfil" },
/* tr */ { IDS_ATTACH_RETRO_REPLAY_IMAGE_TR, "Düz 64KB Retro Replay kartuþ imajý yerleþtir" },

/* en */ { IDS_ATTACH_IDE64_IMAGE,    "Attach raw 64KB IDE64 cartridge image" },
/* da */ { IDS_ATTACH_IDE64_IMAGE_DA, "Tilslut råt 64kB IDE64-cartridge image" },
/* de */ { IDS_ATTACH_IDE64_IMAGE_DE, "64kB IDE64 Image einlegen" },
/* es */ { IDS_ATTACH_IDE64_IMAGE_ES, "Insertar imagen cartucho IDE64 64KB" },
/* fr */ { IDS_ATTACH_IDE64_IMAGE_FR, "Insérer une cartouche IDE64 64KO" },
/* hu */ { IDS_ATTACH_IDE64_IMAGE_HU, "64KB-os IDE64 cartridge képmás csatolása" },
/* it */ { IDS_ATTACH_IDE64_IMAGE_IT, "Seleziona immagine cartuccia IDE64 di 64KB" },
/* ko */ { IDS_ATTACH_IDE64_IMAGE_KO, "64kb IDE64 Ä«Æ®¸®Áö ÀÌ¹ÌÁö¸¦ ºÙÀÌ¼¼¿ä" },
/* nl */ { IDS_ATTACH_IDE64_IMAGE_NL, "Koppel binair 64KB IDE64 cartridge bestand" },
/* pl */ { IDS_ATTACH_IDE64_IMAGE_PL, "Zamontuj surowy obraz kartrid¿a 64KB IDE64" },
/* ru */ { IDS_ATTACH_IDE64_IMAGE_RU, "Attach raw 64KB IDE64 cartridge image" },
/* sv */ { IDS_ATTACH_IDE64_IMAGE_SV, "Anslut rå 64KB IDE64-insticksmodulfil" },
/* tr */ { IDS_ATTACH_IDE64_IMAGE_TR, "Düz 64KB IDE64 kartuþ imajý yerleþtir" },

/* en */ { IDS_ATTACH_SS4_IMAGE,    "Attach raw 32KB Super Snapshot V4 cartridge image" },
/* da */ { IDS_ATTACH_SS4_IMAGE_DA, "Tilslut råt 32kB Super Snapshot V4 cartridge image" },
/* de */ { IDS_ATTACH_SS4_IMAGE_DE, "32kB Super Snapshot V4 Image (raw) einlegen" },
/* es */ { IDS_ATTACH_SS4_IMAGE_ES, "Insertar imagen cartucho Super Snapshot V4 32KB" },
/* fr */ { IDS_ATTACH_SS4_IMAGE_FR, "Attacher une image de cartouche 32 ko Super Snapshot V4" },
/* hu */ { IDS_ATTACH_SS4_IMAGE_HU, "" },  /* fuzzy */
/* it */ { IDS_ATTACH_SS4_IMAGE_IT, "Seleziona immagine cartuccia Super Snapshot V4 di 32KB" },
/* ko */ { IDS_ATTACH_SS4_IMAGE_KO, "" },  /* fuzzy */
/* nl */ { IDS_ATTACH_SS4_IMAGE_NL, "Koppel binair 32KB Super Snapshot V4 cartridge bestand" },
/* pl */ { IDS_ATTACH_SS4_IMAGE_PL, "Zamontuj surowy obraz kartrid¿a 32KB Super Snapshot V4" },
/* ru */ { IDS_ATTACH_SS4_IMAGE_RU, "Attach raw 32KB Super Snapshot V4 cartridge image" },
/* sv */ { IDS_ATTACH_SS4_IMAGE_SV, "Anslut rå 32KB Super Snapshot V4-insticksmodulfil" },
/* tr */ { IDS_ATTACH_SS4_IMAGE_TR, "Düz 32KB Super Snapshot V4 kartuþ imajý yerleþtir" },

/* en */ { IDS_ATTACH_SS5_IMAGE,    "Attach raw 64KB Super Snapshot V5 cartridge image" },
/* da */ { IDS_ATTACH_SS5_IMAGE_DA, "Tilslut råt 64kB Super Snapshot V5 cartridge image" },
/* de */ { IDS_ATTACH_SS5_IMAGE_DE, "64kB Super Snapshot V5 Image (raw) einlegen" },
/* es */ { IDS_ATTACH_SS5_IMAGE_ES, "Insertar imagen cartucho Super Snapshot V5 64KB" },
/* fr */ { IDS_ATTACH_SS5_IMAGE_FR, "Attacher une image de cartouche 64 ko Super Snapshot V5" },
/* hu */ { IDS_ATTACH_SS5_IMAGE_HU, "" },  /* fuzzy */
/* it */ { IDS_ATTACH_SS5_IMAGE_IT, "Seleziona immagine cartuccia Super Snapshot V5 di 64KB" },
/* ko */ { IDS_ATTACH_SS5_IMAGE_KO, "64KB Super Snapshot V5 Ä«Æ®¸®Áö ÀÌ¹ÌÁö¸¦ ºÙÀÌ¼¼¿ä" },
/* nl */ { IDS_ATTACH_SS5_IMAGE_NL, "Koppel binair 64KB Super Snapshot V5 cartridge bestand" },
/* pl */ { IDS_ATTACH_SS5_IMAGE_PL, "Zamontuj surowy obraz kartrid¿a Super Snapshot V5" },
/* ru */ { IDS_ATTACH_SS5_IMAGE_RU, "Attach raw 64KB Super Snapshot V5 cartridge image" },
/* sv */ { IDS_ATTACH_SS5_IMAGE_SV, "Anslut rå 64KB-Super Snapshot V5-insticksmodulfil" },
/* tr */ { IDS_ATTACH_SS5_IMAGE_TR, "Düz 64KB Super Snapshot V5 kartuþ imajý yerleþtir" },

/* en */ { IDS_ATTACH_SB_IMAGE,    "Attach raw Structured Basic cartridge image" },
/* da */ { IDS_ATTACH_SB_IMAGE_DA, "Tilslut rå Strucured BASIC-cartridge image" },
/* de */ { IDS_ATTACH_SB_IMAGE_DE, "Erweiterungsmodul Structured Basic (raw) Image einlegen" },
/* es */ { IDS_ATTACH_SB_IMAGE_ES, "Insertar imagen cartucho Structured Basic" },
/* fr */ { IDS_ATTACH_SB_IMAGE_FR, "Insérer une cartouche Raw Structured Basic" },
/* hu */ { IDS_ATTACH_SB_IMAGE_HU, "Structured Basic cartridge képmás csatolása" },
/* it */ { IDS_ATTACH_SB_IMAGE_IT, "Seleziona immagine cartuccia Structured Basic" },
/* ko */ { IDS_ATTACH_SB_IMAGE_KO, "" },  /* fuzzy */
/* nl */ { IDS_ATTACH_SB_IMAGE_NL, "Koppel binair Structured Basic cartridge bestand" },
/* pl */ { IDS_ATTACH_SB_IMAGE_PL, "Zamontuj surowy obraz kartrid¿a Structured Basic" },
/* ru */ { IDS_ATTACH_SB_IMAGE_RU, "Attach raw Structured Basic cartridge image" },
/* sv */ { IDS_ATTACH_SB_IMAGE_SV, "Anslut rå Strucured Basic-insticksmodulfil" },
/* tr */ { IDS_ATTACH_SB_IMAGE_TR, "Düz yapýlandýrýlmýþ Basic kartuþ imajý yerleþtir" },

/* en */ { IDS_AVAILABLE_CMDLINE_OPTIONS,    "\nAvailable command-line options:\n\n" },
/* da */ { IDS_AVAILABLE_CMDLINE_OPTIONS_DA, "\nMulige kommandolinjeparametre:\n\n" },
/* de */ { IDS_AVAILABLE_CMDLINE_OPTIONS_DE, "\nZulässige Kommandozeilen Optionen:\n\n" },
/* es */ { IDS_AVAILABLE_CMDLINE_OPTIONS_ES, "\nOpciones de comando disponibles:\n\n" },
/* fr */ { IDS_AVAILABLE_CMDLINE_OPTIONS_FR, "\nOptions de ligne de commande disponibles:\n\n" },
/* hu */ { IDS_AVAILABLE_CMDLINE_OPTIONS_HU, "\nLehetséges parancssori opciók:\n\n" },
/* it */ { IDS_AVAILABLE_CMDLINE_OPTIONS_IT, "\nOpzioni disponibili da riga di comando:\n\n" },
/* ko */ { IDS_AVAILABLE_CMDLINE_OPTIONS_KO, "\n»ç¿ë°¡´ÉÇÑ Ä¿¸Çµå ¶óÀÎ ¿É¼Ç:\n\n" },
/* nl */ { IDS_AVAILABLE_CMDLINE_OPTIONS_NL, "\nBeschikbare opdrachtregel opties:\n\n" },
/* pl */ { IDS_AVAILABLE_CMDLINE_OPTIONS_PL, "\nDostêpne opcje wiersza poleceñ:\n\n" },
/* ru */ { IDS_AVAILABLE_CMDLINE_OPTIONS_RU, "\n²Þ×ÜÞÖÝëÕ ÞßæØØ ÚÞÜÐÝÔÝÞÙ áâàÞÚØ:\n\n" },
/* sv */ { IDS_AVAILABLE_CMDLINE_OPTIONS_SV, "\nTillgängliga kommandoradsflaggor:\n\n" },
/* tr */ { IDS_AVAILABLE_CMDLINE_OPTIONS_TR, "\nMevcut komut satýrý seçenekleri:\n\n" },

/* en */ { IDS_ATTACH_4_8_16KB_AT_2000,    "Attach 4/8/16KB cartridge image at $2000" },
/* da */ { IDS_ATTACH_4_8_16KB_AT_2000_DA, "Tilslut 4/8/16kB-cartridge image på $2000" },
/* de */ { IDS_ATTACH_4_8_16KB_AT_2000_DE, "4/8/16KB Modul Image bei $2000 einlegen" },
/* es */ { IDS_ATTACH_4_8_16KB_AT_2000_ES, "Insertar 4/8/16KB imagen cartucho en $2000" },
/* fr */ { IDS_ATTACH_4_8_16KB_AT_2000_FR, "Insertion d'une image 4/8/16 ko à $2000" },
/* hu */ { IDS_ATTACH_4_8_16KB_AT_2000_HU, "4/8/16KB képmás csatolása $2000 címre" },
/* it */ { IDS_ATTACH_4_8_16KB_AT_2000_IT, "Seleziona immagine cartuccia di 4/8/16KB a $2000" },
/* ko */ { IDS_ATTACH_4_8_16KB_AT_2000_KO, "4/8/16KB Ä«Æ®¸®Áö ÀÌ¹ÌÁö ¸¦  $2000 ¿¡ ºÙÀÌ±â" },
/* nl */ { IDS_ATTACH_4_8_16KB_AT_2000_NL, "Koppel 4/8/16KB bestand op $2000" },
/* pl */ { IDS_ATTACH_4_8_16KB_AT_2000_PL, "Wsuñ 4/8/16KB obraz w $2000" },
/* ru */ { IDS_ATTACH_4_8_16KB_AT_2000_RU, "Attach 4/8/16KB cartridge image at $2000" },
/* sv */ { IDS_ATTACH_4_8_16KB_AT_2000_SV, "Anslut 4/8/16KB-modulfil vid $2000" },
/* tr */ { IDS_ATTACH_4_8_16KB_AT_2000_TR, "$2000 adresine 4/8/16KB kartuþ imajý yerleþtir" },

/* en */ { IDS_ATTACH_4_8_16KB_AT_4000,    "Attach 4/8/16KB cartridge image at $4000" },
/* da */ { IDS_ATTACH_4_8_16KB_AT_4000_DA, "Tilslut 4/8/16kB-cartridge image på $4000" },
/* de */ { IDS_ATTACH_4_8_16KB_AT_4000_DE, "4/8/16KB Modul Image bei $4000 einlegen" },
/* es */ { IDS_ATTACH_4_8_16KB_AT_4000_ES, "Insertar 4/8/16KB imagen cartucho en $4000" },
/* fr */ { IDS_ATTACH_4_8_16KB_AT_4000_FR, "Insertion d'une image 4/8/16 ko à $4000" },
/* hu */ { IDS_ATTACH_4_8_16KB_AT_4000_HU, "4/8/16KB képmás csatolása $4000 címre" },
/* it */ { IDS_ATTACH_4_8_16KB_AT_4000_IT, "Seleziona immagine cartuccia di 4/8/16KB a $4000" },
/* ko */ { IDS_ATTACH_4_8_16KB_AT_4000_KO, "4/8/16KB Ä«Æ®¸®Áö ÀÌ¹ÌÁö ¸¦  $4000 ¿¡ ºÙÀÌ±â" },
/* nl */ { IDS_ATTACH_4_8_16KB_AT_4000_NL, "Koppel 4/8/16KB bestand op $4000" },
/* pl */ { IDS_ATTACH_4_8_16KB_AT_4000_PL, "Wsuñ 4/8/16KB obraz w $4000" },
/* ru */ { IDS_ATTACH_4_8_16KB_AT_4000_RU, "Attach 4/8/16KB cartridge image at $4000" },
/* sv */ { IDS_ATTACH_4_8_16KB_AT_4000_SV, "Anslut 4/8/16KB-modulfil vid $4000" },
/* tr */ { IDS_ATTACH_4_8_16KB_AT_4000_TR, "$4000 adresine 4/8/16KB kartuþ imajý yerleþtir" },

/* en */ { IDS_ATTACH_4_8_16KB_AT_6000,    "Attach 4/8/16KB cartridge image at $6000" },
/* da */ { IDS_ATTACH_4_8_16KB_AT_6000_DA, "Tilslut 4/8/16kB-cartridge image på $6000" },
/* de */ { IDS_ATTACH_4_8_16KB_AT_6000_DE, "4/8/16KB Modul Image bei $6000 einlegen" },
/* es */ { IDS_ATTACH_4_8_16KB_AT_6000_ES, "Insertar 4/8/16KB imagen cartucho en $6000" },
/* fr */ { IDS_ATTACH_4_8_16KB_AT_6000_FR, "Insertion d'une image 4/8/16 ko à $6000" },
/* hu */ { IDS_ATTACH_4_8_16KB_AT_6000_HU, "4/8/16KB képmás csatolása $6000 címre" },
/* it */ { IDS_ATTACH_4_8_16KB_AT_6000_IT, "Seleziona immagine cartuccia di 4/8/16KB a $6000" },
/* ko */ { IDS_ATTACH_4_8_16KB_AT_6000_KO, "4/8/16KB Ä«Æ®¸®Áö ÀÌ¹ÌÁö ¸¦  $6000 ¿¡ ºÙÀÌ±â" },
/* nl */ { IDS_ATTACH_4_8_16KB_AT_6000_NL, "Koppel 4/8/16KB bestand op $6000" },
/* pl */ { IDS_ATTACH_4_8_16KB_AT_6000_PL, "Wsuñ 4/8/16KB obraz w $6000" },
/* ru */ { IDS_ATTACH_4_8_16KB_AT_6000_RU, "Attach 4/8/16KB cartridge image at $6000" },
/* sv */ { IDS_ATTACH_4_8_16KB_AT_6000_SV, "Anslut 4/8/16KB-modulfil vid $6000" },
/* tr */ { IDS_ATTACH_4_8_16KB_AT_6000_TR, "$6000 adresine 4/8/16KB kartuþ imajý yerleþtir" },

/* en */ { IDS_ATTACH_4_8KB_AT_A000,    "Attach 4/8KB cartridge image at $A000" },
/* da */ { IDS_ATTACH_4_8KB_AT_A000_DA, "Tilslut 4/8kB-cartridge image på $A000" },
/* de */ { IDS_ATTACH_4_8KB_AT_A000_DE, "4/8KB Modul Image bei $A000 einlegen" },
/* es */ { IDS_ATTACH_4_8KB_AT_A000_ES, "Insertar 4/8KB imagen cartucho en $A000" },
/* fr */ { IDS_ATTACH_4_8KB_AT_A000_FR, "Insertion d'une image 4/8 ko à $A000" },
/* hu */ { IDS_ATTACH_4_8KB_AT_A000_HU, "4/8KB képmás csatolása $A000 címre" },
/* it */ { IDS_ATTACH_4_8KB_AT_A000_IT, "Seleziona immagine cartuccia di 4/8KB a $A000" },
/* ko */ { IDS_ATTACH_4_8KB_AT_A000_KO, "4/8KB Ä«Æ®¸®Áö ÀÌ¹ÌÁö ¸¦  $A000 ¿¡ ºÙÀÌ±â" },
/* nl */ { IDS_ATTACH_4_8KB_AT_A000_NL, "Koppel 4/8KB bestand op $A000" },
/* pl */ { IDS_ATTACH_4_8KB_AT_A000_PL, "Wsuñ 4/8KB obraz w $A000" },
/* ru */ { IDS_ATTACH_4_8KB_AT_A000_RU, "Attach 4/8KB cartridge image at $A000" },
/* sv */ { IDS_ATTACH_4_8KB_AT_A000_SV, "Anslut 4/8KB-modulfil vid $A000" },
/* tr */ { IDS_ATTACH_4_8KB_AT_A000_TR, "$A000 adresine 4/8KB kartuþ imajý yerleþtir" },

/* en */ { IDS_ATTACH_4KB_AT_B000,    "Attach 4KB cartridge image at $B000" },
/* da */ { IDS_ATTACH_4KB_AT_B000_DA, "Tilslut 4kB-cartridge image på $B000" },
/* de */ { IDS_ATTACH_4KB_AT_B000_DE, "4KB Modul Image bei $B000 einlegen" },
/* es */ { IDS_ATTACH_4KB_AT_B000_ES, "Insertar 4KB imagen cartucho en $B000" },
/* fr */ { IDS_ATTACH_4KB_AT_B000_FR, "Insertion d'une image 4 ko à $B000" },
/* hu */ { IDS_ATTACH_4KB_AT_B000_HU, "4KB képmás csatolása $B000 címre" },
/* it */ { IDS_ATTACH_4KB_AT_B000_IT, "Seleziona immagine cartuccia di 4KB a $B000" },
/* ko */ { IDS_ATTACH_4KB_AT_B000_KO, "4KB Ä«Æ®¸®Áö ÀÌ¹ÌÁö ¸¦  $B000 ¿¡ ºÙÀÌ±â" },
/* nl */ { IDS_ATTACH_4KB_AT_B000_NL, "Koppel 4KB bestand op $B000" },
/* pl */ { IDS_ATTACH_4KB_AT_B000_PL, "Wsuñ 4KB obraz w $B000" },
/* ru */ { IDS_ATTACH_4KB_AT_B000_RU, "Attach 4KB cartridge image at $B000" },
/* sv */ { IDS_ATTACH_4KB_AT_B000_SV, "Anslut 4KB-modulfil vid $B000" },
/* tr */ { IDS_ATTACH_4KB_AT_B000_TR, "$B000 adresine 4KB kartuþ imajý yerleþtir" },

/* en */ { IDS_S_AT_D_SPEED,    "%s at %d%% speed, %d fps%s" },
/* da */ { IDS_S_AT_D_SPEED_DA, "%s med %d%% hastighed, %d b/s%s" },
/* de */ { IDS_S_AT_D_SPEED_DE, "%s bei %d%% Geschwindigkeit, %d fps%s" },
/* es */ { IDS_S_AT_D_SPEED_ES, "%s a %d%% velocidad, %d fps%s" },
/* fr */ { IDS_S_AT_D_SPEED_FR, "%s à %d%% de vitesse et %d ips%s" },
/* hu */ { IDS_S_AT_D_SPEED_HU, "%s %d%% sebességen, %d fps%s" },
/* it */ { IDS_S_AT_D_SPEED_IT, "velocità %s al %d%%, %d fps%s" },
/* ko */ { IDS_S_AT_D_SPEED_KO, "%s ¸¦ %d%% ½ºÇÇµå, %d fps%s" },
/* nl */ { IDS_S_AT_D_SPEED_NL, "%s met %d%% snelheid, %d fps%s" },
/* pl */ { IDS_S_AT_D_SPEED_PL, "%s, prêdko¶æ %d%%, %d fps%s" },
/* ru */ { IDS_S_AT_D_SPEED_RU, "%s at %d%% speed, %d fps%s" },
/* sv */ { IDS_S_AT_D_SPEED_SV, "%s med %d%% hastighet, %d b/s%s" },
/* tr */ { IDS_S_AT_D_SPEED_TR, "%s %d%% hýzýnda, %d fps%s" },

/* en */ { IDS_ATTACH,    "Attach" },
/* da */ { IDS_ATTACH_DA, "Tilslut" },
/* de */ { IDS_ATTACH_DE, "Einlegen" },
/* es */ { IDS_ATTACH_ES, "Insertar" },
/* fr */ { IDS_ATTACH_FR, "Insérer" },
/* hu */ { IDS_ATTACH_HU, "Csatolás" },
/* it */ { IDS_ATTACH_IT, "Seleziona" },
/* ko */ { IDS_ATTACH_KO, "ºÙÀÌ´Ù" },
/* nl */ { IDS_ATTACH_NL, "Koppel" },
/* pl */ { IDS_ATTACH_PL, "Zamontuj" },
/* ru */ { IDS_ATTACH_RU, "Attach" },
/* sv */ { IDS_ATTACH_SV, "Anslut" },
/* tr */ { IDS_ATTACH_TR, "Yerleþtir" },

/* en */ { IDS_PARENT,    "Parent" },
/* da */ { IDS_PARENT_DA, "Forælder" },
/* de */ { IDS_PARENT_DE, "Vorgänger" },
/* es */ { IDS_PARENT_ES, "Padre" },
/* fr */ { IDS_PARENT_FR, "Parent" },
/* hu */ { IDS_PARENT_HU, "Szülõ" },
/* it */ { IDS_PARENT_IT, "Su" },
/* ko */ { IDS_PARENT_KO, "ºÎ¸ð" },
/* nl */ { IDS_PARENT_NL, "Bevenliggende directory" },
/* pl */ { IDS_PARENT_PL, "Nadrzêdny" },
/* ru */ { IDS_PARENT_RU, "Parent" },
/* sv */ { IDS_PARENT_SV, "Förälder" },
/* tr */ { IDS_PARENT_TR, "Üst Öðe" },

/* en */ { IDS_ATTACH_READ_ONLY,    "Attach read only" },
/* da */ { IDS_ATTACH_READ_ONLY_DA, "Tilslut skrivebeskyttet" },
/* de */ { IDS_ATTACH_READ_ONLY_DE, "Schreibgeschützt einlegen" },
/* es */ { IDS_ATTACH_READ_ONLY_ES, "Inserta sólo lectura" },
/* fr */ { IDS_ATTACH_READ_ONLY_FR, "Insérer en lecture seule" },
/* hu */ { IDS_ATTACH_READ_ONLY_HU, "Csatolás csak olvasható módon" },
/* it */ { IDS_ATTACH_READ_ONLY_IT, "Seleziona in sola lettura" },
/* ko */ { IDS_ATTACH_READ_ONLY_KO, "" },  /* fuzzy */
/* nl */ { IDS_ATTACH_READ_ONLY_NL, "Koppel alleen lezen" },
/* pl */ { IDS_ATTACH_READ_ONLY_PL, "Zamontuj tylko do odczytu" },
/* ru */ { IDS_ATTACH_READ_ONLY_RU, "Attach read only" },
/* sv */ { IDS_ATTACH_READ_ONLY_SV, "Anslut skrivskyddat" },
/* tr */ { IDS_ATTACH_READ_ONLY_TR, "Salt okunur yerleþtir" },

/* en */ { IDS_NAME,    "Name" },
/* da */ { IDS_NAME_DA, "Navn" },
/* de */ { IDS_NAME_DE, "Name" },
/* es */ { IDS_NAME_ES, "Nombre" },
/* fr */ { IDS_NAME_FR, "Nom" },
/* hu */ { IDS_NAME_HU, "Név" },
/* it */ { IDS_NAME_IT, "Nome" },
/* ko */ { IDS_NAME_KO, "ÀÌ¸§" },
/* nl */ { IDS_NAME_NL, "Naam" },
/* pl */ { IDS_NAME_PL, "Nazwa" },
/* ru */ { IDS_NAME_RU, "¸Üï" },
/* sv */ { IDS_NAME_SV, "Namn" },
/* tr */ { IDS_NAME_TR, "Ýsim" },

/* en */ { IDS_CREATE_IMAGE,    "Create Image" },
/* da */ { IDS_CREATE_IMAGE_DA, "Opret image" },
/* de */ { IDS_CREATE_IMAGE_DE, "Imagedatei erzeugen" },
/* es */ { IDS_CREATE_IMAGE_ES, "Crear imagen" },
/* fr */ { IDS_CREATE_IMAGE_FR, "Création d'image" },
/* hu */ { IDS_CREATE_IMAGE_HU, "Képmás létrehozása" },
/* it */ { IDS_CREATE_IMAGE_IT, "Crea immagine" },
/* ko */ { IDS_CREATE_IMAGE_KO, "ÀÌ¹ÌÁö »ý¼º" },
/* nl */ { IDS_CREATE_IMAGE_NL, "Maak Bestand" },
/* pl */ { IDS_CREATE_IMAGE_PL, "Utwórz obraz" },
/* ru */ { IDS_CREATE_IMAGE_RU, "Create Image" },
/* sv */ { IDS_CREATE_IMAGE_SV, "Skapa avbildning" },
/* tr */ { IDS_CREATE_IMAGE_TR, "Imaj Yarat" },

/* en */ { IDS_NEW_IMAGE,    "New Image" },
/* da */ { IDS_NEW_IMAGE_DA, "Nyt image" },
/* de */ { IDS_NEW_IMAGE_DE, "Neue Imagedatei" },
/* es */ { IDS_NEW_IMAGE_ES, "Nueva imagen" },
/* fr */ { IDS_NEW_IMAGE_FR, "Nouvelle image" },
/* hu */ { IDS_NEW_IMAGE_HU, "Új képmás" },
/* it */ { IDS_NEW_IMAGE_IT, "Nuova immagine" },
/* ko */ { IDS_NEW_IMAGE_KO, "¼¼·Î¿î ÀÌ¹ÌÁö" },
/* nl */ { IDS_NEW_IMAGE_NL, "Nieuw Bestand" },
/* pl */ { IDS_NEW_IMAGE_PL, "Nowy obraz" },
/* ru */ { IDS_NEW_IMAGE_RU, "New Image" },
/* sv */ { IDS_NEW_IMAGE_SV, "Ny avbildning" },
/* tr */ { IDS_NEW_IMAGE_TR, "Yeni Ýmaj" },

/* en */ { IDS_NEW_TAP_IMAGE,    "New TAP Image" },
/* da */ { IDS_NEW_TAP_IMAGE_DA, "Nyt TAP-image" },
/* de */ { IDS_NEW_TAP_IMAGE_DE, "Neue TAP Imagedatei" },
/* es */ { IDS_NEW_TAP_IMAGE_ES, "Nueva imagen TAP" },
/* fr */ { IDS_NEW_TAP_IMAGE_FR, "Nouvelle image TAP" },
/* hu */ { IDS_NEW_TAP_IMAGE_HU, "Új TAP képmás" },
/* it */ { IDS_NEW_TAP_IMAGE_IT, "Nuova immagine TAP" },
/* ko */ { IDS_NEW_TAP_IMAGE_KO, "¼¼·Î¿î TAP ÀÌ¹ÌÁö" },
/* nl */ { IDS_NEW_TAP_IMAGE_NL, "Nieuw TAP bestand" },
/* pl */ { IDS_NEW_TAP_IMAGE_PL, "Obraz New TAP" },
/* ru */ { IDS_NEW_TAP_IMAGE_RU, "New TAP Image" },
/* sv */ { IDS_NEW_TAP_IMAGE_SV, "Ny TAP-avbildning" },
/* tr */ { IDS_NEW_TAP_IMAGE_TR, "Yeni TAP Imajý" },

/* en */ { IDS_OVERWRITE_EXISTING_IMAGE,    "Overwrite existing image?" },
/* da */ { IDS_OVERWRITE_EXISTING_IMAGE_DA, "Overskriv eksisterende image?" },
/* de */ { IDS_OVERWRITE_EXISTING_IMAGE_DE, "Existierende Datei überschreiben?" },
/* es */ { IDS_OVERWRITE_EXISTING_IMAGE_ES, "¿Sobrescribo la imagen actual?" },
/* fr */ { IDS_OVERWRITE_EXISTING_IMAGE_FR, "Écraser les images existantes?" },
/* hu */ { IDS_OVERWRITE_EXISTING_IMAGE_HU, "Felülírjam a meglévõ képmást?" },
/* it */ { IDS_OVERWRITE_EXISTING_IMAGE_IT, "Sovrascrivo l'immagine esistente?" },
/* ko */ { IDS_OVERWRITE_EXISTING_IMAGE_KO, "ÇöÁ¦ ÀÌ¹ÌÁö¸¦ µ¤¾î ¾²±â ÇÒ±î¿ä?" },
/* nl */ { IDS_OVERWRITE_EXISTING_IMAGE_NL, "Bestaand bestand overschrijven?" },
/* pl */ { IDS_OVERWRITE_EXISTING_IMAGE_PL, "Nadpisaæ istniej±cy obraz?" },
/* ru */ { IDS_OVERWRITE_EXISTING_IMAGE_RU, "¿ÕàÕßØáÐâì ØÜÕîéØÙáï ÞÑàÐ×?" },
/* sv */ { IDS_OVERWRITE_EXISTING_IMAGE_SV, "Skriva över existerande avbildning?" },
/* tr */ { IDS_OVERWRITE_EXISTING_IMAGE_TR, "Varolan imajýn üzerine yazýlsýn mý?" },

/* en */ { IDS_ABOUT,    "About VICE" },
/* da */ { IDS_ABOUT_DA, "Om VICE" },
/* de */ { IDS_ABOUT_DE, "Über VICE" },
/* es */ { IDS_ABOUT_ES, "Acerca de VICE" },
/* fr */ { IDS_ABOUT_FR, "À propos de VICE" },
/* hu */ { IDS_ABOUT_HU, "A VICE névjegye" },
/* it */ { IDS_ABOUT_IT, "Informazioni su VICE" },
/* ko */ { IDS_ABOUT_KO, "µµ¿ò VICE" },
/* nl */ { IDS_ABOUT_NL, "Over VICE" },
/* pl */ { IDS_ABOUT_PL, "O VICE" },
/* ru */ { IDS_ABOUT_RU, "¿ÞÔàÞÑÝÕÕ Þ VICE" },
/* sv */ { IDS_ABOUT_SV, "Om VICE" },
/* tr */ { IDS_ABOUT_TR, "VICE Hakkýnda" },

/* en */ { IDS_IMAGE_CONTENTS,    "Image Contents" },
/* da */ { IDS_IMAGE_CONTENTS_DA, "Indhold i image" },
/* de */ { IDS_IMAGE_CONTENTS_DE, "Imagedatei Inhalt" },
/* es */ { IDS_IMAGE_CONTENTS_ES, "Contenidos imagen" },
/* fr */ { IDS_IMAGE_CONTENTS_FR, "Contenu de l'image" },
/* hu */ { IDS_IMAGE_CONTENTS_HU, "Tartalom" },
/* it */ { IDS_IMAGE_CONTENTS_IT, "Contenuti immagine" },
/* ko */ { IDS_IMAGE_CONTENTS_KO, "ÀÌ¹ÌÁö ÄÜÅÙÃ÷" },
/* nl */ { IDS_IMAGE_CONTENTS_NL, "Bestandsinhoud" },
/* pl */ { IDS_IMAGE_CONTENTS_PL, "Zawarto¶æ obrazu" },
/* ru */ { IDS_IMAGE_CONTENTS_RU, "Image Contents" },
/* sv */ { IDS_IMAGE_CONTENTS_SV, "Innehåll i avbildning" },
/* tr */ { IDS_IMAGE_CONTENTS_TR, "Imaj Ýçeriði" },

/* en */ { IDMS_DISABLED,    "Disabled" },
/* da */ { IDMS_DISABLED_DA, "Deaktiveret" },
/* de */ { IDMS_DISABLED_DE, "Deaktiviert" },
/* es */ { IDMS_DISABLED_ES, "Deshabilitado" },
/* fr */ { IDMS_DISABLED_FR, "Désactivé" },
/* hu */ { IDMS_DISABLED_HU, "Tiltva" },
/* it */ { IDMS_DISABLED_IT, "Disattivo" },
/* ko */ { IDMS_DISABLED_KO, "ÀÛµ¿ÇÏÁö¾ÊÀ½" },
/* nl */ { IDMS_DISABLED_NL, "Uitgeschakeld" },
/* pl */ { IDMS_DISABLED_PL, "Wy³±cz" },
/* ru */ { IDMS_DISABLED_RU, "Disabled" },
/* sv */ { IDMS_DISABLED_SV, "Inaktiverad" },
/* tr */ { IDMS_DISABLED_TR, "Pasif" },

/* en */ { IDS_ENABLED,    "Enabled" },
/* da */ { IDS_ENABLED_DA, "Aktiveret" },
/* de */ { IDS_ENABLED_DE, "Aktiviert" },
/* es */ { IDS_ENABLED_ES, "Habilitado" },
/* fr */ { IDS_ENABLED_FR, "Activé" },
/* hu */ { IDS_ENABLED_HU, "Engedélyezve" },
/* it */ { IDS_ENABLED_IT, "Attivato" },
/* ko */ { IDS_ENABLED_KO, "ÀÛµ¿" },
/* nl */ { IDS_ENABLED_NL, "Ingeschakeld" },
/* pl */ { IDS_ENABLED_PL, "W³±cz" },
/* ru */ { IDS_ENABLED_RU, "Enabled" },
/* sv */ { IDS_ENABLED_SV, "Aktiverad" },
/* tr */ { IDS_ENABLED_TR, "Aktif" },

/* en */ { IDS_RS232_DEVICE_1,    "RS232 device 1" },
/* da */ { IDS_RS232_DEVICE_1_DA, "RS232-enhed 1" },
/* de */ { IDS_RS232_DEVICE_1_DE, "RS232 Gerät 1" },
/* es */ { IDS_RS232_DEVICE_1_ES, "Periférico RS232 1" },
/* fr */ { IDS_RS232_DEVICE_1_FR, "1er Périphérique RS232" },
/* hu */ { IDS_RS232_DEVICE_1_HU, "RS232 1-es eszköz" },
/* it */ { IDS_RS232_DEVICE_1_IT, "Dispositivo RS232 #1" },
/* ko */ { IDS_RS232_DEVICE_1_KO, "RS232 µð¹ÙÀÌ½º 1" },
/* nl */ { IDS_RS232_DEVICE_1_NL, "RS232 apparaat 1" },
/* pl */ { IDS_RS232_DEVICE_1_PL, "Urz±dzenie RS232 1" },
/* ru */ { IDS_RS232_DEVICE_1_RU, "RS232 device 1" },
/* sv */ { IDS_RS232_DEVICE_1_SV, "RS232-enhet 1" },
/* tr */ { IDS_RS232_DEVICE_1_TR, "RS232 aygýt 1" },

/* en */ { IDS_RS232_DEVICE_2,    "RS232 device 2" },
/* da */ { IDS_RS232_DEVICE_2_DA, "RS232-enhed 2" },
/* de */ { IDS_RS232_DEVICE_2_DE, "RS232 Gerät 2" },
/* es */ { IDS_RS232_DEVICE_2_ES, "Periférico RS232 2" },
/* fr */ { IDS_RS232_DEVICE_2_FR, "2e Périphérique RS232" },
/* hu */ { IDS_RS232_DEVICE_2_HU, "RS232 2-es eszköz" },
/* it */ { IDS_RS232_DEVICE_2_IT, "Dispositivo RS232 #2" },
/* ko */ { IDS_RS232_DEVICE_2_KO, "RS232 µð¹ÙÀÌ½º 2" },
/* nl */ { IDS_RS232_DEVICE_2_NL, "RS232 apparaat 2" },
/* pl */ { IDS_RS232_DEVICE_2_PL, "Urz±dzenie RS232 2" },
/* ru */ { IDS_RS232_DEVICE_2_RU, "RS232 device 2" },
/* sv */ { IDS_RS232_DEVICE_2_SV, "RS232-enhet 2" },
/* tr */ { IDS_RS232_DEVICE_2_TR, "RS232 aygýt 2" },

/* en */ { IDS_RS232_DEVICE_3,    "RS232 device 3" },
/* da */ { IDS_RS232_DEVICE_3_DA, "RS232-enhed 3" },
/* de */ { IDS_RS232_DEVICE_3_DE, "RS232 Gerät 3" },
/* es */ { IDS_RS232_DEVICE_3_ES, "Periférico RS232 3" },
/* fr */ { IDS_RS232_DEVICE_3_FR, "3e Périphérique RS232" },
/* hu */ { IDS_RS232_DEVICE_3_HU, "RS232 3-as eszköz" },
/* it */ { IDS_RS232_DEVICE_3_IT, "Dispositivo RS232 #3" },
/* ko */ { IDS_RS232_DEVICE_3_KO, "RS232 µð¹ÙÀÌ½º 3" },
/* nl */ { IDS_RS232_DEVICE_3_NL, "RS232 apparaat 3" },
/* pl */ { IDS_RS232_DEVICE_3_PL, "Urz±dzenie RS232 3" },
/* ru */ { IDS_RS232_DEVICE_3_RU, "RS232 device 3" },
/* sv */ { IDS_RS232_DEVICE_3_SV, "RS232-enhet 3" },
/* tr */ { IDS_RS232_DEVICE_3_TR, "RS232 aygýt 3" },

/* en */ { IDS_RS232_DEVICE_4,    "RS232 device 4" },
/* da */ { IDS_RS232_DEVICE_4_DA, "RS232-enhed 4" },
/* de */ { IDS_RS232_DEVICE_4_DE, "RS232 Gerät 4" },
/* es */ { IDS_RS232_DEVICE_4_ES, "Periférico RS232 4" },
/* fr */ { IDS_RS232_DEVICE_4_FR, "4e Périphérique RS232" },
/* hu */ { IDS_RS232_DEVICE_4_HU, "RS232 4-es eszköz" },
/* it */ { IDS_RS232_DEVICE_4_IT, "Dispositivo RS232 #4" },
/* ko */ { IDS_RS232_DEVICE_4_KO, "RS232 µð¹ÙÀÌ½º 4" },
/* nl */ { IDS_RS232_DEVICE_4_NL, "RS232 apparaat 4" },
/* pl */ { IDS_RS232_DEVICE_4_PL, "Urz±dzenie RS232 4" },
/* ru */ { IDS_RS232_DEVICE_4_RU, "RS232 device 4" },
/* sv */ { IDS_RS232_DEVICE_4_SV, "RS232-enhet 4" },
/* tr */ { IDS_RS232_DEVICE_4_TR, "RS232 aygýt 4" },

/* en */ { IDS_NONE,    "None" },
/* da */ { IDS_NONE_DA, "Ingen" },
/* de */ { IDS_NONE_DE, "Kein" },
/* es */ { IDS_NONE_ES, "No" },
/* fr */ { IDS_NONE_FR, "Aucun" },
/* hu */ { IDS_NONE_HU, "Nincs" },
/* it */ { IDS_NONE_IT, "Nessuno" },
/* ko */ { IDS_NONE_KO, "¾øÀ½" },
/* nl */ { IDS_NONE_NL, "Geen" },
/* pl */ { IDS_NONE_PL, "Brak" },
/* ru */ { IDS_NONE_RU, "None" },
/* sv */ { IDS_NONE_SV, "Ingen" },
/* tr */ { IDS_NONE_TR, "Yok" },

/* en */ { IDS_STANDARD,    "Standard" },
/* da */ { IDS_STANDARD_DA, "Standard" },
/* de */ { IDS_STANDARD_DE, "Standard" },
/* es */ { IDS_STANDARD_ES, "Estandar" },
/* fr */ { IDS_STANDARD_FR, "Standard" },
/* hu */ { IDS_STANDARD_HU, "" },  /* fuzzy */
/* it */ { IDS_STANDARD_IT, "Standard" },
/* ko */ { IDS_STANDARD_KO, "" },  /* fuzzy */
/* nl */ { IDS_STANDARD_NL, "Standaard" },
/* pl */ { IDS_STANDARD_PL, "Standard" },
/* ru */ { IDS_STANDARD_RU, "" },  /* fuzzy */
/* sv */ { IDS_STANDARD_SV, "Standard" },
/* tr */ { IDS_STANDARD_TR, "Standart" },

/* en */ { IDS_DD3,    "Dolphin Dos 3" },
/* da */ { IDS_DD3_DA, "Dolphin DOS 3" },
/* de */ { IDS_DD3_DE, "Dolphin Dos 3" },
/* es */ { IDS_DD3_ES, "Dolphin DOS 3" },
/* fr */ { IDS_DD3_FR, "Dolphin DOS 3" },
/* hu */ { IDS_DD3_HU, "" },  /* fuzzy */
/* it */ { IDS_DD3_IT, "Dolphin Dos 3" },
/* ko */ { IDS_DD3_KO, "" },  /* fuzzy */
/* nl */ { IDS_DD3_NL, "Dolphin DOS 3" },
/* pl */ { IDS_DD3_PL, "Dolphin Dos 3" },
/* ru */ { IDS_DD3_RU, "" },  /* fuzzy */
/* sv */ { IDS_DD3_SV, "Dolphin Dos 3" },
/* tr */ { IDS_DD3_TR, "Dolphin Dos 3" },

/* en */ { IDS_FORMEL64,    "Formel64" },
/* da */ { IDS_FORMEL64_DA, "Formel64" },
/* de */ { IDS_FORMEL64_DE, "Formel64" },
/* es */ { IDS_FORMEL64_ES, "Formel 64" },
/* fr */ { IDS_FORMEL64_FR, "Formel64" },
/* hu */ { IDS_FORMEL64_HU, "" },  /* fuzzy */
/* it */ { IDS_FORMEL64_IT, "Formel64" },
/* ko */ { IDS_FORMEL64_KO, "" },  /* fuzzy */
/* nl */ { IDS_FORMEL64_NL, "Formel64" },
/* pl */ { IDS_FORMEL64_PL, "Formel64" },
/* ru */ { IDS_FORMEL64_RU, "Formel64" },
/* sv */ { IDS_FORMEL64_SV, "Formel64" },
/* tr */ { IDS_FORMEL64_TR, "Formel64" },

/* en */ { IDS_FILESYSTEM,    "Filesystem" },
/* da */ { IDS_FILESYSTEM_DA, "Filsystem" },
/* de */ { IDS_FILESYSTEM_DE, "Dateisystem" },
/* es */ { IDS_FILESYSTEM_ES, "Sistema de ficheros" },
/* fr */ { IDS_FILESYSTEM_FR, "Système de fichiers" },
/* hu */ { IDS_FILESYSTEM_HU, "" },  /* fuzzy */
/* it */ { IDS_FILESYSTEM_IT, "Filesystem" },
/* ko */ { IDS_FILESYSTEM_KO, "ÆÄÀÏ½Ã½ºÅÛ" },
/* nl */ { IDS_FILESYSTEM_NL, "Bestandssysteem" },
/* pl */ { IDS_FILESYSTEM_PL, "System plików" },
/* ru */ { IDS_FILESYSTEM_RU, "ÄÐÙÛÞÒÐï áØáâÕÜÐ" },
/* sv */ { IDS_FILESYSTEM_SV, "Filsystem" },
/* tr */ { IDS_FILESYSTEM_TR, "Dosya sistemi" },

/* en */ { IDS_RAW,    "Raw" },
/* da */ { IDS_RAW_DA, "Rå" },
/* de */ { IDS_RAW_DE, "Raw" },
/* es */ { IDS_RAW_ES, "Sin formato" },
/* fr */ { IDS_RAW_FR, "Raw" },
/* hu */ { IDS_RAW_HU, "" },  /* fuzzy */
/* it */ { IDS_RAW_IT, "Raw" },
/* ko */ { IDS_RAW_KO, "Raw" },
/* nl */ { IDS_RAW_NL, "Raw" },
/* pl */ { IDS_RAW_PL, "Surowy" },
/* ru */ { IDS_RAW_RU, "Raw" },
/* sv */ { IDS_RAW_SV, "Rått" },
/* tr */ { IDS_RAW_TR, "Düz" },

/* en */ { IDS_JOYSTICK,    "Joystick" },
/* da */ { IDS_JOYSTICK_DA, "Joystick" },
/* de */ { IDS_JOYSTICK_DE, "Joystick" },
/* es */ { IDS_JOYSTICK_ES, "Joystick" },
/* fr */ { IDS_JOYSTICK_FR, "Joystick" },
/* hu */ { IDS_JOYSTICK_HU, "Botkormány" },
/* it */ { IDS_JOYSTICK_IT, "Joystick" },
/* ko */ { IDS_JOYSTICK_KO, "Á¶ÀÌ½ºÆ½" },
/* nl */ { IDS_JOYSTICK_NL, "Joystick" },
/* pl */ { IDS_JOYSTICK_PL, "Joystick" },
/* ru */ { IDS_JOYSTICK_RU, "´ÖÞÙáâØÚ" },
/* sv */ { IDS_JOYSTICK_SV, "Styrspak" },
/* tr */ { IDS_JOYSTICK_TR, "Joystick" },

/* en */ { IDS_IRQ,    "IRQ" },
/* da */ { IDS_IRQ_DA, "IRQ" },
/* de */ { IDS_IRQ_DE, "IRQ" },
/* es */ { IDS_IRQ_ES, "IRQ" },
/* fr */ { IDS_IRQ_FR, "IRQ" },
/* hu */ { IDS_IRQ_HU, "IRQ" },
/* it */ { IDS_IRQ_IT, "IRQ" },
/* ko */ { IDS_IRQ_KO, "IRQ" },
/* nl */ { IDS_IRQ_NL, "IRQ" },
/* pl */ { IDS_IRQ_PL, "IRQ" },
/* ru */ { IDS_IRQ_RU, "IRQ" },
/* sv */ { IDS_IRQ_SV, "IRQ" },
/* tr */ { IDS_IRQ_TR, "IRQ" },

/* en */ { IDS_NMI,    "NMI" },
/* da */ { IDS_NMI_DA, "NMI" },
/* de */ { IDS_NMI_DE, "NMI" },
/* es */ { IDS_NMI_ES, "NMI" },
/* fr */ { IDS_NMI_FR, "NMI" },
/* hu */ { IDS_NMI_HU, "NMI" },
/* it */ { IDS_NMI_IT, "NMI" },
/* ko */ { IDS_NMI_KO, "NMI" },
/* nl */ { IDS_NMI_NL, "NMI" },
/* pl */ { IDS_NMI_PL, "NMI" },
/* ru */ { IDS_NMI_RU, "NMI" },
/* sv */ { IDS_NMI_SV, "NMI" },
/* tr */ { IDS_NMI_TR, "NMI" },

/* en */ { IDS_ACIA_DEVICE,    "ACIA device" },
/* da */ { IDS_ACIA_DEVICE_DA, "ACIA-enhed" },
/* de */ { IDS_ACIA_DEVICE_DE, "ACIA Gerät" },
/* es */ { IDS_ACIA_DEVICE_ES, "Periférico ACIA" },
/* fr */ { IDS_ACIA_DEVICE_FR, "Périphérique ACIA" },
/* hu */ { IDS_ACIA_DEVICE_HU, "ACIA eszköz" },
/* it */ { IDS_ACIA_DEVICE_IT, "Dispositivo ACIA" },
/* ko */ { IDS_ACIA_DEVICE_KO, "" },  /* fuzzy */
/* nl */ { IDS_ACIA_DEVICE_NL, "ACIA apparaat" },
/* pl */ { IDS_ACIA_DEVICE_PL, "Urz±dzenia ACIA" },
/* ru */ { IDS_ACIA_DEVICE_RU, "ACIA device" },
/* sv */ { IDS_ACIA_DEVICE_SV, "ACIA-enhet" },
/* tr */ { IDS_ACIA_DEVICE_TR, "ACIA aygýtý" },

/* en */ { IDS_ACIA_BASE,    "ACIA base address" },
/* da */ { IDS_ACIA_BASE_DA, "ACIA baseadresse" },
/* de */ { IDS_ACIA_BASE_DE, "ACIA Basis Adresse" },
/* es */ { IDS_ACIA_BASE_ES, "ACIA dirección base" },
/* fr */ { IDS_ACIA_BASE_FR, "Adresse de base ACIA" },
/* hu */ { IDS_ACIA_BASE_HU, "" },  /* fuzzy */
/* it */ { IDS_ACIA_BASE_IT, "indirizzo base ACIA" },
/* ko */ { IDS_ACIA_BASE_KO, "" },  /* fuzzy */
/* nl */ { IDS_ACIA_BASE_NL, "ACIA basisadres" },
/* pl */ { IDS_ACIA_BASE_PL, "Adres bazowy ACIA" },
/* ru */ { IDS_ACIA_BASE_RU, "" },  /* fuzzy */
/* sv */ { IDS_ACIA_BASE_SV, "ACIA-basadress" },
/* tr */ { IDS_ACIA_BASE_TR, "ACIA taban adresi" },

/* en */ { IDS_ACIA_INTERRUPT,    "ACIA interrupt" },
/* da */ { IDS_ACIA_INTERRUPT_DA, "ACIA-interrupt" },
/* de */ { IDS_ACIA_INTERRUPT_DE, "ACIA Interrupt" },
/* es */ { IDS_ACIA_INTERRUPT_ES, "Interruptor ACIA" },
/* fr */ { IDS_ACIA_INTERRUPT_FR, "Interruption ACIA" },
/* hu */ { IDS_ACIA_INTERRUPT_HU, "ACIA megszakítás" },
/* it */ { IDS_ACIA_INTERRUPT_IT, "Interrupt ACIA" },
/* ko */ { IDS_ACIA_INTERRUPT_KO, "" },  /* fuzzy */
/* nl */ { IDS_ACIA_INTERRUPT_NL, "ACIA interrupt" },
/* pl */ { IDS_ACIA_INTERRUPT_PL, "Przerwanie ACIA" },
/* ru */ { IDS_ACIA_INTERRUPT_RU, "ACIA interrupt" },
/* sv */ { IDS_ACIA_INTERRUPT_SV, "ACIA-avbrott" },
/* tr */ { IDS_ACIA_INTERRUPT_TR, "ACIA kesmesi" },

/* en */ { IDS_ACIA_MODE,    "ACIA mode" },
/* da */ { IDS_ACIA_MODE_DA, "ACIA-tilstand" },
/* de */ { IDS_ACIA_MODE_DE, "ACIA Modus" },
/* es */ { IDS_ACIA_MODE_ES, "Modo ACIA" },
/* fr */ { IDS_ACIA_MODE_FR, "Mode ACIA" },
/* hu */ { IDS_ACIA_MODE_HU, "ACIA mód" },
/* it */ { IDS_ACIA_MODE_IT, "Modalità ACIA" },
/* ko */ { IDS_ACIA_MODE_KO, "" },  /* fuzzy */
/* nl */ { IDS_ACIA_MODE_NL, "ACIA modus" },
/* pl */ { IDS_ACIA_MODE_PL, "Tryb ACIA" },
/* ru */ { IDS_ACIA_MODE_RU, "ACIA mode" },
/* sv */ { IDS_ACIA_MODE_SV, "ACIA-läge" },
/* tr */ { IDS_ACIA_MODE_TR, "ACIA modu" },

/* en */ { IDS_ACIA_SETTINGS,    "ACIA settings" },
/* da */ { IDS_ACIA_SETTINGS_DA, "ACIA-indstillinger" },
/* de */ { IDS_ACIA_SETTINGS_DE, "ACIA Einstellungen" },
/* es */ { IDS_ACIA_SETTINGS_ES, "Ajustes ACIA" },
/* fr */ { IDS_ACIA_SETTINGS_FR, "Paramètres ACIA" },
/* hu */ { IDS_ACIA_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_ACIA_SETTINGS_IT, "Impostazioni ACIA" },
/* ko */ { IDS_ACIA_SETTINGS_KO, "ACIA ¼³Á¤" },
/* nl */ { IDS_ACIA_SETTINGS_NL, "ACIA instellingen" },
/* pl */ { IDS_ACIA_SETTINGS_PL, "Ustawienia ACIA" },
/* ru */ { IDS_ACIA_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_ACIA_SETTINGS_SV, "ACIA-inställningar" },
/* tr */ { IDS_ACIA_SETTINGS_TR, "ACIA Ayarlarý" },

/* en */ { IDS_256K_BASE,    "256K base address" },
/* da */ { IDS_256K_BASE_DA, "256k-baseadresse" },
/* de */ { IDS_256K_BASE_DE, "256K Basis Adresse" },
/* es */ { IDS_256K_BASE_ES, "Dirección base 256K" },
/* fr */ { IDS_256K_BASE_FR, "Adresse de base 256K" },
/* hu */ { IDS_256K_BASE_HU, "256K báziscím" },
/* it */ { IDS_256K_BASE_IT, "indirizzo base 256K" },
/* ko */ { IDS_256K_BASE_KO, "" },  /* fuzzy */
/* nl */ { IDS_256K_BASE_NL, "256K basisadres" },
/* pl */ { IDS_256K_BASE_PL, "Adres bazowy 256K" },
/* ru */ { IDS_256K_BASE_RU, "256K base address" },
/* sv */ { IDS_256K_BASE_SV, "256K-basadress" },
/* tr */ { IDS_256K_BASE_TR, "256K taban adresi" },

/* en */ { IDS_256K_FILENAME,    "256K file" },
/* da */ { IDS_256K_FILENAME_DA, "256k-fil" },
/* de */ { IDS_256K_FILENAME_DE, "256K Datei" },
/* es */ { IDS_256K_FILENAME_ES, "Fichero 256K" },
/* fr */ { IDS_256K_FILENAME_FR, "Fichier 256K" },
/* hu */ { IDS_256K_FILENAME_HU, "256K fájl" },
/* it */ { IDS_256K_FILENAME_IT, "File 256K" },
/* ko */ { IDS_256K_FILENAME_KO, "256k ÆÄÀÏ" },
/* nl */ { IDS_256K_FILENAME_NL, "256K bestand" },
/* pl */ { IDS_256K_FILENAME_PL, "Plik 256K" },
/* ru */ { IDS_256K_FILENAME_RU, "256K file" },
/* sv */ { IDS_256K_FILENAME_SV, "256K-fil" },
/* tr */ { IDS_256K_FILENAME_TR, "256K dosyasý" },

/* en */ { IDS_256K_FILENAME_SELECT,    "Select file for 256K" },
/* da */ { IDS_256K_FILENAME_SELECT_DA, "Angiv fil for 256k" },
/* de */ { IDS_256K_FILENAME_SELECT_DE, "Datei für 256K auswählen" },
/* es */ { IDS_256K_FILENAME_SELECT_ES, "Seleccionar fichero para 256K" },
/* fr */ { IDS_256K_FILENAME_SELECT_FR, "Sélectionner fichier pour 256K" },
/* hu */ { IDS_256K_FILENAME_SELECT_HU, "Válasszon fájlt a 256K-hoz" },
/* it */ { IDS_256K_FILENAME_SELECT_IT, "Seleziona file per 256K" },
/* ko */ { IDS_256K_FILENAME_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDS_256K_FILENAME_SELECT_NL, "Selecteer bestand voor 256K" },
/* pl */ { IDS_256K_FILENAME_SELECT_PL, "Wybierz plik 256K" },
/* ru */ { IDS_256K_FILENAME_SELECT_RU, "Select file for 256K" },
/* sv */ { IDS_256K_FILENAME_SELECT_SV, "Ange fil för 256K" },
/* tr */ { IDS_256K_FILENAME_SELECT_TR, "256K için dosya seçin" },

/* en */ { IDS_C64DTV_ROM_FILENAME_SELECT,    "Select file for C64DTV ROM" },
/* da */ { IDS_C64DTV_ROM_FILENAME_SELECT_DA, "Vælg fil for C64DTV-ROM" },
/* de */ { IDS_C64DTV_ROM_FILENAME_SELECT_DE, "Datei für C64DTV ROM auswählen" },
/* es */ { IDS_C64DTV_ROM_FILENAME_SELECT_ES, "Seleccionar fichero para C64DTV ROM" },
/* fr */ { IDS_C64DTV_ROM_FILENAME_SELECT_FR, "Sélectionner fichier RO C64DTV" },
/* hu */ { IDS_C64DTV_ROM_FILENAME_SELECT_HU, "Válasszon fájlt a C64DTV ROM-hoz" },
/* it */ { IDS_C64DTV_ROM_FILENAME_SELECT_IT, "Seleziona il file per la ROM C64DTV" },
/* ko */ { IDS_C64DTV_ROM_FILENAME_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDS_C64DTV_ROM_FILENAME_SELECT_NL, "Selecteer bestand voor C64DTV ROM" },
/* pl */ { IDS_C64DTV_ROM_FILENAME_SELECT_PL, "Wybierz plik ROM C64DTV" },
/* ru */ { IDS_C64DTV_ROM_FILENAME_SELECT_RU, "Select file for C64DTV ROM" },
/* sv */ { IDS_C64DTV_ROM_FILENAME_SELECT_SV, "Välj fil för C64DTV-ROM" },
/* tr */ { IDS_C64DTV_ROM_FILENAME_SELECT_TR, "C64DTV ROM'u için dosya seçin" },

/* en */ { IDS_C64DTV_ROM_FILENAME,    "C64DTV ROM file" },
/* da */ { IDS_C64DTV_ROM_FILENAME_DA, "C64DTV-ROM-fil" },
/* de */ { IDS_C64DTV_ROM_FILENAME_DE, "C64DTV ROM Datei" },
/* es */ { IDS_C64DTV_ROM_FILENAME_ES, "Fichero C64DTV ROM" },
/* fr */ { IDS_C64DTV_ROM_FILENAME_FR, "Fichier ROM C64DTV" },
/* hu */ { IDS_C64DTV_ROM_FILENAME_HU, "C64DTV ROM fájl" },
/* it */ { IDS_C64DTV_ROM_FILENAME_IT, "File ROM C64DTV" },
/* ko */ { IDS_C64DTV_ROM_FILENAME_KO, "C64DTV ROM ÆÄÀÏ" },
/* nl */ { IDS_C64DTV_ROM_FILENAME_NL, "C64DTV ROM-bestand" },
/* pl */ { IDS_C64DTV_ROM_FILENAME_PL, "Plik ROM C64DTV" },
/* ru */ { IDS_C64DTV_ROM_FILENAME_RU, "C64DTV ROM file" },
/* sv */ { IDS_C64DTV_ROM_FILENAME_SV, "C64DTV-ROM-fil" },
/* tr */ { IDS_C64DTV_ROM_FILENAME_TR, "C64DTV ROM dosyasý" },

/* en */ { IDS_C64DTV_REVISION,    "DTV revision" },
/* da */ { IDS_C64DTV_REVISION_DA, "DTV-revision" },
/* de */ { IDS_C64DTV_REVISION_DE, "DTV Revision" },
/* es */ { IDS_C64DTV_REVISION_ES, "Revisión DTV" },
/* fr */ { IDS_C64DTV_REVISION_FR, "Révision DTV" },
/* hu */ { IDS_C64DTV_REVISION_HU, "" },  /* fuzzy */
/* it */ { IDS_C64DTV_REVISION_IT, "Revisione DTV" },
/* ko */ { IDS_C64DTV_REVISION_KO, "DTV ¼öÁ¤" },
/* nl */ { IDS_C64DTV_REVISION_NL, "DTV-revisie" },
/* pl */ { IDS_C64DTV_REVISION_PL, "Wersja DRV" },
/* ru */ { IDS_C64DTV_REVISION_RU, "DTV revision" },
/* sv */ { IDS_C64DTV_REVISION_SV, "DTV-utgåva" },
/* tr */ { IDS_C64DTV_REVISION_TR, "DTV revizyonu" },

/* en */ { IDS_C64DTV_WRITE_ENABLE,    "Writes to C64DTV ROM" },
/* da */ { IDS_C64DTV_WRITE_ENABLE_DA, "Skrivning til C64DTV-ROM" },
/* de */ { IDS_C64DTV_WRITE_ENABLE_DE, "Schreibzugriff auf C64DTV ROM" },
/* es */ { IDS_C64DTV_WRITE_ENABLE_ES, "Escribir en C64DTV ROM" },
/* fr */ { IDS_C64DTV_WRITE_ENABLE_FR, "Écriture dans le ROM C64DTV" },
/* hu */ { IDS_C64DTV_WRITE_ENABLE_HU, "C64DTV ROM-ba írások" },
/* it */ { IDS_C64DTV_WRITE_ENABLE_IT, "Scrivi sulla ROM C64DTV" },
/* ko */ { IDS_C64DTV_WRITE_ENABLE_KO, "" },  /* fuzzy */
/* nl */ { IDS_C64DTV_WRITE_ENABLE_NL, "Schrijven naar C64DTV ROM" },
/* pl */ { IDS_C64DTV_WRITE_ENABLE_PL, "Zapis do ROM-u C64DTV" },
/* ru */ { IDS_C64DTV_WRITE_ENABLE_RU, "Writes to C64DTV ROM" },
/* sv */ { IDS_C64DTV_WRITE_ENABLE_SV, "Skrivningar till C64DTV-ROM" },
/* tr */ { IDS_C64DTV_WRITE_ENABLE_TR, "C64DTV ROM'una yazar" },

/* en */ { IDS_C64DTV_HUMMER_ADC,    "Hummer ADC" },
/* da */ { IDS_C64DTV_HUMMER_ADC_DA, "Hummer ADC" },
/* de */ { IDS_C64DTV_HUMMER_ADC_DE, "Hummer ADC" },
/* es */ { IDS_C64DTV_HUMMER_ADC_ES, "Hummer ADC" },
/* fr */ { IDS_C64DTV_HUMMER_ADC_FR, "Hummer ADC" },
/* hu */ { IDS_C64DTV_HUMMER_ADC_HU, "" },  /* fuzzy */
/* it */ { IDS_C64DTV_HUMMER_ADC_IT, "Hummer ADC" },
/* ko */ { IDS_C64DTV_HUMMER_ADC_KO, "Çè¸Ó ADC" },
/* nl */ { IDS_C64DTV_HUMMER_ADC_NL, "Hummer ADC" },
/* pl */ { IDS_C64DTV_HUMMER_ADC_PL, "Hummer ADC" },
/* ru */ { IDS_C64DTV_HUMMER_ADC_RU, "Hummer ADC" },
/* sv */ { IDS_C64DTV_HUMMER_ADC_SV, "Hummer A/D-omvandlare" },
/* tr */ { IDS_C64DTV_HUMMER_ADC_TR, "Hummer ADC" },

/* en */ { IDS_C64DTV_SETTINGS,    "C64DTV settings" },
/* da */ { IDS_C64DTV_SETTINGS_DA, "C64DTV-indstillinger" },
/* de */ { IDS_C64DTV_SETTINGS_DE, "C64DTV Einstellungen" },
/* es */ { IDS_C64DTV_SETTINGS_ES, "Ajustes C64DTV" },
/* fr */ { IDS_C64DTV_SETTINGS_FR, "Paramètres C64DTV" },
/* hu */ { IDS_C64DTV_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_C64DTV_SETTINGS_IT, "Impostazioni C64DTV" },
/* ko */ { IDS_C64DTV_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_C64DTV_SETTINGS_NL, "C64DTV instellingen" },
/* pl */ { IDS_C64DTV_SETTINGS_PL, "Ustawienia C64DTV" },
/* ru */ { IDS_C64DTV_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_C64DTV_SETTINGS_SV, "C64DTV-inställningar" },
/* tr */ { IDS_C64DTV_SETTINGS_TR, "C64DTV Ayarlarý" },

/* en */ { IDS_MODEL_LINE,    "Model line" },
/* da */ { IDS_MODEL_LINE_DA, "Modelltyp" },
/* de */ { IDS_MODEL_LINE_DE, "Modell Linie" },
/* es */ { IDS_MODEL_LINE_ES, "Linea modelo" },
/* fr */ { IDS_MODEL_LINE_FR, "Ligne du modèle" },
/* hu */ { IDS_MODEL_LINE_HU, "" },  /* fuzzy */
/* it */ { IDS_MODEL_LINE_IT, "Linea" },
/* ko */ { IDS_MODEL_LINE_KO, "" },  /* fuzzy */
/* nl */ { IDS_MODEL_LINE_NL, "Model lijn" },
/* pl */ { IDS_MODEL_LINE_PL, "Linia modelu" },
/* ru */ { IDS_MODEL_LINE_RU, "" },  /* fuzzy */
/* sv */ { IDS_MODEL_LINE_SV, "Modelltyp" },
/* tr */ { IDS_MODEL_LINE_TR, "Model hattý" },

/* en */ { IDS_RAM_SIZE,    "RAM size" },
/* da */ { IDS_RAM_SIZE_DA, "RAM-størrelse" },
/* de */ { IDS_RAM_SIZE_DE, "RAM Größe" },
/* es */ { IDS_RAM_SIZE_ES, "Tamaño RAM" },
/* fr */ { IDS_RAM_SIZE_FR, "Taille de la RAM" },
/* hu */ { IDS_RAM_SIZE_HU, "" },  /* fuzzy */
/* it */ { IDS_RAM_SIZE_IT, "Dimensione RAM" },
/* ko */ { IDS_RAM_SIZE_KO, "" },  /* fuzzy */
/* nl */ { IDS_RAM_SIZE_NL, "RAM grootte" },
/* pl */ { IDS_RAM_SIZE_PL, "Rozmiar RAM" },
/* ru */ { IDS_RAM_SIZE_RU, "" },  /* fuzzy */
/* sv */ { IDS_RAM_SIZE_SV, "RAM-storlek" },
/* tr */ { IDS_RAM_SIZE_TR, "RAM boyutu" },

/* en */ { IDS_RAM_BLOCK_0400_0FFF,    "RAM Block $0400-$0FFF" },
/* da */ { IDS_RAM_BLOCK_0400_0FFF_DA, "RAM-blok $0400-$0FFF" },
/* de */ { IDS_RAM_BLOCK_0400_0FFF_DE, "RAM Block $0400-$0FFF" },
/* es */ { IDS_RAM_BLOCK_0400_0FFF_ES, "Bloque RAM $0400-$0FFF" },
/* fr */ { IDS_RAM_BLOCK_0400_0FFF_FR, "Bloc 0 (3 ko à $0400-$0FFF)" },
/* hu */ { IDS_RAM_BLOCK_0400_0FFF_HU, "Memória blokk $0400-$0FFF" },
/* it */ { IDS_RAM_BLOCK_0400_0FFF_IT, "Blocco RAM $0400-$0FFF" },
/* ko */ { IDS_RAM_BLOCK_0400_0FFF_KO, "" },  /* fuzzy */
/* nl */ { IDS_RAM_BLOCK_0400_0FFF_NL, "RAM blok $0400-$0FFF" },
/* pl */ { IDS_RAM_BLOCK_0400_0FFF_PL, "Blok RAM $0400-$0FFF" },
/* ru */ { IDS_RAM_BLOCK_0400_0FFF_RU, "RAM Block $0400-$0FFF" },
/* sv */ { IDS_RAM_BLOCK_0400_0FFF_SV, "RAM-block $0400-$0FFF" },
/* tr */ { IDS_RAM_BLOCK_0400_0FFF_TR, "RAM Blok $0400-$0FFF" },

/* en */ { IDS_RAM_BLOCK_0800_0FFF,    "RAM Block $0800-$0FFF" },
/* da */ { IDS_RAM_BLOCK_0800_0FFF_DA, "RAM-blok $0800-$0FFF" },
/* de */ { IDS_RAM_BLOCK_0800_0FFF_DE, "RAM Block $0800-$0FFF" },
/* es */ { IDS_RAM_BLOCK_0800_0FFF_ES, "Bloque RAM $0800-$0FFF" },
/* fr */ { IDS_RAM_BLOCK_0800_0FFF_FR, "Bloc 0 (3 ko à $0400-$0FFF)" },
/* hu */ { IDS_RAM_BLOCK_0800_0FFF_HU, "Memória blokk $0800-$0FFF" },
/* it */ { IDS_RAM_BLOCK_0800_0FFF_IT, "Blocco RAM $0800-$0FFF" },
/* ko */ { IDS_RAM_BLOCK_0800_0FFF_KO, "" },  /* fuzzy */
/* nl */ { IDS_RAM_BLOCK_0800_0FFF_NL, "RAM blok $0800-$0FFF" },
/* pl */ { IDS_RAM_BLOCK_0800_0FFF_PL, "Blok RAM $0800-$0FFF" },
/* ru */ { IDS_RAM_BLOCK_0800_0FFF_RU, "RAM Block $0800-$0FFF" },
/* sv */ { IDS_RAM_BLOCK_0800_0FFF_SV, "RAM-block $0800-$0FFF" },
/* tr */ { IDS_RAM_BLOCK_0800_0FFF_TR, "RAM Blok $0800-$0FFF" },

/* en */ { IDS_RAM_BLOCK_1000_1FFF,    "RAM Block $1000-$1FFF" },
/* da */ { IDS_RAM_BLOCK_1000_1FFF_DA, "RAM-blok $1000-$1FFF" },
/* de */ { IDS_RAM_BLOCK_1000_1FFF_DE, "RAM Block $1000-$1FFF" },
/* es */ { IDS_RAM_BLOCK_1000_1FFF_ES, "Bloque RAM $1000-$1FFF" },
/* fr */ { IDS_RAM_BLOCK_1000_1FFF_FR, "Bloc RAM $1000-$1FFF" },
/* hu */ { IDS_RAM_BLOCK_1000_1FFF_HU, "Memória blokk $1000-$1FFF" },
/* it */ { IDS_RAM_BLOCK_1000_1FFF_IT, "Blocco RAM $1000-$1FFF" },
/* ko */ { IDS_RAM_BLOCK_1000_1FFF_KO, "" },  /* fuzzy */
/* nl */ { IDS_RAM_BLOCK_1000_1FFF_NL, "RAM blok $1000-$1FFF" },
/* pl */ { IDS_RAM_BLOCK_1000_1FFF_PL, "Blok RAM $1000-$1FFF" },
/* ru */ { IDS_RAM_BLOCK_1000_1FFF_RU, "RAM Block $1000-$1FFF" },
/* sv */ { IDS_RAM_BLOCK_1000_1FFF_SV, "RAM-block $1000-$1FFF" },
/* tr */ { IDS_RAM_BLOCK_1000_1FFF_TR, "RAM Blok $1000-$1FFF" },

/* en */ { IDS_RAM_BLOCK_2000_3FFF,    "RAM Block $2000-$3FFF" },
/* da */ { IDS_RAM_BLOCK_2000_3FFF_DA, "RAM-blok $2000-$3FFF" },
/* de */ { IDS_RAM_BLOCK_2000_3FFF_DE, "RAM Block $2000-$3FFF" },
/* es */ { IDS_RAM_BLOCK_2000_3FFF_ES, "Bloque RAM $2000-$3FFF" },
/* fr */ { IDS_RAM_BLOCK_2000_3FFF_FR, "Bloc 1 (8 ko à $2000-$3FFF)" },
/* hu */ { IDS_RAM_BLOCK_2000_3FFF_HU, "Memória blokk $2000-$3FFF" },
/* it */ { IDS_RAM_BLOCK_2000_3FFF_IT, "Blocco RAM $2000-$3FFF" },
/* ko */ { IDS_RAM_BLOCK_2000_3FFF_KO, "" },  /* fuzzy */
/* nl */ { IDS_RAM_BLOCK_2000_3FFF_NL, "RAM blok $2000-$3FFF" },
/* pl */ { IDS_RAM_BLOCK_2000_3FFF_PL, "Blok RAM $2000-$3FFF" },
/* ru */ { IDS_RAM_BLOCK_2000_3FFF_RU, "RAM Block $2000-$3FFF" },
/* sv */ { IDS_RAM_BLOCK_2000_3FFF_SV, "RAM-block $2000-$3FFF" },
/* tr */ { IDS_RAM_BLOCK_2000_3FFF_TR, "RAM Blok $2000-$3FFF" },

/* en */ { IDS_RAM_BLOCK_4000_5FFF,    "RAM Block $4000-$5FFF" },
/* da */ { IDS_RAM_BLOCK_4000_5FFF_DA, "RAM-blok $4000-$5FFF" },
/* de */ { IDS_RAM_BLOCK_4000_5FFF_DE, "RAM Block $4000-$5FFF" },
/* es */ { IDS_RAM_BLOCK_4000_5FFF_ES, "Bloque RAM $4000-$5FFF" },
/* fr */ { IDS_RAM_BLOCK_4000_5FFF_FR, "Bloc 2 (8 ko à $4000-$5FFF)" },
/* hu */ { IDS_RAM_BLOCK_4000_5FFF_HU, "Memória blokk $4000-$5FFF" },
/* it */ { IDS_RAM_BLOCK_4000_5FFF_IT, "Blocco RAM $4000-$5FFF" },
/* ko */ { IDS_RAM_BLOCK_4000_5FFF_KO, "" },  /* fuzzy */
/* nl */ { IDS_RAM_BLOCK_4000_5FFF_NL, "RAM blok $4000-$5FFF" },
/* pl */ { IDS_RAM_BLOCK_4000_5FFF_PL, "Blok RAM $4000-$5FFF" },
/* ru */ { IDS_RAM_BLOCK_4000_5FFF_RU, "RAM Block $4000-$5FFF" },
/* sv */ { IDS_RAM_BLOCK_4000_5FFF_SV, "RAM-block $4000-$5FFF" },
/* tr */ { IDS_RAM_BLOCK_4000_5FFF_TR, "RAM Blok $4000-$5FFF" },

/* en */ { IDS_RAM_BLOCK_6000_7FFF,    "RAM Block $6000-$7FFF" },
/* da */ { IDS_RAM_BLOCK_6000_7FFF_DA, "RAM-blok $6000-$7FFF" },
/* de */ { IDS_RAM_BLOCK_6000_7FFF_DE, "RAM Block $6000-$7FFF" },
/* es */ { IDS_RAM_BLOCK_6000_7FFF_ES, "Bloque RAM $6000-$7FFF" },
/* fr */ { IDS_RAM_BLOCK_6000_7FFF_FR, "Bloc 3 (8 ko à $6000-$7FFF)" },
/* hu */ { IDS_RAM_BLOCK_6000_7FFF_HU, "Memória blokk $6000-$7FFF" },
/* it */ { IDS_RAM_BLOCK_6000_7FFF_IT, "Blocco RAM $6000-$7FFF" },
/* ko */ { IDS_RAM_BLOCK_6000_7FFF_KO, "" },  /* fuzzy */
/* nl */ { IDS_RAM_BLOCK_6000_7FFF_NL, "RAM blok $6000-$7FFF" },
/* pl */ { IDS_RAM_BLOCK_6000_7FFF_PL, "Blok RAM $6000-$7FFF" },
/* ru */ { IDS_RAM_BLOCK_6000_7FFF_RU, "RAM Block $6000-$7FFF" },
/* sv */ { IDS_RAM_BLOCK_6000_7FFF_SV, "RAM-block $6000-$7FFF" },
/* tr */ { IDS_RAM_BLOCK_6000_7FFF_TR, "RAM Blok $6000-$7FFF" },

/* en */ { IDS_RAM_BLOCK_A000_BFFF,    "RAM Block $A000-$BFFF" },
/* da */ { IDS_RAM_BLOCK_A000_BFFF_DA, "RAM-blok $A000-$BFFF" },
/* de */ { IDS_RAM_BLOCK_A000_BFFF_DE, "RAM Block $A000-$BFFF" },
/* es */ { IDS_RAM_BLOCK_A000_BFFF_ES, "Bloque RAM $A000-$BFFF" },
/* fr */ { IDS_RAM_BLOCK_A000_BFFF_FR, "Bloc 5 (8 ko à $A000-$BFFF)" },
/* hu */ { IDS_RAM_BLOCK_A000_BFFF_HU, "Memória blokk $A000-$BFFF" },
/* it */ { IDS_RAM_BLOCK_A000_BFFF_IT, "Blocco RAM $A000-$BFFF" },
/* ko */ { IDS_RAM_BLOCK_A000_BFFF_KO, "" },  /* fuzzy */
/* nl */ { IDS_RAM_BLOCK_A000_BFFF_NL, "RAM blok $A000-$BFFF" },
/* pl */ { IDS_RAM_BLOCK_A000_BFFF_PL, "Blok RAM $A000-$BFFF" },
/* ru */ { IDS_RAM_BLOCK_A000_BFFF_RU, "RAM Block $A000-$BFFF" },
/* sv */ { IDS_RAM_BLOCK_A000_BFFF_SV, "RAM-block $A000-$BFFF" },
/* tr */ { IDS_RAM_BLOCK_A000_BFFF_TR, "RAM Blok $A000-$BFFF" },

/* en */ { IDS_RAM_BLOCK_C000_CFFF,    "RAM Block $C000-$CFFF" },
/* da */ { IDS_RAM_BLOCK_C000_CFFF_DA, "RAM-blok $C000-$CFFF" },
/* de */ { IDS_RAM_BLOCK_C000_CFFF_DE, "RAM Block $C000-$CFFF" },
/* es */ { IDS_RAM_BLOCK_C000_CFFF_ES, "Bloque RAM $C000-$CFFF" },
/* fr */ { IDS_RAM_BLOCK_C000_CFFF_FR, "Bloc RAM $C000-$CFFF" },
/* hu */ { IDS_RAM_BLOCK_C000_CFFF_HU, "Memória blokk $C000-$CFFF" },
/* it */ { IDS_RAM_BLOCK_C000_CFFF_IT, "Blocco RAM $C000-$CFFF" },
/* ko */ { IDS_RAM_BLOCK_C000_CFFF_KO, "" },  /* fuzzy */
/* nl */ { IDS_RAM_BLOCK_C000_CFFF_NL, "RAM blok $C000-$CFFF" },
/* pl */ { IDS_RAM_BLOCK_C000_CFFF_PL, "Blok RAM $C000-$CFFF" },
/* ru */ { IDS_RAM_BLOCK_C000_CFFF_RU, "RAM Block $C000-$CFFF" },
/* sv */ { IDS_RAM_BLOCK_C000_CFFF_SV, "RAM-block $C000-$CFFF" },
/* tr */ { IDS_RAM_BLOCK_C000_CFFF_TR, "RAM Blok $C000-$CFFF" },

/* en */ { IDS_CBM2_SETTINGS,    "CBM2 settings" },
/* da */ { IDS_CBM2_SETTINGS_DA, "CBM2-indstillinger" },
/* de */ { IDS_CBM2_SETTINGS_DE, "CBM2 Einstellungen" },
/* es */ { IDS_CBM2_SETTINGS_ES, "Ajustes CBM2" },
/* fr */ { IDS_CBM2_SETTINGS_FR, "Paramètres CBM2" },
/* hu */ { IDS_CBM2_SETTINGS_HU, "CBM2 beállításai" },
/* it */ { IDS_CBM2_SETTINGS_IT, "Impostazioni CBM2" },
/* ko */ { IDS_CBM2_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_CBM2_SETTINGS_NL, "CBM2 instellingen" },
/* pl */ { IDS_CBM2_SETTINGS_PL, "Ustawienia CBM2" },
/* ru */ { IDS_CBM2_SETTINGS_RU, "CBM2 settings" },
/* sv */ { IDS_CBM2_SETTINGS_SV, "CBM2-inställningar" },
/* tr */ { IDS_CBM2_SETTINGS_TR, "CBM2 ayarlarý" },

/* en */ { IDS_CBM5X0_SETTINGS,    "CBM5x0 settings" },
/* da */ { IDS_CBM5X0_SETTINGS_DA, "CBM5x0-indstillinger" },
/* de */ { IDS_CBM5X0_SETTINGS_DE, "CBM5x0 Einstellungen" },
/* es */ { IDS_CBM5X0_SETTINGS_ES, "Ajustes CBM5x0" },
/* fr */ { IDS_CBM5X0_SETTINGS_FR, "Paramètres CBM5x0" },
/* hu */ { IDS_CBM5X0_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_CBM5X0_SETTINGS_IT, "Impostazioni CBM5x0" },
/* ko */ { IDS_CBM5X0_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_CBM5X0_SETTINGS_NL, "CBM5x0 instellingen" },
/* pl */ { IDS_CBM5X0_SETTINGS_PL, "Ustawienia CBM5x0" },
/* ru */ { IDS_CBM5X0_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_CBM5X0_SETTINGS_SV, "CBM5x0-inställningar" },
/* tr */ { IDS_CBM5X0_SETTINGS_TR, "CBM5x0 ayarlarý" },

/* en */ { IDS_DATASETTE,    "Datasette" },
/* da */ { IDS_DATASETTE_DA, "" },  /* fuzzy */
/* de */ { IDS_DATASETTE_DE, "Datasette" },
/* es */ { IDS_DATASETTE_ES, "" },  /* fuzzy */
/* fr */ { IDS_DATASETTE_FR, "" },  /* fuzzy */
/* hu */ { IDS_DATASETTE_HU, "" },  /* fuzzy */
/* it */ { IDS_DATASETTE_IT, "" },  /* fuzzy */
/* ko */ { IDS_DATASETTE_KO, "" },  /* fuzzy */
/* nl */ { IDS_DATASETTE_NL, "Datasette" },
/* pl */ { IDS_DATASETTE_PL, "" },  /* fuzzy */
/* ru */ { IDS_DATASETTE_RU, "" },  /* fuzzy */
/* sv */ { IDS_DATASETTE_SV, "" },  /* fuzzy */
/* tr */ { IDS_DATASETTE_TR, "" },  /* fuzzy */

/* en */ { IDS_RESET_DATASETTE_WITH_CPU,    "Reset Datasette with CPU" },
/* da */ { IDS_RESET_DATASETTE_WITH_CPU_DA, "Reset Datasette ved CPU-reset" },
/* de */ { IDS_RESET_DATASETTE_WITH_CPU_DE, "Zurücksetzen des Bandlaufwerks bei CPU Reset" },
/* es */ { IDS_RESET_DATASETTE_WITH_CPU_ES, "Reiniciar Datasette con la CPU" },
/* fr */ { IDS_RESET_DATASETTE_WITH_CPU_FR, "Réinitialiser la datassette avec l'UCT" },
/* hu */ { IDS_RESET_DATASETTE_WITH_CPU_HU, "Alaphelyzetbe hozás CPU Reset esetén" },
/* it */ { IDS_RESET_DATASETTE_WITH_CPU_IT, "Reset del registratore con CPU" },
/* ko */ { IDS_RESET_DATASETTE_WITH_CPU_KO, "µ¥ÀÌÅ¸¼Â À» CPU ·Î ¸®¼Â ÇÏ½Ã¿À" },
/* nl */ { IDS_RESET_DATASETTE_WITH_CPU_NL, "Reset Datasette met CPU" },
/* pl */ { IDS_RESET_DATASETTE_WITH_CPU_PL, "Resetuj magnetofon razem z CPU" },
/* ru */ { IDS_RESET_DATASETTE_WITH_CPU_RU, "Reset Datasette with CPU" },
/* sv */ { IDS_RESET_DATASETTE_WITH_CPU_SV, "Nollställ Datasette med CPU" },
/* tr */ { IDS_RESET_DATASETTE_WITH_CPU_TR, "CPU ile birlikte Teyp'i resetle" },

/* en */ { IDS_ADDITIONAL_DELAY,    "Additional Delay" },
/* da */ { IDS_ADDITIONAL_DELAY_DA, "Yderligere forsinkelse" },
/* de */ { IDS_ADDITIONAL_DELAY_DE, "Zusätzliche Verzögerung" },
/* es */ { IDS_ADDITIONAL_DELAY_ES, "Retardo adicional" },
/* fr */ { IDS_ADDITIONAL_DELAY_FR, "Délai additionnel" },
/* hu */ { IDS_ADDITIONAL_DELAY_HU, "Többlet szünet" },
/* it */ { IDS_ADDITIONAL_DELAY_IT, "Ritardo aggiuntivo" },
/* ko */ { IDS_ADDITIONAL_DELAY_KO, "" },  /* fuzzy */
/* nl */ { IDS_ADDITIONAL_DELAY_NL, "Extra vertraging" },
/* pl */ { IDS_ADDITIONAL_DELAY_PL, "Dodatkowy delay" },
/* ru */ { IDS_ADDITIONAL_DELAY_RU, "Additional Delay" },
/* sv */ { IDS_ADDITIONAL_DELAY_SV, "Ytterligare fördröjning" },
/* tr */ { IDS_ADDITIONAL_DELAY_TR, "Ekstra Bekletme" },

/* en */ { IDS_DELAY_AT_ZERO_VALUES,    "Datasette zero gap delay" },
/* da */ { IDS_DELAY_AT_ZERO_VALUES_DA, "Datasette nulgabs-forsinkelse" },
/* de */ { IDS_DELAY_AT_ZERO_VALUES_DE, "Bandlaufwerk Zero Gap Verzögerung" },
/* es */ { IDS_DELAY_AT_ZERO_VALUES_ES, "Retardo datasette zero gap" },
/* fr */ { IDS_DELAY_AT_ZERO_VALUES_FR, "Délai du datassette lors de la lecture des groupes de zéros" },
/* hu */ { IDS_DELAY_AT_ZERO_VALUES_HU, "Magnó üres rés késleltetés" },
/* it */ { IDS_DELAY_AT_ZERO_VALUES_IT, "Ritardo del gap di zero del registratore" },
/* ko */ { IDS_DELAY_AT_ZERO_VALUES_KO, "µ¥ÀÌÅ¸¼Â ¿µ Â÷ÀÌ µô·¹ÀÌ" },
/* nl */ { IDS_DELAY_AT_ZERO_VALUES_NL, "Datasette zero gap vertraging" },
/* pl */ { IDS_DELAY_AT_ZERO_VALUES_PL, "Opó¼nienie pocz±tkowej przerwy magnetofonu" },
/* ru */ { IDS_DELAY_AT_ZERO_VALUES_RU, "Datasette zero gap delay" },
/* sv */ { IDS_DELAY_AT_ZERO_VALUES_SV, "Datasette nollgapsfördröjning" },
/* tr */ { IDS_DELAY_AT_ZERO_VALUES_TR, "Teyp sýfýr ara beklemesi" },

/* en */ { IDS_DATASETTE_SETTINGS,    "Datasette settings" },
/* da */ { IDS_DATASETTE_SETTINGS_DA, "Datasetteindstillinger" },
/* de */ { IDS_DATASETTE_SETTINGS_DE, "Datasette Einstellungen" },
/* es */ { IDS_DATASETTE_SETTINGS_ES, "Ajustes Datasette" },
/* fr */ { IDS_DATASETTE_SETTINGS_FR, "Paramètres datassette" },
/* hu */ { IDS_DATASETTE_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_DATASETTE_SETTINGS_IT, "Impostazioni registratore" },
/* ko */ { IDS_DATASETTE_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_DATASETTE_SETTINGS_NL, "Datasette instellingen" },
/* pl */ { IDS_DATASETTE_SETTINGS_PL, "Ustawienia magnetofonu" },
/* ru */ { IDS_DATASETTE_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_DATASETTE_SETTINGS_SV, "Datasetteinställningar" },
/* tr */ { IDS_DATASETTE_SETTINGS_TR, "Teyp Ayarlarý" },

/* en */ { IDS_CP_CLOCK_F83,    "CP Clock F83" },
/* da */ { IDS_CP_CLOCK_F83_DA, "" },  /* fuzzy */
/* de */ { IDS_CP_CLOCK_F83_DE, "CP Clock F83" },
/* es */ { IDS_CP_CLOCK_F83_ES, "" },  /* fuzzy */
/* fr */ { IDS_CP_CLOCK_F83_FR, "" },  /* fuzzy */
/* hu */ { IDS_CP_CLOCK_F83_HU, "" },  /* fuzzy */
/* it */ { IDS_CP_CLOCK_F83_IT, "" },  /* fuzzy */
/* ko */ { IDS_CP_CLOCK_F83_KO, "" },  /* fuzzy */
/* nl */ { IDS_CP_CLOCK_F83_NL, "CP Clock F83" },
/* pl */ { IDS_CP_CLOCK_F83_PL, "" },  /* fuzzy */
/* ru */ { IDS_CP_CLOCK_F83_RU, "" },  /* fuzzy */
/* sv */ { IDS_CP_CLOCK_F83_SV, "" },  /* fuzzy */
/* tr */ { IDS_CP_CLOCK_F83_TR, "" },  /* fuzzy */

/* en */ { IDS_SAVE_CP_CLOCK_F83_RTC_DATA,    "Save CP Clock F83 RTC data when changed" },
/* da */ { IDS_SAVE_CP_CLOCK_F83_RTC_DATA_DA, "" },  /* fuzzy */
/* de */ { IDS_SAVE_CP_CLOCK_F83_RTC_DATA_DE, "CP Clock F83 RTC Daten bei Änderung speichern" },
/* es */ { IDS_SAVE_CP_CLOCK_F83_RTC_DATA_ES, "" },  /* fuzzy */
/* fr */ { IDS_SAVE_CP_CLOCK_F83_RTC_DATA_FR, "" },  /* fuzzy */
/* hu */ { IDS_SAVE_CP_CLOCK_F83_RTC_DATA_HU, "" },  /* fuzzy */
/* it */ { IDS_SAVE_CP_CLOCK_F83_RTC_DATA_IT, "" },  /* fuzzy */
/* ko */ { IDS_SAVE_CP_CLOCK_F83_RTC_DATA_KO, "" },  /* fuzzy */
/* nl */ { IDS_SAVE_CP_CLOCK_F83_RTC_DATA_NL, "Sla de CP Clock F83 RTC data op bij wijziging" },
/* pl */ { IDS_SAVE_CP_CLOCK_F83_RTC_DATA_PL, "" },  /* fuzzy */
/* ru */ { IDS_SAVE_CP_CLOCK_F83_RTC_DATA_RU, "" },  /* fuzzy */
/* sv */ { IDS_SAVE_CP_CLOCK_F83_RTC_DATA_SV, "" },  /* fuzzy */
/* tr */ { IDS_SAVE_CP_CLOCK_F83_RTC_DATA_TR, "" },  /* fuzzy */

/* en */ { IDS_CP_CLOCK_F83_SETTINGS,    "CP Clock F83 settings" },
/* da */ { IDS_CP_CLOCK_F83_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDS_CP_CLOCK_F83_SETTINGS_DE, "CP Clock F83 Einstellungen" },
/* es */ { IDS_CP_CLOCK_F83_SETTINGS_ES, "" },  /* fuzzy */
/* fr */ { IDS_CP_CLOCK_F83_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDS_CP_CLOCK_F83_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_CP_CLOCK_F83_SETTINGS_IT, "" },  /* fuzzy */
/* ko */ { IDS_CP_CLOCK_F83_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_CP_CLOCK_F83_SETTINGS_NL, "CP Clock F83 instellingen" },
/* pl */ { IDS_CP_CLOCK_F83_SETTINGS_PL, "" },  /* fuzzy */
/* ru */ { IDS_CP_CLOCK_F83_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_CP_CLOCK_F83_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDS_CP_CLOCK_F83_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDS_GENERIC_LOGFILE,    "Generic logfile" },
/* da */ { IDS_GENERIC_LOGFILE_DA, "" },  /* fuzzy */
/* de */ { IDS_GENERIC_LOGFILE_DE, "Generische Logdatei" },
/* es */ { IDS_GENERIC_LOGFILE_ES, "" },  /* fuzzy */
/* fr */ { IDS_GENERIC_LOGFILE_FR, "" },  /* fuzzy */
/* hu */ { IDS_GENERIC_LOGFILE_HU, "" },  /* fuzzy */
/* it */ { IDS_GENERIC_LOGFILE_IT, "" },  /* fuzzy */
/* ko */ { IDS_GENERIC_LOGFILE_KO, "" },  /* fuzzy */
/* nl */ { IDS_GENERIC_LOGFILE_NL, "Algemeen log bestand" },
/* pl */ { IDS_GENERIC_LOGFILE_PL, "" },  /* fuzzy */
/* ru */ { IDS_GENERIC_LOGFILE_RU, "" },  /* fuzzy */
/* sv */ { IDS_GENERIC_LOGFILE_SV, "" },  /* fuzzy */
/* tr */ { IDS_GENERIC_LOGFILE_TR, "" },  /* fuzzy */

/* en */ { IDS_SPECIFIED_TAPELOG_FILE,    "Generic logfile" },
/* da */ { IDS_SPECIFIED_TAPELOG_FILE_DA, "" },  /* fuzzy */
/* de */ { IDS_SPECIFIED_TAPELOG_FILE_DE, "Generische Logdatei" },
/* es */ { IDS_SPECIFIED_TAPELOG_FILE_ES, "" },  /* fuzzy */
/* fr */ { IDS_SPECIFIED_TAPELOG_FILE_FR, "" },  /* fuzzy */
/* hu */ { IDS_SPECIFIED_TAPELOG_FILE_HU, "" },  /* fuzzy */
/* it */ { IDS_SPECIFIED_TAPELOG_FILE_IT, "" },  /* fuzzy */
/* ko */ { IDS_SPECIFIED_TAPELOG_FILE_KO, "" },  /* fuzzy */
/* nl */ { IDS_SPECIFIED_TAPELOG_FILE_NL, "Algemeen log bestand" },
/* pl */ { IDS_SPECIFIED_TAPELOG_FILE_PL, "" },  /* fuzzy */
/* ru */ { IDS_SPECIFIED_TAPELOG_FILE_RU, "" },  /* fuzzy */
/* sv */ { IDS_SPECIFIED_TAPELOG_FILE_SV, "" },  /* fuzzy */
/* tr */ { IDS_SPECIFIED_TAPELOG_FILE_TR, "" },  /* fuzzy */

/* en */ { IDS_TAPELOG,    "TapeLog" },
/* da */ { IDS_TAPELOG_DA, "" },  /* fuzzy */
/* de */ { IDS_TAPELOG_DE, "Bandlog" },
/* es */ { IDS_TAPELOG_ES, "" },  /* fuzzy */
/* fr */ { IDS_TAPELOG_FR, "" },  /* fuzzy */
/* hu */ { IDS_TAPELOG_HU, "" },  /* fuzzy */
/* it */ { IDS_TAPELOG_IT, "" },  /* fuzzy */
/* ko */ { IDS_TAPELOG_KO, "" },  /* fuzzy */
/* nl */ { IDS_TAPELOG_NL, "TapeLog" },
/* pl */ { IDS_TAPELOG_PL, "" },  /* fuzzy */
/* ru */ { IDS_TAPELOG_RU, "" },  /* fuzzy */
/* sv */ { IDS_TAPELOG_SV, "" },  /* fuzzy */
/* tr */ { IDS_TAPELOG_TR, "" },  /* fuzzy */

/* en */ { IDS_TAPELOG_DESTINATION,    "TapeLog messages destination" },
/* da */ { IDS_TAPELOG_DESTINATION_DA, "" },  /* fuzzy */
/* de */ { IDS_TAPELOG_DESTINATION_DE, "Ort für Bandlog Nachrichten" },
/* es */ { IDS_TAPELOG_DESTINATION_ES, "" },  /* fuzzy */
/* fr */ { IDS_TAPELOG_DESTINATION_FR, "" },  /* fuzzy */
/* hu */ { IDS_TAPELOG_DESTINATION_HU, "" },  /* fuzzy */
/* it */ { IDS_TAPELOG_DESTINATION_IT, "" },  /* fuzzy */
/* ko */ { IDS_TAPELOG_DESTINATION_KO, "" },  /* fuzzy */
/* nl */ { IDS_TAPELOG_DESTINATION_NL, "TapeLog berichten bestemming" },
/* pl */ { IDS_TAPELOG_DESTINATION_PL, "" },  /* fuzzy */
/* ru */ { IDS_TAPELOG_DESTINATION_RU, "" },  /* fuzzy */
/* sv */ { IDS_TAPELOG_DESTINATION_SV, "" },  /* fuzzy */
/* tr */ { IDS_TAPELOG_DESTINATION_TR, "" },  /* fuzzy */

/* en */ { IDS_TAPELOG_FILENAME_SELECT,    "Select file for TapeLog" },
/* da */ { IDS_TAPELOG_FILENAME_SELECT_DA, "" },  /* fuzzy */
/* de */ { IDS_TAPELOG_FILENAME_SELECT_DE, "Bandlog Datei auswählen" },
/* es */ { IDS_TAPELOG_FILENAME_SELECT_ES, "" },  /* fuzzy */
/* fr */ { IDS_TAPELOG_FILENAME_SELECT_FR, "" },  /* fuzzy */
/* hu */ { IDS_TAPELOG_FILENAME_SELECT_HU, "" },  /* fuzzy */
/* it */ { IDS_TAPELOG_FILENAME_SELECT_IT, "" },  /* fuzzy */
/* ko */ { IDS_TAPELOG_FILENAME_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDS_TAPELOG_FILENAME_SELECT_NL, "Selecteer bestand voor de TapeLog" },
/* pl */ { IDS_TAPELOG_FILENAME_SELECT_PL, "" },  /* fuzzy */
/* ru */ { IDS_TAPELOG_FILENAME_SELECT_RU, "" },  /* fuzzy */
/* sv */ { IDS_TAPELOG_FILENAME_SELECT_SV, "" },  /* fuzzy */
/* tr */ { IDS_TAPELOG_FILENAME_SELECT_TR, "" },  /* fuzzy */

/* en */ { IDS_TAPELOG_FILENAME,    "TapeLog file" },
/* da */ { IDS_TAPELOG_FILENAME_DA, "" },  /* fuzzy */
/* de */ { IDS_TAPELOG_FILENAME_DE, "Bandlog Datei" },
/* es */ { IDS_TAPELOG_FILENAME_ES, "" },  /* fuzzy */
/* fr */ { IDS_TAPELOG_FILENAME_FR, "" },  /* fuzzy */
/* hu */ { IDS_TAPELOG_FILENAME_HU, "" },  /* fuzzy */
/* it */ { IDS_TAPELOG_FILENAME_IT, "" },  /* fuzzy */
/* ko */ { IDS_TAPELOG_FILENAME_KO, "" },  /* fuzzy */
/* nl */ { IDS_TAPELOG_FILENAME_NL, "TapeLog bestand" },
/* pl */ { IDS_TAPELOG_FILENAME_PL, "" },  /* fuzzy */
/* ru */ { IDS_TAPELOG_FILENAME_RU, "" },  /* fuzzy */
/* sv */ { IDS_TAPELOG_FILENAME_SV, "" },  /* fuzzy */
/* tr */ { IDS_TAPELOG_FILENAME_TR, "" },  /* fuzzy */

/* en */ { IDS_TAPELOG_SETTINGS,    "TapeLog settings" },
/* da */ { IDS_TAPELOG_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDS_TAPELOG_SETTINGS_DE, "Bandlog Einstellungen" },
/* es */ { IDS_TAPELOG_SETTINGS_ES, "" },  /* fuzzy */
/* fr */ { IDS_TAPELOG_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDS_TAPELOG_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_TAPELOG_SETTINGS_IT, "" },  /* fuzzy */
/* ko */ { IDS_TAPELOG_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_TAPELOG_SETTINGS_NL, "TapeLog instellingen" },
/* pl */ { IDS_TAPELOG_SETTINGS_PL, "" },  /* fuzzy */
/* ru */ { IDS_TAPELOG_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_TAPELOG_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDS_TAPELOG_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDS_NEVER_EXTEND,    "Never extend" },
/* da */ { IDS_NEVER_EXTEND_DA, "Udvid aldrig" },
/* de */ { IDS_NEVER_EXTEND_DE, "Nie Erweitern" },
/* es */ { IDS_NEVER_EXTEND_ES, "No extender nunca" },
/* fr */ { IDS_NEVER_EXTEND_FR, "Ne jamais étendre" },
/* hu */ { IDS_NEVER_EXTEND_HU, "Nincs kiterjesztés" },
/* it */ { IDS_NEVER_EXTEND_IT, "Mai" },
/* ko */ { IDS_NEVER_EXTEND_KO, "Àý´ë ¿¬ÀåÇÏÁö¾Ê±â" },
/* nl */ { IDS_NEVER_EXTEND_NL, "Nooit uitbreiden" },
/* pl */ { IDS_NEVER_EXTEND_PL, "Nigdy nie rozszerzaj" },
/* ru */ { IDS_NEVER_EXTEND_RU, "Never extend" },
/* sv */ { IDS_NEVER_EXTEND_SV, "Utöka aldrig" },
/* tr */ { IDS_NEVER_EXTEND_TR, "Asla geniþletme" },

/* en */ { IDS_ASK_ON_EXTEND,    "Ask on extend" },
/* da */ { IDS_ASK_ON_EXTEND_DA, "Spørg ved udviddelse" },
/* de */ { IDS_ASK_ON_EXTEND_DE, "Bei Erweiterung Rückfrage" },
/* es */ { IDS_ASK_ON_EXTEND_ES, "Preguntar antes de extender" },
/* fr */ { IDS_ASK_ON_EXTEND_FR, "Demander avant d'étendre" },
/* hu */ { IDS_ASK_ON_EXTEND_HU, "Kérdés kiterjesztés esetén" },
/* it */ { IDS_ASK_ON_EXTEND_IT, "Chiedi all'accesso" },
/* ko */ { IDS_ASK_ON_EXTEND_KO, "¿¬ÀåÇÏ´Â°Í¿¡ ´ëÇÑ Áú¹®" },
/* nl */ { IDS_ASK_ON_EXTEND_NL, "Vragen bij uitbreiden" },
/* pl */ { IDS_ASK_ON_EXTEND_PL, "Zapytaj przy rozszerzaniu" },
/* ru */ { IDS_ASK_ON_EXTEND_RU, "Ask on extend" },
/* sv */ { IDS_ASK_ON_EXTEND_SV, "Fråga vid utökning" },
/* tr */ { IDS_ASK_ON_EXTEND_TR, "Geniþletirken sor" },

/* en */ { IDS_EXTEND_ON_ACCESS,    "Extend on access" },
/* da */ { IDS_EXTEND_ON_ACCESS_DA, "Udvid ved tilgang" },
/* de */ { IDS_EXTEND_ON_ACCESS_DE, "Erweitern wenn nötig" },
/* es */ { IDS_EXTEND_ON_ACCESS_ES, "Extender al acceder" },
/* fr */ { IDS_EXTEND_ON_ACCESS_FR, "Étendre à l'accès" },
/* hu */ { IDS_EXTEND_ON_ACCESS_HU, "Kiterjesztés hozzáféréskor" },
/* it */ { IDS_EXTEND_ON_ACCESS_IT, "All'accesso" },
/* ko */ { IDS_EXTEND_ON_ACCESS_KO, "¿¢¼¼½º ¿¬ÀåÇÏ±â" },
/* nl */ { IDS_EXTEND_ON_ACCESS_NL, "Uitbreiden bij toegang" },
/* pl */ { IDS_EXTEND_ON_ACCESS_PL, "Rozszerz przy dostêpie" },
/* ru */ { IDS_EXTEND_ON_ACCESS_RU, "Extend on access" },
/* sv */ { IDS_EXTEND_ON_ACCESS_SV, "Utöka vid åtkomst" },
/* tr */ { IDS_EXTEND_ON_ACCESS_TR, "Eriþildiðinde geniþlet" },

/* en */ { IDS_NO_TRAPS,    "No traps" },
/* da */ { IDS_NO_TRAPS_DA, "Ingen traps" },
/* de */ { IDS_NO_TRAPS_DE, "Keine Traps" },
/* es */ { IDS_NO_TRAPS_ES, "No interceptar" },
/* fr */ { IDS_NO_TRAPS_FR, "Pas de trappes" },
/* hu */ { IDS_NO_TRAPS_HU, "Folyamatos emuláció" },
/* it */ { IDS_NO_TRAPS_IT, "Non rilevare" },
/* ko */ { IDS_NO_TRAPS_KO, "Æ®·¦ÀÌ ¾ø½À´Ï´Ù" },
/* nl */ { IDS_NO_TRAPS_NL, "Geen traps" },
/* pl */ { IDS_NO_TRAPS_PL, "No traps" },
/* ru */ { IDS_NO_TRAPS_RU, "No traps" },
/* sv */ { IDS_NO_TRAPS_SV, "Ingen infångning" },
/* tr */ { IDS_NO_TRAPS_TR, "Trap Yok" },

/* en */ { IDS_SKIP_CYCLES,    "Skip cycles" },
/* da */ { IDS_SKIP_CYCLES_DA, "Spring over cykler" },
/* de */ { IDS_SKIP_CYCLES_DE, "Zyklen auslassen" },
/* es */ { IDS_SKIP_CYCLES_ES, "Saltar ciclos" },
/* fr */ { IDS_SKIP_CYCLES_FR, "Sauter des cycles" },
/* hu */ { IDS_SKIP_CYCLES_HU, "Ciklusok kihagyása" },
/* it */ { IDS_SKIP_CYCLES_IT, "Salta cicli" },
/* ko */ { IDS_SKIP_CYCLES_KO, "½ÎÀÌÅ¬ ³Ñ¾î°¡±â" },
/* nl */ { IDS_SKIP_CYCLES_NL, "Sla cycli over" },
/* pl */ { IDS_SKIP_CYCLES_PL, "Opuszczaj cykle" },
/* ru */ { IDS_SKIP_CYCLES_RU, "Skip cycles" },
/* sv */ { IDS_SKIP_CYCLES_SV, "Hoppa cykler" },
/* tr */ { IDS_SKIP_CYCLES_TR, "Cyclelarý atla" },

/* en */ { IDS_TRAP_IDLE,    "Trap idle" },
/* da */ { IDS_TRAP_IDLE_DA, "Fang inaktive" },
/* de */ { IDS_TRAP_IDLE_DE, "Trap idle" },
/* es */ { IDS_TRAP_IDLE_ES, "Interceptar tiempos muertos" },
/* fr */ { IDS_TRAP_IDLE_FR, "Traper l’inactivité" },
/* hu */ { IDS_TRAP_IDLE_HU, "Üres ciklusok kihagyása" },
/* it */ { IDS_TRAP_IDLE_IT, "Rileva inattività" },
/* ko */ { IDS_TRAP_IDLE_KO, "Æ®·¦ ¾ÆÀÌµé" },
/* nl */ { IDS_TRAP_IDLE_NL, "Trap idle" },
/* pl */ { IDS_TRAP_IDLE_PL, "Trap idle" },
/* ru */ { IDS_TRAP_IDLE_RU, "Trap idle" },
/* sv */ { IDS_TRAP_IDLE_SV, "Fånga inaktiva" },
/* tr */ { IDS_TRAP_IDLE_TR, "Trap boþa vakit geçirme" },

/* en */ { IDS_DRIVE_TYPE,    "Drive type" },
/* da */ { IDS_DRIVE_TYPE_DA, "Drevtype" },
/* de */ { IDS_DRIVE_TYPE_DE, "Gerätetyp" },
/* es */ { IDS_DRIVE_TYPE_ES, "Tipo unidad disco" },
/* fr */ { IDS_DRIVE_TYPE_FR, "Type de lecteur" },
/* hu */ { IDS_DRIVE_TYPE_HU, "Lemezegység típus" },
/* it */ { IDS_DRIVE_TYPE_IT, "Tipo drive" },
/* ko */ { IDS_DRIVE_TYPE_KO, "µå¶óÀÌºê Å¸ÀÔ" },
/* nl */ { IDS_DRIVE_TYPE_NL, "Drivesoort" },
/* pl */ { IDS_DRIVE_TYPE_PL, "Typ napêdu" },
/* ru */ { IDS_DRIVE_TYPE_RU, "Drive type" },
/* sv */ { IDS_DRIVE_TYPE_SV, "Enhetstyp" },
/* tr */ { IDS_DRIVE_TYPE_TR, "Sürücü tipi" },

/* en */ { IDS_40_TRACK_HANDLING,    "40 track handling" },
/* da */ { IDS_40_TRACK_HANDLING_DA, "Håndtering af 40 spor" },
/* de */ { IDS_40_TRACK_HANDLING_DE, "40 Spur Umgang" },
/* es */ { IDS_40_TRACK_HANDLING_ES, "Manejador 40 pistas" },
/* fr */ { IDS_40_TRACK_HANDLING_FR, "Prise en charge du 40 pistes" },
/* hu */ { IDS_40_TRACK_HANDLING_HU, "40 sáv kezelése" },
/* it */ { IDS_40_TRACK_HANDLING_IT, "Gestione 40 tracce" },
/* ko */ { IDS_40_TRACK_HANDLING_KO, "40 Æ®·¢ ÇÚµå¸µ" },
/* nl */ { IDS_40_TRACK_HANDLING_NL, "40-sporenondersteuning" },
/* pl */ { IDS_40_TRACK_HANDLING_PL, "Obs³uga 40 ¶cie¿ek" },
/* ru */ { IDS_40_TRACK_HANDLING_RU, "40 track handling" },
/* sv */ { IDS_40_TRACK_HANDLING_SV, "Hantering av 40 spår" },
/* tr */ { IDS_40_TRACK_HANDLING_TR, "40 track iþleme" },

/* en */ { IDS_DRIVE_EXPANSION,    "Drive expansion" },
/* da */ { IDS_DRIVE_EXPANSION_DA, "Drevudviddelse" },
/* de */ { IDS_DRIVE_EXPANSION_DE, "Laufwerk Erweiterung" },
/* es */ { IDS_DRIVE_EXPANSION_ES, "Expansión Unidad disco" },
/* fr */ { IDS_DRIVE_EXPANSION_FR, "Expansion de lecteur" },
/* hu */ { IDS_DRIVE_EXPANSION_HU, "Lemezegység bõvítés" },
/* it */ { IDS_DRIVE_EXPANSION_IT, "Espasione del drive" },
/* ko */ { IDS_DRIVE_EXPANSION_KO, "µå¶óÀÌºê È®Àå" },
/* nl */ { IDS_DRIVE_EXPANSION_NL, "Drive uitbreiding" },
/* pl */ { IDS_DRIVE_EXPANSION_PL, "Rozszerzenie napêdu" },
/* ru */ { IDS_DRIVE_EXPANSION_RU, "Drive expansion" },
/* sv */ { IDS_DRIVE_EXPANSION_SV, "Enhetsutökning" },
/* tr */ { IDS_DRIVE_EXPANSION_TR, "Sürücü geniþlemesi" },

/* en */ { IDS_IDLE_METHOD,    "Idle method" },
/* da */ { IDS_IDLE_METHOD_DA, "Tomgangsmetode" },
/* de */ { IDS_IDLE_METHOD_DE, "Idle Methode" },
/* es */ { IDS_IDLE_METHOD_ES, "Método tiempo inactivo" },
/* fr */ { IDS_IDLE_METHOD_FR, "Méthode d'idle" },
/* hu */ { IDS_IDLE_METHOD_HU, "Üresjárat mód" },
/* it */ { IDS_IDLE_METHOD_IT, "Rileva inattività" },
/* ko */ { IDS_IDLE_METHOD_KO, "¹æ¹ý" },
/* nl */ { IDS_IDLE_METHOD_NL, "Idlemethode" },
/* pl */ { IDS_IDLE_METHOD_PL, "Metoda bezczynno¶ci" },
/* ru */ { IDS_IDLE_METHOD_RU, "Idle method" },
/* sv */ { IDS_IDLE_METHOD_SV, "Pausmetod" },
/* tr */ { IDS_IDLE_METHOD_TR, "Boþa vakit geçirme metodu" },

/* en */ { IDS_PARALLEL_CABLE,    "Parallel cable" },
/* da */ { IDS_PARALLEL_CABLE_DA, "Parallelkabel" },
/* de */ { IDS_PARALLEL_CABLE_DE, "Parallelkabel" },
/* es */ { IDS_PARALLEL_CABLE_ES, "Cable paralelo" },
/* fr */ { IDS_PARALLEL_CABLE_FR, "Câble parallèle" },
/* hu */ { IDS_PARALLEL_CABLE_HU, "Párhuzamos kábel" },
/* it */ { IDS_PARALLEL_CABLE_IT, "Cavo parallelo" },
/* ko */ { IDS_PARALLEL_CABLE_KO, "ÆòÇà Ä³ÀÌºí" },
/* nl */ { IDS_PARALLEL_CABLE_NL, "Parallelle kabel" },
/* pl */ { IDS_PARALLEL_CABLE_PL, "£±cze portu równoleg³ego" },
/* ru */ { IDS_PARALLEL_CABLE_RU, "Parallel cable" },
/* sv */ { IDS_PARALLEL_CABLE_SV, "Parallellkabel" },
/* tr */ { IDS_PARALLEL_CABLE_TR, "Paralel kablo" },

/* en */ { IDS_PROFESSIONAL_DOS,    "Professional DOS" },
/* da */ { IDS_PROFESSIONAL_DOS_DA, "" },  /* fuzzy */
/* de */ { IDS_PROFESSIONAL_DOS_DE, "Professional DOS" },
/* es */ { IDS_PROFESSIONAL_DOS_ES, "DOS profesional" },
/* fr */ { IDS_PROFESSIONAL_DOS_FR, "" },  /* fuzzy */
/* hu */ { IDS_PROFESSIONAL_DOS_HU, "" },  /* fuzzy */
/* it */ { IDS_PROFESSIONAL_DOS_IT, "Professional DOS" },
/* ko */ { IDS_PROFESSIONAL_DOS_KO, "" },  /* fuzzy */
/* nl */ { IDS_PROFESSIONAL_DOS_NL, "Professional DOS" },
/* pl */ { IDS_PROFESSIONAL_DOS_PL, "Professional DOS" },
/* ru */ { IDS_PROFESSIONAL_DOS_RU, "" },  /* fuzzy */
/* sv */ { IDS_PROFESSIONAL_DOS_SV, "" },  /* fuzzy */
/* tr */ { IDS_PROFESSIONAL_DOS_TR, "" },  /* fuzzy */

/* en */ { IDS_SUPERCARD_PLUS,    "SuperCard+" },
/* da */ { IDS_SUPERCARD_PLUS_DA, "" },  /* fuzzy */
/* de */ { IDS_SUPERCARD_PLUS_DE, "SuperCard+" },
/* es */ { IDS_SUPERCARD_PLUS_ES, "SuperCard+" },
/* fr */ { IDS_SUPERCARD_PLUS_FR, "" },  /* fuzzy */
/* hu */ { IDS_SUPERCARD_PLUS_HU, "" },  /* fuzzy */
/* it */ { IDS_SUPERCARD_PLUS_IT, "SuperCard+" },
/* ko */ { IDS_SUPERCARD_PLUS_KO, "" },  /* fuzzy */
/* nl */ { IDS_SUPERCARD_PLUS_NL, "SuperCard+" },
/* pl */ { IDS_SUPERCARD_PLUS_PL, "SuperCard+" },
/* ru */ { IDS_SUPERCARD_PLUS_RU, "" },  /* fuzzy */
/* sv */ { IDS_SUPERCARD_PLUS_SV, "" },  /* fuzzy */
/* tr */ { IDS_SUPERCARD_PLUS_TR, "" },  /* fuzzy */

/* en */ { IDS_DRIVE_RTC_SAVE,    "Enable FD2000/4000 RTC data saving" },
/* da */ { IDS_DRIVE_RTC_SAVE_DA, "" },  /* fuzzy */
/* de */ { IDS_DRIVE_RTC_SAVE_DE, "FD2000/4000 RTC Datenspeichung aktivieren" },
/* es */ { IDS_DRIVE_RTC_SAVE_ES, "Permitir grabar datos  FD2000/4000 RTC" },
/* fr */ { IDS_DRIVE_RTC_SAVE_FR, "" },  /* fuzzy */
/* hu */ { IDS_DRIVE_RTC_SAVE_HU, "" },  /* fuzzy */
/* it */ { IDS_DRIVE_RTC_SAVE_IT, "Attiva il salvataggio dei dati RTC del FD2000/4000" },
/* ko */ { IDS_DRIVE_RTC_SAVE_KO, "" },  /* fuzzy */
/* nl */ { IDS_DRIVE_RTC_SAVE_NL, "Opslaan van de FD2000/4000 RTC gegevens inschakelen" },
/* pl */ { IDS_DRIVE_RTC_SAVE_PL, "W³±cz zapis zegara FD2000/4000" },
/* ru */ { IDS_DRIVE_RTC_SAVE_RU, "" },  /* fuzzy */
/* sv */ { IDS_DRIVE_RTC_SAVE_SV, "" },  /* fuzzy */
/* tr */ { IDS_DRIVE_RTC_SAVE_TR, "" },  /* fuzzy */

/* en */ { IDS_DRIVE_RPM,    "RPM (25000..35000)" },
/* da */ { IDS_DRIVE_RPM_DA, "" },  /* fuzzy */
/* de */ { IDS_DRIVE_RPM_DE, "RPM (25000..35000)" },
/* es */ { IDS_DRIVE_RPM_ES, "" },  /* fuzzy */
/* fr */ { IDS_DRIVE_RPM_FR, "" },  /* fuzzy */
/* hu */ { IDS_DRIVE_RPM_HU, "" },  /* fuzzy */
/* it */ { IDS_DRIVE_RPM_IT, "" },  /* fuzzy */
/* ko */ { IDS_DRIVE_RPM_KO, "" },  /* fuzzy */
/* nl */ { IDS_DRIVE_RPM_NL, "RPM (25000..35000)" },
/* pl */ { IDS_DRIVE_RPM_PL, "" },  /* fuzzy */
/* ru */ { IDS_DRIVE_RPM_RU, "" },  /* fuzzy */
/* sv */ { IDS_DRIVE_RPM_SV, "" },  /* fuzzy */
/* tr */ { IDS_DRIVE_RPM_TR, "" },  /* fuzzy */

/* en */ { IDS_DRIVE_WOBBLE,    "Wobble (0..1000)" },
/* da */ { IDS_DRIVE_WOBBLE_DA, "" },  /* fuzzy */
/* de */ { IDS_DRIVE_WOBBLE_DE, "Schwankung (0..1000)" },
/* es */ { IDS_DRIVE_WOBBLE_ES, "" },  /* fuzzy */
/* fr */ { IDS_DRIVE_WOBBLE_FR, "" },  /* fuzzy */
/* hu */ { IDS_DRIVE_WOBBLE_HU, "" },  /* fuzzy */
/* it */ { IDS_DRIVE_WOBBLE_IT, "" },  /* fuzzy */
/* ko */ { IDS_DRIVE_WOBBLE_KO, "" },  /* fuzzy */
/* nl */ { IDS_DRIVE_WOBBLE_NL, "Wiebel (0..1000)" },
/* pl */ { IDS_DRIVE_WOBBLE_PL, "" },  /* fuzzy */
/* ru */ { IDS_DRIVE_WOBBLE_RU, "" },  /* fuzzy */
/* sv */ { IDS_DRIVE_WOBBLE_SV, "" },  /* fuzzy */
/* tr */ { IDS_DRIVE_WOBBLE_TR, "" },  /* fuzzy */

/* en */ { IDS_DRIVE_SETTINGS,    "Drive settings" },
/* da */ { IDS_DRIVE_SETTINGS_DA, "Diskettedrevsindstillinger" },
/* de */ { IDS_DRIVE_SETTINGS_DE, "Floppy Einstellungen" },
/* es */ { IDS_DRIVE_SETTINGS_ES, "Ajustes unidad disco" },
/* fr */ { IDS_DRIVE_SETTINGS_FR, "Paramètres des lecteurs" },
/* hu */ { IDS_DRIVE_SETTINGS_HU, "Lemezegység beállításai" },
/* it */ { IDS_DRIVE_SETTINGS_IT, "Impostazioni drive" },
/* ko */ { IDS_DRIVE_SETTINGS_KO, "µå¶óÀÌºê ¼ÂÆÃ" },
/* nl */ { IDS_DRIVE_SETTINGS_NL, "Driveinstellingen" },
/* pl */ { IDS_DRIVE_SETTINGS_PL, "Ustawienia napêdu" },
/* ru */ { IDS_DRIVE_SETTINGS_RU, "Drive settings" },
/* sv */ { IDS_DRIVE_SETTINGS_SV, "Diskettenhetsinställningar" },
/* tr */ { IDS_DRIVE_SETTINGS_TR, "Sürücü ayarlarý" },

/* en */ { IDS_GEORAM_SIZE,    "GEO-RAM size" },
/* da */ { IDS_GEORAM_SIZE_DA, "GEO-RAM-størrelse" },
/* de */ { IDS_GEORAM_SIZE_DE, "GEO-RAM Größe" },
/* es */ { IDS_GEORAM_SIZE_ES, "Tamaño GEO-RAM" },
/* fr */ { IDS_GEORAM_SIZE_FR, "Taille du GEO-RAM" },
/* hu */ { IDS_GEORAM_SIZE_HU, "" },  /* fuzzy */
/* it */ { IDS_GEORAM_SIZE_IT, "Dimensione GEO-RAM" },
/* ko */ { IDS_GEORAM_SIZE_KO, "" },  /* fuzzy */
/* nl */ { IDS_GEORAM_SIZE_NL, "GEO-RAM grootte" },
/* pl */ { IDS_GEORAM_SIZE_PL, "Rozmiar GEO-RAM" },
/* ru */ { IDS_GEORAM_SIZE_RU, "" },  /* fuzzy */
/* sv */ { IDS_GEORAM_SIZE_SV, "GEO-RAM-storlek" },
/* tr */ { IDS_GEORAM_SIZE_TR, "GEO-RAM boyutu" },

/* en */ { IDS_MASCUERADE_IO_SWAP,    "MasC=uerade I/O swap" },
/* da */ { IDS_MASCUERADE_IO_SWAP_DA, "MasC=uerade I/O ombytning" },
/* de */ { IDS_MASCUERADE_IO_SWAP_DE, "MasC=uerade I/O swap" },
/* es */ { IDS_MASCUERADE_IO_SWAP_ES, "Intercambio máscara Entrada/Salida" },
/* fr */ { IDS_MASCUERADE_IO_SWAP_FR, "Permutation E/S MasC=uerade" },
/* hu */ { IDS_MASCUERADE_IO_SWAP_HU, "" },  /* fuzzy */
/* it */ { IDS_MASCUERADE_IO_SWAP_IT, "I/O swap MasC=uerade" },
/* ko */ { IDS_MASCUERADE_IO_SWAP_KO, "MasC=uerade I/O ½º¿Ò" },
/* nl */ { IDS_MASCUERADE_IO_SWAP_NL, "MasC=uerade I/O verwisseling" },
/* pl */ { IDS_MASCUERADE_IO_SWAP_PL, "Zamiana we/wy MasC=uerade" },
/* ru */ { IDS_MASCUERADE_IO_SWAP_RU, "MasC=uerade I/O swap" },
/* sv */ { IDS_MASCUERADE_IO_SWAP_SV, "MasC=uerade I/O-växling" },
/* tr */ { IDS_MASCUERADE_IO_SWAP_TR, "MasC=uerade G/Ç deðiþimi" },

/* en */ { IDS_GEORAM_FILENAME,    "GEO-RAM file" },
/* da */ { IDS_GEORAM_FILENAME_DA, "GEO-RAM-fil" },
/* de */ { IDS_GEORAM_FILENAME_DE, "GEO-RAM Datei" },
/* es */ { IDS_GEORAM_FILENAME_ES, "Fulcro GEO-RAM" },
/* fr */ { IDS_GEORAM_FILENAME_FR, "Fichier GEO-RAM" },
/* hu */ { IDS_GEORAM_FILENAME_HU, "" },  /* fuzzy */
/* it */ { IDS_GEORAM_FILENAME_IT, "File GEO-RAM" },
/* ko */ { IDS_GEORAM_FILENAME_KO, "GEO-RAM ÆÄÀÏ" },
/* nl */ { IDS_GEORAM_FILENAME_NL, "GEO-RAM bestand" },
/* pl */ { IDS_GEORAM_FILENAME_PL, "Plik GEO-RAM" },
/* ru */ { IDS_GEORAM_FILENAME_RU, "GEO-RAM file" },
/* sv */ { IDS_GEORAM_FILENAME_SV, "GEO-RAM-fil" },
/* tr */ { IDS_GEORAM_FILENAME_TR, "GEO-RAM dosyasý" },

/* en */ { IDS_SAVE_GEORAM_IMAGE_WHEN_CHANGED,    "Save GEO-RAM image when changed" },
/* da */ { IDS_SAVE_GEORAM_IMAGE_WHEN_CHANGED_DA, "Gem GEO-RAM image ved ændringer" },
/* de */ { IDS_SAVE_GEORAM_IMAGE_WHEN_CHANGED_DE, "GEO-RAM Imagedatei bei Änderung speichern" },
/* es */ { IDS_SAVE_GEORAM_IMAGE_WHEN_CHANGED_ES, "Grabar imagen GEO-RAM cuando cambie" },
/* fr */ { IDS_SAVE_GEORAM_IMAGE_WHEN_CHANGED_FR, "Enregistrer l'image GEO-RAM lorsque modifiée" },
/* hu */ { IDS_SAVE_GEORAM_IMAGE_WHEN_CHANGED_HU, "" },  /* fuzzy */
/* it */ { IDS_SAVE_GEORAM_IMAGE_WHEN_CHANGED_IT, "Salva immagine GEO-RAM alla modifica" },
/* ko */ { IDS_SAVE_GEORAM_IMAGE_WHEN_CHANGED_KO, "" },  /* fuzzy */
/* nl */ { IDS_SAVE_GEORAM_IMAGE_WHEN_CHANGED_NL, "GEO-RAM bestand opslaan bij wijziging" },
/* pl */ { IDS_SAVE_GEORAM_IMAGE_WHEN_CHANGED_PL, "Zapisuj obraz GEO-RAM przy zmianie" },
/* ru */ { IDS_SAVE_GEORAM_IMAGE_WHEN_CHANGED_RU, "Save GEO-RAM image when changed" },
/* sv */ { IDS_SAVE_GEORAM_IMAGE_WHEN_CHANGED_SV, "Spara GEO-RAM-avbildning vid ändringar" },
/* tr */ { IDS_SAVE_GEORAM_IMAGE_WHEN_CHANGED_TR, "GEO-RAM imajý deðiþtiðinde kaydet" },

/* en */ { IDS_GEORAM_FILENAME_SELECT,    "Select file for GEO-RAM" },
/* da */ { IDS_GEORAM_FILENAME_SELECT_DA, "Vælg fil for GEO-RAM" },
/* de */ { IDS_GEORAM_FILENAME_SELECT_DE, "Datei für GEO-RAM auswählen" },
/* es */ { IDS_GEORAM_FILENAME_SELECT_ES, "Seleccionar fichero para GEO-RAM" },
/* fr */ { IDS_GEORAM_FILENAME_SELECT_FR, "Sélectionner le fichier GEO-RAM" },
/* hu */ { IDS_GEORAM_FILENAME_SELECT_HU, "" },  /* fuzzy */
/* it */ { IDS_GEORAM_FILENAME_SELECT_IT, "Seleziona file per GEO-RAM" },
/* ko */ { IDS_GEORAM_FILENAME_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDS_GEORAM_FILENAME_SELECT_NL, "Selecteer bestand voor GEO-RAM" },
/* pl */ { IDS_GEORAM_FILENAME_SELECT_PL, "Wybierz plik GEO-RAM" },
/* ru */ { IDS_GEORAM_FILENAME_SELECT_RU, "Select file for GEO-RAM" },
/* sv */ { IDS_GEORAM_FILENAME_SELECT_SV, "Välj fil för GEO-RAM" },
/* tr */ { IDS_GEORAM_FILENAME_SELECT_TR, "GEO-RAM için dosya seçin" },

/* en */ { IDS_GEORAM_SETTINGS,    "GEO-RAM settings" },
/* da */ { IDS_GEORAM_SETTINGS_DA, "GEO-RAM-indstillinger" },
/* de */ { IDS_GEORAM_SETTINGS_DE, "GEO-RAM Einstellungen" },
/* es */ { IDS_GEORAM_SETTINGS_ES, "Ajustes GEO-RAM" },
/* fr */ { IDS_GEORAM_SETTINGS_FR, "Paramètres GEO-RAM" },
/* hu */ { IDS_GEORAM_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_GEORAM_SETTINGS_IT, "Impostazioni GEO-RAM" },
/* ko */ { IDS_GEORAM_SETTINGS_KO, "GEO-RAM ¼ÂÆÃ" },
/* nl */ { IDS_GEORAM_SETTINGS_NL, "GEO-RAM instellingen" },
/* pl */ { IDS_GEORAM_SETTINGS_PL, "Ustawienia GEO-RAM" },
/* ru */ { IDS_GEORAM_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_GEORAM_SETTINGS_SV, "GEO-RAM-inställningar" },
/* tr */ { IDS_GEORAM_SETTINGS_TR, "GEO-RAM ayarlarý" },

/* en */ { IDS_USERPORT_DS1307_RTC_ENABLE,    "Enable Userport DS1307 RTC" },
/* da */ { IDS_USERPORT_DS1307_RTC_ENABLE_DA, "" },  /* fuzzy */
/* de */ { IDS_USERPORT_DS1307_RTC_ENABLE_DE, "Userport DS1307 RTC aktivieren" },
/* es */ { IDS_USERPORT_DS1307_RTC_ENABLE_ES, "" },  /* fuzzy */
/* fr */ { IDS_USERPORT_DS1307_RTC_ENABLE_FR, "" },  /* fuzzy */
/* hu */ { IDS_USERPORT_DS1307_RTC_ENABLE_HU, "" },  /* fuzzy */
/* it */ { IDS_USERPORT_DS1307_RTC_ENABLE_IT, "" },  /* fuzzy */
/* ko */ { IDS_USERPORT_DS1307_RTC_ENABLE_KO, "" },  /* fuzzy */
/* nl */ { IDS_USERPORT_DS1307_RTC_ENABLE_NL, "Userport DS1307 RTC inschakelen" },
/* pl */ { IDS_USERPORT_DS1307_RTC_ENABLE_PL, "" },  /* fuzzy */
/* ru */ { IDS_USERPORT_DS1307_RTC_ENABLE_RU, "" },  /* fuzzy */
/* sv */ { IDS_USERPORT_DS1307_RTC_ENABLE_SV, "" },  /* fuzzy */
/* tr */ { IDS_USERPORT_DS1307_RTC_ENABLE_TR, "" },  /* fuzzy */

/* en */ { IDS_SAVE_USERPORT_DS1307_RTC_DATA,    "Save Userport DS1307 RTC data when changed" },
/* da */ { IDS_SAVE_USERPORT_DS1307_RTC_DATA_DA, "" },  /* fuzzy */
/* de */ { IDS_SAVE_USERPORT_DS1307_RTC_DATA_DE, "DS1307 RTC Daten bei Änderung speichern" },
/* es */ { IDS_SAVE_USERPORT_DS1307_RTC_DATA_ES, "" },  /* fuzzy */
/* fr */ { IDS_SAVE_USERPORT_DS1307_RTC_DATA_FR, "" },  /* fuzzy */
/* hu */ { IDS_SAVE_USERPORT_DS1307_RTC_DATA_HU, "" },  /* fuzzy */
/* it */ { IDS_SAVE_USERPORT_DS1307_RTC_DATA_IT, "" },  /* fuzzy */
/* ko */ { IDS_SAVE_USERPORT_DS1307_RTC_DATA_KO, "" },  /* fuzzy */
/* nl */ { IDS_SAVE_USERPORT_DS1307_RTC_DATA_NL, "Sla de Userport DS1307 RTC gegevens op bij wijziging" },
/* pl */ { IDS_SAVE_USERPORT_DS1307_RTC_DATA_PL, "" },  /* fuzzy */
/* ru */ { IDS_SAVE_USERPORT_DS1307_RTC_DATA_RU, "" },  /* fuzzy */
/* sv */ { IDS_SAVE_USERPORT_DS1307_RTC_DATA_SV, "" },  /* fuzzy */
/* tr */ { IDS_SAVE_USERPORT_DS1307_RTC_DATA_TR, "" },  /* fuzzy */

/* en */ { IDS_USERPORT_DS1307_RTC_SETTINGS,    "Userport DS1307 RTC settings" },
/* da */ { IDS_USERPORT_DS1307_RTC_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDS_USERPORT_DS1307_RTC_SETTINGS_DE, "Userport DS1307 RTC Einstellungen" },
/* es */ { IDS_USERPORT_DS1307_RTC_SETTINGS_ES, "" },  /* fuzzy */
/* fr */ { IDS_USERPORT_DS1307_RTC_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDS_USERPORT_DS1307_RTC_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_USERPORT_DS1307_RTC_SETTINGS_IT, "" },  /* fuzzy */
/* ko */ { IDS_USERPORT_DS1307_RTC_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_USERPORT_DS1307_RTC_SETTINGS_NL, "Userport DS1307 RTC instellingen" },
/* pl */ { IDS_USERPORT_DS1307_RTC_SETTINGS_PL, "" },  /* fuzzy */
/* ru */ { IDS_USERPORT_DS1307_RTC_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_USERPORT_DS1307_RTC_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDS_USERPORT_DS1307_RTC_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDS_USERPORT_RTC58321A_ENABLE,    "Enable Userport RTC58321a" },
/* da */ { IDS_USERPORT_RTC58321A_ENABLE_DA, "" },  /* fuzzy */
/* de */ { IDS_USERPORT_RTC58321A_ENABLE_DE, "Userport RTC58321a aktivieren" },
/* es */ { IDS_USERPORT_RTC58321A_ENABLE_ES, "" },  /* fuzzy */
/* fr */ { IDS_USERPORT_RTC58321A_ENABLE_FR, "" },  /* fuzzy */
/* hu */ { IDS_USERPORT_RTC58321A_ENABLE_HU, "" },  /* fuzzy */
/* it */ { IDS_USERPORT_RTC58321A_ENABLE_IT, "" },  /* fuzzy */
/* ko */ { IDS_USERPORT_RTC58321A_ENABLE_KO, "" },  /* fuzzy */
/* nl */ { IDS_USERPORT_RTC58321A_ENABLE_NL, "Userport RTC58321a inschakelen" },
/* pl */ { IDS_USERPORT_RTC58321A_ENABLE_PL, "" },  /* fuzzy */
/* ru */ { IDS_USERPORT_RTC58321A_ENABLE_RU, "" },  /* fuzzy */
/* sv */ { IDS_USERPORT_RTC58321A_ENABLE_SV, "" },  /* fuzzy */
/* tr */ { IDS_USERPORT_RTC58321A_ENABLE_TR, "" },  /* fuzzy */

/* en */ { IDS_SAVE_USERPORT_RTC58321A_DATA,    "Save Userport RTC58321a data when changed" },
/* da */ { IDS_SAVE_USERPORT_RTC58321A_DATA_DA, "" },  /* fuzzy */
/* de */ { IDS_SAVE_USERPORT_RTC58321A_DATA_DE, "Userport RTC58321a Daten bei Änderung speichern" },
/* es */ { IDS_SAVE_USERPORT_RTC58321A_DATA_ES, "" },  /* fuzzy */
/* fr */ { IDS_SAVE_USERPORT_RTC58321A_DATA_FR, "" },  /* fuzzy */
/* hu */ { IDS_SAVE_USERPORT_RTC58321A_DATA_HU, "" },  /* fuzzy */
/* it */ { IDS_SAVE_USERPORT_RTC58321A_DATA_IT, "" },  /* fuzzy */
/* ko */ { IDS_SAVE_USERPORT_RTC58321A_DATA_KO, "" },  /* fuzzy */
/* nl */ { IDS_SAVE_USERPORT_RTC58321A_DATA_NL, "Sla de Userport RTC58321a gegevens op bij wijziging" },
/* pl */ { IDS_SAVE_USERPORT_RTC58321A_DATA_PL, "" },  /* fuzzy */
/* ru */ { IDS_SAVE_USERPORT_RTC58321A_DATA_RU, "" },  /* fuzzy */
/* sv */ { IDS_SAVE_USERPORT_RTC58321A_DATA_SV, "" },  /* fuzzy */
/* tr */ { IDS_SAVE_USERPORT_RTC58321A_DATA_TR, "" },  /* fuzzy */

/* en */ { IDS_USERPORT_RTC58321A_SETTINGS,    "Userport RTC58321a settings" },
/* da */ { IDS_USERPORT_RTC58321A_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDS_USERPORT_RTC58321A_SETTINGS_DE, "Userport RTC58321a Einstellungen" },
/* es */ { IDS_USERPORT_RTC58321A_SETTINGS_ES, "" },  /* fuzzy */
/* fr */ { IDS_USERPORT_RTC58321A_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDS_USERPORT_RTC58321A_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_USERPORT_RTC58321A_SETTINGS_IT, "" },  /* fuzzy */
/* ko */ { IDS_USERPORT_RTC58321A_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_USERPORT_RTC58321A_SETTINGS_NL, "Userport RTC58321a instellingen" },
/* pl */ { IDS_USERPORT_RTC58321A_SETTINGS_PL, "" },  /* fuzzy */
/* ru */ { IDS_USERPORT_RTC58321A_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_USERPORT_RTC58321A_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDS_USERPORT_RTC58321A_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDS_SAMPLER_FILENAME_SELECT,    "Select file to be used as sampler input" },
/* da */ { IDS_SAMPLER_FILENAME_SELECT_DA, "" },  /* fuzzy */
/* de */ { IDS_SAMPLER_FILENAME_SELECT_DE, "Datei für Sample Input auswählen" },
/* es */ { IDS_SAMPLER_FILENAME_SELECT_ES, "" },  /* fuzzy */
/* fr */ { IDS_SAMPLER_FILENAME_SELECT_FR, "" },  /* fuzzy */
/* hu */ { IDS_SAMPLER_FILENAME_SELECT_HU, "" },  /* fuzzy */
/* it */ { IDS_SAMPLER_FILENAME_SELECT_IT, "" },  /* fuzzy */
/* ko */ { IDS_SAMPLER_FILENAME_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDS_SAMPLER_FILENAME_SELECT_NL, "Selecteer het bestand dat gebruikt gaat worden als sampler invoer" },
/* pl */ { IDS_SAMPLER_FILENAME_SELECT_PL, "" },  /* fuzzy */
/* ru */ { IDS_SAMPLER_FILENAME_SELECT_RU, "" },  /* fuzzy */
/* sv */ { IDS_SAMPLER_FILENAME_SELECT_SV, "" },  /* fuzzy */
/* tr */ { IDS_SAMPLER_FILENAME_SELECT_TR, "" },  /* fuzzy */

/* en */ { IDS_SAMPLER_DEVICE,    "Sampler device" },
/* da */ { IDS_SAMPLER_DEVICE_DA, "" },  /* fuzzy */
/* de */ { IDS_SAMPLER_DEVICE_DE, "Sampler Gerät" },
/* es */ { IDS_SAMPLER_DEVICE_ES, "" },  /* fuzzy */
/* fr */ { IDS_SAMPLER_DEVICE_FR, "" },  /* fuzzy */
/* hu */ { IDS_SAMPLER_DEVICE_HU, "" },  /* fuzzy */
/* it */ { IDS_SAMPLER_DEVICE_IT, "" },  /* fuzzy */
/* ko */ { IDS_SAMPLER_DEVICE_KO, "" },  /* fuzzy */
/* nl */ { IDS_SAMPLER_DEVICE_NL, "Sampler apparaat" },
/* pl */ { IDS_SAMPLER_DEVICE_PL, "" },  /* fuzzy */
/* ru */ { IDS_SAMPLER_DEVICE_RU, "" },  /* fuzzy */
/* sv */ { IDS_SAMPLER_DEVICE_SV, "" },  /* fuzzy */
/* tr */ { IDS_SAMPLER_DEVICE_TR, "" },  /* fuzzy */

/* en */ { IDS_SAMPLER_FILENAME,    "Sampler input media file" },
/* da */ { IDS_SAMPLER_FILENAME_DA, "" },  /* fuzzy */
/* de */ { IDS_SAMPLER_FILENAME_DE, "Sample Input Mediendatei" },
/* es */ { IDS_SAMPLER_FILENAME_ES, "" },  /* fuzzy */
/* fr */ { IDS_SAMPLER_FILENAME_FR, "" },  /* fuzzy */
/* hu */ { IDS_SAMPLER_FILENAME_HU, "" },  /* fuzzy */
/* it */ { IDS_SAMPLER_FILENAME_IT, "" },  /* fuzzy */
/* ko */ { IDS_SAMPLER_FILENAME_KO, "" },  /* fuzzy */
/* nl */ { IDS_SAMPLER_FILENAME_NL, "Sampler invoer mediabestand" },
/* pl */ { IDS_SAMPLER_FILENAME_PL, "" },  /* fuzzy */
/* ru */ { IDS_SAMPLER_FILENAME_RU, "" },  /* fuzzy */
/* sv */ { IDS_SAMPLER_FILENAME_SV, "" },  /* fuzzy */
/* tr */ { IDS_SAMPLER_FILENAME_TR, "" },  /* fuzzy */

/* en */ { IDS_SAMPLER_SETTINGS,    "Sampler settings" },
/* da */ { IDS_SAMPLER_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDS_SAMPLER_SETTINGS_DE, "Sampler Einstellungen" },
/* es */ { IDS_SAMPLER_SETTINGS_ES, "" },  /* fuzzy */
/* fr */ { IDS_SAMPLER_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDS_SAMPLER_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_SAMPLER_SETTINGS_IT, "" },  /* fuzzy */
/* ko */ { IDS_SAMPLER_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_SAMPLER_SETTINGS_NL, "Sampler instellingen" },
/* pl */ { IDS_SAMPLER_SETTINGS_PL, "" },  /* fuzzy */
/* ru */ { IDS_SAMPLER_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_SAMPLER_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDS_SAMPLER_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDS_EXPERT_MODE,    "Expert Cartridge mode" },
/* da */ { IDS_EXPERT_MODE_DA, "Expert Cartridge-tilstand" },
/* de */ { IDS_EXPERT_MODE_DE, "Expert Cartridge Modus" },
/* es */ { IDS_EXPERT_MODE_ES, "Modo Expert Cartridge" },
/* fr */ { IDS_EXPERT_MODE_FR, "Mode Cartouche Expert" },
/* hu */ { IDS_EXPERT_MODE_HU, "" },  /* fuzzy */
/* it */ { IDS_EXPERT_MODE_IT, "Modalità Expert Cartridge" },
/* ko */ { IDS_EXPERT_MODE_KO, "" },  /* fuzzy */
/* nl */ { IDS_EXPERT_MODE_NL, "Expert Cartridge modus" },
/* pl */ { IDS_EXPERT_MODE_PL, "Tryb Expert Cartridge" },
/* ru */ { IDS_EXPERT_MODE_RU, "" },  /* fuzzy */
/* sv */ { IDS_EXPERT_MODE_SV, "Expert Cartridge-läge" },
/* tr */ { IDS_EXPERT_MODE_TR, "Expert Kartuþu modu" },

/* en */ { IDS_OFF,    "Off" },
/* da */ { IDS_OFF_DA, "Slukket" },
/* de */ { IDS_OFF_DE, "Aus" },
/* es */ { IDS_OFF_ES, "Apagado" },
/* fr */ { IDS_OFF_FR, "Off" },
/* hu */ { IDS_OFF_HU, "Ki" },
/* it */ { IDS_OFF_IT, "No" },
/* ko */ { IDS_OFF_KO, "²¨Áü" },
/* nl */ { IDS_OFF_NL, "Uit" },
/* pl */ { IDS_OFF_PL, "Wy³±cz" },
/* ru */ { IDS_OFF_RU, "Off" },
/* sv */ { IDS_OFF_SV, "Av" },
/* tr */ { IDS_OFF_TR, "Kapalý" },

/* en */ { IDS_PRG,    "Prg" },
/* da */ { IDS_PRG_DA, "Prg" },
/* de */ { IDS_PRG_DE, "Prg" },
/* es */ { IDS_PRG_ES, "Prg" },
/* fr */ { IDS_PRG_FR, "Prg" },
/* hu */ { IDS_PRG_HU, "Prg" },
/* it */ { IDS_PRG_IT, "Prg" },
/* ko */ { IDS_PRG_KO, "Prg" },
/* nl */ { IDS_PRG_NL, "Prg" },
/* pl */ { IDS_PRG_PL, "Prg" },
/* ru */ { IDS_PRG_RU, "Prg" },
/* sv */ { IDS_PRG_SV, "Prg" },
/* tr */ { IDS_PRG_TR, "Prg" },

/* en */ { IDS_ON,    "On" },
/* da */ { IDS_ON_DA, "Tændt" },
/* de */ { IDS_ON_DE, "Ein" },
/* es */ { IDS_ON_ES, "Encendido" },
/* fr */ { IDS_ON_FR, "On" },
/* hu */ { IDS_ON_HU, "Be" },
/* it */ { IDS_ON_IT, "Sì" },
/* ko */ { IDS_ON_KO, "ÄÑÁü" },
/* nl */ { IDS_ON_NL, "Aan" },
/* pl */ { IDS_ON_PL, "W³±cz" },
/* ru */ { IDS_ON_RU, "On" },
/* sv */ { IDS_ON_SV, "På" },
/* tr */ { IDS_ON_TR, "Açýk" },

/* en */ { IDS_EXPERT_FILENAME,    "Expert Cartridge file" },
/* da */ { IDS_EXPERT_FILENAME_DA, "Expert Cartridge-fil" },
/* de */ { IDS_EXPERT_FILENAME_DE, "Expert Cartridge Imagedatei" },
/* es */ { IDS_EXPERT_FILENAME_ES, "Fichero Expert Cartrige" },
/* fr */ { IDS_EXPERT_FILENAME_FR, "Fichier Cartouche Expert" },
/* hu */ { IDS_EXPERT_FILENAME_HU, "" },  /* fuzzy */
/* it */ { IDS_EXPERT_FILENAME_IT, "File Expert Cartridge" },
/* ko */ { IDS_EXPERT_FILENAME_KO, "Expert Ä«Æ®¸®Áö ÆÄÀÏ" },
/* nl */ { IDS_EXPERT_FILENAME_NL, "Expert Cartridge-bestand" },
/* pl */ { IDS_EXPERT_FILENAME_PL, "Plik Expert Cartridge" },
/* ru */ { IDS_EXPERT_FILENAME_RU, "" },  /* fuzzy */
/* sv */ { IDS_EXPERT_FILENAME_SV, "Expert Cartridge-fil" },
/* tr */ { IDS_EXPERT_FILENAME_TR, "Expert Kartuþu dosyasý" },

/* en */ { IDS_SAVE_EXPERT_IMAGE_WHEN_CHANGED,    "Save Expert Cartridge image when changed" },
/* da */ { IDS_SAVE_EXPERT_IMAGE_WHEN_CHANGED_DA, "Gem Expert Cartridge image ved ændringer" },
/* de */ { IDS_SAVE_EXPERT_IMAGE_WHEN_CHANGED_DE, "Expert Cartridge Imagedatei bei Änderungen speichern" },
/* es */ { IDS_SAVE_EXPERT_IMAGE_WHEN_CHANGED_ES, "Grabar imagen Expert Cartridge cuando cambie" },
/* fr */ { IDS_SAVE_EXPERT_IMAGE_WHEN_CHANGED_FR, "Sauvegarder l'image Cartouche Expert lorsque modifiée" },
/* hu */ { IDS_SAVE_EXPERT_IMAGE_WHEN_CHANGED_HU, "" },  /* fuzzy */
/* it */ { IDS_SAVE_EXPERT_IMAGE_WHEN_CHANGED_IT, "Salva immagine Expert Cartridge alla modifica" },
/* ko */ { IDS_SAVE_EXPERT_IMAGE_WHEN_CHANGED_KO, "" },  /* fuzzy */
/* nl */ { IDS_SAVE_EXPERT_IMAGE_WHEN_CHANGED_NL, "Expert Cartridge bestand opslaan bij wijziging" },
/* pl */ { IDS_SAVE_EXPERT_IMAGE_WHEN_CHANGED_PL, "Zapisuj obraz Expert Cartridge przy zmianie" },
/* ru */ { IDS_SAVE_EXPERT_IMAGE_WHEN_CHANGED_RU, "Save Expert Cartridge image when changed" },
/* sv */ { IDS_SAVE_EXPERT_IMAGE_WHEN_CHANGED_SV, "Spara Expert Cartrdige-avbildning vid ändringar" },
/* tr */ { IDS_SAVE_EXPERT_IMAGE_WHEN_CHANGED_TR, "Expert Kartuþ imajýný deðiþtiðinde kaydet" },

/* en */ { IDS_EXPERT_FILENAME_SELECT,    "Select file for Expert Cartridge" },
/* da */ { IDS_EXPERT_FILENAME_SELECT_DA, "Vælg fil for Expert Cartridge" },
/* de */ { IDS_EXPERT_FILENAME_SELECT_DE, "Export Cartridge Imagedatei auswählen" },
/* es */ { IDS_EXPERT_FILENAME_SELECT_ES, "Seleccionar fichero para Expert Cartrige" },
/* fr */ { IDS_EXPERT_FILENAME_SELECT_FR, "Sélectionner fichier pour Cartouche Expert" },
/* hu */ { IDS_EXPERT_FILENAME_SELECT_HU, "" },  /* fuzzy */
/* it */ { IDS_EXPERT_FILENAME_SELECT_IT, "Seleziona file per Expert Cartridge" },
/* ko */ { IDS_EXPERT_FILENAME_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDS_EXPERT_FILENAME_SELECT_NL, "Selecteer bestand voor Expert Cartridge" },
/* pl */ { IDS_EXPERT_FILENAME_SELECT_PL, "Wybierz plik Expert Cartridge" },
/* ru */ { IDS_EXPERT_FILENAME_SELECT_RU, "" },  /* fuzzy */
/* sv */ { IDS_EXPERT_FILENAME_SELECT_SV, "Välj fil för Expert Cartridge" },
/* tr */ { IDS_EXPERT_FILENAME_SELECT_TR, "Expert Kartuþu için dosya seçin" },

/* en */ { IDS_EXPERT_SETTINGS,    "Expert Cartridge settings" },
/* da */ { IDS_EXPERT_SETTINGS_DA, "Expert Cartridge-indstillinger" },
/* de */ { IDS_EXPERT_SETTINGS_DE, "Expert Cartridge Einstellungen" },
/* es */ { IDS_EXPERT_SETTINGS_ES, "Ajustes Expert Cartrige" },
/* fr */ { IDS_EXPERT_SETTINGS_FR, "Paramètres Cartouche Expert" },
/* hu */ { IDS_EXPERT_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_EXPERT_SETTINGS_IT, "Impostazioni Expert Cartridge" },
/* ko */ { IDS_EXPERT_SETTINGS_KO, "Expert Ä«Æ®¸®Áö ¼ÂÆÃ" },
/* nl */ { IDS_EXPERT_SETTINGS_NL, "Expert Cartridge instellingen" },
/* pl */ { IDS_EXPERT_SETTINGS_PL, "Ustawienia Expert Cartridge" },
/* ru */ { IDS_EXPERT_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_EXPERT_SETTINGS_SV, "Inställningar för Expert Cartridge" },
/* tr */ { IDS_EXPERT_SETTINGS_TR, "Expert Kartuþu ayarlarý" },

/* en */ { IDS_DQBB_FILENAME,    "DQBB file" },
/* da */ { IDS_DQBB_FILENAME_DA, "DQBB-fil" },
/* de */ { IDS_DQBB_FILENAME_DE, "DQBB Datei" },
/* es */ { IDS_DQBB_FILENAME_ES, "Fichero DQBB" },
/* fr */ { IDS_DQBB_FILENAME_FR, "Fichier DQBB" },
/* hu */ { IDS_DQBB_FILENAME_HU, "DQBB fájl" },
/* it */ { IDS_DQBB_FILENAME_IT, "File DQBB" },
/* ko */ { IDS_DQBB_FILENAME_KO, "DQBB ÆÄÀÏ" },
/* nl */ { IDS_DQBB_FILENAME_NL, "DQBB bestand" },
/* pl */ { IDS_DQBB_FILENAME_PL, "Plik DQBB" },
/* ru */ { IDS_DQBB_FILENAME_RU, "ÄÐÙÛ DQBB" },
/* sv */ { IDS_DQBB_FILENAME_SV, "DQBB-fil" },
/* tr */ { IDS_DQBB_FILENAME_TR, "DQBB dosyasý" },

/* en */ { IDS_SAVE_DQBB_IMAGE_WHEN_CHANGED,    "Save DQBB image when changed" },
/* da */ { IDS_SAVE_DQBB_IMAGE_WHEN_CHANGED_DA, "Gem DQBB image ved ændringer" },
/* de */ { IDS_SAVE_DQBB_IMAGE_WHEN_CHANGED_DE, "DQBB Imagedatei bei Änderung speichern" },
/* es */ { IDS_SAVE_DQBB_IMAGE_WHEN_CHANGED_ES, "Grabar imagen DQBB cuando cambie" },
/* fr */ { IDS_SAVE_DQBB_IMAGE_WHEN_CHANGED_FR, "Enregistrer l'image DQBB lorsque modifiée" },
/* hu */ { IDS_SAVE_DQBB_IMAGE_WHEN_CHANGED_HU, "" },  /* fuzzy */
/* it */ { IDS_SAVE_DQBB_IMAGE_WHEN_CHANGED_IT, "Salva immagine DQBB alla modifica" },
/* ko */ { IDS_SAVE_DQBB_IMAGE_WHEN_CHANGED_KO, "" },  /* fuzzy */
/* nl */ { IDS_SAVE_DQBB_IMAGE_WHEN_CHANGED_NL, "DQBB bestand opslaan bij wijziging" },
/* pl */ { IDS_SAVE_DQBB_IMAGE_WHEN_CHANGED_PL, "Zapisuj obraz DQBB przy zmianie" },
/* ru */ { IDS_SAVE_DQBB_IMAGE_WHEN_CHANGED_RU, "Save DQBB image when changed" },
/* sv */ { IDS_SAVE_DQBB_IMAGE_WHEN_CHANGED_SV, "Spara DQBB-avbildning vid ändringar" },
/* tr */ { IDS_SAVE_DQBB_IMAGE_WHEN_CHANGED_TR, "DQBB imajý deðiþtiðinde kaydet" },

/* en */ { IDS_DQBB_FILENAME_SELECT,    "Select file for DQBB" },
/* da */ { IDS_DQBB_FILENAME_SELECT_DA, "Vælg fil for DQBB" },
/* de */ { IDS_DQBB_FILENAME_SELECT_DE, "Datei für DQBB auswählen" },
/* es */ { IDS_DQBB_FILENAME_SELECT_ES, "Seleccionar fichero para DQBB" },
/* fr */ { IDS_DQBB_FILENAME_SELECT_FR, "Sélectionner fichier pour DQBB" },
/* hu */ { IDS_DQBB_FILENAME_SELECT_HU, "Válasszon fájlt a DQBB-hez" },
/* it */ { IDS_DQBB_FILENAME_SELECT_IT, "Seleziona file per DQBB" },
/* ko */ { IDS_DQBB_FILENAME_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDS_DQBB_FILENAME_SELECT_NL, "Selecteer bestand voor DQBB" },
/* pl */ { IDS_DQBB_FILENAME_SELECT_PL, "Wybierz plik DQBB" },
/* ru */ { IDS_DQBB_FILENAME_SELECT_RU, "Select file for DQBB" },
/* sv */ { IDS_DQBB_FILENAME_SELECT_SV, "Välj fil för DQBB" },
/* tr */ { IDS_DQBB_FILENAME_SELECT_TR, "DQBB için dosya seç" },

/* en */ { IDS_DQBB_SETTINGS,    "Double Quick Brown Box settings" },
/* da */ { IDS_DQBB_SETTINGS_DA, "Double Quick Brown Box indstillinger" },
/* de */ { IDS_DQBB_SETTINGS_DE, "Double Quick Brown Box Einstellungen" },
/* es */ { IDS_DQBB_SETTINGS_ES, "Ajustes Double Quick Brown Box" },
/* fr */ { IDS_DQBB_SETTINGS_FR, "Paramètres Double Quick Brown Box" },
/* hu */ { IDS_DQBB_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_DQBB_SETTINGS_IT, "Impostazioni Double Quick Brown Box" },
/* ko */ { IDS_DQBB_SETTINGS_KO, "Double Quick Brown Box ¼ÂÆÃ" },
/* nl */ { IDS_DQBB_SETTINGS_NL, "Double Quick Brown Box instellingen" },
/* pl */ { IDS_DQBB_SETTINGS_PL, "Ustawienia Double Quick Brown Box" },
/* ru */ { IDS_DQBB_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_DQBB_SETTINGS_SV, "Inställninga för dubbel Quick Brown Box" },
/* tr */ { IDS_DQBB_SETTINGS_TR, "Double Quick Brown Box ayarlarý" },

/* en */ { IDS_ISEPIC_SETTINGS,    "ISEPIC settings" },
/* da */ { IDS_ISEPIC_SETTINGS_DA, "ISEPIC-indstillinger" },
/* de */ { IDS_ISEPIC_SETTINGS_DE, "ISEPIC Einstellungen" },
/* es */ { IDS_ISEPIC_SETTINGS_ES, "Ajustes ISEPIC" },
/* fr */ { IDS_ISEPIC_SETTINGS_FR, "Paramètres ISEPIC" },
/* hu */ { IDS_ISEPIC_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_ISEPIC_SETTINGS_IT, "Impostazioni ISEPIC" },
/* ko */ { IDS_ISEPIC_SETTINGS_KO, "ISEPIC ¼ÂÆÃ" },
/* nl */ { IDS_ISEPIC_SETTINGS_NL, "ISEPIC instellingen" },
/* pl */ { IDS_ISEPIC_SETTINGS_PL, "Ustawienia ISEPIC" },
/* ru */ { IDS_ISEPIC_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_ISEPIC_SETTINGS_SV, "ISEPIC-inställningar" },
/* tr */ { IDS_ISEPIC_SETTINGS_TR, "ISEPIC ayarlarý" },

/* en */ { IDS_ISEPIC_SWITCH,    "ISEPIC switch" },
/* da */ { IDS_ISEPIC_SWITCH_DA, "ISEPIC-kontakt" },
/* de */ { IDS_ISEPIC_SWITCH_DE, "ISEPIC Schalter" },
/* es */ { IDS_ISEPIC_SWITCH_ES, "interruptor ISEPIC" },
/* fr */ { IDS_ISEPIC_SWITCH_FR, "Paramètres d'interrupteur ISEPIC" },
/* hu */ { IDS_ISEPIC_SWITCH_HU, "" },  /* fuzzy */
/* it */ { IDS_ISEPIC_SWITCH_IT, "Pulsante ISEPIC" },
/* ko */ { IDS_ISEPIC_SWITCH_KO, "" },  /* fuzzy */
/* nl */ { IDS_ISEPIC_SWITCH_NL, "ISEPIC schakelaar" },
/* pl */ { IDS_ISEPIC_SWITCH_PL, "Prze³±cznik ISEPIC" },
/* ru */ { IDS_ISEPIC_SWITCH_RU, "ISEPIC switch" },
/* sv */ { IDS_ISEPIC_SWITCH_SV, "ISEPIC-omkopplare" },
/* tr */ { IDS_ISEPIC_SWITCH_TR, "ISEPIC anahtarý" },

/* en */ { IDS_ISEPIC_FILENAME,    "ISEPIC file" },
/* da */ { IDS_ISEPIC_FILENAME_DA, "ISEPIC-fil" },
/* de */ { IDS_ISEPIC_FILENAME_DE, "ISEPIC Imagedatei" },
/* es */ { IDS_ISEPIC_FILENAME_ES, "Fichero ISEPIC" },
/* fr */ { IDS_ISEPIC_FILENAME_FR, "Fichier ISEPIC" },
/* hu */ { IDS_ISEPIC_FILENAME_HU, "" },  /* fuzzy */
/* it */ { IDS_ISEPIC_FILENAME_IT, "File ISEPIC" },
/* ko */ { IDS_ISEPIC_FILENAME_KO, "" },  /* fuzzy */
/* nl */ { IDS_ISEPIC_FILENAME_NL, "ISEPIC bestand" },
/* pl */ { IDS_ISEPIC_FILENAME_PL, "Plik ISEPIC" },
/* ru */ { IDS_ISEPIC_FILENAME_RU, "" },  /* fuzzy */
/* sv */ { IDS_ISEPIC_FILENAME_SV, "ISEPIC-fil" },
/* tr */ { IDS_ISEPIC_FILENAME_TR, "ISEPIC dosyasý" },

/* en */ { IDS_SAVE_ISEPIC_IMAGE_WHEN_CHANGED,    "Save ISEPIC image when changed" },
/* da */ { IDS_SAVE_ISEPIC_IMAGE_WHEN_CHANGED_DA, "Gem ISEPIC image ved ændringer" },
/* de */ { IDS_SAVE_ISEPIC_IMAGE_WHEN_CHANGED_DE, "ISEPIC Imagedatei bei Änderung speichern" },
/* es */ { IDS_SAVE_ISEPIC_IMAGE_WHEN_CHANGED_ES, "Grabar imagen ISEPIC cuando cambie" },
/* fr */ { IDS_SAVE_ISEPIC_IMAGE_WHEN_CHANGED_FR, "Enregistrer l'image ISEPIC lorsque modifiée" },
/* hu */ { IDS_SAVE_ISEPIC_IMAGE_WHEN_CHANGED_HU, "" },  /* fuzzy */
/* it */ { IDS_SAVE_ISEPIC_IMAGE_WHEN_CHANGED_IT, "Salva immagine ISEPIC alla modifica" },
/* ko */ { IDS_SAVE_ISEPIC_IMAGE_WHEN_CHANGED_KO, "" },  /* fuzzy */
/* nl */ { IDS_SAVE_ISEPIC_IMAGE_WHEN_CHANGED_NL, "ISEPIC bestand opslaan bij wijziging" },
/* pl */ { IDS_SAVE_ISEPIC_IMAGE_WHEN_CHANGED_PL, "Zapisuj obraz ISEPIC przy zmianie" },
/* ru */ { IDS_SAVE_ISEPIC_IMAGE_WHEN_CHANGED_RU, "Save ISEPIC image when changed" },
/* sv */ { IDS_SAVE_ISEPIC_IMAGE_WHEN_CHANGED_SV, "Spara ISEPIC-avbildning vid ändringar" },
/* tr */ { IDS_SAVE_ISEPIC_IMAGE_WHEN_CHANGED_TR, "ISEPIC imajý deðiþtiðinde kaydet" },

/* en */ { IDS_ISEPIC_FILENAME_SELECT,    "Select file for ISEPIC" },
/* da */ { IDS_ISEPIC_FILENAME_SELECT_DA, "Vælg fil for ISEPIC" },
/* de */ { IDS_ISEPIC_FILENAME_SELECT_DE, "Datei für ISEPIC auswählen" },
/* es */ { IDS_ISEPIC_FILENAME_SELECT_ES, "Seleccionar fichero para ISEPIC" },
/* fr */ { IDS_ISEPIC_FILENAME_SELECT_FR, "Sélectionner fichier pour ISEPIC" },
/* hu */ { IDS_ISEPIC_FILENAME_SELECT_HU, "" },  /* fuzzy */
/* it */ { IDS_ISEPIC_FILENAME_SELECT_IT, "Seleziona file per ISEPIC" },
/* ko */ { IDS_ISEPIC_FILENAME_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDS_ISEPIC_FILENAME_SELECT_NL, "Selecteer bestand voor ISEPIC" },
/* pl */ { IDS_ISEPIC_FILENAME_SELECT_PL, "Wybierz plik ISEPIC" },
/* ru */ { IDS_ISEPIC_FILENAME_SELECT_RU, "Select file for ISEPIC" },
/* sv */ { IDS_ISEPIC_FILENAME_SELECT_SV, "Välj fil för ISEPIC" },
/* tr */ { IDS_ISEPIC_FILENAME_SELECT_TR, "ISEPIC için dosya seçin" },

/* en */ { IDS_SAVE_BBRTC_DATA_WHEN_CHANGED,    "Save BBRTC data when changed" },
/* da */ { IDS_SAVE_BBRTC_DATA_WHEN_CHANGED_DA, "" },  /* fuzzy */
/* de */ { IDS_SAVE_BBRTC_DATA_WHEN_CHANGED_DE, "BBRTC Daten bei Änderung speichern" },
/* es */ { IDS_SAVE_BBRTC_DATA_WHEN_CHANGED_ES, "" },  /* fuzzy */
/* fr */ { IDS_SAVE_BBRTC_DATA_WHEN_CHANGED_FR, "" },  /* fuzzy */
/* hu */ { IDS_SAVE_BBRTC_DATA_WHEN_CHANGED_HU, "" },  /* fuzzy */
/* it */ { IDS_SAVE_BBRTC_DATA_WHEN_CHANGED_IT, "" },  /* fuzzy */
/* ko */ { IDS_SAVE_BBRTC_DATA_WHEN_CHANGED_KO, "" },  /* fuzzy */
/* nl */ { IDS_SAVE_BBRTC_DATA_WHEN_CHANGED_NL, "Sla de BBRTC data op indien gewijzigd" },
/* pl */ { IDS_SAVE_BBRTC_DATA_WHEN_CHANGED_PL, "" },  /* fuzzy */
/* ru */ { IDS_SAVE_BBRTC_DATA_WHEN_CHANGED_RU, "" },  /* fuzzy */
/* sv */ { IDS_SAVE_BBRTC_DATA_WHEN_CHANGED_SV, "" },  /* fuzzy */
/* tr */ { IDS_SAVE_BBRTC_DATA_WHEN_CHANGED_TR, "" },  /* fuzzy */

/* en */ { IDS_EASYFLASH_SETTINGS,    "EasyFlash settings" },
/* da */ { IDS_EASYFLASH_SETTINGS_DA, "EasyFlash-indstillinger" },
/* de */ { IDS_EASYFLASH_SETTINGS_DE, "EasyFlash Einstellungen" },
/* es */ { IDS_EASYFLASH_SETTINGS_ES, "Ajustes EasyFlash" },
/* fr */ { IDS_EASYFLASH_SETTINGS_FR, "Paramètres datassette" },
/* hu */ { IDS_EASYFLASH_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_EASYFLASH_SETTINGS_IT, "Impostazioni EasyFlash" },
/* ko */ { IDS_EASYFLASH_SETTINGS_KO, "EasyFlash ¼³Á¤" },
/* nl */ { IDS_EASYFLASH_SETTINGS_NL, "EasyFlash instellingen" },
/* pl */ { IDS_EASYFLASH_SETTINGS_PL, "Ustawienia EasyFlash" },
/* ru */ { IDS_EASYFLASH_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_EASYFLASH_SETTINGS_SV, "EasyFlash-inställningar" },
/* tr */ { IDS_EASYFLASH_SETTINGS_TR, "EasyFlash ayarlarý" },

/* en */ { IDS_CONFIG_FILENAME_SELECT,    "Select config file" },
/* da */ { IDS_CONFIG_FILENAME_SELECT_DA, "Vælg konfigurationsfil" },
/* de */ { IDS_CONFIG_FILENAME_SELECT_DE, "Konfigurationsdateiname wählen" },
/* es */ { IDS_CONFIG_FILENAME_SELECT_ES, "Seleccionar fichero de configuración" },
/* fr */ { IDS_CONFIG_FILENAME_SELECT_FR, "Sélectionnez le fichier de configuration" },
/* hu */ { IDS_CONFIG_FILENAME_SELECT_HU, "Válassza ki a konfigurációs fájlt" },
/* it */ { IDS_CONFIG_FILENAME_SELECT_IT, "Seleziona il file di configurazione" },
/* ko */ { IDS_CONFIG_FILENAME_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDS_CONFIG_FILENAME_SELECT_NL, "Selecteer configuratiebestand" },
/* pl */ { IDS_CONFIG_FILENAME_SELECT_PL, "Wybierz plik konfiguracji" },
/* ru */ { IDS_CONFIG_FILENAME_SELECT_RU, "Select config file" },
/* sv */ { IDS_CONFIG_FILENAME_SELECT_SV, "Ange inställningsfil" },
/* tr */ { IDS_CONFIG_FILENAME_SELECT_TR, "Konfigürasyon dosyasýný seçin" },

/* en */ { IDS_JOYSTICK_1,    "Joystick #1" },
/* da */ { IDS_JOYSTICK_1_DA, "" },  /* fuzzy */
/* de */ { IDS_JOYSTICK_1_DE, "Joystick #1" },
/* es */ { IDS_JOYSTICK_1_ES, "" },  /* fuzzy */
/* fr */ { IDS_JOYSTICK_1_FR, "" },  /* fuzzy */
/* hu */ { IDS_JOYSTICK_1_HU, "" },  /* fuzzy */
/* it */ { IDS_JOYSTICK_1_IT, "" },  /* fuzzy */
/* ko */ { IDS_JOYSTICK_1_KO, "" },  /* fuzzy */
/* nl */ { IDS_JOYSTICK_1_NL, "Joystick #1" },
/* pl */ { IDS_JOYSTICK_1_PL, "" },  /* fuzzy */
/* ru */ { IDS_JOYSTICK_1_RU, "" },  /* fuzzy */
/* sv */ { IDS_JOYSTICK_1_SV, "" },  /* fuzzy */
/* tr */ { IDS_JOYSTICK_1_TR, "" },  /* fuzzy */

/* en */ { IDS_JOYSTICK_2,    "Joystick #2" },
/* da */ { IDS_JOYSTICK_2_DA, "" },  /* fuzzy */
/* de */ { IDS_JOYSTICK_2_DE, "Joystick #2" },
/* es */ { IDS_JOYSTICK_2_ES, "" },  /* fuzzy */
/* fr */ { IDS_JOYSTICK_2_FR, "" },  /* fuzzy */
/* hu */ { IDS_JOYSTICK_2_HU, "" },  /* fuzzy */
/* it */ { IDS_JOYSTICK_2_IT, "" },  /* fuzzy */
/* ko */ { IDS_JOYSTICK_2_KO, "" },  /* fuzzy */
/* nl */ { IDS_JOYSTICK_2_NL, "Joystick #2" },
/* pl */ { IDS_JOYSTICK_2_PL, "" },  /* fuzzy */
/* ru */ { IDS_JOYSTICK_2_RU, "" },  /* fuzzy */
/* sv */ { IDS_JOYSTICK_2_SV, "" },  /* fuzzy */
/* tr */ { IDS_JOYSTICK_2_TR, "" },  /* fuzzy */

/* en */ { IDS_USERPORT_ADAPTER,    "Userport joystick adapter" },
/* da */ { IDS_USERPORT_ADAPTER_DA, "Brugerportjoystickadapter" },
/* de */ { IDS_USERPORT_ADAPTER_DE, "Userport Joystick Adapter" },
/* es */ { IDS_USERPORT_ADAPTER_ES, "Adaptador de puerto usuario joystick" },
/* fr */ { IDS_USERPORT_ADAPTER_FR, "Adaptateur joystick port utilisateur" },
/* hu */ { IDS_USERPORT_ADAPTER_HU, "Userport botkormány adapter" },
/* it */ { IDS_USERPORT_ADAPTER_IT, "Adattatore joystick su userport" },
/* ko */ { IDS_USERPORT_ADAPTER_KO, "À¯Á®Æ÷Æ® Á¶ÀÌ½ºÆ½ ¾Æ´äÅÍ" },
/* nl */ { IDS_USERPORT_ADAPTER_NL, "Userport joystick adapter" },
/* pl */ { IDS_USERPORT_ADAPTER_PL, "Z³±cze userportu joysticka" },
/* ru */ { IDS_USERPORT_ADAPTER_RU, "Userport joystick adapter" },
/* sv */ { IDS_USERPORT_ADAPTER_SV, "Adapter för användarportstyrspakar" },
/* tr */ { IDS_USERPORT_ADAPTER_TR, "Userport joystick dönüþtürücü" },

/* en */ { IDS_USERPORT_ADAPTER_TYPE,    "Userport joystick adapter type" },
/* da */ { IDS_USERPORT_ADAPTER_TYPE_DA, "Brugerportjoystickadapter-type" },
/* de */ { IDS_USERPORT_ADAPTER_TYPE_DE, "Userport Joystick Adadptertype" },
/* es */ { IDS_USERPORT_ADAPTER_TYPE_ES, "Tipo de adaptador de puerto usuario joystick" },
/* fr */ { IDS_USERPORT_ADAPTER_TYPE_FR, "Type d'adaptateur joystick port utilisateur " },
/* hu */ { IDS_USERPORT_ADAPTER_TYPE_HU, "Userport botkormány adapter típusa" },
/* it */ { IDS_USERPORT_ADAPTER_TYPE_IT, "Tipo adattatore joystick su userport" },
/* ko */ { IDS_USERPORT_ADAPTER_TYPE_KO, "À¯Á®Æ÷Æ® Á¶ÀÌ½ºÆ½ ¾Æ´äÅÍ Á¾·ù" },
/* nl */ { IDS_USERPORT_ADAPTER_TYPE_NL, "Userport joystick adapter model" },
/* pl */ { IDS_USERPORT_ADAPTER_TYPE_PL, "Typ z³±cza userportu joysticka" },
/* ru */ { IDS_USERPORT_ADAPTER_TYPE_RU, "Userport joystick adapter type" },
/* sv */ { IDS_USERPORT_ADAPTER_TYPE_SV, "Typ av adapter för användarportstyrspakar" },
/* tr */ { IDS_USERPORT_ADAPTER_TYPE_TR, "Userport joystick dönüþtürücü tipi" },

/* en */ { IDS_JOYSTICK_IN_USERPORT_PORT_1,    "Joystick in extra port #1" },
/* da */ { IDS_JOYSTICK_IN_USERPORT_PORT_1_DA, "Joystick i ekstra port 1" },
/* de */ { IDS_JOYSTICK_IN_USERPORT_PORT_1_DE, "Joystick in extra Port #1" },
/* es */ { IDS_JOYSTICK_IN_USERPORT_PORT_1_ES, "Joystick en puerto suplementario #1" },
/* fr */ { IDS_JOYSTICK_IN_USERPORT_PORT_1_FR, "Joystick dans le port suppplémentaire #1" },
/* hu */ { IDS_JOYSTICK_IN_USERPORT_PORT_1_HU, "" },  /* fuzzy */
/* it */ { IDS_JOYSTICK_IN_USERPORT_PORT_1_IT, "Joystick nella porta supplementare #1" },
/* ko */ { IDS_JOYSTICK_IN_USERPORT_PORT_1_KO, "¿¢½ºÆ®¶ó Æ÷Æ® #1 ¿¡ Á¶ÀÌ½ºÆ½" },
/* nl */ { IDS_JOYSTICK_IN_USERPORT_PORT_1_NL, "Joystick in extra poort #1" },
/* pl */ { IDS_JOYSTICK_IN_USERPORT_PORT_1_PL, "Joystick w dodatkowym porcie #1" },
/* ru */ { IDS_JOYSTICK_IN_USERPORT_PORT_1_RU, "´ÖÞÙáâØÚ Ò ÔÞßÞÛÝØâÕÛìÝÞÜ ßÞàâÕ 1" },
/* sv */ { IDS_JOYSTICK_IN_USERPORT_PORT_1_SV, "Styrspak i extraport #1" },
/* tr */ { IDS_JOYSTICK_IN_USERPORT_PORT_1_TR, "Joystick ekstra port #1'de" },

/* en */ { IDS_JOYSTICK_IN_USERPORT_PORT_2,    "Joystick in extra port #2" },
/* da */ { IDS_JOYSTICK_IN_USERPORT_PORT_2_DA, "Joystick i ekstra port 2" },
/* de */ { IDS_JOYSTICK_IN_USERPORT_PORT_2_DE, "Joystick in extra Port #2" },
/* es */ { IDS_JOYSTICK_IN_USERPORT_PORT_2_ES, "Joystick en puerto suplementario #2" },
/* fr */ { IDS_JOYSTICK_IN_USERPORT_PORT_2_FR, "Joystick dans le port suppplémentaire #2" },
/* hu */ { IDS_JOYSTICK_IN_USERPORT_PORT_2_HU, "" },  /* fuzzy */
/* it */ { IDS_JOYSTICK_IN_USERPORT_PORT_2_IT, "Joystick nella porta supplementare #2" },
/* ko */ { IDS_JOYSTICK_IN_USERPORT_PORT_2_KO, "¿¢½ºÆ®¶ó Æ÷Æ® #2 ¿¡ Á¶ÀÌ½ºÆ½" },
/* nl */ { IDS_JOYSTICK_IN_USERPORT_PORT_2_NL, "Joystick in extra poort #2" },
/* pl */ { IDS_JOYSTICK_IN_USERPORT_PORT_2_PL, "Joystick w dodatkowym porcie #2" },
/* ru */ { IDS_JOYSTICK_IN_USERPORT_PORT_2_RU, "´ÖÞÙáâØÚ Ò ÔÞßÞÛÝØâÕÛìÝÞÜ ßÞàâÕ 2" },
/* sv */ { IDS_JOYSTICK_IN_USERPORT_PORT_2_SV, "Styrspak i extraport #2" },
/* tr */ { IDS_JOYSTICK_IN_USERPORT_PORT_2_TR, "Joystick ekstra port #2'de" },

/* en */ { IDS_SIDCART_JOY,    "SIDcart joystick" },
/* da */ { IDS_SIDCART_JOY_DA, "SIDcart joystick" },
/* de */ { IDS_SIDCART_JOY_DE, "SIDcart Joystick" },
/* es */ { IDS_SIDCART_JOY_ES, "Joystick SIDcart" },
/* fr */ { IDS_SIDCART_JOY_FR, "Joystick SIDcart" },
/* hu */ { IDS_SIDCART_JOY_HU, "SIDcart botkormány" },
/* it */ { IDS_SIDCART_JOY_IT, "Joystick SIDcart" },
/* ko */ { IDS_SIDCART_JOY_KO, "SIDcard Á¶ÀÌ½ºÆ½" },
/* nl */ { IDS_SIDCART_JOY_NL, "SIDcart joystick" },
/* pl */ { IDS_SIDCART_JOY_PL, "Joystick SIDcart" },
/* ru */ { IDS_SIDCART_JOY_RU, "SIDcart joystick" },
/* sv */ { IDS_SIDCART_JOY_SV, "SIDcart-styrspak" },
/* tr */ { IDS_SIDCART_JOY_TR, "SID kartuþu joystick'i" },


/* en */ { IDS_JOYSTICK_IN_SIDCART_PORT,    "Joystick in SIDcart joystick port" },
/* da */ { IDS_JOYSTICK_IN_SIDCART_PORT_DA, "Joystick i SIDcart joystickport" },
/* de */ { IDS_JOYSTICK_IN_SIDCART_PORT_DE, "Joystick in SIDcart Joystick Port" },
/* es */ { IDS_JOYSTICK_IN_SIDCART_PORT_ES, "Joystick en puerto joystick SIDcart" },
/* fr */ { IDS_JOYSTICK_IN_SIDCART_PORT_FR, "Joystick dans le port joystick SIDcart" },
/* hu */ { IDS_JOYSTICK_IN_SIDCART_PORT_HU, "Botkormány a SIDcart porton" },
/* it */ { IDS_JOYSTICK_IN_SIDCART_PORT_IT, "Joystick nella porta joystick su SIDcart" },
/* ko */ { IDS_JOYSTICK_IN_SIDCART_PORT_KO, "" },  /* fuzzy */
/* nl */ { IDS_JOYSTICK_IN_SIDCART_PORT_NL, "Joystick in SIDcart joystick poort" },
/* pl */ { IDS_JOYSTICK_IN_SIDCART_PORT_PL, "Joystick w porcie joysticka SIDcart" },
/* ru */ { IDS_JOYSTICK_IN_SIDCART_PORT_RU, "Joystick in SIDcart joystick port" },
/* sv */ { IDS_JOYSTICK_IN_SIDCART_PORT_SV, "Styrspak i SIDcart-spelport" },
/* tr */ { IDS_JOYSTICK_IN_SIDCART_PORT_TR, "Joystick SID kartuþu joystick portuna takýlý" },

/* en */ { IDS_JOYSTICK_SETTINGS,    "Joystick settings" },
/* da */ { IDS_JOYSTICK_SETTINGS_DA, "Joystick indstillinger" },
/* de */ { IDS_JOYSTICK_SETTINGS_DE, "Joystick Einstellungen" },
/* es */ { IDS_JOYSTICK_SETTINGS_ES, "Ajustes de joystick" },
/* fr */ { IDS_JOYSTICK_SETTINGS_FR, "Paramètres des joysticks" },
/* hu */ { IDS_JOYSTICK_SETTINGS_HU, "Botkormány beállításai" },
/* it */ { IDS_JOYSTICK_SETTINGS_IT, "Impostazioni joystick" },
/* ko */ { IDS_JOYSTICK_SETTINGS_KO, "Á¶ÀÌ½ºÆ½ ¼³Á¤" },
/* nl */ { IDS_JOYSTICK_SETTINGS_NL, "Joystick instellingen" },
/* pl */ { IDS_JOYSTICK_SETTINGS_PL, "Ustawienia joysticka" },
/* ru */ { IDS_JOYSTICK_SETTINGS_RU, "½ÐáâàÞÙÚØ ÔÖÞÙáâØÚÐ" },
/* sv */ { IDS_JOYSTICK_SETTINGS_SV, "Inställningar för styrspak" },
/* tr */ { IDS_JOYSTICK_SETTINGS_TR, "Joystick ayarlarý" },

/* en */ { IDS_KEYPAD,    "Numpad" },
/* da */ { IDS_KEYPAD_DA, "Numerisk tastatur" },
/* de */ { IDS_KEYPAD_DE, "Ziffernblock" },
/* es */ { IDS_KEYPAD_ES, "Teclado numérico" },
/* fr */ { IDS_KEYPAD_FR, "Pavé num." },
/* hu */ { IDS_KEYPAD_HU, "Numerikus billentyûk" },
/* it */ { IDS_KEYPAD_IT, "Tastierino numerico" },
/* ko */ { IDS_KEYPAD_KO, "³ÊÆÐµå" },
/* nl */ { IDS_KEYPAD_NL, "Numeriek toetsenbord" },
/* pl */ { IDS_KEYPAD_PL, "Klawiatura numeryczna" },
/* ru */ { IDS_KEYPAD_RU, "Numpad" },
/* sv */ { IDS_KEYPAD_SV, "Numeriskt tangentbord" },
/* tr */ { IDS_KEYPAD_TR, "Numpad" },

/* en */ { IDS_JOY_PORT_0,    "Joy Port 0" },
/* da */ { IDS_JOY_PORT_0_DA, "Joystickport 0" },
/* de */ { IDS_JOY_PORT_0_DE, "Joystick Port 0" },
/* es */ { IDS_JOY_PORT_0_ES, "Puerto Joy 0" },
/* fr */ { IDS_JOY_PORT_0_FR, "Port joystick 0" },
/* hu */ { IDS_JOY_PORT_0_HU, "Botkormány port 0" },
/* it */ { IDS_JOY_PORT_0_IT, "Joystick nella porta 0" },
/* ko */ { IDS_JOY_PORT_0_KO, "" },  /* fuzzy */
/* nl */ { IDS_JOY_PORT_0_NL, "Joy Poort 0" },
/* pl */ { IDS_JOY_PORT_0_PL, "Port joysticka 0" },
/* ru */ { IDS_JOY_PORT_0_RU, "Joy Port 0" },
/* sv */ { IDS_JOY_PORT_0_SV, "Spelport 0" },
/* tr */ { IDS_JOY_PORT_0_TR, "Joy Port 0" },

/* en */ { IDS_JOY_PORT_1,    "Joy Port 1" },
/* da */ { IDS_JOY_PORT_1_DA, "Joystickport 1" },
/* de */ { IDS_JOY_PORT_1_DE, "Joystick Port 1" },
/* es */ { IDS_JOY_PORT_1_ES, "Puerto Joy 1" },
/* fr */ { IDS_JOY_PORT_1_FR, "Port joystick 1" },
/* hu */ { IDS_JOY_PORT_1_HU, "Botkormány port 1" },
/* it */ { IDS_JOY_PORT_1_IT, "Joystick nella porta 1" },
/* ko */ { IDS_JOY_PORT_1_KO, "" },  /* fuzzy */
/* nl */ { IDS_JOY_PORT_1_NL, "Joy Poort 1" },
/* pl */ { IDS_JOY_PORT_1_PL, "Port joysticka 1" },
/* ru */ { IDS_JOY_PORT_1_RU, "Joy Port 1" },
/* sv */ { IDS_JOY_PORT_1_SV, "Spelport 1" },
/* tr */ { IDS_JOY_PORT_1_TR, "Joy Port 1" },

/* en */ { IDS_JOY_PORT_2,    "Joy Port 2" },
/* da */ { IDS_JOY_PORT_2_DA, "Joystickport 2" },
/* de */ { IDS_JOY_PORT_2_DE, "Joystick Port 2" },
/* es */ { IDS_JOY_PORT_2_ES, "Puerto Joy 2" },
/* fr */ { IDS_JOY_PORT_2_FR, "Port joystick 2" },
/* hu */ { IDS_JOY_PORT_2_HU, "Botkormány port 2" },
/* it */ { IDS_JOY_PORT_2_IT, "Joystick nella porta 2" },
/* ko */ { IDS_JOY_PORT_2_KO, "" },  /* fuzzy */
/* nl */ { IDS_JOY_PORT_2_NL, "Joy Poort 2" },
/* pl */ { IDS_JOY_PORT_2_PL, "Port joysticka 2" },
/* ru */ { IDS_JOY_PORT_2_RU, "Joy Port 2" },
/* sv */ { IDS_JOY_PORT_2_SV, "Spelport 2" },
/* tr */ { IDS_JOY_PORT_2_TR, "Joy Port 2" },

/* en */ { IDS_JOY_PORT_3,    "Joy Port 3" },
/* da */ { IDS_JOY_PORT_3_DA, "Joystickport 2" },
/* de */ { IDS_JOY_PORT_3_DE, "Joystick Port 3" },
/* es */ { IDS_JOY_PORT_3_ES, "Puerto Joy 3" },
/* fr */ { IDS_JOY_PORT_3_FR, "Port joystick 3" },
/* hu */ { IDS_JOY_PORT_3_HU, "Botkormány port 3" },
/* it */ { IDS_JOY_PORT_3_IT, "Joystick nella porta 3" },
/* ko */ { IDS_JOY_PORT_3_KO, "" },  /* fuzzy */
/* nl */ { IDS_JOY_PORT_3_NL, "Joy Poort 3" },
/* pl */ { IDS_JOY_PORT_3_PL, "Port joysticka 3" },
/* ru */ { IDS_JOY_PORT_3_RU, "Joy Port 3" },
/* sv */ { IDS_JOY_PORT_3_SV, "Spelport 2" },
/* tr */ { IDS_JOY_PORT_3_TR, "Joy Port 3" },

/* en */ { IDS_STOP_BLUE,    "Stop/Blue" },
/* da */ { IDS_STOP_BLUE_DA, "Stop/Blå" },
/* de */ { IDS_STOP_BLUE_DE, "Stopp/Blau" },
/* es */ { IDS_STOP_BLUE_ES, "Parar/Azul" },
/* fr */ { IDS_STOP_BLUE_FR, "Arrêt/Bleu" },
/* hu */ { IDS_STOP_BLUE_HU, "Leállít/Kék" },
/* it */ { IDS_STOP_BLUE_IT, "Stop/Blu" },
/* ko */ { IDS_STOP_BLUE_KO, "¸ØÃã/ÆÄ¶û" },
/* nl */ { IDS_STOP_BLUE_NL, "Stop/Blauw" },
/* pl */ { IDS_STOP_BLUE_PL, "Zatrzymaj/B³êkitny" },
/* ru */ { IDS_STOP_BLUE_RU, "Stop/Blue" },
/* sv */ { IDS_STOP_BLUE_SV, "Stopp/Blå" },
/* tr */ { IDS_STOP_BLUE_TR, "Durdur/Mavi" },

/* en */ { IDS_SELECT_RED,    "Select/Red" },
/* da */ { IDS_SELECT_RED_DA, "Vælg/Rød" },
/* de */ { IDS_SELECT_RED_DE, "Auswahl/Rot" },
/* es */ { IDS_SELECT_RED_ES, "Seleccionar/Rojo" },
/* fr */ { IDS_SELECT_RED_FR, "Sélectionner/Rouge" },
/* hu */ { IDS_SELECT_RED_HU, "Kiválaszt/Piros" },
/* it */ { IDS_SELECT_RED_IT, "Seleziona/Rosso" },
/* ko */ { IDS_SELECT_RED_KO, "¼±ÅÃ/»¡°­" },
/* nl */ { IDS_SELECT_RED_NL, "Select/Rood" },
/* pl */ { IDS_SELECT_RED_PL, "Wybierz/Czerwony" },
/* ru */ { IDS_SELECT_RED_RU, "Select/Red" },
/* sv */ { IDS_SELECT_RED_SV, "Välj/Röd" },
/* tr */ { IDS_SELECT_RED_TR, "Seçim/Kýrmýzý" },

/* en */ { IDS_REPEAT_YELLOW,    "Repeat/Yellow" },
/* da */ { IDS_REPEAT_YELLOW_DA, "Gentag/Gul" },
/* de */ { IDS_REPEAT_YELLOW_DE, "Wiederhole/Gelb" },
/* es */ { IDS_REPEAT_YELLOW_ES, "Repetir/Amarillo" },
/* fr */ { IDS_REPEAT_YELLOW_FR, "Répéter/Jaune" },
/* hu */ { IDS_REPEAT_YELLOW_HU, "Ismétel/Sárga" },
/* it */ { IDS_REPEAT_YELLOW_IT, "Ripeti/Giallo" },
/* ko */ { IDS_REPEAT_YELLOW_KO, "¹Ýº¹/³ë¶û" },
/* nl */ { IDS_REPEAT_YELLOW_NL, "Repeat/Geel" },
/* pl */ { IDS_REPEAT_YELLOW_PL, "Powtarzaj/¯ó³ty" },
/* ru */ { IDS_REPEAT_YELLOW_RU, "Repeat/Yellow" },
/* sv */ { IDS_REPEAT_YELLOW_SV, "Repetera/Gul" },
/* tr */ { IDS_REPEAT_YELLOW_TR, "Tekrar/Sarý" },

/* en */ { IDS_SHUFFLE_GREEN,    "Shuffle/Green" },
/* da */ { IDS_SHUFFLE_GREEN_DA, "Bland/Grøn" },
/* de */ { IDS_SHUFFLE_GREEN_DE, "Mische/Grün" },
/* es */ { IDS_SHUFFLE_GREEN_ES, "Shuffle/Verde" },
/* fr */ { IDS_SHUFFLE_GREEN_FR, "Mélanger/Vert" },
/* hu */ { IDS_SHUFFLE_GREEN_HU, "Véletlenszerû/Zöld" },
/* it */ { IDS_SHUFFLE_GREEN_IT, "Mischia/Verde" },
/* ko */ { IDS_SHUFFLE_GREEN_KO, "¼ÅÇÃ/ÃÊ·Ï" },
/* nl */ { IDS_SHUFFLE_GREEN_NL, "Shuffle/Groen" },
/* pl */ { IDS_SHUFFLE_GREEN_PL, "Losuj/Zielony" },
/* ru */ { IDS_SHUFFLE_GREEN_RU, "Shuffle/Green" },
/* sv */ { IDS_SHUFFLE_GREEN_SV, "Blanda/Grön" },
/* tr */ { IDS_SHUFFLE_GREEN_TR, "Karýþtýr/Yeþil" },

/* en */ { IDS_FORWARD_CHARCOAL,    "Forward/Charcoal" },
/* da */ { IDS_FORWARD_CHARCOAL_DA, "Fremad/Grafit" },
/* de */ { IDS_FORWARD_CHARCOAL_DE, "Vorwärts/Kohle" },
/* es */ { IDS_FORWARD_CHARCOAL_ES, "Adelante/Negro" },
/* fr */ { IDS_FORWARD_CHARCOAL_FR, "Avancer/Charcoal" },
/* hu */ { IDS_FORWARD_CHARCOAL_HU, "Elõre/Fekete" },
/* it */ { IDS_FORWARD_CHARCOAL_IT, "Avanti/Carboncino" },
/* ko */ { IDS_FORWARD_CHARCOAL_KO, "»¡¸®°¨±â/Â£Àº È¸»ö" },
/* nl */ { IDS_FORWARD_CHARCOAL_NL, "Forward/Houtskool" },
/* pl */ { IDS_FORWARD_CHARCOAL_PL, "W przód/Wêgiel drzewny" },
/* ru */ { IDS_FORWARD_CHARCOAL_RU, "Forward/Charcoal" },
/* sv */ { IDS_FORWARD_CHARCOAL_SV, "Framåt/Svart" },
/* tr */ { IDS_FORWARD_CHARCOAL_TR, "Ýleri/Kömür" },

/* en */ { IDS_REVERSE_CHARCOAL,    "Reverse/Charcoal" },
/* da */ { IDS_REVERSE_CHARCOAL_DA, "Bagud/Grafit" },
/* de */ { IDS_REVERSE_CHARCOAL_DE, "Rückwärts/Kohle" },
/* es */ { IDS_REVERSE_CHARCOAL_ES, "Atrás/Negro" },
/* fr */ { IDS_REVERSE_CHARCOAL_FR, "Reculer/Charcoal" },
/* hu */ { IDS_REVERSE_CHARCOAL_HU, "Vissza/Fekete" },
/* it */ { IDS_REVERSE_CHARCOAL_IT, "Indietro/Carboncino" },
/* ko */ { IDS_REVERSE_CHARCOAL_KO, "µÚÁý±â/Â£Àº È¸»ö" },
/* nl */ { IDS_REVERSE_CHARCOAL_NL, "Reverse/Houtskool" },
/* pl */ { IDS_REVERSE_CHARCOAL_PL, "Do ty³u/Wêglowy" },
/* ru */ { IDS_REVERSE_CHARCOAL_RU, "Reverse/Charcoal" },
/* sv */ { IDS_REVERSE_CHARCOAL_SV, "Bakåt/Svart" },
/* tr */ { IDS_REVERSE_CHARCOAL_TR, "Ters/Kömür" },

/* en */ { IDS_PLAY_PAUSE_GREY,    "Play-Pause/Grey" },
/* da */ { IDS_PLAY_PAUSE_GREY_DA, "Play-Pause/Grå" },
/* de */ { IDS_PLAY_PAUSE_GREY_DE, "Play-Pause/Grau" },
/* es */ { IDS_PLAY_PAUSE_GREY_ES, "Iniciar-Pausa/Gris" },
/* fr */ { IDS_PLAY_PAUSE_GREY_FR, "Play-Pause/Gris" },
/* hu */ { IDS_PLAY_PAUSE_GREY_HU, "Lejátszás-Megállítás/Szürke" },
/* it */ { IDS_PLAY_PAUSE_GREY_IT, "Avvia-Pausa/Grigio" },
/* ko */ { IDS_PLAY_PAUSE_GREY_KO, "Àç»ý-Á¤Áö/È¸»ö" },
/* nl */ { IDS_PLAY_PAUSE_GREY_NL, "Play-Pause/Grijs" },
/* pl */ { IDS_PLAY_PAUSE_GREY_PL, "Odtwarzaj-Pauza/Szary" },
/* ru */ { IDS_PLAY_PAUSE_GREY_RU, "Play-Pause/Grey" },
/* sv */ { IDS_PLAY_PAUSE_GREY_SV, "Spela-Paus/Grå" },
/* tr */ { IDS_PLAY_PAUSE_GREY_TR, "Çal-Duraklat/Gri" },

/* en */ { IDS_JOY_1_DEVICE,    "Joy 1 Device" },
/* da */ { IDS_JOY_1_DEVICE_DA, "Joystick 1 enhed" },
/* de */ { IDS_JOY_1_DEVICE_DE, "Joystick 1 Gerät" },
/* es */ { IDS_JOY_1_DEVICE_ES, "Periférico Joy 1" },
/* fr */ { IDS_JOY_1_DEVICE_FR, "Périphérique joystick 1" },
/* hu */ { IDS_JOY_1_DEVICE_HU, "1-es botkormány eszköz" },
/* it */ { IDS_JOY_1_DEVICE_IT, "Dispositivo Joystick 1" },
/* ko */ { IDS_JOY_1_DEVICE_KO, "" },  /* fuzzy */
/* nl */ { IDS_JOY_1_DEVICE_NL, "Joy 1 Apparaat" },
/* pl */ { IDS_JOY_1_DEVICE_PL, "Urz±dzenie joysticka 1" },
/* ru */ { IDS_JOY_1_DEVICE_RU, "Joy 1 Device" },
/* sv */ { IDS_JOY_1_DEVICE_SV, "Enhet för styrspak 1" },
/* tr */ { IDS_JOY_1_DEVICE_TR, "Joy 1 Aygýtý" },

/* en */ { IDS_JOY_2_DEVICE,    "Joy 2 Device" },
/* da */ { IDS_JOY_2_DEVICE_DA, "Joystick 2 enhed" },
/* de */ { IDS_JOY_2_DEVICE_DE, "Joystick 2 Gerät" },
/* es */ { IDS_JOY_2_DEVICE_ES, "Periférico Joy 2" },
/* fr */ { IDS_JOY_2_DEVICE_FR, "Périphérique joystick 2" },
/* hu */ { IDS_JOY_2_DEVICE_HU, "2-es botkormány eszköz" },
/* it */ { IDS_JOY_2_DEVICE_IT, "Dispositivo Joystick 2" },
/* ko */ { IDS_JOY_2_DEVICE_KO, "" },  /* fuzzy */
/* nl */ { IDS_JOY_2_DEVICE_NL, "Joy 2 Apparaat" },
/* pl */ { IDS_JOY_2_DEVICE_PL, "Urz±dzenie joysticka 2" },
/* ru */ { IDS_JOY_2_DEVICE_RU, "Joy 2 Device" },
/* sv */ { IDS_JOY_2_DEVICE_SV, "Enhet för styrspak 2" },
/* tr */ { IDS_JOY_2_DEVICE_TR, "Joy 2 Aygýtý" },

/* en */ { IDS_USERPORT_JOY_1_DEVICE,    "Userport Joy 1 Device" },
/* da */ { IDS_USERPORT_JOY_1_DEVICE_DA, "Brugerport Joystick 1 enhed" },
/* de */ { IDS_USERPORT_JOY_1_DEVICE_DE, "Userport Joystick 1 Gerät" },
/* es */ { IDS_USERPORT_JOY_1_DEVICE_ES, "Puerto Periférico Joy 1" },
/* fr */ { IDS_USERPORT_JOY_1_DEVICE_FR, "Périphérique Joystick 1 port utilisateur" },
/* hu */ { IDS_USERPORT_JOY_1_DEVICE_HU, "Userport 1-es botkormány eszköz" },
/* it */ { IDS_USERPORT_JOY_1_DEVICE_IT, "Dispositivo joystick 1 su userport" },
/* ko */ { IDS_USERPORT_JOY_1_DEVICE_KO, "" },  /* fuzzy */
/* nl */ { IDS_USERPORT_JOY_1_DEVICE_NL, "Userport Joy 1 Apparaat" },
/* pl */ { IDS_USERPORT_JOY_1_DEVICE_PL, "Urz±dzenie userportu joysticka 1" },
/* ru */ { IDS_USERPORT_JOY_1_DEVICE_RU, "Userport Joy 1 Device" },
/* sv */ { IDS_USERPORT_JOY_1_DEVICE_SV, "Användarportspak 1 enhet" },
/* tr */ { IDS_USERPORT_JOY_1_DEVICE_TR, "Userport Joy 1 Aygýtý" },

/* en */ { IDS_USERPORT_JOY_2_DEVICE,    "Userport Joy 2 Device" },
/* da */ { IDS_USERPORT_JOY_2_DEVICE_DA, "Brugerport Joystick 2 enhed" },
/* de */ { IDS_USERPORT_JOY_2_DEVICE_DE, "Userport Joystick 2 Gerät" },
/* es */ { IDS_USERPORT_JOY_2_DEVICE_ES, "Puerto Periférico Joy 2" },
/* fr */ { IDS_USERPORT_JOY_2_DEVICE_FR, "Périphérique Joystick 2 port utilisateur" },
/* hu */ { IDS_USERPORT_JOY_2_DEVICE_HU, "Userport 2-es botkormány eszköz" },
/* it */ { IDS_USERPORT_JOY_2_DEVICE_IT, "Dispositivo joystick 2 su userport" },
/* ko */ { IDS_USERPORT_JOY_2_DEVICE_KO, "" },  /* fuzzy */
/* nl */ { IDS_USERPORT_JOY_2_DEVICE_NL, "Userport Joy 2 Apparaat" },
/* pl */ { IDS_USERPORT_JOY_2_DEVICE_PL, "Urz±dzenie userportu joysticka 2" },
/* ru */ { IDS_USERPORT_JOY_2_DEVICE_RU, "Userport Joy 2 Device" },
/* sv */ { IDS_USERPORT_JOY_2_DEVICE_SV, "Användarportspak 2 enhet" },
/* tr */ { IDS_USERPORT_JOY_2_DEVICE_TR, "Userport Joy 2 Aygýtý" },

/* en */ { IDS_SIDCART_JOY_DEVICE,    "SIDcart Joy Device" },
/* da */ { IDS_SIDCART_JOY_DEVICE_DA, "SIDcart joystick-enhed" },
/* de */ { IDS_SIDCART_JOY_DEVICE_DE, "SIDcart Joystick Gerät" },
/* es */ { IDS_SIDCART_JOY_DEVICE_ES, "Periférico Joy SIDCart" },
/* fr */ { IDS_SIDCART_JOY_DEVICE_FR, "Périphérique Joystick SIDcart" },
/* hu */ { IDS_SIDCART_JOY_DEVICE_HU, "SIDcart botkormány eszköz" },
/* it */ { IDS_SIDCART_JOY_DEVICE_IT, "Dispositivo Joystick SIDcart" },
/* ko */ { IDS_SIDCART_JOY_DEVICE_KO, "" },  /* fuzzy */
/* nl */ { IDS_SIDCART_JOY_DEVICE_NL, "SIDcart Joy Apparaat" },
/* pl */ { IDS_SIDCART_JOY_DEVICE_PL, "Urz±dzenie joysticka SIDcart" },
/* ru */ { IDS_SIDCART_JOY_DEVICE_RU, "SIDcart Joy Device" },
/* sv */ { IDS_SIDCART_JOY_DEVICE_SV, "SIDcart-spakenhet" },
/* tr */ { IDS_SIDCART_JOY_DEVICE_TR, "SID Kartuþu Joy Aygýtý" },

/* en */ { IDS_JOY_1_FIRE,    "Joy 1 Fire" },
/* da */ { IDS_JOY_1_FIRE_DA, "Joystick 1 skydeknap" },
/* de */ { IDS_JOY_1_FIRE_DE, "Joystick 1 Feuer" },
/* es */ { IDS_JOY_1_FIRE_ES, "Joy 1 Disparo" },
/* fr */ { IDS_JOY_1_FIRE_FR, "Jeu joystick 1" },
/* hu */ { IDS_JOY_1_FIRE_HU, "1-es botkormány tûz" },
/* it */ { IDS_JOY_1_FIRE_IT, "Fuoco Joystick 1" },
/* ko */ { IDS_JOY_1_FIRE_KO, "" },  /* fuzzy */
/* nl */ { IDS_JOY_1_FIRE_NL, "Joy 1 Vuur" },
/* pl */ { IDS_JOY_1_FIRE_PL, "Fire joysticka 1" },
/* ru */ { IDS_JOY_1_FIRE_RU, "Joy 1 Fire" },
/* sv */ { IDS_JOY_1_FIRE_SV, "Fire för styrspak 1" },
/* tr */ { IDS_JOY_1_FIRE_TR, "Joy 1 Ateþ" },

/* en */ { IDS_JOY_2_FIRE,    "Joy 2 Fire" },
/* da */ { IDS_JOY_2_FIRE_DA, "Joystick 2 skydeknap" },
/* de */ { IDS_JOY_2_FIRE_DE, "Joystick 2 Feuer" },
/* es */ { IDS_JOY_2_FIRE_ES, "Joy 2 Disparo" },
/* fr */ { IDS_JOY_2_FIRE_FR, "Jeu joystick 2" },
/* hu */ { IDS_JOY_2_FIRE_HU, "2-es botkormány tûz" },
/* it */ { IDS_JOY_2_FIRE_IT, "Fuoco Joystick 2" },
/* ko */ { IDS_JOY_2_FIRE_KO, "" },  /* fuzzy */
/* nl */ { IDS_JOY_2_FIRE_NL, "Joy 2 Vuur" },
/* pl */ { IDS_JOY_2_FIRE_PL, "Fire joysticka 2" },
/* ru */ { IDS_JOY_2_FIRE_RU, "Joy 2 Fire" },
/* sv */ { IDS_JOY_2_FIRE_SV, "Fire för styrspak 2" },
/* tr */ { IDS_JOY_2_FIRE_TR, "Joy 2 Ateþ" },

/* en */ { IDS_USERPORT_JOY_1_FIRE,    "Userport Joy 1 Fire" },
/* da */ { IDS_USERPORT_JOY_1_FIRE_DA, "Brugerport joystick 1 skydeknap" },
/* de */ { IDS_USERPORT_JOY_1_FIRE_DE, "Userport Joystick 1 Feuer" },
/* es */ { IDS_USERPORT_JOY_1_FIRE_ES, "Puerto Joy 1 Disparo" },
/* fr */ { IDS_USERPORT_JOY_1_FIRE_FR, "FEU Joystick port utilisateur 1" },
/* hu */ { IDS_USERPORT_JOY_1_FIRE_HU, "1-es userport botkormány tûz" },
/* it */ { IDS_USERPORT_JOY_1_FIRE_IT, "Fuoco joystick 1 su userport" },
/* ko */ { IDS_USERPORT_JOY_1_FIRE_KO, "" },  /* fuzzy */
/* nl */ { IDS_USERPORT_JOY_1_FIRE_NL, "Userport Joy 1 Vuur" },
/* pl */ { IDS_USERPORT_JOY_1_FIRE_PL, "Fire userportu joysticka 1" },
/* ru */ { IDS_USERPORT_JOY_1_FIRE_RU, "Userport Joy 1 Fire" },
/* sv */ { IDS_USERPORT_JOY_1_FIRE_SV, "Användarportspak 1 fire" },
/* tr */ { IDS_USERPORT_JOY_1_FIRE_TR, "Userport Joy 1 Ateþ" },

/* en */ { IDS_USERPORT_JOY_2_FIRE,    "Userport Joy 2 Fire" },
/* da */ { IDS_USERPORT_JOY_2_FIRE_DA, "Brugerport joystick 2 skydeknap" },
/* de */ { IDS_USERPORT_JOY_2_FIRE_DE, "Userport Joystick 2 Feuer" },
/* es */ { IDS_USERPORT_JOY_2_FIRE_ES, "Puerto Joy 2 Disparo" },
/* fr */ { IDS_USERPORT_JOY_2_FIRE_FR, "FEU Joystick port utilisateur 2" },
/* hu */ { IDS_USERPORT_JOY_2_FIRE_HU, "2-es userport botkormány tûz" },
/* it */ { IDS_USERPORT_JOY_2_FIRE_IT, "Fuoco joystick 2 su userport" },
/* ko */ { IDS_USERPORT_JOY_2_FIRE_KO, "" },  /* fuzzy */
/* nl */ { IDS_USERPORT_JOY_2_FIRE_NL, "Userport Joy 2 Vuur" },
/* pl */ { IDS_USERPORT_JOY_2_FIRE_PL, "Fire userportu joysticka 2" },
/* ru */ { IDS_USERPORT_JOY_2_FIRE_RU, "Userport Joy 2 Fire" },
/* sv */ { IDS_USERPORT_JOY_2_FIRE_SV, "Användarportspak 2 fire" },
/* tr */ { IDS_USERPORT_JOY_2_FIRE_TR, "Userport Joy 2 Ateþ" },

/* en */ { IDS_SIDCART_JOY_FIRE,    "SIDcart Joy Fire" },
/* da */ { IDS_SIDCART_JOY_FIRE_DA, "SIDcart joystick skydeknap" },
/* de */ { IDS_SIDCART_JOY_FIRE_DE, "SIDcart Joystick Feuer" },
/* es */ { IDS_SIDCART_JOY_FIRE_ES, "Joy SIDCart Disparo" },
/* fr */ { IDS_SIDCART_JOY_FIRE_FR, "FEU du Joystick SIDcart" },
/* hu */ { IDS_SIDCART_JOY_FIRE_HU, "SIDcart botkormány tûz" },
/* it */ { IDS_SIDCART_JOY_FIRE_IT, "Fuoco joystick su SIDcart" },
/* ko */ { IDS_SIDCART_JOY_FIRE_KO, "" },  /* fuzzy */
/* nl */ { IDS_SIDCART_JOY_FIRE_NL, "SIDcart Joy Vuur" },
/* pl */ { IDS_SIDCART_JOY_FIRE_PL, "Fire joysticka SIDcart" },
/* ru */ { IDS_SIDCART_JOY_FIRE_RU, "SIDcart Joy Fire" },
/* sv */ { IDS_SIDCART_JOY_FIRE_SV, "SIDcart-spak fire" },
/* tr */ { IDS_SIDCART_JOY_FIRE_TR, "SID kartuþu Joy Ateþ" },

/* en */ { IDS_SET_INPUT_JOYSTICK_1,    "Joystick #1 emulation: (0: None, 1: Numpad, 2: Keyset A/AI, 3: Keyset B/AI)" },
/* da */ { IDS_SET_INPUT_JOYSTICK_1_DA, "" },  /* fuzzy */
/* de */ { IDS_SET_INPUT_JOYSTICK_1_DE, "Joystick #1 Emulation: (0: Keine, 1: Numpad, 2: Keysatz A/AI, 3: Keysatz B/AI)" },
/* es */ { IDS_SET_INPUT_JOYSTICK_1_ES, "Emulación Joystick #1:  (0: Ninguno, 1: Teclado numerico, 2: Tecladot A/AI, 3: Teclado B/AI)" },
/* fr */ { IDS_SET_INPUT_JOYSTICK_1_FR, "" },  /* fuzzy */
/* hu */ { IDS_SET_INPUT_JOYSTICK_1_HU, "" },  /* fuzzy */
/* it */ { IDS_SET_INPUT_JOYSTICK_1_IT, "Emulazione joystick #1: (0: Nessuno, 1: Tastierino numerico, 2: Keyset A/AI, 3: Keyset B/AI)" },
/* ko */ { IDS_SET_INPUT_JOYSTICK_1_KO, "" },  /* fuzzy */
/* nl */ { IDS_SET_INPUT_JOYSTICK_1_NL, "Emulatie joystick #1: (0: Geen, 1: Numpad, 2: Keyset A/AI, 3: Keyset B/AI)" },
/* pl */ { IDS_SET_INPUT_JOYSTICK_1_PL, "Emulacja joysticka #1: (0: Brak, 1: Klawiatura numeryczna, 2: Klawisze A/AI, 3: Klawisze B/AI)" },
/* ru */ { IDS_SET_INPUT_JOYSTICK_1_RU, "" },  /* fuzzy */
/* sv */ { IDS_SET_INPUT_JOYSTICK_1_SV, "" },  /* fuzzy */
/* tr */ { IDS_SET_INPUT_JOYSTICK_1_TR, "" },  /* fuzzy */

/* en */ { IDS_SET_INPUT_JOYSTICK_2,    "Joystick #2 emulation: (0: None, 1: Numpad, 2: Keyset A/AI, 3: Keyset B/AI)" },
/* da */ { IDS_SET_INPUT_JOYSTICK_2_DA, "Emulering af Joystick 2: (0: ingen, 1: numerisk, 2: tastesæt A/AI, 3: tastesæt B/AI)" },
/* de */ { IDS_SET_INPUT_JOYSTICK_2_DE, "Joystick #2 Emulation: (0: Keine, 1: Numpad, 2: Keysatz A/AI, 3: Keysatz B/AI)" },
/* es */ { IDS_SET_INPUT_JOYSTICK_2_ES, "Emulación Joystick #2: (0:No, 1: Teclado numérico, 2: Keyset A/AI, 3: Keyset B/AI)" },
/* fr */ { IDS_SET_INPUT_JOYSTICK_2_FR, "Émulation joystick #2: (0: Aucune, 1: Numpad, 2: Keyset A/AI, 3: Keyset B/AI)" },
/* hu */ { IDS_SET_INPUT_JOYSTICK_2_HU, "#2-es botkormány emuláció: (0: Nincs, 1: numerikus, 2: 'A' gombkészlet, 3: 'B' gombkészlet)" },
/* it */ { IDS_SET_INPUT_JOYSTICK_2_IT, "Emulazione joystick #2: (0: Nessuna, 1: Tastierino numerico, 2: Keyset A/AI, 3: Keyset B/AI)" },
/* ko */ { IDS_SET_INPUT_JOYSTICK_2_KO, "" },  /* fuzzy */
/* nl */ { IDS_SET_INPUT_JOYSTICK_2_NL, "Emulatie joystick #2: (0: Geen, 1: Numpad, 2: Keyset A/AI, 3: Keyset B/AI)" },
/* pl */ { IDS_SET_INPUT_JOYSTICK_2_PL, "Emulacja joysticka #2: (0: brak, 1: numeryczna, 2: klawisze A/AI, 3: klawisze B/AI)" },
/* ru */ { IDS_SET_INPUT_JOYSTICK_2_RU, "Joystick #2 emulation: (0: None, 1: Numpad, 2: Keyset A/AI, 3: Keyset B/AI)" },
/* sv */ { IDS_SET_INPUT_JOYSTICK_2_SV, "Emulering för styrspak 2: (0: ingen, 1: numerisk, 2: tangentuppsättning A/AI, 3: tangentuppsättning B/AI)" },
/* tr */ { IDS_SET_INPUT_JOYSTICK_2_TR, "Joystick #2 emülasyonu: (0:Yok, 1: Numpad, 2: Tuþ Seti A/AI, 3: Tuþ Seti B/AI)" },

/* en */ { IDS_SET_INPUT_EXTRA_JOYSTICK_1,    "Extra joystick #1 emulation: (0: None, 1: Numpad, 2: Keyset A/AI, 3: Keyset B/AI)" },
/* da */ { IDS_SET_INPUT_EXTRA_JOYSTICK_1_DA, "" },  /* fuzzy */
/* de */ { IDS_SET_INPUT_EXTRA_JOYSTICK_1_DE, "Extra Joystick #1 Emulation: (0: Keine, 1: Numpad, 2: Keysatz A/AI, 3: Keysatz B/AI)" },
/* es */ { IDS_SET_INPUT_EXTRA_JOYSTICK_1_ES, "Emulación Joystick suplementario #1: (0:Ninguno, 1: Teclado numérico, 2: Teclado A/AI, 3: Teclado B/AI)" },
/* fr */ { IDS_SET_INPUT_EXTRA_JOYSTICK_1_FR, "" },  /* fuzzy */
/* hu */ { IDS_SET_INPUT_EXTRA_JOYSTICK_1_HU, "" },  /* fuzzy */
/* it */ { IDS_SET_INPUT_EXTRA_JOYSTICK_1_IT, "Emulazione joystick supplementare #1: (0: Nessuno, 1: Tastierino numerico, 2: Keyset A/AI, 3: Keyset B/AI)" },
/* ko */ { IDS_SET_INPUT_EXTRA_JOYSTICK_1_KO, "" },  /* fuzzy */
/* nl */ { IDS_SET_INPUT_EXTRA_JOYSTICK_1_NL, "Emulatie extra joystick #1: (0: Geen, 1: Numpad, 2: Keyset A/AI, 3: Keyset B/AI)" },
/* pl */ { IDS_SET_INPUT_EXTRA_JOYSTICK_1_PL, "Emulacja dodatkowego joysticka #1: (0: Brak, 1: Klawiatura numeryczna, 2: Klawisze A/AI, 3: Klawisze B/AI)" },
/* ru */ { IDS_SET_INPUT_EXTRA_JOYSTICK_1_RU, "" },  /* fuzzy */
/* sv */ { IDS_SET_INPUT_EXTRA_JOYSTICK_1_SV, "" },  /* fuzzy */
/* tr */ { IDS_SET_INPUT_EXTRA_JOYSTICK_1_TR, "" },  /* fuzzy */

/* en */ { IDS_SET_INPUT_EXTRA_JOYSTICK_2,    "Extra joystick #2 emulation: (0: None, 1: Numpad, 2: Keyset A/AI, 3: Keyset B/AI)" },
/* da */ { IDS_SET_INPUT_EXTRA_JOYSTICK_2_DA, "Emulering af Joystick 2: (0: ingen, 1: numerisk, 2: tastesæt A/AI, 3: tastesæt B/AI)" },
/* de */ { IDS_SET_INPUT_EXTRA_JOYSTICK_2_DE, "Extra Joystick #2 Emulation: (0: Keine, 1: Numpad, 2: Keysatz A/AI, 3: Keysatz B/AI)" },
/* es */ { IDS_SET_INPUT_EXTRA_JOYSTICK_2_ES, "Emulación Joystick suplementario #2: (0:No, 1: Teclado numérico, 2: Keyset A/AI, 3: Keyset B/AI)" },
/* fr */ { IDS_SET_INPUT_EXTRA_JOYSTICK_2_FR, "Émulation Joystick supplémentaire 2: (0: Aucune, 1: Numpad, 2: Keyset A/AI, 3: Keyset B/AI)" },
/* hu */ { IDS_SET_INPUT_EXTRA_JOYSTICK_2_HU, "" },  /* fuzzy */
/* it */ { IDS_SET_INPUT_EXTRA_JOYSTICK_2_IT, "Emulazione joystick supplementare #2: (0: Nessuna, 1: Tastierino numerico, 2: Keyset A/AI, 3: Keyset B/AI)" },
/* ko */ { IDS_SET_INPUT_EXTRA_JOYSTICK_2_KO, "" },  /* fuzzy */
/* nl */ { IDS_SET_INPUT_EXTRA_JOYSTICK_2_NL, "Emulatie extra joystick #2: (0: Geen, 1: Numpad, 2: Keyset A/AI, 3: Keyset B/AI)" },
/* pl */ { IDS_SET_INPUT_EXTRA_JOYSTICK_2_PL, "Emulacja dodatkowego joysticka #2: (0: brak, 1: numeryczna, 2: klawisze A/AI, 3: klawisze B/AI)" },
/* ru */ { IDS_SET_INPUT_EXTRA_JOYSTICK_2_RU, "Extra joystick #2 emulation: (0: None, 1: Numpad, 2: Keyset A/AI, 3: Keyset B/AI)" },
/* sv */ { IDS_SET_INPUT_EXTRA_JOYSTICK_2_SV, "Emulering för extra styrspak 2: (0: ingen, 1: numerisk, 2: tangentuppsättning A/AI, 3: tangentuppsättning B/AI)" },
/* tr */ { IDS_SET_INPUT_EXTRA_JOYSTICK_2_TR, "Ekstra joystick #2 emülasyonu: (0:Yok, 1: Numpad, 2: Tuþ Seti A/AI, 3: Tuþ Seti B/AI)" },

/* en */ { IDS_SET_INPUT_EXTRA_JOYSTICK_3,    "Extra joystick #3 emulation: (0: None, 1: Numpad, 2: Keyset A/AI, 3: Keyset B/AI)" },
/* da */ { IDS_SET_INPUT_EXTRA_JOYSTICK_3_DA, "" },  /* fuzzy */
/* de */ { IDS_SET_INPUT_EXTRA_JOYSTICK_3_DE, "Extra Joystick #3 Emulation: (0: Keine, 1: Numpad, 2: Keysatz A/AI, 3: Keysatz B/AI)" },
/* es */ { IDS_SET_INPUT_EXTRA_JOYSTICK_3_ES, "" },  /* fuzzy */
/* fr */ { IDS_SET_INPUT_EXTRA_JOYSTICK_3_FR, "" },  /* fuzzy */
/* hu */ { IDS_SET_INPUT_EXTRA_JOYSTICK_3_HU, "" },  /* fuzzy */
/* it */ { IDS_SET_INPUT_EXTRA_JOYSTICK_3_IT, "" },  /* fuzzy */
/* ko */ { IDS_SET_INPUT_EXTRA_JOYSTICK_3_KO, "" },  /* fuzzy */
/* nl */ { IDS_SET_INPUT_EXTRA_JOYSTICK_3_NL, "Emulatie extra joystick #3: (0: Geen, 1: Numpad, 2: Keyset A/AI, 3: Keyset B/AI)" },
/* pl */ { IDS_SET_INPUT_EXTRA_JOYSTICK_3_PL, "" },  /* fuzzy */
/* ru */ { IDS_SET_INPUT_EXTRA_JOYSTICK_3_RU, "" },  /* fuzzy */
/* sv */ { IDS_SET_INPUT_EXTRA_JOYSTICK_3_SV, "" },  /* fuzzy */
/* tr */ { IDS_SET_INPUT_EXTRA_JOYSTICK_3_TR, "" },  /* fuzzy */

/* en */ { IDS_SET_INPUT_JOYLL_1,    "Joystick #1 emulation: (0: None, 1: Numpad, 2-5: Amiga Joy 0-3)" },
/* da */ { IDS_SET_INPUT_JOYLL_1_DA, "Joystick 1 emularing: (0: ingen, 1: numerisk, 2-5: Amigajoystick 0-3)" },
/* de */ { IDS_SET_INPUT_JOYLL_1_DE, "Joystick #1 Emulation: (0: Keine, 1: Numpad, 2-5: Amiga Joystick 0-3)" },
/* es */ { IDS_SET_INPUT_JOYLL_1_ES, "Emulación Joystick #1: (0:No, 1: Teclado numérico, 2-5: Amiga Joy0-3 )" },
/* fr */ { IDS_SET_INPUT_JOYLL_1_FR, "Émulation Joystick 1: (0: Aucune, 1: Pav.Num., 2-5: Amiga Joy 0-3)" },
/* hu */ { IDS_SET_INPUT_JOYLL_1_HU, "#1-es botkormány emuláció: (0: nincs, 1: numerikus billentyûk, 2-5: amiga botkormány 0-3)" },
/* it */ { IDS_SET_INPUT_JOYLL_1_IT, "Emulazione joystick #1: (0: Nessuna, 1: Tastierino numerico, 2-5: Joy Amiga 0-3)" },
/* ko */ { IDS_SET_INPUT_JOYLL_1_KO, "" },  /* fuzzy */
/* nl */ { IDS_SET_INPUT_JOYLL_1_NL, "Emulatie joystick #1: (0: Geen, 1: Numpad, 2-5: Amiga Joy 0-3)" },
/* pl */ { IDS_SET_INPUT_JOYLL_1_PL, "Emulacja joysticka #1: (0: brak, 1: numeryczna, 2-5: joystick Amigi 0-3)" },
/* ru */ { IDS_SET_INPUT_JOYLL_1_RU, "Joystick #1 emulation: (0: None, 1: Numpad, 2-5: Amiga Joy 0-3)" },
/* sv */ { IDS_SET_INPUT_JOYLL_1_SV, "Emulering för styrspak 1: (0: ingen, 1: numerisk, 2-5: Amigastyrspak 0-3)" },
/* tr */ { IDS_SET_INPUT_JOYLL_1_TR, "Joystick #1 emülasyonu: (0: Yok, 1: Numpad, 2-5: Amiga Joy 0-3)" },

/* en */ { IDS_SET_INPUT_JOYLL_2,    "Joystick #2 emulation: (0: None, 1: Numpad, 2-5: Amiga Joy 0-3)" },
/* da */ { IDS_SET_INPUT_JOYLL_2_DA, "Joystick 2 emularing: (0: ingen, 1: numerisk, 2-5: Amigajoystick 0-3)" },
/* de */ { IDS_SET_INPUT_JOYLL_2_DE, "Joystick #2 Emulation: (0: Keine, 1: Numpad, 2-5: Amiga Joystick 0-3)" },
/* es */ { IDS_SET_INPUT_JOYLL_2_ES, "Emulación Joystick #2: (0:No, 1: Teclado numérico, 2-5: Amiga Joy0-3 )" },
/* fr */ { IDS_SET_INPUT_JOYLL_2_FR, "Émulation Joystick 2: (0: Aucune, 1: Numpad, 2-5: Amiga Joy 0-33)" },
/* hu */ { IDS_SET_INPUT_JOYLL_2_HU, "#2-es botkormány emuláció: (0: nincs, 1: numerikus billentyûk, 2-5: amiga botkormány 0-3)" },
/* it */ { IDS_SET_INPUT_JOYLL_2_IT, "Emulazione joystick #2: (0: Nessuna, 1: Tastierino numerico, 2-5: Joy Amiga 0-3)" },
/* ko */ { IDS_SET_INPUT_JOYLL_2_KO, "" },  /* fuzzy */
/* nl */ { IDS_SET_INPUT_JOYLL_2_NL, "Emulatie joystick #2: (0: Geen, 1: Numpad, 2-5: Amiga Joy 0-3)" },
/* pl */ { IDS_SET_INPUT_JOYLL_2_PL, "Emulacja joysticka #2: (0: brak, 1: numeryczna, 2-5: joystick Amigi 0-3)" },
/* ru */ { IDS_SET_INPUT_JOYLL_2_RU, "Joystick #2 emulation: (0: None, 1: Numpad, 2-5: Amiga Joy 0-3)" },
/* sv */ { IDS_SET_INPUT_JOYLL_2_SV, "Emulering för styrspak 2: (0: ingen, 1: numerisk, 2-5: Amigastyrspak 0-3)" },
/* tr */ { IDS_SET_INPUT_JOYLL_2_TR, "Joystick #2 emülasyonu: (0: Yok, 1: Numpad, 2-5: Amiga Joy 0-3)" },

/* en */ { IDS_SET_INPUT_EXTRA_JOYLL_1,    "Extra joystick #1 emulation: (0: None, 1: Numpad, 2-5: Amiga Joy 0-3)" },
/* da */ { IDS_SET_INPUT_EXTRA_JOYLL_1_DA, "Ekstra joystick 1 emulering: (0: ingen, 1: numerisk, 2-5: Amigajoystick 0-3)" },
/* de */ { IDS_SET_INPUT_EXTRA_JOYLL_1_DE, "Extra Joystick #1 Emulation: (0: Keine, 1: Numpad, 2-5: Amiga Joystick 0-3)" },
/* es */ { IDS_SET_INPUT_EXTRA_JOYLL_1_ES, "Emulación Joystick suplementario #1: (0:No, 1: Teclado numérico, 2-5: Amiga Joy0-3 )" },
/* fr */ { IDS_SET_INPUT_EXTRA_JOYLL_1_FR, "Émulation Joystick supplémentaire 1: (0: Aucune, 1: Pav.Num., 2-5: Amiga Joy 0-3)" },
/* hu */ { IDS_SET_INPUT_EXTRA_JOYLL_1_HU, "#1-es extra botkormány emuláció (0: Nincs, 1: numerikus, 2-5: Amiga botkormány 0-3)" },
/* it */ { IDS_SET_INPUT_EXTRA_JOYLL_1_IT, "Emulazione joystick supplementare #1: (0: Nessuna, 1: Tastierino numerico, 2-5: Joy Amiga 0-3)" },
/* ko */ { IDS_SET_INPUT_EXTRA_JOYLL_1_KO, "" },  /* fuzzy */
/* nl */ { IDS_SET_INPUT_EXTRA_JOYLL_1_NL, "Emulatie extra joystick #1: (0: Geen, 1: Numpad, 2-5: Amiga Joy 0-3)" },
/* pl */ { IDS_SET_INPUT_EXTRA_JOYLL_1_PL, "Emulacja dodatkowego joysticka #1: (0: brak, 1: numeryczna, 2-5: joystick Amigi 0-3)" },
/* ru */ { IDS_SET_INPUT_EXTRA_JOYLL_1_RU, "Extra joystick #1 emulation: (0: None, 1: Numpad, 2-5: Amiga Joy 0-3)" },
/* sv */ { IDS_SET_INPUT_EXTRA_JOYLL_1_SV, "Emulering för extra styrspak 1: (0: ingen, 1: numerisk, 2-5: Amigastyrspak 0-3)" },
/* tr */ { IDS_SET_INPUT_EXTRA_JOYLL_1_TR, "Ekstra joystick #1 emülasyonu: (0: Yok, 1: Numpad, 2-5: Amiga Joy 0-3)" },

/* en */ { IDS_SET_INPUT_EXTRA_JOYLL_2,    "Extra joystick #2 emulation: (0: None, 1: Numpad, 2-5: Amiga Joy 0-3)" },
/* da */ { IDS_SET_INPUT_EXTRA_JOYLL_2_DA, "Ekstra joystick 2 emulering: (0: ingen, 1: numerisk, 2-5: Amigajoystick 0-3)" },
/* de */ { IDS_SET_INPUT_EXTRA_JOYLL_2_DE, "Extra Joystick #2 Emulation: (0: Keine, 1: Numpad, 2-5: Amiga Joystick 0-3)" },
/* es */ { IDS_SET_INPUT_EXTRA_JOYLL_2_ES, "Emulación Joystick suplementario #2: (0:No, 1: Teclado numérico, 2-5: Amiga Joy0-3 )" },
/* fr */ { IDS_SET_INPUT_EXTRA_JOYLL_2_FR, "Émulation Joystick supplémentaire 2: (0: Aucune, 1: Numpad, 2-5: Amiga Joy 0-3)" },
/* hu */ { IDS_SET_INPUT_EXTRA_JOYLL_2_HU, "#2-es extra botkormány emuláció (0: Nincs, 1: numerikus, 2-5: Amiga botkormány 0-3)" },
/* it */ { IDS_SET_INPUT_EXTRA_JOYLL_2_IT, "Emulazione joystick supplementare #2: (0: Nessuna, 1: Tastierino numerico, 2-5: Joy Amiga 0-3)" },
/* ko */ { IDS_SET_INPUT_EXTRA_JOYLL_2_KO, "" },  /* fuzzy */
/* nl */ { IDS_SET_INPUT_EXTRA_JOYLL_2_NL, "Emulatie extra joystick #2: (0: Geen, 1: Numpad, 2-5: Amiga Joy 0-3)" },
/* pl */ { IDS_SET_INPUT_EXTRA_JOYLL_2_PL, "Emulacja dodatkowego joysticka #2: (0: brak, 1: numeryczna, 2-5: joystick Amigi 0-3)" },
/* ru */ { IDS_SET_INPUT_EXTRA_JOYLL_2_RU, "Extra joystick #2 emulation: (0: None, 1: Numpad, 2-5: Amiga Joy 0-3)" },
/* sv */ { IDS_SET_INPUT_EXTRA_JOYLL_2_SV, "Emulering för extra styrspak 2: (0: ingen, 1: numerisk, 2-5: Amigastyrspak 0-3)" },
/* tr */ { IDS_SET_INPUT_EXTRA_JOYLL_2_TR, "Ekstra joystick #2 emülasyonu: (0: Yok, 1: Numpad, 2-5: Amiga Joy 0-3)" },

/* en */ { IDS_SET_INPUT_EXTRA_JOYLL_3,    "Extra joystick #3 emulation: (0: None, 1: Numpad, 2-5: Amiga Joy 0-3)" },
/* da */ { IDS_SET_INPUT_EXTRA_JOYLL_3_DA, "" },  /* fuzzy */
/* de */ { IDS_SET_INPUT_EXTRA_JOYLL_3_DE, "Extra Joystick #3 Emulation: (0: Keine, 1: Numpad, 2-5: Amiga Joystick 0-3)" },
/* es */ { IDS_SET_INPUT_EXTRA_JOYLL_3_ES, "" },  /* fuzzy */
/* fr */ { IDS_SET_INPUT_EXTRA_JOYLL_3_FR, "" },  /* fuzzy */
/* hu */ { IDS_SET_INPUT_EXTRA_JOYLL_3_HU, "" },  /* fuzzy */
/* it */ { IDS_SET_INPUT_EXTRA_JOYLL_3_IT, "" },  /* fuzzy */
/* ko */ { IDS_SET_INPUT_EXTRA_JOYLL_3_KO, "" },  /* fuzzy */
/* nl */ { IDS_SET_INPUT_EXTRA_JOYLL_3_NL, "Emulatie extra joystick #3: (0: Geen, 1: Numpad, 2-5: Amiga Joy 0-3)" },
/* pl */ { IDS_SET_INPUT_EXTRA_JOYLL_3_PL, "" },  /* fuzzy */
/* ru */ { IDS_SET_INPUT_EXTRA_JOYLL_3_RU, "" },  /* fuzzy */
/* sv */ { IDS_SET_INPUT_EXTRA_JOYLL_3_SV, "" },  /* fuzzy */
/* tr */ { IDS_SET_INPUT_EXTRA_JOYLL_3_TR, "" },  /* fuzzy */

/* en */ { IDS_SAVE_SETTINGS_ON_EXIT,    "Save settings on exit" },
/* da */ { IDS_SAVE_SETTINGS_ON_EXIT_DA, "Gem indstillinger når programmet afsluttes" },
/* de */ { IDS_SAVE_SETTINGS_ON_EXIT_DE, "Einstellungen beim Beenden speichern" },
/* es */ { IDS_SAVE_SETTINGS_ON_EXIT_ES, "Grabar ajustes al salir" },
/* fr */ { IDS_SAVE_SETTINGS_ON_EXIT_FR, "Enregistrer les paramètres à la sortie" },
/* hu */ { IDS_SAVE_SETTINGS_ON_EXIT_HU, "Beállítások mentése kilépéskor" },
/* it */ { IDS_SAVE_SETTINGS_ON_EXIT_IT, "Salva impostazioni in uscita" },
/* ko */ { IDS_SAVE_SETTINGS_ON_EXIT_KO, "Á¾·á½Ã ¼³Á¤ ÀúÀåÇÏ±â" },
/* nl */ { IDS_SAVE_SETTINGS_ON_EXIT_NL, "Sla instellingen op bij afsluiten" },
/* pl */ { IDS_SAVE_SETTINGS_ON_EXIT_PL, "Zapisz ustawienia przy wyj¶ciu" },
/* ru */ { IDS_SAVE_SETTINGS_ON_EXIT_RU, "ÁÞåàÐÝØâì ÝÐáâàÞÙÚØ ßÞáÛÕ ÒëåÞÔÐ" },
/* sv */ { IDS_SAVE_SETTINGS_ON_EXIT_SV, "Spara inställningar vid avslut" },
/* tr */ { IDS_SAVE_SETTINGS_ON_EXIT_TR, "Çýkýþta ayarlarý kaydet" },

/* en */ { IDS_NEVER_SAVE_SETTINGS_EXIT,    "Never save settings on exit" },
/* da */ { IDS_NEVER_SAVE_SETTINGS_EXIT_DA, "Gem aldrig indstillinger når programmet afsluttes" },
/* de */ { IDS_NEVER_SAVE_SETTINGS_EXIT_DE, "Einstellungen beim Beenden nie speichern" },
/* es */ { IDS_NEVER_SAVE_SETTINGS_EXIT_ES, "no grabar ajustes al salir" },
/* fr */ { IDS_NEVER_SAVE_SETTINGS_EXIT_FR, "NE PAS enregistrer les paramètres à la sortie" },
/* hu */ { IDS_NEVER_SAVE_SETTINGS_EXIT_HU, "" },  /* fuzzy */
/* it */ { IDS_NEVER_SAVE_SETTINGS_EXIT_IT, "Non salvare impostazioni in uscita" },
/* ko */ { IDS_NEVER_SAVE_SETTINGS_EXIT_KO, "Á¾·á½Ã Àý´ë ¼³Á¤À» ÀúÀåÇÏÁö ¾Ê½À´Ï´Ù" },
/* nl */ { IDS_NEVER_SAVE_SETTINGS_EXIT_NL, "Instellingen nooit opslaan bij afsluiten" },
/* pl */ { IDS_NEVER_SAVE_SETTINGS_EXIT_PL, "Nigdy nie zapisuj ustawieñ przy wyj¶ciu" },
/* ru */ { IDS_NEVER_SAVE_SETTINGS_EXIT_RU, "½ØÚÞÓÔÐ ÝÕ áÞåàÐÝïâì ÝÐáâàÞÙÚØ ßàØ ÒëåÞÔÕ" },
/* sv */ { IDS_NEVER_SAVE_SETTINGS_EXIT_SV, "Spara aldrig inställningar vid avslut" },
/* tr */ { IDS_NEVER_SAVE_SETTINGS_EXIT_TR, "Çýkýþta ayarlarý asla kaydetme" },

/* en */ { IDS_CONFIRM_QUITING_VICE,    "Confirm quitting VICE" },
/* da */ { IDS_CONFIRM_QUITING_VICE_DA, "Bekræft programafslutning" },
/* de */ { IDS_CONFIRM_QUITING_VICE_DE, "Bestätige Beenden von VICE" },
/* es */ { IDS_CONFIRM_QUITING_VICE_ES, "Confirmar cierre de VICE" },
/* fr */ { IDS_CONFIRM_QUITING_VICE_FR, "Confirmer avant de quitter VICE" },
/* hu */ { IDS_CONFIRM_QUITING_VICE_HU, "" },  /* fuzzy */
/* it */ { IDS_CONFIRM_QUITING_VICE_IT, "Conferma l'uscita da VICE" },
/* ko */ { IDS_CONFIRM_QUITING_VICE_KO, "VICE È®ÀÎ Á¾·á" },
/* nl */ { IDS_CONFIRM_QUITING_VICE_NL, "Bevestigen van het afsluiten van VICE" },
/* pl */ { IDS_CONFIRM_QUITING_VICE_PL, "Potwierdzaj zamkniêcie VICE" },
/* ru */ { IDS_CONFIRM_QUITING_VICE_RU, "¿ÞÔâÒÕàÔØâì ×ÐÚàëâØÕ VICE" },
/* sv */ { IDS_CONFIRM_QUITING_VICE_SV, "Bekräfta för att avsluta VICE" },
/* tr */ { IDS_CONFIRM_QUITING_VICE_TR, "VICE'dan çýkýþý teyit et" },

/* en */ { IDS_NEVER_CONFIRM_QUITING_VICE,    "Never confirm quitting VICE" },
/* da */ { IDS_NEVER_CONFIRM_QUITING_VICE_DA, "Bekræft ikke programafslutning" },
/* de */ { IDS_NEVER_CONFIRM_QUITING_VICE_DE, "Keine Bestätigung beim Beenden von VICE" },
/* es */ { IDS_NEVER_CONFIRM_QUITING_VICE_ES, "Nunca confirmar cierre de VICE" },
/* fr */ { IDS_NEVER_CONFIRM_QUITING_VICE_FR, "Ne pas confirmer pour quitter VICE" },
/* hu */ { IDS_NEVER_CONFIRM_QUITING_VICE_HU, "" },  /* fuzzy */
/* it */ { IDS_NEVER_CONFIRM_QUITING_VICE_IT, "Never confirm quitting VICE" },
/* ko */ { IDS_NEVER_CONFIRM_QUITING_VICE_KO, "VICE ¸¦ Àý´ë ³¡³»Áö ¾Ê½À´Ï´Ù" },
/* nl */ { IDS_NEVER_CONFIRM_QUITING_VICE_NL, "Nooit het afsluiten van VICE bevestigen" },
/* pl */ { IDS_NEVER_CONFIRM_QUITING_VICE_PL, "Nigdy nie potwierdzaj zamkniêcia VICE" },
/* ru */ { IDS_NEVER_CONFIRM_QUITING_VICE_RU, "½ØÚÞÓÔÐ ÝÕ ßÞÔâÒÕàÖÔÐâì ×ÐÚàëâØÕ VICE" },
/* sv */ { IDS_NEVER_CONFIRM_QUITING_VICE_SV, "Bekräfta inte för att avsluta VICE" },
/* tr */ { IDS_NEVER_CONFIRM_QUITING_VICE_TR, "VICE'dan çýkýþý asla teyit etme" },

/* en */ { IDS_SPECIFY_INITIAL_DEFAULT_DIR,    "Specify initial default directory." },
/* da */ { IDS_SPECIFY_INITIAL_DEFAULT_DIR_DA, "" },  /* fuzzy */
/* de */ { IDS_SPECIFY_INITIAL_DEFAULT_DIR_DE, "Initiales Standardverzeichnis wählen." },
/* es */ { IDS_SPECIFY_INITIAL_DEFAULT_DIR_ES, "Especificar directorio por defecto inicial" },
/* fr */ { IDS_SPECIFY_INITIAL_DEFAULT_DIR_FR, "" },  /* fuzzy */
/* hu */ { IDS_SPECIFY_INITIAL_DEFAULT_DIR_HU, "" },  /* fuzzy */
/* it */ { IDS_SPECIFY_INITIAL_DEFAULT_DIR_IT, "Specifica la directory predefinita." },
/* ko */ { IDS_SPECIFY_INITIAL_DEFAULT_DIR_KO, "" },  /* fuzzy */
/* nl */ { IDS_SPECIFY_INITIAL_DEFAULT_DIR_NL, "Geef initiële standaard directory op." },
/* pl */ { IDS_SPECIFY_INITIAL_DEFAULT_DIR_PL, "Okre¶l pocz±tkowy katalog domy¶lny." },
/* ru */ { IDS_SPECIFY_INITIAL_DEFAULT_DIR_RU, "" },  /* fuzzy */
/* sv */ { IDS_SPECIFY_INITIAL_DEFAULT_DIR_SV, "" },  /* fuzzy */
/* tr */ { IDS_SPECIFY_INITIAL_DEFAULT_DIR_TR, "" },  /* fuzzy */

/* en */ { IDS_SPECIFY_INITIAL_TAPE_DIR,    "Specify initial tape directory." },
/* da */ { IDS_SPECIFY_INITIAL_TAPE_DIR_DA, "" },  /* fuzzy */
/* de */ { IDS_SPECIFY_INITIAL_TAPE_DIR_DE, "Intiales Bandverzeichnis wählen." },
/* es */ { IDS_SPECIFY_INITIAL_TAPE_DIR_ES, "Especificar directorio inicial de Cinta" },
/* fr */ { IDS_SPECIFY_INITIAL_TAPE_DIR_FR, "" },  /* fuzzy */
/* hu */ { IDS_SPECIFY_INITIAL_TAPE_DIR_HU, "" },  /* fuzzy */
/* it */ { IDS_SPECIFY_INITIAL_TAPE_DIR_IT, "Specifica la directory iniziale delle cassette." },
/* ko */ { IDS_SPECIFY_INITIAL_TAPE_DIR_KO, "" },  /* fuzzy */
/* nl */ { IDS_SPECIFY_INITIAL_TAPE_DIR_NL, "Geef initiële tape directory op." },
/* pl */ { IDS_SPECIFY_INITIAL_TAPE_DIR_PL, "Wybierz pocz±tkowy katalog ta¶my" },
/* ru */ { IDS_SPECIFY_INITIAL_TAPE_DIR_RU, "" },  /* fuzzy */
/* sv */ { IDS_SPECIFY_INITIAL_TAPE_DIR_SV, "" },  /* fuzzy */
/* tr */ { IDS_SPECIFY_INITIAL_TAPE_DIR_TR, "" },  /* fuzzy */

/* en */ { IDS_SPECIFY_INITIAL_DISK_DIR,    "Specify initial disk directory." },
/* da */ { IDS_SPECIFY_INITIAL_DISK_DIR_DA, "" },  /* fuzzy */
/* de */ { IDS_SPECIFY_INITIAL_DISK_DIR_DE, "Initiales Diskverzeichnis wählen." },
/* es */ { IDS_SPECIFY_INITIAL_DISK_DIR_ES, "Especificar directorio inicial de Disco" },
/* fr */ { IDS_SPECIFY_INITIAL_DISK_DIR_FR, "" },  /* fuzzy */
/* hu */ { IDS_SPECIFY_INITIAL_DISK_DIR_HU, "" },  /* fuzzy */
/* it */ { IDS_SPECIFY_INITIAL_DISK_DIR_IT, "Specifica la directory iniziale dei dischi." },
/* ko */ { IDS_SPECIFY_INITIAL_DISK_DIR_KO, "" },  /* fuzzy */
/* nl */ { IDS_SPECIFY_INITIAL_DISK_DIR_NL, "Geef initiële disk directory op." },
/* pl */ { IDS_SPECIFY_INITIAL_DISK_DIR_PL, "Wybierz pocz±tkowy katalog dysku" },
/* ru */ { IDS_SPECIFY_INITIAL_DISK_DIR_RU, "" },  /* fuzzy */
/* sv */ { IDS_SPECIFY_INITIAL_DISK_DIR_SV, "" },  /* fuzzy */
/* tr */ { IDS_SPECIFY_INITIAL_DISK_DIR_TR, "" },  /* fuzzy */

/* en */ { IDS_SPECIFY_INITIAL_AUTOSTART_DIR,    "Specify initial autostart directory." },
/* da */ { IDS_SPECIFY_INITIAL_AUTOSTART_DIR_DA, "" },  /* fuzzy */
/* de */ { IDS_SPECIFY_INITIAL_AUTOSTART_DIR_DE, "Initiales Autostartverzeichnis wählen." },
/* es */ { IDS_SPECIFY_INITIAL_AUTOSTART_DIR_ES, "Seleccionar directorio inicial de Autoarranque" },
/* fr */ { IDS_SPECIFY_INITIAL_AUTOSTART_DIR_FR, "" },  /* fuzzy */
/* hu */ { IDS_SPECIFY_INITIAL_AUTOSTART_DIR_HU, "" },  /* fuzzy */
/* it */ { IDS_SPECIFY_INITIAL_AUTOSTART_DIR_IT, "Specifica la directory iniziale dell'avvio automatico." },
/* ko */ { IDS_SPECIFY_INITIAL_AUTOSTART_DIR_KO, "" },  /* fuzzy */
/* nl */ { IDS_SPECIFY_INITIAL_AUTOSTART_DIR_NL, "Geef initiële autostart directory op." },
/* pl */ { IDS_SPECIFY_INITIAL_AUTOSTART_DIR_PL, "Wybierz pocz±tkowy katalog autostartu" },
/* ru */ { IDS_SPECIFY_INITIAL_AUTOSTART_DIR_RU, "" },  /* fuzzy */
/* sv */ { IDS_SPECIFY_INITIAL_AUTOSTART_DIR_SV, "" },  /* fuzzy */
/* tr */ { IDS_SPECIFY_INITIAL_AUTOSTART_DIR_TR, "" },  /* fuzzy */

/* en */ { IDS_SPECIFY_INITIAL_CART_DIR,    "Specify initial cartridge directory." },
/* da */ { IDS_SPECIFY_INITIAL_CART_DIR_DA, "" },  /* fuzzy */
/* de */ { IDS_SPECIFY_INITIAL_CART_DIR_DE, "Initiales Modulverzeichnis wählen." },
/* es */ { IDS_SPECIFY_INITIAL_CART_DIR_ES, "Especificar directorio inicial de Cartuchos" },
/* fr */ { IDS_SPECIFY_INITIAL_CART_DIR_FR, "" },  /* fuzzy */
/* hu */ { IDS_SPECIFY_INITIAL_CART_DIR_HU, "" },  /* fuzzy */
/* it */ { IDS_SPECIFY_INITIAL_CART_DIR_IT, "Specifica la directory iniziale delle cartucce." },
/* ko */ { IDS_SPECIFY_INITIAL_CART_DIR_KO, "" },  /* fuzzy */
/* nl */ { IDS_SPECIFY_INITIAL_CART_DIR_NL, "Geef initiële cartridge directory op." },
/* pl */ { IDS_SPECIFY_INITIAL_CART_DIR_PL, "Wybierz pocz±tkowy katalog kartrid¿y" },
/* ru */ { IDS_SPECIFY_INITIAL_CART_DIR_RU, "" },  /* fuzzy */
/* sv */ { IDS_SPECIFY_INITIAL_CART_DIR_SV, "" },  /* fuzzy */
/* tr */ { IDS_SPECIFY_INITIAL_CART_DIR_TR, "" },  /* fuzzy */

/* en */ { IDS_SPECIFY_INITIAL_SNAPSHOT_DIR,    "Specify initial snapshot directory." },
/* da */ { IDS_SPECIFY_INITIAL_SNAPSHOT_DIR_DA, "" },  /* fuzzy */
/* de */ { IDS_SPECIFY_INITIAL_SNAPSHOT_DIR_DE, "Initiales Snapshot verzeichnis wählen." },
/* es */ { IDS_SPECIFY_INITIAL_SNAPSHOT_DIR_ES, "Especificar directorio inicial de Instantaneas" },
/* fr */ { IDS_SPECIFY_INITIAL_SNAPSHOT_DIR_FR, "" },  /* fuzzy */
/* hu */ { IDS_SPECIFY_INITIAL_SNAPSHOT_DIR_HU, "" },  /* fuzzy */
/* it */ { IDS_SPECIFY_INITIAL_SNAPSHOT_DIR_IT, "Specifica la directory iniziale degli snapshot." },
/* ko */ { IDS_SPECIFY_INITIAL_SNAPSHOT_DIR_KO, "" },  /* fuzzy */
/* nl */ { IDS_SPECIFY_INITIAL_SNAPSHOT_DIR_NL, "Geef initiële momentopname directory op." },
/* pl */ { IDS_SPECIFY_INITIAL_SNAPSHOT_DIR_PL, "Wybierz pocz±tkowy katalog zrzutów" },
/* ru */ { IDS_SPECIFY_INITIAL_SNAPSHOT_DIR_RU, "" },  /* fuzzy */
/* sv */ { IDS_SPECIFY_INITIAL_SNAPSHOT_DIR_SV, "" },  /* fuzzy */
/* tr */ { IDS_SPECIFY_INITIAL_SNAPSHOT_DIR_TR, "" },  /* fuzzy */

/* en */ { IDS_ENABLE_FULLSCREEN,    "Enable fullscreen." },
/* da */ { IDS_ENABLE_FULLSCREEN_DA, "" },  /* fuzzy */
/* de */ { IDS_ENABLE_FULLSCREEN_DE, "Vollbild Modus aktivieren." },
/* es */ { IDS_ENABLE_FULLSCREEN_ES, "Permitir pantalla completa" },
/* fr */ { IDS_ENABLE_FULLSCREEN_FR, "" },  /* fuzzy */
/* hu */ { IDS_ENABLE_FULLSCREEN_HU, "" },  /* fuzzy */
/* it */ { IDS_ENABLE_FULLSCREEN_IT, "Attiva schermo intero." },
/* ko */ { IDS_ENABLE_FULLSCREEN_KO, "" },  /* fuzzy */
/* nl */ { IDS_ENABLE_FULLSCREEN_NL, "Volledig Scherm inschakelen." },
/* pl */ { IDS_ENABLE_FULLSCREEN_PL, "W³±cz pe³ny ekran." },
/* ru */ { IDS_ENABLE_FULLSCREEN_RU, "" },  /* fuzzy */
/* sv */ { IDS_ENABLE_FULLSCREEN_SV, "" },  /* fuzzy */
/* tr */ { IDS_ENABLE_FULLSCREEN_TR, "" },  /* fuzzy */

/* en */ { IDS_DISABLE_FULLSCREEN,    "Disable fullscreen." },
/* da */ { IDS_DISABLE_FULLSCREEN_DA, "" },  /* fuzzy */
/* de */ { IDS_DISABLE_FULLSCREEN_DE, "Vollbild Modus deaktivieren." },
/* es */ { IDS_DISABLE_FULLSCREEN_ES, "inhabilitar  pantalla completa" },
/* fr */ { IDS_DISABLE_FULLSCREEN_FR, "" },  /* fuzzy */
/* hu */ { IDS_DISABLE_FULLSCREEN_HU, "" },  /* fuzzy */
/* it */ { IDS_DISABLE_FULLSCREEN_IT, "Disattiva schermo intero." },
/* ko */ { IDS_DISABLE_FULLSCREEN_KO, "" },  /* fuzzy */
/* nl */ { IDS_DISABLE_FULLSCREEN_NL, "Volledig Scherm uitschakelen." },
/* pl */ { IDS_DISABLE_FULLSCREEN_PL, "Wy³±cz pe³ny ekran." },
/* ru */ { IDS_DISABLE_FULLSCREEN_RU, "" },  /* fuzzy */
/* sv */ { IDS_DISABLE_FULLSCREEN_SV, "" },  /* fuzzy */
/* tr */ { IDS_DISABLE_FULLSCREEN_TR, "" },  /* fuzzy */

/* en */ { IDS_ENABLE_STATUSBAR,    "Enable statusbar." },
/* da */ { IDS_ENABLE_STATUSBAR_DA, "" },  /* fuzzy */
/* de */ { IDS_ENABLE_STATUSBAR_DE, "Statusleiste aktivieren." },
/* es */ { IDS_ENABLE_STATUSBAR_ES, "Permitir barra de estado" },
/* fr */ { IDS_ENABLE_STATUSBAR_FR, "" },  /* fuzzy */
/* hu */ { IDS_ENABLE_STATUSBAR_HU, "" },  /* fuzzy */
/* it */ { IDS_ENABLE_STATUSBAR_IT, "Attiva barra di stato." },
/* ko */ { IDS_ENABLE_STATUSBAR_KO, "" },  /* fuzzy */
/* nl */ { IDS_ENABLE_STATUSBAR_NL, "Statusbalk inschakelen" },
/* pl */ { IDS_ENABLE_STATUSBAR_PL, "W³±cz pasek stanu." },
/* ru */ { IDS_ENABLE_STATUSBAR_RU, "" },  /* fuzzy */
/* sv */ { IDS_ENABLE_STATUSBAR_SV, "" },  /* fuzzy */
/* tr */ { IDS_ENABLE_STATUSBAR_TR, "" },  /* fuzzy */

/* en */ { IDS_DISABLE_STATUSBAR,    "Disable statusbar." },
/* da */ { IDS_DISABLE_STATUSBAR_DA, "" },  /* fuzzy */
/* de */ { IDS_DISABLE_STATUSBAR_DE, "Statusleiste deaktivieren." },
/* es */ { IDS_DISABLE_STATUSBAR_ES, "inhabilitar  barra de estado" },
/* fr */ { IDS_DISABLE_STATUSBAR_FR, "" },  /* fuzzy */
/* hu */ { IDS_DISABLE_STATUSBAR_HU, "" },  /* fuzzy */
/* it */ { IDS_DISABLE_STATUSBAR_IT, "Disattiva barra di stato." },
/* ko */ { IDS_DISABLE_STATUSBAR_KO, "" },  /* fuzzy */
/* nl */ { IDS_DISABLE_STATUSBAR_NL, "Statusbalk uitschakelen" },
/* pl */ { IDS_DISABLE_STATUSBAR_PL, "Wy³±cz pasek stanu." },
/* ru */ { IDS_DISABLE_STATUSBAR_RU, "" },  /* fuzzy */
/* sv */ { IDS_DISABLE_STATUSBAR_SV, "" },  /* fuzzy */
/* tr */ { IDS_DISABLE_STATUSBAR_TR, "" },  /* fuzzy */

#if defined(HAVE_PROTO_CYBERGRAPHICS_H) && defined(HAVE_XVIDEO)
/* en */ { IDS_ENABLE_VIDEOOVERLAY,    "Enable video overlay." },
/* da */ { IDS_ENABLE_VIDEOOVERLAY_DA, "" },  /* fuzzy */
/* de */ { IDS_ENABLE_VIDEOOVERLAY_DE, "Video Overlay aktivieren." },
/* es */ { IDS_ENABLE_VIDEOOVERLAY_ES, "Permitir Video overlay" },
/* fr */ { IDS_ENABLE_VIDEOOVERLAY_FR, "" },  /* fuzzy */
/* hu */ { IDS_ENABLE_VIDEOOVERLAY_HU, "" },  /* fuzzy */
/* it */ { IDS_ENABLE_VIDEOOVERLAY_IT, "Attiva overlay video." },
/* ko */ { IDS_ENABLE_VIDEOOVERLAY_KO, "" },  /* fuzzy */
/* nl */ { IDS_ENABLE_VIDEOOVERLAY_NL, "Video overlay inschakelen" },
/* pl */ { IDS_ENABLE_VIDEOOVERLAY_PL, "W³±cz video overlay" },
/* ru */ { IDS_ENABLE_VIDEOOVERLAY_RU, "" },  /* fuzzy */
/* sv */ { IDS_ENABLE_VIDEOOVERLAY_SV, "" },  /* fuzzy */
/* tr */ { IDS_ENABLE_VIDEOOVERLAY_TR, "" },  /* fuzzy */

/* en */ { IDS_DISABLE_VIDEOOVERLAY,    "Disable video overlay." },
/* da */ { IDS_DISABLE_VIDEOOVERLAY_DA, "" },  /* fuzzy */
/* de */ { IDS_DISABLE_VIDEOOVERLAY_DE, "Video Overlay deaktivieren." },
/* es */ { IDS_DISABLE_VIDEOOVERLAY_ES, "Inhabilitar Video overlay" },
/* fr */ { IDS_DISABLE_VIDEOOVERLAY_FR, "" },  /* fuzzy */
/* hu */ { IDS_DISABLE_VIDEOOVERLAY_HU, "" },  /* fuzzy */
/* it */ { IDS_DISABLE_VIDEOOVERLAY_IT, "Disattiva overlay video." },
/* ko */ { IDS_DISABLE_VIDEOOVERLAY_KO, "" },  /* fuzzy */
/* nl */ { IDS_DISABLE_VIDEOOVERLAY_NL, "Video overlay uitschakelen" },
/* pl */ { IDS_DISABLE_VIDEOOVERLAY_PL, "Wy³±cz video overlay" },
/* ru */ { IDS_DISABLE_VIDEOOVERLAY_RU, "" },  /* fuzzy */
/* sv */ { IDS_DISABLE_VIDEOOVERLAY_SV, "" },  /* fuzzy */
/* tr */ { IDS_DISABLE_VIDEOOVERLAY_TR, "" },  /* fuzzy */
#endif

/* en */ { IDS_PET_REU_SIZE,    "PET REU size" },
/* da */ { IDS_PET_REU_SIZE_DA, "PET REU størrelse" },
/* de */ { IDS_PET_REU_SIZE_DE, "PET REU Größe" },
/* es */ { IDS_PET_REU_SIZE_ES, "Tamaño REU PET" },
/* fr */ { IDS_PET_REU_SIZE_FR, "Taille du PET REU" },
/* hu */ { IDS_PET_REU_SIZE_HU, "PET REU mérete" },
/* it */ { IDS_PET_REU_SIZE_IT, "Dimensione PET REU" },
/* ko */ { IDS_PET_REU_SIZE_KO, "" },  /* fuzzy */
/* nl */ { IDS_PET_REU_SIZE_NL, "PET REU grootte" },
/* pl */ { IDS_PET_REU_SIZE_PL, "Rozmiar PET REU" },
/* ru */ { IDS_PET_REU_SIZE_RU, "PET REU size" },
/* sv */ { IDS_PET_REU_SIZE_SV, "Storlek på PET REU" },
/* tr */ { IDS_PET_REU_SIZE_TR, "PET REU boyutu" },

/* en */ { IDS_PET_REU_FILENAME,    "PET REU file" },
/* da */ { IDS_PET_REU_FILENAME_DA, "PET REU-fil" },
/* de */ { IDS_PET_REU_FILENAME_DE, "PET REU Datei" },
/* es */ { IDS_PET_REU_FILENAME_ES, "Fichero REU PET" },
/* fr */ { IDS_PET_REU_FILENAME_FR, "Fichier PET REU" },
/* hu */ { IDS_PET_REU_FILENAME_HU, "PET REU fájl" },
/* it */ { IDS_PET_REU_FILENAME_IT, "File PET REU" },
/* ko */ { IDS_PET_REU_FILENAME_KO, "PET REU ÆÄÀÏ" },
/* nl */ { IDS_PET_REU_FILENAME_NL, "PET REU bestand" },
/* pl */ { IDS_PET_REU_FILENAME_PL, "Plik PET REU" },
/* ru */ { IDS_PET_REU_FILENAME_RU, "PET REU file" },
/* sv */ { IDS_PET_REU_FILENAME_SV, "PET REU-fil" },
/* tr */ { IDS_PET_REU_FILENAME_TR, "PET REU dosyasý" },

/* en */ { IDS_PET_REU_FILENAME_SELECT,    "Select file for PET REU" },
/* da */ { IDS_PET_REU_FILENAME_SELECT_DA, "Vælg fil for PET REU" },
/* de */ { IDS_PET_REU_FILENAME_SELECT_DE, "Datei für PET REU auswählen" },
/* es */ { IDS_PET_REU_FILENAME_SELECT_ES, "Seleccionar fichero para REU PET" },
/* fr */ { IDS_PET_REU_FILENAME_SELECT_FR, "Sélectionner fichier pour PET REU" },
/* hu */ { IDS_PET_REU_FILENAME_SELECT_HU, "Válasszon fájlt a PET REU-hoz" },
/* it */ { IDS_PET_REU_FILENAME_SELECT_IT, "Seleziona il file per il PET REU" },
/* ko */ { IDS_PET_REU_FILENAME_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDS_PET_REU_FILENAME_SELECT_NL, "Selecteer bestand voor PET REU" },
/* pl */ { IDS_PET_REU_FILENAME_SELECT_PL, "Wybierz plik PET REU" },
/* ru */ { IDS_PET_REU_FILENAME_SELECT_RU, "Select file for PET REU" },
/* sv */ { IDS_PET_REU_FILENAME_SELECT_SV, "Välj fil för PET REU" },
/* tr */ { IDS_PET_REU_FILENAME_SELECT_TR, "PET REU için dosya seçin" },

/* en */ { IDS_PET_REU_SETTINGS,    "PET REU settings" },
/* da */ { IDS_PET_REU_SETTINGS_DA, "PET REU-indstillinger" },
/* de */ { IDS_PET_REU_SETTINGS_DE, "PET REU Einstellungen" },
/* es */ { IDS_PET_REU_SETTINGS_ES, "Ajustes PET REU" },
/* fr */ { IDS_PET_REU_SETTINGS_FR, "Paramètres PET REU" },
/* hu */ { IDS_PET_REU_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_PET_REU_SETTINGS_IT, "Impostazioni PET REU" },
/* ko */ { IDS_PET_REU_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_PET_REU_SETTINGS_NL, "PET REU instellingen" },
/* pl */ { IDS_PET_REU_SETTINGS_PL, "Ustawienia PET REU" },
/* ru */ { IDS_PET_REU_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_PET_REU_SETTINGS_SV, "PET REU-Inställningar" },
/* tr */ { IDS_PET_REU_SETTINGS_TR, "PET REU Ayarlarý" },

/* en */ { IDS_RGBI,    "RGBI" },
/* da */ { IDS_RGBI_DA, "" },  /* fuzzy */
/* de */ { IDS_RGBI_DE, "RGBI" },
/* es */ { IDS_RGBI_ES, "RGBI" },
/* fr */ { IDS_RGBI_FR, "" },  /* fuzzy */
/* hu */ { IDS_RGBI_HU, "" },  /* fuzzy */
/* it */ { IDS_RGBI_IT, "RGBI" },
/* ko */ { IDS_RGBI_KO, "" },  /* fuzzy */
/* nl */ { IDS_RGBI_NL, "RGBI" },
/* pl */ { IDS_RGBI_PL, "RGBI" },
/* ru */ { IDS_RGBI_RU, "" },  /* fuzzy */
/* sv */ { IDS_RGBI_SV, "RGBI" },
/* tr */ { IDS_RGBI_TR, "" },  /* fuzzy */

/* en */ { IDS_ANALOG,    "Analog" },
/* da */ { IDS_ANALOG_DA, "" },  /* fuzzy */
/* de */ { IDS_ANALOG_DE, "Analog" },
/* es */ { IDS_ANALOG_ES, "Analógico" },
/* fr */ { IDS_ANALOG_FR, "" },  /* fuzzy */
/* hu */ { IDS_ANALOG_HU, "" },  /* fuzzy */
/* it */ { IDS_ANALOG_IT, "Analogico" },
/* ko */ { IDS_ANALOG_KO, "" },  /* fuzzy */
/* nl */ { IDS_ANALOG_NL, "Analoog" },
/* pl */ { IDS_ANALOG_PL, "Analog" },
/* ru */ { IDS_ANALOG_RU, "" },  /* fuzzy */
/* sv */ { IDS_ANALOG_SV, "Analog" },
/* tr */ { IDS_ANALOG_TR, "" },  /* fuzzy */

/* en */ { IDS_PET_COLOUR_TYPE,    "PET Colour type" },
/* da */ { IDS_PET_COLOUR_TYPE_DA, "" },  /* fuzzy */
/* de */ { IDS_PET_COLOUR_TYPE_DE, "PET Farbtyp" },
/* es */ { IDS_PET_COLOUR_TYPE_ES, "Tipo de color PET" },
/* fr */ { IDS_PET_COLOUR_TYPE_FR, "" },  /* fuzzy */
/* hu */ { IDS_PET_COLOUR_TYPE_HU, "" },  /* fuzzy */
/* it */ { IDS_PET_COLOUR_TYPE_IT, "Tipo PET Colour" },
/* ko */ { IDS_PET_COLOUR_TYPE_KO, "" },  /* fuzzy */
/* nl */ { IDS_PET_COLOUR_TYPE_NL, "PET Colour soort" },
/* pl */ { IDS_PET_COLOUR_TYPE_PL, "Rodzaj koloru PET" },
/* ru */ { IDS_PET_COLOUR_TYPE_RU, "" },  /* fuzzy */
/* sv */ { IDS_PET_COLOUR_TYPE_SV, "PET färgtyp" },
/* tr */ { IDS_PET_COLOUR_TYPE_TR, "" },  /* fuzzy */

/* en */ { IDS_PET_COLOUR_BACKGROUND,    "PET Colour background" },
/* da */ { IDS_PET_COLOUR_BACKGROUND_DA, "" },  /* fuzzy */
/* de */ { IDS_PET_COLOUR_BACKGROUND_DE, "PET Hintergrundfarbe" },
/* es */ { IDS_PET_COLOUR_BACKGROUND_ES, "Color del fondo PET" },
/* fr */ { IDS_PET_COLOUR_BACKGROUND_FR, "" },  /* fuzzy */
/* hu */ { IDS_PET_COLOUR_BACKGROUND_HU, "" },  /* fuzzy */
/* it */ { IDS_PET_COLOUR_BACKGROUND_IT, "Sfondo PET Colour" },
/* ko */ { IDS_PET_COLOUR_BACKGROUND_KO, "" },  /* fuzzy */
/* nl */ { IDS_PET_COLOUR_BACKGROUND_NL, "PET Colour achtergrond kleur" },
/* pl */ { IDS_PET_COLOUR_BACKGROUND_PL, "T³o koloru PET" },
/* ru */ { IDS_PET_COLOUR_BACKGROUND_RU, "" },  /* fuzzy */
/* sv */ { IDS_PET_COLOUR_BACKGROUND_SV, "PET bakgrundsfärg" },
/* tr */ { IDS_PET_COLOUR_BACKGROUND_TR, "" },  /* fuzzy */

/* en */ { IDS_PET_COLOUR_SETTINGS,    "PET Colour settings" },
/* da */ { IDS_PET_COLOUR_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDS_PET_COLOUR_SETTINGS_DE, "PET Farbeinstellungen" },
/* es */ { IDS_PET_COLOUR_SETTINGS_ES, "Ajustes color PET" },
/* fr */ { IDS_PET_COLOUR_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDS_PET_COLOUR_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_PET_COLOUR_SETTINGS_IT, "Impostazioni PET Colour" },
/* ko */ { IDS_PET_COLOUR_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_PET_COLOUR_SETTINGS_NL, "PET Colour instellingen" },
/* pl */ { IDS_PET_COLOUR_SETTINGS_PL, "Ustawienia koloru PET" },
/* ru */ { IDS_PET_COLOUR_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_PET_COLOUR_SETTINGS_SV, "PET-färginställningar" },
/* tr */ { IDS_PET_COLOUR_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDS_V364_SPEECH_FILENAME,    "V364 Speech ROM file" },
/* da */ { IDS_V364_SPEECH_FILENAME_DA, "V364 Speech ROM-fil" },
/* de */ { IDS_V364_SPEECH_FILENAME_DE, "V364 Stimme ROM Datei" },
/* es */ { IDS_V364_SPEECH_FILENAME_ES, "Fichero ROM  V364 Speech" },
/* fr */ { IDS_V364_SPEECH_FILENAME_FR, "Sélectionner le fichier ROM V364 Speech" },
/* hu */ { IDS_V364_SPEECH_FILENAME_HU, "" },  /* fuzzy */
/* it */ { IDS_V364_SPEECH_FILENAME_IT, "File ROM V364 Speech" },
/* ko */ { IDS_V364_SPEECH_FILENAME_KO, "" },  /* fuzzy */
/* nl */ { IDS_V364_SPEECH_FILENAME_NL, "V364 spraak ROM bestand" },
/* pl */ { IDS_V364_SPEECH_FILENAME_PL, "Plik ROM V364 Speech" },
/* ru */ { IDS_V364_SPEECH_FILENAME_RU, "" },  /* fuzzy */
/* sv */ { IDS_V364_SPEECH_FILENAME_SV, "ROM-fil för V364-röst" },
/* tr */ { IDS_V364_SPEECH_FILENAME_TR, "V364 Speech ROM dosyasý" },

/* en */ { IDS_V364_SPEECH_FILENAME_SELECT,    "Select file for V364 Speech" },
/* da */ { IDS_V364_SPEECH_FILENAME_SELECT_DA, "Angiv fil for V364 Speech" },
/* de */ { IDS_V364_SPEECH_FILENAME_SELECT_DE, "Datei für V364 Stimme auswählen" },
/* es */ { IDS_V364_SPEECH_FILENAME_SELECT_ES, "Seleccionar fichero para V364 Speech" },
/* fr */ { IDS_V364_SPEECH_FILENAME_SELECT_FR, "Sélectionner fichier pour V364 Speech" },
/* hu */ { IDS_V364_SPEECH_FILENAME_SELECT_HU, "" },  /* fuzzy */
/* it */ { IDS_V364_SPEECH_FILENAME_SELECT_IT, "Seleziona file per V364 Speech" },
/* ko */ { IDS_V364_SPEECH_FILENAME_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDS_V364_SPEECH_FILENAME_SELECT_NL, "Selecteer bestand voor V364 Spraak" },
/* pl */ { IDS_V364_SPEECH_FILENAME_SELECT_PL, "Wybierz plik mowy V364" },
/* ru */ { IDS_V364_SPEECH_FILENAME_SELECT_RU, "Select file for V364 Speech" },
/* sv */ { IDS_V364_SPEECH_FILENAME_SELECT_SV, "Välj fil för V364-röst" },
/* tr */ { IDS_V364_SPEECH_FILENAME_SELECT_TR, "V364 Speech için dosya seçin" },

/* en */ { IDS_V364_SPEECH_SETTINGS,    "V364 Speech settings" },
/* da */ { IDS_V364_SPEECH_SETTINGS_DA, "V364 tale-indstillinger" },
/* de */ { IDS_V364_SPEECH_SETTINGS_DE, "V364 Stimme Einstellungen" },
/* es */ { IDS_V364_SPEECH_SETTINGS_ES, "Ajustes V364 Speech" },
/* fr */ { IDS_V364_SPEECH_SETTINGS_FR, "Paramètres du V364 Speech" },
/* hu */ { IDS_V364_SPEECH_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_V364_SPEECH_SETTINGS_IT, "Impostazioni V364 Speech" },
/* ko */ { IDS_V364_SPEECH_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_V364_SPEECH_SETTINGS_NL, "V364 Spraak instellingen" },
/* pl */ { IDS_V364_SPEECH_SETTINGS_PL, "Ustawienia mowy V364" },
/* ru */ { IDS_V364_SPEECH_SETTINGS_RU, "¿ÐàÐÜÕâàë àÕçØ V364" },
/* sv */ { IDS_V364_SPEECH_SETTINGS_SV, "V364-röstinställningar" },
/* tr */ { IDS_V364_SPEECH_SETTINGS_TR, "V364 Speech ayarlarý" },

/* en */ { IDS_PET_DWW_FILENAME,    "PET DWW file" },
/* da */ { IDS_PET_DWW_FILENAME_DA, "PET DWW fil" },
/* de */ { IDS_PET_DWW_FILENAME_DE, "PET DWW Datei" },
/* es */ { IDS_PET_DWW_FILENAME_ES, "Fichero DWW PET" },
/* fr */ { IDS_PET_DWW_FILENAME_FR, "Fichier PET DWW" },
/* hu */ { IDS_PET_DWW_FILENAME_HU, "" },  /* fuzzy */
/* it */ { IDS_PET_DWW_FILENAME_IT, "File PET DWW" },
/* ko */ { IDS_PET_DWW_FILENAME_KO, "PET DWW ÆÄÀÏ" },
/* nl */ { IDS_PET_DWW_FILENAME_NL, "PET DWW bestand" },
/* pl */ { IDS_PET_DWW_FILENAME_PL, "Plik PET DWW" },
/* ru */ { IDS_PET_DWW_FILENAME_RU, "PET DWW file" },
/* sv */ { IDS_PET_DWW_FILENAME_SV, "PET DWW-fil" },
/* tr */ { IDS_PET_DWW_FILENAME_TR, "PET DWW dosyasý" },

/* en */ { IDS_PET_DWW_FILENAME_SELECT,    "Select file for PET DWW" },
/* da */ { IDS_PET_DWW_FILENAME_SELECT_DA, "Vælg fil for PET DWW" },
/* de */ { IDS_PET_DWW_FILENAME_SELECT_DE, "Datei für PET DWW auswählen" },
/* es */ { IDS_PET_DWW_FILENAME_SELECT_ES, "Seleccionar fichero para DWW PET" },
/* fr */ { IDS_PET_DWW_FILENAME_SELECT_FR, "Sélectionner fichier pour PET DWW" },
/* hu */ { IDS_PET_DWW_FILENAME_SELECT_HU, "" },  /* fuzzy */
/* it */ { IDS_PET_DWW_FILENAME_SELECT_IT, "Seleziona file per il PET DWW" },
/* ko */ { IDS_PET_DWW_FILENAME_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDS_PET_DWW_FILENAME_SELECT_NL, "Selecteer bestand voor PET DWW" },
/* pl */ { IDS_PET_DWW_FILENAME_SELECT_PL, "Wybierz plik PET DWW" },
/* ru */ { IDS_PET_DWW_FILENAME_SELECT_RU, "Select file for PET DWW" },
/* sv */ { IDS_PET_DWW_FILENAME_SELECT_SV, "Välj fil för PET DWW" },
/* tr */ { IDS_PET_DWW_FILENAME_SELECT_TR, "PET DWW için dosya seçin" },

/* en */ { IDS_PET_DWW_SETTINGS,    "PET DWW settings" },
/* da */ { IDS_PET_DWW_SETTINGS_DA, "PET DWW-indstillinger" },
/* de */ { IDS_PET_DWW_SETTINGS_DE, "PET DWW Einstellungen" },
/* es */ { IDS_PET_DWW_SETTINGS_ES, "Ajustes PET DWW" },
/* fr */ { IDS_PET_DWW_SETTINGS_FR, "Paramètres PET DWW" },
/* hu */ { IDS_PET_DWW_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_PET_DWW_SETTINGS_IT, "Impostazioni PET DWW" },
/* ko */ { IDS_PET_DWW_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_PET_DWW_SETTINGS_NL, "PET DWW instellingen" },
/* pl */ { IDS_PET_DWW_SETTINGS_PL, "Ustawienia PET DWW" },
/* ru */ { IDS_PET_DWW_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_PET_DWW_SETTINGS_SV, "PET DWW-Inställningar" },
/* tr */ { IDS_PET_DWW_SETTINGS_TR, "PET DWW Ayarlarý" },

/* en */ { IDS_MEMORY_HACK_DEVICE,    "Memory expansion hack device" },
/* da */ { IDS_MEMORY_HACK_DEVICE_DA, "" },  /* fuzzy */
/* de */ { IDS_MEMORY_HACK_DEVICE_DE, "Speicher Erweiterungeshack Gerät" },
/* es */ { IDS_MEMORY_HACK_DEVICE_ES, "" },  /* fuzzy */
/* fr */ { IDS_MEMORY_HACK_DEVICE_FR, "" },  /* fuzzy */
/* hu */ { IDS_MEMORY_HACK_DEVICE_HU, "" },  /* fuzzy */
/* it */ { IDS_MEMORY_HACK_DEVICE_IT, "" },  /* fuzzy */
/* ko */ { IDS_MEMORY_HACK_DEVICE_KO, "" },  /* fuzzy */
/* nl */ { IDS_MEMORY_HACK_DEVICE_NL, "Geheugen uitbreidingen hack apparaat" },
/* pl */ { IDS_MEMORY_HACK_DEVICE_PL, "Urz±dzenie hacka rozszerzenia pamiêci" },
/* ru */ { IDS_MEMORY_HACK_DEVICE_RU, "" },  /* fuzzy */
/* sv */ { IDS_MEMORY_HACK_DEVICE_SV, "" },  /* fuzzy */
/* tr */ { IDS_MEMORY_HACK_DEVICE_TR, "" },  /* fuzzy */

/* en */ { IDS_PLUS60K_BASE,    "PLUS60K base address" },
/* da */ { IDS_PLUS60K_BASE_DA, "Baseadresse for PLUS60K" },
/* de */ { IDS_PLUS60K_BASE_DE, "PLUS60K Basis Adresse" },
/* es */ { IDS_PLUS60K_BASE_ES, "Dirección base PLUS60K" },
/* fr */ { IDS_PLUS60K_BASE_FR, "Adresse de base PLUS60K" },
/* hu */ { IDS_PLUS60K_BASE_HU, "PLUS60K báziscíme" },
/* it */ { IDS_PLUS60K_BASE_IT, "indirizzo base PLUS60K" },
/* ko */ { IDS_PLUS60K_BASE_KO, "" },  /* fuzzy */
/* nl */ { IDS_PLUS60K_BASE_NL, "PLUS60K basisadres" },
/* pl */ { IDS_PLUS60K_BASE_PL, "Adres bazowy PLUS60K" },
/* ru */ { IDS_PLUS60K_BASE_RU, "PLUS60K base address" },
/* sv */ { IDS_PLUS60K_BASE_SV, "Basadress för PLUS60K" },
/* tr */ { IDS_PLUS60K_BASE_TR, "PLUS60K taban adresi" },

/* en */ { IDS_PLUS60K_FILENAME,    "PLUS60K file" },
/* da */ { IDS_PLUS60K_FILENAME_DA, "PLUS60K-fil" },
/* de */ { IDS_PLUS60K_FILENAME_DE, "PLUS60K Datei" },
/* es */ { IDS_PLUS60K_FILENAME_ES, "Fichero PLUS60K" },
/* fr */ { IDS_PLUS60K_FILENAME_FR, "Fichier POLUS60K" },
/* hu */ { IDS_PLUS60K_FILENAME_HU, "PLUS60K fájl" },
/* it */ { IDS_PLUS60K_FILENAME_IT, "File PLUS60K" },
/* ko */ { IDS_PLUS60K_FILENAME_KO, "PLUS60K ÆÄÀÏ" },
/* nl */ { IDS_PLUS60K_FILENAME_NL, "PLUS60K bestand" },
/* pl */ { IDS_PLUS60K_FILENAME_PL, "Plik PLUS60K" },
/* ru */ { IDS_PLUS60K_FILENAME_RU, "PLUS60K file" },
/* sv */ { IDS_PLUS60K_FILENAME_SV, "PLUS60K-fil" },
/* tr */ { IDS_PLUS60K_FILENAME_TR, "PLUS60K dosyasý" },

/* en */ { IDS_PLUS60K_FILENAME_SELECT,    "Select file for PLUS60K" },
/* da */ { IDS_PLUS60K_FILENAME_SELECT_DA, "Vælg fil for PLUS60K" },
/* de */ { IDS_PLUS60K_FILENAME_SELECT_DE, "Datei für PLUS60K auswählen" },
/* es */ { IDS_PLUS60K_FILENAME_SELECT_ES, "Seleccionar fichero para PLUS60K" },
/* fr */ { IDS_PLUS60K_FILENAME_SELECT_FR, "Sélectionner fichier pour PLUS60K" },
/* hu */ { IDS_PLUS60K_FILENAME_SELECT_HU, "Válasszon fájlt a PLUS60K-hoz" },
/* it */ { IDS_PLUS60K_FILENAME_SELECT_IT, "Seleziona file per PLUS60K" },
/* ko */ { IDS_PLUS60K_FILENAME_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDS_PLUS60K_FILENAME_SELECT_NL, "Selecteer bestand voor PLUS60K" },
/* pl */ { IDS_PLUS60K_FILENAME_SELECT_PL, "Wybierz plik PLUS60K" },
/* ru */ { IDS_PLUS60K_FILENAME_SELECT_RU, "Select file for PLUS60K" },
/* sv */ { IDS_PLUS60K_FILENAME_SELECT_SV, "Ange fil för PLUS60K" },
/* tr */ { IDS_PLUS60K_FILENAME_SELECT_TR, "PLUS60K için dosya seçin" },

/* en */ { IDS_C64_MEMORY_HACKS_SETTINGS,    "Memory expansion hacks settings" },
/* da */ { IDS_C64_MEMORY_HACKS_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDS_C64_MEMORY_HACKS_SETTINGS_DE, "Speicher Erweiterungshack Einstellungen" },
/* es */ { IDS_C64_MEMORY_HACKS_SETTINGS_ES, "" },  /* fuzzy */
/* fr */ { IDS_C64_MEMORY_HACKS_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDS_C64_MEMORY_HACKS_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_C64_MEMORY_HACKS_SETTINGS_IT, "" },  /* fuzzy */
/* ko */ { IDS_C64_MEMORY_HACKS_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_C64_MEMORY_HACKS_SETTINGS_NL, "Geheugen uitbreiding hacks instellingen" },
/* pl */ { IDS_C64_MEMORY_HACKS_SETTINGS_PL, "Ustawienia hacków rozszerzenia pamiêci" },
/* ru */ { IDS_C64_MEMORY_HACKS_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_C64_MEMORY_HACKS_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDS_C64_MEMORY_HACKS_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDS_VALUE_FIRST_BYTE,    "Value of first byte" },
/* da */ { IDS_VALUE_FIRST_BYTE_DA, "Værdi på første byte" },
/* de */ { IDS_VALUE_FIRST_BYTE_DE, "Wert erstes Byte" },
/* es */ { IDS_VALUE_FIRST_BYTE_ES, "Valor del primer byte" },
/* fr */ { IDS_VALUE_FIRST_BYTE_FR, "Valeur du premier octet" },
/* hu */ { IDS_VALUE_FIRST_BYTE_HU, "Elsõ bájt értéke" },
/* it */ { IDS_VALUE_FIRST_BYTE_IT, "Valore del primo byte" },
/* ko */ { IDS_VALUE_FIRST_BYTE_KO, "" },  /* fuzzy */
/* nl */ { IDS_VALUE_FIRST_BYTE_NL, "Waarde van eerste byte" },
/* pl */ { IDS_VALUE_FIRST_BYTE_PL, "Warto¶æ pierwszego bajta" },
/* ru */ { IDS_VALUE_FIRST_BYTE_RU, "Value of first byte" },
/* sv */ { IDS_VALUE_FIRST_BYTE_SV, "Värde på första byte" },
/* tr */ { IDS_VALUE_FIRST_BYTE_TR, "Ýlk byte deðeri" },

/* en */ { IDS_LENGTH_CONSTANT_VALUES,    "Length of constant values" },
/* da */ { IDS_LENGTH_CONSTANT_VALUES_DA, "Længde af konstant værdi" },
/* de */ { IDS_LENGTH_CONSTANT_VALUES_DE, "Länge der konstanten Werte" },
/* es */ { IDS_LENGTH_CONSTANT_VALUES_ES, "Longitud de valores constantes" },
/* fr */ { IDS_LENGTH_CONSTANT_VALUES_FR, "Longeur des valeurs constantes" },
/* hu */ { IDS_LENGTH_CONSTANT_VALUES_HU, "Állandó értékek hossza" },
/* it */ { IDS_LENGTH_CONSTANT_VALUES_IT, "Lunghezza dei valori costanti" },
/* ko */ { IDS_LENGTH_CONSTANT_VALUES_KO, "" },  /* fuzzy */
/* nl */ { IDS_LENGTH_CONSTANT_VALUES_NL, "Lengte van constante waarden" },
/* pl */ { IDS_LENGTH_CONSTANT_VALUES_PL, "D³ugo¶æ sta³ych warto¶ci" },
/* ru */ { IDS_LENGTH_CONSTANT_VALUES_RU, "Length of constant values" },
/* sv */ { IDS_LENGTH_CONSTANT_VALUES_SV, "Längd på konstanta värden" },
/* tr */ { IDS_LENGTH_CONSTANT_VALUES_TR, "Sabit deðerlerin uzunluðu" },

/* en */ { IDS_LENGTH_CONSTANT_PATTERN,    "Length of constant pattern" },
/* da */ { IDS_LENGTH_CONSTANT_PATTERN_DA, "Længde af konstant mønster" },
/* de */ { IDS_LENGTH_CONSTANT_PATTERN_DE, "Länge des konstanten Musters" },
/* es */ { IDS_LENGTH_CONSTANT_PATTERN_ES, "Longitud de patrones constantes" },
/* fr */ { IDS_LENGTH_CONSTANT_PATTERN_FR, "Longeur des contantes pattern" },
/* hu */ { IDS_LENGTH_CONSTANT_PATTERN_HU, "Állandó minta hossza" },
/* it */ { IDS_LENGTH_CONSTANT_PATTERN_IT, "Lunghezza dei pattern costanti" },
/* ko */ { IDS_LENGTH_CONSTANT_PATTERN_KO, "" },  /* fuzzy */
/* nl */ { IDS_LENGTH_CONSTANT_PATTERN_NL, "Lengte van constant patroon" },
/* pl */ { IDS_LENGTH_CONSTANT_PATTERN_PL, "L³ugo¶æ sta³ego wzorca" },
/* ru */ { IDS_LENGTH_CONSTANT_PATTERN_RU, "Length of constant pattern" },
/* sv */ { IDS_LENGTH_CONSTANT_PATTERN_SV, "Längd på konstant mönster" },
/* tr */ { IDS_LENGTH_CONSTANT_PATTERN_TR, "Sabit desenlerin uzunluðu" },

/* en */ { IDS_RAM_SETTINGS,    "RAM settings" },
/* da */ { IDS_RAM_SETTINGS_DA, "RAM-indstillinger" },
/* de */ { IDS_RAM_SETTINGS_DE, "RAM Einstellungen" },
/* es */ { IDS_RAM_SETTINGS_ES, "Ajustes RAM" },
/* fr */ { IDS_RAM_SETTINGS_FR, "Paramètres de la RAM" },
/* hu */ { IDS_RAM_SETTINGS_HU, "RAM beállításai" },
/* it */ { IDS_RAM_SETTINGS_IT, "Impostazioni RAM" },
/* ko */ { IDS_RAM_SETTINGS_KO, "RAM ¼ÂÆÃ" },
/* nl */ { IDS_RAM_SETTINGS_NL, "RAM-instellingen" },
/* pl */ { IDS_RAM_SETTINGS_PL, "Ustawienia RAM" },
/* ru */ { IDS_RAM_SETTINGS_RU, "½ÐáâàÞÙÚØ RAM" },
/* sv */ { IDS_RAM_SETTINGS_SV, "RAM-inställningar" },
/* tr */ { IDS_RAM_SETTINGS_TR, "RAM ayarlarý" },

/* en */ { IDS_READ_WRITE,    "read/write" },
/* da */ { IDS_READ_WRITE_DA, "skrivbar" },
/* de */ { IDS_READ_WRITE_DE, "lese/schreibe" },
/* es */ { IDS_READ_WRITE_ES, "Leer/Grabar" },
/* fr */ { IDS_READ_WRITE_FR, "lecture/écriture" },
/* hu */ { IDS_READ_WRITE_HU, "írható/olvasható" },
/* it */ { IDS_READ_WRITE_IT, "lettura/scrittura" },
/* ko */ { IDS_READ_WRITE_KO, "ÀÐ±â/¾²±â" },
/* nl */ { IDS_READ_WRITE_NL, "lees/schrijf" },
/* pl */ { IDS_READ_WRITE_PL, "oczyt/zapis" },
/* ru */ { IDS_READ_WRITE_RU, "read/write" },
/* sv */ { IDS_READ_WRITE_SV, "skrivbar" },
/* tr */ { IDS_READ_WRITE_TR, "oku/yaz" },

/* en */ { IDS_READ_ONLY,    "Read-only" },
/* da */ { IDS_READ_ONLY_DA, "Kun læseadgang" },
/* de */ { IDS_READ_ONLY_DE, "Schreibgeschützt" },
/* es */ { IDS_READ_ONLY_ES, "Sólo lectura" },
/* fr */ { IDS_READ_ONLY_FR, "Lecture seule" },
/* hu */ { IDS_READ_ONLY_HU, "" },  /* fuzzy */
/* it */ { IDS_READ_ONLY_IT, "Sola lettura" },
/* ko */ { IDS_READ_ONLY_KO, "ÀÐ±â Àü¿ë" },
/* nl */ { IDS_READ_ONLY_NL, "Alleen-lezen" },
/* pl */ { IDS_READ_ONLY_PL, "Tylko do odczytu" },
/* ru */ { IDS_READ_ONLY_RU, "ÂÞÛìÚÞ çâÕÝØÕ" },
/* sv */ { IDS_READ_ONLY_SV, "Endast läsning" },
/* tr */ { IDS_READ_ONLY_TR, "Salt okunur" },

/* en */ { IDS_RAMCART_READ_WRITE,    "RamCart Read/Write" },
/* da */ { IDS_RAMCART_READ_WRITE_DA, "RamCart læs/skriv" },
/* de */ { IDS_RAMCART_READ_WRITE_DE, "RamCart Lesen/Schreiben" },
/* es */ { IDS_RAMCART_READ_WRITE_ES, "Lectura/Grabación RamCart" },
/* fr */ { IDS_RAMCART_READ_WRITE_FR, "lecture/Écriture RamCart" },
/* hu */ { IDS_RAMCART_READ_WRITE_HU, "" },  /* fuzzy */
/* it */ { IDS_RAMCART_READ_WRITE_IT, "RamCart in lettura/scrittura" },
/* ko */ { IDS_RAMCART_READ_WRITE_KO, "" },  /* fuzzy */
/* nl */ { IDS_RAMCART_READ_WRITE_NL, "RamCart Lees/Schrijf" },
/* pl */ { IDS_RAMCART_READ_WRITE_PL, "Odczyt i zapis na RamCart" },
/* ru */ { IDS_RAMCART_READ_WRITE_RU, "RamCart Read/Write" },
/* sv */ { IDS_RAMCART_READ_WRITE_SV, "Skrivbart RamCart" },
/* tr */ { IDS_RAMCART_READ_WRITE_TR, "RamCart Oku/Yaz" },

/* en */ { IDS_RAMCART_SIZE,    "RamCart size" },
/* da */ { IDS_RAMCART_SIZE_DA, "RamCart-størrelse" },
/* de */ { IDS_RAMCART_SIZE_DE, "RamCart Größe" },
/* es */ { IDS_RAMCART_SIZE_ES, "Tamaño RamCart" },
/* fr */ { IDS_RAMCART_SIZE_FR, "Taille de l'extention RAMCART" },
/* hu */ { IDS_RAMCART_SIZE_HU, "" },  /* fuzzy */
/* it */ { IDS_RAMCART_SIZE_IT, "Dimensione RamCart" },
/* ko */ { IDS_RAMCART_SIZE_KO, "" },  /* fuzzy */
/* nl */ { IDS_RAMCART_SIZE_NL, "RamCart grootte" },
/* pl */ { IDS_RAMCART_SIZE_PL, "Rozmiar RamCart" },
/* ru */ { IDS_RAMCART_SIZE_RU, "" },  /* fuzzy */
/* sv */ { IDS_RAMCART_SIZE_SV, "RamCart-storlek" },
/* tr */ { IDS_RAMCART_SIZE_TR, "RamCart boyutu" },

/* en */ { IDS_RAMCART_FILENAME,    "RamCart file" },
/* da */ { IDS_RAMCART_FILENAME_DA, "RamCart fil" },
/* de */ { IDS_RAMCART_FILENAME_DE, "RamCart Datei" },
/* es */ { IDS_RAMCART_FILENAME_ES, "Fichero RamCart" },
/* fr */ { IDS_RAMCART_FILENAME_FR, "Fichier RamCart" },
/* hu */ { IDS_RAMCART_FILENAME_HU, "" },  /* fuzzy */
/* it */ { IDS_RAMCART_FILENAME_IT, "File RamCart" },
/* ko */ { IDS_RAMCART_FILENAME_KO, "RamCart ÆÄÀÏ" },
/* nl */ { IDS_RAMCART_FILENAME_NL, "RamCart bestand" },
/* pl */ { IDS_RAMCART_FILENAME_PL, "Plik RamCart" },
/* ru */ { IDS_RAMCART_FILENAME_RU, "RamCart file" },
/* sv */ { IDS_RAMCART_FILENAME_SV, "RamCart-fil" },
/* tr */ { IDS_RAMCART_FILENAME_TR, "RamCart dosyasý" },

/* en */ { IDS_SAVE_RAMCART_IMAGE_WHEN_CHANGED,    "Save RamCart image when changed" },
/* da */ { IDS_SAVE_RAMCART_IMAGE_WHEN_CHANGED_DA, "Gem RamCart image ved ændringer" },
/* de */ { IDS_SAVE_RAMCART_IMAGE_WHEN_CHANGED_DE, "RamCart Imagedatei bei Änderung speichern" },
/* es */ { IDS_SAVE_RAMCART_IMAGE_WHEN_CHANGED_ES, "Grabar imagen RamChart cuando cambie" },
/* fr */ { IDS_SAVE_RAMCART_IMAGE_WHEN_CHANGED_FR, "Enregistrer l'image RAMCART lorsque modifiée" },
/* hu */ { IDS_SAVE_RAMCART_IMAGE_WHEN_CHANGED_HU, "" },  /* fuzzy */
/* it */ { IDS_SAVE_RAMCART_IMAGE_WHEN_CHANGED_IT, "Salva immagine RamCart alla modifica" },
/* ko */ { IDS_SAVE_RAMCART_IMAGE_WHEN_CHANGED_KO, "" },  /* fuzzy */
/* nl */ { IDS_SAVE_RAMCART_IMAGE_WHEN_CHANGED_NL, "Ramcart bestand opslaan bij wijziging" },
/* pl */ { IDS_SAVE_RAMCART_IMAGE_WHEN_CHANGED_PL, "Zapisuj obraz RamCart przy zmianie" },
/* ru */ { IDS_SAVE_RAMCART_IMAGE_WHEN_CHANGED_RU, "Save RamCart image when changed" },
/* sv */ { IDS_SAVE_RAMCART_IMAGE_WHEN_CHANGED_SV, "Spara RamCart-avbildning vid ändringar" },
/* tr */ { IDS_SAVE_RAMCART_IMAGE_WHEN_CHANGED_TR, "RamCart imajý deðiþtiðinde kaydet" },

/* en */ { IDS_RAMCART_FILENAME_SELECT,    "Select file for RamCart" },
/* da */ { IDS_RAMCART_FILENAME_SELECT_DA, "Vælg fil for RamCart" },
/* de */ { IDS_RAMCART_FILENAME_SELECT_DE, "Datei für RamCart auswählen" },
/* es */ { IDS_RAMCART_FILENAME_SELECT_ES, "Seleccionar fichero para RamCart" },
/* fr */ { IDS_RAMCART_FILENAME_SELECT_FR, "Sélectionner fichier pour RamCart" },
/* hu */ { IDS_RAMCART_FILENAME_SELECT_HU, "" },  /* fuzzy */
/* it */ { IDS_RAMCART_FILENAME_SELECT_IT, "Seleziona file per RamCart" },
/* ko */ { IDS_RAMCART_FILENAME_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDS_RAMCART_FILENAME_SELECT_NL, "Selecteer bestand voor RamCart" },
/* pl */ { IDS_RAMCART_FILENAME_SELECT_PL, "Wybierz plik RamCart" },
/* ru */ { IDS_RAMCART_FILENAME_SELECT_RU, "Select file for RamCart" },
/* sv */ { IDS_RAMCART_FILENAME_SELECT_SV, "Välj fil för RamCart" },
/* tr */ { IDS_RAMCART_FILENAME_SELECT_TR, "RamCart için dosya seçin" },

/* en */ { IDS_RAMCART_SETTINGS,    "RamCart settings" },
/* da */ { IDS_RAMCART_SETTINGS_DA, "RamCart-indstillinger" },
/* de */ { IDS_RAMCART_SETTINGS_DE, "RamCart Einstellungen" },
/* es */ { IDS_RAMCART_SETTINGS_ES, "Ajustes RamCart" },
/* fr */ { IDS_RAMCART_SETTINGS_FR, "Paramètres RamCart" },
/* hu */ { IDS_RAMCART_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_RAMCART_SETTINGS_IT, "Impostazioni RamCart " },
/* ko */ { IDS_RAMCART_SETTINGS_KO, "RamCart ¼ÂÆÃ" },
/* nl */ { IDS_RAMCART_SETTINGS_NL, "RamCart instellingen" },
/* pl */ { IDS_RAMCART_SETTINGS_PL, "Ustawienia RamCart" },
/* ru */ { IDS_RAMCART_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_RAMCART_SETTINGS_SV, "RamCart-inställningar" },
/* tr */ { IDS_RAMCART_SETTINGS_TR, "RamCart ayarlarý" },

/* en */ { IDS_REU_SIZE,    "REU size" },
/* da */ { IDS_REU_SIZE_DA, "REU størrelse" },
/* de */ { IDS_REU_SIZE_DE, "REU Größe" },
/* es */ { IDS_REU_SIZE_ES, "Tamaño REU" },
/* fr */ { IDS_REU_SIZE_FR, "Taille du REU" },
/* hu */ { IDS_REU_SIZE_HU, "REU mérete" },
/* it */ { IDS_REU_SIZE_IT, "Dimensione REU" },
/* ko */ { IDS_REU_SIZE_KO, "" },  /* fuzzy */
/* nl */ { IDS_REU_SIZE_NL, "REU grootte" },
/* pl */ { IDS_REU_SIZE_PL, "Rozmiar REU" },
/* ru */ { IDS_REU_SIZE_RU, "REU size" },
/* sv */ { IDS_REU_SIZE_SV, "Storlek på minnesexpansion" },
/* tr */ { IDS_REU_SIZE_TR, "REU boyutu" },

/* en */ { IDS_REU_FILENAME,    "REU file" },
/* da */ { IDS_REU_FILENAME_DA, "REU-fil" },
/* de */ { IDS_REU_FILENAME_DE, "REU Datei" },
/* es */ { IDS_REU_FILENAME_ES, "Fichero REU" },
/* fr */ { IDS_REU_FILENAME_FR, "Fichier REU" },
/* hu */ { IDS_REU_FILENAME_HU, "REU fájl" },
/* it */ { IDS_REU_FILENAME_IT, "File del REU" },
/* ko */ { IDS_REU_FILENAME_KO, "REU ÆÄÀÏ" },
/* nl */ { IDS_REU_FILENAME_NL, "REU bestand" },
/* pl */ { IDS_REU_FILENAME_PL, "Plik REU" },
/* ru */ { IDS_REU_FILENAME_RU, "REU file" },
/* sv */ { IDS_REU_FILENAME_SV, "REU-fil" },
/* tr */ { IDS_REU_FILENAME_TR, "REU dosyasý" },

/* en */ { IDS_SAVE_REU_IMAGE_WHEN_CHANGED,    "Save REU image when changed" },
/* da */ { IDS_SAVE_REU_IMAGE_WHEN_CHANGED_DA, "Gem REU image ved ændringer" },
/* de */ { IDS_SAVE_REU_IMAGE_WHEN_CHANGED_DE, "REU Imagedatei bei Änderung speichern" },
/* es */ { IDS_SAVE_REU_IMAGE_WHEN_CHANGED_ES, "Grabar imagen REU cuando cambie" },
/* fr */ { IDS_SAVE_REU_IMAGE_WHEN_CHANGED_FR, "Enregistrer l'image REU lorsque modifiée" },
/* hu */ { IDS_SAVE_REU_IMAGE_WHEN_CHANGED_HU, "" },  /* fuzzy */
/* it */ { IDS_SAVE_REU_IMAGE_WHEN_CHANGED_IT, "Salva immagine REU alla modifica" },
/* ko */ { IDS_SAVE_REU_IMAGE_WHEN_CHANGED_KO, "" },  /* fuzzy */
/* nl */ { IDS_SAVE_REU_IMAGE_WHEN_CHANGED_NL, "REU bestand opslaan bij wijziging" },
/* pl */ { IDS_SAVE_REU_IMAGE_WHEN_CHANGED_PL, "Zapisuj obraz REU przy zmianie" },
/* ru */ { IDS_SAVE_REU_IMAGE_WHEN_CHANGED_RU, "Save REU image when changed" },
/* sv */ { IDS_SAVE_REU_IMAGE_WHEN_CHANGED_SV, "Spara REU-avbildning vid ändringar" },
/* tr */ { IDS_SAVE_REU_IMAGE_WHEN_CHANGED_TR, "REU imajý deðiþtiðinde kaydet" },

/* en */ { IDS_REU_FILENAME_SELECT,    "Select file for REU" },
/* da */ { IDS_REU_FILENAME_SELECT_DA, "Vælg fil for REU" },
/* de */ { IDS_REU_FILENAME_SELECT_DE, "Datei für REU auswählen" },
/* es */ { IDS_REU_FILENAME_SELECT_ES, "Seleccionar fichero para REU" },
/* fr */ { IDS_REU_FILENAME_SELECT_FR, "Sélectionner fichier pour REU" },
/* hu */ { IDS_REU_FILENAME_SELECT_HU, "Válasszon fájlt a REU-hoz" },
/* it */ { IDS_REU_FILENAME_SELECT_IT, "Seleziona il file per il REU" },
/* ko */ { IDS_REU_FILENAME_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDS_REU_FILENAME_SELECT_NL, "Selecteer bestand voor REU" },
/* pl */ { IDS_REU_FILENAME_SELECT_PL, "Wybierz plik REU" },
/* ru */ { IDS_REU_FILENAME_SELECT_RU, "Select file for REU" },
/* sv */ { IDS_REU_FILENAME_SELECT_SV, "Välj fil för REU" },
/* tr */ { IDS_REU_FILENAME_SELECT_TR, "REU için dosya seçin" },

/* en */ { IDS_REU_SETTINGS,    "REU settings" },
/* da */ { IDS_REU_SETTINGS_DA, "REU-indstillinger" },
/* de */ { IDS_REU_SETTINGS_DE, "REU Einstellungen" },
/* es */ { IDS_REU_SETTINGS_ES, "Ajustes REU" },
/* fr */ { IDS_REU_SETTINGS_FR, "Paramètres REU" },
/* hu */ { IDS_REU_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_REU_SETTINGS_IT, "Impostazioni REU" },
/* ko */ { IDS_REU_SETTINGS_KO, "REU ¼ÂÆÃ" },
/* nl */ { IDS_REU_SETTINGS_NL, "REU instellingen" },
/* pl */ { IDS_REU_SETTINGS_PL, "Ustawienia REU" },
/* ru */ { IDS_REU_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_REU_SETTINGS_SV, "REU-Inställningar" },
/* tr */ { IDS_REU_SETTINGS_TR, "REU Ayarlarý" },

/* en */ { IDS_BURST_MOD_SETTINGS,    "Drive burst modification settings" },
/* da */ { IDS_BURST_MOD_SETTINGS_DA, "Drev burst-modifikations-indstillinger" },
/* de */ { IDS_BURST_MOD_SETTINGS_DE, "Laufwerk Burstmodifkation Einstellungen" },
/* es */ { IDS_BURST_MOD_SETTINGS_ES, "Ajuste modificación Burst en controlador" },
/* fr */ { IDS_BURST_MOD_SETTINGS_FR, "Activer l'émulation de Burst Lecteur" },
/* hu */ { IDS_BURST_MOD_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_BURST_MOD_SETTINGS_IT, "Impostazioni modifica burst del drive" },
/* ko */ { IDS_BURST_MOD_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_BURST_MOD_SETTINGS_NL, "Drive burst modificatie instellingen" },
/* pl */ { IDS_BURST_MOD_SETTINGS_PL, "Ustawienia trybu synchronicznego napêdu" },
/* ru */ { IDS_BURST_MOD_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_BURST_MOD_SETTINGS_SV, "Inställningar för diskettburstmodifiering" },
/* tr */ { IDS_BURST_MOD_SETTINGS_TR, "Sürücü atlama modifikasyon ayarlarý" },

/* en */ { IDS_PALETTE_SETTINGS,    "Palette settings" },
/* da */ { IDS_PALETTE_SETTINGS_DA, "Paletteindstillinger" },
/* de */ { IDS_PALETTE_SETTINGS_DE, "Paletteneinstellungen" },
/* es */ { IDS_PALETTE_SETTINGS_ES, "Ajustes Paleta" },
/* fr */ { IDS_PALETTE_SETTINGS_FR, "Paramètres Palette" },
/* hu */ { IDS_PALETTE_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_PALETTE_SETTINGS_IT, "Impostazioni palette" },
/* ko */ { IDS_PALETTE_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_PALETTE_SETTINGS_NL, "Palet instellingen" },
/* pl */ { IDS_PALETTE_SETTINGS_PL, "Ustawienia palety barw" },
/* ru */ { IDS_PALETTE_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_PALETTE_SETTINGS_SV, "Palettinställningar" },
/* tr */ { IDS_PALETTE_SETTINGS_TR, "Palet ayarlarý" },

/* en */ { IDS_COLOR_SETTINGS,    "Color settings" },
/* da */ { IDS_COLOR_SETTINGS_DA, "Farveindstillinger" },
/* de */ { IDS_COLOR_SETTINGS_DE, "Farbeinstellungen" },
/* es */ { IDS_COLOR_SETTINGS_ES, "Ajustes color" },
/* fr */ { IDS_COLOR_SETTINGS_FR, "Paramètres de couleurs" },
/* hu */ { IDS_COLOR_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_COLOR_SETTINGS_IT, "Impostazioni colori" },
/* ko */ { IDS_COLOR_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_COLOR_SETTINGS_NL, "Kleurinstellingen" },
/* pl */ { IDS_COLOR_SETTINGS_PL, "Ustawienia koloru" },
/* ru */ { IDS_COLOR_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_COLOR_SETTINGS_SV, "Färginställningar" },
/* tr */ { IDS_COLOR_SETTINGS_TR, "Renk ayarlarý" },

/* en */ { IDS_RENDER_FILTER_SETTINGS,    "Render filter settings" },
/* da */ { IDS_RENDER_FILTER_SETTINGS_DA, "Renderingsfilterindstillinger" },
/* de */ { IDS_RENDER_FILTER_SETTINGS_DE, "Render Filter Einstellungen" },
/* es */ { IDS_RENDER_FILTER_SETTINGS_ES, "Ajuste filtro de renderizado" },
/* fr */ { IDS_RENDER_FILTER_SETTINGS_FR, "Paramètres filtres de rendu" },
/* hu */ { IDS_RENDER_FILTER_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_RENDER_FILTER_SETTINGS_IT, "Impostazioni filtro di render" },
/* ko */ { IDS_RENDER_FILTER_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_RENDER_FILTER_SETTINGS_NL, "Render filter instellingen" },
/* pl */ { IDS_RENDER_FILTER_SETTINGS_PL, "Ustawienia filtra renderera" },
/* ru */ { IDS_RENDER_FILTER_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_RENDER_FILTER_SETTINGS_SV, "Inställningar för rendrerarfilter" },
/* tr */ { IDS_RENDER_FILTER_SETTINGS_TR, "Ýmge oluþturma filtresi ayarlarý" },

/* en */ { IDS_CRT_SETTINGS,    "CRT emulation settings" },
/* da */ { IDS_CRT_SETTINGS_DA, "Indstillinger for CRT-emulering" },
/* de */ { IDS_CRT_SETTINGS_DE, "CRT Emulator Einstellungen" },
/* es */ { IDS_CRT_SETTINGS_ES, "Ajustes de emulación CRT" },
/* fr */ { IDS_CRT_SETTINGS_FR, "Paramètres d'émulation CRT" },
/* hu */ { IDS_CRT_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_CRT_SETTINGS_IT, "Impostazioni emulazione CRT" },
/* ko */ { IDS_CRT_SETTINGS_KO, "ÄÉÅäµå ·¹ÀÌ ÅÍ¹Ì³Î ¼³Á¤" },
/* nl */ { IDS_CRT_SETTINGS_NL, "CRT emulatie instellingen" },
/* pl */ { IDS_CRT_SETTINGS_PL, "Ustawienia emulacji CRT" },
/* ru */ { IDS_CRT_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_CRT_SETTINGS_SV, "Inställningar för CRT-emulering" },
/* tr */ { IDS_CRT_SETTINGS_TR, "CRT emülasyonu ayarlarý" },

/* en */ { IDS_VICII_PALETTE_FILENAME,    "VICII palette file" },
/* da */ { IDS_VICII_PALETTE_FILENAME_DA, "VICII-palettefil" },
/* de */ { IDS_VICII_PALETTE_FILENAME_DE, "VIC-II Palettendatei" },
/* es */ { IDS_VICII_PALETTE_FILENAME_ES, "Fichero paleta VICII" },
/* fr */ { IDS_VICII_PALETTE_FILENAME_FR, "Fichier de palette VICII" },
/* hu */ { IDS_VICII_PALETTE_FILENAME_HU, "" },  /* fuzzy */
/* it */ { IDS_VICII_PALETTE_FILENAME_IT, "File palette VICII" },
/* ko */ { IDS_VICII_PALETTE_FILENAME_KO, "" },  /* fuzzy */
/* nl */ { IDS_VICII_PALETTE_FILENAME_NL, "VICII palet bestand" },
/* pl */ { IDS_VICII_PALETTE_FILENAME_PL, "Plik palety VICII" },
/* ru */ { IDS_VICII_PALETTE_FILENAME_RU, "" },  /* fuzzy */
/* sv */ { IDS_VICII_PALETTE_FILENAME_SV, "VICII-palettfil" },
/* tr */ { IDS_VICII_PALETTE_FILENAME_TR, "VICII palet dosyasý" },

/* en */ { IDS_VIC_PALETTE_FILENAME,    "VIC palette file" },
/* da */ { IDS_VIC_PALETTE_FILENAME_DA, "VIC-palettefil" },
/* de */ { IDS_VIC_PALETTE_FILENAME_DE, "VIC Palettendatei" },
/* es */ { IDS_VIC_PALETTE_FILENAME_ES, "Fichero paleta VIC" },
/* fr */ { IDS_VIC_PALETTE_FILENAME_FR, "Fichier de palette VIC" },
/* hu */ { IDS_VIC_PALETTE_FILENAME_HU, "" },  /* fuzzy */
/* it */ { IDS_VIC_PALETTE_FILENAME_IT, "File palette VIC" },
/* ko */ { IDS_VIC_PALETTE_FILENAME_KO, "" },  /* fuzzy */
/* nl */ { IDS_VIC_PALETTE_FILENAME_NL, "VIC palet bestand" },
/* pl */ { IDS_VIC_PALETTE_FILENAME_PL, "Plik palety VIC" },
/* ru */ { IDS_VIC_PALETTE_FILENAME_RU, "" },  /* fuzzy */
/* sv */ { IDS_VIC_PALETTE_FILENAME_SV, "VIC-palettfil" },
/* tr */ { IDS_VIC_PALETTE_FILENAME_TR, "VIC palet dosyasý" },

/* en */ { IDS_VDC_PALETTE_FILENAME,    "VDC palette file" },
/* da */ { IDS_VDC_PALETTE_FILENAME_DA, "VDC-palettefil" },
/* de */ { IDS_VDC_PALETTE_FILENAME_DE, "VDC Palettendatei" },
/* es */ { IDS_VDC_PALETTE_FILENAME_ES, "Fichero paleta VDC" },
/* fr */ { IDS_VDC_PALETTE_FILENAME_FR, "Fichier de palette VDC" },
/* hu */ { IDS_VDC_PALETTE_FILENAME_HU, "" },  /* fuzzy */
/* it */ { IDS_VDC_PALETTE_FILENAME_IT, "File palette VDC" },
/* ko */ { IDS_VDC_PALETTE_FILENAME_KO, "" },  /* fuzzy */
/* nl */ { IDS_VDC_PALETTE_FILENAME_NL, "VDC palet bestand" },
/* pl */ { IDS_VDC_PALETTE_FILENAME_PL, "Plik palety VDC" },
/* ru */ { IDS_VDC_PALETTE_FILENAME_RU, "" },  /* fuzzy */
/* sv */ { IDS_VDC_PALETTE_FILENAME_SV, "VDC-palettfil" },
/* tr */ { IDS_VDC_PALETTE_FILENAME_TR, "VDC palet dosyasý" },

/* en */ { IDS_CRTC_PALETTE_FILENAME,    "CRTC palette file" },
/* da */ { IDS_CRTC_PALETTE_FILENAME_DA, "CRTC-palettefil" },
/* de */ { IDS_CRTC_PALETTE_FILENAME_DE, "CRTC Palettendatei" },
/* es */ { IDS_CRTC_PALETTE_FILENAME_ES, "Fichero paleta CRT" },
/* fr */ { IDS_CRTC_PALETTE_FILENAME_FR, "Fichier de palette CTRC" },
/* hu */ { IDS_CRTC_PALETTE_FILENAME_HU, "" },  /* fuzzy */
/* it */ { IDS_CRTC_PALETTE_FILENAME_IT, "File palette CRTC" },
/* ko */ { IDS_CRTC_PALETTE_FILENAME_KO, "" },  /* fuzzy */
/* nl */ { IDS_CRTC_PALETTE_FILENAME_NL, "CRTC palet bestand" },
/* pl */ { IDS_CRTC_PALETTE_FILENAME_PL, "Plik palety CRTC" },
/* ru */ { IDS_CRTC_PALETTE_FILENAME_RU, "" },  /* fuzzy */
/* sv */ { IDS_CRTC_PALETTE_FILENAME_SV, "CRTC-palettfil" },
/* tr */ { IDS_CRTC_PALETTE_FILENAME_TR, "CRTC palet dosyasý" },

/* en */ { IDS_TED_PALETTE_FILENAME,    "TED palette file" },
/* da */ { IDS_TED_PALETTE_FILENAME_DA, "TED-palettefil" },
/* de */ { IDS_TED_PALETTE_FILENAME_DE, "TED Palettendatei" },
/* es */ { IDS_TED_PALETTE_FILENAME_ES, "Fichero paleta TED" },
/* fr */ { IDS_TED_PALETTE_FILENAME_FR, "Fichier de palette TED" },
/* hu */ { IDS_TED_PALETTE_FILENAME_HU, "" },  /* fuzzy */
/* it */ { IDS_TED_PALETTE_FILENAME_IT, "File palette TED" },
/* ko */ { IDS_TED_PALETTE_FILENAME_KO, "" },  /* fuzzy */
/* nl */ { IDS_TED_PALETTE_FILENAME_NL, "TED palet bestand" },
/* pl */ { IDS_TED_PALETTE_FILENAME_PL, "Plik palety TED" },
/* ru */ { IDS_TED_PALETTE_FILENAME_RU, "TED palette file" },
/* sv */ { IDS_TED_PALETTE_FILENAME_SV, "TED-palettfil" },
/* tr */ { IDS_TED_PALETTE_FILENAME_TR, "TED palet dosyasý" },

/* en */ { IDS_PRINTER_OUTPUT_FILENAME_SELECT,    "Select printer dump file" },
/* da */ { IDS_PRINTER_OUTPUT_FILENAME_SELECT_DA, "Vælg udskriftsdumpfil" },
/* de */ { IDS_PRINTER_OUTPUT_FILENAME_SELECT_DE, "Drucker Ausgabedatei wählen" },
/* es */ { IDS_PRINTER_OUTPUT_FILENAME_SELECT_ES, "Seleccionar fichero de volcado impresion" },
/* fr */ { IDS_PRINTER_OUTPUT_FILENAME_SELECT_FR, "Sélectionnez le fichier de sortie d'imprimante" },
/* hu */ { IDS_PRINTER_OUTPUT_FILENAME_SELECT_HU, "Válassza ki a nyomtató kimeneti fájlt" },
/* it */ { IDS_PRINTER_OUTPUT_FILENAME_SELECT_IT, "Seleziona file su cui stampare" },
/* ko */ { IDS_PRINTER_OUTPUT_FILENAME_SELECT_KO, "ÇÁ¸°ÅÍ ´ýÇÁ ÆÄÀÏÀ» ¼±ÅÃÇÏ¼¼¿ä" },
/* nl */ { IDS_PRINTER_OUTPUT_FILENAME_SELECT_NL, "Selecteer printerdumpbestand" },
/* pl */ { IDS_PRINTER_OUTPUT_FILENAME_SELECT_PL, "Wybierz plik zrzutu drukarki" },
/* ru */ { IDS_PRINTER_OUTPUT_FILENAME_SELECT_RU, "Select printer dump file" },
/* sv */ { IDS_PRINTER_OUTPUT_FILENAME_SELECT_SV, "Välj utskriftsdumpfil" },
/* tr */ { IDS_PRINTER_OUTPUT_FILENAME_SELECT_TR, "Yazýcý döküm dosyasýný seçin" },

/* en */ { IDS_PRINTER_4_EMULATION,    "Printer #4 emulation" },
/* da */ { IDS_PRINTER_4_EMULATION_DA, "Printer #4 emulering" },
/* de */ { IDS_PRINTER_4_EMULATION_DE, "Drucker #4 Emulation" },
/* es */ { IDS_PRINTER_4_EMULATION_ES, "Emulación impresora #4" },
/* fr */ { IDS_PRINTER_4_EMULATION_FR, "Émulation d'imprimante 4" },
/* hu */ { IDS_PRINTER_4_EMULATION_HU, "" },  /* fuzzy */
/* it */ { IDS_PRINTER_4_EMULATION_IT, "Emulazione stampante #4" },
/* ko */ { IDS_PRINTER_4_EMULATION_KO, "ÇÁ¸°ÅÍ #4 ¿¡¹Ä·¹ÀÌ¼Ç" },
/* nl */ { IDS_PRINTER_4_EMULATION_NL, "Printer #4 emulatie" },
/* pl */ { IDS_PRINTER_4_EMULATION_PL, "Emulacja drukarki #4" },
/* ru */ { IDS_PRINTER_4_EMULATION_RU, "" },  /* fuzzy */
/* sv */ { IDS_PRINTER_4_EMULATION_SV, "Emulering av skrivare 4" },
/* tr */ { IDS_PRINTER_4_EMULATION_TR, "Yazýcý #4 emülasyonu" },

/* en */ { IDS_PRINTER_5_EMULATION,    "Printer #5 emulation" },
/* da */ { IDS_PRINTER_5_EMULATION_DA, "Printer #5 emulering" },
/* de */ { IDS_PRINTER_5_EMULATION_DE, "Drucker #5 Emulation" },
/* es */ { IDS_PRINTER_5_EMULATION_ES, "Emulación impresora #5" },
/* fr */ { IDS_PRINTER_5_EMULATION_FR, "Émulation d'imprimante 5" },
/* hu */ { IDS_PRINTER_5_EMULATION_HU, "" },  /* fuzzy */
/* it */ { IDS_PRINTER_5_EMULATION_IT, "Emulazione stampante #5" },
/* ko */ { IDS_PRINTER_5_EMULATION_KO, "ÇÁ¸°ÅÍ #5 ¿¡¹Ä·¹ÀÌ¼Ç" },
/* nl */ { IDS_PRINTER_5_EMULATION_NL, "Printer #5 emulatie" },
/* pl */ { IDS_PRINTER_5_EMULATION_PL, "Emulacja drukarki #5" },
/* ru */ { IDS_PRINTER_5_EMULATION_RU, "" },  /* fuzzy */
/* sv */ { IDS_PRINTER_5_EMULATION_SV, "Emulering av skrivare 5" },
/* tr */ { IDS_PRINTER_5_EMULATION_TR, "Yazýcý #5 emülasyonu" },

/* en */ { IDS_PRINTER_6_EMULATION,    "Printer #6 emulation" },
/* da */ { IDS_PRINTER_6_EMULATION_DA, "" },  /* fuzzy */
/* de */ { IDS_PRINTER_6_EMULATION_DE, "Drucker #6 Emulation" },
/* es */ { IDS_PRINTER_6_EMULATION_ES, "Emulación impresora #6" },
/* fr */ { IDS_PRINTER_6_EMULATION_FR, "" },  /* fuzzy */
/* hu */ { IDS_PRINTER_6_EMULATION_HU, "" },  /* fuzzy */
/* it */ { IDS_PRINTER_6_EMULATION_IT, "Emulazione stampante #6" },
/* ko */ { IDS_PRINTER_6_EMULATION_KO, "" },  /* fuzzy */
/* nl */ { IDS_PRINTER_6_EMULATION_NL, "Printer #6 emulatie" },
/* pl */ { IDS_PRINTER_6_EMULATION_PL, "Emulacja drukarki #6" },
/* ru */ { IDS_PRINTER_6_EMULATION_RU, "" },  /* fuzzy */
/* sv */ { IDS_PRINTER_6_EMULATION_SV, "" },  /* fuzzy */
/* tr */ { IDS_PRINTER_6_EMULATION_TR, "" },  /* fuzzy */

/* en */ { IDS_USERPORT_PRINTER_EMULATION,    "Userport printer emulation" },
/* da */ { IDS_USERPORT_PRINTER_EMULATION_DA, "Emulering af printer på brugerporten" },
/* de */ { IDS_USERPORT_PRINTER_EMULATION_DE, "Userport Drucker Emulation" },
/* es */ { IDS_USERPORT_PRINTER_EMULATION_ES, "Emulación puerto usuario impresora" },
/* fr */ { IDS_USERPORT_PRINTER_EMULATION_FR, "Émulation d'imprimante port utilisateur" },
/* hu */ { IDS_USERPORT_PRINTER_EMULATION_HU, "" },  /* fuzzy */
/* it */ { IDS_USERPORT_PRINTER_EMULATION_IT, "Stampante su userport: emulazione" },
/* ko */ { IDS_USERPORT_PRINTER_EMULATION_KO, "À¯Á®Æ÷Æ® ÇÁ¸°ÅÍ ¿¡¹Ä·¹ÀÌ¼Ç" },
/* nl */ { IDS_USERPORT_PRINTER_EMULATION_NL, "Userport printer emulatie" },
/* pl */ { IDS_USERPORT_PRINTER_EMULATION_PL, "Emulacja drukarki w userporcie" },
/* ru */ { IDS_USERPORT_PRINTER_EMULATION_RU, "" },  /* fuzzy */
/* sv */ { IDS_USERPORT_PRINTER_EMULATION_SV, "Emulering av skrivare på användarporten" },
/* tr */ { IDS_USERPORT_PRINTER_EMULATION_TR, "Userport yazýcýsý emülasyonu" },

/* en */ { IDS_PRINTER_4_DRIVER,    "Printer #4 driver" },
/* da */ { IDS_PRINTER_4_DRIVER_DA, "Printer #4 driver" },
/* de */ { IDS_PRINTER_4_DRIVER_DE, "Drucker #4 Treiber" },
/* es */ { IDS_PRINTER_4_DRIVER_ES, "Controlador impresora #4" },
/* fr */ { IDS_PRINTER_4_DRIVER_FR, "Pilote d'imprimante 4" },
/* hu */ { IDS_PRINTER_4_DRIVER_HU, "" },  /* fuzzy */
/* it */ { IDS_PRINTER_4_DRIVER_IT, "Driver stampante #4" },
/* ko */ { IDS_PRINTER_4_DRIVER_KO, "ÇÁ¸°ÅÍ #4 µå¶óÀÌ¹ö" },
/* nl */ { IDS_PRINTER_4_DRIVER_NL, "Printer #4 stuurprogramma" },
/* pl */ { IDS_PRINTER_4_DRIVER_PL, "Sterownik drukarki #4" },
/* ru */ { IDS_PRINTER_4_DRIVER_RU, "Printer #4 driver" },
/* sv */ { IDS_PRINTER_4_DRIVER_SV, "Drivrutin för skrivare 4" },
/* tr */ { IDS_PRINTER_4_DRIVER_TR, "Yazýcý #4 sürücüsü" },

/* en */ { IDS_PRINTER_5_DRIVER,    "Printer #5 driver" },
/* da */ { IDS_PRINTER_5_DRIVER_DA, "Printer #5 driver" },
/* de */ { IDS_PRINTER_5_DRIVER_DE, "Drucker #5 Treiber" },
/* es */ { IDS_PRINTER_5_DRIVER_ES, "Controlador impresora #5" },
/* fr */ { IDS_PRINTER_5_DRIVER_FR, "Pilote d'imprimante 5" },
/* hu */ { IDS_PRINTER_5_DRIVER_HU, "" },  /* fuzzy */
/* it */ { IDS_PRINTER_5_DRIVER_IT, "Driver stampante #5" },
/* ko */ { IDS_PRINTER_5_DRIVER_KO, "IEC ÇÁ¸°ÅÍ #5 µå¶óÀÌ¹ö" },
/* nl */ { IDS_PRINTER_5_DRIVER_NL, "Printer #5 stuurprogramma" },
/* pl */ { IDS_PRINTER_5_DRIVER_PL, "Sterownik drukarki #5" },
/* ru */ { IDS_PRINTER_5_DRIVER_RU, "Printer #5 driver" },
/* sv */ { IDS_PRINTER_5_DRIVER_SV, "Drivrutin för skrivare 5" },
/* tr */ { IDS_PRINTER_5_DRIVER_TR, "Yazýcý #5 sürücüsü" },

/* en */ { IDS_PRINTER_6_DRIVER,    "Printer #6 driver" },
/* da */ { IDS_PRINTER_6_DRIVER_DA, "" },  /* fuzzy */
/* de */ { IDS_PRINTER_6_DRIVER_DE, "Drucker #6 Treiber" },
/* es */ { IDS_PRINTER_6_DRIVER_ES, "Controlador impresora #6" },
/* fr */ { IDS_PRINTER_6_DRIVER_FR, "" },  /* fuzzy */
/* hu */ { IDS_PRINTER_6_DRIVER_HU, "" },  /* fuzzy */
/* it */ { IDS_PRINTER_6_DRIVER_IT, "Driver stampante #6" },
/* ko */ { IDS_PRINTER_6_DRIVER_KO, "" },  /* fuzzy */
/* nl */ { IDS_PRINTER_6_DRIVER_NL, "Printer #6 stuurprogramma" },
/* pl */ { IDS_PRINTER_6_DRIVER_PL, "Sterownik drukarki #6" },
/* ru */ { IDS_PRINTER_6_DRIVER_RU, "" },  /* fuzzy */
/* sv */ { IDS_PRINTER_6_DRIVER_SV, "" },  /* fuzzy */
/* tr */ { IDS_PRINTER_6_DRIVER_TR, "" },  /* fuzzy */

/* en */ { IDS_USERPORT_PRINTER_DRIVER,    "Userport printer driver" },
/* da */ { IDS_USERPORT_PRINTER_DRIVER_DA, "Printerdriver for brugerporten" },
/* de */ { IDS_USERPORT_PRINTER_DRIVER_DE, "Userport Drucker Treiber" },
/* es */ { IDS_USERPORT_PRINTER_DRIVER_ES, "controlador puerto usuario" },
/* fr */ { IDS_USERPORT_PRINTER_DRIVER_FR, "Pilote de l'imprimante sur port utilisateur" },
/* hu */ { IDS_USERPORT_PRINTER_DRIVER_HU, "Userport-os nyomtató meghajtó" },
/* it */ { IDS_USERPORT_PRINTER_DRIVER_IT, "Stampante su userport: driver" },
/* ko */ { IDS_USERPORT_PRINTER_DRIVER_KO, "À¯Á®Æ÷Æ® ÇÁ¸°ÅÍ µå¶óÀÌ¹ö" },
/* nl */ { IDS_USERPORT_PRINTER_DRIVER_NL, "Userport printer stuurprogramma" },
/* pl */ { IDS_USERPORT_PRINTER_DRIVER_PL, "Sterownik drukarki w userporcie" },
/* ru */ { IDS_USERPORT_PRINTER_DRIVER_RU, "Userport printer driver" },
/* sv */ { IDS_USERPORT_PRINTER_DRIVER_SV, "Skrivardrivrutin för användarporten" },
/* tr */ { IDS_USERPORT_PRINTER_DRIVER_TR, "Userport yazýcýsý sürücüsü" },

/* en */ { IDS_PRINTER_4_OUTPUT_TYPE,    "Printer #4 output type" },
/* da */ { IDS_PRINTER_4_OUTPUT_TYPE_DA, "Printer #4 uddatatype" },
/* de */ { IDS_PRINTER_4_OUTPUT_TYPE_DE, "Drucker #4 Ausgabetyp" },
/* es */ { IDS_PRINTER_4_OUTPUT_TYPE_ES, "Tipo salida impresora #4" },
/* fr */ { IDS_PRINTER_4_OUTPUT_TYPE_FR, "Type de sortie de l'imprimante 4" },
/* hu */ { IDS_PRINTER_4_OUTPUT_TYPE_HU, "" },  /* fuzzy */
/* it */ { IDS_PRINTER_4_OUTPUT_TYPE_IT, "Tipo di output stampante #4" },
/* ko */ { IDS_PRINTER_4_OUTPUT_TYPE_KO, "" },  /* fuzzy */
/* nl */ { IDS_PRINTER_4_OUTPUT_TYPE_NL, "Printer #4 uitvoer soort" },
/* pl */ { IDS_PRINTER_4_OUTPUT_TYPE_PL, "Typ wyj¶cia drukarki #4" },
/* ru */ { IDS_PRINTER_4_OUTPUT_TYPE_RU, "Printer #4 output type" },
/* sv */ { IDS_PRINTER_4_OUTPUT_TYPE_SV, "Utdatatyp för skrivare 4" },
/* tr */ { IDS_PRINTER_4_OUTPUT_TYPE_TR, "Yazýcý #4 çýktý tipi" },

/* en */ { IDS_PRINTER_5_OUTPUT_TYPE,    "Printer #5 output type" },
/* da */ { IDS_PRINTER_5_OUTPUT_TYPE_DA, "Printer #5 uddatatype" },
/* de */ { IDS_PRINTER_5_OUTPUT_TYPE_DE, "Drucker #5 Ausgabetyp" },
/* es */ { IDS_PRINTER_5_OUTPUT_TYPE_ES, "Tipo salida impresora #5" },
/* fr */ { IDS_PRINTER_5_OUTPUT_TYPE_FR, "Type de sortie de l'imprimante 5" },
/* hu */ { IDS_PRINTER_5_OUTPUT_TYPE_HU, "" },  /* fuzzy */
/* it */ { IDS_PRINTER_5_OUTPUT_TYPE_IT, "Tipo di output stampante #5" },
/* ko */ { IDS_PRINTER_5_OUTPUT_TYPE_KO, "" },  /* fuzzy */
/* nl */ { IDS_PRINTER_5_OUTPUT_TYPE_NL, "Printer #5 uitvoer soort" },
/* pl */ { IDS_PRINTER_5_OUTPUT_TYPE_PL, "Typ wyj¶cia drukarki #5" },
/* ru */ { IDS_PRINTER_5_OUTPUT_TYPE_RU, "Printer #5 output type" },
/* sv */ { IDS_PRINTER_5_OUTPUT_TYPE_SV, "Utdatatyp för skrivare 5" },
/* tr */ { IDS_PRINTER_5_OUTPUT_TYPE_TR, "Yazýcý #5 çýktý tipi" },

/* en */ { IDS_PRINTER_6_OUTPUT_TYPE,    "Printer #6 output type" },
/* da */ { IDS_PRINTER_6_OUTPUT_TYPE_DA, "" },  /* fuzzy */
/* de */ { IDS_PRINTER_6_OUTPUT_TYPE_DE, "Drucker #6 Ausgabetyp" },
/* es */ { IDS_PRINTER_6_OUTPUT_TYPE_ES, "Tipo salida impresora #6" },
/* fr */ { IDS_PRINTER_6_OUTPUT_TYPE_FR, "" },  /* fuzzy */
/* hu */ { IDS_PRINTER_6_OUTPUT_TYPE_HU, "" },  /* fuzzy */
/* it */ { IDS_PRINTER_6_OUTPUT_TYPE_IT, "Tipo di output stampante #6" },
/* ko */ { IDS_PRINTER_6_OUTPUT_TYPE_KO, "" },  /* fuzzy */
/* nl */ { IDS_PRINTER_6_OUTPUT_TYPE_NL, "Printer #6 uitvoer soort" },
/* pl */ { IDS_PRINTER_6_OUTPUT_TYPE_PL, "Typ wyj¶cia drukarki #6" },
/* ru */ { IDS_PRINTER_6_OUTPUT_TYPE_RU, "" },  /* fuzzy */
/* sv */ { IDS_PRINTER_6_OUTPUT_TYPE_SV, "" },  /* fuzzy */
/* tr */ { IDS_PRINTER_6_OUTPUT_TYPE_TR, "" },  /* fuzzy */

/* en */ { IDS_USERPORT_PRINTER_OUTPUT_TYPE,    "Userport printer output type" },
/* da */ { IDS_USERPORT_PRINTER_OUTPUT_TYPE_DA, "Printer uddatatype på brugerporten" },
/* de */ { IDS_USERPORT_PRINTER_OUTPUT_TYPE_DE, "Userport Drucker Ausgabetyp" },
/* es */ { IDS_USERPORT_PRINTER_OUTPUT_TYPE_ES, "Tipo salida puerto impresora" },
/* fr */ { IDS_USERPORT_PRINTER_OUTPUT_TYPE_FR, "Type de sortie de l'imprimante port utilisateur" },
/* hu */ { IDS_USERPORT_PRINTER_OUTPUT_TYPE_HU, "" },  /* fuzzy */
/* it */ { IDS_USERPORT_PRINTER_OUTPUT_TYPE_IT, "Tipo di output stampante su userport" },
/* ko */ { IDS_USERPORT_PRINTER_OUTPUT_TYPE_KO, "" },  /* fuzzy */
/* nl */ { IDS_USERPORT_PRINTER_OUTPUT_TYPE_NL, "Userport printer uitvoer soort" },
/* pl */ { IDS_USERPORT_PRINTER_OUTPUT_TYPE_PL, "Typ wyj¶cia drukarki w userporcie" },
/* ru */ { IDS_USERPORT_PRINTER_OUTPUT_TYPE_RU, "Userport printer output type" },
/* sv */ { IDS_USERPORT_PRINTER_OUTPUT_TYPE_SV, "Utdatatyp för användarportskrivare" },
/* tr */ { IDS_USERPORT_PRINTER_OUTPUT_TYPE_TR, "Userport yazýcýsý çýktý tipi" },

/* en */ { IDS_PRINTER_4_OUTPUT_DEVICE,    "Printer #4 output device" },
/* da */ { IDS_PRINTER_4_OUTPUT_DEVICE_DA, "Printer #4 uddataenhed" },
/* de */ { IDS_PRINTER_4_OUTPUT_DEVICE_DE, "Drucker #4 Ausgabegerät" },
/* es */ { IDS_PRINTER_4_OUTPUT_DEVICE_ES, "Periférico salida impresora #4" },
/* fr */ { IDS_PRINTER_4_OUTPUT_DEVICE_FR, "Périphérique de sortie de l'imprimante 4" },
/* hu */ { IDS_PRINTER_4_OUTPUT_DEVICE_HU, "" },  /* fuzzy */
/* it */ { IDS_PRINTER_4_OUTPUT_DEVICE_IT, "Dispositivo di output stampante #4" },
/* ko */ { IDS_PRINTER_4_OUTPUT_DEVICE_KO, "" },  /* fuzzy */
/* nl */ { IDS_PRINTER_4_OUTPUT_DEVICE_NL, "Printer #4 uitvoer apparaat" },
/* pl */ { IDS_PRINTER_4_OUTPUT_DEVICE_PL, "Urz±dzenie wyj¶cia drukarki #4" },
/* ru */ { IDS_PRINTER_4_OUTPUT_DEVICE_RU, "Printer #4 output device" },
/* sv */ { IDS_PRINTER_4_OUTPUT_DEVICE_SV, "Textutenhet för skrivare 4" },
/* tr */ { IDS_PRINTER_4_OUTPUT_DEVICE_TR, "Yazýcý #4 çýktý aygýtý" },

/* en */ { IDS_PRINTER_5_OUTPUT_DEVICE,    "Printer #5 output device" },
/* da */ { IDS_PRINTER_5_OUTPUT_DEVICE_DA, "Printer #5 uddataenhed" },
/* de */ { IDS_PRINTER_5_OUTPUT_DEVICE_DE, "Drucker #5 Ausgabegerät" },
/* es */ { IDS_PRINTER_5_OUTPUT_DEVICE_ES, "Periférico salida impresora #5" },
/* fr */ { IDS_PRINTER_5_OUTPUT_DEVICE_FR, "Périphérique de sortie de l'imprimante 5" },
/* hu */ { IDS_PRINTER_5_OUTPUT_DEVICE_HU, "" },  /* fuzzy */
/* it */ { IDS_PRINTER_5_OUTPUT_DEVICE_IT, "Dispositivo di output stampante #5" },
/* ko */ { IDS_PRINTER_5_OUTPUT_DEVICE_KO, "" },  /* fuzzy */
/* nl */ { IDS_PRINTER_5_OUTPUT_DEVICE_NL, "Printer #5 uitvoer apparaat" },
/* pl */ { IDS_PRINTER_5_OUTPUT_DEVICE_PL, "Urz±dzenie wyj¶cia drukarki #5" },
/* ru */ { IDS_PRINTER_5_OUTPUT_DEVICE_RU, "Printer #5 output device" },
/* sv */ { IDS_PRINTER_5_OUTPUT_DEVICE_SV, "Textutenhet för skrivare 5" },
/* tr */ { IDS_PRINTER_5_OUTPUT_DEVICE_TR, "Yazýcý #5 çýktý aygýtý" },

/* en */ { IDS_PRINTER_6_OUTPUT_DEVICE,    "Printer #6 output device" },
/* da */ { IDS_PRINTER_6_OUTPUT_DEVICE_DA, "" },  /* fuzzy */
/* de */ { IDS_PRINTER_6_OUTPUT_DEVICE_DE, "Drucker #6 Ausgabegerät" },
/* es */ { IDS_PRINTER_6_OUTPUT_DEVICE_ES, "Periférico salida impresora #6" },
/* fr */ { IDS_PRINTER_6_OUTPUT_DEVICE_FR, "" },  /* fuzzy */
/* hu */ { IDS_PRINTER_6_OUTPUT_DEVICE_HU, "" },  /* fuzzy */
/* it */ { IDS_PRINTER_6_OUTPUT_DEVICE_IT, "Dispositivo di output stampante #6" },
/* ko */ { IDS_PRINTER_6_OUTPUT_DEVICE_KO, "" },  /* fuzzy */
/* nl */ { IDS_PRINTER_6_OUTPUT_DEVICE_NL, "Printer #6 uitvoer apparaat" },
/* pl */ { IDS_PRINTER_6_OUTPUT_DEVICE_PL, "Urz±dzenie wyj¶cia drukarki #6" },
/* ru */ { IDS_PRINTER_6_OUTPUT_DEVICE_RU, "" },  /* fuzzy */
/* sv */ { IDS_PRINTER_6_OUTPUT_DEVICE_SV, "" },  /* fuzzy */
/* tr */ { IDS_PRINTER_6_OUTPUT_DEVICE_TR, "" },  /* fuzzy */

/* en */ { IDS_USERPORT_PRINTER_OUTPUT_DEVICE,    "Userport printer output device" },
/* da */ { IDS_USERPORT_PRINTER_OUTPUT_DEVICE_DA, "Brugerport printer uddataenhed" },
/* de */ { IDS_USERPORT_PRINTER_OUTPUT_DEVICE_DE, "Userport Drucker Ausgabegerät" },
/* es */ { IDS_USERPORT_PRINTER_OUTPUT_DEVICE_ES, "Periférico salida puerto impresora" },
/* fr */ { IDS_USERPORT_PRINTER_OUTPUT_DEVICE_FR, "Périphérique de sortie de l'imprimante port utilisateur" },
/* hu */ { IDS_USERPORT_PRINTER_OUTPUT_DEVICE_HU, "" },  /* fuzzy */
/* it */ { IDS_USERPORT_PRINTER_OUTPUT_DEVICE_IT, "Dispositivo di output stampante su userport" },
/* ko */ { IDS_USERPORT_PRINTER_OUTPUT_DEVICE_KO, "" },  /* fuzzy */
/* nl */ { IDS_USERPORT_PRINTER_OUTPUT_DEVICE_NL, "Userport printer uitvoer apparaat" },
/* pl */ { IDS_USERPORT_PRINTER_OUTPUT_DEVICE_PL, "Urz±dzenie wyj¶cia drukarki w userporcie" },
/* ru */ { IDS_USERPORT_PRINTER_OUTPUT_DEVICE_RU, "Userport printer output device" },
/* sv */ { IDS_USERPORT_PRINTER_OUTPUT_DEVICE_SV, "Textutenhet för användarportskrivare" },
/* tr */ { IDS_USERPORT_PRINTER_OUTPUT_DEVICE_TR, "Userport yazýcýsý çýktý aygýtý" },

/* en */ { IDS_PRINTER_4_SEND_FORMFEED,    "Send formfeed to printer #4" },
/* da */ { IDS_PRINTER_4_SEND_FORMFEED_DA, "Send sideskift til printer #4" },
/* de */ { IDS_PRINTER_4_SEND_FORMFEED_DE, "Seitenvorschub zum Drucker #4 senden" },
/* es */ { IDS_PRINTER_4_SEND_FORMFEED_ES, "Enviar alimentación formulario a impresora #4" },
/* fr */ { IDS_PRINTER_4_SEND_FORMFEED_FR, "Envoyer saut-de-page à l'imprimante 4" },
/* hu */ { IDS_PRINTER_4_SEND_FORMFEED_HU, "" },  /* fuzzy */
/* it */ { IDS_PRINTER_4_SEND_FORMFEED_IT, "Invia formfeed alla stampante #4" },
/* ko */ { IDS_PRINTER_4_SEND_FORMFEED_KO, "" },  /* fuzzy */
/* nl */ { IDS_PRINTER_4_SEND_FORMFEED_NL, "Zend paginadoorvoer naar printer #4" },
/* pl */ { IDS_PRINTER_4_SEND_FORMFEED_PL, "Wy¶lij Form Feed do drukarki #4" },
/* ru */ { IDS_PRINTER_4_SEND_FORMFEED_RU, "Send formfeed to printer #4" },
/* sv */ { IDS_PRINTER_4_SEND_FORMFEED_SV, "Sänd sidmatning till skrivare 4" },
/* tr */ { IDS_PRINTER_4_SEND_FORMFEED_TR, "Yazýcý #4'e sayfa ilerletme gönder" },

/* en */ { IDS_PRINTER_5_SEND_FORMFEED,    "Send formfeed to printer #5" },
/* da */ { IDS_PRINTER_5_SEND_FORMFEED_DA, "Send sideskift til printer #5" },
/* de */ { IDS_PRINTER_5_SEND_FORMFEED_DE, "Seitenvorschub zum Drucker #5 senden" },
/* es */ { IDS_PRINTER_5_SEND_FORMFEED_ES, "Enviar alimentación formulario a impresora #5" },
/* fr */ { IDS_PRINTER_5_SEND_FORMFEED_FR, "Envoyer saut-de-page à l'imprimante 5" },
/* hu */ { IDS_PRINTER_5_SEND_FORMFEED_HU, "" },  /* fuzzy */
/* it */ { IDS_PRINTER_5_SEND_FORMFEED_IT, "Invia formfeed alla stampante #5" },
/* ko */ { IDS_PRINTER_5_SEND_FORMFEED_KO, "" },  /* fuzzy */
/* nl */ { IDS_PRINTER_5_SEND_FORMFEED_NL, "Zend paginadoorvoer naar printer #5" },
/* pl */ { IDS_PRINTER_5_SEND_FORMFEED_PL, "Wy¶lij Form Feed do drukarki #5" },
/* ru */ { IDS_PRINTER_5_SEND_FORMFEED_RU, "Send formfeed to printer #5" },
/* sv */ { IDS_PRINTER_5_SEND_FORMFEED_SV, "Sänd sidmatning till skrivare 5" },
/* tr */ { IDS_PRINTER_5_SEND_FORMFEED_TR, "Yazýcý #5'e sayfa ilerletme gönder" },

/* en */ { IDS_PRINTER_6_SEND_FORMFEED,    "Send formfeed to printer #6" },
/* da */ { IDS_PRINTER_6_SEND_FORMFEED_DA, "" },  /* fuzzy */
/* de */ { IDS_PRINTER_6_SEND_FORMFEED_DE, "Seitenvorschub zum Drucker #6 senden" },
/* es */ { IDS_PRINTER_6_SEND_FORMFEED_ES, "Enviar alimentación formulario a impresora #6" },
/* fr */ { IDS_PRINTER_6_SEND_FORMFEED_FR, "" },  /* fuzzy */
/* hu */ { IDS_PRINTER_6_SEND_FORMFEED_HU, "" },  /* fuzzy */
/* it */ { IDS_PRINTER_6_SEND_FORMFEED_IT, "Invia formfeed alla stampante #6" },
/* ko */ { IDS_PRINTER_6_SEND_FORMFEED_KO, "" },  /* fuzzy */
/* nl */ { IDS_PRINTER_6_SEND_FORMFEED_NL, "Zend paginadoorvoer naar printer #6" },
/* pl */ { IDS_PRINTER_6_SEND_FORMFEED_PL, "Wy¶lij Form Feed do drukarki #6" },
/* ru */ { IDS_PRINTER_6_SEND_FORMFEED_RU, "" },  /* fuzzy */
/* sv */ { IDS_PRINTER_6_SEND_FORMFEED_SV, "" },  /* fuzzy */
/* tr */ { IDS_PRINTER_6_SEND_FORMFEED_TR, "" },  /* fuzzy */

/* en */ { IDS_USERPORT_PRINTER_SEND_FORMFEED,    "Send formfeed to userport printer" },
/* da */ { IDS_USERPORT_PRINTER_SEND_FORMFEED_DA, "Send sideskift til brugerports-printer" },
/* de */ { IDS_USERPORT_PRINTER_SEND_FORMFEED_DE, "Seitenvorschub zum Userport-Drucker senden" },
/* es */ { IDS_USERPORT_PRINTER_SEND_FORMFEED_ES, "Enviar alimentación formulario a puerto impresora" },
/* fr */ { IDS_USERPORT_PRINTER_SEND_FORMFEED_FR, "Envoyer saut-de-page à l'imprimante sur port utilisateur" },
/* hu */ { IDS_USERPORT_PRINTER_SEND_FORMFEED_HU, "" },  /* fuzzy */
/* it */ { IDS_USERPORT_PRINTER_SEND_FORMFEED_IT, "Invia formfeed alla stampante su userport" },
/* ko */ { IDS_USERPORT_PRINTER_SEND_FORMFEED_KO, "" },  /* fuzzy */
/* nl */ { IDS_USERPORT_PRINTER_SEND_FORMFEED_NL, "Zend paginadoorvoer naar userport printer" },
/* pl */ { IDS_USERPORT_PRINTER_SEND_FORMFEED_PL, "Wy¶lij Form Feed do drukarki w userporcie" },
/* ru */ { IDS_USERPORT_PRINTER_SEND_FORMFEED_RU, "Send formfeed to userport printer" },
/* sv */ { IDS_USERPORT_PRINTER_SEND_FORMFEED_SV, "Sänd sidmatning till användarportskrivare" },
/* tr */ { IDS_USERPORT_PRINTER_SEND_FORMFEED_TR, "Userport yazýcýsý'na sayfa ilerletme gönder" },

/* en */ { IDS_PRINTER_DEVICE_1_FILENAME,    "Printer device #1 output" },
/* da */ { IDS_PRINTER_DEVICE_1_FILENAME_DA, "Printer enhed #1 uddata" },
/* de */ { IDS_PRINTER_DEVICE_1_FILENAME_DE, "Drucker Gerät #1 Ausgabe" },
/* es */ { IDS_PRINTER_DEVICE_1_FILENAME_ES, "Periférico de salida impresora #1" },
/* fr */ { IDS_PRINTER_DEVICE_1_FILENAME_FR, "Sortie du périphérique d'imprimante texte 1" },
/* hu */ { IDS_PRINTER_DEVICE_1_FILENAME_HU, "" },  /* fuzzy */
/* it */ { IDS_PRINTER_DEVICE_1_FILENAME_IT, "Output stampante #1..." },
/* ko */ { IDS_PRINTER_DEVICE_1_FILENAME_KO, "" },  /* fuzzy */
/* nl */ { IDS_PRINTER_DEVICE_1_FILENAME_NL, "Printer apparaat #1 uitvoer" },
/* pl */ { IDS_PRINTER_DEVICE_1_FILENAME_PL, "Wyj¶cie urz±dzenia drukarki #1" },
/* ru */ { IDS_PRINTER_DEVICE_1_FILENAME_RU, "Printer device #1 output" },
/* sv */ { IDS_PRINTER_DEVICE_1_FILENAME_SV, "Utdata för skrivarenhet 1" },
/* tr */ { IDS_PRINTER_DEVICE_1_FILENAME_TR, "Yazýcý aygýtý #1 çýktýsý" },

/* en */ { IDS_PRINTER_DEVICE_2_FILENAME,    "Printer device #2 output" },
/* da */ { IDS_PRINTER_DEVICE_2_FILENAME_DA, "Printer enhed #2 uddata" },
/* de */ { IDS_PRINTER_DEVICE_2_FILENAME_DE, "Drucker Gerät #2 Ausgabe" },
/* es */ { IDS_PRINTER_DEVICE_2_FILENAME_ES, "Periférico de salida impresora #2" },
/* fr */ { IDS_PRINTER_DEVICE_2_FILENAME_FR, "Sortie du périphérique d'imprimante texte 2" },
/* hu */ { IDS_PRINTER_DEVICE_2_FILENAME_HU, "" },  /* fuzzy */
/* it */ { IDS_PRINTER_DEVICE_2_FILENAME_IT, "Output stampante #2..." },
/* ko */ { IDS_PRINTER_DEVICE_2_FILENAME_KO, "" },  /* fuzzy */
/* nl */ { IDS_PRINTER_DEVICE_2_FILENAME_NL, "Printer apparaat #2 uitvoer" },
/* pl */ { IDS_PRINTER_DEVICE_2_FILENAME_PL, "Wyj¶cie urz±dzenia drukarki #2" },
/* ru */ { IDS_PRINTER_DEVICE_2_FILENAME_RU, "Printer device #2 output" },
/* sv */ { IDS_PRINTER_DEVICE_2_FILENAME_SV, "Utdata för skrivarenhet 2" },
/* tr */ { IDS_PRINTER_DEVICE_2_FILENAME_TR, "Yazýcý aygýtý #2 çýktýsý" },

/* en */ { IDS_PRINTER_DEVICE_3_FILENAME,    "Printer device #3 output" },
/* da */ { IDS_PRINTER_DEVICE_3_FILENAME_DA, "Printer enhed #3 uddata" },
/* de */ { IDS_PRINTER_DEVICE_3_FILENAME_DE, "Drucker Gerät #3 Ausgabe" },
/* es */ { IDS_PRINTER_DEVICE_3_FILENAME_ES, "Periférico de salida impresora #3" },
/* fr */ { IDS_PRINTER_DEVICE_3_FILENAME_FR, "Sortie du périphérique d'imprimante texte 3" },
/* hu */ { IDS_PRINTER_DEVICE_3_FILENAME_HU, "" },  /* fuzzy */
/* it */ { IDS_PRINTER_DEVICE_3_FILENAME_IT, "Output stampante #3..." },
/* ko */ { IDS_PRINTER_DEVICE_3_FILENAME_KO, "" },  /* fuzzy */
/* nl */ { IDS_PRINTER_DEVICE_3_FILENAME_NL, "Printer apparaat #3 uitvoer" },
/* pl */ { IDS_PRINTER_DEVICE_3_FILENAME_PL, "Wyj¶cie urz±dzenia drukarki #3" },
/* ru */ { IDS_PRINTER_DEVICE_3_FILENAME_RU, "Printer device #3 output" },
/* sv */ { IDS_PRINTER_DEVICE_3_FILENAME_SV, "Utdata för skrivarenhet 3" },
/* tr */ { IDS_PRINTER_DEVICE_3_FILENAME_TR, "Yazýcý aygýtý #3 çýktýsý" },

/* en */ { IDS_ENABLE_IEC_PRINTER_4,    "IEC emulation for printer #4" },
/* da */ { IDS_ENABLE_IEC_PRINTER_4_DA, "IEC-emulering for printer #4" },
/* de */ { IDS_ENABLE_IEC_PRINTER_4_DE, "IEC Geräte Emulation für Drucker #4" },
/* es */ { IDS_ENABLE_IEC_PRINTER_4_ES, "Emulación IEC para impresora #4" },
/* fr */ { IDS_ENABLE_IEC_PRINTER_4_FR, "Activer l'émulation IEC pour l'imprimante 4" },
/* hu */ { IDS_ENABLE_IEC_PRINTER_4_HU, "" },  /* fuzzy */
/* it */ { IDS_ENABLE_IEC_PRINTER_4_IT, "Emulazione IEC per la stampante #4" },
/* ko */ { IDS_ENABLE_IEC_PRINTER_4_KO, "" },  /* fuzzy */
/* nl */ { IDS_ENABLE_IEC_PRINTER_4_NL, "IEC emulatie voor printer #4" },
/* pl */ { IDS_ENABLE_IEC_PRINTER_4_PL, "Emulacja IEC drukarki #4" },
/* ru */ { IDS_ENABLE_IEC_PRINTER_4_RU, "IEC emulation for printer #4" },
/* sv */ { IDS_ENABLE_IEC_PRINTER_4_SV, "IEC-emulering för skrivare 4" },
/* tr */ { IDS_ENABLE_IEC_PRINTER_4_TR, "Yazýcý #4 için IEC emülasyonu" },

/* en */ { IDS_ENABLE_IEC_PRINTER_5,    "IEC emulation for printer #5" },
/* da */ { IDS_ENABLE_IEC_PRINTER_5_DA, "IEC-emulering for printer #5" },
/* de */ { IDS_ENABLE_IEC_PRINTER_5_DE, "IEC Geräte Emulation für Drucker #5" },
/* es */ { IDS_ENABLE_IEC_PRINTER_5_ES, "Emulación IEC para impresora #5" },
/* fr */ { IDS_ENABLE_IEC_PRINTER_5_FR, "Activer l'émulation IEC pour l'imprimante 5" },
/* hu */ { IDS_ENABLE_IEC_PRINTER_5_HU, "" },  /* fuzzy */
/* it */ { IDS_ENABLE_IEC_PRINTER_5_IT, "Emulazione IEC per la stampante #5" },
/* ko */ { IDS_ENABLE_IEC_PRINTER_5_KO, "" },  /* fuzzy */
/* nl */ { IDS_ENABLE_IEC_PRINTER_5_NL, "IEC emulatie voor printer #5" },
/* pl */ { IDS_ENABLE_IEC_PRINTER_5_PL, "Emulacja IEC drukarki #5" },
/* ru */ { IDS_ENABLE_IEC_PRINTER_5_RU, "IEC emulation for printer #5" },
/* sv */ { IDS_ENABLE_IEC_PRINTER_5_SV, "IEC-emulering för skrivare 5" },
/* tr */ { IDS_ENABLE_IEC_PRINTER_5_TR, "Yazýcý #5 için IEC emülasyonu" },

/* en */ { IDS_ENABLE_IEC_PRINTER_6,    "IEC emulation for printer #6" },
/* da */ { IDS_ENABLE_IEC_PRINTER_6_DA, "" },  /* fuzzy */
/* de */ { IDS_ENABLE_IEC_PRINTER_6_DE, "IEC Geräte Emulation für Drucker #6" },
/* es */ { IDS_ENABLE_IEC_PRINTER_6_ES, "Emulación IEC para impresora #6" },
/* fr */ { IDS_ENABLE_IEC_PRINTER_6_FR, "" },  /* fuzzy */
/* hu */ { IDS_ENABLE_IEC_PRINTER_6_HU, "" },  /* fuzzy */
/* it */ { IDS_ENABLE_IEC_PRINTER_6_IT, "Emulazione IEC per la stampante #6" },
/* ko */ { IDS_ENABLE_IEC_PRINTER_6_KO, "" },  /* fuzzy */
/* nl */ { IDS_ENABLE_IEC_PRINTER_6_NL, "IEC emulatie voor printer #6" },
/* pl */ { IDS_ENABLE_IEC_PRINTER_6_PL, "Emulacja IEC drukarki #6" },
/* ru */ { IDS_ENABLE_IEC_PRINTER_6_RU, "" },  /* fuzzy */
/* sv */ { IDS_ENABLE_IEC_PRINTER_6_SV, "" },  /* fuzzy */
/* tr */ { IDS_ENABLE_IEC_PRINTER_6_TR, "" },  /* fuzzy */

/* en */ { IDS_ENABLE_IEC_DEVICE_7,    "IEC emulation for device #7" },
/* da */ { IDS_ENABLE_IEC_DEVICE_7_DA, "" },  /* fuzzy */
/* de */ { IDS_ENABLE_IEC_DEVICE_7_DE, "IEC Emulation für Gerät #7" },
/* es */ { IDS_ENABLE_IEC_DEVICE_7_ES, "Emulación IEC para impresora #7" },
/* fr */ { IDS_ENABLE_IEC_DEVICE_7_FR, "" },  /* fuzzy */
/* hu */ { IDS_ENABLE_IEC_DEVICE_7_HU, "" },  /* fuzzy */
/* it */ { IDS_ENABLE_IEC_DEVICE_7_IT, "Emulazione IEC per la stampante #7" },
/* ko */ { IDS_ENABLE_IEC_DEVICE_7_KO, "" },  /* fuzzy */
/* nl */ { IDS_ENABLE_IEC_DEVICE_7_NL, "IEC emulatie voor printer #7" },
/* pl */ { IDS_ENABLE_IEC_DEVICE_7_PL, "Emulacja IEC urz±dzenia #7" },
/* ru */ { IDS_ENABLE_IEC_DEVICE_7_RU, "" },  /* fuzzy */
/* sv */ { IDS_ENABLE_IEC_DEVICE_7_SV, "" },  /* fuzzy */
/* tr */ { IDS_ENABLE_IEC_DEVICE_7_TR, "" },  /* fuzzy */

/* en */ { IDS_PRINTER_SETTINGS,    "Printer settings" },
/* da */ { IDS_PRINTER_SETTINGS_DA, "Printerindstillinger" },
/* de */ { IDS_PRINTER_SETTINGS_DE, "Drucker Einstellungen" },
/* es */ { IDS_PRINTER_SETTINGS_ES, "Ajustes de impresora" },
/* fr */ { IDS_PRINTER_SETTINGS_FR, "Paramètres d'imprimante" },
/* hu */ { IDS_PRINTER_SETTINGS_HU, "Nyomtató beállításai" },
/* it */ { IDS_PRINTER_SETTINGS_IT, "Impostazioni stampanti" },
/* ko */ { IDS_PRINTER_SETTINGS_KO, "ÇÁ¸°ÅÍ ¼ÂÆÃ" },
/* nl */ { IDS_PRINTER_SETTINGS_NL, "Printer instellingen" },
/* pl */ { IDS_PRINTER_SETTINGS_PL, "Ustawienia drukarki" },
/* ru */ { IDS_PRINTER_SETTINGS_RU, "½ÐáâàÞÙÚØ ßàØÝâÕàÐ" },
/* sv */ { IDS_PRINTER_SETTINGS_SV, "Skrivarinställningar" },
/* tr */ { IDS_PRINTER_SETTINGS_TR, "Yazýcý ayarlarý" },

/* en */ { IDS_VICII_MODEL_SELECTION,    "VICII model selection" },
/* da */ { IDS_VICII_MODEL_SELECTION_DA, "" },  /* fuzzy */
/* de */ { IDS_VICII_MODEL_SELECTION_DE, "VIC-II Modell Auswahl" },
/* es */ { IDS_VICII_MODEL_SELECTION_ES, "" },  /* fuzzy */
/* fr */ { IDS_VICII_MODEL_SELECTION_FR, "" },  /* fuzzy */
/* hu */ { IDS_VICII_MODEL_SELECTION_HU, "" },  /* fuzzy */
/* it */ { IDS_VICII_MODEL_SELECTION_IT, "" },  /* fuzzy */
/* ko */ { IDS_VICII_MODEL_SELECTION_KO, "" },  /* fuzzy */
/* nl */ { IDS_VICII_MODEL_SELECTION_NL, "VICII model instellingen" },
/* pl */ { IDS_VICII_MODEL_SELECTION_PL, "" },  /* fuzzy */
/* ru */ { IDS_VICII_MODEL_SELECTION_RU, "" },  /* fuzzy */
/* sv */ { IDS_VICII_MODEL_SELECTION_SV, "" },  /* fuzzy */
/* tr */ { IDS_VICII_MODEL_SELECTION_TR, "" },  /* fuzzy */

/* en */ { IDS_MAGIC_VOICE_FILENAME,    "Magic Voice file" },
/* da */ { IDS_MAGIC_VOICE_FILENAME_DA, "Magic Voice-fil" },
/* de */ { IDS_MAGIC_VOICE_FILENAME_DE, "Magic Voice Datei" },
/* es */ { IDS_MAGIC_VOICE_FILENAME_ES, "Fichero Magic Voice" },
/* fr */ { IDS_MAGIC_VOICE_FILENAME_FR, "Fichier Magic Voice" },
/* hu */ { IDS_MAGIC_VOICE_FILENAME_HU, "" },  /* fuzzy */
/* it */ { IDS_MAGIC_VOICE_FILENAME_IT, "File Magic Voice" },
/* ko */ { IDS_MAGIC_VOICE_FILENAME_KO, "" },  /* fuzzy */
/* nl */ { IDS_MAGIC_VOICE_FILENAME_NL, "Magic Voice bestand" },
/* pl */ { IDS_MAGIC_VOICE_FILENAME_PL, "Plik Magic Voice" },
/* ru */ { IDS_MAGIC_VOICE_FILENAME_RU, "" },  /* fuzzy */
/* sv */ { IDS_MAGIC_VOICE_FILENAME_SV, "Magic Voice-fil" },
/* tr */ { IDS_MAGIC_VOICE_FILENAME_TR, "Magic Voice dosyasý" },

/* en */ { IDS_MAGIC_VOICE_FILENAME_SELECT,    "Select file for Magic Voice" },
/* da */ { IDS_MAGIC_VOICE_FILENAME_SELECT_DA, "Vælg fil for Magic Voice" },
/* de */ { IDS_MAGIC_VOICE_FILENAME_SELECT_DE, "Datei für Magic Voice auswählen" },
/* es */ { IDS_MAGIC_VOICE_FILENAME_SELECT_ES, "Seleccionar fichero para Magic Voice" },
/* fr */ { IDS_MAGIC_VOICE_FILENAME_SELECT_FR, "Sélectionner fichier pour Magic Voice" },
/* hu */ { IDS_MAGIC_VOICE_FILENAME_SELECT_HU, "" },  /* fuzzy */
/* it */ { IDS_MAGIC_VOICE_FILENAME_SELECT_IT, "Seleziona file per Magic Voice" },
/* ko */ { IDS_MAGIC_VOICE_FILENAME_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDS_MAGIC_VOICE_FILENAME_SELECT_NL, "Selecteer bestand voor Magic Voice" },
/* pl */ { IDS_MAGIC_VOICE_FILENAME_SELECT_PL, "Wybierz filtr Magic Voice" },
/* ru */ { IDS_MAGIC_VOICE_FILENAME_SELECT_RU, "Select file for Magic Voice" },
/* sv */ { IDS_MAGIC_VOICE_FILENAME_SELECT_SV, "Välj fil för Magic Voice" },
/* tr */ { IDS_MAGIC_VOICE_FILENAME_SELECT_TR, "Magic Voice için dosya seçin" },

/* en */ { IDS_MAGIC_VOICE_SETTINGS,    "Magic Voice settings" },
/* da */ { IDS_MAGIC_VOICE_SETTINGS_DA, "Magic Voice indstillinger" },
/* de */ { IDS_MAGIC_VOICE_SETTINGS_DE, "Magic Voice Einstellungen" },
/* es */ { IDS_MAGIC_VOICE_SETTINGS_ES, "Ajustes Magic Voice" },
/* fr */ { IDS_MAGIC_VOICE_SETTINGS_FR, "Paramètres Magic Voice" },
/* hu */ { IDS_MAGIC_VOICE_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_MAGIC_VOICE_SETTINGS_IT, "Impostazioni Magic Voice" },
/* ko */ { IDS_MAGIC_VOICE_SETTINGS_KO, "Magic Voice ¼³Á¤" },
/* nl */ { IDS_MAGIC_VOICE_SETTINGS_NL, "Magic Voice instellingen" },
/* pl */ { IDS_MAGIC_VOICE_SETTINGS_PL, "Ustawienia Magic Voice" },
/* ru */ { IDS_MAGIC_VOICE_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_MAGIC_VOICE_SETTINGS_SV, "Magic Voice-inställningar" },
/* tr */ { IDS_MAGIC_VOICE_SETTINGS_TR, "Magic Voice ayarlarý" },

/* en */ { IDS_USERPORT_RS232,    "Userport RS232" },
/* da */ { IDS_USERPORT_RS232_DA, "Brugerport RS232" },
/* de */ { IDS_USERPORT_RS232_DE, "Userport RS232" },
/* es */ { IDS_USERPORT_RS232_ES, "Puerto RS232" },
/* fr */ { IDS_USERPORT_RS232_FR, "RS232 port utilisateur" },
/* hu */ { IDS_USERPORT_RS232_HU, "Userport RS232" },
/* it */ { IDS_USERPORT_RS232_IT, "Userport RS232" },
/* ko */ { IDS_USERPORT_RS232_KO, "" },  /* fuzzy */
/* nl */ { IDS_USERPORT_RS232_NL, "Userport RS232" },
/* pl */ { IDS_USERPORT_RS232_PL, "Userport RS232" },
/* ru */ { IDS_USERPORT_RS232_RU, "Userport RS232" },
/* sv */ { IDS_USERPORT_RS232_SV, "Användarport-RS232" },
/* tr */ { IDS_USERPORT_RS232_TR, "Userport RS232" },

/* en */ { IDS_USERPORT_DEVICE,    "Userport RS232 device" },
/* da */ { IDS_USERPORT_DEVICE_DA, "Brugerportens RS232-enhed" },
/* de */ { IDS_USERPORT_DEVICE_DE, "Userport RS232 Gerät" },
/* es */ { IDS_USERPORT_DEVICE_ES, "Periférico puerto RS232" },
/* fr */ { IDS_USERPORT_DEVICE_FR, "Périphérique de port utilisateur RS232" },
/* hu */ { IDS_USERPORT_DEVICE_HU, "Userport RS232 eszköz" },
/* it */ { IDS_USERPORT_DEVICE_IT, "Dispositivo RS232 su userport" },
/* ko */ { IDS_USERPORT_DEVICE_KO, "" },  /* fuzzy */
/* nl */ { IDS_USERPORT_DEVICE_NL, "Userport RS232 apparaat" },
/* pl */ { IDS_USERPORT_DEVICE_PL, "Urz±dzenie userportu RS232" },
/* ru */ { IDS_USERPORT_DEVICE_RU, "Userport RS232 device" },
/* sv */ { IDS_USERPORT_DEVICE_SV, "Användarportens RS232-enhet" },
/* tr */ { IDS_USERPORT_DEVICE_TR, "Userport RS232 aygýtý" },

/* en */ { IDS_USERPORT_BAUD_RATE,    "Userport RS232 baud rate" },
/* da */ { IDS_USERPORT_BAUD_RATE_DA, "Brugerportens RS232-baudhastighet" },
/* de */ { IDS_USERPORT_BAUD_RATE_DE, "Userport RS232 Baud Rate" },
/* es */ { IDS_USERPORT_BAUD_RATE_ES, "Velocidad (baudios) puerto RS232" },
/* fr */ { IDS_USERPORT_BAUD_RATE_FR, "Périphérique de port utilisateur RS232" },
/* hu */ { IDS_USERPORT_BAUD_RATE_HU, "Userport RS232 átviteli ráta" },
/* it */ { IDS_USERPORT_BAUD_RATE_IT, "Velocità RS232 su userport" },
/* ko */ { IDS_USERPORT_BAUD_RATE_KO, "" },  /* fuzzy */
/* nl */ { IDS_USERPORT_BAUD_RATE_NL, "Userport RS232 baudrate" },
/* pl */ { IDS_USERPORT_BAUD_RATE_PL, "Szybko¶æ transmisji userportu RS232" },
/* ru */ { IDS_USERPORT_BAUD_RATE_RU, "Userport RS232 baud rate" },
/* sv */ { IDS_USERPORT_BAUD_RATE_SV, "Användarportens RS232-baudhastighet" },
/* tr */ { IDS_USERPORT_BAUD_RATE_TR, "Userport RS232 baud rate" },

/* en */ { IDS_RS232_USERPORT_SETTINGS,    "RS232 userport settings" },
/* da */ { IDS_RS232_USERPORT_SETTINGS_DA, "RS232-brugerportindstillinger" },
/* de */ { IDS_RS232_USERPORT_SETTINGS_DE, "RS232 Userport Einstellungen" },
/* es */ { IDS_RS232_USERPORT_SETTINGS_ES, "Ajustes puerto RS232" },
/* fr */ { IDS_RS232_USERPORT_SETTINGS_FR, "Paramètres port utilisateur RS232" },
/* hu */ { IDS_RS232_USERPORT_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_RS232_USERPORT_SETTINGS_IT, "Impostazioni RS232 su userport" },
/* ko */ { IDS_RS232_USERPORT_SETTINGS_KO, "RS232 À¯Á®Æ÷Æ® ¼³Á¤" },
/* nl */ { IDS_RS232_USERPORT_SETTINGS_NL, "RS232 userport instellingen" },
/* pl */ { IDS_RS232_USERPORT_SETTINGS_PL, "Ustawienia userportu RS232" },
/* ru */ { IDS_RS232_USERPORT_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_RS232_USERPORT_SETTINGS_SV, "RS232-användarportinställningar" },
/* tr */ { IDS_RS232_USERPORT_SETTINGS_TR, "RS232 userport ayarlarý" },

/* en */ { IDS_FAST,    "Fast" },
/* da */ { IDS_FAST_DA, "Hurtig" },
/* de */ { IDS_FAST_DE, "Schnell" },
/* es */ { IDS_FAST_ES, "Rápido" },
/* fr */ { IDS_FAST_FR, "Rapide" },
/* hu */ { IDS_FAST_HU, "" },  /* fuzzy */
/* it */ { IDS_FAST_IT, "Veloce" },
/* ko */ { IDS_FAST_KO, "ºü¸¥" },
/* nl */ { IDS_FAST_NL, "Snel" },
/* pl */ { IDS_FAST_PL, "Szybko" },
/* ru */ { IDS_FAST_RU, "Fast" },
/* sv */ { IDS_FAST_SV, "Snabbt" },
/* tr */ { IDS_FAST_TR, "Hýzlý" },

/* en */ { IDS_INTERPOLATING,    "Interpolating" },
/* da */ { IDS_INTERPOLATING_DA, "Interpolerende" },
/* de */ { IDS_INTERPOLATING_DE, "Interpolierend" },
/* es */ { IDS_INTERPOLATING_ES, "Interpolando" },
/* fr */ { IDS_INTERPOLATING_FR, "Par interpolation" },
/* hu */ { IDS_INTERPOLATING_HU, "" },  /* fuzzy */
/* it */ { IDS_INTERPOLATING_IT, "Interpolazione" },
/* ko */ { IDS_INTERPOLATING_KO, "º¸°£ÇÏ´Ù" },
/* nl */ { IDS_INTERPOLATING_NL, "Interpoleren" },
/* pl */ { IDS_INTERPOLATING_PL, "Interpolacja" },
/* ru */ { IDS_INTERPOLATING_RU, "" },  /* fuzzy */
/* sv */ { IDS_INTERPOLATING_SV, "Interpolerande" },
/* tr */ { IDS_INTERPOLATING_TR, "Ara deðer bulma" },

/* en */ { IDS_RESAMPLING,    "Resampling" },
/* da */ { IDS_RESAMPLING_DA, "Resampling" },
/* de */ { IDS_RESAMPLING_DE, "Resampling" },
/* es */ { IDS_RESAMPLING_ES, "Remuestreo" },
/* fr */ { IDS_RESAMPLING_FR, "Rééchantillonnage" },
/* hu */ { IDS_RESAMPLING_HU, "" },  /* fuzzy */
/* it */ { IDS_RESAMPLING_IT, "Ricampionamento" },
/* ko */ { IDS_RESAMPLING_KO, "¸®»ùÇÃ¸µ" },
/* nl */ { IDS_RESAMPLING_NL, "Resampling" },
/* pl */ { IDS_RESAMPLING_PL, "Przesamplowanie" },
/* ru */ { IDS_RESAMPLING_RU, "Resampling" },
/* sv */ { IDS_RESAMPLING_SV, "Omsamplande" },
/* tr */ { IDS_RESAMPLING_TR, "Yeniden örnekleme" },

/* en */ { IDS_FAST_RESAMPLING,    "Fast resampling" },
/* da */ { IDS_FAST_RESAMPLING_DA, "Hurtig omsampling" },
/* de */ { IDS_FAST_RESAMPLING_DE, "Schnelles Resampling" },
/* es */ { IDS_FAST_RESAMPLING_ES, "Remuestreo rápido" },
/* fr */ { IDS_FAST_RESAMPLING_FR, "Rééchantillonage rapide" },
/* hu */ { IDS_FAST_RESAMPLING_HU, "" },  /* fuzzy */
/* it */ { IDS_FAST_RESAMPLING_IT, "Ricampionamento veloce" },
/* ko */ { IDS_FAST_RESAMPLING_KO, "ºü¸¥ ¸®¼Àºí¸µ" },
/* nl */ { IDS_FAST_RESAMPLING_NL, "Snelle resampling" },
/* pl */ { IDS_FAST_RESAMPLING_PL, "Szybkie przesamplowanie" },
/* ru */ { IDS_FAST_RESAMPLING_RU, "Fast resampling" },
/* sv */ { IDS_FAST_RESAMPLING_SV, "Snabb omsampling" },
/* tr */ { IDS_FAST_RESAMPLING_TR, "Hýzlý yeniden örnekleme" },

/* en */ { IDS_SID_STEREO,    "SID Stereo" },
/* da */ { IDS_SID_STEREO_DA, "SID-stereo" },
/* de */ { IDS_SID_STEREO_DE, "SID Stereo" },
/* es */ { IDS_SID_STEREO_ES, "SID estereo" },
/* fr */ { IDS_SID_STEREO_FR, "SID Stéréo" },
/* hu */ { IDS_SID_STEREO_HU, "SID sztereó" },
/* it */ { IDS_SID_STEREO_IT, "Stereo SID" },
/* ko */ { IDS_SID_STEREO_KO, "" },  /* fuzzy */
/* nl */ { IDS_SID_STEREO_NL, "Stereo SID" },
/* pl */ { IDS_SID_STEREO_PL, "Stereo SID" },
/* ru */ { IDS_SID_STEREO_RU, "SID Stereo" },
/* sv */ { IDS_SID_STEREO_SV, "SID-stereo" },
/* tr */ { IDS_SID_STEREO_TR, "Stereo SID" },

/* en */ { IDS_AMOUNT_OF_EXTRA_SIDS,    "Amount of extra sids" },
/* da */ { IDS_AMOUNT_OF_EXTRA_SIDS_DA, "Antal ekstra SID-kredse" },
/* de */ { IDS_AMOUNT_OF_EXTRA_SIDS_DE, "Anzahl der extra SID Chips" },
/* es */ { IDS_AMOUNT_OF_EXTRA_SIDS_ES, "Cantidad de chips SID extra" },
/* fr */ { IDS_AMOUNT_OF_EXTRA_SIDS_FR, "Quantité de SIDs supplémantaires" },
/* hu */ { IDS_AMOUNT_OF_EXTRA_SIDS_HU, "" },  /* fuzzy */
/* it */ { IDS_AMOUNT_OF_EXTRA_SIDS_IT, "Numero di sid aggiuntivi" },
/* ko */ { IDS_AMOUNT_OF_EXTRA_SIDS_KO, "" },  /* fuzzy */
/* nl */ { IDS_AMOUNT_OF_EXTRA_SIDS_NL, "Aantal extra SID chips" },
/* pl */ { IDS_AMOUNT_OF_EXTRA_SIDS_PL, "Liczba dodatkowych SID-ów" },
/* ru */ { IDS_AMOUNT_OF_EXTRA_SIDS_RU, "" },  /* fuzzy */
/* sv */ { IDS_AMOUNT_OF_EXTRA_SIDS_SV, "Antal extra SID-kretsar" },
/* tr */ { IDS_AMOUNT_OF_EXTRA_SIDS_TR, "Ekstra sid adedi" },

/* en */ { IDS_STEREO_SID_AT,    "Stereo SID at" },
/* da */ { IDS_STEREO_SID_AT_DA, "Stereo-SID på" },
/* de */ { IDS_STEREO_SID_AT_DE, "Stereo SID bei" },
/* es */ { IDS_STEREO_SID_AT_ES, "SID estereo en" },
/* fr */ { IDS_STEREO_SID_AT_FR, "SID Stéréo à" },
/* hu */ { IDS_STEREO_SID_AT_HU, "Sztereó SID itt:" },
/* it */ { IDS_STEREO_SID_AT_IT, "Stereo SID a" },
/* ko */ { IDS_STEREO_SID_AT_KO, "" },  /* fuzzy */
/* nl */ { IDS_STEREO_SID_AT_NL, "Stereo SID op" },
/* pl */ { IDS_STEREO_SID_AT_PL, "SID stereo w" },
/* ru */ { IDS_STEREO_SID_AT_RU, "Stereo SID at" },
/* sv */ { IDS_STEREO_SID_AT_SV, "Stereo-SID på" },
/* tr */ { IDS_STEREO_SID_AT_TR, "Stereo SID konumu" },

/* en */ { IDS_TRIPLE_SID_AT,    "Triple SID at" },
/* da */ { IDS_TRIPLE_SID_AT_DA, "Tredie SID på" },
/* de */ { IDS_TRIPLE_SID_AT_DE, "Dritter SID auf" },
/* es */ { IDS_TRIPLE_SID_AT_ES, "Triple SID en" },
/* fr */ { IDS_TRIPLE_SID_AT_FR, "Triple SID à" },
/* hu */ { IDS_TRIPLE_SID_AT_HU, "" },  /* fuzzy */
/* it */ { IDS_TRIPLE_SID_AT_IT, "Triplo SID a" },
/* ko */ { IDS_TRIPLE_SID_AT_KO, "" },  /* fuzzy */
/* nl */ { IDS_TRIPLE_SID_AT_NL, "Derde SID op" },
/* pl */ { IDS_TRIPLE_SID_AT_PL, "Potrójny SID w" },
/* ru */ { IDS_TRIPLE_SID_AT_RU, "" },  /* fuzzy */
/* sv */ { IDS_TRIPLE_SID_AT_SV, "Trippel-SID på" },
/* tr */ { IDS_TRIPLE_SID_AT_TR, "Üçlü SID konumu" },

/* en */ { IDS_QUAD_SID_AT,    "Quad SID at" },
/* da */ { IDS_QUAD_SID_AT_DA, "" },  /* fuzzy */
/* de */ { IDS_QUAD_SID_AT_DE, "Vierter SID auf" },
/* es */ { IDS_QUAD_SID_AT_ES, "" },  /* fuzzy */
/* fr */ { IDS_QUAD_SID_AT_FR, "" },  /* fuzzy */
/* hu */ { IDS_QUAD_SID_AT_HU, "" },  /* fuzzy */
/* it */ { IDS_QUAD_SID_AT_IT, "" },  /* fuzzy */
/* ko */ { IDS_QUAD_SID_AT_KO, "" },  /* fuzzy */
/* nl */ { IDS_QUAD_SID_AT_NL, "" },  /* fuzzy */
/* pl */ { IDS_QUAD_SID_AT_PL, "" },  /* fuzzy */
/* ru */ { IDS_QUAD_SID_AT_RU, "" },  /* fuzzy */
/* sv */ { IDS_QUAD_SID_AT_SV, "" },  /* fuzzy */
/* tr */ { IDS_QUAD_SID_AT_TR, "" },  /* fuzzy */

/* en */ { IDS_SID_FILTERS,    "SID filters" },
/* da */ { IDS_SID_FILTERS_DA, "SID-filtre" },
/* de */ { IDS_SID_FILTERS_DE, "SID Filter" },
/* es */ { IDS_SID_FILTERS_ES, "Filtros SID" },
/* fr */ { IDS_SID_FILTERS_FR, "Filtres SID" },
/* hu */ { IDS_SID_FILTERS_HU, "" },  /* fuzzy */
/* it */ { IDS_SID_FILTERS_IT, "Filtri SID" },
/* ko */ { IDS_SID_FILTERS_KO, "SID ÇÊÅÍ" },
/* nl */ { IDS_SID_FILTERS_NL, "SID filters" },
/* pl */ { IDS_SID_FILTERS_PL, "Filtry SID" },
/* ru */ { IDS_SID_FILTERS_RU, "ÄØÛìâàë SID" },
/* sv */ { IDS_SID_FILTERS_SV, "SID-filter" },
/* tr */ { IDS_SID_FILTERS_TR, "SID filtreleri" },

/* en */ { IDS_SAMPLE_METHOD,    "Sample method" },
/* da */ { IDS_SAMPLE_METHOD_DA, "Samplingmetode" },
/* de */ { IDS_SAMPLE_METHOD_DE, "Sample Methode" },
/* es */ { IDS_SAMPLE_METHOD_ES, "Método de muestreo" },
/* fr */ { IDS_SAMPLE_METHOD_FR, "Méthode d'échantillonnage" },
/* hu */ { IDS_SAMPLE_METHOD_HU, "Mintavételezés módja" },
/* it */ { IDS_SAMPLE_METHOD_IT, "Metodo di campionamento" },
/* ko */ { IDS_SAMPLE_METHOD_KO, "" },  /* fuzzy */
/* nl */ { IDS_SAMPLE_METHOD_NL, "Sample methode" },
/* pl */ { IDS_SAMPLE_METHOD_PL, "Metoda samplowania" },
/* ru */ { IDS_SAMPLE_METHOD_RU, "Sample method" },
/* sv */ { IDS_SAMPLE_METHOD_SV, "Samplingsmetod" },
/* tr */ { IDS_SAMPLE_METHOD_TR, "Örnekleme metodu" },

/* en */ { IDS_PASSBAND_0_90,    "Passband (0..90%)" },
/* da */ { IDS_PASSBAND_0_90_DA, "Pasbånd (0..90%)" },
/* de */ { IDS_PASSBAND_0_90_DE, "Passband (0..90%)" },
/* es */ { IDS_PASSBAND_0_90_ES, "Pasabanda (0..90%)" },
/* fr */ { IDS_PASSBAND_0_90_FR, "Bande passante (0..90%)" },
/* hu */ { IDS_PASSBAND_0_90_HU, "" },  /* fuzzy */
/* it */ { IDS_PASSBAND_0_90_IT, "Banda passante (0..90%)" },
/* ko */ { IDS_PASSBAND_0_90_KO, "" },  /* fuzzy */
/* nl */ { IDS_PASSBAND_0_90_NL, "Passband (0..90%)" },
/* pl */ { IDS_PASSBAND_0_90_PL, "Pasmo przepuszczania (0..90%)" },
/* ru */ { IDS_PASSBAND_0_90_RU, "Passband (0..90%)" },
/* sv */ { IDS_PASSBAND_0_90_SV, "Passband (0..90%)" },
/* tr */ { IDS_PASSBAND_0_90_TR, "Passband (0..90%)" },

/* en */ { IDS_GAIN_90_100,    "Gain (90..100)" },
/* da */ { IDS_GAIN_90_100_DA, "" },  /* fuzzy */
/* de */ { IDS_GAIN_90_100_DE, "Gain (90..100)" },
/* es */ { IDS_GAIN_90_100_ES, "Ganancia (90..100)" },
/* fr */ { IDS_GAIN_90_100_FR, "" },  /* fuzzy */
/* hu */ { IDS_GAIN_90_100_HU, "" },  /* fuzzy */
/* it */ { IDS_GAIN_90_100_IT, "Guadagno (90..100)" },
/* ko */ { IDS_GAIN_90_100_KO, "" },  /* fuzzy */
/* nl */ { IDS_GAIN_90_100_NL, "Toename (90..100)" },
/* pl */ { IDS_GAIN_90_100_PL, "Wzmocnienie (90..100)" },
/* ru */ { IDS_GAIN_90_100_RU, "" },  /* fuzzy */
/* sv */ { IDS_GAIN_90_100_SV, "" },  /* fuzzy */
/* tr */ { IDS_GAIN_90_100_TR, "" },  /* fuzzy */

/* en */ { IDS_GAIN_1_200,    "Gain (1..200)" },
/* da */ { IDS_GAIN_1_200_DA, "" },  /* fuzzy */
/* de */ { IDS_GAIN_1_200_DE, "Gain (1..200)" },
/* es */ { IDS_GAIN_1_200_ES, "Ganancia (1..200)" },
/* fr */ { IDS_GAIN_1_200_FR, "" },  /* fuzzy */
/* hu */ { IDS_GAIN_1_200_HU, "" },  /* fuzzy */
/* it */ { IDS_GAIN_1_200_IT, "Guadagno (1..200)" },
/* ko */ { IDS_GAIN_1_200_KO, "" },  /* fuzzy */
/* nl */ { IDS_GAIN_1_200_NL, "Toename (1..200)" },
/* pl */ { IDS_GAIN_1_200_PL, "Wzmocnienie (1..200)" },
/* ru */ { IDS_GAIN_1_200_RU, "" },  /* fuzzy */
/* sv */ { IDS_GAIN_1_200_SV, "" },  /* fuzzy */
/* tr */ { IDS_GAIN_1_200_TR, "" },  /* fuzzy */

/* en */ { IDS_BIAS_M5000_P5000,    "Filter bias (-5000..5000)" },
/* da */ { IDS_BIAS_M5000_P5000_DA, "" },  /* fuzzy */
/* de */ { IDS_BIAS_M5000_P5000_DE, "Filter Bias (-5000..5000)" },
/* es */ { IDS_BIAS_M5000_P5000_ES, "Filtros bias (-5000..5000)" },
/* fr */ { IDS_BIAS_M5000_P5000_FR, "" },  /* fuzzy */
/* hu */ { IDS_BIAS_M5000_P5000_HU, "" },  /* fuzzy */
/* it */ { IDS_BIAS_M5000_P5000_IT, "Bias del filtro (-5000..5000)" },
/* ko */ { IDS_BIAS_M5000_P5000_KO, "" },  /* fuzzy */
/* nl */ { IDS_BIAS_M5000_P5000_NL, "Filter voorkeur (-5000..5000)" },
/* pl */ { IDS_BIAS_M5000_P5000_PL, "Nastawienie filtra (-5000..5000)" },
/* ru */ { IDS_BIAS_M5000_P5000_RU, "" },  /* fuzzy */
/* sv */ { IDS_BIAS_M5000_P5000_SV, "" },  /* fuzzy */
/* tr */ { IDS_BIAS_M5000_P5000_TR, "" },  /* fuzzy */

/* en */ { IDS_NOT_IMPLEMENTED_YET,    "Not implemented yet!" },
/* da */ { IDS_NOT_IMPLEMENTED_YET_DA, "Endnu ikke implementeret!" },
/* de */ { IDS_NOT_IMPLEMENTED_YET_DE, "Noch nicht implementiert!" },
/* es */ { IDS_NOT_IMPLEMENTED_YET_ES, "¡No implementado aún!" },
/* fr */ { IDS_NOT_IMPLEMENTED_YET_FR, "Pas encore implémenté!" },
/* hu */ { IDS_NOT_IMPLEMENTED_YET_HU, "Még nem implementált!" },
/* it */ { IDS_NOT_IMPLEMENTED_YET_IT, "Non ancora implementato!" },
/* ko */ { IDS_NOT_IMPLEMENTED_YET_KO, "" },  /* fuzzy */
/* nl */ { IDS_NOT_IMPLEMENTED_YET_NL, "Nog niet geïmplementeerd!" },
/* pl */ { IDS_NOT_IMPLEMENTED_YET_PL, "Jeszcze nie zaimplementowano!" },
/* ru */ { IDS_NOT_IMPLEMENTED_YET_RU, "Not implemented yet!" },
/* sv */ { IDS_NOT_IMPLEMENTED_YET_SV, "Ej implementerat ännu!" },
/* tr */ { IDS_NOT_IMPLEMENTED_YET_TR, "Henüz tamamlanmadý!" },

/* en */ { IDS_SID_SETTINGS,    "SID settings" },
/* da */ { IDS_SID_SETTINGS_DA, "SID-indstillinger" },
/* de */ { IDS_SID_SETTINGS_DE, "SID Einstellungen" },
/* es */ { IDS_SID_SETTINGS_ES, "Ajustes SID" },
/* fr */ { IDS_SID_SETTINGS_FR, "Paramètres SID" },
/* hu */ { IDS_SID_SETTINGS_HU, "SID beállításai" },
/* it */ { IDS_SID_SETTINGS_IT, "Impostazioni SID" },
/* ko */ { IDS_SID_SETTINGS_KO, "SID ¼³Á¤" },
/* nl */ { IDS_SID_SETTINGS_NL, "SID-instellingen" },
/* pl */ { IDS_SID_SETTINGS_PL, "Ustawienia SID" },
/* ru */ { IDS_SID_SETTINGS_RU, "½ÐáâàÞÙÚØ SID" },
/* sv */ { IDS_SID_SETTINGS_SV, "SID-inställningar" },
/* tr */ { IDS_SID_SETTINGS_TR, "SID ayarlarý" },

/* en */ { IDS_FLEXIBLE,    "Flexible" },
/* da */ { IDS_FLEXIBLE_DA, "Fleksibel" },
/* de */ { IDS_FLEXIBLE_DE, "Flexibel" },
/* es */ { IDS_FLEXIBLE_ES, "Flexible" },
/* fr */ { IDS_FLEXIBLE_FR, "Flexible" },
/* hu */ { IDS_FLEXIBLE_HU, "Rugalmas" },
/* it */ { IDS_FLEXIBLE_IT, "Flessibile" },
/* ko */ { IDS_FLEXIBLE_KO, "À¯¿¬ÇÑ" },
/* nl */ { IDS_FLEXIBLE_NL, "Flexibel" },
/* pl */ { IDS_FLEXIBLE_PL, "Elastyczna" },
/* ru */ { IDS_FLEXIBLE_RU, "Flexible" },
/* sv */ { IDS_FLEXIBLE_SV, "Flexibel" },
/* tr */ { IDS_FLEXIBLE_TR, "Esnek" },

/* en */ { IDS_ADJUSTING,    "Adjusting" },
/* da */ { IDS_ADJUSTING_DA, "Tilpassende" },
/* de */ { IDS_ADJUSTING_DE, "Anpassend" },
/* es */ { IDS_ADJUSTING_ES, "Ajustado" },
/* fr */ { IDS_ADJUSTING_FR, "Ajusté" },
/* hu */ { IDS_ADJUSTING_HU, "Igazodó" },
/* it */ { IDS_ADJUSTING_IT, "Adattabile" },
/* ko */ { IDS_ADJUSTING_KO, "ÀûÀÀ" },
/* nl */ { IDS_ADJUSTING_NL, "Aanpassen" },
/* pl */ { IDS_ADJUSTING_PL, "Dopasowana" },
/* ru */ { IDS_ADJUSTING_RU, "Adjusting" },
/* sv */ { IDS_ADJUSTING_SV, "Anpassande" },
/* tr */ { IDS_ADJUSTING_TR, "Düzeltme" },

/* en */ { IDS_EXACT,    "Exact" },
/* da */ { IDS_EXACT_DA, "Nøjagtig" },
/* de */ { IDS_EXACT_DE, "Exakt" },
/* es */ { IDS_EXACT_ES, "Exacto" },
/* fr */ { IDS_EXACT_FR, "Exact" },
/* hu */ { IDS_EXACT_HU, "Pontos" },
/* it */ { IDS_EXACT_IT, "Esatta" },
/* ko */ { IDS_EXACT_KO, "Á¤È®ÇÑ" },
/* nl */ { IDS_EXACT_NL, "Exact" },
/* pl */ { IDS_EXACT_PL, "Dok³adna" },
/* ru */ { IDS_EXACT_RU, "Exact" },
/* sv */ { IDS_EXACT_SV, "Exakt" },
/* tr */ { IDS_EXACT_TR, "Aynen" },

/* en */ { IDS_SAMPLE_RATE,    "Sample rate" },
/* da */ { IDS_SAMPLE_RATE_DA, "Samplingfrekvens" },
/* de */ { IDS_SAMPLE_RATE_DE, "Sample Rate" },
/* es */ { IDS_SAMPLE_RATE_ES, "Velocidad de sampleado" },
/* fr */ { IDS_SAMPLE_RATE_FR, "Taux d'échantillonage" },
/* hu */ { IDS_SAMPLE_RATE_HU, "Mintavételezési ráta" },
/* it */ { IDS_SAMPLE_RATE_IT, "Frequenza di campionamento" },
/* ko */ { IDS_SAMPLE_RATE_KO, "»ùÇÃ ·¡ÀÌÆ®" },
/* nl */ { IDS_SAMPLE_RATE_NL, "Sample snelheid" },
/* pl */ { IDS_SAMPLE_RATE_PL, "Próbkowanie" },
/* ru */ { IDS_SAMPLE_RATE_RU, "Sample rate" },
/* sv */ { IDS_SAMPLE_RATE_SV, "Samplingshastighet" },
/* tr */ { IDS_SAMPLE_RATE_TR, "Örnek hýzý" },

/* en */ { IDS_BUFFER_SIZE,    "Buffer size" },
/* da */ { IDS_BUFFER_SIZE_DA, "Bufferstørrelse" },
/* de */ { IDS_BUFFER_SIZE_DE, "Puffergröße" },
/* es */ { IDS_BUFFER_SIZE_ES, "Tamaño del buffer" },
/* fr */ { IDS_BUFFER_SIZE_FR, "Taille du tampon" },
/* hu */ { IDS_BUFFER_SIZE_HU, "Puffer méret" },
/* it */ { IDS_BUFFER_SIZE_IT, "Dimensione buffer" },
/* ko */ { IDS_BUFFER_SIZE_KO, "¹öÆÛ »çÀÌÁî" },
/* nl */ { IDS_BUFFER_SIZE_NL, "Buffergrootte" },
/* pl */ { IDS_BUFFER_SIZE_PL, "Rozmiar buffora" },
/* ru */ { IDS_BUFFER_SIZE_RU, "Buffer size" },
/* sv */ { IDS_BUFFER_SIZE_SV, "Buffertstorlek" },
/* tr */ { IDS_BUFFER_SIZE_TR, "Arabellek boyutu" },

/* en */ { IDS_FRAGMENT_SIZE,    "Fragment size" },
/* da */ { IDS_FRAGMENT_SIZE_DA, "Fragmentstørrelse" },
/* de */ { IDS_FRAGMENT_SIZE_DE, "Fragmentgröße" },
/* es */ { IDS_FRAGMENT_SIZE_ES, "Tamaño de fragmento" },
/* fr */ { IDS_FRAGMENT_SIZE_FR, "taille du fragment" },
/* hu */ { IDS_FRAGMENT_SIZE_HU, "Töredék mérete" },
/* it */ { IDS_FRAGMENT_SIZE_IT, "Dimensione frammento" },
/* ko */ { IDS_FRAGMENT_SIZE_KO, "" },  /* fuzzy */
/* nl */ { IDS_FRAGMENT_SIZE_NL, "Fragment grootte" },
/* pl */ { IDS_FRAGMENT_SIZE_PL, "Rozmiar fragmentu" },
/* ru */ { IDS_FRAGMENT_SIZE_RU, "Fragment size" },
/* sv */ { IDS_FRAGMENT_SIZE_SV, "Fragmentstorlek" },
/* tr */ { IDS_FRAGMENT_SIZE_TR, "Parça boyutu" },

/* en */ { IDS_SPEED_ADJUSTMENT,    "Speed Adjustment" },
/* da */ { IDS_SPEED_ADJUSTMENT_DA, "Hastighedsjustering" },
/* de */ { IDS_SPEED_ADJUSTMENT_DE, "Geschwindigkeitsanpassung" },
/* es */ { IDS_SPEED_ADJUSTMENT_ES, "Ajuste velocidad" },
/* fr */ { IDS_SPEED_ADJUSTMENT_FR, "Ajustement de vitesse" },
/* hu */ { IDS_SPEED_ADJUSTMENT_HU, "Sebesség beállítás" },
/* it */ { IDS_SPEED_ADJUSTMENT_IT, "Variazione velocità" },
/* ko */ { IDS_SPEED_ADJUSTMENT_KO, "" },  /* fuzzy */
/* nl */ { IDS_SPEED_ADJUSTMENT_NL, "Snelheidsaanpassing" },
/* pl */ { IDS_SPEED_ADJUSTMENT_PL, "Regulacja prêdko¶ci" },
/* ru */ { IDS_SPEED_ADJUSTMENT_RU, "½ÐáâàÞÙÚÐ áÚÞàÞáâØ" },
/* sv */ { IDS_SPEED_ADJUSTMENT_SV, "Hastighetsjusering" },
/* tr */ { IDS_SPEED_ADJUSTMENT_TR, "Hýz Ayarlamasý" },

/* en */ { IDS_SOUND_SETTINGS,    "Sound settings" },
/* da */ { IDS_SOUND_SETTINGS_DA, "Lydindstillinger" },
/* de */ { IDS_SOUND_SETTINGS_DE, "Sound Einstellungen" },
/* es */ { IDS_SOUND_SETTINGS_ES, "Ajustes de sonido" },
/* fr */ { IDS_SOUND_SETTINGS_FR, "Paramètres son" },
/* hu */ { IDS_SOUND_SETTINGS_HU, "Hang beállításai" },
/* it */ { IDS_SOUND_SETTINGS_IT, "Impostazioni audio" },
/* ko */ { IDS_SOUND_SETTINGS_KO, "¼Ò¸® ¼³Á¤" },
/* nl */ { IDS_SOUND_SETTINGS_NL, "Geluidsinstellingen" },
/* pl */ { IDS_SOUND_SETTINGS_PL, "Ustawienia d¼wiêku" },
/* ru */ { IDS_SOUND_SETTINGS_RU, "½ÐáâàÞÙÚØ ×ÒãÚÐ" },
/* sv */ { IDS_SOUND_SETTINGS_SV, "Ljudinställningar" },
/* tr */ { IDS_SOUND_SETTINGS_TR, "Ses ayarlarý" },

/* en */ { IDS_VIC20_SETTINGS,    "VIC20 settings" },
/* da */ { IDS_VIC20_SETTINGS_DA, "VIC20-indstillinger" },
/* de */ { IDS_VIC20_SETTINGS_DE, "VIC20 Einstellungen" },
/* es */ { IDS_VIC20_SETTINGS_ES, "Ajustes VIC20" },
/* fr */ { IDS_VIC20_SETTINGS_FR, "Paramètres VIC20" },
/* hu */ { IDS_VIC20_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_VIC20_SETTINGS_IT, "Impostazioni VIC20" },
/* ko */ { IDS_VIC20_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_VIC20_SETTINGS_NL, "VIC20 instellingen" },
/* pl */ { IDS_VIC20_SETTINGS_PL, "Ustawienia VIC20" },
/* ru */ { IDS_VIC20_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_VIC20_SETTINGS_SV, "VIC20-inställningar" },
/* tr */ { IDS_VIC20_SETTINGS_TR, "VIC20 ayarlarý" },

/* en */ { IDS_SPRITE_SPRITE_COL,    "Sprite-sprite collisions" },
/* da */ { IDS_SPRITE_SPRITE_COL_DA, "Sprite-til-sprite-kollision" },
/* de */ { IDS_SPRITE_SPRITE_COL_DE, "Sprite-sprite Kollisionen" },
/* es */ { IDS_SPRITE_SPRITE_COL_ES, "Colisiones sprite-sprite" },
/* fr */ { IDS_SPRITE_SPRITE_COL_FR, "Collisions sprite-sprite" },
/* hu */ { IDS_SPRITE_SPRITE_COL_HU, "" },  /* fuzzy */
/* it */ { IDS_SPRITE_SPRITE_COL_IT, "Collisioni sprite-sprite" },
/* ko */ { IDS_SPRITE_SPRITE_COL_KO, "½ºÇÁ¶óÀÌÆ®-½ºÇÁ¶óÀÌÆ® Ãæµ¹" },
/* nl */ { IDS_SPRITE_SPRITE_COL_NL, "Sprite-sprite botsingen" },
/* pl */ { IDS_SPRITE_SPRITE_COL_PL, "Kolizje duszków" },
/* ru */ { IDS_SPRITE_SPRITE_COL_RU, "Sprite-sprite collisions" },
/* sv */ { IDS_SPRITE_SPRITE_COL_SV, "Sprite-till-sprite-kollision" },
/* tr */ { IDS_SPRITE_SPRITE_COL_TR, "Yartýk-yaratýk çarpýþmalarý" },

/* en */ { IDS_SPRITE_BACKGROUND_COL,    "Sprite-background collisions" },
/* da */ { IDS_SPRITE_BACKGROUND_COL_DA, "Sprite-til-baggrunds-kollision" },
/* de */ { IDS_SPRITE_BACKGROUND_COL_DE, "Sprite-Hintergrund Kollisionen" },
/* es */ { IDS_SPRITE_BACKGROUND_COL_ES, "Colisiones sprite-fondo" },
/* fr */ { IDS_SPRITE_BACKGROUND_COL_FR, "Collisions sprite-arrière-plan" },
/* hu */ { IDS_SPRITE_BACKGROUND_COL_HU, "" },  /* fuzzy */
/* it */ { IDS_SPRITE_BACKGROUND_COL_IT, "Collisioni sprite-sfondo" },
/* ko */ { IDS_SPRITE_BACKGROUND_COL_KO, "½ºÇÁ¶óÀÌÆ®-¹è°æ Ãæµ¹" },
/* nl */ { IDS_SPRITE_BACKGROUND_COL_NL, "Sprite-achtergrond botsingen" },
/* pl */ { IDS_SPRITE_BACKGROUND_COL_PL, "Kolizje duszków z t³em" },
/* ru */ { IDS_SPRITE_BACKGROUND_COL_RU, "Sprite-background collisions" },
/* sv */ { IDS_SPRITE_BACKGROUND_COL_SV, "Sprite-till-bakgrund-kollision" },
/* tr */ { IDS_SPRITE_BACKGROUND_COL_TR, "Yaratýk-arkaplan çarpýþmalarý" },

/* en */ { IDS_NEW_LUMINANCES,    "New luminances" },
/* da */ { IDS_NEW_LUMINANCES_DA, "Nye lysstyrker" },
/* de */ { IDS_NEW_LUMINANCES_DE, "Neue Helligkeitsemulation" },
/* es */ { IDS_NEW_LUMINANCES_ES, "Nuevas luminancias" },
/* fr */ { IDS_NEW_LUMINANCES_FR, "Nouvelles luminescences" },
/* hu */ { IDS_NEW_LUMINANCES_HU, "" },  /* fuzzy */
/* it */ { IDS_NEW_LUMINANCES_IT, "Nuove luminanze" },
/* ko */ { IDS_NEW_LUMINANCES_KO, "¼¼·Î¿î ¹à±â" },
/* nl */ { IDS_NEW_LUMINANCES_NL, "Nieuwe luminanties" },
/* pl */ { IDS_NEW_LUMINANCES_PL, "Nowy typ jaskrawo¶ci" },
/* ru */ { IDS_NEW_LUMINANCES_RU, "New luminances" },
/* sv */ { IDS_NEW_LUMINANCES_SV, "Nya ljusstyrkor" },
/* tr */ { IDS_NEW_LUMINANCES_TR, "Yeni parlaklýklar" },

/* en */ { IDS_VSPBUG,    "VSP bug" },
/* da */ { IDS_VSPBUG_DA, "" },  /* fuzzy */
/* de */ { IDS_VSPBUG_DE, "VSP Fehler" },
/* es */ { IDS_VSPBUG_ES, "" },  /* fuzzy */
/* fr */ { IDS_VSPBUG_FR, "" },  /* fuzzy */
/* hu */ { IDS_VSPBUG_HU, "" },  /* fuzzy */
/* it */ { IDS_VSPBUG_IT, "" },  /* fuzzy */
/* ko */ { IDS_VSPBUG_KO, "" },  /* fuzzy */
/* nl */ { IDS_VSPBUG_NL, "VSP bug" },
/* pl */ { IDS_VSPBUG_PL, "" },  /* fuzzy */
/* ru */ { IDS_VSPBUG_RU, "" },  /* fuzzy */
/* sv */ { IDS_VSPBUG_SV, "" },  /* fuzzy */
/* tr */ { IDS_VSPBUG_TR, "" },  /* fuzzy */

/* en */ { IDS_VICII_SETTINGS,    "VIC-II settings" },
/* da */ { IDS_VICII_SETTINGS_DA, "VIC-II-indstillinger" },
/* de */ { IDS_VICII_SETTINGS_DE, "VIC-II Einstellungen" },
/* es */ { IDS_VICII_SETTINGS_ES, "Ajustes VIC-II" },
/* fr */ { IDS_VICII_SETTINGS_FR, "Paramètres VIC-II" },
/* hu */ { IDS_VICII_SETTINGS_HU, "VIC-II beállításai" },
/* it */ { IDS_VICII_SETTINGS_IT, "Impostazioni VIC-II" },
/* ko */ { IDS_VICII_SETTINGS_KO, "VIC-II ¼³Á¤" },
/* nl */ { IDS_VICII_SETTINGS_NL, "VIC-II-instellingen" },
/* pl */ { IDS_VICII_SETTINGS_PL, "Ustawienia VIC-II" },
/* ru */ { IDS_VICII_SETTINGS_RU, "½ÐáâàÞÙÚØ VIC-II" },
/* sv */ { IDS_VICII_SETTINGS_SV, "VIC-II-inställningar" },
/* tr */ { IDS_VICII_SETTINGS_TR, "VIC-II ayarlarý" },

/* en */ { IDS_VIC_SETTINGS,    "VIC settings" },
/* da */ { IDS_VIC_SETTINGS_DA, "VIC-indstillinger" },
/* de */ { IDS_VIC_SETTINGS_DE, "VIC Einstellungen" },
/* es */ { IDS_VIC_SETTINGS_ES, "Ajustes VIC" },
/* fr */ { IDS_VIC_SETTINGS_FR, "Paramètres VIC" },
/* hu */ { IDS_VIC_SETTINGS_HU, "VIC beállításai" },
/* it */ { IDS_VIC_SETTINGS_IT, "Impostazioni VIC" },
/* ko */ { IDS_VIC_SETTINGS_KO, "VIC ¼ÂÆÃ" },
/* nl */ { IDS_VIC_SETTINGS_NL, "VIC instellingen" },
/* pl */ { IDS_VIC_SETTINGS_PL, "Ustawienia VIC" },
/* ru */ { IDS_VIC_SETTINGS_RU, "VIC settings" },
/* sv */ { IDS_VIC_SETTINGS_SV, "VIC-inställningar" },
/* tr */ { IDS_VIC_SETTINGS_TR, "VIC ayarlarý" },

/* en */ { IDS_TED_SETTINGS,    "TED settings" },
/* da */ { IDS_TED_SETTINGS_DA, "TED-indstillinger" },
/* de */ { IDS_TED_SETTINGS_DE, "TED Einstellungen" },
/* es */ { IDS_TED_SETTINGS_ES, "Ajustes TED" },
/* fr */ { IDS_TED_SETTINGS_FR, "Paramètres TED" },
/* hu */ { IDS_TED_SETTINGS_HU, "TED beállításai" },
/* it */ { IDS_TED_SETTINGS_IT, "Impostazioni TED" },
/* ko */ { IDS_TED_SETTINGS_KO, "TED ¼ÂÆÃ" },
/* nl */ { IDS_TED_SETTINGS_NL, "TED-instellingen" },
/* pl */ { IDS_TED_SETTINGS_PL, "Ustawienia TED" },
/* ru */ { IDS_TED_SETTINGS_RU, "½ÐáâàÞÙÚØ TED" },
/* sv */ { IDS_TED_SETTINGS_SV, "TED-inställningar" },
/* tr */ { IDS_TED_SETTINGS_TR, "TED ayarlarý" },

/* en */ { IDS_ATTACH_TAPE_IMAGE,    "Attach tape image" },
/* da */ { IDS_ATTACH_TAPE_IMAGE_DA, "Tilslut bånd-image" },
/* de */ { IDS_ATTACH_TAPE_IMAGE_DE, "Band Image einlegen" },
/* es */ { IDS_ATTACH_TAPE_IMAGE_ES, "Insertar imagen de cinta" },
/* fr */ { IDS_ATTACH_TAPE_IMAGE_FR, "Insérer une image de datassette" },
/* hu */ { IDS_ATTACH_TAPE_IMAGE_HU, "Szalag képmás csatolása" },
/* it */ { IDS_ATTACH_TAPE_IMAGE_IT, "Seleziona immagine cassetta" },
/* ko */ { IDS_ATTACH_TAPE_IMAGE_KO, "Å×ÀÌÇÁ ÀÌ¹ÌÁö ºÙ¿©³Ö±â" },
/* nl */ { IDS_ATTACH_TAPE_IMAGE_NL, "Koppel tapebestand" },
/* pl */ { IDS_ATTACH_TAPE_IMAGE_PL, "Zamontuj obraz ta¶my" },
/* ru */ { IDS_ATTACH_TAPE_IMAGE_RU, "´ÞÑÐÒØâì ÞÑàÐ× ×ÐßØáØ" },
/* sv */ { IDS_ATTACH_TAPE_IMAGE_SV, "Anslut bandavbildningsfil" },
/* tr */ { IDS_ATTACH_TAPE_IMAGE_TR, "Teyp imajýný yerleþtir" },

/* en */ { IDS_AUTOSTART_IMAGE,    "Autostart disk/tape image" },
/* da */ { IDS_AUTOSTART_IMAGE_DA, "Autostart disk-/bånd-image" },
/* de */ { IDS_AUTOSTART_IMAGE_DE, "Autostart von Disk/Band Image" },
/* es */ { IDS_AUTOSTART_IMAGE_ES, "Autoarranque imagen de disco/cinta" },
/* fr */ { IDS_AUTOSTART_IMAGE_FR, "Autodémarrage image disque/datassette" },
/* hu */ { IDS_AUTOSTART_IMAGE_HU, "Lemez/szalag képmás automatikus indítása" },
/* it */ { IDS_AUTOSTART_IMAGE_IT, "Avvia automaticamente immagine disco/cassetta" },
/* ko */ { IDS_AUTOSTART_IMAGE_KO, "ÀÚµ¿ ½ÃÀÛ µð½ºÅ©/Å×ÀÌÇÁ ÀÌ¹ÌÁö" },
/* nl */ { IDS_AUTOSTART_IMAGE_NL, "Autostart disk-/tapebestand" },
/* pl */ { IDS_AUTOSTART_IMAGE_PL, "Automatycznie startuj z obrazu dysku lub ta¶my" },
/* ru */ { IDS_AUTOSTART_IMAGE_RU, "°ÒâÞÜÐâØçÕáÚØ ×ÐßãáâØâì ÞÑàÐ× ÔØáÚÐ/ÚÐááÕâë" },
/* sv */ { IDS_AUTOSTART_IMAGE_SV, "Autostarta disk-/bandavbildningsfil" },
/* tr */ { IDS_AUTOSTART_IMAGE_TR, "Disk/Teyp imajýný otomatik baþlat" },

/* en */ { IDS_IDE64_FILENAME,    "HD image file" },
/* da */ { IDS_IDE64_FILENAME_DA, "HD-imagefil" },
/* de */ { IDS_IDE64_FILENAME_DE, "HD Image Name" },
/* es */ { IDS_IDE64_FILENAME_ES, "Fichero imagen HD" },
/* fr */ { IDS_IDE64_FILENAME_FR, "Nom d'image HD" },
/* hu */ { IDS_IDE64_FILENAME_HU, "Merevlemez képmás fájl" },
/* it */ { IDS_IDE64_FILENAME_IT, "File immagine HD" },
/* ko */ { IDS_IDE64_FILENAME_KO, "" },  /* fuzzy */
/* nl */ { IDS_IDE64_FILENAME_NL, "HD bestand" },
/* pl */ { IDS_IDE64_FILENAME_PL, "Plik obrazu HD" },
/* ru */ { IDS_IDE64_FILENAME_RU, "" },  /* fuzzy */
/* sv */ { IDS_IDE64_FILENAME_SV, "HD-avbildningsfil" },
/* tr */ { IDS_IDE64_FILENAME_TR, "HD imaj dosyasý" },

/* en */ { IDS_IDE64_FILENAME_SELECT,    "Select HD image file" },
/* da */ { IDS_IDE64_FILENAME_SELECT_DA, "Angiv HD-imagefil" },
/* de */ { IDS_IDE64_FILENAME_SELECT_DE, "HD Imagedatei auswählen" },
/* es */ { IDS_IDE64_FILENAME_SELECT_ES, "Seleccionar fichero imagen HD" },
/* fr */ { IDS_IDE64_FILENAME_SELECT_FR, "Sélectionnez le fichier image du D.D." },
/* hu */ { IDS_IDE64_FILENAME_SELECT_HU, "Válassza ki a merevlemez képmás fájlt" },
/* it */ { IDS_IDE64_FILENAME_SELECT_IT, "Seleziona il file immagine HD" },
/* ko */ { IDS_IDE64_FILENAME_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDS_IDE64_FILENAME_SELECT_NL, "Selecteer HD bestand" },
/* pl */ { IDS_IDE64_FILENAME_SELECT_PL, "Wybierz plik obrazu HD" },
/* ru */ { IDS_IDE64_FILENAME_SELECT_RU, "Select HD image file" },
/* sv */ { IDS_IDE64_FILENAME_SELECT_SV, "Ange HD-avbildningsfil" },
/* tr */ { IDS_IDE64_FILENAME_SELECT_TR, "HD imaj dosyasý seç" },

/* en */ { IDS_IDE64_HD_1_SETTINGS,    "IDE64 primary master device settings" },
/* da */ { IDS_IDE64_HD_1_SETTINGS_DA, "IDE64 primær master enhedsindstillinger" },
/* de */ { IDS_IDE64_HD_1_SETTINGS_DE, "IDE64 Primär Master Geräteeinstellungen" },
/* es */ { IDS_IDE64_HD_1_SETTINGS_ES, "Ajustes periférico maestro primario IDE64" },
/* fr */ { IDS_IDE64_HD_1_SETTINGS_FR, "Paramètres périphérique maître primaire IDE64" },
/* hu */ { IDS_IDE64_HD_1_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_IDE64_HD_1_SETTINGS_IT, "Impostazioni dispositivo IDE64 master sul canale primario" },
/* ko */ { IDS_IDE64_HD_1_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_IDE64_HD_1_SETTINGS_NL, "IDE64 primair hoofd apparaat instellingen" },
/* pl */ { IDS_IDE64_HD_1_SETTINGS_PL, "Ustawienia pierwszego urz±dzenia master IDE64" },
/* ru */ { IDS_IDE64_HD_1_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_IDE64_HD_1_SETTINGS_SV, "Inställningar för primär IDE64-masterenhet" },
/* tr */ { IDS_IDE64_HD_1_SETTINGS_TR, "IDE64 birincil ana sürücü ayarlarý" },

/* en */ { IDS_IDE64_HD_2_SETTINGS,    "IDE64 primary slave device settings" },
/* da */ { IDS_IDE64_HD_2_SETTINGS_DA, "IDE64 primær slave enhedsindstillinger" },
/* de */ { IDS_IDE64_HD_2_SETTINGS_DE, "IDE64 Primär Slave Geräteeinstellungen" },
/* es */ { IDS_IDE64_HD_2_SETTINGS_ES, "Ajustes periférico esclavo primario IDE64" },
/* fr */ { IDS_IDE64_HD_2_SETTINGS_FR, "Paramètres périphérique esclave primaire IDE64" },
/* hu */ { IDS_IDE64_HD_2_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_IDE64_HD_2_SETTINGS_IT, "Impostazioni dispositivo IDE64 slave sul canale primario" },
/* ko */ { IDS_IDE64_HD_2_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_IDE64_HD_2_SETTINGS_NL, "IDE64 HD primair slaaf apparaat instellingen" },
/* pl */ { IDS_IDE64_HD_2_SETTINGS_PL, "Ustawienia pierwszego urz±dzenia slave IDE64" },
/* ru */ { IDS_IDE64_HD_2_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_IDE64_HD_2_SETTINGS_SV, "Inställningar för primär IDE64-slavenhet" },
/* tr */ { IDS_IDE64_HD_2_SETTINGS_TR, "IDE64 birincil baðýmlý sürücü ayarlarý" },

/* en */ { IDS_IDE64_HD_3_SETTINGS,    "IDE64 secondary master device settings" },
/* da */ { IDS_IDE64_HD_3_SETTINGS_DA, "IDE64 sekundær master enhedsindstillinger" },
/* de */ { IDS_IDE64_HD_3_SETTINGS_DE, "IDE64 Sekundär Master Geräteeinstellungen" },
/* es */ { IDS_IDE64_HD_3_SETTINGS_ES, "Ajustes periférico maestro secundario IDE64" },
/* fr */ { IDS_IDE64_HD_3_SETTINGS_FR, "Paramètres périphérique maître secondaire IDE64" },
/* hu */ { IDS_IDE64_HD_3_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_IDE64_HD_3_SETTINGS_IT, "Impostazioni dispositivo IDE64 slave sul canale secondario" },
/* ko */ { IDS_IDE64_HD_3_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_IDE64_HD_3_SETTINGS_NL, "IDE64 secundair hoofd apparaat instellingen" },
/* pl */ { IDS_IDE64_HD_3_SETTINGS_PL, "Ustawienia drugiego urz±dzenia master IDE64" },
/* ru */ { IDS_IDE64_HD_3_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_IDE64_HD_3_SETTINGS_SV, "Inställningar för sekundär IDE64-masterenhet" },
/* tr */ { IDS_IDE64_HD_3_SETTINGS_TR, "IDE64 ikincil ana sürücü ayarlarý" },

/* en */ { IDS_IDE64_HD_4_SETTINGS,    "IDE64 secondary slave device settings" },
/* da */ { IDS_IDE64_HD_4_SETTINGS_DA, "IDE64 sekundær slave enhedsindstillinger" },
/* de */ { IDS_IDE64_HD_4_SETTINGS_DE, "IDE64 Sekundär Slave Geräteeinstellungen" },
/* es */ { IDS_IDE64_HD_4_SETTINGS_ES, "Ajustes periférico esclavo secundario IDE64" },
/* fr */ { IDS_IDE64_HD_4_SETTINGS_FR, "Paramètres esclave secondaire IDE64" },
/* hu */ { IDS_IDE64_HD_4_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_IDE64_HD_4_SETTINGS_IT, "Impostazioni dispositivo IDE64 master sul canale secondario" },
/* ko */ { IDS_IDE64_HD_4_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_IDE64_HD_4_SETTINGS_NL, "IDE64 secundair slaaf apparaat instellingen" },
/* pl */ { IDS_IDE64_HD_4_SETTINGS_PL, "Ustawienia drugiego urz±dzenia slave IDE64" },
/* ru */ { IDS_IDE64_HD_4_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_IDE64_HD_4_SETTINGS_SV, "Inställningar för sekundär IDE64-slavenhet" },
/* tr */ { IDS_IDE64_HD_4_SETTINGS_TR, "IDE64 ikincil baðýmlý sürücü ayarlarý" },

/* en */ { IDS_IDE64_VERSION,    "IDE64 version" },
/* da */ { IDS_IDE64_VERSION_DA, "" },  /* fuzzy */
/* de */ { IDS_IDE64_VERSION_DE, "IDE64 Version" },
/* es */ { IDS_IDE64_VERSION_ES, "" },  /* fuzzy */
/* fr */ { IDS_IDE64_VERSION_FR, "" },  /* fuzzy */
/* hu */ { IDS_IDE64_VERSION_HU, "" },  /* fuzzy */
/* it */ { IDS_IDE64_VERSION_IT, "" },  /* fuzzy */
/* ko */ { IDS_IDE64_VERSION_KO, "" },  /* fuzzy */
/* nl */ { IDS_IDE64_VERSION_NL, "IDE64 versie" },
/* pl */ { IDS_IDE64_VERSION_PL, "Wersja IDE64" },
/* ru */ { IDS_IDE64_VERSION_RU, "" },  /* fuzzy */
/* sv */ { IDS_IDE64_VERSION_SV, "" },  /* fuzzy */
/* tr */ { IDS_IDE64_VERSION_TR, "" },  /* fuzzy */

/* en */ { IDS_USB_SERVER,    "USB Server" },
/* da */ { IDS_USB_SERVER_DA, "" },  /* fuzzy */
/* de */ { IDS_USB_SERVER_DE, "USB Server" },
/* es */ { IDS_USB_SERVER_ES, "Servidor USB" },
/* fr */ { IDS_USB_SERVER_FR, "" },  /* fuzzy */
/* hu */ { IDS_USB_SERVER_HU, "" },  /* fuzzy */
/* it */ { IDS_USB_SERVER_IT, "Server USB" },
/* ko */ { IDS_USB_SERVER_KO, "" },  /* fuzzy */
/* nl */ { IDS_USB_SERVER_NL, "USB Server" },
/* pl */ { IDS_USB_SERVER_PL, "Serwer USB" },
/* ru */ { IDS_USB_SERVER_RU, "" },  /* fuzzy */
/* sv */ { IDS_USB_SERVER_SV, "" },  /* fuzzy */
/* tr */ { IDS_USB_SERVER_TR, "" },  /* fuzzy */

/* en */ { IDS_USB_SERVER_ADDRESS,    "USB Server address to bind to" },
/* da */ { IDS_USB_SERVER_ADDRESS_DA, "" },  /* fuzzy */
/* de */ { IDS_USB_SERVER_ADDRESS_DE, "USB Server Adresse" },
/* es */ { IDS_USB_SERVER_ADDRESS_ES, "Seleccionar dirección servidorUSB" },
/* fr */ { IDS_USB_SERVER_ADDRESS_FR, "" },  /* fuzzy */
/* hu */ { IDS_USB_SERVER_ADDRESS_HU, "" },  /* fuzzy */
/* it */ { IDS_USB_SERVER_ADDRESS_IT, "Indirizzo server USB a cui collegarsi" },
/* ko */ { IDS_USB_SERVER_ADDRESS_KO, "" },  /* fuzzy */
/* nl */ { IDS_USB_SERVER_ADDRESS_NL, "USB server adres waaraan gebonden moet worden" },
/* pl */ { IDS_USB_SERVER_ADDRESS_PL, "Wprowad¼ adres serwera USB" },
/* ru */ { IDS_USB_SERVER_ADDRESS_RU, "" },  /* fuzzy */
/* sv */ { IDS_USB_SERVER_ADDRESS_SV, "" },  /* fuzzy */
/* tr */ { IDS_USB_SERVER_ADDRESS_TR, "" },  /* fuzzy */

/* en */ { IDS_IDE64_RTC_SAVE,    "Enable saving of the IDE64 RTC data" },
/* da */ { IDS_IDE64_RTC_SAVE_DA, "" },  /* fuzzy */
/* de */ { IDS_IDE64_RTC_SAVE_DE, "IDE64 RTC Datenspeicherung aktivieren" },
/* es */ { IDS_IDE64_RTC_SAVE_ES, "Permitir grabar datos de IDE64 RTC" },
/* fr */ { IDS_IDE64_RTC_SAVE_FR, "" },  /* fuzzy */
/* hu */ { IDS_IDE64_RTC_SAVE_HU, "" },  /* fuzzy */
/* it */ { IDS_IDE64_RTC_SAVE_IT, "Attiva salvataggio dei dati RTC dell'IDE64" },
/* ko */ { IDS_IDE64_RTC_SAVE_KO, "" },  /* fuzzy */
/* nl */ { IDS_IDE64_RTC_SAVE_NL, "Activeer opslaan van de IDE64 RTC data" },
/* pl */ { IDS_IDE64_RTC_SAVE_PL, "W³±cz zapis zegara IDE64" },
/* ru */ { IDS_IDE64_RTC_SAVE_RU, "" },  /* fuzzy */
/* sv */ { IDS_IDE64_RTC_SAVE_SV, "" },  /* fuzzy */
/* tr */ { IDS_IDE64_RTC_SAVE_TR, "" },  /* fuzzy */

/* en */ { IDS_AUTODETECT,    "Autodetect image size" },
/* da */ { IDS_AUTODETECT_DA, "Autodetektér størrelse på image" },
/* de */ { IDS_AUTODETECT_DE, "Image Größe automatisch erkennen" },
/* es */ { IDS_AUTODETECT_ES, "Autodetección del tamaño de la imagen" },
/* fr */ { IDS_AUTODETECT_FR, "Autodétection de la taille de l'image" },
/* hu */ { IDS_AUTODETECT_HU, "" },  /* fuzzy */
/* it */ { IDS_AUTODETECT_IT, "Rileva automaticamente immagine" },
/* ko */ { IDS_AUTODETECT_KO, "ÀÌ¹ÌÁö Å©±â ÀÚµ¿À¸·Î Ã£±â" },
/* nl */ { IDS_AUTODETECT_NL, "Automatisch detecteren bestandsgrootte" },
/* pl */ { IDS_AUTODETECT_PL, "Automatycznie wykrywaj rozmiar obrazu" },
/* ru */ { IDS_AUTODETECT_RU, "°ÒâÞÜÐâØçÕáÚÞÕ ÞßàÕÔÕÛÕÝØÕ àÐ×ÜÕàÐ Ø×ÞÑàÐÖÕÝØï" },
/* sv */ { IDS_AUTODETECT_SV, "Autodetektera storlek på avbildning" },
/* tr */ { IDS_AUTODETECT_TR, "Imaj boyutunu otomatik algýla" },

/* en */ { IDS_CYLINDERS,    "Cylinders" },
/* da */ { IDS_CYLINDERS_DA, "Cylindrer" },
/* de */ { IDS_CYLINDERS_DE, "Zylinder" },
/* es */ { IDS_CYLINDERS_ES, "Cilindros" },
/* fr */ { IDS_CYLINDERS_FR, "Cylindres" },
/* hu */ { IDS_CYLINDERS_HU, "Sávok" },
/* it */ { IDS_CYLINDERS_IT, "Cilindri" },
/* ko */ { IDS_CYLINDERS_KO, "½Ç¸°´õ" },
/* nl */ { IDS_CYLINDERS_NL, "Cylinders" },
/* pl */ { IDS_CYLINDERS_PL, "Cylindry" },
/* ru */ { IDS_CYLINDERS_RU, "Cylinders" },
/* sv */ { IDS_CYLINDERS_SV, "Cylindrar" },
/* tr */ { IDS_CYLINDERS_TR, "Silindirler" },

/* en */ { IDS_HEADS,    "Heads" },
/* da */ { IDS_HEADS_DA, "Hoveder" },
/* de */ { IDS_HEADS_DE, "Köpfe" },
/* es */ { IDS_HEADS_ES, "Cabezas" },
/* fr */ { IDS_HEADS_FR, "Têtes" },
/* hu */ { IDS_HEADS_HU, "Fejek" },
/* it */ { IDS_HEADS_IT, "Testine" },
/* ko */ { IDS_HEADS_KO, "Çìµå" },
/* nl */ { IDS_HEADS_NL, "Koppen" },
/* pl */ { IDS_HEADS_PL, "G³owice" },
/* ru */ { IDS_HEADS_RU, "Heads" },
/* sv */ { IDS_HEADS_SV, "Huvuden" },
/* tr */ { IDS_HEADS_TR, "Kafalar" },

/* en */ { IDS_SECTORS,    "Sectors" },
/* da */ { IDS_SECTORS_DA, "Sektorer" },
/* de */ { IDS_SECTORS_DE, "Sektoren" },
/* es */ { IDS_SECTORS_ES, "Sectores" },
/* fr */ { IDS_SECTORS_FR, "Secteurs" },
/* hu */ { IDS_SECTORS_HU, "Szektorok" },
/* it */ { IDS_SECTORS_IT, "Settori" },
/* ko */ { IDS_SECTORS_KO, "¼½ÅÍ" },
/* nl */ { IDS_SECTORS_NL, "Sectoren" },
/* pl */ { IDS_SECTORS_PL, "Sektory" },
/* ru */ { IDS_SECTORS_RU, "Sectors" },
/* sv */ { IDS_SECTORS_SV, "Sektorer" },
/* tr */ { IDS_SECTORS_TR, "Sektörler" },

/* en */ { IDS_IDE64_SETTINGS,    "IDE64 settings" },
/* da */ { IDS_IDE64_SETTINGS_DA, "IDE64-indstillinger" },
/* de */ { IDS_IDE64_SETTINGS_DE, "IDE64 Einstellungen" },
/* es */ { IDS_IDE64_SETTINGS_ES, "Ajustes IDE64" },
/* fr */ { IDS_IDE64_SETTINGS_FR, "Paramètres IDE64" },
/* hu */ { IDS_IDE64_SETTINGS_HU, "IDE64 beállításai" },
/* it */ { IDS_IDE64_SETTINGS_IT, "Impostazioni IDE64" },
/* ko */ { IDS_IDE64_SETTINGS_KO, "IDE64 ¼³Á¤" },
/* nl */ { IDS_IDE64_SETTINGS_NL, "IDE64 instellingen" },
/* pl */ { IDS_IDE64_SETTINGS_PL, "Ustawienia IDE64" },
/* ru */ { IDS_IDE64_SETTINGS_RU, "IDE64 settings" },
/* sv */ { IDS_IDE64_SETTINGS_SV, "IDE64-inställningar" },
/* tr */ { IDS_IDE64_SETTINGS_TR, "IDE64 ayarlarý" },

/* en */ { IDS_BIOS_REVISION,    "BIOS Revision" },
/* da */ { IDS_BIOS_REVISION_DA, "BIOS-revision" },
/* de */ { IDS_BIOS_REVISION_DE, "BIOS Revision" },
/* es */ { IDS_BIOS_REVISION_ES, "Revisión BIOS" },
/* fr */ { IDS_BIOS_REVISION_FR, "Révision BIOS" },
/* hu */ { IDS_BIOS_REVISION_HU, "BIOS változat" },
/* it */ { IDS_BIOS_REVISION_IT, "Revisione BIOS" },
/* ko */ { IDS_BIOS_REVISION_KO, "" },  /* fuzzy */
/* nl */ { IDS_BIOS_REVISION_NL, "BIOS revisie" },
/* pl */ { IDS_BIOS_REVISION_PL, "Wersja BIOS" },
/* ru */ { IDS_BIOS_REVISION_RU, "BIOS Revision" },
/* sv */ { IDS_BIOS_REVISION_SV, "BIOS-utgåva" },
/* tr */ { IDS_BIOS_REVISION_TR, "BIOS Revizyonu" },

/* en */ { IDS_BIOS_FLASH_JUMPER,    "BIOS Flash Jumper" },
/* da */ { IDS_BIOS_FLASH_JUMPER_DA, "BIOS-flashjumper" },
/* de */ { IDS_BIOS_FLASH_JUMPER_DE, "BIOS Flashjumper" },
/* es */ { IDS_BIOS_FLASH_JUMPER_ES, "BIOS Flash Jumper" },
/* fr */ { IDS_BIOS_FLASH_JUMPER_FR, "Flash Jumper BIOS" },
/* hu */ { IDS_BIOS_FLASH_JUMPER_HU, "BIOS Flash jumper" },
/* it */ { IDS_BIOS_FLASH_JUMPER_IT, "Flash Jumper del BIOS" },
/* ko */ { IDS_BIOS_FLASH_JUMPER_KO, "" },  /* fuzzy */
/* nl */ { IDS_BIOS_FLASH_JUMPER_NL, "BIOS Flash Jumper" },
/* pl */ { IDS_BIOS_FLASH_JUMPER_PL, "Zworka BIOS Flash" },
/* ru */ { IDS_BIOS_FLASH_JUMPER_RU, "BIOS Flash Jumper" },
/* sv */ { IDS_BIOS_FLASH_JUMPER_SV, "BIOS-flashbygel" },
/* tr */ { IDS_BIOS_FLASH_JUMPER_TR, "BIOS Flash Jumper" },

/* en */ { IDS_BIOS_FLASH_JUMPER,    "BIOS Flash Jumper" },
/* da */ { IDS_BIOS_FLASH_JUMPER_DA, "BIOS-flashjumper" },
/* de */ { IDS_BIOS_FLASH_JUMPER_DE, "BIOS Flashjumper" },
/* es */ { IDS_BIOS_FLASH_JUMPER_ES, "BIOS Flash Jumper" },
/* fr */ { IDS_BIOS_FLASH_JUMPER_FR, "Flash Jumper BIOS" },
/* hu */ { IDS_BIOS_FLASH_JUMPER_HU, "BIOS Flash jumper" },
/* it */ { IDS_BIOS_FLASH_JUMPER_IT, "Flash Jumper del BIOS" },
/* ko */ { IDS_BIOS_FLASH_JUMPER_KO, "" },  /* fuzzy */
/* nl */ { IDS_BIOS_FLASH_JUMPER_NL, "BIOS Flash Jumper" },
/* pl */ { IDS_BIOS_FLASH_JUMPER_PL, "Zworka BIOS Flash" },
/* ru */ { IDS_BIOS_FLASH_JUMPER_RU, "BIOS Flash Jumper" },
/* sv */ { IDS_BIOS_FLASH_JUMPER_SV, "BIOS-flashbygel" },
/* tr */ { IDS_BIOS_FLASH_JUMPER_TR, "BIOS Flash Jumper" },

/* en */ { IDS_BANK_JUMPER,    "Bank jumper" },
/* da */ { IDS_BANK_JUMPER_DA, "Bank Jumper" },
/* de */ { IDS_BANK_JUMPER_DE, "Bank Jumper" },
/* es */ { IDS_BANK_JUMPER_ES, "Seleccionar conmutador banco" },
/* fr */ { IDS_BANK_JUMPER_FR, "Bank jumper" },
/* hu */ { IDS_BANK_JUMPER_HU, "" },  /* fuzzy */
/* it */ { IDS_BANK_JUMPER_IT, "Ponticello bank" },
/* ko */ { IDS_BANK_JUMPER_KO, "" },  /* fuzzy */
/* nl */ { IDS_BANK_JUMPER_NL, "Bank jumper" },
/* pl */ { IDS_BANK_JUMPER_PL, "Zworka banków" },
/* ru */ { IDS_BANK_JUMPER_RU, "" },  /* fuzzy */
/* sv */ { IDS_BANK_JUMPER_SV, "Bankbygel" },
/* tr */ { IDS_BANK_JUMPER_TR, "Bank jumperý" },

/* en */ { IDS_SAVE_BIOS_WHEN_CHANGED,    "Save BIOS when changed" },
/* da */ { IDS_SAVE_BIOS_WHEN_CHANGED_DA, "Gem BIOS ved ændringer" },
/* de */ { IDS_SAVE_BIOS_WHEN_CHANGED_DE, "BIOS bei Änderung speichern" },
/* es */ { IDS_SAVE_BIOS_WHEN_CHANGED_ES, "Grabar BIOS cuando cambien" },
/* fr */ { IDS_SAVE_BIOS_WHEN_CHANGED_FR, "Enregistrer le BIOS lorsque modifié" },
/* hu */ { IDS_SAVE_BIOS_WHEN_CHANGED_HU, "BIOS mentése változás után" },
/* it */ { IDS_SAVE_BIOS_WHEN_CHANGED_IT, "Salva BIOS alla modifica" },
/* ko */ { IDS_SAVE_BIOS_WHEN_CHANGED_KO, "" },  /* fuzzy */
/* nl */ { IDS_SAVE_BIOS_WHEN_CHANGED_NL, "BIOS opslaan bij wijziging" },
/* pl */ { IDS_SAVE_BIOS_WHEN_CHANGED_PL, "Zapisuj BIOS przy zmianie" },
/* ru */ { IDS_SAVE_BIOS_WHEN_CHANGED_RU, "Save BIOS when changed" },
/* sv */ { IDS_SAVE_BIOS_WHEN_CHANGED_SV, "Spara BIOS vid ändring" },
/* tr */ { IDS_SAVE_BIOS_WHEN_CHANGED_TR, "BIOS deðiþtiðinde kaydet" },

/* en */ { IDS_BIOS_FILE,    "BIOS File" },
/* da */ { IDS_BIOS_FILE_DA, "BIOS-fil" },
/* de */ { IDS_BIOS_FILE_DE, "BIOS Datei" },
/* es */ { IDS_BIOS_FILE_ES, "Fichero BIOS" },
/* fr */ { IDS_BIOS_FILE_FR, "Fichier du BIOS" },
/* hu */ { IDS_BIOS_FILE_HU, "BIOS fájl" },
/* it */ { IDS_BIOS_FILE_IT, "File BIOS" },
/* ko */ { IDS_BIOS_FILE_KO, "" },  /* fuzzy */
/* nl */ { IDS_BIOS_FILE_NL, "BIOS Bestand" },
/* pl */ { IDS_BIOS_FILE_PL, "Plik BIOS" },
/* ru */ { IDS_BIOS_FILE_RU, "BIOS File" },
/* sv */ { IDS_BIOS_FILE_SV, "BIOS-fil" },
/* tr */ { IDS_BIOS_FILE_TR, "BIOS Dosyasý" },

/* en */ { IDS_SELECT_BIOS_FILE,    "Select BIOS file" },
/* da */ { IDS_SELECT_BIOS_FILE_DA, "Vælg BIOS-fil" },
/* de */ { IDS_SELECT_BIOS_FILE_DE, "BIOS Datei wählen" },
/* es */ { IDS_SELECT_BIOS_FILE_ES, "Seleccionar fichero BIOS" },
/* fr */ { IDS_SELECT_BIOS_FILE_FR, "Sélectionner le fichier du BIOS" },
/* hu */ { IDS_SELECT_BIOS_FILE_HU, "Válasszon BIOS fájlt" },
/* it */ { IDS_SELECT_BIOS_FILE_IT, "Seleziona file BIOS" },
/* ko */ { IDS_SELECT_BIOS_FILE_KO, "" },  /* fuzzy */
/* nl */ { IDS_SELECT_BIOS_FILE_NL, "Selecteer BIOS bestand" },
/* pl */ { IDS_SELECT_BIOS_FILE_PL, "Wybierz plik BIOS" },
/* ru */ { IDS_SELECT_BIOS_FILE_RU, "Select BIOS file" },
/* sv */ { IDS_SELECT_BIOS_FILE_SV, "Välj BIOS-fil" },
/* tr */ { IDS_SELECT_BIOS_FILE_TR, "BIOS dosyasý seçin" },

/* en */ { IDS_MMC64_IMAGE_READ_ONLY,    "MMC/SD Image read-only" },
/* da */ { IDS_MMC64_IMAGE_READ_ONLY_DA, "MMC/SD-image skrivebeskyttet" },
/* de */ { IDS_MMC64_IMAGE_READ_ONLY_DE, "MMC/SD Image Schreibschutz" },
/* es */ { IDS_MMC64_IMAGE_READ_ONLY_ES, "Imagen MMC/SD de sólo lectura" },
/* fr */ { IDS_MMC64_IMAGE_READ_ONLY_FR, "Image MMC/SD en lecture seule" },
/* hu */ { IDS_MMC64_IMAGE_READ_ONLY_HU, "MMC/SD képmás csak olvasható" },
/* it */ { IDS_MMC64_IMAGE_READ_ONLY_IT, "Immagine MMC/SD in sola lettura" },
/* ko */ { IDS_MMC64_IMAGE_READ_ONLY_KO, "" },  /* fuzzy */
/* nl */ { IDS_MMC64_IMAGE_READ_ONLY_NL, "MMC/SD bestand alleen-lezen" },
/* pl */ { IDS_MMC64_IMAGE_READ_ONLY_PL, "Obraz MMC/SC tylko do odczytu" },
/* ru */ { IDS_MMC64_IMAGE_READ_ONLY_RU, "MMC/SD Image read-only" },
/* sv */ { IDS_MMC64_IMAGE_READ_ONLY_SV, "MMC/SD-avbildning skrivskyddad" },
/* tr */ { IDS_MMC64_IMAGE_READ_ONLY_TR, "Salt Okunur MMC/SD Imajý" },

/* en */ { IDS_MMC64_IMAGE_FILE,    "MMC/SD Image File" },
/* da */ { IDS_MMC64_IMAGE_FILE_DA, "MMC/SD-image" },
/* de */ { IDS_MMC64_IMAGE_FILE_DE, "MMC/SD Image Name" },
/* es */ { IDS_MMC64_IMAGE_FILE_ES, "Fichero imagen MMC/SD" },
/* fr */ { IDS_MMC64_IMAGE_FILE_FR, "Nom d'image MMC/SD" },
/* hu */ { IDS_MMC64_IMAGE_FILE_HU, "MMC/SD képmás fájl" },
/* it */ { IDS_MMC64_IMAGE_FILE_IT, "File immagine MMC/SD" },
/* ko */ { IDS_MMC64_IMAGE_FILE_KO, "" },  /* fuzzy */
/* nl */ { IDS_MMC64_IMAGE_FILE_NL, "MMC/SD bestand" },
/* pl */ { IDS_MMC64_IMAGE_FILE_PL, "Plik obrazu MMC/SD" },
/* ru */ { IDS_MMC64_IMAGE_FILE_RU, "MMC/SD Image File" },
/* sv */ { IDS_MMC64_IMAGE_FILE_SV, "MMC/SD-avbildningsfil" },
/* tr */ { IDS_MMC64_IMAGE_FILE_TR, "MMC/SD Imaj Dosyasý" },

/* en */ { IDS_SD_TYPE,    "Card type" },
/* da */ { IDS_SD_TYPE_DA, "Korttype" },
/* de */ { IDS_SD_TYPE_DE, "Card Typ" },
/* es */ { IDS_SD_TYPE_ES, "Tipo de tarjeta" },
/* fr */ { IDS_SD_TYPE_FR, "Type de carte" },
/* hu */ { IDS_SD_TYPE_HU, "" },  /* fuzzy */
/* it */ { IDS_SD_TYPE_IT, "Tipo card" },
/* ko */ { IDS_SD_TYPE_KO, "Ä«µå Á¾·ù" },
/* nl */ { IDS_SD_TYPE_NL, "Card soort" },
/* pl */ { IDS_SD_TYPE_PL, "Typ karty" },
/* ru */ { IDS_SD_TYPE_RU, "" },  /* fuzzy */
/* sv */ { IDS_SD_TYPE_SV, "Korttyp" },
/* tr */ { IDS_SD_TYPE_TR, "Kart tipi" },

/* en */ { IDS_CLOCKPORT_DEVICE,    "Clockport device" },
/* da */ { IDS_CLOCKPORT_DEVICE_DA, "" },  /* fuzzy */
/* de */ { IDS_CLOCKPORT_DEVICE_DE, "Clockport Gerät" },
/* es */ { IDS_CLOCKPORT_DEVICE_ES, "" },  /* fuzzy */
/* fr */ { IDS_CLOCKPORT_DEVICE_FR, "" },  /* fuzzy */
/* hu */ { IDS_CLOCKPORT_DEVICE_HU, "" },  /* fuzzy */
/* it */ { IDS_CLOCKPORT_DEVICE_IT, "" },  /* fuzzy */
/* ko */ { IDS_CLOCKPORT_DEVICE_KO, "" },  /* fuzzy */
/* nl */ { IDS_CLOCKPORT_DEVICE_NL, "Clockport apparaat" },
/* pl */ { IDS_CLOCKPORT_DEVICE_PL, "" },  /* fuzzy */
/* ru */ { IDS_CLOCKPORT_DEVICE_RU, "" },  /* fuzzy */
/* sv */ { IDS_CLOCKPORT_DEVICE_SV, "" },  /* fuzzy */
/* tr */ { IDS_CLOCKPORT_DEVICE_TR, "" },  /* fuzzy */

/* en */ { IDS_SHORTBUS_DIGIMAX,    "Short bus DigiMAX device" },
/* da */ { IDS_SHORTBUS_DIGIMAX_DA, "" },  /* fuzzy */
/* de */ { IDS_SHORTBUS_DIGIMAX_DE, "Shortbus DigiMAX Gerät" },
/* es */ { IDS_SHORTBUS_DIGIMAX_ES, "" },  /* fuzzy */
/* fr */ { IDS_SHORTBUS_DIGIMAX_FR, "" },  /* fuzzy */
/* hu */ { IDS_SHORTBUS_DIGIMAX_HU, "" },  /* fuzzy */
/* it */ { IDS_SHORTBUS_DIGIMAX_IT, "" },  /* fuzzy */
/* ko */ { IDS_SHORTBUS_DIGIMAX_KO, "" },  /* fuzzy */
/* nl */ { IDS_SHORTBUS_DIGIMAX_NL, "Short Bus DigiMAX apparaat" },
/* pl */ { IDS_SHORTBUS_DIGIMAX_PL, "" },  /* fuzzy */
/* ru */ { IDS_SHORTBUS_DIGIMAX_RU, "" },  /* fuzzy */
/* sv */ { IDS_SHORTBUS_DIGIMAX_SV, "" },  /* fuzzy */
/* tr */ { IDS_SHORTBUS_DIGIMAX_TR, "" },  /* fuzzy */

/* en */ { IDS_SHORTBUS_DIGIMAX_BASE,    "Short bus DigiMAX base address" },
/* da */ { IDS_SHORTBUS_DIGIMAX_BASE_DA, "" },  /* fuzzy */
/* de */ { IDS_SHORTBUS_DIGIMAX_BASE_DE, "Shortbus DigiMAX Geräteadresse" },
/* es */ { IDS_SHORTBUS_DIGIMAX_BASE_ES, "" },  /* fuzzy */
/* fr */ { IDS_SHORTBUS_DIGIMAX_BASE_FR, "" },  /* fuzzy */
/* hu */ { IDS_SHORTBUS_DIGIMAX_BASE_HU, "" },  /* fuzzy */
/* it */ { IDS_SHORTBUS_DIGIMAX_BASE_IT, "" },  /* fuzzy */
/* ko */ { IDS_SHORTBUS_DIGIMAX_BASE_KO, "" },  /* fuzzy */
/* nl */ { IDS_SHORTBUS_DIGIMAX_BASE_NL, "Short Bus DigiMAX apparaat adres" },
/* pl */ { IDS_SHORTBUS_DIGIMAX_BASE_PL, "" },  /* fuzzy */
/* ru */ { IDS_SHORTBUS_DIGIMAX_BASE_RU, "" },  /* fuzzy */
/* sv */ { IDS_SHORTBUS_DIGIMAX_BASE_SV, "" },  /* fuzzy */
/* tr */ { IDS_SHORTBUS_DIGIMAX_BASE_TR, "" },  /* fuzzy */

/* en */ { IDS_SELECT_MMC64_IMAGE_FILE,    "Select MMC/SD image file" },
/* da */ { IDS_SELECT_MMC64_IMAGE_FILE_DA, "Vælg MMC/SD-image" },
/* de */ { IDS_SELECT_MMC64_IMAGE_FILE_DE, "MMC/SD Image Date wählen" },
/* es */ { IDS_SELECT_MMC64_IMAGE_FILE_ES, "Seleccionar fichero imagen MMC/SD" },
/* fr */ { IDS_SELECT_MMC64_IMAGE_FILE_FR, "Sélectionnez le fichier image du MMC/SD" },
/* hu */ { IDS_SELECT_MMC64_IMAGE_FILE_HU, "Válassza ki az MMC/SD képmás fájlt" },
/* it */ { IDS_SELECT_MMC64_IMAGE_FILE_IT, "Seleziona file immagine MMC/SD" },
/* ko */ { IDS_SELECT_MMC64_IMAGE_FILE_KO, "" },  /* fuzzy */
/* nl */ { IDS_SELECT_MMC64_IMAGE_FILE_NL, "Selecteer MMC/SD bestand" },
/* pl */ { IDS_SELECT_MMC64_IMAGE_FILE_PL, "Wybierz plik obrazu MMC/SD" },
/* ru */ { IDS_SELECT_MMC64_IMAGE_FILE_RU, "Select MMC/SD image file" },
/* sv */ { IDS_SELECT_MMC64_IMAGE_FILE_SV, "Ange MMC/SD-avbildningsfil" },
/* tr */ { IDS_SELECT_MMC64_IMAGE_FILE_TR, "MMC/SD imaj dosyasý seçin" },

/* en */ { IDS_MMC64_SETTINGS,    "MMC64 settings" },
/* da */ { IDS_MMC64_SETTINGS_DA, "MMC64-indstillinger" },
/* de */ { IDS_MMC64_SETTINGS_DE, "MMC64 Einstellungen" },
/* es */ { IDS_MMC64_SETTINGS_ES, "Ajustes MMC64" },
/* fr */ { IDS_MMC64_SETTINGS_FR, "Paramètres MMC64..." },
/* hu */ { IDS_MMC64_SETTINGS_HU, "MMC64 beállításai" },
/* it */ { IDS_MMC64_SETTINGS_IT, "Impostazioni MMC64" },
/* ko */ { IDS_MMC64_SETTINGS_KO, "MMC64 ¼³Á¤" },
/* nl */ { IDS_MMC64_SETTINGS_NL, "MMC64 instellingen" },
/* pl */ { IDS_MMC64_SETTINGS_PL, "Ustawienia MMC64" },
/* ru */ { IDS_MMC64_SETTINGS_RU, "MMC64 settings" },
/* sv */ { IDS_MMC64_SETTINGS_SV, "MMC64-inställningar..." },
/* tr */ { IDS_MMC64_SETTINGS_TR, "MMC64 ayarlarý" },

/* en */ { IDS_SELECT_EEPROM_FILE,    "Select EEPROM file" },
/* da */ { IDS_SELECT_EEPROM_FILE_DA, "Vælg EEPROM-fil" },
/* de */ { IDS_SELECT_EEPROM_FILE_DE, "EEPROM Datei wählen" },
/* es */ { IDS_SELECT_EEPROM_FILE_ES, "Seleccionar fichero EEPROM" },
/* fr */ { IDS_SELECT_EEPROM_FILE_FR, "Sélectionner le fichier EEPROM" },
/* hu */ { IDS_SELECT_EEPROM_FILE_HU, "" },  /* fuzzy */
/* it */ { IDS_SELECT_EEPROM_FILE_IT, "Seleziona file EEPROM" },
/* ko */ { IDS_SELECT_EEPROM_FILE_KO, "" },  /* fuzzy */
/* nl */ { IDS_SELECT_EEPROM_FILE_NL, "Selecteer EEPROM bestand" },
/* pl */ { IDS_SELECT_EEPROM_FILE_PL, "Wybierz plik EEPROM" },
/* ru */ { IDS_SELECT_EEPROM_FILE_RU, "Select EEPROM file" },
/* sv */ { IDS_SELECT_EEPROM_FILE_SV, "Välj EEPROM-fil" },
/* tr */ { IDS_SELECT_EEPROM_FILE_TR, "EEPROM dosyasý seçin" },

/* en */ { IDS_SELECT_CARD_IMAGE_FILE,    "Select card image file" },
/* da */ { IDS_SELECT_CARD_IMAGE_FILE_DA, "Angiv kortimagefil" },
/* de */ { IDS_SELECT_CARD_IMAGE_FILE_DE, "Card Imagedatei auswählen" },
/* es */ { IDS_SELECT_CARD_IMAGE_FILE_ES, "Seleccionar fichero imagen tarjeta" },
/* fr */ { IDS_SELECT_CARD_IMAGE_FILE_FR, "Sélectionnez le fichier image carte" },
/* hu */ { IDS_SELECT_CARD_IMAGE_FILE_HU, "" },  /* fuzzy */
/* it */ { IDS_SELECT_CARD_IMAGE_FILE_IT, "Seleziona file immagine card" },
/* ko */ { IDS_SELECT_CARD_IMAGE_FILE_KO, "" },  /* fuzzy */
/* nl */ { IDS_SELECT_CARD_IMAGE_FILE_NL, "Selecteer card bestand" },
/* pl */ { IDS_SELECT_CARD_IMAGE_FILE_PL, "Wybierz plik obrazu karty" },
/* ru */ { IDS_SELECT_CARD_IMAGE_FILE_RU, "Select card image file" },
/* sv */ { IDS_SELECT_CARD_IMAGE_FILE_SV, "Ange kortavbildningsfil" },
/* tr */ { IDS_SELECT_CARD_IMAGE_FILE_TR, "Kart imaj dosyasý seç" },

/* en */ { IDS_CARD_FILE,    "Card image file" },
/* da */ { IDS_CARD_FILE_DA, "Kortimagefil" },
/* de */ { IDS_CARD_FILE_DE, "Card Imagedatei" },
/* es */ { IDS_CARD_FILE_ES, "Fichero imagen tarjeta" },
/* fr */ { IDS_CARD_FILE_FR, "Fichier image de carte" },
/* hu */ { IDS_CARD_FILE_HU, "" },  /* fuzzy */
/* it */ { IDS_CARD_FILE_IT, "File immagine Card" },
/* ko */ { IDS_CARD_FILE_KO, "Ä«µå ÀÌ¹ÌÁö ÆÄÀÏ" },
/* nl */ { IDS_CARD_FILE_NL, "Card bestand" },
/* pl */ { IDS_CARD_FILE_PL, "Plik obrazu karty" },
/* ru */ { IDS_CARD_FILE_RU, "" },  /* fuzzy */
/* sv */ { IDS_CARD_FILE_SV, "Kortavbildningsfil" },
/* tr */ { IDS_CARD_FILE_TR, "Kart imaj dosyasý" },

/* en */ { IDS_CARD_READ_WRITE,    "Card read/write" },
/* da */ { IDS_CARD_READ_WRITE_DA, "Kort læs/skriv" },
/* de */ { IDS_CARD_READ_WRITE_DE, "Card lese/schreibe" },
/* es */ { IDS_CARD_READ_WRITE_ES, "Tarjeta lectura/grabación" },
/* fr */ { IDS_CARD_READ_WRITE_FR, "Lecture/Écriture Carte" },
/* hu */ { IDS_CARD_READ_WRITE_HU, "" },  /* fuzzy */
/* it */ { IDS_CARD_READ_WRITE_IT, "Card in lettura/scrittura" },
/* ko */ { IDS_CARD_READ_WRITE_KO, "" },  /* fuzzy */
/* nl */ { IDS_CARD_READ_WRITE_NL, "Card lees/schrijf" },
/* pl */ { IDS_CARD_READ_WRITE_PL, "odczyt/zapis na kartê" },
/* ru */ { IDS_CARD_READ_WRITE_RU, "" },  /* fuzzy */
/* sv */ { IDS_CARD_READ_WRITE_SV, "Skrivbart kort" },
/* tr */ { IDS_CARD_READ_WRITE_TR, "Kart oku/yaz" },

/* en */ { IDS_EEPROM_FILE,    "EEPROM image file" },
/* da */ { IDS_EEPROM_FILE_DA, "EEPROM image-fil" },
/* de */ { IDS_EEPROM_FILE_DE, "EEPROM Imagedatei" },
/* es */ { IDS_EEPROM_FILE_ES, "Fichero imagen EEPROM" },
/* fr */ { IDS_EEPROM_FILE_FR, "Nom d'image EEPROM" },
/* hu */ { IDS_EEPROM_FILE_HU, "" },  /* fuzzy */
/* it */ { IDS_EEPROM_FILE_IT, "File immagine EEPROM" },
/* ko */ { IDS_EEPROM_FILE_KO, "EEPROM ÀÌ¹ÌÁö ÆÄÀÏ" },
/* nl */ { IDS_EEPROM_FILE_NL, "EEPROM bestand" },
/* pl */ { IDS_EEPROM_FILE_PL, "Plik obrazu EEPROM" },
/* ru */ { IDS_EEPROM_FILE_RU, "" },  /* fuzzy */
/* sv */ { IDS_EEPROM_FILE_SV, "EEPROM-avbildningsfil" },
/* tr */ { IDS_EEPROM_FILE_TR, "EEPROM imaj dosyasý" },

/* en */ { IDS_WRITE_TO_EEPROM_WHEN_CHANGED,    "Save EEPROM image when changed" },
/* da */ { IDS_WRITE_TO_EEPROM_WHEN_CHANGED_DA, "Gem EEPROM image ved ændringer" },
/* de */ { IDS_WRITE_TO_EEPROM_WHEN_CHANGED_DE, "GEO-RAM Imagedatei bei Änderung speichern" },
/* es */ { IDS_WRITE_TO_EEPROM_WHEN_CHANGED_ES, "Grabar imagen EEPROM cuando cambie" },
/* fr */ { IDS_WRITE_TO_EEPROM_WHEN_CHANGED_FR, "Enregistrer l'image EEPROM lorsque modifiée" },
/* hu */ { IDS_WRITE_TO_EEPROM_WHEN_CHANGED_HU, "" },  /* fuzzy */
/* it */ { IDS_WRITE_TO_EEPROM_WHEN_CHANGED_IT, "Salva immagine EEPROM alla modifica" },
/* ko */ { IDS_WRITE_TO_EEPROM_WHEN_CHANGED_KO, "" },  /* fuzzy */
/* nl */ { IDS_WRITE_TO_EEPROM_WHEN_CHANGED_NL, "GEO-RAM bestand opslaan indien gewijzigd" },
/* pl */ { IDS_WRITE_TO_EEPROM_WHEN_CHANGED_PL, "Zapisuj obraz EEPROM przy zmianie" },
/* ru */ { IDS_WRITE_TO_EEPROM_WHEN_CHANGED_RU, "Save EEPROM image when changed" },
/* sv */ { IDS_WRITE_TO_EEPROM_WHEN_CHANGED_SV, "Spara EEPROM-avbildning vid ändringar" },
/* tr */ { IDS_WRITE_TO_EEPROM_WHEN_CHANGED_TR, "EEPROM imajý deðiþtiðinde kaydet" },

/* en */ { IDS_EEPROM_READ_WRITE,    "EEPROM read/write" },
/* da */ { IDS_EEPROM_READ_WRITE_DA, "EEPROM læs/skriv" },
/* de */ { IDS_EEPROM_READ_WRITE_DE, "EEPROM lese/schreibe" },
/* es */ { IDS_EEPROM_READ_WRITE_ES, "Lectura/grabación EEPROM" },
/* fr */ { IDS_EEPROM_READ_WRITE_FR, "Lecture/Écriture EEPROM" },
/* hu */ { IDS_EEPROM_READ_WRITE_HU, "" },  /* fuzzy */
/* it */ { IDS_EEPROM_READ_WRITE_IT, "EEPROM in lettura/scrittura" },
/* ko */ { IDS_EEPROM_READ_WRITE_KO, "" },  /* fuzzy */
/* nl */ { IDS_EEPROM_READ_WRITE_NL, "EEPROM lees/schrijf" },
/* pl */ { IDS_EEPROM_READ_WRITE_PL, "odczyt/zapis EEPROM" },
/* ru */ { IDS_EEPROM_READ_WRITE_RU, "EEPROM read/write" },
/* sv */ { IDS_EEPROM_READ_WRITE_SV, "Skrivbar EEPROM" },
/* tr */ { IDS_EEPROM_READ_WRITE_TR, "EEPROM oku/yaz" },

/* en */ { IDS_RESCUE_MODE,    "Rescue mode" },
/* da */ { IDS_RESCUE_MODE_DA, "Fejlsikret tilstand" },
/* de */ { IDS_RESCUE_MODE_DE, "Rescue Modus" },
/* es */ { IDS_RESCUE_MODE_ES, "Modo rescate" },
/* fr */ { IDS_RESCUE_MODE_FR, "Mode de secours" },
/* hu */ { IDS_RESCUE_MODE_HU, "" },  /* fuzzy */
/* it */ { IDS_RESCUE_MODE_IT, "Modalità ripristino" },
/* ko */ { IDS_RESCUE_MODE_KO, "" },  /* fuzzy */
/* nl */ { IDS_RESCUE_MODE_NL, "Reddingsmodus" },
/* pl */ { IDS_RESCUE_MODE_PL, "Tryb ratunkowy" },
/* ru */ { IDS_RESCUE_MODE_RU, "Rescue mode" },
/* sv */ { IDS_RESCUE_MODE_SV, "Räddningsläge" },
/* tr */ { IDS_RESCUE_MODE_TR, "Kurtarma modu" },

/* en */ { IDS_GMOD2_SETTINGS,    "GMod2 settings" },
/* da */ { IDS_GMOD2_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDS_GMOD2_SETTINGS_DE, "GMod2 Einstellungen" },
/* es */ { IDS_GMOD2_SETTINGS_ES, "" },  /* fuzzy */
/* fr */ { IDS_GMOD2_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDS_GMOD2_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_GMOD2_SETTINGS_IT, "" },  /* fuzzy */
/* ko */ { IDS_GMOD2_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_GMOD2_SETTINGS_NL, "GMod2 instellingen" },
/* pl */ { IDS_GMOD2_SETTINGS_PL, "" },  /* fuzzy */
/* ru */ { IDS_GMOD2_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_GMOD2_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDS_GMOD2_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDS_MMCREPLAY_SETTINGS,    "MMC Replay settings" },
/* da */ { IDS_MMCREPLAY_SETTINGS_DA, "MMC Replay-indstillinger" },
/* de */ { IDS_MMCREPLAY_SETTINGS_DE, "MMC Replay Einstellungen" },
/* es */ { IDS_MMCREPLAY_SETTINGS_ES, "Ajustes MMC Replay" },
/* fr */ { IDS_MMCREPLAY_SETTINGS_FR, "Paramètres MMC Replay" },
/* hu */ { IDS_MMCREPLAY_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_MMCREPLAY_SETTINGS_IT, "Impostazioni MMC Replay" },
/* ko */ { IDS_MMCREPLAY_SETTINGS_KO, "MMC Replay ¼³Á¤" },
/* nl */ { IDS_MMCREPLAY_SETTINGS_NL, "MMC Replay instellingen" },
/* pl */ { IDS_MMCREPLAY_SETTINGS_PL, "Ustawienia MMC Replay" },
/* ru */ { IDS_MMCREPLAY_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_MMCREPLAY_SETTINGS_SV, "MMC Replay-inställningar" },
/* tr */ { IDS_MMCREPLAY_SETTINGS_TR, "MMC Replay ayarlarý" },

/* en */ { IDS_RETROREPLAY_SETTINGS,    "Retro Replay settings" },
/* da */ { IDS_RETROREPLAY_SETTINGS_DA, "Retro Replay-indstillinger" },
/* de */ { IDS_RETROREPLAY_SETTINGS_DE, "Retro Replay Einstellungen" },
/* es */ { IDS_RETROREPLAY_SETTINGS_ES, "Ajustes Retro Replay" },
/* fr */ { IDS_RETROREPLAY_SETTINGS_FR, "Paramètres Retro Replay" },
/* hu */ { IDS_RETROREPLAY_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_RETROREPLAY_SETTINGS_IT, "Impostazioni Retro Replay" },
/* ko */ { IDS_RETROREPLAY_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_RETROREPLAY_SETTINGS_NL, "Retro Replay instellingen" },
/* pl */ { IDS_RETROREPLAY_SETTINGS_PL, "Ustawienia Retro Replay" },
/* ru */ { IDS_RETROREPLAY_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_RETROREPLAY_SETTINGS_SV, "Retro Replay-inställningar" },
/* tr */ { IDS_RETROREPLAY_SETTINGS_TR, "Retro Replay ayarlarý" },

/* en */ { IDS_SELECT_GENERIC,    "Select generic cartridge image" },
/* da */ { IDS_SELECT_GENERIC_DA, "Vælg standardcartridgeimage" },
/* de */ { IDS_SELECT_GENERIC_DE, "Unverselles Erweiterungsmodul Imagedatei wählen" },
/* es */ { IDS_SELECT_GENERIC_ES, "Seleccionar imagen cartucho generico" },
/* fr */ { IDS_SELECT_GENERIC_FR, "Sélectionner l'image de cartouche générique" },
/* hu */ { IDS_SELECT_GENERIC_HU, "" },  /* fuzzy */
/* it */ { IDS_SELECT_GENERIC_IT, "Seleziona immagine cartuccia generica" },
/* ko */ { IDS_SELECT_GENERIC_KO, "" },  /* fuzzy */
/* nl */ { IDS_SELECT_GENERIC_NL, "Selecteer algemeen cartridge bestand" },
/* pl */ { IDS_SELECT_GENERIC_PL, "Wybierz typowy obraz kartrid¿a" },
/* ru */ { IDS_SELECT_GENERIC_RU, "Select generic cartridge image" },
/* sv */ { IDS_SELECT_GENERIC_SV, "Välj generisk insticksmodulavbildning" },
/* tr */ { IDS_SELECT_GENERIC_TR, "Jenerik kartuþ imajý seç" },

/* en */ { IDS_SELECT_UM,    "Select UltiMem image" },
/* da */ { IDS_SELECT_UM_DA, "" },  /* fuzzy */
/* de */ { IDS_SELECT_UM_DE, "UltiMem Image wählen" },
/* es */ { IDS_SELECT_UM_ES, "" },  /* fuzzy */
/* fr */ { IDS_SELECT_UM_FR, "" },  /* fuzzy */
/* hu */ { IDS_SELECT_UM_HU, "" },  /* fuzzy */
/* it */ { IDS_SELECT_UM_IT, "" },  /* fuzzy */
/* ko */ { IDS_SELECT_UM_KO, "" },  /* fuzzy */
/* nl */ { IDS_SELECT_UM_NL, "Selecteer UltiMem bestand" },
/* pl */ { IDS_SELECT_UM_PL, "" },  /* fuzzy */
/* ru */ { IDS_SELECT_UM_RU, "" },  /* fuzzy */
/* sv */ { IDS_SELECT_UM_SV, "" },  /* fuzzy */
/* tr */ { IDS_SELECT_UM_TR, "" },  /* fuzzy */

/* en */ { IDS_SELECT_FP,    "Select Vic Flash Plugin image" },
/* da */ { IDS_SELECT_FP_DA, "Vælg Vic Flash Plugin-image" },
/* de */ { IDS_SELECT_FP_DE, "Vic Flash Plugin Image auswählen" },
/* es */ { IDS_SELECT_FP_ES, "Seleccionar imagen Vic Flash Plugin" },
/* fr */ { IDS_SELECT_FP_FR, "Sélectionner l'image Vic Flash Plugin" },
/* hu */ { IDS_SELECT_FP_HU, "" },  /* fuzzy */
/* it */ { IDS_SELECT_FP_IT, "Seleziona immagine Vic Flash Plugin " },
/* ko */ { IDS_SELECT_FP_KO, "" },  /* fuzzy */
/* nl */ { IDS_SELECT_FP_NL, "Selecteer Vic Flash Plugin bestand" },
/* pl */ { IDS_SELECT_FP_PL, "Wybierz obraz Vic Flash Plugin" },
/* ru */ { IDS_SELECT_FP_RU, "Select Vic Flash Plugin image" },
/* sv */ { IDS_SELECT_FP_SV, "Välj avbildning för Vic Flash-insticksmodul" },
/* tr */ { IDS_SELECT_FP_TR, "Vic Flash Plugin imajý seç" },

/* en */ { IDS_SELECT_BEHR_BONZ,    "Select Behr Bonz image" },
/* da */ { IDS_SELECT_BEHR_BONZ_DA, "" },  /* fuzzy */
/* de */ { IDS_SELECT_BEHR_BONZ_DE, "Behr Bonz Imagedatei wählen" },
/* es */ { IDS_SELECT_BEHR_BONZ_ES, "" },  /* fuzzy */
/* fr */ { IDS_SELECT_BEHR_BONZ_FR, "" },  /* fuzzy */
/* hu */ { IDS_SELECT_BEHR_BONZ_HU, "" },  /* fuzzy */
/* it */ { IDS_SELECT_BEHR_BONZ_IT, "" },  /* fuzzy */
/* ko */ { IDS_SELECT_BEHR_BONZ_KO, "" },  /* fuzzy */
/* nl */ { IDS_SELECT_BEHR_BONZ_NL, "Selecteer Behr Bonz bestand" },
/* pl */ { IDS_SELECT_BEHR_BONZ_PL, "" },  /* fuzzy */
/* ru */ { IDS_SELECT_BEHR_BONZ_RU, "" },  /* fuzzy */
/* sv */ { IDS_SELECT_BEHR_BONZ_SV, "" },  /* fuzzy */
/* tr */ { IDS_SELECT_BEHR_BONZ_TR, "" },  /* fuzzy */

/* en */ { IDS_SELECT_MEGACART,    "Select Mega-Cart image" },
/* da */ { IDS_SELECT_MEGACART_DA, "Vælg MegaCart-imagefil" },
/* de */ { IDS_SELECT_MEGACART_DE, "Mega-Cart Image Date wählen" },
/* es */ { IDS_SELECT_MEGACART_ES, "Seleccionar imagen Mega-Cart" },
/* fr */ { IDS_SELECT_MEGACART_FR, "Sélectionner l'image Mega-Cart" },
/* hu */ { IDS_SELECT_MEGACART_HU, "Válassza ki a Mega-Cart képmást" },
/* it */ { IDS_SELECT_MEGACART_IT, "Seleziona immagine Mega-Cart" },
/* ko */ { IDS_SELECT_MEGACART_KO, "" },  /* fuzzy */
/* nl */ { IDS_SELECT_MEGACART_NL, "Selecteer Mega-Cart bestand" },
/* pl */ { IDS_SELECT_MEGACART_PL, "Wybierz obraz Mega-Cart" },
/* ru */ { IDS_SELECT_MEGACART_RU, "Select Mega-Cart image" },
/* sv */ { IDS_SELECT_MEGACART_SV, "Välj Mega-Cart-modulfil" },
/* tr */ { IDS_SELECT_MEGACART_TR, "Mega-Cart imajý seç" },

/* en */ { IDS_SELECT_FINAL_EXPANSION,    "Select Final Expansion image" },
/* da */ { IDS_SELECT_FINAL_EXPANSION_DA, "Vælg Final Expansion-imagefil" },
/* de */ { IDS_SELECT_FINAL_EXPANSION_DE, "Final Expansion Imagedatei auswählen" },
/* es */ { IDS_SELECT_FINAL_EXPANSION_ES, "Seleccionar imagen Final Expansion" },
/* fr */ { IDS_SELECT_FINAL_EXPANSION_FR, "Sélectionner l'image ROM Final Expansion" },
/* hu */ { IDS_SELECT_FINAL_EXPANSION_HU, "Válassza ki a Final Expansion képmást" },
/* it */ { IDS_SELECT_FINAL_EXPANSION_IT, "Seleziona immagine Final Expansion" },
/* ko */ { IDS_SELECT_FINAL_EXPANSION_KO, "" },  /* fuzzy */
/* nl */ { IDS_SELECT_FINAL_EXPANSION_NL, "Selecteer Final Expansion bestand" },
/* pl */ { IDS_SELECT_FINAL_EXPANSION_PL, "Wybierz obraz Final Expansion" },
/* ru */ { IDS_SELECT_FINAL_EXPANSION_RU, "Select Final Expansion image" },
/* sv */ { IDS_SELECT_FINAL_EXPANSION_SV, "Välj avbildning för Final Expansion" },
/* tr */ { IDS_SELECT_FINAL_EXPANSION_TR, "Final Expansion imajý seç" },

/* en */ { IDS_SELECT_CARTRIDGE_IMAGE,    "Select cartridge file" },
/* da */ { IDS_SELECT_CARTRIDGE_IMAGE_DA, "Vælg cartridge-fil" },
/* de */ { IDS_SELECT_CARTRIDGE_IMAGE_DE, "Erweiterungsmodul Imagedatei wählen" },
/* es */ { IDS_SELECT_CARTRIDGE_IMAGE_ES, "Seleccionar imagen cartucho" },
/* fr */ { IDS_SELECT_CARTRIDGE_IMAGE_FR, "Sélection le fichier de cartouche" },
/* hu */ { IDS_SELECT_CARTRIDGE_IMAGE_HU, "Cartridge fájl kiválasztása" },
/* it */ { IDS_SELECT_CARTRIDGE_IMAGE_IT, "Seleziona file cartuccia" },
/* ko */ { IDS_SELECT_CARTRIDGE_IMAGE_KO, "" },  /* fuzzy */
/* nl */ { IDS_SELECT_CARTRIDGE_IMAGE_NL, "Selecteer cartridge bestand" },
/* pl */ { IDS_SELECT_CARTRIDGE_IMAGE_PL, "Wybierz plik kartrid¿a" },
/* ru */ { IDS_SELECT_CARTRIDGE_IMAGE_RU, "²ëÑÕàØâÕ äÐÙÛ ÚÐàâàØÔÖÐ" },
/* sv */ { IDS_SELECT_CARTRIDGE_IMAGE_SV, "Välj insticksmodulfil" },
/* tr */ { IDS_SELECT_CARTRIDGE_IMAGE_TR, "Kartuþ dosyasýný seç" },

/* en */ { IDS_SID_CART,    "SID cartridge" },
/* da */ { IDS_SID_CART_DA, "SID-cartridge" },
/* de */ { IDS_SID_CART_DE, "SID Modul" },
/* es */ { IDS_SID_CART_ES, "Cartucho SID" },
/* fr */ { IDS_SID_CART_FR, "Cartouche SID" },
/* hu */ { IDS_SID_CART_HU, "SID cartridge" },
/* it */ { IDS_SID_CART_IT, "Cartuccia SID" },
/* ko */ { IDS_SID_CART_KO, "SID Ä«Æ®¸®Áö" },
/* nl */ { IDS_SID_CART_NL, "SID-cartridge" },
/* pl */ { IDS_SID_CART_PL, "Kartrid¿ SID" },
/* ru */ { IDS_SID_CART_RU, "ºÐàâàØÔÖ SID" },
/* sv */ { IDS_SID_CART_SV, "SID-insticksmodul" },
/* tr */ { IDS_SID_CART_TR, "SID kartuþu" },

/* en */ { IDS_SID_ENGINE_MODEL,    "SID engine and model" },
/* da */ { IDS_SID_ENGINE_MODEL_DA, "SID-motor og -model" },
/* de */ { IDS_SID_ENGINE_MODEL_DE, "SID Engine und Modell" },
/* es */ { IDS_SID_ENGINE_MODEL_ES, "Modelo y motor SID" },
/* fr */ { IDS_SID_ENGINE_MODEL_FR, "Engin SID et modèle" },
/* hu */ { IDS_SID_ENGINE_MODEL_HU, "" },  /* fuzzy */
/* it */ { IDS_SID_ENGINE_MODEL_IT, "Motore e modello  SID" },
/* ko */ { IDS_SID_ENGINE_MODEL_KO, "" },  /* fuzzy */
/* nl */ { IDS_SID_ENGINE_MODEL_NL, "SID kern en model" },
/* pl */ { IDS_SID_ENGINE_MODEL_PL, "silnik i model SID" },
/* ru */ { IDS_SID_ENGINE_MODEL_RU, "" },  /* fuzzy */
/* sv */ { IDS_SID_ENGINE_MODEL_SV, "SID-motor och -modell" },
/* tr */ { IDS_SID_ENGINE_MODEL_TR, "SID motoru ve modeli" },

/* en */ { IDS_SID_ENGINE,    "SID engine" },
/* da */ { IDS_SID_ENGINE_DA, "SID-motor" },
/* de */ { IDS_SID_ENGINE_DE, "SID Engine" },
/* es */ { IDS_SID_ENGINE_ES, "Motor SID" },
/* fr */ { IDS_SID_ENGINE_FR, "Engin SID" },
/* hu */ { IDS_SID_ENGINE_HU, "" },  /* fuzzy */
/* it */ { IDS_SID_ENGINE_IT, "Motore SID" },
/* ko */ { IDS_SID_ENGINE_KO, "" },  /* fuzzy */
/* nl */ { IDS_SID_ENGINE_NL, "SID kern" },
/* pl */ { IDS_SID_ENGINE_PL, "Silnik SID" },
/* ru */ { IDS_SID_ENGINE_RU, "" },  /* fuzzy */
/* sv */ { IDS_SID_ENGINE_SV, "SID-motor" },
/* tr */ { IDS_SID_ENGINE_TR, "SID motoru" },

/* en */ { IDS_SID_MODEL,    "SID model" },
/* da */ { IDS_SID_MODEL_DA, "SID-model" },
/* de */ { IDS_SID_MODEL_DE, "SID Modell" },
/* es */ { IDS_SID_MODEL_ES, "Modelo SID" },
/* fr */ { IDS_SID_MODEL_FR, "Modèle SID" },
/* hu */ { IDS_SID_MODEL_HU, "SID modell" },
/* it */ { IDS_SID_MODEL_IT, "Modello SID" },
/* ko */ { IDS_SID_MODEL_KO, "SID ¸ðµ¨" },
/* nl */ { IDS_SID_MODEL_NL, "SID model" },
/* pl */ { IDS_SID_MODEL_PL, "Model SID-a" },
/* ru */ { IDS_SID_MODEL_RU, "ÜÞÔÕÛì ×ÒãÚÞÓÕÝÕàÐâÞàÐ SID" },
/* sv */ { IDS_SID_MODEL_SV, "SID-modell" },
/* tr */ { IDS_SID_MODEL_TR, "SID modeli" },

/* en */ { IDS_SID_ADDRESS,    "SID address" },
/* da */ { IDS_SID_ADDRESS_DA, "SID-adresse" },
/* de */ { IDS_SID_ADDRESS_DE, "SID Adresse" },
/* es */ { IDS_SID_ADDRESS_ES, "Dirección SID" },
/* fr */ { IDS_SID_ADDRESS_FR, "Adresse SID" },
/* hu */ { IDS_SID_ADDRESS_HU, "SID cím" },
/* it */ { IDS_SID_ADDRESS_IT, "Indirizzo SID" },
/* ko */ { IDS_SID_ADDRESS_KO, "SID ÁÖ¼Ò" },
/* nl */ { IDS_SID_ADDRESS_NL, "SID-adres" },
/* pl */ { IDS_SID_ADDRESS_PL, "Adres SID" },
/* ru */ { IDS_SID_ADDRESS_RU, "°ÔàÕá SID" },
/* sv */ { IDS_SID_ADDRESS_SV, "SID-adress" },
/* tr */ { IDS_SID_ADDRESS_TR, "SID adresi" },

/* en */ { IDS_SID_CLOCK,    "SID clock" },
/* da */ { IDS_SID_CLOCK_DA, "SID-klok" },
/* de */ { IDS_SID_CLOCK_DE, "SID Takt" },
/* es */ { IDS_SID_CLOCK_ES, "Reloj SID" },
/* fr */ { IDS_SID_CLOCK_FR, "Horloge SID" },
/* hu */ { IDS_SID_CLOCK_HU, "SID órajel" },
/* it */ { IDS_SID_CLOCK_IT, "Clock SID" },
/* ko */ { IDS_SID_CLOCK_KO, "SID Å¬¶ô" },
/* nl */ { IDS_SID_CLOCK_NL, "SID clock" },
/* pl */ { IDS_SID_CLOCK_PL, "Zegar SID" },
/* ru */ { IDS_SID_CLOCK_RU, "SID ÒàÕÜï" },
/* sv */ { IDS_SID_CLOCK_SV, "SID-klocka" },
/* tr */ { IDS_SID_CLOCK_TR, "SID saati" },

/* en */ { IDS_SID_DIGIBLASTER,    "Digiblaster add-on" },
/* da */ { IDS_SID_DIGIBLASTER_DA, "Digiblaster udviddelse" },
/* de */ { IDS_SID_DIGIBLASTER_DE, "Digiblaster add-on" },
/* es */ { IDS_SID_DIGIBLASTER_ES, "Añadir Digiblaster" },
/* fr */ { IDS_SID_DIGIBLASTER_FR, "Add-on digiblaster" },
/* hu */ { IDS_SID_DIGIBLASTER_HU, "Digiblaster kiterjesztés" },
/* it */ { IDS_SID_DIGIBLASTER_IT, "Estensione digiblaster" },
/* ko */ { IDS_SID_DIGIBLASTER_KO, "Digiblaster ¿¡µå ¿Â" },
/* nl */ { IDS_SID_DIGIBLASTER_NL, "Digiblaster add-on" },
/* pl */ { IDS_SID_DIGIBLASTER_PL, "Dodatek digiblaster" },
/* ru */ { IDS_SID_DIGIBLASTER_RU, "Digiblaster add-on" },
/* sv */ { IDS_SID_DIGIBLASTER_SV, "Digiblaster-tillägg" },
/* tr */ { IDS_SID_DIGIBLASTER_TR, "Digiblaster eklentisi" },

/* en */ { IDS_SIDCART_SETTINGS,    "SID cart settings" },
/* da */ { IDS_SIDCART_SETTINGS_DA, "SID-cartridgeindstillinger" },
/* de */ { IDS_SIDCART_SETTINGS_DE, "SID Modul Einstellungen" },
/* es */ { IDS_SIDCART_SETTINGS_ES, "Ajustes SID Cart" },
/* fr */ { IDS_SIDCART_SETTINGS_FR, "Paramètres SID Cart" },
/* hu */ { IDS_SIDCART_SETTINGS_HU, "SID kártya beállításai..." },
/* it */ { IDS_SIDCART_SETTINGS_IT, "Impostazioni cartuccia SID" },
/* ko */ { IDS_SIDCART_SETTINGS_KO, "SID Ä«Æ® ¼³Á¤" },
/* nl */ { IDS_SIDCART_SETTINGS_NL, "SID cart instellingen" },
/* pl */ { IDS_SIDCART_SETTINGS_PL, "Ustawienia karty SID" },
/* ru */ { IDS_SIDCART_SETTINGS_RU, "SID cart settings" },
/* sv */ { IDS_SIDCART_SETTINGS_SV, "SID-modulinställningarg" },
/* tr */ { IDS_SIDCART_SETTINGS_TR, "SID kartuþu ayarlarý" },

/* en */ { IDS_LOAD_FLIP_LIST_FILE,    "Load flip list file" },
/* da */ { IDS_LOAD_FLIP_LIST_FILE_DA, "Indlæs bladrelistefil" },
/* de */ { IDS_LOAD_FLIP_LIST_FILE_DE, "Fliplist Datei laden" },
/* es */ { IDS_LOAD_FLIP_LIST_FILE_ES, "Cargar lista de ficheros a conmutar" },
/* fr */ { IDS_LOAD_FLIP_LIST_FILE_FR, "Charger un fichier de groupement de disques" },
/* hu */ { IDS_LOAD_FLIP_LIST_FILE_HU, "Lemezlista betöltése" },
/* it */ { IDS_LOAD_FLIP_LIST_FILE_IT, "Carica file fliplist" },
/* ko */ { IDS_LOAD_FLIP_LIST_FILE_KO, "ÇÃ¸³ ¸®½ºÆ® ÆÄÀÏ ºÒ·¯¿À±â" },
/* nl */ { IDS_LOAD_FLIP_LIST_FILE_NL, "Laad fliplijstbestand" },
/* pl */ { IDS_LOAD_FLIP_LIST_FILE_PL, "Wczytaj plik listy podmiany no¶ników" },
/* ru */ { IDS_LOAD_FLIP_LIST_FILE_RU, "Load flip list file" },
/* sv */ { IDS_LOAD_FLIP_LIST_FILE_SV, "Läs vallistefil" },
/* tr */ { IDS_LOAD_FLIP_LIST_FILE_TR, "Sýralama listesi dosyasýný yükle" },

/* en */ { IDS_CANNOT_READ_FLIP_LIST,    "Cannot read flip list file" },
/* da */ { IDS_CANNOT_READ_FLIP_LIST_DA, "Kunne ikke læse bladrelistefil" },
/* de */ { IDS_CANNOT_READ_FLIP_LIST_DE, "Lesen von Fliplist Datei fehlgeschlagen" },
/* es */ { IDS_CANNOT_READ_FLIP_LIST_ES, "No puedo leer fichero de lista de intercambio" },
/* fr */ { IDS_CANNOT_READ_FLIP_LIST_FR, "Impossible de lire le fichier de groupement" },
/* hu */ { IDS_CANNOT_READ_FLIP_LIST_HU, "Nem lehet olvasni a lemezlista fájlt" },
/* it */ { IDS_CANNOT_READ_FLIP_LIST_IT, "Non è possibile caricare il file della flip list" },
/* ko */ { IDS_CANNOT_READ_FLIP_LIST_KO, "ÇÃ¸³ ¸®½ºÆ® ÆÄÀÏÀ» ÀÐÀ»¼ö ¾ø½À´Ï´Ù" },
/* nl */ { IDS_CANNOT_READ_FLIP_LIST_NL, "Kan het fliplijstbestand niet lezen" },
/* pl */ { IDS_CANNOT_READ_FLIP_LIST_PL, "Nie mo¿na odczytaæ pliku listy podmiany no¶ników" },
/* ru */ { IDS_CANNOT_READ_FLIP_LIST_RU, "Cannot read flip list file" },
/* sv */ { IDS_CANNOT_READ_FLIP_LIST_SV, "Kunde inte läsa vallistefil" },
/* tr */ { IDS_CANNOT_READ_FLIP_LIST_TR, "Sýralama listesi dosyasý okunamadý" },

/* en */ { IDS_SAVE_FLIP_LIST_FILE,    "Save flip list file" },
/* da */ { IDS_SAVE_FLIP_LIST_FILE_DA, "Gem bladrelistefil" },
/* de */ { IDS_SAVE_FLIP_LIST_FILE_DE, "Fliplist Datei speichern" },
/* es */ { IDS_SAVE_FLIP_LIST_FILE_ES, "Grabar lista de ficheros a conmutar" },
/* fr */ { IDS_SAVE_FLIP_LIST_FILE_FR, "Enregistrer le fichier de groupement de disques actuel" },
/* hu */ { IDS_SAVE_FLIP_LIST_FILE_HU, "Lemezlista mentése fájlba" },
/* it */ { IDS_SAVE_FLIP_LIST_FILE_IT, "Salva file fliplist" },
/* ko */ { IDS_SAVE_FLIP_LIST_FILE_KO, "ÇÃ¸³ ¸®½ºÆ® ÆÄÀÏ ÀúÀåÇÏ±â" },
/* nl */ { IDS_SAVE_FLIP_LIST_FILE_NL, "Fliplijstbestand opslaan" },
/* pl */ { IDS_SAVE_FLIP_LIST_FILE_PL, "Zapisz plik listy podmiany no¶ników" },
/* ru */ { IDS_SAVE_FLIP_LIST_FILE_RU, "Save flip list file" },
/* sv */ { IDS_SAVE_FLIP_LIST_FILE_SV, "Spara vallistefil" },
/* tr */ { IDS_SAVE_FLIP_LIST_FILE_TR, "Sýralama listesi dosyasýný kaydet" },

/* en */ { IDS_CANNOT_WRITE_FLIP_LIST,    "Cannot write flip list file" },
/* da */ { IDS_CANNOT_WRITE_FLIP_LIST_DA, "Kunne ikke skrive bladrelistefil" },
/* de */ { IDS_CANNOT_WRITE_FLIP_LIST_DE, "Schreiben von Fliplist Datei fehlgeschlagen" },
/* es */ { IDS_CANNOT_WRITE_FLIP_LIST_ES, "No puedo grabar fichero de lista de intercambio" },
/* fr */ { IDS_CANNOT_WRITE_FLIP_LIST_FR, "Impossible d'écrire le fichier de groupement de disques" },
/* hu */ { IDS_CANNOT_WRITE_FLIP_LIST_HU, "Nem sikerült a lemezlistát fájlba írni" },
/* it */ { IDS_CANNOT_WRITE_FLIP_LIST_IT, "Non è possibile scrivere il file della flip list" },
/* ko */ { IDS_CANNOT_WRITE_FLIP_LIST_KO, "ÇÃ¸³ ¸®½ºÆ® ÆÄÀÏÀ» ¾µ¼ö ¾ø½À´Ï´Ù" },
/* nl */ { IDS_CANNOT_WRITE_FLIP_LIST_NL, "Kan fliplijstbestand niet schrijven" },
/* pl */ { IDS_CANNOT_WRITE_FLIP_LIST_PL, "Nie mo¿na zapisaæ pliku listy wymiany no¶ników" },
/* ru */ { IDS_CANNOT_WRITE_FLIP_LIST_RU, "Cannot write flip list file" },
/* sv */ { IDS_CANNOT_WRITE_FLIP_LIST_SV, "Kan inte skriva vallistefil" },
/* tr */ { IDS_CANNOT_WRITE_FLIP_LIST_TR, "Sýralama listesi dosyasý yazýlamadý" },

/* en */ { IDS_ATTACH_CART0_LOW,    "Attach cartridge image for Cartridge0 Low" },
/* da */ { IDS_ATTACH_CART0_LOW_DA, "Tilslut cartridge image for Cartridge0 Lav" },
/* de */ { IDS_ATTACH_CART0_LOW_DE, "Cartridge0 (low) Erweiterungsmodul einlegen" },
/* es */ { IDS_ATTACH_CART0_LOW_ES, "Insertar imagen cartucho para cartucho0 bajo" },
/* fr */ { IDS_ATTACH_CART0_LOW_FR, "Attacher une image de cartouche pour la Cartouche0 Basse" },
/* hu */ { IDS_ATTACH_CART0_LOW_HU, "" },  /* fuzzy */
/* it */ { IDS_ATTACH_CART0_LOW_IT, "Seleziona immagine cartuccia per la Cartridge0 Low" },
/* ko */ { IDS_ATTACH_CART0_LOW_KO, "" },  /* fuzzy */
/* nl */ { IDS_ATTACH_CART0_LOW_NL, "Koppel cartridge bestand voor 'Cartridge0 Low'" },
/* pl */ { IDS_ATTACH_CART0_LOW_PL, "Zamontuj obraz kartrid¿a do Cartridge0 Low" },
/* ru */ { IDS_ATTACH_CART0_LOW_RU, "" },  /* fuzzy */
/* sv */ { IDS_ATTACH_CART0_LOW_SV, "Anslut insticksmodulfil för modul0 låg" },
/* tr */ { IDS_ATTACH_CART0_LOW_TR, "Kartuþ0 Low için kartuþ imajý yerleþtirin" },

/* en */ { IDS_ATTACH_CART0_HIGH,    "Attach cartridge image for Cartridge0 High" },
/* da */ { IDS_ATTACH_CART0_HIGH_DA, "Tilslut cartridge image for Cartridge0 Høj" },
/* de */ { IDS_ATTACH_CART0_HIGH_DE, "Cartridge0 (high) Erweiterungsmodul einlegen" },
/* es */ { IDS_ATTACH_CART0_HIGH_ES, "Insertar imagen cartucho para cartucho0 alto" },
/* fr */ { IDS_ATTACH_CART0_HIGH_FR, "Attacher une image de cartouche pour la Cartouche0 Haute" },
/* hu */ { IDS_ATTACH_CART0_HIGH_HU, "" },  /* fuzzy */
/* it */ { IDS_ATTACH_CART0_HIGH_IT, "Seleziona immagine cartuccia per la Cartridge0 High" },
/* ko */ { IDS_ATTACH_CART0_HIGH_KO, "" },  /* fuzzy */
/* nl */ { IDS_ATTACH_CART0_HIGH_NL, "Koppel cartridge bestand voor 'Cartridge0 High'" },
/* pl */ { IDS_ATTACH_CART0_HIGH_PL, "Zamontuj obraz kartrid¿a do Cartridge0 High" },
/* ru */ { IDS_ATTACH_CART0_HIGH_RU, "" },  /* fuzzy */
/* sv */ { IDS_ATTACH_CART0_HIGH_SV, "Anslut insticksmodulfil för modul0 hög" },
/* tr */ { IDS_ATTACH_CART0_HIGH_TR, "Kartuþ0 High için kartuþ imajý yerleþtirin" },

/* en */ { IDS_ATTACH_CART1_LOW,    "Attach cartridge image for Cartridge1 Low" },
/* da */ { IDS_ATTACH_CART1_LOW_DA, "Tilslut cartridge image for Cartridge1 Lav" },
/* de */ { IDS_ATTACH_CART1_LOW_DE, "Cartridge1 (low) Erweiterungsmodul einlegen" },
/* es */ { IDS_ATTACH_CART1_LOW_ES, "Insertar imagen cartucho para cartucho1 bajo" },
/* fr */ { IDS_ATTACH_CART1_LOW_FR, "Insérer une cartouche pour Cartouche1 basse" },
/* hu */ { IDS_ATTACH_CART1_LOW_HU, "Cartridge képmás csatolása alsó Cartridge1-hez" },
/* it */ { IDS_ATTACH_CART1_LOW_IT, "Seleziona immagine cartuccia per la Cartridge Low" },
/* ko */ { IDS_ATTACH_CART1_LOW_KO, "" },  /* fuzzy */
/* nl */ { IDS_ATTACH_CART1_LOW_NL, "Koppel cartridge bestand voor 'Cartridge1 Low'" },
/* pl */ { IDS_ATTACH_CART1_LOW_PL, "Zamontuj obraz kartrid¿a do Cartridge1 Low" },
/* ru */ { IDS_ATTACH_CART1_LOW_RU, "Attach cartridge image for Cartridge1 Low" },
/* sv */ { IDS_ATTACH_CART1_LOW_SV, "Anslut insticksmodulfil för modul1 låg" },
/* tr */ { IDS_ATTACH_CART1_LOW_TR, "Kartuþ1 Low için kartuþ imajý yerleþtirin" },

/* en */ { IDS_ATTACH_CART1_HIGH,    "Attach cartridge image for Cartridge1 High" },
/* da */ { IDS_ATTACH_CART1_HIGH_DA, "Tilslut cartridge image for Cartridge1 Høj" },
/* de */ { IDS_ATTACH_CART1_HIGH_DE, "Cartridge1 (high) Erweiterungsmodul einlegen" },
/* es */ { IDS_ATTACH_CART1_HIGH_ES, "Insertar imagen cartucho para cartucho1 alto" },
/* fr */ { IDS_ATTACH_CART1_HIGH_FR, "Insérer une cartouche pour Cartouche1 haute" },
/* hu */ { IDS_ATTACH_CART1_HIGH_HU, "Cartridge képmás csatolása felsõ Cartridge1-hez" },
/* it */ { IDS_ATTACH_CART1_HIGH_IT, "Seleziona immagine cartuccia per la Cartridge1 High" },
/* ko */ { IDS_ATTACH_CART1_HIGH_KO, "" },  /* fuzzy */
/* nl */ { IDS_ATTACH_CART1_HIGH_NL, "Koppel cartridge bestand voor 'Cartridge1 High'" },
/* pl */ { IDS_ATTACH_CART1_HIGH_PL, "Zamontuj obraz kartrid¿a do Cartridge1 High" },
/* ru */ { IDS_ATTACH_CART1_HIGH_RU, "Attach cartridge image for Cartridge1 High" },
/* sv */ { IDS_ATTACH_CART1_HIGH_SV, "Anslut insticksmodulfil för modul1 hög" },
/* tr */ { IDS_ATTACH_CART1_HIGH_TR, "Kartuþ1 High için kartuþ imajý yerleþtirin" },

/* en */ { IDS_ATTACH_CART2_LOW,    "Attach cartridge image for Cartridge2 Low" },
/* da */ { IDS_ATTACH_CART2_LOW_DA, "Tilslut cartridge image for Cartridge2 Lav" },
/* de */ { IDS_ATTACH_CART2_LOW_DE, "Cartridge2 (low) Erweiterungsmodul einlegen" },
/* es */ { IDS_ATTACH_CART2_LOW_ES, "Insertar imagen cartucho para cartucho2 bajo" },
/* fr */ { IDS_ATTACH_CART2_LOW_FR, "Insérer une cartouche pour Cartouche2 basse" },
/* hu */ { IDS_ATTACH_CART2_LOW_HU, "Cartridge képmás csatolása alsó Cartridge2-höz" },
/* it */ { IDS_ATTACH_CART2_LOW_IT, "Seleziona immagine cartuccia per la Cartridge2 Low" },
/* ko */ { IDS_ATTACH_CART2_LOW_KO, "" },  /* fuzzy */
/* nl */ { IDS_ATTACH_CART2_LOW_NL, "Koppel cartridge bestand voor 'Cartridge2 Low'" },
/* pl */ { IDS_ATTACH_CART2_LOW_PL, "Zamontuj obraz kartrid¿a do Cartridge2 Low" },
/* ru */ { IDS_ATTACH_CART2_LOW_RU, "Attach cartridge image for Cartridge2 Low" },
/* sv */ { IDS_ATTACH_CART2_LOW_SV, "Anslut insticksmodulfil för modul2 låg" },
/* tr */ { IDS_ATTACH_CART2_LOW_TR, "Kartuþ2 Low için kartuþ imajý yerleþtirin" },

/* en */ { IDS_ATTACH_CART2_HIGH,    "Attach cartridge image for Cartridge2 High" },
/* da */ { IDS_ATTACH_CART2_HIGH_DA, "Tilslut cartridge image for Cartridge2 Høj" },
/* de */ { IDS_ATTACH_CART2_HIGH_DE, "Cartridge1 (high) Erweiterungsmodul einlegen" },
/* es */ { IDS_ATTACH_CART2_HIGH_ES, "Insertar imagen cartucho para cartucho2 alto" },
/* fr */ { IDS_ATTACH_CART2_HIGH_FR, "Insérer une cartouche pour Cartouche2 haute" },
/* hu */ { IDS_ATTACH_CART2_HIGH_HU, "Cartridge képmás csatolása felsõ Cartridge2-höz" },
/* it */ { IDS_ATTACH_CART2_HIGH_IT, "Seleziona immagine cartuccia per la Cartridge2 High" },
/* ko */ { IDS_ATTACH_CART2_HIGH_KO, "" },  /* fuzzy */
/* nl */ { IDS_ATTACH_CART2_HIGH_NL, "Koppel cartridge bestand voor 'Cartridge2 High'" },
/* pl */ { IDS_ATTACH_CART2_HIGH_PL, "Zamontuj obraz kartrid¿a do Cartridge2 High" },
/* ru */ { IDS_ATTACH_CART2_HIGH_RU, "Attach cartridge image for Cartridge2 High" },
/* sv */ { IDS_ATTACH_CART2_HIGH_SV, "Anslut insticksmodulfil för modul2 hög" },
/* tr */ { IDS_ATTACH_CART2_HIGH_TR, "Kartuþ2 High için kartuþ imajý yerleþtirin" },

/* en */ { IDS_ATTACH_CART,    "Attach cartridge image" },
/* da */ { IDS_ATTACH_CART_DA, "Tilslut cartridge-image" },
/* de */ { IDS_ATTACH_CART_DE, "Erweiterungsmodul einlegen" },
/* es */ { IDS_ATTACH_CART_ES, "Insertar imagen de cartucho" },
/* fr */ { IDS_ATTACH_CART_FR, "Insérer une cartouche" },
/* hu */ { IDS_ATTACH_CART_HU, "Cartridge képmás csatolása" },
/* it */ { IDS_ATTACH_CART_IT, "Seleziona immagine cartuccia" },
/* ko */ { IDS_ATTACH_CART_KO, "Ä«Æ®¸®Áö ÀÌ¹ÌÁö ºÙÀÌ±â" },
/* nl */ { IDS_ATTACH_CART_NL, "Koppel cartridge bestand" },
/* pl */ { IDS_ATTACH_CART_PL, "Zamontuj obraz kartrid¿a" },
/* ru */ { IDS_ATTACH_CART_RU, "´ÞÑÐÒØâì ÞÑàÐ× ÚÐàâàØÔÖÐ" },
/* sv */ { IDS_ATTACH_CART_SV, "Anslut insticksmodulfil" },
/* tr */ { IDS_ATTACH_CART_TR, "Kartuþ imajý yerleþtir" },

/* en */ { IDS_SAVE_SNAPSHOT_IMAGE,    "Save snapshot image" },
/* da */ { IDS_SAVE_SNAPSHOT_IMAGE_DA, "Gem snapshot" },
/* de */ { IDS_SAVE_SNAPSHOT_IMAGE_DE, "Snapshot Datei speichern" },
/* es */ { IDS_SAVE_SNAPSHOT_IMAGE_ES, "Grabar Instantánea" },
/* fr */ { IDS_SAVE_SNAPSHOT_IMAGE_FR, "Enregistrer le fichier de sauvegarde" },
/* hu */ { IDS_SAVE_SNAPSHOT_IMAGE_HU, "Pillanatkép fájl mentése" },
/* it */ { IDS_SAVE_SNAPSHOT_IMAGE_IT, "Salva immagine snapshot" },
/* ko */ { IDS_SAVE_SNAPSHOT_IMAGE_KO, "½º³À¼¦ ÀÌ¹ÌÁö ÀúÀå" },
/* nl */ { IDS_SAVE_SNAPSHOT_IMAGE_NL, "Momentopnamebestand opslaan" },
/* pl */ { IDS_SAVE_SNAPSHOT_IMAGE_PL, "Zapisz obraz zrzutu" },
/* ru */ { IDS_SAVE_SNAPSHOT_IMAGE_RU, "ÁÞåàÐÝØâì áÝÕßèÞâ" },
/* sv */ { IDS_SAVE_SNAPSHOT_IMAGE_SV, "Spara ögonblicksbildfil" },
/* tr */ { IDS_SAVE_SNAPSHOT_IMAGE_TR, "Anlýk görüntü imajýný kaydet" },

/* en */ { IDS_LOAD_SNAPSHOT_IMAGE,    "Load snapshot image" },
/* da */ { IDS_LOAD_SNAPSHOT_IMAGE_DA, "Indlæs snapshot" },
/* de */ { IDS_LOAD_SNAPSHOT_IMAGE_DE, "Snapshot laden" },
/* es */ { IDS_LOAD_SNAPSHOT_IMAGE_ES, "Cargar Instantánea" },
/* fr */ { IDS_LOAD_SNAPSHOT_IMAGE_FR, "Charger le fichier de sauvegarde" },
/* hu */ { IDS_LOAD_SNAPSHOT_IMAGE_HU, "Pillanatkép betöltése" },
/* it */ { IDS_LOAD_SNAPSHOT_IMAGE_IT, "Carica immagine snapshot" },
/* ko */ { IDS_LOAD_SNAPSHOT_IMAGE_KO, "½º³À¼¦ ÀÌ¹ÌÁö ºÒ·¯¿À±â" },
/* nl */ { IDS_LOAD_SNAPSHOT_IMAGE_NL, "Laad momentopnamebestand" },
/* pl */ { IDS_LOAD_SNAPSHOT_IMAGE_PL, "Wczytaj obraz zrzutu" },
/* ru */ { IDS_LOAD_SNAPSHOT_IMAGE_RU, "·ÐÓàã×Øâì áÝÕßèÞâ" },
/* sv */ { IDS_LOAD_SNAPSHOT_IMAGE_SV, "Läser in ögonblicksbild" },
/* tr */ { IDS_LOAD_SNAPSHOT_IMAGE_TR, "Anlýk görüntü imajýný yükle" },

/* en */ { IDS_SAVE_CURRENTLY_ATTACHED_DISKS,    "Save currently attached disk images" },
/* da */ { IDS_SAVE_CURRENTLY_ATTACHED_DISKS_DA, "Gem tilsluttede disk-images" },
/* de */ { IDS_SAVE_CURRENTLY_ATTACHED_DISKS_DE, "Aktuell eingelegte Diskimages speichern" },
/* es */ { IDS_SAVE_CURRENTLY_ATTACHED_DISKS_ES, "Grabar imágenes de disco actualmente insertados" },
/* fr */ { IDS_SAVE_CURRENTLY_ATTACHED_DISKS_FR, "Sauvegarder les disques présentement attachés" },
/* hu */ { IDS_SAVE_CURRENTLY_ATTACHED_DISKS_HU, "A jelenleg csatolt lemezek mentése" },
/* it */ { IDS_SAVE_CURRENTLY_ATTACHED_DISKS_IT, "Salva immagini dischi attualmente selezionati" },
/* ko */ { IDS_SAVE_CURRENTLY_ATTACHED_DISKS_KO, "ÇöÁ¦ ºÙ¾îÀÖ´Â µð½ºÅ© ÀÌ¹ÌÁö ÀúÀå" },
/* nl */ { IDS_SAVE_CURRENTLY_ATTACHED_DISKS_NL, "Opslaan huidige gekoppelde schijfbestanden" },
/* pl */ { IDS_SAVE_CURRENTLY_ATTACHED_DISKS_PL, "Zapisz zaontowane obrazy dyskietek" },
/* ru */ { IDS_SAVE_CURRENTLY_ATTACHED_DISKS_RU, "ÁÞåàÐÝØâì ØáßÞÛì×ãÕÜëÕ ÞÑàÐ×ë ÔØáÚÞÒ" },
/* sv */ { IDS_SAVE_CURRENTLY_ATTACHED_DISKS_SV, "Spara anslutna diskettavbildningar" },
/* tr */ { IDS_SAVE_CURRENTLY_ATTACHED_DISKS_TR, "Halihazýrda yerleþtirilmiþ disk imajlarýný kaydet" },

/* en */ { IDS_SAVE_CURRENTLY_ATTACHED_ROMS,    "Save currently loaded ROM images" },
/* da */ { IDS_SAVE_CURRENTLY_ATTACHED_ROMS_DA, "Gem tilsluttede ROM-images" },
/* de */ { IDS_SAVE_CURRENTLY_ATTACHED_ROMS_DE, "Aktuelle ROMs speichern" },
/* es */ { IDS_SAVE_CURRENTLY_ATTACHED_ROMS_ES, "Grabar imágenes ROM actualmente cargadas" },
/* fr */ { IDS_SAVE_CURRENTLY_ATTACHED_ROMS_FR, "Sauvegarder les images ROM présentement chargés" },
/* hu */ { IDS_SAVE_CURRENTLY_ATTACHED_ROMS_HU, "A jelenleg betöltött ROM-ok mentése" },
/* it */ { IDS_SAVE_CURRENTLY_ATTACHED_ROMS_IT, "Salva immagini ROM attualmente caricate" },
/* ko */ { IDS_SAVE_CURRENTLY_ATTACHED_ROMS_KO, "ÇöÁ¦ ºÒ·¯¿À±â µÈ ROM ÀÌ¹ÌÁö ÀúÀå" },
/* nl */ { IDS_SAVE_CURRENTLY_ATTACHED_ROMS_NL, "Opslaan huidig gekoppelde ROMs" },
/* pl */ { IDS_SAVE_CURRENTLY_ATTACHED_ROMS_PL, "Zapisz wczytane obrazy ROM-ów" },
/* ru */ { IDS_SAVE_CURRENTLY_ATTACHED_ROMS_RU, "ÁÞåàÐÝØâì ×ÐÓàãÖÕÝÝëÕ ROM ÞÑàÐ×ë" },
/* sv */ { IDS_SAVE_CURRENTLY_ATTACHED_ROMS_SV, "Spara inlästa ROM-avbildningar" },
/* tr */ { IDS_SAVE_CURRENTLY_ATTACHED_ROMS_TR, "Halihazýrda yüklenmiþ ROM imajlarýný kaydet" },

/* en */ { IDS_SNAPSHOT_FILE,    "snapshot file" },
/* da */ { IDS_SNAPSHOT_FILE_DA, "snapshot-fil" },
/* de */ { IDS_SNAPSHOT_FILE_DE, "Snapshot Datei" },
/* es */ { IDS_SNAPSHOT_FILE_ES, "Fichero Instantánea" },
/* fr */ { IDS_SNAPSHOT_FILE_FR, "Fichier de sauvegarde" },
/* hu */ { IDS_SNAPSHOT_FILE_HU, "pillanatkép fájl" },
/* it */ { IDS_SNAPSHOT_FILE_IT, "File snapshot" },
/* ko */ { IDS_SNAPSHOT_FILE_KO, "" },  /* fuzzy */
/* nl */ { IDS_SNAPSHOT_FILE_NL, "momentopnamebestand" },
/* pl */ { IDS_SNAPSHOT_FILE_PL, "plik zrzutu" },
/* ru */ { IDS_SNAPSHOT_FILE_RU, "snapshot file" },
/* sv */ { IDS_SNAPSHOT_FILE_SV, "ögonblicksbildfil" },
/* tr */ { IDS_SNAPSHOT_FILE_TR, "Anlýk görüntü dosyasý" },

/* en */ { IDS_SELECT_SNAPSHOT_FILE,    "Select snapshot file" },
/* da */ { IDS_SELECT_SNAPSHOT_FILE_DA, "Vælg snapshot-fil" },
/* de */ { IDS_SELECT_SNAPSHOT_FILE_DE, "Snapshot Datei wählen" },
/* es */ { IDS_SELECT_SNAPSHOT_FILE_ES, "Seleccionar fichero Instantánea" },
/* fr */ { IDS_SELECT_SNAPSHOT_FILE_FR, "Sélectionner le fichier de sauvegarde" },
/* hu */ { IDS_SELECT_SNAPSHOT_FILE_HU, "Válasszon pillanatkép fájlt" },
/* it */ { IDS_SELECT_SNAPSHOT_FILE_IT, "Seleziona file snapshot" },
/* ko */ { IDS_SELECT_SNAPSHOT_FILE_KO, "" },  /* fuzzy */
/* nl */ { IDS_SELECT_SNAPSHOT_FILE_NL, "Selecteer momentopnamebestand" },
/* pl */ { IDS_SELECT_SNAPSHOT_FILE_PL, "Wybierz plik zrzutu" },
/* ru */ { IDS_SELECT_SNAPSHOT_FILE_RU, "Select snapshot file" },
/* sv */ { IDS_SELECT_SNAPSHOT_FILE_SV, "Välj ögonblicksbildfil" },
/* tr */ { IDS_SELECT_SNAPSHOT_FILE_TR, "Anlýk görüntü dosyasý seçin" },

/* en */ { IDS_CRT_EMULATION,    "CRT emulation" },
/* da */ { IDS_CRT_EMULATION_DA, "CRT-emulering" },
/* de */ { IDS_CRT_EMULATION_DE, "CRT Emulation" },
/* es */ { IDS_CRT_EMULATION_ES, "Emulación CRT" },
/* fr */ { IDS_CRT_EMULATION_FR, "Émulation CRT" },
/* hu */ { IDS_CRT_EMULATION_HU, "" },  /* fuzzy */
/* it */ { IDS_CRT_EMULATION_IT, "Emulazione CRT" },
/* ko */ { IDS_CRT_EMULATION_KO, "ÄÉÅäµå ·¹ÀÌ ÅÍ¹Ì³Î ¿¡¹Ä·¹ÀÌÅÍ" },
/* nl */ { IDS_CRT_EMULATION_NL, "CRT emulatie" },
/* pl */ { IDS_CRT_EMULATION_PL, "Emulacja CRT" },
/* ru */ { IDS_CRT_EMULATION_RU, "" },  /* fuzzy */
/* sv */ { IDS_CRT_EMULATION_SV, "CRT-emulering" },
/* tr */ { IDS_CRT_EMULATION_TR, "CRT emülasyonu" },

/* en */ { IDS_SCALE2X,    "Scale2x" },
/* da */ { IDS_SCALE2X_DA, "Scale2x" },
/* de */ { IDS_SCALE2X_DE, "Scale2×" },
/* es */ { IDS_SCALE2X_ES, "Scale2x" },
/* fr */ { IDS_SCALE2X_FR, "Scale2x" },
/* hu */ { IDS_SCALE2X_HU, "Élsimítás" },
/* it */ { IDS_SCALE2X_IT, "Scale2x" },
/* ko */ { IDS_SCALE2X_KO, "Scale2x" },
/* nl */ { IDS_SCALE2X_NL, "Scale2x" },
/* pl */ { IDS_SCALE2X_PL, "Scale2x" },
/* ru */ { IDS_SCALE2X_RU, "Scale2x" },
/* sv */ { IDS_SCALE2X_SV, "Scale2x" },
/* tr */ { IDS_SCALE2X_TR, "Scale2x" },

/* en */ { IDS_RENDER_FILTER,    "Render filter" },
/* da */ { IDS_RENDER_FILTER_DA, "Renderingsfilter" },
/* de */ { IDS_RENDER_FILTER_DE, "Render Filter" },
/* es */ { IDS_RENDER_FILTER_ES, "Filtro de renderizado" },
/* fr */ { IDS_RENDER_FILTER_FR, "Filtre de rendu" },
/* hu */ { IDS_RENDER_FILTER_HU, "" },  /* fuzzy */
/* it */ { IDS_RENDER_FILTER_IT, "Filtro di render" },
/* ko */ { IDS_RENDER_FILTER_KO, "·£´õ ÇÊÅÍ" },
/* nl */ { IDS_RENDER_FILTER_NL, "Render filter" },
/* pl */ { IDS_RENDER_FILTER_PL, "Filtr renderera" },
/* ru */ { IDS_RENDER_FILTER_RU, "Render filter" },
/* sv */ { IDS_RENDER_FILTER_SV, "Rendrerarfilter" },
/* tr */ { IDS_RENDER_FILTER_TR, "Ýmge oluþturma filtresi" },

/* en */ { IDS_EXTERNAL_PALETTE_SELECT,    "Select External Palette file" },
/* da */ { IDS_EXTERNAL_PALETTE_SELECT_DA, "Brug ekstern palette-fil" },
/* de */ { IDS_EXTERNAL_PALETTE_SELECT_DE, "Benutze externe Palette (Datei)" },
/* es */ { IDS_EXTERNAL_PALETTE_SELECT_ES, "Seleccionar fichero paleta externa" },
/* fr */ { IDS_EXTERNAL_PALETTE_SELECT_FR, "Utiliser une palette externe (fichier)" },
/* hu */ { IDS_EXTERNAL_PALETTE_SELECT_HU, "Válasszon külsõ paletta fájlt" },
/* it */ { IDS_EXTERNAL_PALETTE_SELECT_IT, "Seleziona file palette esterna" },
/* ko */ { IDS_EXTERNAL_PALETTE_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDS_EXTERNAL_PALETTE_SELECT_NL, "Selecteer extern paletbestand" },
/* pl */ { IDS_EXTERNAL_PALETTE_SELECT_PL, "Wybierz plik zewnêtrznej palety kolorów" },
/* ru */ { IDS_EXTERNAL_PALETTE_SELECT_RU, "Select External Palette file" },
/* sv */ { IDS_EXTERNAL_PALETTE_SELECT_SV, "Använd en extern palett (fil)" },
/* tr */ { IDS_EXTERNAL_PALETTE_SELECT_TR, "Harici Palet dosyasý seçin" },

/* en */ { IDS_GAMMA_0_4,    "Gamma (0..4)" },
/* da */ { IDS_GAMMA_0_4_DA, "Gamma (0..4)" },
/* de */ { IDS_GAMMA_0_4_DE, "Gamma (0..4)" },
/* es */ { IDS_GAMMA_0_4_ES, "Gamma (0..4)" },
/* fr */ { IDS_GAMMA_0_4_FR, "Intensité (0..4)" },
/* hu */ { IDS_GAMMA_0_4_HU, "Gamma (0..4)" },
/* it */ { IDS_GAMMA_0_4_IT, "Gamma (0..4)" },
/* ko */ { IDS_GAMMA_0_4_KO, "°¨¸¶ (0..4)" },
/* nl */ { IDS_GAMMA_0_4_NL, "Gamma (0..4)" },
/* pl */ { IDS_GAMMA_0_4_PL, "Gamma (0..4)" },
/* ru */ { IDS_GAMMA_0_4_RU, "Gamma (0..4)" },
/* sv */ { IDS_GAMMA_0_4_SV, "Gamma (0..4)" },
/* tr */ { IDS_GAMMA_0_4_TR, "Gama (0..4)" },

/* en */ { IDS_SCANLINE_SHADE_0_1,    "Scanline shade (0..1)" },
/* da */ { IDS_SCANLINE_SHADE_0_1_DA, "Skanlinjeskygge (0..1)" },
/* de */ { IDS_SCANLINE_SHADE_0_1_DE, "Scanline Schatten (0..1)" },
/* es */ { IDS_SCANLINE_SHADE_0_1_ES, "Sombra linea de escaneo (0..1)" },
/* fr */ { IDS_SCANLINE_SHADE_0_1_FR, "Ombrage des lignes de balayage (0..1)" },
/* hu */ { IDS_SCANLINE_SHADE_0_1_HU, "" },  /* fuzzy */
/* it */ { IDS_SCANLINE_SHADE_0_1_IT, "Ombra scanline (0..1)" },
/* ko */ { IDS_SCANLINE_SHADE_0_1_KO, "ÁÖ»ç¼± °¡¸®°Ô (0..1)" },
/* nl */ { IDS_SCANLINE_SHADE_0_1_NL, "Scanlijnschaduw (0..1)" },
/* pl */ { IDS_SCANLINE_SHADE_0_1_PL, "Cieniowane scanline (0..1)" },
/* ru */ { IDS_SCANLINE_SHADE_0_1_RU, "Scanline shade (0..1)" },
/* sv */ { IDS_SCANLINE_SHADE_0_1_SV, "Skuggning av skannlinjer (0..1)" },
/* tr */ { IDS_SCANLINE_SHADE_0_1_TR, "Tarama satýrý nüansý (0..1)" },

/* en */ { IDS_BLUR_0_1,    "Blur (0..1)" },
/* da */ { IDS_BLUR_0_1_DA, "Udtværing (0..1)" },
/* de */ { IDS_BLUR_0_1_DE, "Unschärfe (0..1)" },
/* es */ { IDS_BLUR_0_1_ES, "Blur (0..1)" },
/* fr */ { IDS_BLUR_0_1_FR, "Flou (0..1)" },
/* hu */ { IDS_BLUR_0_1_HU, "" },  /* fuzzy */
/* it */ { IDS_BLUR_0_1_IT, "Sfuocatura (0..1)" },
/* ko */ { IDS_BLUR_0_1_KO, "Èå¸´ÇÔ (0..1)" },
/* nl */ { IDS_BLUR_0_1_NL, "Wazigheid (0..1)" },
/* pl */ { IDS_BLUR_0_1_PL, "Rozmycie (0..1)" },
/* ru */ { IDS_BLUR_0_1_RU, "Blur (0..1)" },
/* sv */ { IDS_BLUR_0_1_SV, "Suddighet (0..1)" },
/* tr */ { IDS_BLUR_0_1_TR, "Bulanýklýk (0..1)" },

/* en */ { IDS_TINT_0_2,    "Tint (0..2)" },
/* da */ { IDS_TINT_0_2_DA, "Farvetone (0..2)" },
/* de */ { IDS_TINT_0_2_DE, "Farbton (0..2)" },
/* es */ { IDS_TINT_0_2_ES, "Matiz (0..2)" },
/* fr */ { IDS_TINT_0_2_FR, "Teinte (0..2)" },
/* hu */ { IDS_TINT_0_2_HU, "Szín (0..2)" },
/* it */ { IDS_TINT_0_2_IT, "Sfumatura (0..2)" },
/* ko */ { IDS_TINT_0_2_KO, "Æ¾Æ® (0..2)" },
/* nl */ { IDS_TINT_0_2_NL, "Tint (0..2)" },
/* pl */ { IDS_TINT_0_2_PL, "Odcieñ (0..2)" },
/* ru */ { IDS_TINT_0_2_RU, "¾ââÕÝÞÚ (0..2)" },
/* sv */ { IDS_TINT_0_2_SV, "Färgton (0..2)" },
/* tr */ { IDS_TINT_0_2_TR, "Ton (0..2)" },

/* en */ { IDS_ODDLINE_PHASE_0_2,    "Odd lines phase (0..2)" },
/* da */ { IDS_ODDLINE_PHASE_0_2_DA, "Fase for ulige linjer (0..2)" },
/* de */ { IDS_ODDLINE_PHASE_0_2_DE, "Ungerade Zeilenphase (0..2)" },
/* es */ { IDS_ODDLINE_PHASE_0_2_ES, "Lineas fase impar (0..2)" },
/* fr */ { IDS_ODDLINE_PHASE_0_2_FR, "Phase lignes impaires (0..2)" },
/* hu */ { IDS_ODDLINE_PHASE_0_2_HU, "" },  /* fuzzy */
/* it */ { IDS_ODDLINE_PHASE_0_2_IT, "Fase linee dispari (0..2)" },
/* ko */ { IDS_ODDLINE_PHASE_0_2_KO, "" },  /* fuzzy */
/* nl */ { IDS_ODDLINE_PHASE_0_2_NL, "Oneven lijnen fase (0..2)" },
/* pl */ { IDS_ODDLINE_PHASE_0_2_PL, "Faza linii nieparzystych (0..2)" },
/* ru */ { IDS_ODDLINE_PHASE_0_2_RU, "Odd lines phase (0..2)" },
/* sv */ { IDS_ODDLINE_PHASE_0_2_SV, "Fas för udda linjer (0..2)" },
/* tr */ { IDS_ODDLINE_PHASE_0_2_TR, "Tek satýr fazý (0..2)" },

/* en */ { IDS_ODDLINE_OFFSET_0_2,    "Odd lines offset (0..2)" },
/* da */ { IDS_ODDLINE_OFFSET_0_2_DA, "Offset for ulige linjer (0..2)" },
/* de */ { IDS_ODDLINE_OFFSET_0_2_DE, "Ungerader Zeilenoffset (0..2)" },
/* es */ { IDS_ODDLINE_OFFSET_0_2_ES, "Offset lineas impares (0..2)" },
/* fr */ { IDS_ODDLINE_OFFSET_0_2_FR, "Décalage lignes impaires (0..2)" },
/* hu */ { IDS_ODDLINE_OFFSET_0_2_HU, "" },  /* fuzzy */
/* it */ { IDS_ODDLINE_OFFSET_0_2_IT, "Offset linee dispari (0..2)" },
/* ko */ { IDS_ODDLINE_OFFSET_0_2_KO, "È¦¼ö ¶óÀÎ ¿ÀÇÁ¼Â (0..2)" },
/* nl */ { IDS_ODDLINE_OFFSET_0_2_NL, "Oneven lijnen compensatie (0..2)" },
/* pl */ { IDS_ODDLINE_OFFSET_0_2_PL, "Wyrównanie linii nieparzystych (0..2)" },
/* ru */ { IDS_ODDLINE_OFFSET_0_2_RU, "Odd lines offset (0..2)" },
/* sv */ { IDS_ODDLINE_OFFSET_0_2_SV, "Utjämning för udda linjer (0..2)" },
/* tr */ { IDS_ODDLINE_OFFSET_0_2_TR, "Tek satýr ofseti (0..2)" },

/* en */ { IDS_VICII_EXTERNAL_PALETTE,    "External VICII palette" },
/* da */ { IDS_VICII_EXTERNAL_PALETTE_DA, "Ekstern VICII palette" },
/* de */ { IDS_VICII_EXTERNAL_PALETTE_DE, "Externe VIC-II Palette" },
/* es */ { IDS_VICII_EXTERNAL_PALETTE_ES, "Paleta VICII externa" },
/* fr */ { IDS_VICII_EXTERNAL_PALETTE_FR, "Palette externe VICII" },
/* hu */ { IDS_VICII_EXTERNAL_PALETTE_HU, "" },  /* fuzzy */
/* it */ { IDS_VICII_EXTERNAL_PALETTE_IT, "Palette VICII esterna" },
/* ko */ { IDS_VICII_EXTERNAL_PALETTE_KO, "" },  /* fuzzy */
/* nl */ { IDS_VICII_EXTERNAL_PALETTE_NL, "Extern VICII palet" },
/* pl */ { IDS_VICII_EXTERNAL_PALETTE_PL, "Zewnêtrzna paleta VICII" },
/* ru */ { IDS_VICII_EXTERNAL_PALETTE_RU, "External VICII palette" },
/* sv */ { IDS_VICII_EXTERNAL_PALETTE_SV, "Extern VICII-palett" },
/* tr */ { IDS_VICII_EXTERNAL_PALETTE_TR, "Harici VICII paleti" },

/* en */ { IDS_VIC_EXTERNAL_PALETTE,    "External VIC palette" },
/* da */ { IDS_VIC_EXTERNAL_PALETTE_DA, "Ekstern VIC palette" },
/* de */ { IDS_VIC_EXTERNAL_PALETTE_DE, "Externe VIC Palette" },
/* es */ { IDS_VIC_EXTERNAL_PALETTE_ES, "Paleta VIC externa" },
/* fr */ { IDS_VIC_EXTERNAL_PALETTE_FR, "Palette externe VIC" },
/* hu */ { IDS_VIC_EXTERNAL_PALETTE_HU, "" },  /* fuzzy */
/* it */ { IDS_VIC_EXTERNAL_PALETTE_IT, "Palette VIC esterna" },
/* ko */ { IDS_VIC_EXTERNAL_PALETTE_KO, "" },  /* fuzzy */
/* nl */ { IDS_VIC_EXTERNAL_PALETTE_NL, "Extern VIC palet" },
/* pl */ { IDS_VIC_EXTERNAL_PALETTE_PL, "Zewnêtrzna paleta VIC" },
/* ru */ { IDS_VIC_EXTERNAL_PALETTE_RU, "External VIC palette" },
/* sv */ { IDS_VIC_EXTERNAL_PALETTE_SV, "Extern VIC-palett" },
/* tr */ { IDS_VIC_EXTERNAL_PALETTE_TR, "Harici VIC paleti" },

/* en */ { IDS_TED_EXTERNAL_PALETTE,    "External TED palette" },
/* da */ { IDS_TED_EXTERNAL_PALETTE_DA, "Ekstern TED palette" },
/* de */ { IDS_TED_EXTERNAL_PALETTE_DE, "Externe TED Palette" },
/* es */ { IDS_TED_EXTERNAL_PALETTE_ES, "Paleta TED externa" },
/* fr */ { IDS_TED_EXTERNAL_PALETTE_FR, "Palette externe TED" },
/* hu */ { IDS_TED_EXTERNAL_PALETTE_HU, "" },  /* fuzzy */
/* it */ { IDS_TED_EXTERNAL_PALETTE_IT, "Palette TED esterna" },
/* ko */ { IDS_TED_EXTERNAL_PALETTE_KO, "" },  /* fuzzy */
/* nl */ { IDS_TED_EXTERNAL_PALETTE_NL, "Extern TED palet" },
/* pl */ { IDS_TED_EXTERNAL_PALETTE_PL, "Zewnêtrzna paleta TED" },
/* ru */ { IDS_TED_EXTERNAL_PALETTE_RU, "External TED palette" },
/* sv */ { IDS_TED_EXTERNAL_PALETTE_SV, "Extern TED-palett" },
/* tr */ { IDS_TED_EXTERNAL_PALETTE_TR, "Harici TED paleti" },

/* en */ { IDS_VDC_EXTERNAL_PALETTE,    "External VDC palette" },
/* da */ { IDS_VDC_EXTERNAL_PALETTE_DA, "Ekstern VDC palette" },
/* de */ { IDS_VDC_EXTERNAL_PALETTE_DE, "Externe VDC Palette" },
/* es */ { IDS_VDC_EXTERNAL_PALETTE_ES, "Paleta VDC externa" },
/* fr */ { IDS_VDC_EXTERNAL_PALETTE_FR, "Palette externe VDC" },
/* hu */ { IDS_VDC_EXTERNAL_PALETTE_HU, "" },  /* fuzzy */
/* it */ { IDS_VDC_EXTERNAL_PALETTE_IT, "Palette VDC esterna" },
/* ko */ { IDS_VDC_EXTERNAL_PALETTE_KO, "" },  /* fuzzy */
/* nl */ { IDS_VDC_EXTERNAL_PALETTE_NL, "Extern VDC palet" },
/* pl */ { IDS_VDC_EXTERNAL_PALETTE_PL, "Zewnêtrzna paleta VDC" },
/* ru */ { IDS_VDC_EXTERNAL_PALETTE_RU, "External VDC palette" },
/* sv */ { IDS_VDC_EXTERNAL_PALETTE_SV, "Extern VDC-palett" },
/* tr */ { IDS_VDC_EXTERNAL_PALETTE_TR, "Harici VDC paleti" },

/* en */ { IDS_CRTC_EXTERNAL_PALETTE,    "External CRTC palette" },
/* da */ { IDS_CRTC_EXTERNAL_PALETTE_DA, "Ekstern CRTC palette" },
/* de */ { IDS_CRTC_EXTERNAL_PALETTE_DE, "Externe CRTC Palette" },
/* es */ { IDS_CRTC_EXTERNAL_PALETTE_ES, "Paleta CRT externa" },
/* fr */ { IDS_CRTC_EXTERNAL_PALETTE_FR, "Palette externe CRTC" },
/* hu */ { IDS_CRTC_EXTERNAL_PALETTE_HU, "" },  /* fuzzy */
/* it */ { IDS_CRTC_EXTERNAL_PALETTE_IT, "Palette CRTC esterna" },
/* ko */ { IDS_CRTC_EXTERNAL_PALETTE_KO, "" },  /* fuzzy */
/* nl */ { IDS_CRTC_EXTERNAL_PALETTE_NL, "Extern CRTC palet" },
/* pl */ { IDS_CRTC_EXTERNAL_PALETTE_PL, "Zewnêtrzna paleta CRTC" },
/* ru */ { IDS_CRTC_EXTERNAL_PALETTE_RU, "External CRTC palette" },
/* sv */ { IDS_CRTC_EXTERNAL_PALETTE_SV, "Extern CRTC-palett" },
/* tr */ { IDS_CRTC_EXTERNAL_PALETTE_TR, "Harici CRTC paleti" },

/* en */ { IDS_VICII_PALETTE_NAME,    "VICII palette file" },
/* da */ { IDS_VICII_PALETTE_NAME_DA, "VICII-palettefil" },
/* de */ { IDS_VICII_PALETTE_NAME_DE, "VIC-II Palettendatei" },
/* es */ { IDS_VICII_PALETTE_NAME_ES, "Fichero paleta VICII" },
/* fr */ { IDS_VICII_PALETTE_NAME_FR, "Fichier de palette VICII" },
/* hu */ { IDS_VICII_PALETTE_NAME_HU, "" },  /* fuzzy */
/* it */ { IDS_VICII_PALETTE_NAME_IT, "File palette VICII" },
/* ko */ { IDS_VICII_PALETTE_NAME_KO, "" },  /* fuzzy */
/* nl */ { IDS_VICII_PALETTE_NAME_NL, "VICII palet bestand" },
/* pl */ { IDS_VICII_PALETTE_NAME_PL, "Plik palety VICII" },
/* ru */ { IDS_VICII_PALETTE_NAME_RU, "" },  /* fuzzy */
/* sv */ { IDS_VICII_PALETTE_NAME_SV, "VICII-palettfil" },
/* tr */ { IDS_VICII_PALETTE_NAME_TR, "VICII palet dosyasý" },

/* en */ { IDS_VIC_PALETTE_NAME,    "VIC palette file" },
/* da */ { IDS_VIC_PALETTE_NAME_DA, "VIC-palettefil" },
/* de */ { IDS_VIC_PALETTE_NAME_DE, "VIC Palettendatei" },
/* es */ { IDS_VIC_PALETTE_NAME_ES, "Fichero paleta VIC" },
/* fr */ { IDS_VIC_PALETTE_NAME_FR, "Fichier de palette VIC" },
/* hu */ { IDS_VIC_PALETTE_NAME_HU, "" },  /* fuzzy */
/* it */ { IDS_VIC_PALETTE_NAME_IT, "File palette VIC" },
/* ko */ { IDS_VIC_PALETTE_NAME_KO, "" },  /* fuzzy */
/* nl */ { IDS_VIC_PALETTE_NAME_NL, "VIC palet bestand" },
/* pl */ { IDS_VIC_PALETTE_NAME_PL, "Plik palety VIC" },
/* ru */ { IDS_VIC_PALETTE_NAME_RU, "" },  /* fuzzy */
/* sv */ { IDS_VIC_PALETTE_NAME_SV, "VIC-palettfil" },
/* tr */ { IDS_VIC_PALETTE_NAME_TR, "VIC palet dosyasý" },

/* en */ { IDS_TED_PALETTE_NAME,    "TED palette file" },
/* da */ { IDS_TED_PALETTE_NAME_DA, "TED-palettefil" },
/* de */ { IDS_TED_PALETTE_NAME_DE, "TED Palettendatei" },
/* es */ { IDS_TED_PALETTE_NAME_ES, "Fichero paleta TED" },
/* fr */ { IDS_TED_PALETTE_NAME_FR, "Fichier de palette TED" },
/* hu */ { IDS_TED_PALETTE_NAME_HU, "" },  /* fuzzy */
/* it */ { IDS_TED_PALETTE_NAME_IT, "File palette TED" },
/* ko */ { IDS_TED_PALETTE_NAME_KO, "" },  /* fuzzy */
/* nl */ { IDS_TED_PALETTE_NAME_NL, "TED palet bestand" },
/* pl */ { IDS_TED_PALETTE_NAME_PL, "Plik palety TED" },
/* ru */ { IDS_TED_PALETTE_NAME_RU, "TED palette file" },
/* sv */ { IDS_TED_PALETTE_NAME_SV, "TED-palettfil" },
/* tr */ { IDS_TED_PALETTE_NAME_TR, "TED palet dosyasý" },

/* en */ { IDS_VDC_PALETTE_NAME,    "VDC palette file" },
/* da */ { IDS_VDC_PALETTE_NAME_DA, "VDC-palettefil" },
/* de */ { IDS_VDC_PALETTE_NAME_DE, "VDC Palettendatei" },
/* es */ { IDS_VDC_PALETTE_NAME_ES, "Fichero paleta VDC" },
/* fr */ { IDS_VDC_PALETTE_NAME_FR, "Fichier de palette VDC" },
/* hu */ { IDS_VDC_PALETTE_NAME_HU, "" },  /* fuzzy */
/* it */ { IDS_VDC_PALETTE_NAME_IT, "File palette VDC" },
/* ko */ { IDS_VDC_PALETTE_NAME_KO, "" },  /* fuzzy */
/* nl */ { IDS_VDC_PALETTE_NAME_NL, "VDC palet bestand" },
/* pl */ { IDS_VDC_PALETTE_NAME_PL, "Plik palety VDC" },
/* ru */ { IDS_VDC_PALETTE_NAME_RU, "" },  /* fuzzy */
/* sv */ { IDS_VDC_PALETTE_NAME_SV, "VDC-palettfil" },
/* tr */ { IDS_VDC_PALETTE_NAME_TR, "VDC palet dosyasý" },

/* en */ { IDS_CRTC_PALETTE_NAME,    "CRTC palette file" },
/* da */ { IDS_CRTC_PALETTE_NAME_DA, "CRTC-palettefil" },
/* de */ { IDS_CRTC_PALETTE_NAME_DE, "CRTC Palettendatei" },
/* es */ { IDS_CRTC_PALETTE_NAME_ES, "Fichero paleta CRT" },
/* fr */ { IDS_CRTC_PALETTE_NAME_FR, "Fichier de palette CTRC" },
/* hu */ { IDS_CRTC_PALETTE_NAME_HU, "" },  /* fuzzy */
/* it */ { IDS_CRTC_PALETTE_NAME_IT, "File palette CRTC" },
/* ko */ { IDS_CRTC_PALETTE_NAME_KO, "" },  /* fuzzy */
/* nl */ { IDS_CRTC_PALETTE_NAME_NL, "CRTC palet bestand" },
/* pl */ { IDS_CRTC_PALETTE_NAME_PL, "Plik palety CRTC" },
/* ru */ { IDS_CRTC_PALETTE_NAME_RU, "" },  /* fuzzy */
/* sv */ { IDS_CRTC_PALETTE_NAME_SV, "CRTC-palettfil" },
/* tr */ { IDS_CRTC_PALETTE_NAME_TR, "CRTC palet dosyasý" },

/* en */ { IDS_SATURATION_0_2,    "Saturation (0..2)" },
/* da */ { IDS_SATURATION_0_2_DA, "Mætning (0..2)" },
/* de */ { IDS_SATURATION_0_2_DE, "Sättigung (0..2)" },
/* es */ { IDS_SATURATION_0_2_ES, "Saturacion (0..2)" },
/* fr */ { IDS_SATURATION_0_2_FR, "Saturation (0..2)" },
/* hu */ { IDS_SATURATION_0_2_HU, "Telítettség (0..2)" },
/* it */ { IDS_SATURATION_0_2_IT, "Saturazione (0..2)" },
/* ko */ { IDS_SATURATION_0_2_KO, "Æ÷È­µµ (0..2)" },
/* nl */ { IDS_SATURATION_0_2_NL, "Verzadiging (0..2)" },
/* pl */ { IDS_SATURATION_0_2_PL, "Nasycenie (0..2)" },
/* ru */ { IDS_SATURATION_0_2_RU, "Saturation (0..2)" },
/* sv */ { IDS_SATURATION_0_2_SV, "Mättnad (0..2)" },
/* tr */ { IDS_SATURATION_0_2_TR, "Doygunluk (0..2)" },

/* en */ { IDS_CONTRAST_0_2,    "Contrast (0..2)" },
/* da */ { IDS_CONTRAST_0_2_DA, "Kontrast (0..2)" },
/* de */ { IDS_CONTRAST_0_2_DE, "Kontrast (0..2)" },
/* es */ { IDS_CONTRAST_0_2_ES, "Contraste (0..2)" },
/* fr */ { IDS_CONTRAST_0_2_FR, "Contraste (0..2)" },
/* hu */ { IDS_CONTRAST_0_2_HU, "Kontraszt (0..2)" },
/* it */ { IDS_CONTRAST_0_2_IT, "Contrasto (0..2)" },
/* ko */ { IDS_CONTRAST_0_2_KO, "¸í¾Ï (0..2)" },
/* nl */ { IDS_CONTRAST_0_2_NL, "Contrast (0..2)" },
/* pl */ { IDS_CONTRAST_0_2_PL, "Kontrast (0..2)" },
/* ru */ { IDS_CONTRAST_0_2_RU, "ºÞÝâàÐáâÝÞáâì (0..2)" },
/* sv */ { IDS_CONTRAST_0_2_SV, "Kontrast (0..2)" },
/* tr */ { IDS_CONTRAST_0_2_TR, "Karþýtlýk (0..2)" },

/* en */ { IDS_BRIGHTNESS_0_2,    "Brightness (0..2)" },
/* da */ { IDS_BRIGHTNESS_0_2_DA, "Lysstyrke (0..2)" },
/* de */ { IDS_BRIGHTNESS_0_2_DE, "Helligkeit (0..2)" },
/* es */ { IDS_BRIGHTNESS_0_2_ES, "Brillo (0..2)" },
/* fr */ { IDS_BRIGHTNESS_0_2_FR, "Luminosité (0..2)" },
/* hu */ { IDS_BRIGHTNESS_0_2_HU, "Fényesség (0..2)" },
/* it */ { IDS_BRIGHTNESS_0_2_IT, "Luminosità (0..2)" },
/* ko */ { IDS_BRIGHTNESS_0_2_KO, "¹à±â (0..2)" },
/* nl */ { IDS_BRIGHTNESS_0_2_NL, "Helderheid (0..2)" },
/* pl */ { IDS_BRIGHTNESS_0_2_PL, "Jasno¶æ (0..2)" },
/* ru */ { IDS_BRIGHTNESS_0_2_RU, "ÏàÚÞáâì (0..2)" },
/* sv */ { IDS_BRIGHTNESS_0_2_SV, "Ljusstyrka (0..2)" },
/* tr */ { IDS_BRIGHTNESS_0_2_TR, "Parlaklýk (0..2)" },

/* en */ { IDS_VIDEO_SETTINGS,    "Video settings" },
/* da */ { IDS_VIDEO_SETTINGS_DA, "Grafikindstillinger" },
/* de */ { IDS_VIDEO_SETTINGS_DE, "Video Einstellungen" },
/* es */ { IDS_VIDEO_SETTINGS_ES, "Ajustes de video" },
/* fr */ { IDS_VIDEO_SETTINGS_FR, "Paramètres vidéo" },
/* hu */ { IDS_VIDEO_SETTINGS_HU, "Videó beállításai" },
/* it */ { IDS_VIDEO_SETTINGS_IT, "Impostazioni video" },
/* ko */ { IDS_VIDEO_SETTINGS_KO, "ºñµð·Î ¼ÂÆÃ" },
/* nl */ { IDS_VIDEO_SETTINGS_NL, "Video instellingen" },
/* pl */ { IDS_VIDEO_SETTINGS_PL, "Ustawienia wideo" },
/* ru */ { IDS_VIDEO_SETTINGS_RU, "½ÐáâàÞÙÚØ ÒØÔÕÞ" },
/* sv */ { IDS_VIDEO_SETTINGS_SV, "Grafikinställningar" },
/* tr */ { IDS_VIDEO_SETTINGS_TR, "Görüntü ayarlarý" },

/* en */ { IDS_AUTO_FROM_ROM,    "Auto (from ROM)" },
/* da */ { IDS_AUTO_FROM_ROM_DA, "Automatisk (fra ROM)" },
/* de */ { IDS_AUTO_FROM_ROM_DE, "Automatisch (von ROM)" },
/* es */ { IDS_AUTO_FROM_ROM_ES, "Auto (desde ROM)" },
/* fr */ { IDS_AUTO_FROM_ROM_FR, "Auto (depuis ROM)" },
/* hu */ { IDS_AUTO_FROM_ROM_HU, "Automatikus (ROM-ból)" },
/* it */ { IDS_AUTO_FROM_ROM_IT, "Auto (dalla ROM)" },
/* ko */ { IDS_AUTO_FROM_ROM_KO, "ÀÚµ¿ (ROM ¿¡¼­)" },
/* nl */ { IDS_AUTO_FROM_ROM_NL, "Automatisch (van ROM)" },
/* pl */ { IDS_AUTO_FROM_ROM_PL, "Automatycznie (z ROM)" },
/* ru */ { IDS_AUTO_FROM_ROM_RU, "Auto (from ROM)" },
/* sv */ { IDS_AUTO_FROM_ROM_SV, "Automatisk (från ROM)" },
/* tr */ { IDS_AUTO_FROM_ROM_TR, "Otomatik (ROM'dan)" },

/* en */ { IDS_40_COLUMNS,    "40 Columns" },
/* da */ { IDS_40_COLUMNS_DA, "40 kolonner" },
/* de */ { IDS_40_COLUMNS_DE, "40 Spalten" },
/* es */ { IDS_40_COLUMNS_ES, "40 columnas" },
/* fr */ { IDS_40_COLUMNS_FR, "40 Colonnes" },
/* hu */ { IDS_40_COLUMNS_HU, "40 Oszlop" },
/* it */ { IDS_40_COLUMNS_IT, "40 Colonne" },
/* ko */ { IDS_40_COLUMNS_KO, "40Ä­" },
/* nl */ { IDS_40_COLUMNS_NL, "40 Kolommen" },
/* pl */ { IDS_40_COLUMNS_PL, "40 kolumn" },
/* ru */ { IDS_40_COLUMNS_RU, "40 ÚÞÛÞÝÞÚ" },
/* sv */ { IDS_40_COLUMNS_SV, "40 kolumner" },
/* tr */ { IDS_40_COLUMNS_TR, "40 Sütun" },

/* en */ { IDS_80_COLUMNS,    "80 Columns" },
/* da */ { IDS_80_COLUMNS_DA, "80 kolonner" },
/* de */ { IDS_80_COLUMNS_DE, "80 Spalten" },
/* es */ { IDS_80_COLUMNS_ES, "80 columnas" },
/* fr */ { IDS_80_COLUMNS_FR, "80 Colonnes" },
/* hu */ { IDS_80_COLUMNS_HU, "80 Oszlop" },
/* it */ { IDS_80_COLUMNS_IT, "80 Colonne" },
/* ko */ { IDS_80_COLUMNS_KO, "80Ä­" },
/* nl */ { IDS_80_COLUMNS_NL, "80 Kolommen" },
/* pl */ { IDS_80_COLUMNS_PL, "80 kolumn" },
/* ru */ { IDS_80_COLUMNS_RU, "80 ÚÞÛÞÝÞÚ" },
/* sv */ { IDS_80_COLUMNS_SV, "80 kolumner" },
/* tr */ { IDS_80_COLUMNS_TR, "80 Sütun" },

/* en */ { IDS_256_BYTE,    "256 Byte" },
/* da */ { IDS_256_BYTE_DA, "256 byte" },
/* de */ { IDS_256_BYTE_DE, "256 Byte" },
/* es */ { IDS_256_BYTE_ES, "256 Bytes" },
/* fr */ { IDS_256_BYTE_FR, "256 Octets" },
/* hu */ { IDS_256_BYTE_HU, "256 Bájt" },
/* it */ { IDS_256_BYTE_IT, "256 Byte" },
/* ko */ { IDS_256_BYTE_KO, "256 byte" },
/* nl */ { IDS_256_BYTE_NL, "256 Byte" },
/* pl */ { IDS_256_BYTE_PL, "256 bajtów" },
/* ru */ { IDS_256_BYTE_RU, "256 Byte" },
/* sv */ { IDS_256_BYTE_SV, "256 byte" },
/* tr */ { IDS_256_BYTE_TR, "256 Byte" },

/* en */ { IDS_2_KBYTE,    "2 kByte" },
/* da */ { IDS_2_KBYTE_DA, "2 kbyte" },
/* de */ { IDS_2_KBYTE_DE, "2 kByte" },
/* es */ { IDS_2_KBYTE_ES, "2 kBytes" },
/* fr */ { IDS_2_KBYTE_FR, "2 ko" },
/* hu */ { IDS_2_KBYTE_HU, "2 kBájt" },
/* it */ { IDS_2_KBYTE_IT, "2 kByte" },
/* ko */ { IDS_2_KBYTE_KO, "2 kbyte" },
/* nl */ { IDS_2_KBYTE_NL, "2 kByte" },
/* pl */ { IDS_2_KBYTE_PL, "2 kBajty " },
/* ru */ { IDS_2_KBYTE_RU, "2 kByte" },
/* sv */ { IDS_2_KBYTE_SV, "2 kbyte" },
/* tr */ { IDS_2_KBYTE_TR, "2 kByte" },

/* en */ { IDS_GRAPHICS,    "Graphics" },
/* da */ { IDS_GRAPHICS_DA, "Grafisk" },
/* de */ { IDS_GRAPHICS_DE, "Grafik" },
/* es */ { IDS_GRAPHICS_ES, "Gráficos" },
/* fr */ { IDS_GRAPHICS_FR, "Graphiques" },
/* hu */ { IDS_GRAPHICS_HU, "Grafikus" },
/* it */ { IDS_GRAPHICS_IT, "Grafica" },
/* ko */ { IDS_GRAPHICS_KO, "±×·¡ÇÈ" },
/* nl */ { IDS_GRAPHICS_NL, "Grafiek" },
/* pl */ { IDS_GRAPHICS_PL, "Grafika" },
/* ru */ { IDS_GRAPHICS_RU, "³àÐäØÚÐ" },
/* sv */ { IDS_GRAPHICS_SV, "Grafisk" },
/* tr */ { IDS_GRAPHICS_TR, "Grafikler" },

/* en */ { IDS_TEXT,    "Text" },
/* da */ { IDS_TEXT_DA, "Tekst" },
/* de */ { IDS_TEXT_DE, "Text" },
/* es */ { IDS_TEXT_ES, "Texto" },
/* fr */ { IDS_TEXT_FR, "Texte" },
/* hu */ { IDS_TEXT_HU, "" },  /* fuzzy */
/* it */ { IDS_TEXT_IT, "Testo" },
/* ko */ { IDS_TEXT_KO, "ÅØ½ºÆ®" },
/* nl */ { IDS_TEXT_NL, "Tekst" },
/* pl */ { IDS_TEXT_PL, "Tekst" },
/* ru */ { IDS_TEXT_RU, "ÂÕÚáâ" },
/* sv */ { IDS_TEXT_SV, "Text" },
/* tr */ { IDS_TEXT_TR, "Metin" },

/* en */ { IDS_BUSINESS_UK,    "Business (UK)" },
/* da */ { IDS_BUSINESS_UK_DA, "Business (Storbritannien)" },
/* de */ { IDS_BUSINESS_UK_DE, "Business (UK)" },
/* es */ { IDS_BUSINESS_UK_ES, "Negocios (UK)" },
/* fr */ { IDS_BUSINESS_UK_FR, "Bureautique (UK)" },
/* hu */ { IDS_BUSINESS_UK_HU, "Hivatalos (UK)" },
/* it */ { IDS_BUSINESS_UK_IT, "Business (UK)" },
/* ko */ { IDS_BUSINESS_UK_KO, "ºñÁö´Ï½º(UK)" },
/* nl */ { IDS_BUSINESS_UK_NL, "Zakelijk (UK)" },
/* pl */ { IDS_BUSINESS_UK_PL, "Biznesowa (UK)" },
/* ru */ { IDS_BUSINESS_UK_RU, "Business (UK)" },
/* sv */ { IDS_BUSINESS_UK_SV, "Business (Storbritannien)" },
/* tr */ { IDS_BUSINESS_UK_TR, "Business (UK)" },

/* en */ { IDS_MEMORY,    "Memory" },
/* da */ { IDS_MEMORY_DA, "Hukommelse" },
/* de */ { IDS_MEMORY_DE, "Speicher" },
/* es */ { IDS_MEMORY_ES, "Memoria" },
/* fr */ { IDS_MEMORY_FR, "Mémoire" },
/* hu */ { IDS_MEMORY_HU, "Memória" },
/* it */ { IDS_MEMORY_IT, "Memoria" },
/* ko */ { IDS_MEMORY_KO, "¸Þ¸ð¸®" },
/* nl */ { IDS_MEMORY_NL, "Geheugen" },
/* pl */ { IDS_MEMORY_PL, "Pamiêæ" },
/* ru */ { IDS_MEMORY_RU, "¿ÐÜïâì" },
/* sv */ { IDS_MEMORY_SV, "Minne" },
/* tr */ { IDS_MEMORY_TR, "Bellek" },

/* en */ { IDS_VIDEO_SIZE,    "Video size" },
/* da */ { IDS_VIDEO_SIZE_DA, "Skærmbredde" },
/* de */ { IDS_VIDEO_SIZE_DE, "Videospeichergröße" },
/* es */ { IDS_VIDEO_SIZE_ES, "Tamaño de pantalla" },
/* fr */ { IDS_VIDEO_SIZE_FR, "Taille vidéo" },
/* hu */ { IDS_VIDEO_SIZE_HU, "Videó méret" },
/* it */ { IDS_VIDEO_SIZE_IT, "Dimensione video" },
/* ko */ { IDS_VIDEO_SIZE_KO, "ºñµð¿À Å©±â" },
/* nl */ { IDS_VIDEO_SIZE_NL, "Videogrootte" },
/* pl */ { IDS_VIDEO_SIZE_PL, "Rozmiar wideo" },
/* ru */ { IDS_VIDEO_SIZE_RU, "ÀÐ×ÜÕà ÒØÔÕÞ" },
/* sv */ { IDS_VIDEO_SIZE_SV, "Skärmbredd" },
/* tr */ { IDS_VIDEO_SIZE_TR, "Görüntü boyutu" },

/* en */ { IDS_IO_SIZE,    "I/O size" },
/* da */ { IDS_IO_SIZE_DA, "I/O-størrelse" },
/* de */ { IDS_IO_SIZE_DE, "I/O Größe" },
/* es */ { IDS_IO_SIZE_ES, "Tamaño de entradas/salidas" },
/* fr */ { IDS_IO_SIZE_FR, "Taille d'E/S" },
/* hu */ { IDS_IO_SIZE_HU, "I/O méret" },
/* it */ { IDS_IO_SIZE_IT, "Dimensione I/O" },
/* ko */ { IDS_IO_SIZE_KO, "I/O Å©±â" },
/* nl */ { IDS_IO_SIZE_NL, "I/O-grootte" },
/* pl */ { IDS_IO_SIZE_PL, "Rozmiar we/wy" },
/* ru */ { IDS_IO_SIZE_RU, "ÀÐ×ÜÕà I/O" },
/* sv */ { IDS_IO_SIZE_SV, "I/O-utrymme" },
/* tr */ { IDS_IO_SIZE_TR, "G/Ç boyutu" },

/* en */ { IDS_KEYBOARD,    "Keyboard" },
/* da */ { IDS_KEYBOARD_DA, "Tastatur" },
/* de */ { IDS_KEYBOARD_DE, "Tastatur" },
/* es */ { IDS_KEYBOARD_ES, "Teclado" },
/* fr */ { IDS_KEYBOARD_FR, "Clavier" },
/* hu */ { IDS_KEYBOARD_HU, "Billentyûzet" },
/* it */ { IDS_KEYBOARD_IT, "Tastiera" },
/* ko */ { IDS_KEYBOARD_KO, "Å°º¸µå" },
/* nl */ { IDS_KEYBOARD_NL, "Toetsenbord" },
/* pl */ { IDS_KEYBOARD_PL, "Klawiatura" },
/* ru */ { IDS_KEYBOARD_RU, "ºÛÐÒØÐâãàÐ" },
/* sv */ { IDS_KEYBOARD_SV, "Tangentbord" },
/* tr */ { IDS_KEYBOARD_TR, "Klavye" },

/* en */ { IDS_CRTC_CHIP,    "CRTC chip" },
/* da */ { IDS_CRTC_CHIP_DA, "CRTC-kreds" },
/* de */ { IDS_CRTC_CHIP_DE, "CRTC Chip" },
/* es */ { IDS_CRTC_CHIP_ES, "Chip CRTC" },
/* fr */ { IDS_CRTC_CHIP_FR, "Puce CRTC" },
/* hu */ { IDS_CRTC_CHIP_HU, "CRTC csip" },
/* it */ { IDS_CRTC_CHIP_IT, "Chip CRTC" },
/* ko */ { IDS_CRTC_CHIP_KO, "" },  /* fuzzy */
/* nl */ { IDS_CRTC_CHIP_NL, "CRTC chip" },
/* pl */ { IDS_CRTC_CHIP_PL, "Uk³ad CRTC" },
/* ru */ { IDS_CRTC_CHIP_RU, "CRTC chip" },
/* sv */ { IDS_CRTC_CHIP_SV, "CRTC-krets" },
/* tr */ { IDS_CRTC_CHIP_TR, "CRTC çipi" },

/* en */ { IDS_SUPERPET_IO,    "SuperPET I/O enable (disables 8x96)" },
/* da */ { IDS_SUPERPET_IO_DA, "Aktivér SuperPET-I/O (deaktiverer 8x96)" },
/* de */ { IDS_SUPERPET_IO_DE, "SuperPET I/O aktivieren (deaktiviert 8x96)" },
/* es */ { IDS_SUPERPET_IO_ES, "SuperPET entradas/salidas habilitadas (deshabilitado 8x96)" },
/* fr */ { IDS_SUPERPET_IO_FR, "E/S SuperPET activé (désactive 8x96)" },
/* hu */ { IDS_SUPERPET_IO_HU, "SuperPET I/O engedélyezése (8x96-ot tiltja)" },
/* it */ { IDS_SUPERPET_IO_IT, "Attiva I/O SuperPET (disattiva 8x96)" },
/* ko */ { IDS_SUPERPET_IO_KO, "SuperPET I/O »ç¿ëÇÏ±â (»ç¿ëÇÏÁö¾Ê±â 8x96)" },
/* nl */ { IDS_SUPERPET_IO_NL, "SuperPET-I/O inschakelen (zet 8x96 uit)" },
/* pl */ { IDS_SUPERPET_IO_PL, "W³±cz SuperPET I/O (wy³±cza 8x96)" },
/* ru */ { IDS_SUPERPET_IO_RU, "SuperPET I/O enable (disables 8x96)" },
/* sv */ { IDS_SUPERPET_IO_SV, "Aktivera SuperPET-I/O (inaktiverar 8x96)" },
/* tr */ { IDS_SUPERPET_IO_TR, "SuperPET G/Ç aktif et (8x96'yý pasifleþtir)" },

/* en */ { IDS_9xxx_AS_RAM,    "$9*** as RAM" },
/* da */ { IDS_9xxx_AS_RAM_DA, "$9*** som RAM" },
/* de */ { IDS_9xxx_AS_RAM_DE, "$9*** als RAM" },
/* es */ { IDS_9xxx_AS_RAM_ES, "$9*** como RAM" },
/* fr */ { IDS_9xxx_AS_RAM_FR, "$9*** comme RAM" },
/* hu */ { IDS_9xxx_AS_RAM_HU, "$9*** RAM-ként" },
/* it */ { IDS_9xxx_AS_RAM_IT, "$9*** come RAM" },
/* ko */ { IDS_9xxx_AS_RAM_KO, "" },  /* fuzzy */
/* nl */ { IDS_9xxx_AS_RAM_NL, "$9*** als RAM" },
/* pl */ { IDS_9xxx_AS_RAM_PL, "$9*** jako RAM" },
/* ru */ { IDS_9xxx_AS_RAM_RU, "$9*** as RAM" },
/* sv */ { IDS_9xxx_AS_RAM_SV, "$9*** som RAM" },
/* tr */ { IDS_9xxx_AS_RAM_TR, "RAM olarak $9***" },

/* en */ { IDS_Axxx_AS_RAM,    "$A*** as RAM" },
/* da */ { IDS_Axxx_AS_RAM_DA, "$A*** som RAM" },
/* de */ { IDS_Axxx_AS_RAM_DE, "$A*** als RAM" },
/* es */ { IDS_Axxx_AS_RAM_ES, "$A*** como RAM" },
/* fr */ { IDS_Axxx_AS_RAM_FR, "$A*** comme RAM (8296 seulement)" },
/* hu */ { IDS_Axxx_AS_RAM_HU, "$A*** RAM-ként" },
/* it */ { IDS_Axxx_AS_RAM_IT, "$A*** come RAM" },
/* ko */ { IDS_Axxx_AS_RAM_KO, "" },  /* fuzzy */
/* nl */ { IDS_Axxx_AS_RAM_NL, "$A*** als RAM" },
/* pl */ { IDS_Axxx_AS_RAM_PL, "$A*** jako RAM" },
/* ru */ { IDS_Axxx_AS_RAM_RU, "$A*** as RAM" },
/* sv */ { IDS_Axxx_AS_RAM_SV, "$A*** som RAM" },
/* tr */ { IDS_Axxx_AS_RAM_TR, "RAM olarak $A***" },

/* en */ { IDS_PET_SETTINGS,    "PET settings" },
/* da */ { IDS_PET_SETTINGS_DA, "PET-indstillinger" },
/* de */ { IDS_PET_SETTINGS_DE, "PET Einstellungen" },
/* es */ { IDS_PET_SETTINGS_ES, "Ajustes PET" },
/* fr */ { IDS_PET_SETTINGS_FR, "Paramètres PET" },
/* hu */ { IDS_PET_SETTINGS_HU, "PET beállításai" },
/* it */ { IDS_PET_SETTINGS_IT, "Impostazioni PET" },
/* ko */ { IDS_PET_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_PET_SETTINGS_NL, "PET instellingen" },
/* pl */ { IDS_PET_SETTINGS_PL, "Ustawienia PET" },
/* ru */ { IDS_PET_SETTINGS_RU, "PET settings" },
/* sv */ { IDS_PET_SETTINGS_SV, "PET-inställningar" },
/* tr */ { IDS_PET_SETTINGS_TR, "PET ayarlarý" },

/* en */ { IDS_CURRENT_MODEL,    "Current Model" },
/* da */ { IDS_CURRENT_MODEL_DA, "Nuværende model" },
/* de */ { IDS_CURRENT_MODEL_DE, "Aktuelles Modell" },
/* es */ { IDS_CURRENT_MODEL_ES, "modelo actual" },
/* fr */ { IDS_CURRENT_MODEL_FR, "Modèle actuel" },
/* hu */ { IDS_CURRENT_MODEL_HU, "Jelenlegi modell" },
/* it */ { IDS_CURRENT_MODEL_IT, "Modello attuale" },
/* ko */ { IDS_CURRENT_MODEL_KO, "" },  /* fuzzy */
/* nl */ { IDS_CURRENT_MODEL_NL, "Huidig Model" },
/* pl */ { IDS_CURRENT_MODEL_PL, "Obecny model" },
/* ru */ { IDS_CURRENT_MODEL_RU, "Current Model" },
/* sv */ { IDS_CURRENT_MODEL_SV, "Aktuell modell" },
/* tr */ { IDS_CURRENT_MODEL_TR, "Geçerli Model" },

/* en */ { IDS_PET_MODEL,    "PET Model" },
/* da */ { IDS_PET_MODEL_DA, "PET-model" },
/* de */ { IDS_PET_MODEL_DE, "PET Modell" },
/* es */ { IDS_PET_MODEL_ES, "Modelo PET" },
/* fr */ { IDS_PET_MODEL_FR, "Modèle PET" },
/* hu */ { IDS_PET_MODEL_HU, "PET modell" },
/* it */ { IDS_PET_MODEL_IT, "Modello PET" },
/* ko */ { IDS_PET_MODEL_KO, "" },  /* fuzzy */
/* nl */ { IDS_PET_MODEL_NL, "PET model" },
/* pl */ { IDS_PET_MODEL_PL, "Model PET" },
/* ru */ { IDS_PET_MODEL_RU, "¼ÞÔÕÛì PET" },
/* sv */ { IDS_PET_MODEL_SV, "PET-modell" },
/* tr */ { IDS_PET_MODEL_TR, "PET Modeli" },

/* en */ { IDS_PLUS4_SETTINGS,    "Plus4 settings" },
/* da */ { IDS_PLUS4_SETTINGS_DA, "Plus4-indstillinger" },
/* de */ { IDS_PLUS4_SETTINGS_DE, "Plus4 Einstellungen" },
/* es */ { IDS_PLUS4_SETTINGS_ES, "Ajustes Plus4" },
/* fr */ { IDS_PLUS4_SETTINGS_FR, "Paramètres Plus4" },
/* hu */ { IDS_PLUS4_SETTINGS_HU, "Plus4 beállításai" },
/* it */ { IDS_PLUS4_SETTINGS_IT, "Impostazioni Plus4" },
/* ko */ { IDS_PLUS4_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_PLUS4_SETTINGS_NL, "Plus4 instellingen" },
/* pl */ { IDS_PLUS4_SETTINGS_PL, "Ustawienia Plus4" },
/* ru */ { IDS_PLUS4_SETTINGS_RU, "Plus4 settings" },
/* sv */ { IDS_PLUS4_SETTINGS_SV, "Plus4-inställningar" },
/* tr */ { IDS_PLUS4_SETTINGS_TR, "Plus4 ayarlarý" },

/* en */ { IDS_SELECT_INTERNAL_FUNCTION_ROM_FILENAME,    "Select Internal Function ROM file" },
/* da */ { IDS_SELECT_INTERNAL_FUNCTION_ROM_FILENAME_DA, "Vælg image for intern funktions-ROM" },
/* de */ { IDS_SELECT_INTERNAL_FUNCTION_ROM_FILENAME_DE, "Internes Funktions ROM Imagedatei auswählen" },
/* es */ { IDS_SELECT_INTERNAL_FUNCTION_ROM_FILENAME_ES, "Seleccionar fichero ROM función interno" },
/* fr */ { IDS_SELECT_INTERNAL_FUNCTION_ROM_FILENAME_FR, "Sélectionner le fichier image ROM Fonction interne" },
/* hu */ { IDS_SELECT_INTERNAL_FUNCTION_ROM_FILENAME_HU, "Adja meg a belsõ Function ROM képmás nevét" },
/* it */ { IDS_SELECT_INTERNAL_FUNCTION_ROM_FILENAME_IT, "Seleziona file Function ROM interna" },
/* ko */ { IDS_SELECT_INTERNAL_FUNCTION_ROM_FILENAME_KO, "" },  /* fuzzy */
/* nl */ { IDS_SELECT_INTERNAL_FUNCTION_ROM_FILENAME_NL, "Selecteer Intern Functie ROM bestand" },
/* pl */ { IDS_SELECT_INTERNAL_FUNCTION_ROM_FILENAME_PL, "Wybierz plik wewnêtrznego Function ROM-u" },
/* ru */ { IDS_SELECT_INTERNAL_FUNCTION_ROM_FILENAME_RU, "Select Internal Function ROM file" },
/* sv */ { IDS_SELECT_INTERNAL_FUNCTION_ROM_FILENAME_SV, "Välj avbildning för internt funktions-ROM" },
/* tr */ { IDS_SELECT_INTERNAL_FUNCTION_ROM_FILENAME_TR, "Dahili Function ROM dosyasý seçin" },

/* en */ { IDS_SELECT_EXTERNAL_FUNCTION_ROM_FILENAME,    "Select External Function ROM file" },
/* da */ { IDS_SELECT_EXTERNAL_FUNCTION_ROM_FILENAME_DA, "Vælg image for ekstern funktions-ROM" },
/* de */ { IDS_SELECT_EXTERNAL_FUNCTION_ROM_FILENAME_DE, "Externes Funktions ROM Imagedatei auswählen" },
/* es */ { IDS_SELECT_EXTERNAL_FUNCTION_ROM_FILENAME_ES, "Seleccionar fichero ROM función externo" },
/* fr */ { IDS_SELECT_EXTERNAL_FUNCTION_ROM_FILENAME_FR, "Sélectionner le fichier image ROM Fonction externe" },
/* hu */ { IDS_SELECT_EXTERNAL_FUNCTION_ROM_FILENAME_HU, "Adja meg a külsõ Function ROM képmás nevét" },
/* it */ { IDS_SELECT_EXTERNAL_FUNCTION_ROM_FILENAME_IT, "Seleziona file Function ROM esterna" },
/* ko */ { IDS_SELECT_EXTERNAL_FUNCTION_ROM_FILENAME_KO, "" },  /* fuzzy */
/* nl */ { IDS_SELECT_EXTERNAL_FUNCTION_ROM_FILENAME_NL, "Selecteer Extern Functie ROM bestand" },
/* pl */ { IDS_SELECT_EXTERNAL_FUNCTION_ROM_FILENAME_PL, "Wybierz plik zewnêtrznego Function ROM-u" },
/* ru */ { IDS_SELECT_EXTERNAL_FUNCTION_ROM_FILENAME_RU, "Select External Function ROM file" },
/* sv */ { IDS_SELECT_EXTERNAL_FUNCTION_ROM_FILENAME_SV, "Välj avbildning för externt funktions-ROM" },
/* tr */ { IDS_SELECT_EXTERNAL_FUNCTION_ROM_FILENAME_TR, "Harici Function ROM dosyasý seçin" },

/* en */ { IDS_MACHINE_TYPE,    "Machine type" },
/* da */ { IDS_MACHINE_TYPE_DA, "Maskintype" },
/* de */ { IDS_MACHINE_TYPE_DE, "Maschinen Typ" },
/* es */ { IDS_MACHINE_TYPE_ES, "Tipo de máquina" },
/* fr */ { IDS_MACHINE_TYPE_FR, "Type de machine" },
/* hu */ { IDS_MACHINE_TYPE_HU, "Gép típus" },
/* it */ { IDS_MACHINE_TYPE_IT, "Tipo computer" },
/* ko */ { IDS_MACHINE_TYPE_KO, "ÄÄÇ»ÅÍ À¯Çü" },
/* nl */ { IDS_MACHINE_TYPE_NL, "Machinesoort" },
/* pl */ { IDS_MACHINE_TYPE_PL, "Typ maszyny" },
/* ru */ { IDS_MACHINE_TYPE_RU, "ÂØß ÚÞÜßìîâÕàÐ" },
/* sv */ { IDS_MACHINE_TYPE_SV, "Maskintyp" },
/* tr */ { IDS_MACHINE_TYPE_TR, "Makine tipi" },

/* en */ { IDS_INTERNAL_FUNCTION_ROM,    "Internal Function ROM" },
/* da */ { IDS_INTERNAL_FUNCTION_ROM_DA, "Intern funktions-ROM" },
/* de */ { IDS_INTERNAL_FUNCTION_ROM_DE, "Internes Funktions ROM" },
/* es */ { IDS_INTERNAL_FUNCTION_ROM_ES, "ROM de función interna" },
/* fr */ { IDS_INTERNAL_FUNCTION_ROM_FR, "Image ROM Fonction interne" },
/* hu */ { IDS_INTERNAL_FUNCTION_ROM_HU, "Belsõ Function ROM" },
/* it */ { IDS_INTERNAL_FUNCTION_ROM_IT, "Function ROM interna" },
/* ko */ { IDS_INTERNAL_FUNCTION_ROM_KO, "" },  /* fuzzy */
/* nl */ { IDS_INTERNAL_FUNCTION_ROM_NL, "Interne Functie ROM" },
/* pl */ { IDS_INTERNAL_FUNCTION_ROM_PL, "Wewnêtrzny Function ROM" },
/* ru */ { IDS_INTERNAL_FUNCTION_ROM_RU, "Internal Function ROM" },
/* sv */ { IDS_INTERNAL_FUNCTION_ROM_SV, "Internt funktions-ROM" },
/* tr */ { IDS_INTERNAL_FUNCTION_ROM_TR, "Uluslararasý Function ROM" },

/* en */ { IDS_INTERNAL_FUNCTION_ROM_FILENAME,    "Internal Function ROM file" },
/* da */ { IDS_INTERNAL_FUNCTION_ROM_FILENAME_DA, "Intern funktions-ROM" },
/* de */ { IDS_INTERNAL_FUNCTION_ROM_FILENAME_DE, "Interne Funktions ROM Datei" },
/* es */ { IDS_INTERNAL_FUNCTION_ROM_FILENAME_ES, "Fichero ROM función interno" },
/* fr */ { IDS_INTERNAL_FUNCTION_ROM_FILENAME_FR, "Fichier ROM Fonction interne" },
/* hu */ { IDS_INTERNAL_FUNCTION_ROM_FILENAME_HU, "Belsõ Function ROM fájl" },
/* it */ { IDS_INTERNAL_FUNCTION_ROM_FILENAME_IT, "File Function ROM interna" },
/* ko */ { IDS_INTERNAL_FUNCTION_ROM_FILENAME_KO, "ÀÎÅÍ³Î Function ROM ÆÄÀÏ" },
/* nl */ { IDS_INTERNAL_FUNCTION_ROM_FILENAME_NL, "Interne Functie ROM bestand" },
/* pl */ { IDS_INTERNAL_FUNCTION_ROM_FILENAME_PL, "Plik wewnêtrznego Function ROM-u" },
/* ru */ { IDS_INTERNAL_FUNCTION_ROM_FILENAME_RU, "Internal Function ROM file" },
/* sv */ { IDS_INTERNAL_FUNCTION_ROM_FILENAME_SV, "Internt funktions-ROM-fil" },
/* tr */ { IDS_INTERNAL_FUNCTION_ROM_FILENAME_TR, "Dahili Function ROM dosyasý" },

/* en */ { IDS_EXTERNAL_FUNCTION_ROM,    "External Function ROM" },
/* da */ { IDS_EXTERNAL_FUNCTION_ROM_DA, "Extern funktions-ROM" },
/* de */ { IDS_EXTERNAL_FUNCTION_ROM_DE, "Externes Funktions ROM" },
/* es */ { IDS_EXTERNAL_FUNCTION_ROM_ES, "ROM de función externa" },
/* fr */ { IDS_EXTERNAL_FUNCTION_ROM_FR, "Image ROM Fonction externe" },
/* hu */ { IDS_EXTERNAL_FUNCTION_ROM_HU, "Külsõ Function ROM" },
/* it */ { IDS_EXTERNAL_FUNCTION_ROM_IT, "Function ROM esterna" },
/* ko */ { IDS_EXTERNAL_FUNCTION_ROM_KO, "" },  /* fuzzy */
/* nl */ { IDS_EXTERNAL_FUNCTION_ROM_NL, "Externe Functie ROM" },
/* pl */ { IDS_EXTERNAL_FUNCTION_ROM_PL, "Zewnêtrzny Function ROM" },
/* ru */ { IDS_EXTERNAL_FUNCTION_ROM_RU, "External Function ROM" },
/* sv */ { IDS_EXTERNAL_FUNCTION_ROM_SV, "Externt funktions-ROM" },
/* tr */ { IDS_EXTERNAL_FUNCTION_ROM_TR, "Harici Function ROM" },

/* en */ { IDS_EXTERNAL_FUNCTION_ROM_FILENAME,    "External Function ROM file" },
/* da */ { IDS_EXTERNAL_FUNCTION_ROM_FILENAME_DA, "Extern funktions-ROM" },
/* de */ { IDS_EXTERNAL_FUNCTION_ROM_FILENAME_DE, "Externe Funktions ROM Datei" },
/* es */ { IDS_EXTERNAL_FUNCTION_ROM_FILENAME_ES, "Fichero ROM función externo" },
/* fr */ { IDS_EXTERNAL_FUNCTION_ROM_FILENAME_FR, "Fichier ROM Fonction externe" },
/* hu */ { IDS_EXTERNAL_FUNCTION_ROM_FILENAME_HU, "Külsõ Function ROM fájl" },
/* it */ { IDS_EXTERNAL_FUNCTION_ROM_FILENAME_IT, "File Function ROM esterna" },
/* ko */ { IDS_EXTERNAL_FUNCTION_ROM_FILENAME_KO, "¿ÜºÎ ÇÔ¼ö ROM ÆÄÀÏ" },
/* nl */ { IDS_EXTERNAL_FUNCTION_ROM_FILENAME_NL, "Externe Functie ROM bestand" },
/* pl */ { IDS_EXTERNAL_FUNCTION_ROM_FILENAME_PL, "Plik zewnêtrznego Function ROM-u" },
/* ru */ { IDS_EXTERNAL_FUNCTION_ROM_FILENAME_RU, "External Function ROM file" },
/* sv */ { IDS_EXTERNAL_FUNCTION_ROM_FILENAME_SV, "Externt funktions-ROM" },
/* tr */ { IDS_EXTERNAL_FUNCTION_ROM_FILENAME_TR, "Harici Function ROM dosyasý" },

/* en */ { IDS_RAM_BANKS_2_AND_3,    "RAM banks 2 & 3" },
/* da */ { IDS_RAM_BANKS_2_AND_3_DA, "RAM bank 2 & 3" },
/* de */ { IDS_RAM_BANKS_2_AND_3_DE, "RAM Bänkte 2 & 3" },
/* es */ { IDS_RAM_BANKS_2_AND_3_ES, "RAM bancos 2 y 3" },
/* fr */ { IDS_RAM_BANKS_2_AND_3_FR, "Banques RAM 2 & 3" },
/* hu */ { IDS_RAM_BANKS_2_AND_3_HU, "2-es és 3-as RAM bankok" },
/* it */ { IDS_RAM_BANKS_2_AND_3_IT, "Banchi RAM 2 & 3" },
/* ko */ { IDS_RAM_BANKS_2_AND_3_KO, "" },  /* fuzzy */
/* nl */ { IDS_RAM_BANKS_2_AND_3_NL, "RAM banken 2 & 3" },
/* pl */ { IDS_RAM_BANKS_2_AND_3_PL, "Banki RAM 2 i 3" },
/* ru */ { IDS_RAM_BANKS_2_AND_3_RU, "RAM banks 2 & 3" },
/* sv */ { IDS_RAM_BANKS_2_AND_3_SV, "RAM-bank 2 & 3" },
/* tr */ { IDS_RAM_BANKS_2_AND_3_TR, "RAM banklarý 2 & 3" },

/* en */ { IDS_INTERNAL_FUNCTION_RTC_SAVE,    "Enable Internal Function RTC data saving" },
/* da */ { IDS_INTERNAL_FUNCTION_RTC_SAVE_DA, "" },  /* fuzzy */
/* de */ { IDS_INTERNAL_FUNCTION_RTC_SAVE_DE, "Interne Funktions RTC Speicherung aktivieren" },
/* es */ { IDS_INTERNAL_FUNCTION_RTC_SAVE_ES, "Permitir grabar datos de Internal Function RTC" },
/* fr */ { IDS_INTERNAL_FUNCTION_RTC_SAVE_FR, "" },  /* fuzzy */
/* hu */ { IDS_INTERNAL_FUNCTION_RTC_SAVE_HU, "" },  /* fuzzy */
/* it */ { IDS_INTERNAL_FUNCTION_RTC_SAVE_IT, "Attiva salvataggio dei dati RTC dell'Internal Function" },
/* ko */ { IDS_INTERNAL_FUNCTION_RTC_SAVE_KO, "" },  /* fuzzy */
/* nl */ { IDS_INTERNAL_FUNCTION_RTC_SAVE_NL, "Opslaan van de interne functie RTC gegevens inschakelen" },
/* pl */ { IDS_INTERNAL_FUNCTION_RTC_SAVE_PL, "W³±cz zapis zegara wewnêtrznego Function ROM-u" },
/* ru */ { IDS_INTERNAL_FUNCTION_RTC_SAVE_RU, "" },  /* fuzzy */
/* sv */ { IDS_INTERNAL_FUNCTION_RTC_SAVE_SV, "" },  /* fuzzy */
/* tr */ { IDS_INTERNAL_FUNCTION_RTC_SAVE_TR, "" },  /* fuzzy */

/* en */ { IDS_EXTERNAL_FUNCTION_RTC_SAVE,    "Enable External Function RTC data saving" },
/* da */ { IDS_EXTERNAL_FUNCTION_RTC_SAVE_DA, "" },  /* fuzzy */
/* de */ { IDS_EXTERNAL_FUNCTION_RTC_SAVE_DE, "Externe Funktions RTC Speicherung aktivieren" },
/* es */ { IDS_EXTERNAL_FUNCTION_RTC_SAVE_ES, "Permitir grabar datos de External Function RTC" },
/* fr */ { IDS_EXTERNAL_FUNCTION_RTC_SAVE_FR, "" },  /* fuzzy */
/* hu */ { IDS_EXTERNAL_FUNCTION_RTC_SAVE_HU, "" },  /* fuzzy */
/* it */ { IDS_EXTERNAL_FUNCTION_RTC_SAVE_IT, "Attiva salvataggio dei dati RTC dell'External Function" },
/* ko */ { IDS_EXTERNAL_FUNCTION_RTC_SAVE_KO, "" },  /* fuzzy */
/* nl */ { IDS_EXTERNAL_FUNCTION_RTC_SAVE_NL, "Opslaan van de externe functie RTC gegevens inschakelen" },
/* pl */ { IDS_EXTERNAL_FUNCTION_RTC_SAVE_PL, "W³±cz zapis zegara zewnêtrznego Function ROM-u" },
/* ru */ { IDS_EXTERNAL_FUNCTION_RTC_SAVE_RU, "" },  /* fuzzy */
/* sv */ { IDS_EXTERNAL_FUNCTION_RTC_SAVE_SV, "" },  /* fuzzy */
/* tr */ { IDS_EXTERNAL_FUNCTION_RTC_SAVE_TR, "" },  /* fuzzy */

/* en */ { IDS_C128_SETTINGS,    "C128 settings" },
/* da */ { IDS_C128_SETTINGS_DA, "C128-indstillinger" },
/* de */ { IDS_C128_SETTINGS_DE, "C128 Einstellungen" },
/* es */ { IDS_C128_SETTINGS_ES, "Ajustes C128" },
/* fr */ { IDS_C128_SETTINGS_FR, "Paramètres C128" },
/* hu */ { IDS_C128_SETTINGS_HU, "C128 beállításai" },
/* it */ { IDS_C128_SETTINGS_IT, "Impostazioni C128" },
/* ko */ { IDS_C128_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_C128_SETTINGS_NL, "C128 instellingen" },
/* pl */ { IDS_C128_SETTINGS_PL, "Ustawienia C128" },
/* ru */ { IDS_C128_SETTINGS_RU, "½ÐáâàÞÙÚØ C128" },
/* sv */ { IDS_C128_SETTINGS_SV, "C128-inställningar" },
/* tr */ { IDS_C128_SETTINGS_TR, "C128 ayarlarý" },

/* en */ { IDS_SELECT_ROM_FILE,    "Select ROM file" },
/* da */ { IDS_SELECT_ROM_FILE_DA, "Vælg ROM-fil" },
/* de */ { IDS_SELECT_ROM_FILE_DE, "ROM Datei wählen" },
/* es */ { IDS_SELECT_ROM_FILE_ES, "Seleccionar fichero ROM" },
/* fr */ { IDS_SELECT_ROM_FILE_FR, "Sélectionner fichier ROM" },
/* hu */ { IDS_SELECT_ROM_FILE_HU, "Válasszon ROM fájlt" },
/* it */ { IDS_SELECT_ROM_FILE_IT, "Seleziona file ROM" },
/* ko */ { IDS_SELECT_ROM_FILE_KO, "" },  /* fuzzy */
/* nl */ { IDS_SELECT_ROM_FILE_NL, "Selecteer ROM bestand" },
/* pl */ { IDS_SELECT_ROM_FILE_PL, "wybierz plik ROM" },
/* ru */ { IDS_SELECT_ROM_FILE_RU, "²ëÑàÐâì äÐÙÛ ROM" },
/* sv */ { IDS_SELECT_ROM_FILE_SV, "Välj ROM-fil" },
/* tr */ { IDS_SELECT_ROM_FILE_TR, "ROM dosyasý seçin" },

/* en */ { IDS_COMPUTER_ROM_SETTINGS,    "Computer ROM settings" },
/* da */ { IDS_COMPUTER_ROM_SETTINGS_DA, "Computer ROM-indstillinger" },
/* de */ { IDS_COMPUTER_ROM_SETTINGS_DE, "Computer ROM Einstellungen" },
/* es */ { IDS_COMPUTER_ROM_SETTINGS_ES, "Ajustes ROM ordenador" },
/* fr */ { IDS_COMPUTER_ROM_SETTINGS_FR, "Paramètres ROM ordinateur" },
/* hu */ { IDS_COMPUTER_ROM_SETTINGS_HU, "Számítógép ROM beállításai" },
/* it */ { IDS_COMPUTER_ROM_SETTINGS_IT, "Impostazioni ROM del computer" },
/* ko */ { IDS_COMPUTER_ROM_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_COMPUTER_ROM_SETTINGS_NL, "Computer ROM instellingen" },
/* pl */ { IDS_COMPUTER_ROM_SETTINGS_PL, "Ustawienia ROM-u komputera" },
/* ru */ { IDS_COMPUTER_ROM_SETTINGS_RU, "Computer ROM settings" },
/* sv */ { IDS_COMPUTER_ROM_SETTINGS_SV, "ROM-inställningar för dator" },
/* tr */ { IDS_COMPUTER_ROM_SETTINGS_TR, "Bilgisayar ROM ayarlarý" },

/* en */ { IDS_DRIVE_ROM_SETTINGS,    "Drive ROM settings" },
/* da */ { IDS_DRIVE_ROM_SETTINGS_DA, "Drev ROM-indstillinger" },
/* de */ { IDS_DRIVE_ROM_SETTINGS_DE, "Floppy ROM Einstellungen" },
/* es */ { IDS_DRIVE_ROM_SETTINGS_ES, "Ajustes ROM unidad disco" },
/* fr */ { IDS_DRIVE_ROM_SETTINGS_FR, "Paramètres ROM Lecteur" },
/* hu */ { IDS_DRIVE_ROM_SETTINGS_HU, "Lemezegység ROM beállításai" },
/* it */ { IDS_DRIVE_ROM_SETTINGS_IT, "Impostazioni ROM del drive" },
/* ko */ { IDS_DRIVE_ROM_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_DRIVE_ROM_SETTINGS_NL, "Drive ROM instellingen" },
/* pl */ { IDS_DRIVE_ROM_SETTINGS_PL, "Ustawienia ROM-u napêdu" },
/* ru */ { IDS_DRIVE_ROM_SETTINGS_RU, "Drive ROM settings" },
/* sv */ { IDS_DRIVE_ROM_SETTINGS_SV, "ROM-inställningar för diskettenhet" },
/* tr */ { IDS_DRIVE_ROM_SETTINGS_TR, "Sürücü ROM ayarlarý" },

/* en */ { IDS_DIGIMAX_ENABLED,    "DigiMAX Enabled" },
/* da */ { IDS_DIGIMAX_ENABLED_DA, "DigiMAX aktivéret" },
/* de */ { IDS_DIGIMAX_ENABLED_DE, "DigiMAX aktiviert" },
/* es */ { IDS_DIGIMAX_ENABLED_ES, "Permitir DigiMAX" },
/* fr */ { IDS_DIGIMAX_ENABLED_FR, "DigiMAX Activé" },
/* hu */ { IDS_DIGIMAX_ENABLED_HU, "" },  /* fuzzy */
/* it */ { IDS_DIGIMAX_ENABLED_IT, "Attivato DigiMAX" },
/* ko */ { IDS_DIGIMAX_ENABLED_KO, "" },  /* fuzzy */
/* nl */ { IDS_DIGIMAX_ENABLED_NL, "DigiMAX ingeschakeld" },
/* pl */ { IDS_DIGIMAX_ENABLED_PL, "DigiMAX w³±czony" },
/* ru */ { IDS_DIGIMAX_ENABLED_RU, "DigiMAX Enabled" },
/* sv */ { IDS_DIGIMAX_ENABLED_SV, "DigiMAX aktiverad" },
/* tr */ { IDS_DIGIMAX_ENABLED_TR, "DigiMAX'ý Aktif Et" },

/* en */ { IDS_DIGIMAX_BASE,    "DigiMAX base" },
/* da */ { IDS_DIGIMAX_BASE_DA, "DigiMAX-base" },
/* de */ { IDS_DIGIMAX_BASE_DE, "DigiMAX Basis" },
/* es */ { IDS_DIGIMAX_BASE_ES, "DigiMAX base" },
/* fr */ { IDS_DIGIMAX_BASE_FR, "Base DigiMAX" },
/* hu */ { IDS_DIGIMAX_BASE_HU, "" },  /* fuzzy */
/* it */ { IDS_DIGIMAX_BASE_IT, "Indirizzo base DigiMAX" },
/* ko */ { IDS_DIGIMAX_BASE_KO, "" },  /* fuzzy */
/* nl */ { IDS_DIGIMAX_BASE_NL, "DigiMAX basisadres" },
/* pl */ { IDS_DIGIMAX_BASE_PL, "Baza DigiMAX" },
/* ru */ { IDS_DIGIMAX_BASE_RU, "DigiMAX base" },
/* sv */ { IDS_DIGIMAX_BASE_SV, "DigiMAX-bas" },
/* tr */ { IDS_DIGIMAX_BASE_TR, "DigiMAX taban" },

/* en */ { IDS_DIGIMAX_SETTINGS,    "DigiMAX settings" },
/* da */ { IDS_DIGIMAX_SETTINGS_DA, "DigiMAX-indstillinger" },
/* de */ { IDS_DIGIMAX_SETTINGS_DE, "DigiMAX Einstellungen" },
/* es */ { IDS_DIGIMAX_SETTINGS_ES, "Ajustes DigiMAX" },
/* fr */ { IDS_DIGIMAX_SETTINGS_FR, "Paramètres DigiMAX" },
/* hu */ { IDS_DIGIMAX_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_DIGIMAX_SETTINGS_IT, "Impostazioni DigiMAX" },
/* ko */ { IDS_DIGIMAX_SETTINGS_KO, "DigiMAX ¼³Á¤" },
/* nl */ { IDS_DIGIMAX_SETTINGS_NL, "DigiMAX instellingen" },
/* pl */ { IDS_DIGIMAX_SETTINGS_PL, "Ustawienia DigiMAX" },
/* ru */ { IDS_DIGIMAX_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_DIGIMAX_SETTINGS_SV, "DigiMAX-inställningar" },
/* tr */ { IDS_DIGIMAX_SETTINGS_TR, "DigiMAX ayarlarý" },

/* en */ { IDS_ASK,    "Ask" },
/* da */ { IDS_ASK_DA, "" },  /* fuzzy */
/* de */ { IDS_ASK_DE, "Nachfragen" },
/* es */ { IDS_ASK_ES, "Preguntar" },
/* fr */ { IDS_ASK_FR, "" },  /* fuzzy */
/* hu */ { IDS_ASK_HU, "" },  /* fuzzy */
/* it */ { IDS_ASK_IT, "Chiedi" },
/* ko */ { IDS_ASK_KO, "" },  /* fuzzy */
/* nl */ { IDS_ASK_NL, "Vraag" },
/* pl */ { IDS_ASK_PL, "Spytaj" },
/* ru */ { IDS_ASK_RU, "" },  /* fuzzy */
/* sv */ { IDS_ASK_SV, "Fråga" },
/* tr */ { IDS_ASK_TR, "" },  /* fuzzy */

/* en */ { IDS_CONTINUE,    "Continue" },
/* da */ { IDS_CONTINUE_DA, "Fortsæt" },
/* de */ { IDS_CONTINUE_DE, "Fortsetzen" },
/* es */ { IDS_CONTINUE_ES, "Continuar" },
/* fr */ { IDS_CONTINUE_FR, "" },  /* fuzzy */
/* hu */ { IDS_CONTINUE_HU, "Folytatás" },
/* it */ { IDS_CONTINUE_IT, "Continua" },
/* ko */ { IDS_CONTINUE_KO, "°è¼Ó" },
/* nl */ { IDS_CONTINUE_NL, "Doorgaan" },
/* pl */ { IDS_CONTINUE_PL, "Kontynuuj" },
/* ru */ { IDS_CONTINUE_RU, "¿àÞÔÞÛÖØâì" },
/* sv */ { IDS_CONTINUE_SV, "Fortsätt" },
/* tr */ { IDS_CONTINUE_TR, "Devam" },

/* en */ { IDS_START_MONITOR,    "Start monitor" },
/* da */ { IDS_START_MONITOR_DA, "" },  /* fuzzy */
/* de */ { IDS_START_MONITOR_DE, "Monitor aktivieren" },
/* es */ { IDS_START_MONITOR_ES, "Activar monitor" },
/* fr */ { IDS_START_MONITOR_FR, "" },  /* fuzzy */
/* hu */ { IDS_START_MONITOR_HU, "" },  /* fuzzy */
/* it */ { IDS_START_MONITOR_IT, "Attiva monitor" },
/* ko */ { IDS_START_MONITOR_KO, "" },  /* fuzzy */
/* nl */ { IDS_START_MONITOR_NL, "Start monitor" },
/* pl */ { IDS_START_MONITOR_PL, "Uruchom monitor" },
/* ru */ { IDS_START_MONITOR_RU, "" },  /* fuzzy */
/* sv */ { IDS_START_MONITOR_SV, "Starta monitor" },
/* tr */ { IDS_START_MONITOR_TR, "" },  /* fuzzy */

/* en */ { IDS_HARD_RESET,    "Hard reset" },
/* da */ { IDS_HARD_RESET_DA, "" },  /* fuzzy */
/* de */ { IDS_HARD_RESET_DE, "Harter Reset" },
/* es */ { IDS_HARD_RESET_ES, "Reiniciar en frio" },
/* fr */ { IDS_HARD_RESET_FR, "" },  /* fuzzy */
/* hu */ { IDS_HARD_RESET_HU, "" },  /* fuzzy */
/* it */ { IDS_HARD_RESET_IT, "Hard Reset" },
/* ko */ { IDS_HARD_RESET_KO, "" },  /* fuzzy */
/* nl */ { IDS_HARD_RESET_NL, "Harde reset" },
/* pl */ { IDS_HARD_RESET_PL, "Reset sprzêtu" },
/* ru */ { IDS_HARD_RESET_RU, "" },  /* fuzzy */
/* sv */ { IDS_HARD_RESET_SV, "Hård nollställning" },
/* tr */ { IDS_HARD_RESET_TR, "" },  /* fuzzy */

/* en */ { IDS_QUIT_EMULATOR,    "Quit emulator" },
/* da */ { IDS_QUIT_EMULATOR_DA, "" },  /* fuzzy */
/* de */ { IDS_QUIT_EMULATOR_DE, "Emulator beenden" },
/* es */ { IDS_QUIT_EMULATOR_ES, "Salir del emulador" },
/* fr */ { IDS_QUIT_EMULATOR_FR, "" },  /* fuzzy */
/* hu */ { IDS_QUIT_EMULATOR_HU, "" },  /* fuzzy */
/* it */ { IDS_QUIT_EMULATOR_IT, "Esci" },
/* ko */ { IDS_QUIT_EMULATOR_KO, "" },  /* fuzzy */
/* nl */ { IDS_QUIT_EMULATOR_NL, "Emulator afsluiten" },
/* pl */ { IDS_QUIT_EMULATOR_PL, "Wyj¶cie z emulatora" },
/* ru */ { IDS_QUIT_EMULATOR_RU, "" },  /* fuzzy */
/* sv */ { IDS_QUIT_EMULATOR_SV, "" },  /* fuzzy */
/* tr */ { IDS_QUIT_EMULATOR_TR, "" },  /* fuzzy */

/* en */ { IDS_JOYPORT_S_DEVICE,    "%s device" },
/* da */ { IDS_JOYPORT_S_DEVICE_DA, "" },  /* fuzzy */
/* de */ { IDS_JOYPORT_S_DEVICE_DE, "%s Gerät" },
/* es */ { IDS_JOYPORT_S_DEVICE_ES, "" },  /* fuzzy */
/* fr */ { IDS_JOYPORT_S_DEVICE_FR, "" },  /* fuzzy */
/* hu */ { IDS_JOYPORT_S_DEVICE_HU, "" },  /* fuzzy */
/* it */ { IDS_JOYPORT_S_DEVICE_IT, "" },  /* fuzzy */
/* ko */ { IDS_JOYPORT_S_DEVICE_KO, "" },  /* fuzzy */
/* nl */ { IDS_JOYPORT_S_DEVICE_NL, "%s apparaat" },
/* pl */ { IDS_JOYPORT_S_DEVICE_PL, "" },  /* fuzzy */
/* ru */ { IDS_JOYPORT_S_DEVICE_RU, "" },  /* fuzzy */
/* sv */ { IDS_JOYPORT_S_DEVICE_SV, "" },  /* fuzzy */
/* tr */ { IDS_JOYPORT_S_DEVICE_TR, "" },  /* fuzzy */

/* en */ { IDS_JOYPORT_SETTINGS,    "Control port settings" },
/* da */ { IDS_JOYPORT_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDS_JOYPORT_SETTINGS_DE, "Control Port Einstellungen" },
/* es */ { IDS_JOYPORT_SETTINGS_ES, "" },  /* fuzzy */
/* fr */ { IDS_JOYPORT_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDS_JOYPORT_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_JOYPORT_SETTINGS_IT, "" },  /* fuzzy */
/* ko */ { IDS_JOYPORT_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_JOYPORT_SETTINGS_NL, "Spel connector instellingen" },
/* pl */ { IDS_JOYPORT_SETTINGS_PL, "" },  /* fuzzy */
/* ru */ { IDS_JOYPORT_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_JOYPORT_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDS_JOYPORT_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDS_DEFAULT_JAM_ACTION,    "Default CPU JAM action" },
/* da */ { IDS_DEFAULT_JAM_ACTION_DA, "" },  /* fuzzy */
/* de */ { IDS_DEFAULT_JAM_ACTION_DE, "Standard CPU JAM Aktion" },
/* es */ { IDS_DEFAULT_JAM_ACTION_ES, "Acción CPU JAM por defecto" },
/* fr */ { IDS_DEFAULT_JAM_ACTION_FR, "" },  /* fuzzy */
/* hu */ { IDS_DEFAULT_JAM_ACTION_HU, "" },  /* fuzzy */
/* it */ { IDS_DEFAULT_JAM_ACTION_IT, "Azione predefinita al JAM della CPU" },
/* ko */ { IDS_DEFAULT_JAM_ACTION_KO, "" },  /* fuzzy */
/* nl */ { IDS_DEFAULT_JAM_ACTION_NL, "Standaard CPU VASTLOPER actie" },
/* pl */ { IDS_DEFAULT_JAM_ACTION_PL, "Domy¶lne dzia³anie zatkania CPU" },
/* ru */ { IDS_DEFAULT_JAM_ACTION_RU, "" },  /* fuzzy */
/* sv */ { IDS_DEFAULT_JAM_ACTION_SV, "" },  /* fuzzy */
/* tr */ { IDS_DEFAULT_JAM_ACTION_TR, "" },  /* fuzzy */

/* en */ { IDS_JAMACTION_SETTINGS,    "CPU JAM action settings" },
/* da */ { IDS_JAMACTION_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDS_JAMACTION_SETTINGS_DE, "CPU JAM Aktion Einstellungen" },
/* es */ { IDS_JAMACTION_SETTINGS_ES, "Ajustes acción CPU JAM" },
/* fr */ { IDS_JAMACTION_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDS_JAMACTION_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_JAMACTION_SETTINGS_IT, "Impostazioni azione al JAM della CPU" },
/* ko */ { IDS_JAMACTION_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_JAMACTION_SETTINGS_NL, "CPU JAM actie instellingen" },
/* pl */ { IDS_JAMACTION_SETTINGS_PL, "Ustawienia dzia³ania CPU JAM" },
/* ru */ { IDS_JAMACTION_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_JAMACTION_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDS_JAMACTION_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDS_DRIVESOUND_ENABLED,    "Drive sound enabled" },
/* da */ { IDS_DRIVESOUND_ENABLED_DA, "" },  /* fuzzy */
/* de */ { IDS_DRIVESOUND_ENABLED_DE, "Laufwerk Geräuschemulation aktivieren" },
/* es */ { IDS_DRIVESOUND_ENABLED_ES, "Permitir controlador de sonido" },
/* fr */ { IDS_DRIVESOUND_ENABLED_FR, "" },  /* fuzzy */
/* hu */ { IDS_DRIVESOUND_ENABLED_HU, "" },  /* fuzzy */
/* it */ { IDS_DRIVESOUND_ENABLED_IT, "Emulazione suoni del drive attiva" },
/* ko */ { IDS_DRIVESOUND_ENABLED_KO, "" },  /* fuzzy */
/* nl */ { IDS_DRIVESOUND_ENABLED_NL, "Drive geluiden ingeschakeld" },
/* pl */ { IDS_DRIVESOUND_ENABLED_PL, "W³±cz d¼wiêki napêdu" },
/* ru */ { IDS_DRIVESOUND_ENABLED_RU, "" },  /* fuzzy */
/* sv */ { IDS_DRIVESOUND_ENABLED_SV, "" },  /* fuzzy */
/* tr */ { IDS_DRIVESOUND_ENABLED_TR, "" },  /* fuzzy */

/* en */ { IDS_DRIVESOUND_SETTINGS,    "Drive sound settings" },
/* da */ { IDS_DRIVESOUND_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDS_DRIVESOUND_SETTINGS_DE, "Laufwerk Geräuschemulation Einstellungen" },
/* es */ { IDS_DRIVESOUND_SETTINGS_ES, "Ajustes controlador de sonido" },
/* fr */ { IDS_DRIVESOUND_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDS_DRIVESOUND_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_DRIVESOUND_SETTINGS_IT, "Impostazioni suoni del drive" },
/* ko */ { IDS_DRIVESOUND_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_DRIVESOUND_SETTINGS_NL, "Drive geluiden instellingen" },
/* pl */ { IDS_DRIVESOUND_SETTINGS_PL, "Ustawienia d¼wiêków napêdu" },
/* ru */ { IDS_DRIVESOUND_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_DRIVESOUND_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDS_DRIVESOUND_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDS_DS12C887RTC_ENABLED,    "DS12C887 RTC Enabled" },
/* da */ { IDS_DS12C887RTC_ENABLED_DA, "DS12C887 RTC Aktiveret" },
/* de */ { IDS_DS12C887RTC_ENABLED_DE, "DS12C887 RTC aktiviert" },
/* es */ { IDS_DS12C887RTC_ENABLED_ES, "DS12C887 RTC Permitida" },
/* fr */ { IDS_DS12C887RTC_ENABLED_FR, "DS12C887 RTC Activé" },
/* hu */ { IDS_DS12C887RTC_ENABLED_HU, "" },  /* fuzzy */
/* it */ { IDS_DS12C887RTC_ENABLED_IT, "Attivato DS12C887 RTC" },
/* ko */ { IDS_DS12C887RTC_ENABLED_KO, "" },  /* fuzzy */
/* nl */ { IDS_DS12C887RTC_ENABLED_NL, "DS12C887 Aan/Uit" },
/* pl */ { IDS_DS12C887RTC_ENABLED_PL, "W³±czenie DS12C887 RTC" },
/* ru */ { IDS_DS12C887RTC_ENABLED_RU, "DS12C887 RTC Enabled" },
/* sv */ { IDS_DS12C887RTC_ENABLED_SV, "DS12C887 RTC aktiverad" },
/* tr */ { IDS_DS12C887RTC_ENABLED_TR, "DS12C887 RTC Aktif" },

/* en */ { IDS_DS12C887RTC_BASE,    "DS12C887 RTC base" },
/* da */ { IDS_DS12C887RTC_BASE_DA, "DS12C887 RTC base" },
/* de */ { IDS_DS12C887RTC_BASE_DE, "DS12C887 RTC Base" },
/* es */ { IDS_DS12C887RTC_BASE_ES, "DS12C887 RTC base" },
/* fr */ { IDS_DS12C887RTC_BASE_FR, "Base RTC DS12C887" },
/* hu */ { IDS_DS12C887RTC_BASE_HU, "" },  /* fuzzy */
/* it */ { IDS_DS12C887RTC_BASE_IT, "Indirizzo base RTC DS12C887" },
/* ko */ { IDS_DS12C887RTC_BASE_KO, "" },  /* fuzzy */
/* nl */ { IDS_DS12C887RTC_BASE_NL, "DS12C887 RTC basisadres" },
/* pl */ { IDS_DS12C887RTC_BASE_PL, "Baza DS12C887 RTC" },
/* ru */ { IDS_DS12C887RTC_BASE_RU, "DS12C887 RTC base" },
/* sv */ { IDS_DS12C887RTC_BASE_SV, "DS12C887 RTC-bas" },
/* tr */ { IDS_DS12C887RTC_BASE_TR, "DS12C887 RTC tabaný" },

/* en */ { IDS_DS12C887RTC_RUN_MODE,    "DS12C887 RTC running mode" },
/* da */ { IDS_DS12C887RTC_RUN_MODE_DA, "" },  /* fuzzy */
/* de */ { IDS_DS12C887RTC_RUN_MODE_DE, "DS12C887 RTC Ausführungsmodus" },
/* es */ { IDS_DS12C887RTC_RUN_MODE_ES, "" },  /* fuzzy */
/* fr */ { IDS_DS12C887RTC_RUN_MODE_FR, "" },  /* fuzzy */
/* hu */ { IDS_DS12C887RTC_RUN_MODE_HU, "" },  /* fuzzy */
/* it */ { IDS_DS12C887RTC_RUN_MODE_IT, "" },  /* fuzzy */
/* ko */ { IDS_DS12C887RTC_RUN_MODE_KO, "" },  /* fuzzy */
/* nl */ { IDS_DS12C887RTC_RUN_MODE_NL, "DS12C887 RTC start modus" },
/* pl */ { IDS_DS12C887RTC_RUN_MODE_PL, "" },  /* fuzzy */
/* ru */ { IDS_DS12C887RTC_RUN_MODE_RU, "" },  /* fuzzy */
/* sv */ { IDS_DS12C887RTC_RUN_MODE_SV, "" },  /* fuzzy */
/* tr */ { IDS_DS12C887RTC_RUN_MODE_TR, "" },  /* fuzzy */

/* en */ { IDS_DS12C887RTC_SAVE,    "Enable DS12C887 RTC data saving" },
/* da */ { IDS_DS12C887RTC_SAVE_DA, "" },  /* fuzzy */
/* de */ { IDS_DS12C887RTC_SAVE_DE, "DS12C887 RTC Speicherung aktivieren" },
/* es */ { IDS_DS12C887RTC_SAVE_ES, "Permitir grabar datos de  DS12C887 RTC" },
/* fr */ { IDS_DS12C887RTC_SAVE_FR, "" },  /* fuzzy */
/* hu */ { IDS_DS12C887RTC_SAVE_HU, "" },  /* fuzzy */
/* it */ { IDS_DS12C887RTC_SAVE_IT, "Attiva salvataggio dei dati RTC del DS12C887" },
/* ko */ { IDS_DS12C887RTC_SAVE_KO, "" },  /* fuzzy */
/* nl */ { IDS_DS12C887RTC_SAVE_NL, "Opslaan van de DS12C887 RTC gegevens inschakelen" },
/* pl */ { IDS_DS12C887RTC_SAVE_PL, "W³±cz zapis zegara DS12C887" },
/* ru */ { IDS_DS12C887RTC_SAVE_RU, "" },  /* fuzzy */
/* sv */ { IDS_DS12C887RTC_SAVE_SV, "" },  /* fuzzy */
/* tr */ { IDS_DS12C887RTC_SAVE_TR, "" },  /* fuzzy */

/* en */ { IDS_DS12C887RTC_SETTINGS,    "DS12C887 RTC settings" },
/* da */ { IDS_DS12C887RTC_SETTINGS_DA, "DS12C887 RTC-indstillinger" },
/* de */ { IDS_DS12C887RTC_SETTINGS_DE, "DS12C887 RTC Einstellungen" },
/* es */ { IDS_DS12C887RTC_SETTINGS_ES, "Ajustes DS12C887 RTC" },
/* fr */ { IDS_DS12C887RTC_SETTINGS_FR, "Paramètres DS12C887 RTC" },
/* hu */ { IDS_DS12C887RTC_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_DS12C887RTC_SETTINGS_IT, "Impostazioni RTC DS12C887" },
/* ko */ { IDS_DS12C887RTC_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_DS12C887RTC_SETTINGS_NL, "DS12C887 RTC instellingen" },
/* pl */ { IDS_DS12C887RTC_SETTINGS_PL, "Ustawienia DS12C887 RTC" },
/* ru */ { IDS_DS12C887RTC_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_DS12C887RTC_SETTINGS_SV, "DS12C887 RTC-inställningar" },
/* tr */ { IDS_DS12C887RTC_SETTINGS_TR, "DS12C887 RTC ayarlarý" },

/* en */ { IDS_SFX_SE_ENABLED,    "SFX Sound Expander Enabled" },
/* da */ { IDS_SFX_SE_ENABLED_DA, "SFX Sound Expander Aktiveret" },
/* de */ { IDS_SFX_SE_ENABLED_DE, "SFX Sound Expander aktiviert" },
/* es */ { IDS_SFX_SE_ENABLED_ES, "Permitir SFX Sound Expander" },
/* fr */ { IDS_SFX_SE_ENABLED_FR, "SFX Sound Expander Activé" },
/* hu */ { IDS_SFX_SE_ENABLED_HU, "SFX Sound Expander engedélyezve" },
/* it */ { IDS_SFX_SE_ENABLED_IT, "Attivato SFX Sound Expander" },
/* ko */ { IDS_SFX_SE_ENABLED_KO, "" },  /* fuzzy */
/* nl */ { IDS_SFX_SE_ENABLED_NL, "SFX Sound Expander ingeschakeld" },
/* pl */ { IDS_SFX_SE_ENABLED_PL, "SFX Sound Expander w³±czony" },
/* ru */ { IDS_SFX_SE_ENABLED_RU, "SFX Sound Expander Enabled" },
/* sv */ { IDS_SFX_SE_ENABLED_SV, "SFX Sound Expander aktiverad" },
/* tr */ { IDS_SFX_SE_ENABLED_TR, "SFX Sound Expander Aktif" },

/* en */ { IDS_SFX_SE_CHIP,    "SFX Sound Expander Chip" },
/* da */ { IDS_SFX_SE_CHIP_DA, "SFX Sound Expander chip" },
/* de */ { IDS_SFX_SE_CHIP_DE, "SFX Sound Expander Chip" },
/* es */ { IDS_SFX_SE_CHIP_ES, "SFX Sound Expander Chip" },
/* fr */ { IDS_SFX_SE_CHIP_FR, "Puce SFX Sound Expander" },
/* hu */ { IDS_SFX_SE_CHIP_HU, "SFX Sound Expander csip" },
/* it */ { IDS_SFX_SE_CHIP_IT, "Chip SFX Sound Expander" },
/* ko */ { IDS_SFX_SE_CHIP_KO, "" },  /* fuzzy */
/* nl */ { IDS_SFX_SE_CHIP_NL, "SFX Sound Expander Chip" },
/* pl */ { IDS_SFX_SE_CHIP_PL, "Uk³ad SFX Sound Expander" },
/* ru */ { IDS_SFX_SE_CHIP_RU, "SFX Sound Expander Chip" },
/* sv */ { IDS_SFX_SE_CHIP_SV, "SFX Sound Expander-krets" },
/* tr */ { IDS_SFX_SE_CHIP_TR, "SFX Sound Expander Çipi" },

/* en */ { IDS_SFX_SS_ENABLED,    "SFX Sound Sampler Enabled" },
/* da */ { IDS_SFX_SS_ENABLED_DA, "SFX Sound Sampler Aktiveret" },
/* de */ { IDS_SFX_SS_ENABLED_DE, "SFX Sound Sampler aktiviert" },
/* es */ { IDS_SFX_SS_ENABLED_ES, "Permitir SFX Sound Sampler" },
/* fr */ { IDS_SFX_SS_ENABLED_FR, "SFX Sound Sampler Activé" },
/* hu */ { IDS_SFX_SS_ENABLED_HU, "" },  /* fuzzy */
/* it */ { IDS_SFX_SS_ENABLED_IT, "Attivato SFX Sound Sampler" },
/* ko */ { IDS_SFX_SS_ENABLED_KO, "" },  /* fuzzy */
/* nl */ { IDS_SFX_SS_ENABLED_NL, "SFX Sound Sampler ingeschakeld" },
/* pl */ { IDS_SFX_SS_ENABLED_PL, "W³±czony sampler d¼wiêku SFX" },
/* ru */ { IDS_SFX_SS_ENABLED_RU, "" },  /* fuzzy */
/* sv */ { IDS_SFX_SS_ENABLED_SV, "SFX Sound Sampler aktiverad" },
/* tr */ { IDS_SFX_SS_ENABLED_TR, "SFX Sound Sampler Aktif" },

/* en */ { IDS_SFX_SE_SETTINGS,    "SFX Sound Expander settings" },
/* da */ { IDS_SFX_SE_SETTINGS_DA, "SFX Sound Expander-indstillinger" },
/* de */ { IDS_SFX_SE_SETTINGS_DE, "SFX Sound Expander Einstellungen" },
/* es */ { IDS_SFX_SE_SETTINGS_ES, "Ajustes SFX Sound Expander" },
/* fr */ { IDS_SFX_SE_SETTINGS_FR, "Paramètres du SFX Sound Expander" },
/* hu */ { IDS_SFX_SE_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_SFX_SE_SETTINGS_IT, "Impostazioni SFX Sound Expander" },
/* ko */ { IDS_SFX_SE_SETTINGS_KO, "SFX Sound Expander ¼³Á¤" },
/* nl */ { IDS_SFX_SE_SETTINGS_NL, "SFX Sound Expander instellingen" },
/* pl */ { IDS_SFX_SE_SETTINGS_PL, "Ustawienia SFX Sound Expander" },
/* ru */ { IDS_SFX_SE_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_SFX_SE_SETTINGS_SV, "Inställningar för SFX Sound Expander" },
/* tr */ { IDS_SFX_SE_SETTINGS_TR, "SFX Sound Expander Ayarlarý" },

/* en */ { IDS_SFX_SS_SETTINGS,    "SFX Sound Sampler settings" },
/* da */ { IDS_SFX_SS_SETTINGS_DA, "SFX Sound Sampler-indstillinger" },
/* de */ { IDS_SFX_SS_SETTINGS_DE, "SFX Sound Sampler Einstellungen" },
/* es */ { IDS_SFX_SS_SETTINGS_ES, "Ajustes SFX Sound Sampler" },
/* fr */ { IDS_SFX_SS_SETTINGS_FR, "Paramètres du SFX Sound Sampler" },
/* hu */ { IDS_SFX_SS_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_SFX_SS_SETTINGS_IT, "Impostazioni SFX Sound Sampler" },
/* ko */ { IDS_SFX_SS_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_SFX_SS_SETTINGS_NL, "SFX Sound Sampler instellingen" },
/* pl */ { IDS_SFX_SS_SETTINGS_PL, "Ustawienia samplera d¼wiêku SFX" },
/* ru */ { IDS_SFX_SS_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_SFX_SS_SETTINGS_SV, "Inställningar för SFX Sound Sampler" },
/* tr */ { IDS_SFX_SS_SETTINGS_TR, "SFX Sound Sampler ayarlarý" },

/* en */ { IDS_SCPU64_JIFFY_ENABLED,    "Jiffy switch enabled" },
/* da */ { IDS_SCPU64_JIFFY_ENABLED_DA, "" },  /* fuzzy */
/* de */ { IDS_SCPU64_JIFFY_ENABLED_DE, "Jiffy Schalter aktiviert" },
/* es */ { IDS_SCPU64_JIFFY_ENABLED_ES, "Permitir conmutador Jiffy" },
/* fr */ { IDS_SCPU64_JIFFY_ENABLED_FR, "Jiffy switch activée" },
/* hu */ { IDS_SCPU64_JIFFY_ENABLED_HU, "" },  /* fuzzy */
/* it */ { IDS_SCPU64_JIFFY_ENABLED_IT, "Pulsante Jiffy attivo" },
/* ko */ { IDS_SCPU64_JIFFY_ENABLED_KO, "" },  /* fuzzy */
/* nl */ { IDS_SCPU64_JIFFY_ENABLED_NL, "Jiffy schakelaar ingeschakeld" },
/* pl */ { IDS_SCPU64_JIFFY_ENABLED_PL, "Prze³±cznik jiffy w³±czony" },
/* ru */ { IDS_SCPU64_JIFFY_ENABLED_RU, "" },  /* fuzzy */
/* sv */ { IDS_SCPU64_JIFFY_ENABLED_SV, "jiffyomkopplare aktiverad" },
/* tr */ { IDS_SCPU64_JIFFY_ENABLED_TR, "" },  /* fuzzy */

/* en */ { IDS_SCPU64_SPEED_ENABLED,    "Speed switch enabled" },
/* da */ { IDS_SCPU64_SPEED_ENABLED_DA, "" },  /* fuzzy */
/* de */ { IDS_SCPU64_SPEED_ENABLED_DE, "Speed Schalter aktiviert" },
/* es */ { IDS_SCPU64_SPEED_ENABLED_ES, "Permitir conmutador de velocidad" },
/* fr */ { IDS_SCPU64_SPEED_ENABLED_FR, "Speed switch activée" },
/* hu */ { IDS_SCPU64_SPEED_ENABLED_HU, "" },  /* fuzzy */
/* it */ { IDS_SCPU64_SPEED_ENABLED_IT, "Pulsante di velocità attivo" },
/* ko */ { IDS_SCPU64_SPEED_ENABLED_KO, "" },  /* fuzzy */
/* nl */ { IDS_SCPU64_SPEED_ENABLED_NL, "Snelheid schakelaar ingeschakeld" },
/* pl */ { IDS_SCPU64_SPEED_ENABLED_PL, "Prze³±cznik speed w³±czony" },
/* ru */ { IDS_SCPU64_SPEED_ENABLED_RU, "" },  /* fuzzy */
/* sv */ { IDS_SCPU64_SPEED_ENABLED_SV, "Hastighetsomkopplare aktiverad" },
/* tr */ { IDS_SCPU64_SPEED_ENABLED_TR, "" },  /* fuzzy */

/* en */ { IDS_SCPU64_SIMM_SIZE,    "SIMM size" },
/* da */ { IDS_SCPU64_SIMM_SIZE_DA, "" },  /* fuzzy */
/* de */ { IDS_SCPU64_SIMM_SIZE_DE, "SIMM Größe" },
/* es */ { IDS_SCPU64_SIMM_SIZE_ES, "Tamaño SIMM" },
/* fr */ { IDS_SCPU64_SIMM_SIZE_FR, "Taille de la SIMM" },
/* hu */ { IDS_SCPU64_SIMM_SIZE_HU, "" },  /* fuzzy */
/* it */ { IDS_SCPU64_SIMM_SIZE_IT, "Dimensione SIMM" },
/* ko */ { IDS_SCPU64_SIMM_SIZE_KO, "" },  /* fuzzy */
/* nl */ { IDS_SCPU64_SIMM_SIZE_NL, "SIMM grootte" },
/* pl */ { IDS_SCPU64_SIMM_SIZE_PL, "Ilo¶æ SIMM" },
/* ru */ { IDS_SCPU64_SIMM_SIZE_RU, "" },  /* fuzzy */
/* sv */ { IDS_SCPU64_SIMM_SIZE_SV, "SIMM-storlek" },
/* tr */ { IDS_SCPU64_SIMM_SIZE_TR, "" },  /* fuzzy */

/* en */ { IDS_SCPU64_SETTINGS,    "SuperCPU64 settings" },
/* da */ { IDS_SCPU64_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDS_SCPU64_SETTINGS_DE, "SuperCPU64 Einstellungen" },
/* es */ { IDS_SCPU64_SETTINGS_ES, "Ajustes SuperCPU64" },
/* fr */ { IDS_SCPU64_SETTINGS_FR, "Paramètres SuperCPU64" },
/* hu */ { IDS_SCPU64_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_SCPU64_SETTINGS_IT, "Impostazioni SuperCPU64" },
/* ko */ { IDS_SCPU64_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_SCPU64_SETTINGS_NL, "SuperCPU64 instellingen" },
/* pl */ { IDS_SCPU64_SETTINGS_PL, "Ustawienia SuperCPU64" },
/* ru */ { IDS_SCPU64_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_SCPU64_SETTINGS_SV, "SuperCPU64-inställningar" },
/* tr */ { IDS_SCPU64_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDS_SOUND_RECORDING_STOPPED,    "Sound Recording stopped..." },
/* da */ { IDS_SOUND_RECORDING_STOPPED_DA, "Lydoptagelse stoppet..." },
/* de */ { IDS_SOUND_RECORDING_STOPPED_DE, "Tonaufnahme gestoppt..." },
/* es */ { IDS_SOUND_RECORDING_STOPPED_ES, "Grabación de sonido detenida..." },
/* fr */ { IDS_SOUND_RECORDING_STOPPED_FR, "Enregistrement du son arrêté..." },
/* hu */ { IDS_SOUND_RECORDING_STOPPED_HU, "" },  /* fuzzy */
/* it */ { IDS_SOUND_RECORDING_STOPPED_IT, "Registrazione audio interrotta..." },
/* ko */ { IDS_SOUND_RECORDING_STOPPED_KO, "»ç¿îµå ·¹ÄÚµù ¸ØÃã..." },
/* nl */ { IDS_SOUND_RECORDING_STOPPED_NL, "Geluidsopname gestopt..." },
/* pl */ { IDS_SOUND_RECORDING_STOPPED_PL, "Zatrzymano odtwarzanie d¼wiêku..." },
/* ru */ { IDS_SOUND_RECORDING_STOPPED_RU, "·ÐßØáì ×ÒãÚÐ ÞáâàÐÝÞÒÛÕÝÐ..." },
/* sv */ { IDS_SOUND_RECORDING_STOPPED_SV, "Ljudinspelning stoppad..." },
/* tr */ { IDS_SOUND_RECORDING_STOPPED_TR, "Ses Kaydý durduruldu..." },

/* en */ { IDS_SOUND_RECORDING_STARTED,    "Sound Recording started..." },
/* da */ { IDS_SOUND_RECORDING_STARTED_DA, "Lydoptagelse startet..." },
/* de */ { IDS_SOUND_RECORDING_STARTED_DE, "Tonaufnahme gestartet..." },
/* es */ { IDS_SOUND_RECORDING_STARTED_ES, "Grabación de sonido iniciada..." },
/* fr */ { IDS_SOUND_RECORDING_STARTED_FR, "Démarrage de l'enregistrement du son..." },
/* hu */ { IDS_SOUND_RECORDING_STARTED_HU, "" },  /* fuzzy */
/* it */ { IDS_SOUND_RECORDING_STARTED_IT, "Registrazione audio avviata..." },
/* ko */ { IDS_SOUND_RECORDING_STARTED_KO, "»ç¿îµå ·¹ÄÚµùÀÌ ½ÃÀÛÇß½À´Ï´Ù..." },
/* nl */ { IDS_SOUND_RECORDING_STARTED_NL, "Geluidsopname gestart..." },
/* pl */ { IDS_SOUND_RECORDING_STARTED_PL, "Rozpoczêto odtwarzanie d¼wiêku..." },
/* ru */ { IDS_SOUND_RECORDING_STARTED_RU, "·ÐßØáì ×ÒãÚÐ ÝÐçÐâÐ..." },
/* sv */ { IDS_SOUND_RECORDING_STARTED_SV, "Ljudinspelning startad..." },
/* tr */ { IDS_SOUND_RECORDING_STARTED_TR, "Ses Kaydý baþladý..." },

/* en */ { IDS_SELECT_RECORD_FILE,    "Select Sound Record File" },
/* da */ { IDS_SELECT_RECORD_FILE_DA, "Vælg lydoptagelsesfil" },
/* de */ { IDS_SELECT_RECORD_FILE_DE, "Tonaufnahmedatei wählen" },
/* es */ { IDS_SELECT_RECORD_FILE_ES, "Seleccionar fichero grabación sonido" },
/* fr */ { IDS_SELECT_RECORD_FILE_FR, "Sélectionnez le fichier d'enregistrement du son" },
/* hu */ { IDS_SELECT_RECORD_FILE_HU, "Válassza ki a hangfelvétel kimeneti fájlt" },
/* it */ { IDS_SELECT_RECORD_FILE_IT, "Seleziona file per registrazione audio" },
/* ko */ { IDS_SELECT_RECORD_FILE_KO, "" },  /* fuzzy */
/* nl */ { IDS_SELECT_RECORD_FILE_NL, "Selecteer geluidsopnamebestand" },
/* pl */ { IDS_SELECT_RECORD_FILE_PL, "Wybierz plik zapisu d¼wiêku" },
/* ru */ { IDS_SELECT_RECORD_FILE_RU, "Select Sound Record File" },
/* sv */ { IDS_SELECT_RECORD_FILE_SV, "Välj ljudinspelningsfil" },
/* tr */ { IDS_SELECT_RECORD_FILE_TR, "Ses Kayýt Dosyasý Seçin" },

/* en */ { IDS_SOUND_RECORD_FORMAT,    "Sound Record Format" },
/* da */ { IDS_SOUND_RECORD_FORMAT_DA, "Format for lydoptagelse" },
/* de */ { IDS_SOUND_RECORD_FORMAT_DE, "Tonaufnahmeformat" },
/* es */ { IDS_SOUND_RECORD_FORMAT_ES, "Formato grabación de sonido" },
/* fr */ { IDS_SOUND_RECORD_FORMAT_FR, "Format de l'enregistrement du son" },
/* hu */ { IDS_SOUND_RECORD_FORMAT_HU, "Hangfelvétel formátuma" },
/* it */ { IDS_SOUND_RECORD_FORMAT_IT, "Formato registrazione audio" },
/* ko */ { IDS_SOUND_RECORD_FORMAT_KO, "»ç¿îµå ·¹ÄÚµå Æ÷¸Ë" },
/* nl */ { IDS_SOUND_RECORD_FORMAT_NL, "Geluidsopname Formaat" },
/* pl */ { IDS_SOUND_RECORD_FORMAT_PL, "Format zapisu d¼wiêku" },
/* ru */ { IDS_SOUND_RECORD_FORMAT_RU, "Sound Record Format" },
/* sv */ { IDS_SOUND_RECORD_FORMAT_SV, "Format för ljudinspelning" },
/* tr */ { IDS_SOUND_RECORD_FORMAT_TR, "Ses Kayýt Biçimi" },

/* en */ { IDS_SOUND_RECORD_FILE,    "Sound Record File" },
/* da */ { IDS_SOUND_RECORD_FILE_DA, "Lydoptagelsesfil" },
/* de */ { IDS_SOUND_RECORD_FILE_DE, "Tonaufnahmedatei" },
/* es */ { IDS_SOUND_RECORD_FILE_ES, "Fichero grabación de sonido" },
/* fr */ { IDS_SOUND_RECORD_FILE_FR, "Fichier d'enregistrement du son" },
/* hu */ { IDS_SOUND_RECORD_FILE_HU, "Hangfelvétel fájl" },
/* it */ { IDS_SOUND_RECORD_FILE_IT, "File registrazione audio" },
/* ko */ { IDS_SOUND_RECORD_FILE_KO, "»ç¿îµå ·¹ÄÚµå ÆÄÀÏ" },
/* nl */ { IDS_SOUND_RECORD_FILE_NL, "Geluid opnamebestand" },
/* pl */ { IDS_SOUND_RECORD_FILE_PL, "Plik zapisu d¼wiêku" },
/* ru */ { IDS_SOUND_RECORD_FILE_RU, "Sound Record File" },
/* sv */ { IDS_SOUND_RECORD_FILE_SV, "Ljudinspelningsfil" },
/* tr */ { IDS_SOUND_RECORD_FILE_TR, "Ses Kayýt Dosyasý" },

/* en */ { IDS_SOUND_RECORD_SETTINGS,    "Sound record settings" },
/* da */ { IDS_SOUND_RECORD_SETTINGS_DA, "Lydoptagelsesindstillinger" },
/* de */ { IDS_SOUND_RECORD_SETTINGS_DE, "Tonaufnahme Einstellungen" },
/* es */ { IDS_SOUND_RECORD_SETTINGS_ES, "Ajustes grabación de sonido" },
/* fr */ { IDS_SOUND_RECORD_SETTINGS_FR, "Paramètres de l'enregistrement du son" },
/* hu */ { IDS_SOUND_RECORD_SETTINGS_HU, "Hangfelvétel beállításai" },
/* it */ { IDS_SOUND_RECORD_SETTINGS_IT, "Impostazioni registrazione audio" },
/* ko */ { IDS_SOUND_RECORD_SETTINGS_KO, "»ç¿îµå ·¹ÄÚµå ¼ÂÆÃ" },
/* nl */ { IDS_SOUND_RECORD_SETTINGS_NL, "Geluidsopname instellingen" },
/* pl */ { IDS_SOUND_RECORD_SETTINGS_PL, "Ustawienia zapisu d¼wiêku" },
/* ru */ { IDS_SOUND_RECORD_SETTINGS_RU, "½ÐáâàÞÙÚØ ×ÐßØáØ ×ÒãÚÐ" },
/* sv */ { IDS_SOUND_RECORD_SETTINGS_SV, "Inställningar för ljudinspelning" },
/* tr */ { IDS_SOUND_RECORD_SETTINGS_TR, "Ses kayýt ayarlarý" },

/* en */ { IDS_VERY_SMALL,    "Very small" },
/* da */ { IDS_VERY_SMALL_DA, "" },  /* fuzzy */
/* de */ { IDS_VERY_SMALL_DE, "Sehr klein" },
/* es */ { IDS_VERY_SMALL_ES, "Muy pequeño" },
/* fr */ { IDS_VERY_SMALL_FR, "" },  /* fuzzy */
/* hu */ { IDS_VERY_SMALL_HU, "" },  /* fuzzy */
/* it */ { IDS_VERY_SMALL_IT, "Molto piccolo" },
/* ko */ { IDS_VERY_SMALL_KO, "" },  /* fuzzy */
/* nl */ { IDS_VERY_SMALL_NL, "Erg klein" },
/* pl */ { IDS_VERY_SMALL_PL, "Bardzo ma³y" },
/* ru */ { IDS_VERY_SMALL_RU, "" },  /* fuzzy */
/* sv */ { IDS_VERY_SMALL_SV, "Väldigt liten" },
/* tr */ { IDS_VERY_SMALL_TR, "" },  /* fuzzy */

/* en */ { IDS_SMALL,    "Small" },
/* da */ { IDS_SMALL_DA, "Lille" },
/* de */ { IDS_SMALL_DE, "Klein" },
/* es */ { IDS_SMALL_ES, "Pequeño" },
/* fr */ { IDS_SMALL_FR, "Petit" },
/* hu */ { IDS_SMALL_HU, "Kicsi" },
/* it */ { IDS_SMALL_IT, "Piccolo" },
/* ko */ { IDS_SMALL_KO, "ÀÛ´Ù" },
/* nl */ { IDS_SMALL_NL, "Klein" },
/* pl */ { IDS_SMALL_PL, "Ma³y" },
/* ru */ { IDS_SMALL_RU, "¼ÐÛëÙ" },
/* sv */ { IDS_SMALL_SV, "Liten" },
/* tr */ { IDS_SMALL_TR, "Küçük" },

/* en */ { IDS_MEDIUM,    "Medium" },
/* da */ { IDS_MEDIUM_DA, "Medium" },
/* de */ { IDS_MEDIUM_DE, "Mittel" },
/* es */ { IDS_MEDIUM_ES, "Medio" },
/* fr */ { IDS_MEDIUM_FR, "Moyen" },
/* hu */ { IDS_MEDIUM_HU, "Közepes" },
/* it */ { IDS_MEDIUM_IT, "Medio" },
/* ko */ { IDS_MEDIUM_KO, "Áß°£" },
/* nl */ { IDS_MEDIUM_NL, "Middelmatig" },
/* pl */ { IDS_MEDIUM_PL, "¦redni" },
/* ru */ { IDS_MEDIUM_RU, "ÁàÕÔÝØÙ" },
/* sv */ { IDS_MEDIUM_SV, "Medium" },
/* tr */ { IDS_MEDIUM_TR, "Orta" },

/* en */ { IDS_LARGE,    "Large" },
/* da */ { IDS_LARGE_DA, "Stor" },
/* de */ { IDS_LARGE_DE, "Groß" },
/* es */ { IDS_LARGE_ES, "Grande" },
/* fr */ { IDS_LARGE_FR, "Grand" },
/* hu */ { IDS_LARGE_HU, "Nagy" },
/* it */ { IDS_LARGE_IT, "Grande" },
/* ko */ { IDS_LARGE_KO, "Å«" },
/* nl */ { IDS_LARGE_NL, "Groot" },
/* pl */ { IDS_LARGE_PL, "Wielki" },
/* ru */ { IDS_LARGE_RU, "±ÞÛìèÞÙ" },
/* sv */ { IDS_LARGE_SV, "Stor" },
/* tr */ { IDS_LARGE_TR, "Büyük" },

/* en */ { IDS_VERY_LARGE,    "Very large" },
/* da */ { IDS_VERY_LARGE_DA, "" },  /* fuzzy */
/* de */ { IDS_VERY_LARGE_DE, "Sehr groß" },
/* es */ { IDS_VERY_LARGE_ES, "Muy grande" },
/* fr */ { IDS_VERY_LARGE_FR, "" },  /* fuzzy */
/* hu */ { IDS_VERY_LARGE_HU, "" },  /* fuzzy */
/* it */ { IDS_VERY_LARGE_IT, "Molto grande" },
/* ko */ { IDS_VERY_LARGE_KO, "" },  /* fuzzy */
/* nl */ { IDS_VERY_LARGE_NL, "Erg groot" },
/* pl */ { IDS_VERY_LARGE_PL, "Bardzo du¿y" },
/* ru */ { IDS_VERY_LARGE_RU, "" },  /* fuzzy */
/* sv */ { IDS_VERY_LARGE_SV, "Väldigt stor" },
/* tr */ { IDS_VERY_LARGE_TR, "" },  /* fuzzy */

/* en */ { IDS_BORDER_MODE,    "Border mode" },
/* da */ { IDS_BORDER_MODE_DA, "Ramme-tilstand" },
/* de */ { IDS_BORDER_MODE_DE, "Rahmen Modus" },
/* es */ { IDS_BORDER_MODE_ES, "Modo del borde" },
/* fr */ { IDS_BORDER_MODE_FR, "Mode Bordure" },
/* hu */ { IDS_BORDER_MODE_HU, "Keret mód" },
/* it */ { IDS_BORDER_MODE_IT, "Modalità del bordo" },
/* ko */ { IDS_BORDER_MODE_KO, "º¸´õ ¸ðµå" },
/* nl */ { IDS_BORDER_MODE_NL, "Bordermodus" },
/* pl */ { IDS_BORDER_MODE_PL, "Tryb ramki" },
/* ru */ { IDS_BORDER_MODE_RU, "Border mode" },
/* sv */ { IDS_BORDER_MODE_SV, "Ramläge" },
/* tr */ { IDS_BORDER_MODE_TR, "Çerçeve modu" },

/* en */ { IDMS_NORMAL,    "Normal" },
/* da */ { IDMS_NORMAL_DA, "Normal" },
/* de */ { IDMS_NORMAL_DE, "Normal" },
/* es */ { IDMS_NORMAL_ES, "Normal" },
/* fr */ { IDMS_NORMAL_FR, "Normal" },
/* hu */ { IDMS_NORMAL_HU, "Normál" },
/* it */ { IDMS_NORMAL_IT, "Normale" },
/* ko */ { IDMS_NORMAL_KO, "º¸Åë" },
/* nl */ { IDMS_NORMAL_NL, "Normaal" },
/* pl */ { IDMS_NORMAL_PL, "Zwyk³y" },
/* ru */ { IDMS_NORMAL_RU, "Normal" },
/* sv */ { IDMS_NORMAL_SV, "Normal" },
/* tr */ { IDMS_NORMAL_TR, "Normal" },

/* en */ { IDS_FULL,    "Full" },
/* da */ { IDS_FULL_DA, "Fuld" },
/* de */ { IDS_FULL_DE, "Full" },
/* es */ { IDS_FULL_ES, "Completo" },
/* fr */ { IDS_FULL_FR, "Complet" },
/* hu */ { IDS_FULL_HU, "Teljes" },
/* it */ { IDS_FULL_IT, "Intero" },
/* ko */ { IDS_FULL_KO, "°¡µæ" },
/* nl */ { IDS_FULL_NL, "Volledig" },
/* pl */ { IDS_FULL_PL, "Pe³ny" },
/* ru */ { IDS_FULL_RU, "Full" },
/* sv */ { IDS_FULL_SV, "Full" },
/* tr */ { IDS_FULL_TR, "Tam" },

/* en */ { IDS_DEBUG,    "Debug" },
/* da */ { IDS_DEBUG_DA, "Debug" },
/* de */ { IDS_DEBUG_DE, "Debug" },
/* es */ { IDS_DEBUG_ES, "Depurar" },
/* fr */ { IDS_DEBUG_FR, "Débug" },
/* hu */ { IDS_DEBUG_HU, "Nyomkövetés" },
/* it */ { IDS_DEBUG_IT, "Debug" },
/* ko */ { IDS_DEBUG_KO, "µð¹ö±×" },
/* nl */ { IDS_DEBUG_NL, "Debug" },
/* pl */ { IDS_DEBUG_PL, "Debug" },
/* ru */ { IDS_DEBUG_RU, "Debug" },
/* sv */ { IDS_DEBUG_SV, "Felsök" },
/* tr */ { IDS_DEBUG_TR, "Hata Ayýklama" },

/* en */ { IDS_VOLUME,    "Volume (0-100%)" },
/* da */ { IDS_VOLUME_DA, "Volume (0-100 %)" },
/* de */ { IDS_VOLUME_DE, "Lautstärke (0-100%)" },
/* es */ { IDS_VOLUME_ES, "Volumen (0-100%)" },
/* fr */ { IDS_VOLUME_FR, "Volume (0-100%)" },
/* hu */ { IDS_VOLUME_HU, "Hangerõ (0-100%)" },
/* it */ { IDS_VOLUME_IT, "Volume (0-100%)" },
/* ko */ { IDS_VOLUME_KO, "º¼·ý (0-100%)" },
/* nl */ { IDS_VOLUME_NL, "Volume (0-100%)" },
/* pl */ { IDS_VOLUME_PL, "G³o¶no¶æ (0-100%)" },
/* ru */ { IDS_VOLUME_RU, "Volume (0-100%)" },
/* sv */ { IDS_VOLUME_SV, "Volym (0-100 %)" },
/* tr */ { IDS_VOLUME_TR, "Ses Düzeyi (0-100%)" },

/* en */ { IDS_RANDOM_TAPE_WOBBLE,    "Random tape wobble" },
/* da */ { IDS_RANDOM_TAPE_WOBBLE_DA, "" },  /* fuzzy */
/* de */ { IDS_RANDOM_TAPE_WOBBLE_DE, "Zufälliges Bandeiern" },
/* es */ { IDS_RANDOM_TAPE_WOBBLE_ES, "" },  /* fuzzy */
/* fr */ { IDS_RANDOM_TAPE_WOBBLE_FR, "" },  /* fuzzy */
/* hu */ { IDS_RANDOM_TAPE_WOBBLE_HU, "" },  /* fuzzy */
/* it */ { IDS_RANDOM_TAPE_WOBBLE_IT, "" },  /* fuzzy */
/* ko */ { IDS_RANDOM_TAPE_WOBBLE_KO, "" },  /* fuzzy */
/* nl */ { IDS_RANDOM_TAPE_WOBBLE_NL, "Willekeurige tape wiebel" },
/* pl */ { IDS_RANDOM_TAPE_WOBBLE_PL, "" },  /* fuzzy */
/* ru */ { IDS_RANDOM_TAPE_WOBBLE_RU, "" },  /* fuzzy */
/* sv */ { IDS_RANDOM_TAPE_WOBBLE_SV, "" },  /* fuzzy */
/* tr */ { IDS_RANDOM_TAPE_WOBBLE_TR, "" },  /* fuzzy */

/* en */ { IDS_SOUND_OUTPUT_MODE,    "Sound output mode" },
/* da */ { IDS_SOUND_OUTPUT_MODE_DA, "Lydindstillinger" },
/* de */ { IDS_SOUND_OUTPUT_MODE_DE, "Ton Ausgabe Modus" },
/* es */ { IDS_SOUND_OUTPUT_MODE_ES, "Modo salida sonido" },
/* fr */ { IDS_SOUND_OUTPUT_MODE_FR, "Mode de sortie de son" },
/* hu */ { IDS_SOUND_OUTPUT_MODE_HU, "" },  /* fuzzy */
/* it */ { IDS_SOUND_OUTPUT_MODE_IT, "Modalità di uscita audio" },
/* ko */ { IDS_SOUND_OUTPUT_MODE_KO, "" },  /* fuzzy */
/* nl */ { IDS_SOUND_OUTPUT_MODE_NL, "Geluid uitvoer modus" },
/* pl */ { IDS_SOUND_OUTPUT_MODE_PL, "Tryb wyj¶cia d¼wiêku" },
/* ru */ { IDS_SOUND_OUTPUT_MODE_RU, "" },  /* fuzzy */
/* sv */ { IDS_SOUND_OUTPUT_MODE_SV, "Utdataläge för ljud" },
/* tr */ { IDS_SOUND_OUTPUT_MODE_TR, "Ses çýkýþý modu" },

/* en */ { IDS_SYSTEM,    "System" },
/* da */ { IDS_SYSTEM_DA, "System" },
/* de */ { IDS_SYSTEM_DE, "System" },
/* es */ { IDS_SYSTEM_ES, "Sistema" },
/* fr */ { IDS_SYSTEM_FR, "Système" },
/* hu */ { IDS_SYSTEM_HU, "" },  /* fuzzy */
/* it */ { IDS_SYSTEM_IT, "Sistema" },
/* ko */ { IDS_SYSTEM_KO, "" },  /* fuzzy */
/* nl */ { IDS_SYSTEM_NL, "Systeem" },
/* pl */ { IDS_SYSTEM_PL, "System" },
/* ru */ { IDS_SYSTEM_RU, "System" },
/* sv */ { IDS_SYSTEM_SV, "System" },
/* tr */ { IDS_SYSTEM_TR, "Sistem" },

/* en */ { IDS_MONO,    "Mono" },
/* da */ { IDS_MONO_DA, "Mono" },
/* de */ { IDS_MONO_DE, "Mono" },
/* es */ { IDS_MONO_ES, "Mono" },
/* fr */ { IDS_MONO_FR, "Mono" },
/* hu */ { IDS_MONO_HU, "" },  /* fuzzy */
/* it */ { IDS_MONO_IT, "Mono" },
/* ko */ { IDS_MONO_KO, "" },  /* fuzzy */
/* nl */ { IDS_MONO_NL, "Mono" },
/* pl */ { IDS_MONO_PL, "Mono" },
/* ru */ { IDS_MONO_RU, "¼ÞÝÞ" },
/* sv */ { IDS_MONO_SV, "Mono" },
/* tr */ { IDS_MONO_TR, "Mono" },

/* en */ { IDS_STEREO,    "Stereo" },
/* da */ { IDS_STEREO_DA, "Stereo" },
/* de */ { IDS_STEREO_DE, "Stereo" },
/* es */ { IDS_STEREO_ES, "Estereo" },
/* fr */ { IDS_STEREO_FR, "Stéréo" },
/* hu */ { IDS_STEREO_HU, "" },  /* fuzzy */
/* it */ { IDS_STEREO_IT, "Stereo" },
/* ko */ { IDS_STEREO_KO, "" },  /* fuzzy */
/* nl */ { IDS_STEREO_NL, "Stereo" },
/* pl */ { IDS_STEREO_PL, "Stereo" },
/* ru */ { IDS_STEREO_RU, "ÁâÕàÕÞ" },
/* sv */ { IDS_STEREO_SV, "Stereo" },
/* tr */ { IDS_STEREO_TR, "Stereo" },

/* en */ { IDS_MOUSE_TYPE,    "Mouse type" },
/* da */ { IDS_MOUSE_TYPE_DA, "Mussetype" },
/* de */ { IDS_MOUSE_TYPE_DE, "Maustyp" },
/* es */ { IDS_MOUSE_TYPE_ES, "Tipo ratón" },
/* fr */ { IDS_MOUSE_TYPE_FR, "Type de souris" },
/* hu */ { IDS_MOUSE_TYPE_HU, "Egér típusa" },
/* it */ { IDS_MOUSE_TYPE_IT, "Tipo di mouse" },
/* ko */ { IDS_MOUSE_TYPE_KO, "¸¶¿ì½º Å¸ÀÔ" },
/* nl */ { IDS_MOUSE_TYPE_NL, "Muis soort" },
/* pl */ { IDS_MOUSE_TYPE_PL, "Typ myszy" },
/* ru */ { IDS_MOUSE_TYPE_RU, "ÂØß ÜëèØ" },
/* sv */ { IDS_MOUSE_TYPE_SV, "Mustyp" },
/* tr */ { IDS_MOUSE_TYPE_TR, "Mouse tipi" },

/* en */ { IDS_MOUSE_SETTINGS,    "Mouse settings" },
/* da */ { IDS_MOUSE_SETTINGS_DA, "Musseindstillinger" },
/* de */ { IDS_MOUSE_SETTINGS_DE, "Maus Einstellungen" },
/* es */ { IDS_MOUSE_SETTINGS_ES, "Ajustes ratón" },
/* fr */ { IDS_MOUSE_SETTINGS_FR, "Paramètres de souris" },
/* hu */ { IDS_MOUSE_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_MOUSE_SETTINGS_IT, "Impostazioni mouse" },
/* ko */ { IDS_MOUSE_SETTINGS_KO, "¸¶¿ì½º ¼ÂÆÃ" },
/* nl */ { IDS_MOUSE_SETTINGS_NL, "Muis instellingen" },
/* pl */ { IDS_MOUSE_SETTINGS_PL, "Ustawienia myszy" },
/* ru */ { IDS_MOUSE_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_MOUSE_SETTINGS_SV, "Musinställningar" },
/* tr */ { IDS_MOUSE_SETTINGS_TR, "Mouse Ayarlarý" },

/* en */ { IDS_MOUSE_PORT,    "Mouse port" },
/* da */ { IDS_MOUSE_PORT_DA, "Musseport" },
/* de */ { IDS_MOUSE_PORT_DE, "Maus Port" },
/* es */ { IDS_MOUSE_PORT_ES, "Puerto del ratón" },
/* fr */ { IDS_MOUSE_PORT_FR, "Port de la souris" },
/* hu */ { IDS_MOUSE_PORT_HU, "Egér port" },
/* it */ { IDS_MOUSE_PORT_IT, "Porta del mouse" },
/* ko */ { IDS_MOUSE_PORT_KO, "¸¶¿ì½º Æ÷Æ®" },
/* nl */ { IDS_MOUSE_PORT_NL, "Muis poort" },
/* pl */ { IDS_MOUSE_PORT_PL, "Port myszy" },
/* ru */ { IDS_MOUSE_PORT_RU, "¿Þàâ ÜëèØ" },
/* sv */ { IDS_MOUSE_PORT_SV, "Musport" },
/* tr */ { IDS_MOUSE_PORT_TR, "Mouse portu" },

/* en */ { IDS_SMART_MOUSE_RTC_SAVE,    "Enable Smart Mouse RTC saving" },
/* da */ { IDS_SMART_MOUSE_RTC_SAVE_DA, "" },  /* fuzzy */
/* de */ { IDS_SMART_MOUSE_RTC_SAVE_DE, "Smart Mouse RTC Speicherung aktivieren" },
/* es */ { IDS_SMART_MOUSE_RTC_SAVE_ES, "Permitir grabar Smart Mouse RTC" },
/* fr */ { IDS_SMART_MOUSE_RTC_SAVE_FR, "" },  /* fuzzy */
/* hu */ { IDS_SMART_MOUSE_RTC_SAVE_HU, "" },  /* fuzzy */
/* it */ { IDS_SMART_MOUSE_RTC_SAVE_IT, "Attiva salvataggio RTC dello Smart Mouse" },
/* ko */ { IDS_SMART_MOUSE_RTC_SAVE_KO, "" },  /* fuzzy */
/* nl */ { IDS_SMART_MOUSE_RTC_SAVE_NL, "Opslaan van de Smart Mouse RTC inschakelen" },
/* pl */ { IDS_SMART_MOUSE_RTC_SAVE_PL, "W³±cz zapis zegara Smart Mouse" },
/* ru */ { IDS_SMART_MOUSE_RTC_SAVE_RU, "" },  /* fuzzy */
/* sv */ { IDS_SMART_MOUSE_RTC_SAVE_SV, "" },  /* fuzzy */
/* tr */ { IDS_SMART_MOUSE_RTC_SAVE_TR, "" },  /* fuzzy */

/* en */ { IDS_AUTOSTART_VIRTUAL_FS,    "Virtual FS" },
/* da */ { IDS_AUTOSTART_VIRTUAL_FS_DA, "Virtuelt FS" },
/* de */ { IDS_AUTOSTART_VIRTUAL_FS_DE, "Virtual FS" },
/* es */ { IDS_AUTOSTART_VIRTUAL_FS_ES, "FS virtual" },
/* fr */ { IDS_AUTOSTART_VIRTUAL_FS_FR, "SF virtuel" },
/* hu */ { IDS_AUTOSTART_VIRTUAL_FS_HU, "Virtuális fájlrendszer" },
/* it */ { IDS_AUTOSTART_VIRTUAL_FS_IT, "Virtual FS" },
/* ko */ { IDS_AUTOSTART_VIRTUAL_FS_KO, "¹öÃß¾ó ÆÄÀÏ½ºÅ¸ÀÏ" },
/* nl */ { IDS_AUTOSTART_VIRTUAL_FS_NL, "Virtueel bestandssysteem" },
/* pl */ { IDS_AUTOSTART_VIRTUAL_FS_PL, "Wirtualny system plików" },
/* ru */ { IDS_AUTOSTART_VIRTUAL_FS_RU, "Virtual FS" },
/* sv */ { IDS_AUTOSTART_VIRTUAL_FS_SV, "Virtuellt FS" },
/* tr */ { IDS_AUTOSTART_VIRTUAL_FS_TR, "Sanal FS" },

/* en */ { IDS_AUTOSTART_INJECT,    "Inject to RAM" },
/* da */ { IDS_AUTOSTART_INJECT_DA, "Indskyd til RAM" },
/* de */ { IDS_AUTOSTART_INJECT_DE, "Inject ins RAM" },
/* es */ { IDS_AUTOSTART_INJECT_ES, "Introd. en RAM" },
/* fr */ { IDS_AUTOSTART_INJECT_FR, "Injecter dans le RAM" },
/* hu */ { IDS_AUTOSTART_INJECT_HU, "" },  /* fuzzy */
/* it */ { IDS_AUTOSTART_INJECT_IT, "Immetti su RAM" },
/* ko */ { IDS_AUTOSTART_INJECT_KO, "RAM ¿¡ ÁÖÀÔÇÏ±â" },
/* nl */ { IDS_AUTOSTART_INJECT_NL, "Injecteer in RAM" },
/* pl */ { IDS_AUTOSTART_INJECT_PL, "Wy¶lij do RAM-u" },
/* ru */ { IDS_AUTOSTART_INJECT_RU, "Inject to RAM" },
/* sv */ { IDS_AUTOSTART_INJECT_SV, "Lägg in i RAM" },
/* tr */ { IDS_AUTOSTART_INJECT_TR, "RAM'e enjekte et" },

/* en */ { IDS_AUTOSTART_DISK,    "Disk image" },
/* da */ { IDS_AUTOSTART_DISK_DA, "Diskimage" },
/* de */ { IDS_AUTOSTART_DISK_DE, "Disk Image" },
/* es */ { IDS_AUTOSTART_DISK_ES, "Imag. disco" },
/* fr */ { IDS_AUTOSTART_DISK_FR, "Image de disque" },
/* hu */ { IDS_AUTOSTART_DISK_HU, "Lemez képmás" },
/* it */ { IDS_AUTOSTART_DISK_IT, "Immagine disco" },
/* ko */ { IDS_AUTOSTART_DISK_KO, "µð½ºÅ© ÀÌ¹ÌÁö" },
/* nl */ { IDS_AUTOSTART_DISK_NL, "Diskbestand" },
/* pl */ { IDS_AUTOSTART_DISK_PL, "Obraz dyskietki" },
/* ru */ { IDS_AUTOSTART_DISK_RU, "¾ÑàÐ× ÔØáÚÐ" },
/* sv */ { IDS_AUTOSTART_DISK_SV, "Diskavbildning" },
/* tr */ { IDS_AUTOSTART_DISK_TR, "Disk imajý" },

/* en */ { IDS_AUTOSTART_DISK_IMAGE_SELECT,    "Select file for PRG autostart disk" },
/* da */ { IDS_AUTOSTART_DISK_IMAGE_SELECT_DA, "Vælg fil for PRG autostartdiskette" },
/* de */ { IDS_AUTOSTART_DISK_IMAGE_SELECT_DE, "Datei für PRG Autostart auswählen" },
/* es */ { IDS_AUTOSTART_DISK_IMAGE_SELECT_ES, "Seleccionar fichero par autorranque disco para PRG" },
/* fr */ { IDS_AUTOSTART_DISK_IMAGE_SELECT_FR, "Sélectionner fichier pour autodémarrage PRG" },
/* hu */ { IDS_AUTOSTART_DISK_IMAGE_SELECT_HU, "Válasszon lemezfájlt a PRG automatikus indításhoz" },
/* it */ { IDS_AUTOSTART_DISK_IMAGE_SELECT_IT, "Seleziona file per avvio automatico PRG" },
/* ko */ { IDS_AUTOSTART_DISK_IMAGE_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDS_AUTOSTART_DISK_IMAGE_SELECT_NL, "Selecteer bestand voor PRG autostart disk" },
/* pl */ { IDS_AUTOSTART_DISK_IMAGE_SELECT_PL, "Wybierz plik dysku do automatycznego startu z PRG" },
/* ru */ { IDS_AUTOSTART_DISK_IMAGE_SELECT_RU, "Select file for PRG autostart disk" },
/* sv */ { IDS_AUTOSTART_DISK_IMAGE_SELECT_SV, "Välj fil för PRG-autostartdisk" },
/* tr */ { IDS_AUTOSTART_DISK_IMAGE_SELECT_TR, "PRG otomatik baþlatma için dosya seçin" },

/* en */ { IDS_WARP_ON_AUTOSTART,    "Autostart warp" },
/* da */ { IDS_WARP_ON_AUTOSTART_DA, "Autostart i warp-tilstand" },
/* de */ { IDS_WARP_ON_AUTOSTART_DE, "Autostart Warp" },
/* es */ { IDS_WARP_ON_AUTOSTART_ES, "Autoarranque Warp" },
/* fr */ { IDS_WARP_ON_AUTOSTART_FR, "Turbo à l'autodémarrage" },
/* hu */ { IDS_WARP_ON_AUTOSTART_HU, "Automatikus indítás hipergyors" },
/* it */ { IDS_WARP_ON_AUTOSTART_IT, "Avvio automatico in turbo" },
/* ko */ { IDS_WARP_ON_AUTOSTART_KO, "ÀÚµ¿ ½ÃÀÛ warp" },
/* nl */ { IDS_WARP_ON_AUTOSTART_NL, "Autostart warp" },
/* pl */ { IDS_WARP_ON_AUTOSTART_PL, "Turbo przy autostarcie" },
/* ru */ { IDS_WARP_ON_AUTOSTART_RU, "Autostart warp" },
/* sv */ { IDS_WARP_ON_AUTOSTART_SV, "Starta warp automatiskt" },
/* tr */ { IDS_WARP_ON_AUTOSTART_TR, "Warp modunda otomatik baþlat" },

/* en */ { IDS_AUTOSTART_DELAY,    "Autostart delay (in seconds)" },
/* da */ { IDS_AUTOSTART_DELAY_DA, "" },  /* fuzzy */
/* de */ { IDS_AUTOSTART_DELAY_DE, "Autostart Verzögerung (in Sekunden)" },
/* es */ { IDS_AUTOSTART_DELAY_ES, "" },  /* fuzzy */
/* fr */ { IDS_AUTOSTART_DELAY_FR, "" },  /* fuzzy */
/* hu */ { IDS_AUTOSTART_DELAY_HU, "" },  /* fuzzy */
/* it */ { IDS_AUTOSTART_DELAY_IT, "" },  /* fuzzy */
/* ko */ { IDS_AUTOSTART_DELAY_KO, "" },  /* fuzzy */
/* nl */ { IDS_AUTOSTART_DELAY_NL, "Autostart vertraging (in seconden)" },
/* pl */ { IDS_AUTOSTART_DELAY_PL, "" },  /* fuzzy */
/* ru */ { IDS_AUTOSTART_DELAY_RU, "" },  /* fuzzy */
/* sv */ { IDS_AUTOSTART_DELAY_SV, "" },  /* fuzzy */
/* tr */ { IDS_AUTOSTART_DELAY_TR, "" },  /* fuzzy */

/* en */ { IDS_AUTOSTART_RANDOM_DELAY,    "Autostart random delay" },
/* da */ { IDS_AUTOSTART_RANDOM_DELAY_DA, "Tilfældig forsinkelse på autostart" },
/* de */ { IDS_AUTOSTART_RANDOM_DELAY_DE, "Autostart Zufallsverzögerung" },
/* es */ { IDS_AUTOSTART_RANDOM_DELAY_ES, "Autoarranque retardo aleatorio" },
/* fr */ { IDS_AUTOSTART_RANDOM_DELAY_FR, "Délai aléatoire à l'autodémarrage" },
/* hu */ { IDS_AUTOSTART_RANDOM_DELAY_HU, "" },  /* fuzzy */
/* it */ { IDS_AUTOSTART_RANDOM_DELAY_IT, "Ritardo casuale all'avvio automatico" },
/* ko */ { IDS_AUTOSTART_RANDOM_DELAY_KO, "" },  /* fuzzy */
/* nl */ { IDS_AUTOSTART_RANDOM_DELAY_NL, "Willekeurige vertraging van de autostart" },
/* pl */ { IDS_AUTOSTART_RANDOM_DELAY_PL, "Losowe opó¼nienie autostartu" },
/* ru */ { IDS_AUTOSTART_RANDOM_DELAY_RU, "Autostart random delay" },
/* sv */ { IDS_AUTOSTART_RANDOM_DELAY_SV, "Slumpmässig fördröjning för autostart" },
/* tr */ { IDS_AUTOSTART_RANDOM_DELAY_TR, "Rastgele gecikmeyle otomatik baþlat" },

/* en */ { IDS_RUN_WITH_COLON,    "Use ':' with RUN" },
/* da */ { IDS_RUN_WITH_COLON_DA, "Brug \":\" med RUN" },
/* de */ { IDS_RUN_WITH_COLON_DE, "Verwende ':' mit RUN" },
/* es */ { IDS_RUN_WITH_COLON_ES, "Usar ':' con RUN" },
/* fr */ { IDS_RUN_WITH_COLON_FR, "Utiliser \":\" avec RUN" },
/* hu */ { IDS_RUN_WITH_COLON_HU, "" },  /* fuzzy */
/* it */ { IDS_RUN_WITH_COLON_IT, "Usa ':' con RUN" },
/* ko */ { IDS_RUN_WITH_COLON_KO, "':' ¸¦ RUN °ú »ç¿ëÇÏ±â" },
/* nl */ { IDS_RUN_WITH_COLON_NL, "Plak ':' achter RUN" },
/* pl */ { IDS_RUN_WITH_COLON_PL, "U¿ywaj ':' z RUN" },
/* ru */ { IDS_RUN_WITH_COLON_RU, "Use ':' with RUN" },
/* sv */ { IDS_RUN_WITH_COLON_SV, "Använd \":\" med RUN" },
/* tr */ { IDS_RUN_WITH_COLON_TR, "ÇALIÞTIR ile ':' kullan" },

/* en */ { IDS_AUTOSTART_PRG_MODE,    "PRG autostart mode" },
/* da */ { IDS_AUTOSTART_PRG_MODE_DA, "PRG-autostarttilstand" },
/* de */ { IDS_AUTOSTART_PRG_MODE_DE, "PRG Autostartmodus" },
/* es */ { IDS_AUTOSTART_PRG_MODE_ES, "Modo autoarranque PRG" },
/* fr */ { IDS_AUTOSTART_PRG_MODE_FR, "Mode d'autodémarrage des PRG" },
/* hu */ { IDS_AUTOSTART_PRG_MODE_HU, "PRG automatikus indítás mód" },
/* it */ { IDS_AUTOSTART_PRG_MODE_IT, "Avvio automatico PRG" },
/* ko */ { IDS_AUTOSTART_PRG_MODE_KO, "PRG ¿ÀÅä½º¸¶Æ® ¸ðµå" },
/* nl */ { IDS_AUTOSTART_PRG_MODE_NL, "PRG-autostartmodus" },
/* pl */ { IDS_AUTOSTART_PRG_MODE_PL, "Tryb autostartu z PRG" },
/* ru */ { IDS_AUTOSTART_PRG_MODE_RU, "PRG autostart mode" },
/* sv */ { IDS_AUTOSTART_PRG_MODE_SV, "Autostartläge för PRG" },
/* tr */ { IDS_AUTOSTART_PRG_MODE_TR, "PRG otomatik baþlatma modu" },

/* en */ { IDS_AUTOSTART_LOAD_TO_BASIC_START,    "Load to BASIC start" },
/* da */ { IDS_AUTOSTART_LOAD_TO_BASIC_START_DA, "Indlæs ved BASIC start" },
/* de */ { IDS_AUTOSTART_LOAD_TO_BASIC_START_DE, "Lade an Basic Start" },
/* es */ { IDS_AUTOSTART_LOAD_TO_BASIC_START_ES, "Cargar a inicio BASIC" },
/* fr */ { IDS_AUTOSTART_LOAD_TO_BASIC_START_FR, "Charger au démarrage BASIC" },
/* hu */ { IDS_AUTOSTART_LOAD_TO_BASIC_START_HU, "" },  /* fuzzy */
/* it */ { IDS_AUTOSTART_LOAD_TO_BASIC_START_IT, "Carica all'inizio del BASIC" },
/* ko */ { IDS_AUTOSTART_LOAD_TO_BASIC_START_KO, "BASIC ½ÃÀÛ ¿¡ ·ÎµåÇÏ±â" },
/* nl */ { IDS_AUTOSTART_LOAD_TO_BASIC_START_NL, "Laad naar BASIC start" },
/* pl */ { IDS_AUTOSTART_LOAD_TO_BASIC_START_PL, "Wczytuj do BASIC-a" },
/* ru */ { IDS_AUTOSTART_LOAD_TO_BASIC_START_RU, "" },  /* fuzzy */
/* sv */ { IDS_AUTOSTART_LOAD_TO_BASIC_START_SV, "Läs till BASIC-start" },
/* tr */ { IDS_AUTOSTART_LOAD_TO_BASIC_START_TR, "BASIC baþlangýcýna yükle" },

/* en */ { IDS_AUTOSTART_DISK_IMAGE_FILENAME,    "PRG autostart disk image" },
/* da */ { IDS_AUTOSTART_DISK_IMAGE_FILENAME_DA, "PRG Autostart disk-image" },
/* de */ { IDS_AUTOSTART_DISK_IMAGE_FILENAME_DE, "PRG Autostart Disk Image" },
/* es */ { IDS_AUTOSTART_DISK_IMAGE_FILENAME_ES, "Autoarranque imagen disco PRG" },
/* fr */ { IDS_AUTOSTART_DISK_IMAGE_FILENAME_FR, "Autocharger PRG d'une image de disque" },
/* hu */ { IDS_AUTOSTART_DISK_IMAGE_FILENAME_HU, "PRG automatikus indítás képmás" },
/* it */ { IDS_AUTOSTART_DISK_IMAGE_FILENAME_IT, "Immagine disco avvio automatico PRG" },
/* ko */ { IDS_AUTOSTART_DISK_IMAGE_FILENAME_KO, "PRG ¿ÀÅä½º¸¶Æ® µð½ºÅ© ÀÌ¹ÌÁö" },
/* nl */ { IDS_AUTOSTART_DISK_IMAGE_FILENAME_NL, "PRG-autostartdiskbestand" },
/* pl */ { IDS_AUTOSTART_DISK_IMAGE_FILENAME_PL, "Obraz dyskietki do autostartu z PRG" },
/* ru */ { IDS_AUTOSTART_DISK_IMAGE_FILENAME_RU, "PRG autostart disk image" },
/* sv */ { IDS_AUTOSTART_DISK_IMAGE_FILENAME_SV, "Autostartdiskavbildning för PRG" },
/* tr */ { IDS_AUTOSTART_DISK_IMAGE_FILENAME_TR, "PRG otomatik baþlatma disk imajý" },

/* en */ { IDS_AUTOSTART_SETTINGS,    "Autostart settings" },
/* da */ { IDS_AUTOSTART_SETTINGS_DA, "Autostart-indstillinger" },
/* de */ { IDS_AUTOSTART_SETTINGS_DE, "Autostart Einstellungen" },
/* es */ { IDS_AUTOSTART_SETTINGS_ES, "Ajustes de autoinicio" },
/* fr */ { IDS_AUTOSTART_SETTINGS_FR, "Paramètres Autodémarrage" },
/* hu */ { IDS_AUTOSTART_SETTINGS_HU, "Autostart beállításai" },
/* it */ { IDS_AUTOSTART_SETTINGS_IT, "Impostazioni avvio automatico" },
/* ko */ { IDS_AUTOSTART_SETTINGS_KO, "ÀÚµ¿½ÃÀÛ ¼ÂÆÃ" },
/* nl */ { IDS_AUTOSTART_SETTINGS_NL, "Autostart instellingen" },
/* pl */ { IDS_AUTOSTART_SETTINGS_PL, "Ustawienia autostartu" },
/* ru */ { IDS_AUTOSTART_SETTINGS_RU, "½ÐáâàÞÙÚØ ÐÒâÞ×ÐßãáÚÐ" },
/* sv */ { IDS_AUTOSTART_SETTINGS_SV, "Autostartinställningar" },
/* tr */ { IDS_AUTOSTART_SETTINGS_TR, "Otomatik baþlatma ayarlarý" },

/* en */ { IDS_ERROR_SAVING_EASYFLASH_CRT,    "Error saving EasyFlash crt file" },
/* da */ { IDS_ERROR_SAVING_EASYFLASH_CRT_DA, "Fejl under skrivning af EasyFlash CRT-fil" },
/* de */ { IDS_ERROR_SAVING_EASYFLASH_CRT_DE, "Fehler beim Speichern der EasyFlash CRT Datei" },
/* es */ { IDS_ERROR_SAVING_EASYFLASH_CRT_ES, "Error al grabar fichero EasyFlash crt" },
/* fr */ { IDS_ERROR_SAVING_EASYFLASH_CRT_FR, "Erreur de sauvegarde du fichier CRT EasyFlash" },
/* hu */ { IDS_ERROR_SAVING_EASYFLASH_CRT_HU, "Hiba az EasyFlash crt fájl mentése közben" },
/* it */ { IDS_ERROR_SAVING_EASYFLASH_CRT_IT, "Errore durante il salvataggio del file crt EasyFlash" },
/* ko */ { IDS_ERROR_SAVING_EASYFLASH_CRT_KO, "EasyFlash crt ÆÄÀÏ ÀúÀå ¿¡·¯°¡ ¹ß»ýÇß½À´Ï´Ù" },
/* nl */ { IDS_ERROR_SAVING_EASYFLASH_CRT_NL, "Kan het EasyFlash crt bestand niet opslaan" },
/* pl */ { IDS_ERROR_SAVING_EASYFLASH_CRT_PL, "B³±d zapisu pliku crt EasyFlash" },
/* ru */ { IDS_ERROR_SAVING_EASYFLASH_CRT_RU, "Error saving EasyFlash crt file" },
/* sv */ { IDS_ERROR_SAVING_EASYFLASH_CRT_SV, "Fel vid skrivning av EasyFlash-crt-fil." },
/* tr */ { IDS_ERROR_SAVING_EASYFLASH_CRT_TR, "EasyFlash crt dosyasý kayýt hatasý" },

/* en */ { IDS_EASYFLASH_JUMPER,    "EasyFlash jumper" },
/* da */ { IDS_EASYFLASH_JUMPER_DA, "EasyFlashjumper" },
/* de */ { IDS_EASYFLASH_JUMPER_DE, "EasyFlash Jumper" },
/* es */ { IDS_EASYFLASH_JUMPER_ES, "Puente EasyFlash" },
/* fr */ { IDS_EASYFLASH_JUMPER_FR, "Jumper EasyFlash" },
/* hu */ { IDS_EASYFLASH_JUMPER_HU, "EasyFlash jumper" },
/* it */ { IDS_EASYFLASH_JUMPER_IT, "Ponticello EasyFlash" },
/* ko */ { IDS_EASYFLASH_JUMPER_KO, "" },  /* fuzzy */
/* nl */ { IDS_EASYFLASH_JUMPER_NL, "EasyFlash jumper" },
/* pl */ { IDS_EASYFLASH_JUMPER_PL, "Zworka EasyFlash" },
/* ru */ { IDS_EASYFLASH_JUMPER_RU, "EasyFlash jumper" },
/* sv */ { IDS_EASYFLASH_JUMPER_SV, "EasyFlash-bygel" },
/* tr */ { IDS_EASYFLASH_JUMPER_TR, "EasyFlash jumper" },

/* en */ { IDS_SAVE_EASYFLASH_CRT_ON_DETACH,    "Save to EasyFlash CRT file on detach" },
/* da */ { IDS_SAVE_EASYFLASH_CRT_ON_DETACH_DA, "Gem til EasyFlash CRT-fil ved afbrydelse" },
/* de */ { IDS_SAVE_EASYFLASH_CRT_ON_DETACH_DE, "EasyFlash CRT Imagedatei beim entfernen speichern" },
/* es */ { IDS_SAVE_EASYFLASH_CRT_ON_DETACH_ES, "Grabar fichero al extraer EasyFlash CRT" },
/* fr */ { IDS_SAVE_EASYFLASH_CRT_ON_DETACH_FR, "Enregistrer sur fichier CRT EasyFlash au détachement" },
/* hu */ { IDS_SAVE_EASYFLASH_CRT_ON_DETACH_HU, "EasyFlash CRT fájl mentése leválasztáskor" },
/* it */ { IDS_SAVE_EASYFLASH_CRT_ON_DETACH_IT, "Salva su file CRT EasyFlash alla rimozione" },
/* ko */ { IDS_SAVE_EASYFLASH_CRT_ON_DETACH_KO, "" },  /* fuzzy */
/* nl */ { IDS_SAVE_EASYFLASH_CRT_ON_DETACH_NL, "Sla op naar het EasyFlash CRT bestand bij ontkoppelen" },
/* pl */ { IDS_SAVE_EASYFLASH_CRT_ON_DETACH_PL, "Zapisz do pliku CRT Easy Flash przy od³±czeniu" },
/* ru */ { IDS_SAVE_EASYFLASH_CRT_ON_DETACH_RU, "Save to EasyFlash CRT file on detach" },
/* sv */ { IDS_SAVE_EASYFLASH_CRT_ON_DETACH_SV, "Spara till EasyFlash-CRT-fil vid frånkoppling" },
/* tr */ { IDS_SAVE_EASYFLASH_CRT_ON_DETACH_TR, "Çýkartýldýðýnda EasyFlash CRT dosyasýna kaydet" },

/* en */ { IDS_OPTIMIZE_EASYFLASH_CRT_SAVE,    "Optimize EasyFlash CRT when saving to file" },
/* da */ { IDS_OPTIMIZE_EASYFLASH_CRT_SAVE_DA, "" },  /* fuzzy */
/* de */ { IDS_OPTIMIZE_EASYFLASH_CRT_SAVE_DE, "EasyFlash CRT beim Speichern optimieren" },
/* es */ { IDS_OPTIMIZE_EASYFLASH_CRT_SAVE_ES, "" },  /* fuzzy */
/* fr */ { IDS_OPTIMIZE_EASYFLASH_CRT_SAVE_FR, "" },  /* fuzzy */
/* hu */ { IDS_OPTIMIZE_EASYFLASH_CRT_SAVE_HU, "" },  /* fuzzy */
/* it */ { IDS_OPTIMIZE_EASYFLASH_CRT_SAVE_IT, "" },  /* fuzzy */
/* ko */ { IDS_OPTIMIZE_EASYFLASH_CRT_SAVE_KO, "" },  /* fuzzy */
/* nl */ { IDS_OPTIMIZE_EASYFLASH_CRT_SAVE_NL, "Optimaliseer het EasyFlash CRT bestand bij opslaan" },
/* pl */ { IDS_OPTIMIZE_EASYFLASH_CRT_SAVE_PL, "" },  /* fuzzy */
/* ru */ { IDS_OPTIMIZE_EASYFLASH_CRT_SAVE_RU, "" },  /* fuzzy */
/* sv */ { IDS_OPTIMIZE_EASYFLASH_CRT_SAVE_SV, "" },  /* fuzzy */
/* tr */ { IDS_OPTIMIZE_EASYFLASH_CRT_SAVE_TR, "" },  /* fuzzy */

/* en */ { IDS_SAVE_EASYFLASH_CRT_NOW,    "Save to EasyFlash CRT now" },
/* da */ { IDS_SAVE_EASYFLASH_CRT_NOW_DA, "Gem til EasyFlash CRT nu" },
/* de */ { IDS_SAVE_EASYFLASH_CRT_NOW_DE, "EasyFlash CRT Imagedatei jetzt speichern" },
/* es */ { IDS_SAVE_EASYFLASH_CRT_NOW_ES, "Grabar en EasyFlash CRT ahora" },
/* fr */ { IDS_SAVE_EASYFLASH_CRT_NOW_FR, "Enregistrer sur fichier CRT EasyFlash maintenant" },
/* hu */ { IDS_SAVE_EASYFLASH_CRT_NOW_HU, "EasyFlash CRT fájl mentése most" },
/* it */ { IDS_SAVE_EASYFLASH_CRT_NOW_IT, "Salva su CRT EasyFlash ora" },
/* ko */ { IDS_SAVE_EASYFLASH_CRT_NOW_KO, "" },  /* fuzzy */
/* nl */ { IDS_SAVE_EASYFLASH_CRT_NOW_NL, "Sla nu op naar het EasyFlash CRT bestand" },
/* pl */ { IDS_SAVE_EASYFLASH_CRT_NOW_PL, "Zapisz do CRT Easy Flash teraz" },
/* ru */ { IDS_SAVE_EASYFLASH_CRT_NOW_RU, "Save to EasyFlash CRT now" },
/* sv */ { IDS_SAVE_EASYFLASH_CRT_NOW_SV, "Spara till EasyFlash-CRT nu" },
/* tr */ { IDS_SAVE_EASYFLASH_CRT_NOW_TR, "Hemen EasyFlash CRT'ye kaydet" },

/* en */ { IDS_6569_PAL,    "6569 (PAL)" },
/* da */ { IDS_6569_PAL_DA, "6569 (PAL)" },
/* de */ { IDS_6569_PAL_DE, "6569 (PAL)" },
/* es */ { IDS_6569_PAL_ES, "6569 (PAL)" },
/* fr */ { IDS_6569_PAL_FR, "6569 (PAL)" },
/* hu */ { IDS_6569_PAL_HU, "" },  /* fuzzy */
/* it */ { IDS_6569_PAL_IT, "6569 (PAL)" },
/* ko */ { IDS_6569_PAL_KO, "" },  /* fuzzy */
/* nl */ { IDS_6569_PAL_NL, "6569 (PAL)" },
/* pl */ { IDS_6569_PAL_PL, "6569 (PAL)" },
/* ru */ { IDS_6569_PAL_RU, "6569 (PAL)" },
/* sv */ { IDS_6569_PAL_SV, "6569 (PAL)" },
/* tr */ { IDS_6569_PAL_TR, "6569 (PAL)" },

/* en */ { IDS_8565_PAL,    "8565 (PAL)" },
/* da */ { IDS_8565_PAL_DA, "8565 (PAL)" },
/* de */ { IDS_8565_PAL_DE, "8565 (PAL)" },
/* es */ { IDS_8565_PAL_ES, "8565 (PAL)" },
/* fr */ { IDS_8565_PAL_FR, "8565 (PAL)" },
/* hu */ { IDS_8565_PAL_HU, "" },  /* fuzzy */
/* it */ { IDS_8565_PAL_IT, "8565 (PAL)" },
/* ko */ { IDS_8565_PAL_KO, "" },  /* fuzzy */
/* nl */ { IDS_8565_PAL_NL, "8565 (PAL)" },
/* pl */ { IDS_8565_PAL_PL, "8565 (PAL)" },
/* ru */ { IDS_8565_PAL_RU, "8565 (PAL)" },
/* sv */ { IDS_8565_PAL_SV, "8565 (PAL)" },
/* tr */ { IDS_8565_PAL_TR, "8565 (PAL)" },

/* en */ { IDS_6569R1_OLD_PAL,    "6569R1 (old PAL)" },
/* da */ { IDS_6569R1_OLD_PAL_DA, "6569R1 (gammel PAL)" },
/* de */ { IDS_6569R1_OLD_PAL_DE, "6569R1 (alt PAL)" },
/* es */ { IDS_6569R1_OLD_PAL_ES, "6569R1 (PAL antiguo)" },
/* fr */ { IDS_6569R1_OLD_PAL_FR, "6569R1 (ancien PAL)" },
/* hu */ { IDS_6569R1_OLD_PAL_HU, "" },  /* fuzzy */
/* it */ { IDS_6569R1_OLD_PAL_IT, "6569R1 (PAL vecchio)" },
/* ko */ { IDS_6569R1_OLD_PAL_KO, "6569R1 (±¸Çü PAL)" },
/* nl */ { IDS_6569R1_OLD_PAL_NL, "6569R1 (oude PAL)" },
/* pl */ { IDS_6569R1_OLD_PAL_PL, "6569R1 (stary PAL)" },
/* ru */ { IDS_6569R1_OLD_PAL_RU, "" },  /* fuzzy */
/* sv */ { IDS_6569R1_OLD_PAL_SV, "6569R1 (gammal PAL)" },
/* tr */ { IDS_6569R1_OLD_PAL_TR, "6569R1 (eski PAL)" },

/* en */ { IDS_6567_NTSC,    "6567 (NTSC)" },
/* da */ { IDS_6567_NTSC_DA, "6567 (NTSC)" },
/* de */ { IDS_6567_NTSC_DE, "6567 (NTSC)" },
/* es */ { IDS_6567_NTSC_ES, "6567 (NTSC)" },
/* fr */ { IDS_6567_NTSC_FR, "6567 (NTSC)" },
/* hu */ { IDS_6567_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDS_6567_NTSC_IT, "6567 (NTSC)" },
/* ko */ { IDS_6567_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDS_6567_NTSC_NL, "6567 (NTSC)" },
/* pl */ { IDS_6567_NTSC_PL, "6567 (NTSC)" },
/* ru */ { IDS_6567_NTSC_RU, "6567 (NTSC)" },
/* sv */ { IDS_6567_NTSC_SV, "6567 (NTSC)" },
/* tr */ { IDS_6567_NTSC_TR, "6567 (NTSC)" },

/* en */ { IDS_8562_NTSC,    "8562 (NTSC)" },
/* da */ { IDS_8562_NTSC_DA, "8562 (NTSC)" },
/* de */ { IDS_8562_NTSC_DE, "8562 (NTSC)" },
/* es */ { IDS_8562_NTSC_ES, "8562 (NTSC)" },
/* fr */ { IDS_8562_NTSC_FR, "8562 (NTSC)" },
/* hu */ { IDS_8562_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDS_8562_NTSC_IT, "8562 (NTSC)" },
/* ko */ { IDS_8562_NTSC_KO, "" },  /* fuzzy */
/* nl */ { IDS_8562_NTSC_NL, "8562 (NTSC)" },
/* pl */ { IDS_8562_NTSC_PL, "8562 (NTSC)" },
/* ru */ { IDS_8562_NTSC_RU, "8562 (NTSC)" },
/* sv */ { IDS_8562_NTSC_SV, "8562 (NTSC)" },
/* tr */ { IDS_8562_NTSC_TR, "8562 (NTSC)" },

/* en */ { IDS_6567R56A_OLD_NTSC,    "6567R56A (old NTSC)" },
/* da */ { IDS_6567R56A_OLD_NTSC_DA, "6567R56A (gammel NTSC)" },
/* de */ { IDS_6567R56A_OLD_NTSC_DE, "6567R56A (alt NTSC)" },
/* es */ { IDS_6567R56A_OLD_NTSC_ES, "6567R56A (NTSC antiguo)" },
/* fr */ { IDS_6567R56A_OLD_NTSC_FR, "6567R56A (ancien NTSC)" },
/* hu */ { IDS_6567R56A_OLD_NTSC_HU, "" },  /* fuzzy */
/* it */ { IDS_6567R56A_OLD_NTSC_IT, "6567R56A (NTSC vecchio)" },
/* ko */ { IDS_6567R56A_OLD_NTSC_KO, "6567R56A (±¸Çü NTSC)" },
/* nl */ { IDS_6567R56A_OLD_NTSC_NL, "6567R56A (oude NTSC)" },
/* pl */ { IDS_6567R56A_OLD_NTSC_PL, "6567R56A (stary NTSC)" },
/* ru */ { IDS_6567R56A_OLD_NTSC_RU, "" },  /* fuzzy */
/* sv */ { IDS_6567R56A_OLD_NTSC_SV, "6567R56A (gammal NTSC)" },
/* tr */ { IDS_6567R56A_OLD_NTSC_TR, "6567R56A (eski NTSC)" },

/* en */ { IDS_6572_PAL_N,    "6572 (PAL-N)" },
/* da */ { IDS_6572_PAL_N_DA, "6572 (PAL-N)" },
/* de */ { IDS_6572_PAL_N_DE, "6572 (PAL-N)" },
/* es */ { IDS_6572_PAL_N_ES, "6572 (PAL-N)" },
/* fr */ { IDS_6572_PAL_N_FR, "6572 (PAL-N)" },
/* hu */ { IDS_6572_PAL_N_HU, "" },  /* fuzzy */
/* it */ { IDS_6572_PAL_N_IT, "6572 (PAL-N)" },
/* ko */ { IDS_6572_PAL_N_KO, "" },  /* fuzzy */
/* nl */ { IDS_6572_PAL_N_NL, "6572 (PAL-N)" },
/* pl */ { IDS_6572_PAL_N_PL, "6572 (PAL-N)" },
/* ru */ { IDS_6572_PAL_N_RU, "6572 (PAL-N)" },
/* sv */ { IDS_6572_PAL_N_SV, "6572 (PAL-N)" },
/* tr */ { IDS_6572_PAL_N_TR, "6572 (PAL-N)" },

/* en */ { IDS_6526_OLD,    "6526 (old)" },
/* da */ { IDS_6526_OLD_DA, "6526 (gammel)" },
/* de */ { IDS_6526_OLD_DE, "6526 (alt)" },
/* es */ { IDS_6526_OLD_ES, "6526 (antiguo)" },
/* fr */ { IDS_6526_OLD_FR, "6526 (ancien)" },
/* hu */ { IDS_6526_OLD_HU, "" },  /* fuzzy */
/* it */ { IDS_6526_OLD_IT, "6526 (vecchio)" },
/* ko */ { IDS_6526_OLD_KO, "6526 (±¸Çü)" },
/* nl */ { IDS_6526_OLD_NL, "6526 (oud)" },
/* pl */ { IDS_6526_OLD_PL, "6526 (stary)" },
/* ru */ { IDS_6526_OLD_RU, "6526 (old)" },
/* sv */ { IDS_6526_OLD_SV, "6526 (gammal)" },
/* tr */ { IDS_6526_OLD_TR, "6526 (eski)" },

/* en */ { IDS_6526A_NEW,    "6526A (new)" },
/* da */ { IDS_6526A_NEW_DA, "6526A (ny)" },
/* de */ { IDS_6526A_NEW_DE, "6526A (neu)" },
/* es */ { IDS_6526A_NEW_ES, "6526A (nuevo)" },
/* fr */ { IDS_6526A_NEW_FR, "6526A (récent)" },
/* hu */ { IDS_6526A_NEW_HU, "" },  /* fuzzy */
/* it */ { IDS_6526A_NEW_IT, "6526A (nuovo)" },
/* ko */ { IDS_6526A_NEW_KO, "6526A (»õ·Î¿î)" },
/* nl */ { IDS_6526A_NEW_NL, "6526A (nieuw)" },
/* pl */ { IDS_6526A_NEW_PL, "" },  /* fuzzy */
/* ru */ { IDS_6526A_NEW_RU, "6526A (new)" },
/* sv */ { IDS_6526A_NEW_SV, "" },  /* fuzzy */
/* tr */ { IDS_6526A_NEW_TR, "6526A (yeni)" },

/* en */ { IDS_DISCRETE,    "Discrete" },
/* da */ { IDS_DISCRETE_DA, "Diskret" },
/* de */ { IDS_DISCRETE_DE, "Diskret" },
/* es */ { IDS_DISCRETE_ES, "Discreto" },
/* fr */ { IDS_DISCRETE_FR, "Discrète" },
/* hu */ { IDS_DISCRETE_HU, "" },  /* fuzzy */
/* it */ { IDS_DISCRETE_IT, "Discreto" },
/* ko */ { IDS_DISCRETE_KO, "ÀÌ»êÇü" },
/* nl */ { IDS_DISCRETE_NL, "Discreet" },
/* pl */ { IDS_DISCRETE_PL, "Dyskretny" },
/* ru */ { IDS_DISCRETE_RU, "Discrete" },
/* sv */ { IDS_DISCRETE_SV, "Diskret" },
/* tr */ { IDS_DISCRETE_TR, "Baðlantýyý kes" },

/* en */ { IDS_CUSTOM_IC,    "Custom IC" },
/* da */ { IDS_CUSTOM_IC_DA, "Egen IC" },
/* de */ { IDS_CUSTOM_IC_DE, "Benutzerdefinierte IC" },
/* es */ { IDS_CUSTOM_IC_ES, "IC habitual" },
/* fr */ { IDS_CUSTOM_IC_FR, "IC Personnalisé" },
/* hu */ { IDS_CUSTOM_IC_HU, "" },  /* fuzzy */
/* it */ { IDS_CUSTOM_IC_IT, "IC personalizzato" },
/* ko */ { IDS_CUSTOM_IC_KO, "Æ¯Á¤ IC" },
/* nl */ { IDS_CUSTOM_IC_NL, "Aangepast IC" },
/* pl */ { IDS_CUSTOM_IC_PL, "W³asne IC" },
/* ru */ { IDS_CUSTOM_IC_RU, "Custom IC" },
/* sv */ { IDS_CUSTOM_IC_SV, "Egen krets" },
/* tr */ { IDS_CUSTOM_IC_TR, "Özel IC" },

/* en */ { IDS_VICII_MODEL,    "VIC-II model" },
/* da */ { IDS_VICII_MODEL_DA, "VIC-II-model" },
/* de */ { IDS_VICII_MODEL_DE, "VIC-II Modell" },
/* es */ { IDS_VICII_MODEL_ES, "Modelo VIC-II" },
/* fr */ { IDS_VICII_MODEL_FR, "Modèle VIC-II" },
/* hu */ { IDS_VICII_MODEL_HU, "" },  /* fuzzy */
/* it */ { IDS_VICII_MODEL_IT, "Modello VIC-II" },
/* ko */ { IDS_VICII_MODEL_KO, "VIC-II ¸ðµ¨" },
/* nl */ { IDS_VICII_MODEL_NL, "VIC-II-model" },
/* pl */ { IDS_VICII_MODEL_PL, "Model VIC-II" },
/* ru */ { IDS_VICII_MODEL_RU, "¼ÞÔÕÛì VIC-II" },
/* sv */ { IDS_VICII_MODEL_SV, "VIC II-modell" },
/* tr */ { IDS_VICII_MODEL_TR, "VIC-II modeli" },

/* en */ { IDS_CIA1_MODEL,    "CIA 1 model" },
/* da */ { IDS_CIA1_MODEL_DA, "CIA 1 model" },
/* de */ { IDS_CIA1_MODEL_DE, "CIA 1 Modell" },
/* es */ { IDS_CIA1_MODEL_ES, "Modelo CIA 1" },
/* fr */ { IDS_CIA1_MODEL_FR, "Modèle CIA 1" },
/* hu */ { IDS_CIA1_MODEL_HU, "" },  /* fuzzy */
/* it */ { IDS_CIA1_MODEL_IT, "Modello CIA 1" },
/* ko */ { IDS_CIA1_MODEL_KO, "CIA 1 ¸ðµ¨" },
/* nl */ { IDS_CIA1_MODEL_NL, "CIA 1-model" },
/* pl */ { IDS_CIA1_MODEL_PL, "Model CIA 1" },
/* ru */ { IDS_CIA1_MODEL_RU, "¼ÞÔÕÛì CIA 1" },
/* sv */ { IDS_CIA1_MODEL_SV, "CIA 1-modell" },
/* tr */ { IDS_CIA1_MODEL_TR, "CIA 1 modeli" },

/* en */ { IDS_CIA2_MODEL,    "CIA 2 model" },
/* da */ { IDS_CIA2_MODEL_DA, "CIA 2 model" },
/* de */ { IDS_CIA2_MODEL_DE, "CIA 2 Modell" },
/* es */ { IDS_CIA2_MODEL_ES, "Modelo CIA 2" },
/* fr */ { IDS_CIA2_MODEL_FR, "Modèle CIA 2" },
/* hu */ { IDS_CIA2_MODEL_HU, "" },  /* fuzzy */
/* it */ { IDS_CIA2_MODEL_IT, "Modello CIA 2" },
/* ko */ { IDS_CIA2_MODEL_KO, "CIA 2 ¸ðµ¨" },
/* nl */ { IDS_CIA2_MODEL_NL, "CIA 2-model" },
/* pl */ { IDS_CIA2_MODEL_PL, "Model CIA 2" },
/* ru */ { IDS_CIA2_MODEL_RU, "¼ÞÔÕÛì CIA 2" },
/* sv */ { IDS_CIA2_MODEL_SV, "CIA 2-modell" },
/* tr */ { IDS_CIA2_MODEL_TR, "CIA 2 modeli" },

/* en */ { IDS_GLUE_LOGIC,    "Glue logic" },
/* da */ { IDS_GLUE_LOGIC_DA, "Glue logic" },
/* de */ { IDS_GLUE_LOGIC_DE, "Glue Logik" },
/* es */ { IDS_GLUE_LOGIC_ES, "Glue logic" },
/* fr */ { IDS_GLUE_LOGIC_FR, "Glue logic" },
/* hu */ { IDS_GLUE_LOGIC_HU, "" },  /* fuzzy */
/* it */ { IDS_GLUE_LOGIC_IT, "Glue logic" },
/* ko */ { IDS_GLUE_LOGIC_KO, "±Û·ç ·ÎÁ÷" },
/* nl */ { IDS_GLUE_LOGIC_NL, "Glue logic" },
/* pl */ { IDS_GLUE_LOGIC_PL, "Glue logic" },
/* ru */ { IDS_GLUE_LOGIC_RU, "Glue logic" },
/* sv */ { IDS_GLUE_LOGIC_SV, "Klisterlogik" },
/* tr */ { IDS_GLUE_LOGIC_TR, "Glue logic" },

/* en */ { IDS_IEC_RESET,    "Reset IEC bus with computer" },
/* da */ { IDS_IEC_RESET_DA, "" },  /* fuzzy */
/* de */ { IDS_IEC_RESET_DE, "IEC mit Computer zurücksetzen" },
/* es */ { IDS_IEC_RESET_ES, "Reiniciar bus IEC con el ordenador" },
/* fr */ { IDS_IEC_RESET_FR, "" },  /* fuzzy */
/* hu */ { IDS_IEC_RESET_HU, "" },  /* fuzzy */
/* it */ { IDS_IEC_RESET_IT, "Reset bus IEC con computer" },
/* ko */ { IDS_IEC_RESET_KO, "" },  /* fuzzy */
/* nl */ { IDS_IEC_RESET_NL, "Reset IEC bus met de computer" },
/* pl */ { IDS_IEC_RESET_PL, "Zeruj szynê IEC z komputerem" },
/* ru */ { IDS_IEC_RESET_RU, "" },  /* fuzzy */
/* sv */ { IDS_IEC_RESET_SV, "" },  /* fuzzy */
/* tr */ { IDS_IEC_RESET_TR, "" },  /* fuzzy */

/* en */ { IDS_KERNAL_REVISION,    "Kernal revision" },
/* da */ { IDS_KERNAL_REVISION_DA, "" },  /* fuzzy */
/* de */ { IDS_KERNAL_REVISION_DE, "Kernal Revision" },
/* es */ { IDS_KERNAL_REVISION_ES, "" },  /* fuzzy */
/* fr */ { IDS_KERNAL_REVISION_FR, "" },  /* fuzzy */
/* hu */ { IDS_KERNAL_REVISION_HU, "" },  /* fuzzy */
/* it */ { IDS_KERNAL_REVISION_IT, "" },  /* fuzzy */
/* ko */ { IDS_KERNAL_REVISION_KO, "" },  /* fuzzy */
/* nl */ { IDS_KERNAL_REVISION_NL, "Kernal revisie" },
/* pl */ { IDS_KERNAL_REVISION_PL, "" },  /* fuzzy */
/* ru */ { IDS_KERNAL_REVISION_RU, "" },  /* fuzzy */
/* sv */ { IDS_KERNAL_REVISION_SV, "" },  /* fuzzy */
/* tr */ { IDS_KERNAL_REVISION_TR, "" },  /* fuzzy */

/* en */ { IDS_C64_MODEL_SETTINGS,    "C64 model settings" },
/* da */ { IDS_C64_MODEL_SETTINGS_DA, "C64-model indstillinger" },
/* de */ { IDS_C64_MODEL_SETTINGS_DE, "C64 Modell Einstellungen" },
/* es */ { IDS_C64_MODEL_SETTINGS_ES, "Ajustes modelo C64" },
/* fr */ { IDS_C64_MODEL_SETTINGS_FR, "Paramètres du modèle C64" },
/* hu */ { IDS_C64_MODEL_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_C64_MODEL_SETTINGS_IT, "Impostazioni modello C64" },
/* ko */ { IDS_C64_MODEL_SETTINGS_KO, "C64 ¸ðµ¨ ¼ÂÆÃ" },
/* nl */ { IDS_C64_MODEL_SETTINGS_NL, "C64 model instellingen" },
/* pl */ { IDS_C64_MODEL_SETTINGS_PL, "Ustawienia modelu C64" },
/* ru */ { IDS_C64_MODEL_SETTINGS_RU, "C64 model settings" },
/* sv */ { IDS_C64_MODEL_SETTINGS_SV, "C64-modellinställningar" },
/* tr */ { IDS_C64_MODEL_SETTINGS_TR, "C64 model ayarlarý" },

/* en */ { IDS_CIA_MODEL_SETTINGS,    "CIA model settings" },
/* da */ { IDS_CIA_MODEL_SETTINGS_DA, "CIA-model indstillinger" },
/* de */ { IDS_CIA_MODEL_SETTINGS_DE, "CIA Modell Einstellungen" },
/* es */ { IDS_CIA_MODEL_SETTINGS_ES, "Ajustes modelo CIA" },
/* fr */ { IDS_CIA_MODEL_SETTINGS_FR, "Paramètres du modèle CIA" },
/* hu */ { IDS_CIA_MODEL_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_CIA_MODEL_SETTINGS_IT, "Impostazioni modello CIA" },
/* ko */ { IDS_CIA_MODEL_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_CIA_MODEL_SETTINGS_NL, "CIA model instellingen" },
/* pl */ { IDS_CIA_MODEL_SETTINGS_PL, "Ustawienia modelu CIA" },
/* ru */ { IDS_CIA_MODEL_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_CIA_MODEL_SETTINGS_SV, "CIA-modellinställningar" },
/* tr */ { IDS_CIA_MODEL_SETTINGS_TR, "CIA model ayarlarý" },

/* en */ { IDS_SCALE,    "Scale" },
/* da */ { IDS_SCALE_DA, "" },  /* fuzzy */
/* de */ { IDS_SCALE_DE, "Scale" },
/* es */ { IDS_SCALE_ES, "" },  /* fuzzy */
/* fr */ { IDS_SCALE_FR, "" },  /* fuzzy */
/* hu */ { IDS_SCALE_HU, "" },  /* fuzzy */
/* it */ { IDS_SCALE_IT, "" },  /* fuzzy */
/* ko */ { IDS_SCALE_KO, "" },  /* fuzzy */
/* nl */ { IDS_SCALE_NL, "Schaal" },
/* pl */ { IDS_SCALE_PL, "" },  /* fuzzy */
/* ru */ { IDS_SCALE_RU, "" },  /* fuzzy */
/* sv */ { IDS_SCALE_SV, "" },  /* fuzzy */
/* tr */ { IDS_SCALE_TR, "" },  /* fuzzy */

/* en */ { IDS_CROP_LEFT_TOP,    "Crop left top" },
/* da */ { IDS_CROP_LEFT_TOP_DA, "" },  /* fuzzy */
/* de */ { IDS_CROP_LEFT_TOP_DE, "links oben abschneiden" },
/* es */ { IDS_CROP_LEFT_TOP_ES, "" },  /* fuzzy */
/* fr */ { IDS_CROP_LEFT_TOP_FR, "" },  /* fuzzy */
/* hu */ { IDS_CROP_LEFT_TOP_HU, "" },  /* fuzzy */
/* it */ { IDS_CROP_LEFT_TOP_IT, "" },  /* fuzzy */
/* ko */ { IDS_CROP_LEFT_TOP_KO, "" },  /* fuzzy */
/* nl */ { IDS_CROP_LEFT_TOP_NL, "Linker bovenkant afknippen" },
/* pl */ { IDS_CROP_LEFT_TOP_PL, "" },  /* fuzzy */
/* ru */ { IDS_CROP_LEFT_TOP_RU, "" },  /* fuzzy */
/* sv */ { IDS_CROP_LEFT_TOP_SV, "" },  /* fuzzy */
/* tr */ { IDS_CROP_LEFT_TOP_TR, "" },  /* fuzzy */

/* en */ { IDS_CROP_MIDDLE_TOP,    "Crop middle top" },
/* da */ { IDS_CROP_MIDDLE_TOP_DA, "" },  /* fuzzy */
/* de */ { IDS_CROP_MIDDLE_TOP_DE, "mitte oben abschneiden" },
/* es */ { IDS_CROP_MIDDLE_TOP_ES, "" },  /* fuzzy */
/* fr */ { IDS_CROP_MIDDLE_TOP_FR, "" },  /* fuzzy */
/* hu */ { IDS_CROP_MIDDLE_TOP_HU, "" },  /* fuzzy */
/* it */ { IDS_CROP_MIDDLE_TOP_IT, "" },  /* fuzzy */
/* ko */ { IDS_CROP_MIDDLE_TOP_KO, "" },  /* fuzzy */
/* nl */ { IDS_CROP_MIDDLE_TOP_NL, "Midden bovenkant afknippen" },
/* pl */ { IDS_CROP_MIDDLE_TOP_PL, "" },  /* fuzzy */
/* ru */ { IDS_CROP_MIDDLE_TOP_RU, "" },  /* fuzzy */
/* sv */ { IDS_CROP_MIDDLE_TOP_SV, "" },  /* fuzzy */
/* tr */ { IDS_CROP_MIDDLE_TOP_TR, "" },  /* fuzzy */

/* en */ { IDS_CROP_RIGHT_TOP,    "Crop right top" },
/* da */ { IDS_CROP_RIGHT_TOP_DA, "" },  /* fuzzy */
/* de */ { IDS_CROP_RIGHT_TOP_DE, "rechts oben abschneiden" },
/* es */ { IDS_CROP_RIGHT_TOP_ES, "" },  /* fuzzy */
/* fr */ { IDS_CROP_RIGHT_TOP_FR, "" },  /* fuzzy */
/* hu */ { IDS_CROP_RIGHT_TOP_HU, "" },  /* fuzzy */
/* it */ { IDS_CROP_RIGHT_TOP_IT, "" },  /* fuzzy */
/* ko */ { IDS_CROP_RIGHT_TOP_KO, "" },  /* fuzzy */
/* nl */ { IDS_CROP_RIGHT_TOP_NL, "Rechter bovenkant afknippen" },
/* pl */ { IDS_CROP_RIGHT_TOP_PL, "" },  /* fuzzy */
/* ru */ { IDS_CROP_RIGHT_TOP_RU, "" },  /* fuzzy */
/* sv */ { IDS_CROP_RIGHT_TOP_SV, "" },  /* fuzzy */
/* tr */ { IDS_CROP_RIGHT_TOP_TR, "" },  /* fuzzy */

/* en */ { IDS_CROP_LEFT_CENTER,    "Crop left center" },
/* da */ { IDS_CROP_LEFT_CENTER_DA, "" },  /* fuzzy */
/* de */ { IDS_CROP_LEFT_CENTER_DE, "links mitte abschneiden" },
/* es */ { IDS_CROP_LEFT_CENTER_ES, "" },  /* fuzzy */
/* fr */ { IDS_CROP_LEFT_CENTER_FR, "" },  /* fuzzy */
/* hu */ { IDS_CROP_LEFT_CENTER_HU, "" },  /* fuzzy */
/* it */ { IDS_CROP_LEFT_CENTER_IT, "" },  /* fuzzy */
/* ko */ { IDS_CROP_LEFT_CENTER_KO, "" },  /* fuzzy */
/* nl */ { IDS_CROP_LEFT_CENTER_NL, "Linker middendeel afknippen" },
/* pl */ { IDS_CROP_LEFT_CENTER_PL, "" },  /* fuzzy */
/* ru */ { IDS_CROP_LEFT_CENTER_RU, "" },  /* fuzzy */
/* sv */ { IDS_CROP_LEFT_CENTER_SV, "" },  /* fuzzy */
/* tr */ { IDS_CROP_LEFT_CENTER_TR, "" },  /* fuzzy */

/* en */ { IDS_CROP_MIDDLE_CENTER,    "Crop middle center" },
/* da */ { IDS_CROP_MIDDLE_CENTER_DA, "" },  /* fuzzy */
/* de */ { IDS_CROP_MIDDLE_CENTER_DE, "mitte mitte abschneiden" },
/* es */ { IDS_CROP_MIDDLE_CENTER_ES, "" },  /* fuzzy */
/* fr */ { IDS_CROP_MIDDLE_CENTER_FR, "" },  /* fuzzy */
/* hu */ { IDS_CROP_MIDDLE_CENTER_HU, "" },  /* fuzzy */
/* it */ { IDS_CROP_MIDDLE_CENTER_IT, "" },  /* fuzzy */
/* ko */ { IDS_CROP_MIDDLE_CENTER_KO, "" },  /* fuzzy */
/* nl */ { IDS_CROP_MIDDLE_CENTER_NL, "Middelste midden afknippen" },
/* pl */ { IDS_CROP_MIDDLE_CENTER_PL, "" },  /* fuzzy */
/* ru */ { IDS_CROP_MIDDLE_CENTER_RU, "" },  /* fuzzy */
/* sv */ { IDS_CROP_MIDDLE_CENTER_SV, "" },  /* fuzzy */
/* tr */ { IDS_CROP_MIDDLE_CENTER_TR, "" },  /* fuzzy */

/* en */ { IDS_CROP_RIGHT_CENTER,    "Crop right center" },
/* da */ { IDS_CROP_RIGHT_CENTER_DA, "" },  /* fuzzy */
/* de */ { IDS_CROP_RIGHT_CENTER_DE, "rechts mitte abschneiden" },
/* es */ { IDS_CROP_RIGHT_CENTER_ES, "" },  /* fuzzy */
/* fr */ { IDS_CROP_RIGHT_CENTER_FR, "" },  /* fuzzy */
/* hu */ { IDS_CROP_RIGHT_CENTER_HU, "" },  /* fuzzy */
/* it */ { IDS_CROP_RIGHT_CENTER_IT, "" },  /* fuzzy */
/* ko */ { IDS_CROP_RIGHT_CENTER_KO, "" },  /* fuzzy */
/* nl */ { IDS_CROP_RIGHT_CENTER_NL, "Rechter middendeel afknippen" },
/* pl */ { IDS_CROP_RIGHT_CENTER_PL, "" },  /* fuzzy */
/* ru */ { IDS_CROP_RIGHT_CENTER_RU, "" },  /* fuzzy */
/* sv */ { IDS_CROP_RIGHT_CENTER_SV, "" },  /* fuzzy */
/* tr */ { IDS_CROP_RIGHT_CENTER_TR, "" },  /* fuzzy */

/* en */ { IDS_CROP_LEFT_BOTTOM,    "Crop left bottom" },
/* da */ { IDS_CROP_LEFT_BOTTOM_DA, "" },  /* fuzzy */
/* de */ { IDS_CROP_LEFT_BOTTOM_DE, "links unten abschneiden" },
/* es */ { IDS_CROP_LEFT_BOTTOM_ES, "" },  /* fuzzy */
/* fr */ { IDS_CROP_LEFT_BOTTOM_FR, "" },  /* fuzzy */
/* hu */ { IDS_CROP_LEFT_BOTTOM_HU, "" },  /* fuzzy */
/* it */ { IDS_CROP_LEFT_BOTTOM_IT, "" },  /* fuzzy */
/* ko */ { IDS_CROP_LEFT_BOTTOM_KO, "" },  /* fuzzy */
/* nl */ { IDS_CROP_LEFT_BOTTOM_NL, "Linker onderkant afknippen" },
/* pl */ { IDS_CROP_LEFT_BOTTOM_PL, "" },  /* fuzzy */
/* ru */ { IDS_CROP_LEFT_BOTTOM_RU, "" },  /* fuzzy */
/* sv */ { IDS_CROP_LEFT_BOTTOM_SV, "" },  /* fuzzy */
/* tr */ { IDS_CROP_LEFT_BOTTOM_TR, "" },  /* fuzzy */

/* en */ { IDS_CROP_MIDDLE_BOTTOM,    "Crop middle bottom" },
/* da */ { IDS_CROP_MIDDLE_BOTTOM_DA, "" },  /* fuzzy */
/* de */ { IDS_CROP_MIDDLE_BOTTOM_DE, "mitte unten abschneiden" },
/* es */ { IDS_CROP_MIDDLE_BOTTOM_ES, "" },  /* fuzzy */
/* fr */ { IDS_CROP_MIDDLE_BOTTOM_FR, "" },  /* fuzzy */
/* hu */ { IDS_CROP_MIDDLE_BOTTOM_HU, "" },  /* fuzzy */
/* it */ { IDS_CROP_MIDDLE_BOTTOM_IT, "" },  /* fuzzy */
/* ko */ { IDS_CROP_MIDDLE_BOTTOM_KO, "" },  /* fuzzy */
/* nl */ { IDS_CROP_MIDDLE_BOTTOM_NL, "Midden onderkant afknippen" },
/* pl */ { IDS_CROP_MIDDLE_BOTTOM_PL, "" },  /* fuzzy */
/* ru */ { IDS_CROP_MIDDLE_BOTTOM_RU, "" },  /* fuzzy */
/* sv */ { IDS_CROP_MIDDLE_BOTTOM_SV, "" },  /* fuzzy */
/* tr */ { IDS_CROP_MIDDLE_BOTTOM_TR, "" },  /* fuzzy */

/* en */ { IDS_CROP_RIGHT_BOTTOM,    "Crop right bottom" },
/* da */ { IDS_CROP_RIGHT_BOTTOM_DA, "" },  /* fuzzy */
/* de */ { IDS_CROP_RIGHT_BOTTOM_DE, "rechts unten abschneiden" },
/* es */ { IDS_CROP_RIGHT_BOTTOM_ES, "" },  /* fuzzy */
/* fr */ { IDS_CROP_RIGHT_BOTTOM_FR, "" },  /* fuzzy */
/* hu */ { IDS_CROP_RIGHT_BOTTOM_HU, "" },  /* fuzzy */
/* it */ { IDS_CROP_RIGHT_BOTTOM_IT, "" },  /* fuzzy */
/* ko */ { IDS_CROP_RIGHT_BOTTOM_KO, "" },  /* fuzzy */
/* nl */ { IDS_CROP_RIGHT_BOTTOM_NL, "Rechter onderkant afknippen" },
/* pl */ { IDS_CROP_RIGHT_BOTTOM_PL, "" },  /* fuzzy */
/* ru */ { IDS_CROP_RIGHT_BOTTOM_RU, "" },  /* fuzzy */
/* sv */ { IDS_CROP_RIGHT_BOTTOM_SV, "" },  /* fuzzy */
/* tr */ { IDS_CROP_RIGHT_BOTTOM_TR, "" },  /* fuzzy */

/* en */ { IDS_BORDERIZE,    "Borderize" },
/* da */ { IDS_BORDERIZE_DA, "" },  /* fuzzy */
/* de */ { IDS_BORDERIZE_DE, "Einrahmen" },
/* es */ { IDS_BORDERIZE_ES, "" },  /* fuzzy */
/* fr */ { IDS_BORDERIZE_FR, "" },  /* fuzzy */
/* hu */ { IDS_BORDERIZE_HU, "" },  /* fuzzy */
/* it */ { IDS_BORDERIZE_IT, "" },  /* fuzzy */
/* ko */ { IDS_BORDERIZE_KO, "" },  /* fuzzy */
/* nl */ { IDS_BORDERIZE_NL, "Vul in met border" },
/* pl */ { IDS_BORDERIZE_PL, "" },  /* fuzzy */
/* ru */ { IDS_BORDERIZE_RU, "" },  /* fuzzy */
/* sv */ { IDS_BORDERIZE_SV, "" },  /* fuzzy */
/* tr */ { IDS_BORDERIZE_TR, "" },  /* fuzzy */

/* en */ { IDS_BLACK_AND_WHITE,    "Black & white" },
/* da */ { IDS_BLACK_AND_WHITE_DA, "" },  /* fuzzy */
/* de */ { IDS_BLACK_AND_WHITE_DE, "Schwarz & Weiß" },
/* es */ { IDS_BLACK_AND_WHITE_ES, "" },  /* fuzzy */
/* fr */ { IDS_BLACK_AND_WHITE_FR, "" },  /* fuzzy */
/* hu */ { IDS_BLACK_AND_WHITE_HU, "" },  /* fuzzy */
/* it */ { IDS_BLACK_AND_WHITE_IT, "" },  /* fuzzy */
/* ko */ { IDS_BLACK_AND_WHITE_KO, "" },  /* fuzzy */
/* nl */ { IDS_BLACK_AND_WHITE_NL, "Zwart & wit" },
/* pl */ { IDS_BLACK_AND_WHITE_PL, "" },  /* fuzzy */
/* ru */ { IDS_BLACK_AND_WHITE_RU, "" },  /* fuzzy */
/* sv */ { IDS_BLACK_AND_WHITE_SV, "" },  /* fuzzy */
/* tr */ { IDS_BLACK_AND_WHITE_TR, "" },  /* fuzzy */

/* en */ { IDS_2_COLORS,    "2 colors" },
/* da */ { IDS_2_COLORS_DA, "" },  /* fuzzy */
/* de */ { IDS_2_COLORS_DE, "2 Farben" },
/* es */ { IDS_2_COLORS_ES, "2 colores" },
/* fr */ { IDS_2_COLORS_FR, "" },  /* fuzzy */
/* hu */ { IDS_2_COLORS_HU, "" },  /* fuzzy */
/* it */ { IDS_2_COLORS_IT, "2 colori" },
/* ko */ { IDS_2_COLORS_KO, "" },  /* fuzzy */
/* nl */ { IDS_2_COLORS_NL, "2 kleuren" },
/* pl */ { IDS_2_COLORS_PL, "2 barwy" },
/* ru */ { IDS_2_COLORS_RU, "" },  /* fuzzy */
/* sv */ { IDS_2_COLORS_SV, "2 fäger" },
/* tr */ { IDS_2_COLORS_TR, "" },  /* fuzzy */

/* en */ { IDS_4_COLORS,    "4 colors" },
/* da */ { IDS_4_COLORS_DA, "" },  /* fuzzy */
/* de */ { IDS_4_COLORS_DE, "4 Farben" },
/* es */ { IDS_4_COLORS_ES, "4 colores" },
/* fr */ { IDS_4_COLORS_FR, "" },  /* fuzzy */
/* hu */ { IDS_4_COLORS_HU, "" },  /* fuzzy */
/* it */ { IDS_4_COLORS_IT, "4 colori" },
/* ko */ { IDS_4_COLORS_KO, "" },  /* fuzzy */
/* nl */ { IDS_4_COLORS_NL, "4 kleuren" },
/* pl */ { IDS_4_COLORS_PL, "4 barwy" },
/* ru */ { IDS_4_COLORS_RU, "" },  /* fuzzy */
/* sv */ { IDS_4_COLORS_SV, "4 färger" },
/* tr */ { IDS_4_COLORS_TR, "" },  /* fuzzy */

/* en */ { IDS_GRAY_SCALE,    "Gray scale" },
/* da */ { IDS_GRAY_SCALE_DA, "" },  /* fuzzy */
/* de */ { IDS_GRAY_SCALE_DE, "Graustufe" },
/* es */ { IDS_GRAY_SCALE_ES, "" },  /* fuzzy */
/* fr */ { IDS_GRAY_SCALE_FR, "" },  /* fuzzy */
/* hu */ { IDS_GRAY_SCALE_HU, "" },  /* fuzzy */
/* it */ { IDS_GRAY_SCALE_IT, "" },  /* fuzzy */
/* ko */ { IDS_GRAY_SCALE_KO, "" },  /* fuzzy */
/* nl */ { IDS_GRAY_SCALE_NL, "Grijswaarden" },
/* pl */ { IDS_GRAY_SCALE_PL, "" },  /* fuzzy */
/* ru */ { IDS_GRAY_SCALE_RU, "" },  /* fuzzy */
/* sv */ { IDS_GRAY_SCALE_SV, "" },  /* fuzzy */
/* tr */ { IDS_GRAY_SCALE_TR, "" },  /* fuzzy */

/* en */ { IDS_DITHER,    "Dither" },
/* da */ { IDS_DITHER_DA, "" },  /* fuzzy */
/* de */ { IDS_DITHER_DE, "annähern" },
/* es */ { IDS_DITHER_ES, "" },  /* fuzzy */
/* fr */ { IDS_DITHER_FR, "" },  /* fuzzy */
/* hu */ { IDS_DITHER_HU, "" },  /* fuzzy */
/* it */ { IDS_DITHER_IT, "" },  /* fuzzy */
/* ko */ { IDS_DITHER_KO, "" },  /* fuzzy */
/* nl */ { IDS_DITHER_NL, "Dither" },
/* pl */ { IDS_DITHER_PL, "" },  /* fuzzy */
/* ru */ { IDS_DITHER_RU, "" },  /* fuzzy */
/* sv */ { IDS_DITHER_SV, "" },  /* fuzzy */
/* tr */ { IDS_DITHER_TR, "" },  /* fuzzy */

/* en */ { IDS_IGNORE,    "Ignore" },
/* da */ { IDS_IGNORE_DA, "" },  /* fuzzy */
/* de */ { IDS_IGNORE_DE, "ignorieren" },
/* es */ { IDS_IGNORE_ES, "" },  /* fuzzy */
/* fr */ { IDS_IGNORE_FR, "" },  /* fuzzy */
/* hu */ { IDS_IGNORE_HU, "" },  /* fuzzy */
/* it */ { IDS_IGNORE_IT, "" },  /* fuzzy */
/* ko */ { IDS_IGNORE_KO, "" },  /* fuzzy */
/* nl */ { IDS_IGNORE_NL, "Negeren" },
/* pl */ { IDS_IGNORE_PL, "" },  /* fuzzy */
/* ru */ { IDS_IGNORE_RU, "" },  /* fuzzy */
/* sv */ { IDS_IGNORE_SV, "" },  /* fuzzy */
/* tr */ { IDS_IGNORE_TR, "" },  /* fuzzy */

/* en */ { IDS_WHITE,    "White" },
/* da */ { IDS_WHITE_DA, "Hvid" },
/* de */ { IDS_WHITE_DE, "Weiß" },
/* es */ { IDS_WHITE_ES, "Blanco" },
/* fr */ { IDS_WHITE_FR, "Blanc" },
/* hu */ { IDS_WHITE_HU, "" },  /* fuzzy */
/* it */ { IDS_WHITE_IT, "Bianco" },
/* ko */ { IDS_WHITE_KO, "Èò»ö" },
/* nl */ { IDS_WHITE_NL, "Wit" },
/* pl */ { IDS_WHITE_PL, "Bia³e" },
/* ru */ { IDS_WHITE_RU, "White" },
/* sv */ { IDS_WHITE_SV, "Vit" },
/* tr */ { IDS_WHITE_TR, "Beyaz" },

/* en */ { IDS_AMBER,    "Amber" },
/* da */ { IDS_AMBER_DA, "Rav-farvet" },
/* de */ { IDS_AMBER_DE, "Bernstein" },
/* es */ { IDS_AMBER_ES, "Ambar" },
/* fr */ { IDS_AMBER_FR, "Ambre" },
/* hu */ { IDS_AMBER_HU, "" },  /* fuzzy */
/* it */ { IDS_AMBER_IT, "Ambra" },
/* ko */ { IDS_AMBER_KO, "È²»ö" },
/* nl */ { IDS_AMBER_NL, "Amber" },
/* pl */ { IDS_AMBER_PL, "Bursztynowe" },
/* ru */ { IDS_AMBER_RU, "Amber" },
/* sv */ { IDS_AMBER_SV, "Bärnstensfärgad" },
/* tr */ { IDS_AMBER_TR, "Kehribar" },

/* en */ { IDS_GREEN,    "Green" },
/* da */ { IDS_GREEN_DA, "" },  /* fuzzy */
/* de */ { IDS_GREEN_DE, "Grün" },
/* es */ { IDS_GREEN_ES, "" },  /* fuzzy */
/* fr */ { IDS_GREEN_FR, "" },  /* fuzzy */
/* hu */ { IDS_GREEN_HU, "" },  /* fuzzy */
/* it */ { IDS_GREEN_IT, "" },  /* fuzzy */
/* ko */ { IDS_GREEN_KO, "" },  /* fuzzy */
/* nl */ { IDS_GREEN_NL, "Groen" },
/* pl */ { IDS_GREEN_PL, "" },  /* fuzzy */
/* ru */ { IDS_GREEN_RU, "" },  /* fuzzy */
/* sv */ { IDS_GREEN_SV, "" },  /* fuzzy */
/* tr */ { IDS_GREEN_TR, "" },  /* fuzzy */

/* en */ { IDS_DOODLE_OVERSIZE_HANDLING,    "Doodle oversize handling" },
/* da */ { IDS_DOODLE_OVERSIZE_HANDLING_DA, "" },  /* fuzzy */
/* de */ { IDS_DOODLE_OVERSIZE_HANDLING_DE, "Doodle Übergrößenbehandlung" },
/* es */ { IDS_DOODLE_OVERSIZE_HANDLING_ES, "" },  /* fuzzy */
/* fr */ { IDS_DOODLE_OVERSIZE_HANDLING_FR, "" },  /* fuzzy */
/* hu */ { IDS_DOODLE_OVERSIZE_HANDLING_HU, "" },  /* fuzzy */
/* it */ { IDS_DOODLE_OVERSIZE_HANDLING_IT, "" },  /* fuzzy */
/* ko */ { IDS_DOODLE_OVERSIZE_HANDLING_KO, "" },  /* fuzzy */
/* nl */ { IDS_DOODLE_OVERSIZE_HANDLING_NL, "Doodle 'te groot' afhandeling" },
/* pl */ { IDS_DOODLE_OVERSIZE_HANDLING_PL, "" },  /* fuzzy */
/* ru */ { IDS_DOODLE_OVERSIZE_HANDLING_RU, "" },  /* fuzzy */
/* sv */ { IDS_DOODLE_OVERSIZE_HANDLING_SV, "" },  /* fuzzy */
/* tr */ { IDS_DOODLE_OVERSIZE_HANDLING_TR, "" },  /* fuzzy */

/* en */ { IDS_DOODLE_UNDERSIZE_HANDLING,    "Doodle undersize handling" },
/* da */ { IDS_DOODLE_UNDERSIZE_HANDLING_DA, "" },  /* fuzzy */
/* de */ { IDS_DOODLE_UNDERSIZE_HANDLING_DE, "Doodle Untergrößenbehandlung" },
/* es */ { IDS_DOODLE_UNDERSIZE_HANDLING_ES, "" },  /* fuzzy */
/* fr */ { IDS_DOODLE_UNDERSIZE_HANDLING_FR, "" },  /* fuzzy */
/* hu */ { IDS_DOODLE_UNDERSIZE_HANDLING_HU, "" },  /* fuzzy */
/* it */ { IDS_DOODLE_UNDERSIZE_HANDLING_IT, "" },  /* fuzzy */
/* ko */ { IDS_DOODLE_UNDERSIZE_HANDLING_KO, "" },  /* fuzzy */
/* nl */ { IDS_DOODLE_UNDERSIZE_HANDLING_NL, "Doodle 'te klein' afhandeling" },
/* pl */ { IDS_DOODLE_UNDERSIZE_HANDLING_PL, "" },  /* fuzzy */
/* ru */ { IDS_DOODLE_UNDERSIZE_HANDLING_RU, "" },  /* fuzzy */
/* sv */ { IDS_DOODLE_UNDERSIZE_HANDLING_SV, "" },  /* fuzzy */
/* tr */ { IDS_DOODLE_UNDERSIZE_HANDLING_TR, "" },  /* fuzzy */

/* en */ { IDS_DOODLE_MULTICOLOR_HANDLING,    "Doodle multicolor handling" },
/* da */ { IDS_DOODLE_MULTICOLOR_HANDLING_DA, "" },  /* fuzzy */
/* de */ { IDS_DOODLE_MULTICOLOR_HANDLING_DE, "Doodle Multicolorbehandlung" },
/* es */ { IDS_DOODLE_MULTICOLOR_HANDLING_ES, "" },  /* fuzzy */
/* fr */ { IDS_DOODLE_MULTICOLOR_HANDLING_FR, "" },  /* fuzzy */
/* hu */ { IDS_DOODLE_MULTICOLOR_HANDLING_HU, "" },  /* fuzzy */
/* it */ { IDS_DOODLE_MULTICOLOR_HANDLING_IT, "" },  /* fuzzy */
/* ko */ { IDS_DOODLE_MULTICOLOR_HANDLING_KO, "" },  /* fuzzy */
/* nl */ { IDS_DOODLE_MULTICOLOR_HANDLING_NL, "Doodle multicolor afhandeling" },
/* pl */ { IDS_DOODLE_MULTICOLOR_HANDLING_PL, "" },  /* fuzzy */
/* ru */ { IDS_DOODLE_MULTICOLOR_HANDLING_RU, "" },  /* fuzzy */
/* sv */ { IDS_DOODLE_MULTICOLOR_HANDLING_SV, "" },  /* fuzzy */
/* tr */ { IDS_DOODLE_MULTICOLOR_HANDLING_TR, "" },  /* fuzzy */

/* en */ { IDS_DOODLE_TED_LUMINOSITY_HANDLING,    "Doodle TED luminosity handling" },
/* da */ { IDS_DOODLE_TED_LUMINOSITY_HANDLING_DA, "" },  /* fuzzy */
/* de */ { IDS_DOODLE_TED_LUMINOSITY_HANDLING_DE, "Doodle TED Luma Behandlung" },
/* es */ { IDS_DOODLE_TED_LUMINOSITY_HANDLING_ES, "" },  /* fuzzy */
/* fr */ { IDS_DOODLE_TED_LUMINOSITY_HANDLING_FR, "" },  /* fuzzy */
/* hu */ { IDS_DOODLE_TED_LUMINOSITY_HANDLING_HU, "" },  /* fuzzy */
/* it */ { IDS_DOODLE_TED_LUMINOSITY_HANDLING_IT, "" },  /* fuzzy */
/* ko */ { IDS_DOODLE_TED_LUMINOSITY_HANDLING_KO, "" },  /* fuzzy */
/* nl */ { IDS_DOODLE_TED_LUMINOSITY_HANDLING_NL, "Doodle TED helderheid afhandeling" },
/* pl */ { IDS_DOODLE_TED_LUMINOSITY_HANDLING_PL, "" },  /* fuzzy */
/* ru */ { IDS_DOODLE_TED_LUMINOSITY_HANDLING_RU, "" },  /* fuzzy */
/* sv */ { IDS_DOODLE_TED_LUMINOSITY_HANDLING_SV, "" },  /* fuzzy */
/* tr */ { IDS_DOODLE_TED_LUMINOSITY_HANDLING_TR, "" },  /* fuzzy */

/* en */ { IDS_DOODLE_CRTC_TEXT_COLOR,    "Doodle CRTC text color" },
/* da */ { IDS_DOODLE_CRTC_TEXT_COLOR_DA, "" },  /* fuzzy */
/* de */ { IDS_DOODLE_CRTC_TEXT_COLOR_DE, "Doodle CRTC Textfarben" },
/* es */ { IDS_DOODLE_CRTC_TEXT_COLOR_ES, "" },  /* fuzzy */
/* fr */ { IDS_DOODLE_CRTC_TEXT_COLOR_FR, "" },  /* fuzzy */
/* hu */ { IDS_DOODLE_CRTC_TEXT_COLOR_HU, "" },  /* fuzzy */
/* it */ { IDS_DOODLE_CRTC_TEXT_COLOR_IT, "" },  /* fuzzy */
/* ko */ { IDS_DOODLE_CRTC_TEXT_COLOR_KO, "" },  /* fuzzy */
/* nl */ { IDS_DOODLE_CRTC_TEXT_COLOR_NL, "Doodle CRTC tekst kleur" },
/* pl */ { IDS_DOODLE_CRTC_TEXT_COLOR_PL, "" },  /* fuzzy */
/* ru */ { IDS_DOODLE_CRTC_TEXT_COLOR_RU, "" },  /* fuzzy */
/* sv */ { IDS_DOODLE_CRTC_TEXT_COLOR_SV, "" },  /* fuzzy */
/* tr */ { IDS_DOODLE_CRTC_TEXT_COLOR_TR, "" },  /* fuzzy */

/* en */ { IDS_KOALA_OVERSIZE_HANDLING,    "Koala oversize handling" },
/* da */ { IDS_KOALA_OVERSIZE_HANDLING_DA, "" },  /* fuzzy */
/* de */ { IDS_KOALA_OVERSIZE_HANDLING_DE, "Koala Übergrößenbehandlung" },
/* es */ { IDS_KOALA_OVERSIZE_HANDLING_ES, "" },  /* fuzzy */
/* fr */ { IDS_KOALA_OVERSIZE_HANDLING_FR, "" },  /* fuzzy */
/* hu */ { IDS_KOALA_OVERSIZE_HANDLING_HU, "" },  /* fuzzy */
/* it */ { IDS_KOALA_OVERSIZE_HANDLING_IT, "" },  /* fuzzy */
/* ko */ { IDS_KOALA_OVERSIZE_HANDLING_KO, "" },  /* fuzzy */
/* nl */ { IDS_KOALA_OVERSIZE_HANDLING_NL, "Koala 'te groot' afhandeling" },
/* pl */ { IDS_KOALA_OVERSIZE_HANDLING_PL, "" },  /* fuzzy */
/* ru */ { IDS_KOALA_OVERSIZE_HANDLING_RU, "" },  /* fuzzy */
/* sv */ { IDS_KOALA_OVERSIZE_HANDLING_SV, "" },  /* fuzzy */
/* tr */ { IDS_KOALA_OVERSIZE_HANDLING_TR, "" },  /* fuzzy */

/* en */ { IDS_KOALA_UNDERSIZE_HANDLING,    "Koala undersize handling" },
/* da */ { IDS_KOALA_UNDERSIZE_HANDLING_DA, "" },  /* fuzzy */
/* de */ { IDS_KOALA_UNDERSIZE_HANDLING_DE, "Koala Untergrößenbehandlung" },
/* es */ { IDS_KOALA_UNDERSIZE_HANDLING_ES, "" },  /* fuzzy */
/* fr */ { IDS_KOALA_UNDERSIZE_HANDLING_FR, "" },  /* fuzzy */
/* hu */ { IDS_KOALA_UNDERSIZE_HANDLING_HU, "" },  /* fuzzy */
/* it */ { IDS_KOALA_UNDERSIZE_HANDLING_IT, "" },  /* fuzzy */
/* ko */ { IDS_KOALA_UNDERSIZE_HANDLING_KO, "" },  /* fuzzy */
/* nl */ { IDS_KOALA_UNDERSIZE_HANDLING_NL, "Koala 'te klein' afhandeling" },
/* pl */ { IDS_KOALA_UNDERSIZE_HANDLING_PL, "" },  /* fuzzy */
/* ru */ { IDS_KOALA_UNDERSIZE_HANDLING_RU, "" },  /* fuzzy */
/* sv */ { IDS_KOALA_UNDERSIZE_HANDLING_SV, "" },  /* fuzzy */
/* tr */ { IDS_KOALA_UNDERSIZE_HANDLING_TR, "" },  /* fuzzy */

/* en */ { IDS_KOALA_TED_LUMINOSITY_HANDLING,    "Koala TED luminosity handling" },
/* da */ { IDS_KOALA_TED_LUMINOSITY_HANDLING_DA, "" },  /* fuzzy */
/* de */ { IDS_KOALA_TED_LUMINOSITY_HANDLING_DE, "Koala TED Luma Behandlung" },
/* es */ { IDS_KOALA_TED_LUMINOSITY_HANDLING_ES, "" },  /* fuzzy */
/* fr */ { IDS_KOALA_TED_LUMINOSITY_HANDLING_FR, "" },  /* fuzzy */
/* hu */ { IDS_KOALA_TED_LUMINOSITY_HANDLING_HU, "" },  /* fuzzy */
/* it */ { IDS_KOALA_TED_LUMINOSITY_HANDLING_IT, "" },  /* fuzzy */
/* ko */ { IDS_KOALA_TED_LUMINOSITY_HANDLING_KO, "" },  /* fuzzy */
/* nl */ { IDS_KOALA_TED_LUMINOSITY_HANDLING_NL, "Koala TED helderheid afhandeling" },
/* pl */ { IDS_KOALA_TED_LUMINOSITY_HANDLING_PL, "" },  /* fuzzy */
/* ru */ { IDS_KOALA_TED_LUMINOSITY_HANDLING_RU, "" },  /* fuzzy */
/* sv */ { IDS_KOALA_TED_LUMINOSITY_HANDLING_SV, "" },  /* fuzzy */
/* tr */ { IDS_KOALA_TED_LUMINOSITY_HANDLING_TR, "" },  /* fuzzy */

/* en */ { IDS_KOALA_CRTC_TEXT_COLOR,    "Koala CRTC text color" },
/* da */ { IDS_KOALA_CRTC_TEXT_COLOR_DA, "" },  /* fuzzy */
/* de */ { IDS_KOALA_CRTC_TEXT_COLOR_DE, "Koala CRTC Textfarben" },
/* es */ { IDS_KOALA_CRTC_TEXT_COLOR_ES, "" },  /* fuzzy */
/* fr */ { IDS_KOALA_CRTC_TEXT_COLOR_FR, "" },  /* fuzzy */
/* hu */ { IDS_KOALA_CRTC_TEXT_COLOR_HU, "" },  /* fuzzy */
/* it */ { IDS_KOALA_CRTC_TEXT_COLOR_IT, "" },  /* fuzzy */
/* ko */ { IDS_KOALA_CRTC_TEXT_COLOR_KO, "" },  /* fuzzy */
/* nl */ { IDS_KOALA_CRTC_TEXT_COLOR_NL, "Koala CRTC tekst kleur" },
/* pl */ { IDS_KOALA_CRTC_TEXT_COLOR_PL, "" },  /* fuzzy */
/* ru */ { IDS_KOALA_CRTC_TEXT_COLOR_RU, "" },  /* fuzzy */
/* sv */ { IDS_KOALA_CRTC_TEXT_COLOR_SV, "" },  /* fuzzy */
/* tr */ { IDS_KOALA_CRTC_TEXT_COLOR_TR, "" },  /* fuzzy */

/* en */ { IDS_SCREENSHOT_SETTINGS,    "Screenshot settings" },
/* da */ { IDS_SCREENSHOT_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDS_SCREENSHOT_SETTINGS_DE, "Screenshot Einstellungen" },
/* es */ { IDS_SCREENSHOT_SETTINGS_ES, "" },  /* fuzzy */
/* fr */ { IDS_SCREENSHOT_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDS_SCREENSHOT_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_SCREENSHOT_SETTINGS_IT, "" },  /* fuzzy */
/* ko */ { IDS_SCREENSHOT_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_SCREENSHOT_SETTINGS_NL, "Schermafdruk instellingen" },
/* pl */ { IDS_SCREENSHOT_SETTINGS_PL, "" },  /* fuzzy */
/* ru */ { IDS_SCREENSHOT_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_SCREENSHOT_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDS_SCREENSHOT_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDS_DETACH_ALL,    "Detach all involved carts" },
/* da */ { IDS_DETACH_ALL_DA, "" },  /* fuzzy */
/* de */ { IDS_DETACH_ALL_DE, "Sämtliche betroffene Erweiterungsmodule entfernen" },
/* es */ { IDS_DETACH_ALL_ES, "" },  /* fuzzy */
/* fr */ { IDS_DETACH_ALL_FR, "" },  /* fuzzy */
/* hu */ { IDS_DETACH_ALL_HU, "" },  /* fuzzy */
/* it */ { IDS_DETACH_ALL_IT, "" },  /* fuzzy */
/* ko */ { IDS_DETACH_ALL_KO, "" },  /* fuzzy */
/* nl */ { IDS_DETACH_ALL_NL, "Ontkoppel alle betrokken cartridges" },
/* pl */ { IDS_DETACH_ALL_PL, "" },  /* fuzzy */
/* ru */ { IDS_DETACH_ALL_RU, "" },  /* fuzzy */
/* sv */ { IDS_DETACH_ALL_SV, "" },  /* fuzzy */
/* tr */ { IDS_DETACH_ALL_TR, "" },  /* fuzzy */

/* en */ { IDS_DETACH_LAST,    "Detach last inserted cart" },
/* da */ { IDS_DETACH_LAST_DA, "" },  /* fuzzy */
/* de */ { IDS_DETACH_LAST_DE, "Letztes eingelegtes Erweiterungsmodul entfernen" },
/* es */ { IDS_DETACH_LAST_ES, "" },  /* fuzzy */
/* fr */ { IDS_DETACH_LAST_FR, "" },  /* fuzzy */
/* hu */ { IDS_DETACH_LAST_HU, "" },  /* fuzzy */
/* it */ { IDS_DETACH_LAST_IT, "" },  /* fuzzy */
/* ko */ { IDS_DETACH_LAST_KO, "" },  /* fuzzy */
/* nl */ { IDS_DETACH_LAST_NL, "Ontkoppel de cartridge die als laatste was geactiveerd" },
/* pl */ { IDS_DETACH_LAST_PL, "" },  /* fuzzy */
/* ru */ { IDS_DETACH_LAST_RU, "" },  /* fuzzy */
/* sv */ { IDS_DETACH_LAST_SV, "" },  /* fuzzy */
/* tr */ { IDS_DETACH_LAST_TR, "" },  /* fuzzy */

/* en */ { IDS_AND_WIRES,    "'AND' the wires" },
/* da */ { IDS_AND_WIRES_DA, "" },  /* fuzzy */
/* de */ { IDS_AND_WIRES_DE, "'AND' der Verbindungen" },
/* es */ { IDS_AND_WIRES_ES, "" },  /* fuzzy */
/* fr */ { IDS_AND_WIRES_FR, "" },  /* fuzzy */
/* hu */ { IDS_AND_WIRES_HU, "" },  /* fuzzy */
/* it */ { IDS_AND_WIRES_IT, "" },  /* fuzzy */
/* ko */ { IDS_AND_WIRES_KO, "" },  /* fuzzy */
/* nl */ { IDS_AND_WIRES_NL, "'AND' de draden" },
/* pl */ { IDS_AND_WIRES_PL, "" },  /* fuzzy */
/* ru */ { IDS_AND_WIRES_RU, "" },  /* fuzzy */
/* sv */ { IDS_AND_WIRES_SV, "" },  /* fuzzy */
/* tr */ { IDS_AND_WIRES_TR, "" },  /* fuzzy */

/* en */ { IDS_IO_COLLISION_HANDLING,    "I/O collision handling" },
/* da */ { IDS_IO_COLLISION_HANDLING_DA, "I/O kollisionshåndtering" },
/* de */ { IDS_IO_COLLISION_HANDLING_DE, "I/O Kollisionsbehandlung" },
/* es */ { IDS_IO_COLLISION_HANDLING_ES, "Manejo de colisiones I/O" },
/* fr */ { IDS_IO_COLLISION_HANDLING_FR, "Prise en charge collision E/S" },
/* hu */ { IDS_IO_COLLISION_HANDLING_HU, "" },  /* fuzzy */
/* it */ { IDS_IO_COLLISION_HANDLING_IT, "Gestione delle collisioni di I/O" },
/* ko */ { IDS_IO_COLLISION_HANDLING_KO, "" },  /* fuzzy */
/* nl */ { IDS_IO_COLLISION_HANDLING_NL, "I/O conflict afhandeling" },
/* pl */ { IDS_IO_COLLISION_HANDLING_PL, "Obs³uga kolizji we/wy" },
/* ru */ { IDS_IO_COLLISION_HANDLING_RU, "I/O collision handling" },
/* sv */ { IDS_IO_COLLISION_HANDLING_SV, "hantering av I/O-kollisioner" },
/* tr */ { IDS_IO_COLLISION_HANDLING_TR, "G/Ç çakýþma iþleme" },

/* en */ { IDS_IO_COLLISION_SETTINGS,    "I/O collision settings" },
/* da */ { IDS_IO_COLLISION_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDS_IO_COLLISION_SETTINGS_DE, "I/O Kollisionsbehandlung" },
/* es */ { IDS_IO_COLLISION_SETTINGS_ES, "" },  /* fuzzy */
/* fr */ { IDS_IO_COLLISION_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDS_IO_COLLISION_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_IO_COLLISION_SETTINGS_IT, "" },  /* fuzzy */
/* ko */ { IDS_IO_COLLISION_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_IO_COLLISION_SETTINGS_NL, "I/O conflict instellingen" },
/* pl */ { IDS_IO_COLLISION_SETTINGS_PL, "" },  /* fuzzy */
/* ru */ { IDS_IO_COLLISION_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_IO_COLLISION_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDS_IO_COLLISION_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDS_RUNNING,    "Running" },
/* da */ { IDS_RUNNING_DA, "" },  /* fuzzy */
/* de */ { IDS_RUNNING_DE, "Laufend" },
/* es */ { IDS_RUNNING_ES, "" },  /* fuzzy */
/* fr */ { IDS_RUNNING_FR, "" },  /* fuzzy */
/* hu */ { IDS_RUNNING_HU, "" },  /* fuzzy */
/* it */ { IDS_RUNNING_IT, "" },  /* fuzzy */
/* ko */ { IDS_RUNNING_KO, "" },  /* fuzzy */
/* nl */ { IDS_RUNNING_NL, "Werkend" },
/* pl */ { IDS_RUNNING_PL, "" },  /* fuzzy */
/* ru */ { IDS_RUNNING_RU, "" },  /* fuzzy */
/* sv */ { IDS_RUNNING_SV, "" },  /* fuzzy */
/* tr */ { IDS_RUNNING_TR, "" },  /* fuzzy */

/* en */ { IDS_HALTED,    "Halted" },
/* da */ { IDS_HALTED_DA, "" },  /* fuzzy */
/* de */ { IDS_HALTED_DE, "Gestoppt" },
/* es */ { IDS_HALTED_ES, "" },  /* fuzzy */
/* fr */ { IDS_HALTED_FR, "" },  /* fuzzy */
/* hu */ { IDS_HALTED_HU, "" },  /* fuzzy */
/* it */ { IDS_HALTED_IT, "" },  /* fuzzy */
/* ko */ { IDS_HALTED_KO, "" },  /* fuzzy */
/* nl */ { IDS_HALTED_NL, "Gestopt" },
/* pl */ { IDS_HALTED_PL, "" },  /* fuzzy */
/* ru */ { IDS_HALTED_RU, "" },  /* fuzzy */
/* sv */ { IDS_HALTED_SV, "" },  /* fuzzy */
/* tr */ { IDS_HALTED_TR, "" },  /* fuzzy */

/* en */ { IDS_GENERIC_CARTS,    "Generic Cartridges" },
/* da */ { IDS_GENERIC_CARTS_DA, "" },  /* fuzzy */
/* de */ { IDS_GENERIC_CARTS_DE, "Universelle Erweiterungsmodule" },
/* es */ { IDS_GENERIC_CARTS_ES, "" },  /* fuzzy */
/* fr */ { IDS_GENERIC_CARTS_FR, "" },  /* fuzzy */
/* hu */ { IDS_GENERIC_CARTS_HU, "" },  /* fuzzy */
/* it */ { IDS_GENERIC_CARTS_IT, "" },  /* fuzzy */
/* ko */ { IDS_GENERIC_CARTS_KO, "" },  /* fuzzy */
/* nl */ { IDS_GENERIC_CARTS_NL, "Algemene Cartridges" },
/* pl */ { IDS_GENERIC_CARTS_PL, "" },  /* fuzzy */
/* ru */ { IDS_GENERIC_CARTS_RU, "" },  /* fuzzy */
/* sv */ { IDS_GENERIC_CARTS_SV, "" },  /* fuzzy */
/* tr */ { IDS_GENERIC_CARTS_TR, "" },  /* fuzzy */

/* en */ { IDS_FREEZER_CARTS,    "Freezer Cartridges" },
/* da */ { IDS_FREEZER_CARTS_DA, "" },  /* fuzzy */
/* de */ { IDS_FREEZER_CARTS_DE, "Freezer Erweiterungsmodule" },
/* es */ { IDS_FREEZER_CARTS_ES, "" },  /* fuzzy */
/* fr */ { IDS_FREEZER_CARTS_FR, "" },  /* fuzzy */
/* hu */ { IDS_FREEZER_CARTS_HU, "" },  /* fuzzy */
/* it */ { IDS_FREEZER_CARTS_IT, "" },  /* fuzzy */
/* ko */ { IDS_FREEZER_CARTS_KO, "" },  /* fuzzy */
/* nl */ { IDS_FREEZER_CARTS_NL, "Freezer Cartridges" },
/* pl */ { IDS_FREEZER_CARTS_PL, "" },  /* fuzzy */
/* ru */ { IDS_FREEZER_CARTS_RU, "" },  /* fuzzy */
/* sv */ { IDS_FREEZER_CARTS_SV, "" },  /* fuzzy */
/* tr */ { IDS_FREEZER_CARTS_TR, "" },  /* fuzzy */

/* en */ { IDS_UTIL_CARTS,    "Utility Cartridges" },
/* da */ { IDS_UTIL_CARTS_DA, "" },  /* fuzzy */
/* de */ { IDS_UTIL_CARTS_DE, "Utility Erweiterungsmodule" },
/* es */ { IDS_UTIL_CARTS_ES, "" },  /* fuzzy */
/* fr */ { IDS_UTIL_CARTS_FR, "" },  /* fuzzy */
/* hu */ { IDS_UTIL_CARTS_HU, "" },  /* fuzzy */
/* it */ { IDS_UTIL_CARTS_IT, "" },  /* fuzzy */
/* ko */ { IDS_UTIL_CARTS_KO, "" },  /* fuzzy */
/* nl */ { IDS_UTIL_CARTS_NL, "Gebruiksprogramma Cartridges" },
/* pl */ { IDS_UTIL_CARTS_PL, "" },  /* fuzzy */
/* ru */ { IDS_UTIL_CARTS_RU, "" },  /* fuzzy */
/* sv */ { IDS_UTIL_CARTS_SV, "" },  /* fuzzy */
/* tr */ { IDS_UTIL_CARTS_TR, "" },  /* fuzzy */

/* en */ { IDS_GAME_CARTS,    "Game Cartridges" },
/* da */ { IDS_GAME_CARTS_DA, "" },  /* fuzzy */
/* de */ { IDS_GAME_CARTS_DE, "Spiel Erweiterungsmodule" },
/* es */ { IDS_GAME_CARTS_ES, "" },  /* fuzzy */
/* fr */ { IDS_GAME_CARTS_FR, "" },  /* fuzzy */
/* hu */ { IDS_GAME_CARTS_HU, "" },  /* fuzzy */
/* it */ { IDS_GAME_CARTS_IT, "" },  /* fuzzy */
/* ko */ { IDS_GAME_CARTS_KO, "" },  /* fuzzy */
/* nl */ { IDS_GAME_CARTS_NL, "Spel Cartridges" },
/* pl */ { IDS_GAME_CARTS_PL, "" },  /* fuzzy */
/* ru */ { IDS_GAME_CARTS_RU, "" },  /* fuzzy */
/* sv */ { IDS_GAME_CARTS_SV, "" },  /* fuzzy */
/* tr */ { IDS_GAME_CARTS_TR, "" },  /* fuzzy */

/* en */ { IDS_RAMEX_CARTS,    "RAM Expansion Cartridges" },
/* da */ { IDS_RAMEX_CARTS_DA, "" },  /* fuzzy */
/* de */ { IDS_RAMEX_CARTS_DE, "RAM Erweiterungsmodule" },
/* es */ { IDS_RAMEX_CARTS_ES, "" },  /* fuzzy */
/* fr */ { IDS_RAMEX_CARTS_FR, "" },  /* fuzzy */
/* hu */ { IDS_RAMEX_CARTS_HU, "" },  /* fuzzy */
/* it */ { IDS_RAMEX_CARTS_IT, "" },  /* fuzzy */
/* ko */ { IDS_RAMEX_CARTS_KO, "" },  /* fuzzy */
/* nl */ { IDS_RAMEX_CARTS_NL, "RAM Uitbreiding Cartridges" },
/* pl */ { IDS_RAMEX_CARTS_PL, "" },  /* fuzzy */
/* ru */ { IDS_RAMEX_CARTS_RU, "" },  /* fuzzy */
/* sv */ { IDS_RAMEX_CARTS_SV, "" },  /* fuzzy */
/* tr */ { IDS_RAMEX_CARTS_TR, "" },  /* fuzzy */

/* en */ { IDS_SYMBOLIC,    "Symbolic" },
/* da */ { IDS_SYMBOLIC_DA, "Symbolsk" },
/* de */ { IDS_SYMBOLIC_DE, "Symbolisch" },
/* es */ { IDS_SYMBOLIC_ES, "Simbólico" },
/* fr */ { IDS_SYMBOLIC_FR, "Symbolique" },
/* hu */ { IDS_SYMBOLIC_HU, "" },  /* fuzzy */
/* it */ { IDS_SYMBOLIC_IT, "Simbolica" },
/* ko */ { IDS_SYMBOLIC_KO, "»óÂ¡" },
/* nl */ { IDS_SYMBOLIC_NL, "Symbolisch" },
/* pl */ { IDS_SYMBOLIC_PL, "Symboliczne" },
/* ru */ { IDS_SYMBOLIC_RU, "Symbolic" },
/* sv */ { IDS_SYMBOLIC_SV, "Symbolisk" },
/* tr */ { IDS_SYMBOLIC_TR, "Sembolik" },

/* en */ { IDS_POSITIONAL,    "Positional" },
/* da */ { IDS_POSITIONAL_DA, "Positionsbestemt" },
/* de */ { IDS_POSITIONAL_DE, "Positional" },
/* es */ { IDS_POSITIONAL_ES, "Posicional" },
/* fr */ { IDS_POSITIONAL_FR, "Positionnel" },
/* hu */ { IDS_POSITIONAL_HU, "" },  /* fuzzy */
/* it */ { IDS_POSITIONAL_IT, "Posizionale" },
/* ko */ { IDS_POSITIONAL_KO, "À§Ä¡»óÀÇ" },
/* nl */ { IDS_POSITIONAL_NL, "Positioneel" },
/* pl */ { IDS_POSITIONAL_PL, "Pozycjonalne" },
/* ru */ { IDS_POSITIONAL_RU, "Positional" },
/* sv */ { IDS_POSITIONAL_SV, "Positionsriktig" },
/* tr */ { IDS_POSITIONAL_TR, "Konumsal" },

/* en */ { IDS_SYMBOLIC_USER,    "Symbolic (user)" },
/* da */ { IDS_SYMBOLIC_USER_DA, "" },  /* fuzzy */
/* de */ { IDS_SYMBOLIC_USER_DE, "Symbolisch (Benutzer)" },
/* es */ { IDS_SYMBOLIC_USER_ES, "" },  /* fuzzy */
/* fr */ { IDS_SYMBOLIC_USER_FR, "" },  /* fuzzy */
/* hu */ { IDS_SYMBOLIC_USER_HU, "" },  /* fuzzy */
/* it */ { IDS_SYMBOLIC_USER_IT, "" },  /* fuzzy */
/* ko */ { IDS_SYMBOLIC_USER_KO, "" },  /* fuzzy */
/* nl */ { IDS_SYMBOLIC_USER_NL, "Symbolisch (gebruiker)" },
/* pl */ { IDS_SYMBOLIC_USER_PL, "" },  /* fuzzy */
/* ru */ { IDS_SYMBOLIC_USER_RU, "" },  /* fuzzy */
/* sv */ { IDS_SYMBOLIC_USER_SV, "" },  /* fuzzy */
/* tr */ { IDS_SYMBOLIC_USER_TR, "" },  /* fuzzy */

/* en */ { IDS_POSITIONAL_USER,    "Positional (user)" },
/* da */ { IDS_POSITIONAL_USER_DA, "" },  /* fuzzy */
/* de */ { IDS_POSITIONAL_USER_DE, "Positionszuordnung (Benutzer)" },
/* es */ { IDS_POSITIONAL_USER_ES, "" },  /* fuzzy */
/* fr */ { IDS_POSITIONAL_USER_FR, "" },  /* fuzzy */
/* hu */ { IDS_POSITIONAL_USER_HU, "" },  /* fuzzy */
/* it */ { IDS_POSITIONAL_USER_IT, "" },  /* fuzzy */
/* ko */ { IDS_POSITIONAL_USER_KO, "" },  /* fuzzy */
/* nl */ { IDS_POSITIONAL_USER_NL, "Positioneel (gebruiker)" },
/* pl */ { IDS_POSITIONAL_USER_PL, "" },  /* fuzzy */
/* ru */ { IDS_POSITIONAL_USER_RU, "" },  /* fuzzy */
/* sv */ { IDS_POSITIONAL_USER_SV, "" },  /* fuzzy */
/* tr */ { IDS_POSITIONAL_USER_TR, "" },  /* fuzzy */

/* en */ { IDS_SYMBOLIC_KEYMAP_FILENAME_SELECT,    "Select file for user symbolic keymap" },
/* da */ { IDS_SYMBOLIC_KEYMAP_FILENAME_SELECT_DA, "" },  /* fuzzy */
/* de */ { IDS_SYMBOLIC_KEYMAP_FILENAME_SELECT_DE, "Symbolische Keymap Datei auswählen" },
/* es */ { IDS_SYMBOLIC_KEYMAP_FILENAME_SELECT_ES, "" },  /* fuzzy */
/* fr */ { IDS_SYMBOLIC_KEYMAP_FILENAME_SELECT_FR, "" },  /* fuzzy */
/* hu */ { IDS_SYMBOLIC_KEYMAP_FILENAME_SELECT_HU, "" },  /* fuzzy */
/* it */ { IDS_SYMBOLIC_KEYMAP_FILENAME_SELECT_IT, "" },  /* fuzzy */
/* ko */ { IDS_SYMBOLIC_KEYMAP_FILENAME_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDS_SYMBOLIC_KEYMAP_FILENAME_SELECT_NL, "Selecteer bestand voor symbolische toetstoewijzing" },
/* pl */ { IDS_SYMBOLIC_KEYMAP_FILENAME_SELECT_PL, "" },  /* fuzzy */
/* ru */ { IDS_SYMBOLIC_KEYMAP_FILENAME_SELECT_RU, "" },  /* fuzzy */
/* sv */ { IDS_SYMBOLIC_KEYMAP_FILENAME_SELECT_SV, "" },  /* fuzzy */
/* tr */ { IDS_SYMBOLIC_KEYMAP_FILENAME_SELECT_TR, "" },  /* fuzzy */

/* en */ { IDS_POSITIONAL_KEYMAP_FILENAME_SELECT,    "Select file for user positional keymap" },
/* da */ { IDS_POSITIONAL_KEYMAP_FILENAME_SELECT_DA, "" },  /* fuzzy */
/* de */ { IDS_POSITIONAL_KEYMAP_FILENAME_SELECT_DE, "Positions Keymap Datei auswählen" },
/* es */ { IDS_POSITIONAL_KEYMAP_FILENAME_SELECT_ES, "" },  /* fuzzy */
/* fr */ { IDS_POSITIONAL_KEYMAP_FILENAME_SELECT_FR, "" },  /* fuzzy */
/* hu */ { IDS_POSITIONAL_KEYMAP_FILENAME_SELECT_HU, "" },  /* fuzzy */
/* it */ { IDS_POSITIONAL_KEYMAP_FILENAME_SELECT_IT, "" },  /* fuzzy */
/* ko */ { IDS_POSITIONAL_KEYMAP_FILENAME_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDS_POSITIONAL_KEYMAP_FILENAME_SELECT_NL, "Selecteer bestand voor positionele toetstoewijzing" },
/* pl */ { IDS_POSITIONAL_KEYMAP_FILENAME_SELECT_PL, "" },  /* fuzzy */
/* ru */ { IDS_POSITIONAL_KEYMAP_FILENAME_SELECT_RU, "" },  /* fuzzy */
/* sv */ { IDS_POSITIONAL_KEYMAP_FILENAME_SELECT_SV, "" },  /* fuzzy */
/* tr */ { IDS_POSITIONAL_KEYMAP_FILENAME_SELECT_TR, "" },  /* fuzzy */

/* en */ { IDS_ACTIVE_KEYMAP,    "Active keymap" },
/* da */ { IDS_ACTIVE_KEYMAP_DA, "" },  /* fuzzy */
/* de */ { IDS_ACTIVE_KEYMAP_DE, "Aktive Keymap" },
/* es */ { IDS_ACTIVE_KEYMAP_ES, "" },  /* fuzzy */
/* fr */ { IDS_ACTIVE_KEYMAP_FR, "" },  /* fuzzy */
/* hu */ { IDS_ACTIVE_KEYMAP_HU, "" },  /* fuzzy */
/* it */ { IDS_ACTIVE_KEYMAP_IT, "" },  /* fuzzy */
/* ko */ { IDS_ACTIVE_KEYMAP_KO, "" },  /* fuzzy */
/* nl */ { IDS_ACTIVE_KEYMAP_NL, "Actieve toetstoewijzing" },
/* pl */ { IDS_ACTIVE_KEYMAP_PL, "" },  /* fuzzy */
/* ru */ { IDS_ACTIVE_KEYMAP_RU, "" },  /* fuzzy */
/* sv */ { IDS_ACTIVE_KEYMAP_SV, "" },  /* fuzzy */
/* tr */ { IDS_ACTIVE_KEYMAP_TR, "" },  /* fuzzy */

/* en */ { IDS_KEYBOARD_MAPPING,    "Keyboard mapping" },
/* da */ { IDS_KEYBOARD_MAPPING_DA, "" },  /* fuzzy */
/* de */ { IDS_KEYBOARD_MAPPING_DE, "Tastatur Mapping" },
/* es */ { IDS_KEYBOARD_MAPPING_ES, "" },  /* fuzzy */
/* fr */ { IDS_KEYBOARD_MAPPING_FR, "" },  /* fuzzy */
/* hu */ { IDS_KEYBOARD_MAPPING_HU, "" },  /* fuzzy */
/* it */ { IDS_KEYBOARD_MAPPING_IT, "" },  /* fuzzy */
/* ko */ { IDS_KEYBOARD_MAPPING_KO, "" },  /* fuzzy */
/* nl */ { IDS_KEYBOARD_MAPPING_NL, "Toetsenbord toewijzing" },
/* pl */ { IDS_KEYBOARD_MAPPING_PL, "" },  /* fuzzy */
/* ru */ { IDS_KEYBOARD_MAPPING_RU, "" },  /* fuzzy */
/* sv */ { IDS_KEYBOARD_MAPPING_SV, "" },  /* fuzzy */
/* tr */ { IDS_KEYBOARD_MAPPING_TR, "" },  /* fuzzy */

/* en */ { IDS_USER_SYMBOLIC_KEYMAP_FILENAME,    "User symbolic keymap file" },
/* da */ { IDS_USER_SYMBOLIC_KEYMAP_FILENAME_DA, "" },  /* fuzzy */
/* de */ { IDS_USER_SYMBOLIC_KEYMAP_FILENAME_DE, "Symbolische Keymap Datei" },
/* es */ { IDS_USER_SYMBOLIC_KEYMAP_FILENAME_ES, "" },  /* fuzzy */
/* fr */ { IDS_USER_SYMBOLIC_KEYMAP_FILENAME_FR, "" },  /* fuzzy */
/* hu */ { IDS_USER_SYMBOLIC_KEYMAP_FILENAME_HU, "" },  /* fuzzy */
/* it */ { IDS_USER_SYMBOLIC_KEYMAP_FILENAME_IT, "" },  /* fuzzy */
/* ko */ { IDS_USER_SYMBOLIC_KEYMAP_FILENAME_KO, "" },  /* fuzzy */
/* nl */ { IDS_USER_SYMBOLIC_KEYMAP_FILENAME_NL, "Symbolisch gebruikers toetstoewijzingsbestand" },
/* pl */ { IDS_USER_SYMBOLIC_KEYMAP_FILENAME_PL, "" },  /* fuzzy */
/* ru */ { IDS_USER_SYMBOLIC_KEYMAP_FILENAME_RU, "" },  /* fuzzy */
/* sv */ { IDS_USER_SYMBOLIC_KEYMAP_FILENAME_SV, "" },  /* fuzzy */
/* tr */ { IDS_USER_SYMBOLIC_KEYMAP_FILENAME_TR, "" },  /* fuzzy */

/* en */ { IDS_USER_POSITIONAL_KEYMAP_FILENAME,    "User positional keymap file" },
/* da */ { IDS_USER_POSITIONAL_KEYMAP_FILENAME_DA, "" },  /* fuzzy */
/* de */ { IDS_USER_POSITIONAL_KEYMAP_FILENAME_DE, "Positions Keymap Datei" },
/* es */ { IDS_USER_POSITIONAL_KEYMAP_FILENAME_ES, "" },  /* fuzzy */
/* fr */ { IDS_USER_POSITIONAL_KEYMAP_FILENAME_FR, "" },  /* fuzzy */
/* hu */ { IDS_USER_POSITIONAL_KEYMAP_FILENAME_HU, "" },  /* fuzzy */
/* it */ { IDS_USER_POSITIONAL_KEYMAP_FILENAME_IT, "" },  /* fuzzy */
/* ko */ { IDS_USER_POSITIONAL_KEYMAP_FILENAME_KO, "" },  /* fuzzy */
/* nl */ { IDS_USER_POSITIONAL_KEYMAP_FILENAME_NL, "Positioneel gebruikers toetstoewijzingsbestand" },
/* pl */ { IDS_USER_POSITIONAL_KEYMAP_FILENAME_PL, "" },  /* fuzzy */
/* ru */ { IDS_USER_POSITIONAL_KEYMAP_FILENAME_RU, "" },  /* fuzzy */
/* sv */ { IDS_USER_POSITIONAL_KEYMAP_FILENAME_SV, "" },  /* fuzzy */
/* tr */ { IDS_USER_POSITIONAL_KEYMAP_FILENAME_TR, "" },  /* fuzzy */

/* en */ { IDS_KEYMAP_SETTINGS,    "Keymap settings" },
/* da */ { IDS_KEYMAP_SETTINGS_DA, "" },  /* fuzzy */
/* de */ { IDS_KEYMAP_SETTINGS_DE, "Keymap Einstellungen" },
/* es */ { IDS_KEYMAP_SETTINGS_ES, "" },  /* fuzzy */
/* fr */ { IDS_KEYMAP_SETTINGS_FR, "" },  /* fuzzy */
/* hu */ { IDS_KEYMAP_SETTINGS_HU, "" },  /* fuzzy */
/* it */ { IDS_KEYMAP_SETTINGS_IT, "" },  /* fuzzy */
/* ko */ { IDS_KEYMAP_SETTINGS_KO, "" },  /* fuzzy */
/* nl */ { IDS_KEYMAP_SETTINGS_NL, "Toetstoewijzing instellingen" },
/* pl */ { IDS_KEYMAP_SETTINGS_PL, "" },  /* fuzzy */
/* ru */ { IDS_KEYMAP_SETTINGS_RU, "" },  /* fuzzy */
/* sv */ { IDS_KEYMAP_SETTINGS_SV, "" },  /* fuzzy */
/* tr */ { IDS_KEYMAP_SETTINGS_TR, "" },  /* fuzzy */

/* en */ { IDS_PSID_SELECT,    "Select SID file to load" },
/* da */ { IDS_PSID_SELECT_DA, "Vælg SID-fil" },
/* de */ { IDS_PSID_SELECT_DE, "SID Datei wählen" },
/* es */ { IDS_PSID_SELECT_ES, "Seleccionar fichero SID para cargar" },
/* fr */ { IDS_PSID_SELECT_FR, "Sélectionner le fichier SID à charger" },
/* hu */ { IDS_PSID_SELECT_HU, "" },  /* fuzzy */
/* it */ { IDS_PSID_SELECT_IT, "Seleziona un file SID da caricare" },
/* ko */ { IDS_PSID_SELECT_KO, "" },  /* fuzzy */
/* nl */ { IDS_PSID_SELECT_NL, "Selecteer SID bestand voor laden" },
/* pl */ { IDS_PSID_SELECT_PL, "Wybierz plik SID do wczytania" },
/* ru */ { IDS_PSID_SELECT_RU, "" },  /* fuzzy */
/* sv */ { IDS_PSID_SELECT_SV, "Välj SID-fil att läsa in" },
/* tr */ { IDS_PSID_SELECT_TR, "Yüklenecek SID dosyasýný seçin" },

};
