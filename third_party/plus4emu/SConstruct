# vim: syntax=python

from __future__ import print_function
import sys, os

win64CrossCompile = int(ARGUMENTS.get('win64', 0))
mingwCrossCompile = win64CrossCompile or int(ARGUMENTS.get('win32', 0))
linux32CrossCompile = int(ARGUMENTS.get('linux32', 0))
# on Linux, statically linked SDL version >= 1.2.10 that was not built
# without video support (--disable-video) breaks FLTK
disableSDL = int(ARGUMENTS.get('nosdl', 0))
disableLua = int(ARGUMENTS.get('nolua', 0))
enableGLShaders = int(ARGUMENTS.get('glshaders', 1))
enableDebug = int(ARGUMENTS.get('debug', 0))
buildRelease = not enableDebug and int(ARGUMENTS.get('release', 1))
# for mingwCrossCompile, use LuaJIT instead of Lua 5.3
useLuaJIT = int(ARGUMENTS.get('luajit', 0))
userFlags = ARGUMENTS.get('cflags', '')
disablePkgConfig = int(ARGUMENTS.get('nopkgconfig',
                                     int(linux32CrossCompile or \
                                         mingwCrossCompile)))
enableBuildCache = int(ARGUMENTS.get('cache', 0))

compilerFlags = ''
if buildRelease:
    if linux32CrossCompile or (mingwCrossCompile and not win64CrossCompile):
        compilerFlags = ' -march=pentium2 '
if enableDebug and not buildRelease:
    compilerFlags = ' -Wno-long-long -Wshadow -g -O0 ' + compilerFlags
    compilerFlags = ' -Wall -W -ansi -pedantic ' + compilerFlags
else:
    compilerFlags = ' -Wall -O3 ' + compilerFlags + ' -mtune=generic '
    compilerFlags = compilerFlags + ' -fno-inline-functions '
    compilerFlags = compilerFlags + ' -fomit-frame-pointer -ffast-math '

# -----------------------------------------------------------------------------

# pkgname : [ pkgconfig, [ package_names ],
#             linux_flags, mingw_flags, c_header, cxx_header, optional ]

fltkLibsLinux = '-lfltk -lfltk_images -lfltk_jpeg -lfltk_png'
fltkLibsMinGW = fltkLibsLinux + ' -lz -lcomdlg32 -lcomctl32 -lole32'
fltkLibsMinGW = fltkLibsMinGW + ' -luuid -lws2_32 -lwinmm -lgdi32'
fltkLibsLinux = fltkLibsLinux + ' -lfltk_z -lXcursor -lXinerama -lXrender'
fltkLibsLinux = fltkLibsLinux + ' -lXext -lXft -lXfixes -lX11 -lfontconfig -ldl'

packageConfigs = {
    'FLTK' : [
        'fltk-config --use-images --cflags --cxxflags --ldflags', [''],
        fltkLibsLinux, fltkLibsMinGW, '', 'FL/Fl.H', 0 ],
    'FLTK-GL' : [
        'fltk-config --use-gl --use-images --cflags --cxxflags --ldflags', [''],
        '-lfltk_gl ' + fltkLibsLinux + ' -lGL',
        '-lfltk_gl ' + fltkLibsMinGW + ' -lopengl32',
        '', 'FL/Fl_Gl_Window.H', 1 ],
    'sndfile' : [
        'pkg-config --silence-errors --cflags --libs',
        ['sndfile'],
        '-lsndfile', '-lsndfile', 'sndfile.h', '', 0 ],
    'PortAudio' : [
        'pkg-config --silence-errors --cflags --libs',
        ['portaudio-2.0', 'portaudio2', 'portaudio'],
        '-lportaudio -lasound -lm -ldl -lpthread -lrt', '-lportaudio -lwinmm',
        'portaudio.h', '', 0 ],
    'Lua' : [
        'pkg-config --silence-errors --cflags --libs',
        ['lua-5.3', 'lua53', 'lua-5.2', 'lua52', 'lua-5.1', 'lua51', 'lua'],
        '-llua', '', 'lua.h', '', 1 ],
    'SDL' : [
        'pkg-config --silence-errors --cflags --libs',
        ['sdl', 'sdl1'],
        '-lSDL', '-lSDL -lwinmm', 'SDL/SDL.h', '', 1]
}

