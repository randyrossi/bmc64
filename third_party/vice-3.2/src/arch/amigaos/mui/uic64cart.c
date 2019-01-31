/*
 * uic64cart.c
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

#include "vice.h"
#ifdef AMIGA_M68K
#define _INLINE_MUIMASTER_H
#endif
#include "mui.h"

#include "cartridge.h"
#include "uic64cart.h"
#include "intl.h"
#include "translate.h"

static video_canvas_t *c64cart_canvas;

#define CART_LOAD(name, cartname, cartid) \
static ULONG name( struct Hook *hook, Object *obj, APTR arg ) \
{                                                             \
    char *fname = NULL;                                       \
    fname = BrowseFile(cartname, "#?", c64cart_canvas);       \
    if (fname != NULL) {                                      \
        return cartridge_attach_image(cartid, fname);         \
    }                                                         \
    return 0;                                                 \
}

#ifdef AMIGA_MORPHOS
#define CART_HOOK(name, cart_load) \
static const struct Hook name = { { NULL, NULL }, (VOID *)HookEntry, (VOID *)cart_load, NULL }
#else
#define CART_HOOK(name, cart_load) \
static const struct Hook name = { { NULL, NULL }, (VOID *)cart_load, NULL, NULL }
#endif

#define CART_METHOD(button, hook) \
DoMethod(button, MUIM_Notify, MUIA_Pressed, FALSE, app, 2, MUIM_CallHook, &hook)

CART_LOAD(Generic8KB, CARTRIDGE_NAME_GENERIC_8KB, CARTRIDGE_GENERIC_8KB)
CART_LOAD(Generic16KB, CARTRIDGE_NAME_GENERIC_16KB, CARTRIDGE_GENERIC_16KB)
CART_LOAD(Ultimax, CARTRIDGE_NAME_ULTIMAX, CARTRIDGE_ULTIMAX)

static APTR build_gui_generic(void)
{
    APTR app, ui, ok, cancel;
    APTR generic8k_button, generic16k_button, ultimax_button;

    CART_HOOK(Generic8KHook, Generic8KB);
    CART_HOOK(Generic16KHook, Generic16KB);
    CART_HOOK(UltimaxHook, Ultimax);

    app = mui_get_app();

    ui = GroupObject,
           BUTTON(generic8k_button, CARTRIDGE_NAME_GENERIC_8KB)
           BUTTON(generic16k_button, CARTRIDGE_NAME_GENERIC_16KB)
           BUTTON(ultimax_button, CARTRIDGE_NAME_ULTIMAX)
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);

        CART_METHOD(generic8k_button, Generic8KHook);
        CART_METHOD(generic16k_button, Generic16KHook);
        CART_METHOD(ultimax_button, UltimaxHook);
    }

    return ui;
}

CART_LOAD(AR1, CARTRIDGE_NAME_ACTION_REPLAY, CARTRIDGE_ACTION_REPLAY)
CART_LOAD(AR2, CARTRIDGE_NAME_ACTION_REPLAY2, CARTRIDGE_ACTION_REPLAY2)
CART_LOAD(AR3, CARTRIDGE_NAME_ACTION_REPLAY3, CARTRIDGE_ACTION_REPLAY3)
CART_LOAD(AR4, CARTRIDGE_NAME_ACTION_REPLAY4, CARTRIDGE_ACTION_REPLAY4)
CART_LOAD(AP, CARTRIDGE_NAME_ATOMIC_POWER, CARTRIDGE_ATOMIC_POWER)
CART_LOAD(CAP, CARTRIDGE_NAME_CAPTURE, CARTRIDGE_CAPTURE)
CART_LOAD(DSM, CARTRIDGE_NAME_DIASHOW_MAKER, CARTRIDGE_DIASHOW_MAKER)
CART_LOAD(EXP, CARTRIDGE_NAME_EXPERT, CARTRIDGE_EXPERT)
CART_LOAD(FC1, CARTRIDGE_NAME_FINAL_I, CARTRIDGE_FINAL_I)
CART_LOAD(FC3, CARTRIDGE_NAME_FINAL_III, CARTRIDGE_FINAL_III)
CART_LOAD(FCP, CARTRIDGE_NAME_FINAL_PLUS, CARTRIDGE_FINAL_PLUS)
CART_LOAD(F64, CARTRIDGE_NAME_FORMEL64, CARTRIDGE_FORMEL64)
CART_LOAD(FF, CARTRIDGE_NAME_FREEZE_FRAME, CARTRIDGE_FREEZE_FRAME)
CART_LOAD(FM, CARTRIDGE_NAME_FREEZE_MACHINE, CARTRIDGE_FREEZE_MACHINE)
CART_LOAD(FUN, CARTRIDGE_NAME_FUNPLAY, CARTRIDGE_FUNPLAY)
CART_LOAD(GK, CARTRIDGE_NAME_GAME_KILLER, CARTRIDGE_GAME_KILLER)
CART_LOAD(KCS, CARTRIDGE_NAME_KCS_POWER, CARTRIDGE_KCS_POWER)
CART_LOAD(MF, CARTRIDGE_NAME_MAGIC_FORMEL, CARTRIDGE_MAGIC_FORMEL)
CART_LOAD(MMCR, CARTRIDGE_NAME_MMC_REPLAY, CARTRIDGE_MMC_REPLAY)
CART_LOAD(RR, CARTRIDGE_NAME_RETRO_REPLAY, CARTRIDGE_RETRO_REPLAY)
CART_LOAD(SS64, CARTRIDGE_NAME_SNAPSHOT64, CARTRIDGE_SNAPSHOT64)
CART_LOAD(SS4, CARTRIDGE_NAME_SUPER_SNAPSHOT, CARTRIDGE_SUPER_SNAPSHOT)
CART_LOAD(SS5, CARTRIDGE_NAME_SUPER_SNAPSHOT_V5, CARTRIDGE_SUPER_SNAPSHOT_V5)

static APTR build_gui_freezer(void)
{
    APTR app, ui, ok, cancel;
    APTR ar1_button, ar2_button, ar3_button, ar4_button, ap_button, cap_button, dsm_button;
    APTR exp_button, fc1_button, fc3_button, fcp_button, f64_button, ff_button, fm_button;
    APTR fun_button, gk_button, kcs_button, mf_button, mmcr_button, rr_button, ss64_button;
    APTR ss4_button, ss5_button;

    CART_HOOK(AR1Hook, AR1);
    CART_HOOK(AR2Hook, AR2);
    CART_HOOK(AR3Hook, AR3);
    CART_HOOK(AR4Hook, AR4);
    CART_HOOK(APHook, AP);
    CART_HOOK(CAPHook, CAP);
    CART_HOOK(DSMHook, DSM);
    CART_HOOK(EXPHook, EXP);
    CART_HOOK(FC1Hook, FC1);
    CART_HOOK(FC3Hook, FC3);
    CART_HOOK(FCPHook, FCP);
    CART_HOOK(F64Hook, F64);
    CART_HOOK(FFHook, FF);
    CART_HOOK(FMHook, FM);
    CART_HOOK(FUNHook, FUN);
    CART_HOOK(GKHook, GK);
    CART_HOOK(KCSHook, KCS);
    CART_HOOK(MFHook, MF);
    CART_HOOK(MMCRHook, MMCR);
    CART_HOOK(RRHook, RR);
    CART_HOOK(SS64Hook, SS64);
    CART_HOOK(SS4Hook, SS4);
    CART_HOOK(SS5Hook, SS5);

    app = mui_get_app();

    ui = GroupObject,
           BUTTON(ar1_button, CARTRIDGE_NAME_ACTION_REPLAY)
           BUTTON(ar2_button, CARTRIDGE_NAME_ACTION_REPLAY2)
           BUTTON(ar3_button, CARTRIDGE_NAME_ACTION_REPLAY3)
           BUTTON(ar4_button, CARTRIDGE_NAME_ACTION_REPLAY4)
           BUTTON(ap_button, CARTRIDGE_NAME_ATOMIC_POWER)
           BUTTON(cap_button, CARTRIDGE_NAME_CAPTURE)
           BUTTON(dsm_button, CARTRIDGE_NAME_DIASHOW_MAKER)
           BUTTON(exp_button, CARTRIDGE_NAME_EXPERT)
           BUTTON(fc1_button, CARTRIDGE_NAME_FINAL_I)
           BUTTON(fc3_button, CARTRIDGE_NAME_FINAL_III)
           BUTTON(fcp_button, CARTRIDGE_NAME_FINAL_PLUS)
           BUTTON(f64_button, CARTRIDGE_NAME_FORMEL64)
           BUTTON(ff_button, CARTRIDGE_NAME_FREEZE_FRAME)
           BUTTON(fm_button, CARTRIDGE_NAME_FREEZE_MACHINE)
           BUTTON(fun_button, CARTRIDGE_NAME_FUNPLAY)
           BUTTON(gk_button, CARTRIDGE_NAME_GAME_KILLER)
           BUTTON(kcs_button, CARTRIDGE_NAME_KCS_POWER)
           BUTTON(mf_button, CARTRIDGE_NAME_MAGIC_FORMEL)
           BUTTON(mmcr_button, CARTRIDGE_NAME_MMC_REPLAY)
           BUTTON(rr_button, CARTRIDGE_NAME_RETRO_REPLAY)
           BUTTON(ss64_button, CARTRIDGE_NAME_SNAPSHOT64)
           BUTTON(ss4_button, CARTRIDGE_NAME_SUPER_SNAPSHOT)
           BUTTON(ss5_button, CARTRIDGE_NAME_SUPER_SNAPSHOT_V5)
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);

        CART_METHOD(ar1_button, AR1Hook);
        CART_METHOD(ar2_button, AR2Hook);
        CART_METHOD(ar3_button, AR3Hook);
        CART_METHOD(ar4_button, AR4Hook);
        CART_METHOD(ap_button, APHook);
        CART_METHOD(cap_button, CAPHook);
        CART_METHOD(dsm_button, DSMHook);
        CART_METHOD(exp_button, EXPHook);
        CART_METHOD(fc1_button, FC1Hook);
        CART_METHOD(fc3_button, FC3Hook);
        CART_METHOD(fcp_button, FCPHook);
        CART_METHOD(f64_button, F64Hook);
        CART_METHOD(ff_button, FFHook);
        CART_METHOD(fm_button, FMHook);
        CART_METHOD(fun_button, FUNHook);
        CART_METHOD(gk_button, GKHook);
        CART_METHOD(kcs_button, KCSHook);
        CART_METHOD(mf_button, MFHook);
        CART_METHOD(mmcr_button, MMCRHook);
        CART_METHOD(rr_button, RRHook);
        CART_METHOD(ss64_button, SS64Hook);
        CART_METHOD(ss4_button, SS4Hook);
        CART_METHOD(ss5_button, SS5Hook);
    }

    return ui;
}

#ifdef HAVE_RAWNET
CART_LOAD(RRN3, CARTRIDGE_NAME_RRNETMK3, CARTRIDGE_RRNETMK3)
#endif
CART_LOAD(WS, CARTRIDGE_NAME_WARPSPEED, CARTRIDGE_WARPSPEED)
CART_LOAD(WL, CARTRIDGE_NAME_WESTERMANN, CARTRIDGE_WESTERMANN)
CART_LOAD(C80, CARTRIDGE_NAME_COMAL80, CARTRIDGE_COMAL80)
CART_LOAD(DEP256, CARTRIDGE_NAME_DELA_EP256, CARTRIDGE_DELA_EP256)
CART_LOAD(DEP64, CARTRIDGE_NAME_DELA_EP64, CARTRIDGE_DELA_EP64)
CART_LOAD(DEP7x8, CARTRIDGE_NAME_DELA_EP7x8, CARTRIDGE_DELA_EP7x8)
CART_LOAD(EC, CARTRIDGE_NAME_EASYCALC, CARTRIDGE_EASYCALC)
CART_LOAD(EF, CARTRIDGE_NAME_EASYFLASH, CARTRIDGE_EASYFLASH)
CART_LOAD(EPYX, CARTRIDGE_NAME_EPYX_FASTLOAD, CARTRIDGE_EPYX_FASTLOAD)
CART_LOAD(EXOS, CARTRIDGE_NAME_EXOS, CARTRIDGE_EXOS)
CART_LOAD(IDE64, CARTRIDGE_NAME_IDE64, CARTRIDGE_IDE64)
CART_LOAD(I488, CARTRIDGE_NAME_IEEE488, CARTRIDGE_IEEE488)
CART_LOAD(KS, CARTRIDGE_NAME_KINGSOFT, CARTRIDGE_KINGSOFT)
CART_LOAD(M5, CARTRIDGE_NAME_MACH5, CARTRIDGE_MACH5)
CART_LOAD(MD, CARTRIDGE_NAME_MAGIC_DESK, CARTRIDGE_MAGIC_DESK)
CART_LOAD(MV, CARTRIDGE_NAME_MAGIC_VOICE, CARTRIDGE_MAGIC_VOICE)
CART_LOAD(MA, CARTRIDGE_NAME_MIKRO_ASSEMBLER, CARTRIDGE_MIKRO_ASSEMBLER)
CART_LOAD(MMC64, CARTRIDGE_NAME_MMC64, CARTRIDGE_MMC64)
CART_LOAD(P64, CARTRIDGE_NAME_P64, CARTRIDGE_P64)
CART_LOAD(PF, CARTRIDGE_NAME_PAGEFOX, CARTRIDGE_PAGEFOX)
CART_LOAD(REX, CARTRIDGE_NAME_REX, CARTRIDGE_REX)
CART_LOAD(REP256, CARTRIDGE_NAME_REX_EP256, CARTRIDGE_REX_EP256)
CART_LOAD(ROSS, CARTRIDGE_NAME_ROSS, CARTRIDGE_ROSS)
CART_LOAD(SIMON, CARTRIDGE_NAME_SIMONS_BASIC, CARTRIDGE_SIMONS_BASIC)
CART_LOAD(SD, CARTRIDGE_NAME_STARDOS, CARTRIDGE_STARDOS)
CART_LOAD(SB, CARTRIDGE_NAME_STRUCTURED_BASIC, CARTRIDGE_STRUCTURED_BASIC)
CART_LOAD(SE5, CARTRIDGE_NAME_SUPER_EXPLODE_V5, CARTRIDGE_SUPER_EXPLODE_V5)

static APTR build_gui_util(void)
{
    APTR app, ui, ok, cancel;
#ifdef HAVE_RAWNET
    APTR rrn3_button;
#endif
    APTR ws_button, wl_button, c80_button, dep256_button, dep64_button, dep7x8_button;
    APTR ec_button, ef_button, epyx_button, exos_button, ide64_button, i488_button;
    APTR ks_button, m5_button, md_button, mv_button, ma_button, mmc64_button;
    APTR p64_button, pf_button, rex_button, rep256_button, ross_button, simon_button;
    APTR sb_button, sd_button, se5_button;

#ifdef HAVE_RAWNET
    CART_HOOK(RRN3Hook, RRN3);
#endif
    CART_HOOK(WSHook, WS);
    CART_HOOK(WLHook, WL);
    CART_HOOK(C80Hook, C80);
    CART_HOOK(DEP256Hook, DEP256);
    CART_HOOK(DEP64Hook, DEP64);
    CART_HOOK(DEP7x8Hook, DEP7x8);
    CART_HOOK(ECHook, EC);
    CART_HOOK(EFHook, EF);
    CART_HOOK(EPYXHook, EPYX);
    CART_HOOK(EXOSHook, EXOS);
    CART_HOOK(IDE64Hook, IDE64);
    CART_HOOK(I488Hook, I488);
    CART_HOOK(KSHook, KS);
    CART_HOOK(M5Hook, M5);
    CART_HOOK(MDHook, MD);
    CART_HOOK(MVHook, MV);
    CART_HOOK(MAHook, MA);
    CART_HOOK(MMC64Hook, MMC64);
    CART_HOOK(P64Hook, P64);
    CART_HOOK(PFHook, PF);
    CART_HOOK(REXHook, REX);
    CART_HOOK(REP256Hook, REP256);
    CART_HOOK(ROSSHook, ROSS);
    CART_HOOK(SIMONHook, SIMON);
    CART_HOOK(SDHook, SD);
    CART_HOOK(SBHook, SB);
    CART_HOOK(SE5Hook, SE5);

    app = mui_get_app();

    ui = GroupObject,
           BUTTON(c80_button, CARTRIDGE_NAME_COMAL80)
           BUTTON(dep256_button, CARTRIDGE_NAME_DELA_EP256)
           BUTTON(dep64_button, CARTRIDGE_NAME_DELA_EP64)
           BUTTON(dep7x8_button, CARTRIDGE_NAME_DELA_EP7x8)
           BUTTON(ec_button, CARTRIDGE_NAME_EASYCALC)
           BUTTON(ef_button, CARTRIDGE_NAME_EASYFLASH)
           BUTTON(epyx_button, CARTRIDGE_NAME_EPYX_FASTLOAD)
           BUTTON(exos_button, CARTRIDGE_NAME_EXOS)
           BUTTON(ide64_button, CARTRIDGE_NAME_IDE64)
           BUTTON(i488_button, CARTRIDGE_NAME_IEEE488)
           BUTTON(ks_button, CARTRIDGE_NAME_KINGSOFT)
           BUTTON(m5_button, CARTRIDGE_NAME_MACH5)
           BUTTON(md_button, CARTRIDGE_NAME_MAGIC_DESK)
           BUTTON(mv_button, CARTRIDGE_NAME_MAGIC_VOICE)
           BUTTON(ma_button, CARTRIDGE_NAME_MIKRO_ASSEMBLER)
           BUTTON(mmc64_button, CARTRIDGE_NAME_MMC64)
           BUTTON(p64_button, CARTRIDGE_NAME_P64)
           BUTTON(pf_button, CARTRIDGE_NAME_PAGEFOX)
           BUTTON(rex_button, CARTRIDGE_NAME_REX)
           BUTTON(rep256_button, CARTRIDGE_NAME_REX_EP256)
#ifdef HAVE_RAWNET
           BUTTON(rrn3_button, CARTRIDGE_NAME_RRNETMK3)
#endif
           BUTTON(ross_button, CARTRIDGE_NAME_ROSS)
           BUTTON(simon_button, CARTRIDGE_NAME_SIMONS_BASIC)
           BUTTON(sd_button, CARTRIDGE_NAME_STARDOS)
           BUTTON(sb_button, CARTRIDGE_NAME_STRUCTURED_BASIC)
           BUTTON(se5_button, CARTRIDGE_NAME_SUPER_EXPLODE_V5)
           BUTTON(ws_button, CARTRIDGE_NAME_WARPSPEED)
           BUTTON(wl_button, CARTRIDGE_NAME_WESTERMANN)
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);

#ifdef HAVE_RAWNET
        CART_METHOD(rrn3_button, RRN3Hook);
#endif
        CART_METHOD(ws_button, WSHook);
        CART_METHOD(wl_button, WLHook);
        CART_METHOD(c80_button, C80Hook);
        CART_METHOD(dep256_button, DEP256Hook);
        CART_METHOD(dep64_button, DEP64Hook);
        CART_METHOD(dep7x8_button, DEP7x8Hook);
        CART_METHOD(ec_button, ECHook);
        CART_METHOD(ef_button, EFHook);
        CART_METHOD(epyx_button, EPYXHook);
        CART_METHOD(exos_button, EXOSHook);
        CART_METHOD(ide64_button, IDE64Hook);
        CART_METHOD(i488_button, I488Hook);
        CART_METHOD(ks_button, KSHook);
        CART_METHOD(m5_button, M5Hook);
        CART_METHOD(md_button, MDHook);
        CART_METHOD(mv_button, MVHook);
        CART_METHOD(ma_button, MAHook);
        CART_METHOD(mmc64_button, MMC64Hook);
        CART_METHOD(p64_button, P64Hook);
        CART_METHOD(pf_button, PFHook);
        CART_METHOD(rex_button, REXHook);
        CART_METHOD(rep256_button, REP256Hook);
        CART_METHOD(ross_button, ROSSHook);
        CART_METHOD(simon_button, SIMONHook);
        CART_METHOD(sd_button, SDHook);
        CART_METHOD(sb_button, SBHook);
        CART_METHOD(se5_button, SE5Hook);
    }

    return ui;
}

CART_LOAD(DIN, CARTRIDGE_NAME_DINAMIC, CARTRIDGE_DINAMIC)
CART_LOAD(GMOD2, CARTRIDGE_NAME_GMOD2, CARTRIDGE_GMOD2)
CART_LOAD(GS, CARTRIDGE_NAME_GS, CARTRIDGE_GS)
CART_LOAD(OCEAN, CARTRIDGE_NAME_OCEAN, CARTRIDGE_OCEAN)
CART_LOAD(RGCD, CARTRIDGE_NAME_RGCD, CARTRIDGE_RGCD)
CART_LOAD(SR128, CARTRIDGE_NAME_SILVERROCK_128, CARTRIDGE_SILVERROCK_128)
CART_LOAD(SG, CARTRIDGE_NAME_SUPER_GAMES, CARTRIDGE_SUPER_GAMES)
CART_LOAD(ZAX, CARTRIDGE_NAME_ZAXXON, CARTRIDGE_ZAXXON)

static APTR build_gui_game(void)
{
    APTR app, ui, ok, cancel;
    APTR din_button, gmod2_button, gs_button, ocean_button, rgcd_button, sg_button;
    APTR sr128_button, zax_button;

    CART_HOOK(DINHook, DIN);
    CART_HOOK(GMOD2Hook, GMOD2);
    CART_HOOK(GSHook, GS);
    CART_HOOK(OCEANHook, OCEAN);
    CART_HOOK(RGCDHook, RGCD);
    CART_HOOK(SR128Hook, SR128);
    CART_HOOK(SGHook, SG);
    CART_HOOK(ZAXHook, ZAX);

    app = mui_get_app();

    ui = GroupObject,
           BUTTON(din_button, CARTRIDGE_NAME_DINAMIC)
           BUTTON(gmod2_button, CARTRIDGE_NAME_GMOD2)
           BUTTON(gs_button, CARTRIDGE_NAME_GS)
           BUTTON(ocean_button, CARTRIDGE_NAME_OCEAN)
           BUTTON(rgcd_button, CARTRIDGE_NAME_RGCD)
           BUTTON(sr128_button, CARTRIDGE_NAME_SILVERROCK_128)
           BUTTON(sg_button, CARTRIDGE_NAME_SUPER_GAMES)
           BUTTON(zax_button, CARTRIDGE_NAME_ZAXXON)
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);

        CART_METHOD(din_button, DINHook);
        CART_METHOD(gmod2_button, GMOD2Hook);
        CART_METHOD(gs_button, GSHook);
        CART_METHOD(ocean_button, OCEANHook);
        CART_METHOD(rgcd_button, RGCDHook);
        CART_METHOD(sr128_button, SR128Hook);
        CART_METHOD(sg_button, SGHook);
        CART_METHOD(zax_button, ZAXHook);
    }

    return ui;
}

CART_LOAD(DQBB, CARTRIDGE_NAME_DQBB, CARTRIDGE_DQBB)
CART_LOAD(GEO, CARTRIDGE_NAME_GEORAM, CARTRIDGE_GEORAM)
CART_LOAD(IP, CARTRIDGE_NAME_ISEPIC, CARTRIDGE_ISEPIC)
CART_LOAD(RC, CARTRIDGE_NAME_RAMCART, CARTRIDGE_RAMCART)
CART_LOAD(REU, CARTRIDGE_NAME_REU, CARTRIDGE_REU)

static APTR build_gui_ramex(void)
{
    APTR app, ui, ok, cancel;
    APTR dqbb_button, geo_button, ip_button, rc_button, reu_button;

    CART_HOOK(DQBBHook, DQBB);
    CART_HOOK(GEOHook, GEO);
    CART_HOOK(IPHook, IP);
    CART_HOOK(RCHook, RC);
    CART_HOOK(REUHook, REU);

    app = mui_get_app();

    ui = GroupObject,
           BUTTON(dqbb_button, CARTRIDGE_NAME_DQBB)
           BUTTON(geo_button, CARTRIDGE_NAME_GEORAM)
           BUTTON(ip_button, CARTRIDGE_NAME_ISEPIC)
           BUTTON(rc_button, CARTRIDGE_NAME_RAMCART)
           BUTTON(reu_button, CARTRIDGE_NAME_REU)
           OK_CANCEL_BUTTON
         End;

    if (ui != NULL) {
        DoMethod(cancel, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, MUIV_Application_ReturnID_Quit);

        DoMethod(ok, MUIM_Notify, MUIA_Pressed, FALSE,
                 app, 2, MUIM_Application_ReturnID, BTN_OK);

        CART_METHOD(dqbb_button, DQBBHook);
        CART_METHOD(geo_button, GEOHook);
        CART_METHOD(ip_button, IPHook);
        CART_METHOD(rc_button, RCHook);
        CART_METHOD(reu_button, REUHook);
    }

    return ui;
}

void ui_c64cart_generic_settings_dialog(video_canvas_t *canvas)
{
    APTR window;

    c64cart_canvas = canvas;

    window = mui_make_simple_window(build_gui_generic(), translate_text(IDS_GENERIC_CARTS));

    if (window != NULL) {
        mui_add_window(window);
        set(window, MUIA_Window_Open, TRUE);
        mui_run();
        set(window, MUIA_Window_Open, FALSE);
        mui_rem_window(window);
        MUI_DisposeObject(window);
    }
}

void ui_c64cart_freezer_settings_dialog(video_canvas_t *canvas)
{
    APTR window;

    c64cart_canvas = canvas;

    window = mui_make_simple_window(build_gui_freezer(), translate_text(IDS_FREEZER_CARTS));

    if (window != NULL) {
        mui_add_window(window);
        set(window, MUIA_Window_Open, TRUE);
        mui_run();
        set(window, MUIA_Window_Open, FALSE);
        mui_rem_window(window);
        MUI_DisposeObject(window);
    }
}

void ui_c64cart_util_settings_dialog(video_canvas_t *canvas)
{
    APTR window;

    c64cart_canvas = canvas;

    window = mui_make_simple_window(build_gui_util(), translate_text(IDS_UTIL_CARTS));

    if (window != NULL) {
        mui_add_window(window);
        set(window, MUIA_Window_Open, TRUE);
        mui_run();
        set(window, MUIA_Window_Open, FALSE);
        mui_rem_window(window);
        MUI_DisposeObject(window);
    }
}

void ui_c64cart_game_settings_dialog(video_canvas_t *canvas)
{
    APTR window;

    c64cart_canvas = canvas;

    window = mui_make_simple_window(build_gui_game(), translate_text(IDS_GAME_CARTS));

    if (window != NULL) {
        mui_add_window(window);
        set(window, MUIA_Window_Open, TRUE);
        mui_run();
        set(window, MUIA_Window_Open, FALSE);
        mui_rem_window(window);
        MUI_DisposeObject(window);
    }
}

void ui_c64cart_ramex_settings_dialog(video_canvas_t *canvas)
{
    APTR window;

    c64cart_canvas = canvas;

    window = mui_make_simple_window(build_gui_ramex(), translate_text(IDS_RAMEX_CARTS));

    if (window != NULL) {
        mui_add_window(window);
        set(window, MUIA_Window_Open, TRUE);
        mui_run();
        set(window, MUIA_Window_Open, FALSE);
        mui_rem_window(window);
        MUI_DisposeObject(window);
    }
}
