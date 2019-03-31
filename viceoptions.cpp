//
// viceoptions.cpp
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "viceoptions.h"
#include <circle/logger.h>
#include <circle/util.h>
#include <circle/sysconfig.h>
#include <string.h>
#include <stdlib.h>

#define INVALID_VALUE	((unsigned) -1)

ViceOptions *ViceOptions::s_pThis = 0;

ViceOptions::ViceOptions (void) :
	m_nCanvasWidth (DEFAULT_CANVAS_WIDTH),
	m_nCanvasHeight (DEFAULT_CANVAS_HEIGHT),
	m_nMachineTiming (MACHINE_TIMING_PAL),
	m_bHideConsole(true),
	m_bDemoMode(false)
{
	s_pThis = this;

	CBcmPropertyTags Tags;
	if (!Tags.GetTag (PROPTAG_GET_COMMAND_LINE,
		&m_TagCommandLine, sizeof m_TagCommandLine))
	{
		return;
	}

	if (m_TagCommandLine.Tag.nValueLength >= sizeof m_TagCommandLine.String)
	{
		return;
	}
	m_TagCommandLine.String[m_TagCommandLine.Tag.nValueLength] = '\0';
	
	m_pOptions = (char *) m_TagCommandLine.String;

	// Set the default volume we mount for fatfs
	m_disk_partition = 0; // this tells fatfs 'auto'
	strcpy(m_disk_volume, "SD");

	char *pOption;
	while ((pOption = GetToken ()) != 0)
	{
		char *pValue = GetOptionValue (pOption);

		if (strcmp (pOption, "canvas_width") == 0)
		{
			unsigned nValue;
			if (   (nValue = GetDecimal (pValue)) != INVALID_VALUE
			    && 384 <= nValue && nValue <= 1980)
			{
				m_nCanvasWidth = nValue;
			}
		}
		else if (strcmp (pOption, "canvas_height") == 0)
		{
			unsigned nValue;
			if (   (nValue = GetDecimal (pValue)) != INVALID_VALUE
			    && 272 <= nValue && nValue <= 1080)
			{
				m_nCanvasHeight = nValue;
			}
		}
		else if (strcmp (pOption, "machine_timing") == 0)
		{
			if (strcmp(pValue, "ntsc") == 0)
			{
				m_nMachineTiming = MACHINE_TIMING_NTSC;
			}
			else {
				m_nMachineTiming = MACHINE_TIMING_PAL;
			}
		}
		else if (strcmp (pOption, "hide_console") == 0)
		{
			if (strcmp(pValue, "1") == 0)
			{
				m_bHideConsole = true;
			}
			else {
				m_bHideConsole = false;
			}
		}
		else if (strcmp (pOption, "demo") == 0)
		{
			if (strcmp(pValue, "1") == 0)
			{
				m_bDemoMode = true;
			}
			else {
				m_bDemoMode = false;
			}
		}
		else if (strcmp (pOption, "disk_volume") == 0)
		{
			if (strlen(pValue) > 0) {
				strncpy(m_disk_volume, pValue, VOLUME_NAME_LEN - 1);
			}
		}
		else if (strcmp (pOption, "disk_partition") == 0)
		{
			m_disk_partition = atoi(pValue);
			if (m_disk_partition < 0) m_disk_partition = 0;
		}
	}
}

ViceOptions::~ViceOptions (void)
{
	s_pThis = 0;
}

unsigned ViceOptions::GetCanvasWidth (void) const
{
	return m_nCanvasWidth;
}

unsigned ViceOptions::GetCanvasHeight (void) const
{
	return m_nCanvasHeight;
}

unsigned ViceOptions::GetMachineTiming (void) const
{
	return m_nMachineTiming;
}

bool ViceOptions::GetHideConsole (void) const
{
	return m_bHideConsole;
}

void ViceOptions::SetHideConsole (bool value)
{
	m_bHideConsole = value;
}

bool ViceOptions::GetDemoMode (void) const
{
	return m_bDemoMode;
}

int ViceOptions::GetDiskPartition (void) const
{
        return m_disk_partition;
}

const char* ViceOptions::GetDiskVolume (void) const
{
        return m_disk_volume;
}

ViceOptions *ViceOptions::Get (void)
{
	return s_pThis;
}

char *ViceOptions::GetToken (void)
{
	while (*m_pOptions != '\0')
	{
		if (*m_pOptions != ' ')
		{
			break;
		}

		m_pOptions++;
	}

	if (*m_pOptions == '\0')
	{
		return 0;
	}

	char *pToken = m_pOptions;

	while (*m_pOptions != '\0')
	{
		if (*m_pOptions == ' ')
		{
			*m_pOptions++ = '\0';

			break;
		}

		m_pOptions++;
	}

	return pToken;
}

char *ViceOptions::GetOptionValue (char *pOption)
{
	while (*pOption != '\0')
	{
		if (*pOption == '=')
		{
			break;
		}

		pOption++;
	}

	if (*pOption == '\0')
	{
		return 0;
	}

	*pOption++ = '\0';

	return pOption;
}

unsigned ViceOptions::GetDecimal (char *pString)
{
	if (   pString == 0
	    || *pString == '\0')
	{
		return INVALID_VALUE;
	}

	unsigned nResult = 0;

	char chChar;
	while ((chChar = *pString++) != '\0')
	{
		if (!('0' <= chChar && chChar <= '9'))
		{
			return INVALID_VALUE;
		}

		unsigned nPrevResult = nResult;

		nResult = nResult * 10 + (chChar - '0');
		if (   nResult < nPrevResult
		    || nResult == INVALID_VALUE)
		{
			return INVALID_VALUE;
		}
	}

	return nResult;
}