def configurePackage(env, pkgName):
    global packageConfigs, disablePkgConfig
    global linux32CrossCompile, mingwCrossCompile, win64CrossCompile
    if not disablePkgConfig:
        for s in packageConfigs[pkgName][1]:
            if not s:
                print('Checking for package ' + pkgName + '...', end = ' ')
                # hack to work around fltk-config adding unwanted compiler flags
                savedCFlags = env['CCFLAGS']
                savedCXXFlags = env['CXXFLAGS']
            else:
                print('Checking for package ' + s + '...', end = ' ')
                s = ' ' + s
            try:
                if not env.ParseConfig(packageConfigs[pkgName][0] + s):
                    raise Exception()
                print('yes')
                if not s:
                    env['CCFLAGS'] = savedCFlags
                    env['CXXFLAGS'] = savedCXXFlags
                    if ['_FORTIFY_SOURCE', '2'] in env['CPPDEFINES']:
                        env['CPPDEFINES'].remove(['_FORTIFY_SOURCE', '2'])
                return 1
            except:
                print('no')
                continue
        pkgFound = 0
    else:
        configure = env.Configure()
        if packageConfigs[pkgName][4]:
            pkgFound = configure.CheckCHeader(packageConfigs[pkgName][4])
        else:
            pkgFound = configure.CheckCXXHeader(packageConfigs[pkgName][5])
        configure.Finish()
        if pkgFound:
            env.MergeFlags(
                packageConfigs[pkgName][2 + int(bool(mingwCrossCompile))])
    if not pkgFound:
        if not packageConfigs[pkgName][6]:
            print(' *** error configuring ' + pkgName)
            Exit(-1)
        print('WARNING: package ' + pkgName + ' not found')
        return 0
    return 1

if enableBuildCache:
    CacheDir("./.build_cache")

programNamePrefix = ""
buildingLinuxPackage = 0
if not mingwCrossCompile:
    if sys.platform[:5] == 'linux':
        try:
            instPrefix = os.environ["UB_INSTALLDIR"]
            if instPrefix:
                instPrefix += "/usr"
                buildingLinuxPackage = 1
        except:
            pass
    if not buildingLinuxPackage:
        instPrefix = os.environ["HOME"]
        instShareDir = instPrefix + "/.local/share"
    else:
        instShareDir = instPrefix + "/share"
    instBinDir = instPrefix + "/bin"
    instDataDir = instShareDir + "/plus4emu"
    instPixmapDir = instShareDir + "/pixmaps"
    instDesktopDir = instShareDir + "/applications"
    instROMDir = instDataDir + "/roms"
    instConfDir = instDataDir + "/config"
    instDiskDir = instDataDir + "/disk"
    programNamePrefix = "p4"
    plus4emuLibEnvironment = Environment(ENV = { 'PATH' : os.environ['PATH'],
                                                 'HOME' : os.environ['HOME'] })
else:
    plus4emuLibEnvironment = Environment(ENV = { 'PATH' : os.environ['PATH'] })

if linux32CrossCompile:
    compilerFlags = ' -m32 ' + compilerFlags
plus4emuLibEnvironment.Append(CCFLAGS = Split(compilerFlags))
plus4emuLibEnvironment.Append(CPPPATH = ['.', './src'])
if userFlags:
    plus4emuLibEnvironment.MergeFlags(userFlags)
if not mingwCrossCompile:
    plus4emuLibEnvironment.Append(CPPPATH = ['/usr/local/include'])
if sys.platform[:6] == 'darwin':
    plus4emuLibEnvironment.Append(CPPPATH = ['/usr/X11R6/include'])
if not linux32CrossCompile:
    linkFlags = ' -L. '
else:
    linkFlags = ' -m32 -L. -L/usr/X11R6/lib '
