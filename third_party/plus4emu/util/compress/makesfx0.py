#!/usr/bin/python

import string
import re
import os

def assembleFile(infileName, outfileName):
    tmpName = '__ca65_tmp__' + str(os.getpid()) + '__.o'
    try:
        os.remove(tmpName)
    except:
        pass
    os.spawnvp(os.P_WAIT, 'ca65', ['ca65', '--target', 'none',
                                   infileName, '-o', tmpName])
    os.spawnvp(os.P_WAIT, 'ld65', ['ld65', '--target', 'none',
                                   tmpName, '-o', outfileName])
    os.remove(tmpName)

def assembleDecompressor(decompMode):
    f = open('decompress0_sfx.s', 'r')
    outputText = ''
    c16Mode = int((decompMode & 1) != 0)
    noCRCCheck = int((decompMode & 2) != 0)
    noReadBuffer = int((decompMode & 3) == 2)
    noBorderEffect = int((decompMode & 1) != 0)
    noColorMemoryClear = int((decompMode & 4) != 0)
    noROMEnableRestore = int((decompMode & 8) != 0)
    noCLI = int((decompMode & 16) != 0)
    runBasicProgram = int((decompMode & 32) != 0)
    sfxStartAddress = 4109
    if not (c16Mode or noReadBuffer):
        sfxStartAddress = 4117
    for s in f:
        t = re.sub('define SFX_C16_MODE  *0',
                   'define SFX_C16_MODE ' + str(c16Mode), s)
        t = re.sub('define NO_CRC_CHECK  *0',
                   'define NO_CRC_CHECK ' + str(noCRCCheck), t)
        t = re.sub('define NO_READ_BUFFER  *0',
                   'define NO_READ_BUFFER ' + str(noReadBuffer), t)
        t = re.sub('define NO_BORDER_EFFECT  *0',
                   'define NO_BORDER_EFFECT ' + str(noBorderEffect), t)
        t = re.sub('define NO_COLOR_MEMORY_CLEAR  *0',
                   'define NO_COLOR_MEMORY_CLEAR ' + str(noColorMemoryClear), t)
        t = re.sub('define NO_ROM_ENABLE_RESTORE  *0',
                   'define NO_ROM_ENABLE_RESTORE ' + str(noROMEnableRestore), t)
        t = re.sub('define SFX_NO_CLI  *0',
                   'define SFX_NO_CLI ' + str(noCLI), t)
        t = re.sub('define SFX_RUN_BASIC_PROGRAM  *0',
                   'define SFX_RUN_BASIC_PROGRAM ' + str(runBasicProgram), t)
        t = re.sub('sfxStartAddress = .*',
                   'sfxStartAddress = ' + str(sfxStartAddress), t)
        outputText = outputText + t
    f.close()
    asmFileName = '__ca65_tmp__' + str(os.getpid()) + '__.s'
    prgFileName = '__' + str(os.getpid()) + '__' + str(decompMode) + '__.prg'
    try:
        os.remove(asmFileName)
    except:
        pass
    try:
        os.remove(prgFileName)
    except:
        pass
    f = open(asmFileName, 'w')
    f.write(outputText)
    f.flush()
    f.close()
    assembleFile(asmFileName, prgFileName)
    os.remove(asmFileName)

outputData = []
outputDataPos = 8192
for i in range(0, 256):
    outputData = outputData + [0]
    outputDataPos = outputDataPos + 1

for i in range(0, 64):
    assembleDecompressor(i)
    prgFileName = '__' + str(os.getpid()) + '__' + str(i) + '__.prg'
    f = open(prgFileName, 'rb')
    fileData = []
    while 1:
        c = f.read(1)
        if len(c) != 1:
            break
        fileData = fileData + [ord(c)]
    f.close()
    os.remove(prgFileName)
    outputData[(i * 4) + 0] = outputDataPos & 0xFF
    outputData[(i * 4) + 1] = (outputDataPos & 0xFF00) / 256
    outputData[(i * 4) + 2] = len(fileData) & 0xFF
    outputData[(i * 4) + 3] = (len(fileData) & 0xFF00) / 256
    for i in range(0, len(fileData)):
        outputData = outputData + [fileData[i]]
        outputDataPos = outputDataPos + 1

tmpFileName = '__' + str(os.getpid()) + '__.prg'
try:
    os.remove(tmpFileName)
except:
    pass
f = open(tmpFileName, 'wb')
f.write(chr(0x00))
f.write(chr(0x20))
for i in range(0, len(outputData)):
    f.write(chr(outputData[i]))
f.flush()
f.close()
try:
    os.remove('sfxdecomp0.prg')
except:
    pass
os.spawnvp(os.P_WAIT,
           '../../p4compress',
           ['../../p4compress', '-m0', '-raw', '8192', '-9',
            tmpFileName, 'sfxdecomp0.prg'])
os.remove(tmpFileName)

f = open('sfxdecomp0.prg', 'rb')
fileData = []
while 1:
    c = f.read(1)
    if len(c) != 1:
        break
    fileData = fileData + [ord(c)]
f.close()
os.remove('sfxdecomp0.prg')

f = open('sfxcode0.cpp', 'w')
f.write('\nconst unsigned char Decompressor_M0::sfxModuleLibrary['
        + str(len(fileData)) + '] = {\n')
for i in range(0, len(fileData)):
    f.write(('%s 0x%02X' % ([' ', ','][int((i % 12) != 0)], fileData[i]))
            + ['', ',\n', '\n', '\n'][int((i % 12) == 11)
                                      + (int((i + 1) >= len(fileData)) * 2)])
f.write('};\n\n')
f.flush()
f.close()

