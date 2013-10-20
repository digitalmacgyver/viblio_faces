#include <Utils.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef N_WINDOWS
	#include <tchar.h>
	#include <Windows.h>
#else
	#include <sys/param.h>
	#include <limits.h>
	#include <dlfcn.h>
	#include <libgen.h>
	#include <dirent.h>
	#include <errno.h>
	#include <ctype.h>
	#include <string.h>
	#define _tcscmp strcmp
	#define _tcscpy strcpy
	#define _tcscat strcat
	#define _tcslen strlen
	#define _tcsdup strdup
	#define _fgetts fgets
	#define _fputts fputs
	#define _tfopen fopen
	#define _sntprintf snprintf
	#define _tcsftime strftime
#endif

#ifndef N_PRODUCT_HAS_NO_LICENSES

#include <NLicensing.h>

#define N_LICENSE_MANAGER_SERVER_ADDRESS N_T("/local")
#define N_LICENSE_MANAGER_SERVER_POST N_T("5000")

#endif

#include "Utils.h"
#include <NCore.h>

void OnStart(const NChar * szTitle, const NChar * szDescription, const NChar * szVersion, const NChar * szCopyright, int argc, NChar * * argv)
{
	int i;
	NResult result;

	_tprintf(N_T("%s tutorial\n"), szTitle);
	_tprintf(N_T("description: %s\n"), szDescription);
	_tprintf(N_T("version: %s\n"), szVersion);
	_tprintf(N_T("copyright: %s\n\n"), szCopyright);

	if(argc > 1)
	{
		_tprintf(N_T("arguments:\n"));
		for(i = 1; i < argc; i++)
		{
			_tprintf(N_T("\t%s\n"), argv[i]);
		}
		_tprintf(N_T("\n"));
	}

	result = NCoreOnStart();
	if (NFailed(result))
	{
		PrintError(result);
	}
}

void OnExit()
{
	NCoreOnExitEx(NFalse);
}

NResult ObtainLicense(const NChar ** arszComponentsList, NInt componentsCount)
{
#ifndef N_PRODUCT_HAS_NO_LICENSES
	NResult result;
	NBool available = NFalse;
	NInt i;

	for (i = 0; i < componentsCount; i++)
	{
		if (_tcscmp(arszComponentsList[i], N_T("")) == 0)
		{
			continue;
		}

		result = NLicenseObtainComponents(N_LICENSE_MANAGER_SERVER_ADDRESS, N_LICENSE_MANAGER_SERVER_POST, arszComponentsList[i], &available);
		if (NFailed(result))
		{
			PrintErrorMsg(N_T("NLicenseObtainComponents() failed, result = %d\n"), result);
			return result;
		}
		_ftprintf(stderr, N_T("license for %s %s\n"), arszComponentsList[i], available ? N_T("obtained") : N_T("not available"));
		if (!available)
		{
			// If not available
			return N_E_FAILED;
		}
	}
#else
	N_UNREFERENCED_PARAMETER(arszComponentsList);
	N_UNREFERENCED_PARAMETER(componentsCount);
#endif // !N_PRODUCT_HAS_NO_LICENSES

	return N_OK;
}

NResult ReleaseLicense(const NChar ** arszComponentsList, NInt componentsCount)
{
#ifndef N_PRODUCT_HAS_NO_LICENSES
	NResult result;
	NInt i;

	for (i = 0; i < componentsCount; i++)
	{
		if (_tcscmp(arszComponentsList[i], N_T("")) == 0)
		{
			continue;
		}

		result = NLicenseReleaseComponents(arszComponentsList[i]);
		if (NFailed(result))
		{
			PrintErrorMsg(N_T("NLicenseReleaseComponents() failed, result = %d\n"), result);
			return result;
		}
		_ftprintf(stderr, N_T("license for %s released\n"), arszComponentsList[i]);
	}
#else
	N_UNREFERENCED_PARAMETER(arszComponentsList);
	N_UNREFERENCED_PARAMETER(componentsCount);
#endif // !N_PRODUCT_HAS_NO_LICENSES

	return N_OK;
}

void PrintError(NResult result)
{
	NErrorReport(result);
}

void PrintErrorMsg(NChar * szErrorMessage, NResult result)
{
	_ftprintf(stderr, szErrorMessage, result);
	_ftprintf(stderr, N_T("\n"));
	PrintError(result);
}