plus4emuLibEnvironment.Append(LINKFLAGS = Split(linkFlags))
if mingwCrossCompile:
    wordSize = ['32', '64'][int(bool(win64CrossCompile))]
    mingwPrefix = 'C:/mingw' + wordSize
    plus4emuLibEnvironment.Prepend(CCFLAGS = ['-m' + wordSize])
    plus4emuLibEnvironment.Prepend(LINKFLAGS = ['-m' + wordSize])
    plus4emuLibEnvironment.Append(CPPPATH = [mingwPrefix + '/include'])
    if sys.platform[:3] == 'win':
        toolNamePrefix = ''
    elif win64CrossCompile:
        toolNamePrefix = 'x86_64-w64-mingw32-'
    else:
        toolNamePrefix = 'i686-w64-mingw32-'
    plus4emuLibEnvironment['AR'] = toolNamePrefix + 'ar'
    plus4emuLibEnvironment['CC'] = toolNamePrefix + 'gcc'
    plus4emuLibEnvironment['CPP'] = toolNamePrefix + 'cpp'
    plus4emuLibEnvironment['CXX'] = toolNamePrefix + 'g++'
    plus4emuLibEnvironment['LINK'] = toolNamePrefix + 'g++'
    plus4emuLibEnvironment['RANLIB'] = toolNamePrefix + 'ranlib'
    plus4emuLibEnvironment['PROGSUFFIX'] = '.exe'
    packageConfigs['Lua'][3] = '-llua' + ['53', '51'][int(bool(useLuaJIT))]
    plus4emuLibEnvironment.Append(
        CPPPATH = [mingwPrefix
                   + '/include/lua' + ['5.3', '5.1'][int(bool(useLuaJIT))]])
    plus4emuLibEnvironment.Append(LIBS = ['user32', 'kernel32'])
    plus4emuLibEnvironment.Prepend(CCFLAGS = ['-mthreads'])
    plus4emuLibEnvironment.Prepend(LINKFLAGS = ['-mthreads'])
elif sys.platform[:5] == 'linux':
    plus4emuLibEnvironment.Prepend(LINKFLAGS = ['-Wl,--as-needed'])
if buildRelease:
    plus4emuLibEnvironment.Append(LINKFLAGS = ['-s'])

plus4emuGUIEnvironment = plus4emuLibEnvironment.Clone()
if mingwCrossCompile:
    plus4emuGUIEnvironment.Prepend(LINKFLAGS = ['-mwindows'])
else:
    plus4emuGUIEnvironment.Append(LIBS = ['pthread'])
configurePackage(plus4emuGUIEnvironment, 'FLTK')
makecfgEnvironment = plus4emuGUIEnvironment.Clone()
configurePackage(plus4emuGUIEnvironment, 'sndfile')
haveLua = 0
haveSDL = 0
if not disableLua:
    haveLua = configurePackage(plus4emuGUIEnvironment, 'Lua')
if not disableSDL:
    haveSDL = configurePackage(plus4emuGUIEnvironment, 'SDL')
configurePackage(plus4emuGUIEnvironment, 'PortAudio')

plus4emuGLGUIEnvironment = plus4emuGUIEnvironment.Clone()
disableOpenGL = 1
if configurePackage(plus4emuGLGUIEnvironment, 'FLTK-GL'):
    configure = plus4emuGLGUIEnvironment.Configure()
    if configure.CheckCHeader('GL/gl.h'):
        disableOpenGL = 0
        if enableGLShaders:
            if not configure.CheckType('PFNGLCOMPILESHADERPROC',
                                       '#include <GL/gl.h>\n'
                                       + '#include <GL/glext.h>'):
                print('WARNING: disabling GL shader support')
                enableGLShaders = 0
    configure.Finish()
if sys.platform[:5] == 'linux' and not mingwCrossCompile:
    plus4emuGUIEnvironment.Append(LIBS = ['X11'])
    plus4emuGLGUIEnvironment.Append(LIBS = ['GL', 'X11'])
if disableOpenGL:
    print('WARNING: OpenGL is not found, only software video will be supported')
    enableGLShaders = 0
    plus4emuGLGUIEnvironment = plus4emuGUIEnvironment.Clone()
    plus4emuGLGUIEnvironment.Append(CCFLAGS = ['-DDISABLE_OPENGL_DISPLAY'])

plus4emuLibEnvironment['CPPPATH'] = plus4emuGLGUIEnvironment['CPPPATH']

imageLibTestProgram = '''
    #include <FL/Fl.H>
    #include <FL/Fl_Shared_Image.H>
    int main()
    {
      Fl_Shared_Image *tmp = Fl_Shared_Image::get("foo");
      tmp->release();
      return 0;
    }
'''

def imageLibTest(env1, env2, env3):
    # remove unneeded and possibly conflicting libraries added by fltk-config
    for libName in ['jpeg', 'png']:
        if libName in env1['LIBS']:
            tmpEnv = env1.Clone()
            tmpEnv['LIBS'].remove(libName)
            tmpConfig = tmpEnv.Configure()
            if tmpConfig.TryLink(imageLibTestProgram, '.cpp'):
                env1['LIBS'].remove(libName)
                env2['LIBS'].remove(libName)
                env3['LIBS'].remove(libName)
            tmpConfig.Finish()

