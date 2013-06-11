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

static class DateProperty : public Script::Property
{
public:
	DateProperty() : Property("date") {};
	DateProperty(const char *id) : Property(id) {};

private:
	virtual void setProperty(char *dp, char *tp, size_t size);
	virtual void getProperty(char *dp, char *tp, size_t size);
	virtual void adjProperty(char *dp, size_t size, int value);
	size_t getPropertySize(void)
		{return 10;};
} pDate;

static void adjdate(char *buf, size_t size, int year, int mon, int day)
{
	int dy = 0, dm = 0, dd = 0;
	time_t now;
	struct tm dt, *od;
	if(strchr(buf, '/'))
		sscanf(buf, "%02d/%02d/%04d", &dm, &dd, &dy);
	else if(strchr(buf, '.'))
		sscanf(buf, "%02d.%02d.%04d", &dd, &dm, &dy);
	else
		sscanf(buf, "%04d%02d%02d", &dy, &dm, &dd);
	
	dm += mon;
	if(dm < 0)
		year += (dm - 11) / 12 - 1;
	else
		year += dm / 12;

	dm %= 12;

	dy += year;

	memset(&dt, 0, sizeof(dt));
	dt.tm_year = dy - 1900;
	dt.tm_mday = dd;
	dt.tm_mon = dm - 1;
	now = mktime(&dt);
	now += (day * 24l * 60l * 60l);
	od = localtime(&now);
	++od->tm_mon;
	od->tm_year += 1900;

	if(size < 5)
		return;

	if(size < 10)
		sprintf(buf, "%02u/%02u", od->tm_mon, od->tm_mday);
	else
		sprintf(buf, "%02u/%02u/%04u", od->tm_mon, od->tm_mday, od->tm_year);
}
	
static class YearProperty : public DateProperty
{
public:
	YearProperty() : DateProperty("year") {};
	
private:
	void getProperty(char *dp, char *tp, size_t size);

	void adjProperty(char *dp, size_t sz, int value)
		{adjdate(dp, sz, value, 0, 0);};
} pYear;

static class MonthProperty : public DateProperty
{
public:
	MonthProperty() : DateProperty("month") {};

private:
	void getProperty(char *dp, char *tp, size_t size);

	void adjProperty(char *dp, size_t sz, int value)
		{adjdate(dp, sz, 0, value, 0);};

	size_t getPropertySize(void)
		{return 6;};
} pMon;

static class WeekdayProperty : public DateProperty
{
public:
	WeekdayProperty() : DateProperty("weekday") {};

private:
	void setProperty(char *dp, char *tp, size_t size);

	size_t getPropertySize(void)
		{return 3;};
} pWeekday;

static class DayProperty : public DateProperty
{
public:
	DayProperty() : DateProperty("day") {};

private:
	void getProperty(char *dp, char *tp, size_t size);

	size_t getPropertySize(void)
		{return 6;};

} pDay;

void YearProperty::getProperty(char *dp, char *tp, size_t size)
{
	time_t now;
	struct tm *dt;
	int year;
	char *ext = strchr(dp, '.');

	time(&now);
	dt = localtime(&now);

	if(ext)
		ext = strchr(++ext, '.');
	else
	{
		ext = strchr(dp, '/');
		if(ext)
			ext = strchr(++ext, '/');
	}
	if(ext)
	{
		year = atoi(++ext);
		if(year < 100)
			year += 1900 + (dt->tm_year / 100) * 100;
	}
	else if(strlen(dp) > 7)
	{
		strncpy(tp, dp, 4);
		tp[4] = 0;
		return;
	}
	else
		year = 1900 + dt->tm_year;
	
	snprintf(tp, size, "%04d", year);
}

void DayProperty::getProperty(char *dp, char *tp, size_t size)
{
	char *ext;
	ext = strchr(dp, '.');
	if(ext)
	{
		snprintf(tp, size, "%02d", atoi(dp) % 31);
		return;
	}
	ext = strchr(dp, '/');
	if(ext)
	{
		snprintf(tp, size, "%02d", atoi(++ext) % 31);
		return;
	}
	if(strlen(dp) < 4)
	{
		strcpy(tp, "00");
		return;
	}
	if(strlen(dp) < 8)
	{
		strncpy(tp, dp + 2, 2);
		tp[2] = 0;
		return;
	}
	strncpy(tp, dp + 6, 2);
	tp[2] = 0;
	return;
}

void MonthProperty::getProperty(char *dp, char *tp, size_t size)
{
	char *ext;
	ext = strchr(dp, '/');
	if(ext)
	{
		snprintf(tp, size, "%02d", atoi(dp) % 31);
		return;
	}
	ext = strchr(dp, '.');
	if(ext)
	{
		snprintf(tp, size, "%02d", atoi(++ext) % 31);
		return;
	}
	if(strlen(dp) < 4)
	{
		strcpy(tp, "00");
		return;
	}
	if(strlen(dp) < 8)
	{
		strncpy(tp, dp, 2);
		tp[2] = 0;
		return;
	}
	strncpy(tp, dp + 4, 2);
	tp[2] = 0;
	return;
}

