// Copyright (C) 1999-2001 Open Source Telecom Corporation.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
// 
// As a special exception to the GNU General Public License, permission is 
// granted for additional uses of the text contained in its release 
// of ccscript.
// 
// The exception is that, if you link the ccscript library with other
// files to produce an executable, this does not by itself cause the
// resulting executable to be covered by the GNU General Public License.
// Your use of that executable is in no way restricted on account of
// linking the ccscript library code into it.
// 
// This exception does not however invalidate any other reasons why
// the executable file might be covered by the GNU General Public License.
// 
// This exception applies only to the code released under the 
// name ccscript.  If you copy code from other releases into a copy of
// ccscript, as the General Public License permits, the exception does
// not apply to the code that you add in this way.  To avoid misleading
// anyone as to the status of such modified files, you must delete
// this exception notice from them.
//
// If you write modifications of your own for ccscript, it is your choice
// whether to permit this exception to apply to your modifications.
// If you do not wish that, delete this exception notice.  

#include <cc++/config.h>
#include <ctime>
#include "script.h"

#ifdef	CCXX_NAMESPACES
namespace ost {
#endif

static class TimeProperty : public Script::Property
{
public:
	TimeProperty() : Property("time") {};
	TimeProperty(const char *id) : Property(id) {};

private:
	virtual void setProperty(char *dp, char *tp, size_t size);
	virtual void getProperty(char *dp, char *tp, size_t size);
	virtual void adjProperty(char *dp, size_t size, int value);
	size_t getPropertySize(void)
		{return 8;};
} pTime;

static void adjtime(char *buf, size_t size, int hr, int min, int sec)
{
	int dh = 0, dm = 0, ds = 0;
	if(strchr(buf, ':'))
		sscanf(buf, "%02d:%02d:%02d", &dh, &dm, &ds);
	else
		sscanf(buf, "%02d%02d%02d", &dh, &dm, &ds);

	dh %= 24;
	dm %= 60;
	ds %= 60;

	ds += sec;
	if(ds < 0)
		min += (ds - 59 ) / 60 - 1;
	else
		min += ds / 60;
	sec %= 60;

	dm += min;
	if(dm < 0)
		hr += (dm - 59) / 60 - 1;
	else
		hr += dm / 60;
	dm %= 60;

	dh += hr;
	dh %= 24;

	if(dh < 0)
		dh = -dh;

	if(dm < 0)
		dm = -dm;

	if(ds < 0)
		ds = -ds;

	if(size < 5)
		return;

	if(size < 8)
		sprintf(buf, "%02u:%02u", dh, dm);
	else
		sprintf(buf, "%02u:%02u:%02u", dh, dm, ds);

}
	
static class HourProperty : public TimeProperty
{
public:
	HourProperty() : TimeProperty("hour") {};
	
private:
	void getProperty(char *dp, char *tp, size_t size);

	void adjProperty(char *dp, size_t sz, int value)
		{adjtime(dp, sz, value, 0, 0);};

	size_t getPropertySize(void)
		{return 6;};
} pHour;

static class MinProperty : public TimeProperty
{
public:
	MinProperty() : TimeProperty("minute") {};

private:
	void getProperty(char *dp, char *tp, size_t size);

	void adjProperty(char *dp, size_t sz, int value)
		{adjtime(dp, sz, 0, value, 0);};

	size_t getPropertySize(void)
		{return 6;};
} pMin;

static class SecProperty : public TimeProperty
{
public:
	SecProperty() : TimeProperty("second") {};

private:
	void getProperty(char *dp, char *tp, size_t size);

} pSec;

void HourProperty::getProperty(char *dp, char *tp, size_t size)
{
	if(strchr(dp, ':'))
	{
		snprintf(tp, size, "%02d", atoi(dp) % 24);
		return;
	}
	if(strlen(dp) < 4)
	{
		strcpy(tp, "00");
		return;
	}

	strncpy(tp, dp, 2);
	tp[2] = 0;
}
	
void SecProperty::getProperty(char *dp, char *tp, size_t size)
{
	unsigned sec;
	char *ext;
	ext = strchr(dp, ':');
	if(ext)
	{
		ext = strchr(dp, ':');
		if(ext)
			sec = atoi(++ext);
		else
			sec = 0;
		sec %= 60;
		snprintf(tp, size, "%02d", sec);
		return;
	}
	if(strlen(dp) < 6)
	{
		strcpy(tp, "00");
		return;
	}
	strncpy(tp, dp + 4, 2);
	tp[2] = 0;
}

void MinProperty::getProperty(char *dp, char *tp, size_t size)
{
	unsigned min;
	char *ext;
	ext = strchr(dp, ':');
	if(ext)
	{
		min = atoi(++ext);
		min %= 60;
		snprintf(tp, size, "%02d", min);
		return;
	}
	if(strlen(dp) < 4)
	{
		strcpy(tp, "00");
		return;
	}
	strncpy(tp, dp + 2, 2);
	tp[2] = 0;
}

void TimeProperty::adjProperty(char *dp, size_t sz, int value)
{
	if(sz < 8)
		adjtime(dp, sz, 0, value, 0);
	else
		adjtime(dp, sz, 0, 0, value);
}

void TimeProperty::getProperty(char *dp, char *tp, size_t size)
{
	unsigned hr, min = 0, sec = 0;
	char *ext;

	hr = atoi(dp);
	ext = strchr(dp, ':');
	if(ext)
	{
		min = atoi(++ext);
		ext = strchr(ext, ':');
	}
	if(ext)
		sec = atoi(++ext);

	hr %= 24;
	min %= 60;
	sec %= 60;

	if(size < 4)
		return;
	if(size < 6)
		sprintf(tp, "%02d%02d", hr, min);
	else
		sprintf(tp, "%02d%02d%02d", hr, min, sec);
	return;
}

void TimeProperty::setProperty(char *dp, char *tp, size_t size)
{
	time_t now;
	struct tm *dt;
	size_t sz = 8;
	unsigned hr = 0, min = 0, sec = 0;
	
	if(size < 8)
		sz = size;

	strncpy(dp, "00:00:00", sz);
	if(size < 5)
		return;

	sz = strlen(tp);
	if(!sz)
	{
		::time(&now);
		dt = ::localtime(&now);
		if(size < 8)
			sprintf(dp, "%02d:%02d", dt->tm_hour, dt->tm_min);
		else
			sprintf(dp, "%02d:%02d:%02d", dt->tm_hour, dt->tm_min, dt->tm_sec);
		return;
	}
	if(strchr(tp, ':'))
		sscanf(tp, "%02d:%02d:%02d", &hr, &min, &sec);
	else
		sscanf(tp, "%02d%02d%02d", &hr, &min, &sec);
	hr %= 24;
	min %= 60;
	sec %= 60;
 	snprintf(dp, size + 1, "%02d:%02d:%02d", hr, min, sec);
}

#ifdef	CCXX_NAMESPACES
};
#endif