imageLibTest(makecfgEnvironment,
             plus4emuGUIEnvironment, plus4emuGLGUIEnvironment)

configure = plus4emuLibEnvironment.Configure()
if configure.CheckType('PaStreamCallbackTimeInfo', '#include <portaudio.h>'):
    havePortAudioV19 = 1
else:
    havePortAudioV19 = 0
    print('WARNING: using old v18 PortAudio interface')
fltkVersion13 = 0
if configure.CheckCXXHeader('FL/Fl_Cairo.H'):
    fltkVersion13 = 1
else:
    plus4emuLibEnvironment.Append(CPPPATH = ['./Fl_Native_File_Chooser'])
    plus4emuGUIEnvironment.Append(CPPPATH = ['./Fl_Native_File_Chooser'])
    plus4emuGLGUIEnvironment.Append(CPPPATH = ['./Fl_Native_File_Chooser'])
    makecfgEnvironment.Append(CPPPATH = ['./Fl_Native_File_Chooser'])
if configure.CheckCHeader('stdint.h'):
    plus4emuLibEnvironment.Append(CCFLAGS = ['-DHAVE_STDINT_H'])
if sys.platform[:5] == 'linux' and not mingwCrossCompile:
    if configure.CheckCHeader('linux/fd.h'):
        plus4emuLibEnvironment.Append(CCFLAGS = ['-DHAVE_LINUX_FD_H'])
configure.Finish()

if not havePortAudioV19:
    plus4emuLibEnvironment.Append(CCFLAGS = ['-DUSING_OLD_PORTAUDIO_API'])
if haveSDL:
    plus4emuLibEnvironment.Append(CCFLAGS = ['-DHAVE_SDL_H'])
if haveLua:
    plus4emuLibEnvironment.Append(CCFLAGS = ['-DHAVE_LUA_H'])
if enableGLShaders:
    plus4emuLibEnvironment.Append(CCFLAGS = ['-DENABLE_GL_SHADERS'])
if not fltkVersion13:
    plus4emuLibEnvironment.Append(CCFLAGS = ['-DFLTK1'])

plus4emuGUIEnvironment.MergeFlags(plus4emuLibEnvironment['CCFLAGS'])
plus4emuGLGUIEnvironment.MergeFlags(plus4emuLibEnvironment['CCFLAGS'])
makecfgEnvironment.MergeFlags(plus4emuLibEnvironment['CCFLAGS'])

def fluidCompile(flNames):
    cppNames = []
    for flName in flNames:
        if flName.endswith('.fl'):
            cppName = flName[:-3] + '_fl.cpp'
            hppName = flName[:-3] + '_fl.hpp'
            plus4emuLibEnvironment.Command(
                [cppName, hppName], flName,
                'fluid -c -o %s -h %s $SOURCES' % (cppName, hppName))
            cppNames += [cppName]
    return cppNames

plus4emuLibSources = Split('''
    src/cpu.cpp
    src/cpuoptbl.cpp
    src/memory.cpp
    src/render.cpp
    src/ted_api.cpp
    src/ted_init.cpp
    src/ted_main.cpp
    src/ted_read.cpp
    src/ted_snd.cpp
    src/ted_write.cpp
    src/vc1541.cpp
    src/vc1551.cpp
    src/vc1581.cpp
    src/via6522.cpp
    src/plus4vm.cpp
    src/vm.cpp
    src/acia6551.cpp
    src/bplist.cpp
    src/cia8520.cpp
    src/d64image.cpp
    src/disasm.cpp
    src/display.cpp
    src/dotconf.c
    src/fileio.cpp
    src/iecdrive.cpp
    src/mps801.cpp
    src/riot6532.cpp
    src/snd_conv.cpp
    src/soundio.cpp
    src/system.cpp
    src/tape.cpp
    src/vc1526.cpp
    src/videorec.cpp
    src/wd177x.cpp
''')

plus4emuLibSources2 = []
if not fltkVersion13:
    plus4emuLibSources2 += ['Fl_Native_File_Chooser/Fl_Native_File_Chooser.cxx']