void DateProperty::adjProperty(char *dp, size_t sz, int value)
{
	adjdate(dp, sz, 0, 0, value);
}

void DateProperty::getProperty(char *dp, char *tp, size_t size)
{
	unsigned year = 0, mon = 0, day = 0;
	char *ext;
	time_t now;
	struct tm *dt;

	ext = strchr(dp, '/');
	if(ext)
	{
		mon = atoi(dp);
		day = atoi(++ext);
		ext = strchr(ext, '/');
		if(ext)
			year = atoi(++ext);
	}
	else if(NULL != (ext = strchr(dp, '.')))
	{
		day = atoi(dp);
		mon = atoi(++ext);
		ext = strchr(ext, '.');
		if(ext)
			year = atoi(++ext);
	}		

	if(!year)
	{
		time(&now);
		dt = localtime(&now);
		++dt->tm_mon;
		year = dt->tm_year + 1900;
		if(dt->tm_mday > day && dt->tm_mon == mon)
			++year;
		if(dt->tm_mon > mon)
			++year; 
		if(!mon)
			mon = dt->tm_mon;
		if(!day)
			day = dt->tm_mday;
	}
	if(size < 4)
		return;
	if(size < 8)
		snprintf(tp, size, "%02d%02d", mon, day);
	else
		snprintf(tp, size, "%04d%02d%02d", year, mon, day);
	return;
}

void DateProperty::setProperty(char *dp, char *tp, size_t size)
{
	time_t now;
	struct tm *dt;
	size_t sz = 10;
	unsigned year = 0, mon = 0, day = 0;

	if(size < 10)
		sz = size;

	strncpy(dp, "00/00/0000", sz);
	if(size < 5)
		return;

	sz = strlen(tp);
	time(&now);
	dt = localtime(&now);
	dt->tm_year += 1900;
	++dt->tm_mon;

	if(!sz)
	{
		time(&now);
		dt = localtime(&now);
		++dt->tm_mon;
		dt->tm_year += 1900;
		if(size < 10)
			sprintf(dp, "%02d/%02d", dt->tm_mon, dt->tm_mday);
		else
			sprintf(dp, "%02d/%02d/%04d", dt->tm_mon, dt->tm_mday, dt->tm_year);
		return;
	}
	if(strchr(tp, '/'))
		sscanf(tp, "%02d/%02d/%04d", &mon, &day, &year);
	else if(strrchr(tp, '.'))
		sscanf(tp, "%02d.%02d.%04d", &day, &mon, &year);
	else if(strlen(tp) > 7)
		sscanf(tp, "%04%02%02d", &year, &mon, &day);
	else if(strlen(tp) > 5)
		sscanf(tp, "%02d%02d%02d", &year, &mon, &day);
	else sscanf(tp, "%02d%02d", &mon, &day);

	if(!year)
		year = dt->tm_year;

	if(!mon)
		mon = dt->tm_mon;

	if(!day)
		day = dt->tm_mday;

	if(size < 5)
		return;
	if(size < 10)
		snprintf(dp, size + 1, "%02d/%02d", mon, day);
	else
		snprintf(dp, size + 1, "%02d/%02d/%04d", mon, day, year);
}

void WeekdayProperty::setProperty(char *dp, char *tp, size_t size)
{
	static	char *days[] =
		{"sun", "mon", "tue", "wed", "thu", "fri", "sat"};

	time_t now;
	struct tm *dt, dtm;
	size_t sz = 10;
	unsigned year = 0, mon = 0, day = 0;

	sz = strlen(tp);
	time(&now);
	dt = localtime(&now);
	dt->tm_year += 1900;
	++dt->tm_mon;

	if(sz < 4)
	{
		time(&now);
		dt = localtime(&now);
		++dt->tm_mon;
		dt->tm_year += 1900;
		snprintf(dp, 4, "%3s", days[dt->tm_wday]);
		return;
	}
	if(strchr(tp, '/'))
		sscanf(tp, "%02d/%02d/%04d", &mon, &day, &year);
	else if(strrchr(tp, '.'))
		sscanf(tp, "%02d.%02d.%04d", &day, &mon, &year);
	else if(strlen(tp) > 7)
		sscanf(tp, "%04%02%02d", &year, &mon, &day);
	else if(strlen(tp) > 5)
		sscanf(tp, "%02d%02d%02d", &year, &mon, &day);
	else sscanf(tp, "%02d%02d", &mon, &day);

	if(!year)
		year = dt->tm_year;

	if(!mon)
		mon = dt->tm_mon;

	if(!day)
		day = dt->tm_mday;

	memset(&dtm, 0, sizeof(dtm));
	dtm.tm_mday = day;
	dtm.tm_mon = mon - 1;
	if(year > 200)
		dtm.tm_year = year - 1900;
	else
		dtm.tm_year = year;
	now = mktime(&dtm);
	dt = localtime(&now);
	snprintf(dp, 4, "%3s", days[dt->tm_wday]);
}

#ifdef	CCXX_NAMESPACES
};
#endif