NBool ReadAllBytes(const NChar * szFileName, NByte ** ppBuffer, NSizeType * pSize)
{
	FILE *fp;
	NSizeType bufferSize;
	NSizeType bufferRead;
	NByte * buffer;
	NResult ret;

	fp = _tfopen(szFileName, N_T("rb"));
	if (!fp)
	{
		return NFalse;
	}
	fseek(fp, 0, SEEK_END);
	bufferSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	ret = NAlloc(bufferSize * sizeof(NByte), (void **)&buffer);
	if (NFailed(ret))
	{
		PrintErrorMsg(N_T("NAlloc() failed, error %d\n"), ret);
		return NFalse;
	}
	bufferRead = fread(buffer, sizeof(NByte), bufferSize, fp);
	fclose(fp);

	if (bufferRead != bufferSize)
	{
		NFree(buffer);
		return NFalse;
	}

	*ppBuffer = buffer;
	*pSize = bufferSize;

	return NTrue;
}

NBool ReadAllBytesN(const NChar * szFileName, HNBuffer * phBuffer)
{
	NSizeType bufferSize;
	NByte *buffer = NULL;
	NResult ret;

	if (!ReadAllBytes(szFileName, &buffer, &bufferSize))
		return NFalse;

	ret = NBufferCreateFromPtr(buffer, bufferSize, NTrue, phBuffer);
	if(NFailed(ret))
	{
		NFree(buffer);
		PrintErrorMsg(N_T("NBufferCreateFromPtr() failed, error %d\n"), ret);
		return NFalse;
	}

	return NTrue;
}

NBool WriteAllBytes(const NChar * szFileName, const NByte * pBuffer, NSizeType bufferSize)
{
	FILE *fp;
	NSizeType bytesWritten;

	fp = _tfopen(szFileName, N_T("wb"));
	if (!fp)
	{
		return NFalse;
	}

	bytesWritten = fwrite(pBuffer, sizeof(NByte), bufferSize, fp);
	fclose(fp);

	if (bytesWritten != bufferSize)
	{
		return NFalse;
	}

	return NTrue;
}

NBool WriteAllBytesN(const NChar * szFileName, HNBuffer hBuffer)
{
	NSizeType bufferSize;
	void * pBuffer;
	NResult result;

	result = NBufferGetSize(hBuffer, &bufferSize);
	if (NFailed(result))
	{
		PrintErrorMsg(N_T("NBufferGetSize() failed(result = %d)!\n"), result);
		return NFalse;
	}

	result = NBufferGetPtr(hBuffer, &pBuffer);
	if (NFailed(result))
	{
		PrintErrorMsg(N_T("NBufferGetPtr() failed(result = %d)!\n"), result);
		return NFalse;
	}

	return WriteAllBytes(szFileName, pBuffer, bufferSize);
}

NResult DecodeTime(NLong dateTime, time_t * pTime, NInt * pMiliseconds)
{
	NLong UnixEpoch = 621355968000000000LL;
	NLong TicksPerSecond = 10000000LL;
	NLong TicksPerMillisecond = 10000LL;
	NLong value1, value2;

	dateTime -= UnixEpoch;
	value1 = dateTime / TicksPerSecond;
	value2 = (dateTime - value1 * TicksPerSecond) / TicksPerMillisecond; 
	if(dateTime < 0 || value1 > (NLong)(((NULong)((time_t)-1)) / 2))
		return N_E_OVERFLOW;
	if(dateTime - value1 * TicksPerSecond - value2 * TicksPerMillisecond >= (TicksPerMillisecond / 2))
	{
		if (++value2 == 1000)
		{
			value2 = 0;
			if (value1 == (NLong)(((NULong)((time_t)-1)) / 2)) return N_E_OVERFLOW;
			value1++;
		}
	}
	*pTime = (time_t) value1;
	*pMiliseconds = (NInt)value2;

	return N_OK;
}

NChar * FormatTime(NLong dateTime)
{
	NChar * buffer = NULL;
	time_t time;
	struct tm  *ts;
	NInt miliseconds;
	NChar format[256];

	if (NFailed(DecodeTime(dateTime, &time, &miliseconds)))
	{
		return NULL;
	}

	buffer = malloc(sizeof(NChar) * 256);
	if (!buffer)
		return NULL;

	_sntprintf(format, 256, N_T("%%Y-%%m-%%d %%H:%%M:%%S:%ld"), (long)miliseconds);

	ts = localtime(&time);
	_tcsftime(buffer, 256, format, ts);
	
	return buffer;
}

#ifndef N_WINDOWS

void Sleep(unsigned long milliseconds)
{
	struct timespec t, r;
	t.tv_sec = milliseconds / 1000;
	t.tv_nsec = (milliseconds % 1000) * 1000000;
	while (nanosleep(&t, &r) == -1)
	{
		t = r;
	}
}

#endif