plus4emuLibSources2 += Split('''
    src/cfg_db.cpp
    src/charconv.cpp
    src/compress.cpp
    src/comprlib.cpp
    src/decompm2.cpp
    src/emucfg.cpp
    src/fldisp.cpp
    src/gldisp.cpp
    src/guicolor.cpp
    src/joystick.cpp
    src/pngwrite.cpp
    src/script.cpp
    src/sndio_pa.cpp
    src/vmthread.cpp
''')
if disableOpenGL:
    plus4emuLibSources2.remove('src/gldisp.cpp')
plus4emuLib = plus4emuLibEnvironment.StaticLibrary(
    'plus4emu', plus4emuLibSources + plus4emuLibSources2)

# -----------------------------------------------------------------------------

residLibEnvironment = plus4emuLibEnvironment.Clone()
residLibEnvironment.Append(CPPPATH = ['./resid'])

residLibSources = Split('''
    resid/dac.cpp
    resid/envelope.cpp
    resid/extfilt.cpp
    resid/filter.cpp
    resid/pot.cpp
    resid/sid.cpp
    resid/version.cpp
    resid/voice.cpp
    resid/wave.cpp
''')

residLib = residLibEnvironment.StaticLibrary('resid', residLibSources)

# -----------------------------------------------------------------------------

def fixDefFile(env, target, source):
    f = open('plus4lib/plus4emu.def', 'rb')
    s = []
    for tmp in f:
        s = s + [tmp]
    f.close()
    if s.__len__() > 0:
        if s[0][:7] != 'LIBRARY':
            f = open('plus4lib/plus4emu.def', 'wb')
            f.write('LIBRARY plus4emu.dll\r\n')
            for tmp in s:
                f.write(tmp)
            f.close()

if mingwCrossCompile or sys.platform[:5] == 'linux':
    plus4emuDLLEnvironment = plus4emuLibEnvironment.Clone()
    plus4emuDLLEnvironment.Append(CPPPATH = ['./plus4lib'])
    plus4emuDLLEnvironment.Append(LIBS = ['sndfile'])
    if not mingwCrossCompile:
        plus4emuDLLEnvironment.Append(CCFLAGS = ['-fvisibility=hidden'])
        plus4emuDLLEnvironment.Append(LIBS = ['pthread'])
        plus4emuDLL = plus4emuDLLEnvironment.SharedLibrary(
            'plus4lib/plus4emu',
            plus4emuLibSources + residLibSources + ['plus4lib/plus4api.cpp'])
    else:
        plus4emuDLLEnvironment.Prepend(
            LINKFLAGS = ['-mwindows', '-Wl,--output-def,plus4lib/plus4emu.def'])
        plus4emuDLLEnvironment.Prepend(LIBS = ['plus4emu', 'resid'])
        plus4emuDLLEnvironment['SHLIBPREFIX'] = ''
        plus4emuDLLEnvironment['SHLIBSUFFIX'] = ''
        plus4emuDLL = plus4emuDLLEnvironment.SharedLibrary(
            'plus4lib/plus4emu.dll', ['plus4lib/plus4api.cpp'])
        # add LIBRARY line to the .def file if needed
        plus4emuDLLEnvironment.Command('__fix_plus4emu_def_file__', plus4emuDLL,
                                       fixDefFile)
    Depends(plus4emuDLL, plus4emuLib)
    Depends(plus4emuDLL, residLib)

# -----------------------------------------------------------------------------

plus4emuEnvironment = plus4emuGLGUIEnvironment.Clone()
plus4emuEnvironment.Append(CPPPATH = ['./gui'])
plus4emuEnvironment.Prepend(LIBS = ['plus4emu', 'resid'])

plus4emuSources = ['gui/gui.cpp']
plus4emuSources += fluidCompile(['gui/gui.fl', 'gui/disk_cfg.fl',
                                 'gui/disp_cfg.fl', 'gui/kbd_cfg.fl',
                                 'gui/snd_cfg.fl', 'gui/vm_cfg.fl',
                                 'gui/debug.fl', 'gui/printer.fl',
                                 'gui/about.fl'])
plus4emuSources += ['gui/debugger.cpp', 'gui/monitor.cpp', 'gui/main.cpp']
if mingwCrossCompile:
    plus4emuResourceObject = plus4emuEnvironment.Command(
        'resource/resource.o',
        ['resource/plus4emu.rc', 'resource/Cbm4.ico', 'resource/1551.ico',
         'resource/Plus4Mon4.ico', 'resource/Plus4i.ico',
         'resource/CbmFile.ico'],
        toolNamePrefix + 'windres -v --use-temp-file '
        + '--preprocessor="gcc -E -xc -DRC_INVOKED" '
        + '-o $TARGET resource/plus4emu.rc')
    plus4emuSources += [plus4emuResourceObject]
plus4emu = plus4emuEnvironment.Program('plus4emu', plus4emuSources)
Depends(plus4emu, plus4emuLib)
Depends(plus4emu, residLib)

if sys.platform[:6] == 'darwin':
    Command('plus4emu.app/Contents/MacOS/plus4emu', 'plus4emu',
            'mkdir -p plus4emu.app/Contents/MacOS ; cp -pf $SOURCES $TARGET')

# -----------------------------------------------------------------------------

tapconvEnvironment = plus4emuGUIEnvironment.Clone()
if mingwCrossCompile:
    tapconvEnvironment['LINKFLAGS'].remove('-mwindows')
tapconvEnvironment.Prepend(LIBS = ['plus4emu'])
tapconv = tapconvEnvironment.Program(programNamePrefix + 'tapconv',
                                     ['util/tapconv.cpp'])
Depends(tapconv, plus4emuLib)

# -----------------------------------------------------------------------------

makecfgEnvironment.Append(CPPPATH = ['./installer'])
makecfgEnvironment.Prepend(LIBS = ['plus4emu'])

makecfg = makecfgEnvironment.Program(programNamePrefix + 'makecfg',
    ['installer/makecfg.cpp'] + fluidCompile(['installer/mkcfg.fl']))
Depends(makecfg, plus4emuLib)

if sys.platform[:6] == 'darwin':
    Command('plus4emu.app/Contents/MacOS/' + programNamePrefix + 'makecfg',
            programNamePrefix + 'makecfg',
            'mkdir -p plus4emu.app/Contents/MacOS ; cp -pf $SOURCES $TARGET')

# -----------------------------------------------------------------------------

compressLibEnvironment = plus4emuLibEnvironment.Clone()
compressLibEnvironment.Append(CPPPATH = ['./util/compress'])
compressLibEnvironment.Prepend(LIBS = [plus4emuLib])
compressLib = compressLibEnvironment.StaticLibrary(
    'compress',
    Split('''
        util/compress/compress0.cpp
        util/compress/compress1.cpp
        util/compress/compress2.cpp
        util/compress/compress3.cpp
        util/compress/compress5.cpp
        util/compress/compress.cpp
        util/compress/decompress0.cpp
        util/compress/decompress1.cpp
        util/compress/decompress2.cpp
        util/compress/decompress3.cpp
        util/compress/decompress5.cpp
    '''))

# -----------------------------------------------------------------------------

p4fliconvLibEnvironment = plus4emuGLGUIEnvironment.Clone()
if mingwCrossCompile:
    p4fliconvLibEnvironment['LINKFLAGS'].remove('-mwindows')
p4fliconvLibEnvironment.Append(CPPPATH = ['./util/compress',
                                          './util/p4fliconv'])

p4fliconvLibSources = ['util/p4fliconv/dither.cpp',
                       'util/p4fliconv/flicfg.cpp',
                       'util/p4fliconv/hiresfli.cpp',
                       'util/p4fliconv/hiresnofli.cpp',
                       'util/p4fliconv/hrbmifli.cpp',
                       'util/p4fliconv/imageconv.cpp',
                       'util/p4fliconv/imgwrite.cpp',
                       'util/p4fliconv/interlace7.cpp',
                       'util/p4fliconv/mcbmifli.cpp',
                       'util/p4fliconv/mcchar.cpp',
                       'util/p4fliconv/mcfli.cpp',
                       'util/p4fliconv/mcifli.cpp',
                       'util/p4fliconv/mcnofli.cpp',
                       'util/p4fliconv/p4fliconv.cpp',
                       'util/p4fliconv/p4slib.cpp',
                       'util/p4fliconv/prgdata.cpp']
if not disableOpenGL:
    p4fliconvLibSources += ['util/p4fliconv/flidisp.cpp',
                            fluidCompile(['util/p4fliconv/p4fliconv.fl'])]
p4fliconvLib = p4fliconvLibEnvironment.StaticLibrary('p4fliconv',
                                                     p4fliconvLibSources)

p4fliconvEnvironment = p4fliconvLibEnvironment.Clone()
p4fliconvEnvironment.Prepend(LIBS = ['p4fliconv', 'compress', 'plus4emu'])

p4fliconv = p4fliconvEnvironment.Program(
    'p4fliconv', ['util/p4fliconv/main.cpp'])
Depends(p4fliconv, p4fliconvLib)
Depends(p4fliconv, compressLib)
Depends(p4fliconv, plus4emuLib)

if mingwCrossCompile:
    p4fliconvGUIEnvironment = p4fliconvEnvironment.Clone()
    p4fliconvGUIEnvironment.Prepend(LINKFLAGS = ['-mwindows'])
    p4fliconvGUIMain = p4fliconvGUIEnvironment.Object(
        'guimain', 'util/p4fliconv/main.cpp')
    p4fliconvGUI = p4fliconvGUIEnvironment.Program(
        'p4fliconv_gui', [p4fliconvGUIMain])
    Depends(p4fliconvGUI, p4fliconvLib)
    Depends(p4fliconvGUI, compressLib)
    Depends(p4fliconvGUI, plus4emuLib)

if sys.platform[:6] == 'darwin':
    Command('plus4emu.app/Contents/MacOS/p4fliconv', 'p4fliconv',
            'mkdir -p plus4emu.app/Contents/MacOS ; cp -pf $SOURCES $TARGET')

p4sconvEnvironment = p4fliconvLibEnvironment.Clone()
p4sconvEnvironment.Prepend(LIBS = ['p4fliconv', 'compress', 'plus4emu'])
p4sconv = p4sconvEnvironment.Program('p4sconv', ['util/p4fliconv/p4sconv.cpp'])
Depends(p4sconv, p4fliconvLib)
Depends(p4sconv, compressLib)
Depends(p4sconv, plus4emuLib)

# -----------------------------------------------------------------------------

compressEnvironment = compressLibEnvironment.Clone()
compressEnvironment.Prepend(LIBS = ['compress'])
compress = compressEnvironment.Program(programNamePrefix + 'compress',
                                       ['util/compress/main.cpp'])
Depends(compress, compressLib)
Depends(compress, plus4emuLib)

# -----------------------------------------------------------------------------

if not mingwCrossCompile:
    if buildingLinuxPackage:
        makecfgEnvironment.InstallAs([instBinDir + "/plus4emu.bin",
                                      instBinDir + "/plus4emu"],
                                     [plus4emu, "installer/plus4emu"])
    else:
        makecfgEnvironment.Install(instBinDir, plus4emu)
    makecfgEnvironment.Install(instBinDir,
                               [tapconv, makecfg, p4fliconv, p4sconv, compress])
    makecfgEnvironment.Install(instPixmapDir, ["resource/Cbm4.png"])
    makecfgEnvironment.Install(instDesktopDir, ["resource/plus4emu.desktop"])
    if not buildingLinuxPackage:
        confFileList = [instConfDir + '/P4_Keyboard_HU.cfg',
                        instConfDir + '/P4_Keyboard_US.cfg',
                        instConfDir + '/shaders/pal.glsl',
                        instConfDir + '/shaders/ntsc.glsl']
        confFiles = 0
        f = open("./installer/makecfg.cpp")
        for l in f:
          if not confFiles:
            confFiles = "machineConfigFileNames" in l
          elif "};" in l:
            confFiles = None
            break
          elif '"' in l:
            confFileList += [instConfDir + '/'
                             + l[l.find('"') + 1:l.rfind('"')]]
        f.close()
        f = None
        makecfgEnvironment.Command(
            [confFileList], [makecfg],
            ['./' + programNamePrefix + 'makecfg -f "' + instDataDir + '"'])
    makecfgEnvironment.Install(instROMDir,
                               ["roms/1526_07c.rom", "roms/3plus1.rom",
                                "roms/dos15412.rom", "roms/dos1541.rom",
                                "roms/dos1551.rom", "roms/dos1581.rom",
                                "roms/mps801.rom", "roms/p4_basic.rom",
                                "roms/p4fileio.rom", "roms/p4kernal.rom",
                                "roms/p4_ntsc.rom"])
    makecfgEnvironment.Install(instConfDir,
                               ["config/clearkbd.cfg", "config/p4_keys.cfg",
                                "config/p4keyshu.cfg"])
    makecfgEnvironment.Install(instDiskDir, ["disk/disk.zip"])
    makecfgEnvironment.Alias("install",
                             [instBinDir, instPixmapDir, instDesktopDir,
                              instDataDir, instROMDir, instConfDir,
                              instDiskDir])

