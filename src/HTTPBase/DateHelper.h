/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#ifndef __HTTPBASE_DATE_HELPER_H__
#define __HTTPBASE_DATE_HELPER_H__

#include <time.h>
#include <cstring>
#include <errno.h>
#include "Common/Typedefs.h"
#include "Tracker/Trace.h"

using std::strerror;

class CStringIndex;
class CDateHelper
{
public:
    static bool GetHTTPDateByString(const char* pString, size_t len, struct tm* pOutDate);
    static bool GetCookieDateByString(const char* pString, size_t len, struct tm* pOutDate);
    static bool GetTimeOfDayByString(
        const char* pString, size_t len, int* pOutHour, int* pOutMinute, int* pOutSecond);
    static bool IsValidDate(struct tm* pDate);

    static bool IsExpired(tSecondTick seconds)
    {
        time_t now;
        time(&now);
        return static_cast<tSecondTick>(now) >= seconds;
    }

    /**
     *  Convert GMT Date to the seconds ticks compared to UNIX beginning time (1970,...)
     * 
     * @param pGMTDate GMT Date.
     * @param pOutSecTick The output parameter, to store the calculation result.
     * @return Indicate the conversion is OK or not.
     */
    static bool GMTDate2SecondTickets(struct tm* pGMTDate, tSecondTick* pOutSecTick)
    {
        // mktime is consider the parameter as the local time.
        time_t tv = mktime(pGMTDate);
        if (tv != -1) {
            *pOutSecTick = tv - s_GMTLocalDeltaSeconds;
            return true;
        }
        return false;
    }

    static bool IsLeapYear(int year)
    {
        return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    }

    static bool GetDateByASCString(const char* pString, size_t len, struct tm* pDate);
    static bool GetDateByIMFString(const char* pString, size_t len, struct tm* pDate);
    static bool GetDateByRFC850String(const char* pString, size_t len, struct tm* pDate);

    /**
     * @brief Serialize struct tm data to a buffer.
     * @param pBuffer, the buffer to store the serialized data.
     * @param len, the buffer length.
     * @param pDate, the HTTP date to be serialized.
     * @param pOutLen, the output parameter which store the actually serialized data length.
     * @return Return the true if the buffer store whole serialized data.
     */
    static bool Serialize(char* pBuffer, size_t len, struct tm* pDate, size_t* pOutLen);
    static bool Serialize(char* pBuffer, size_t len, tSecondTick seconds, size_t* pOutLen)
    {
        struct tm tmValue;
        if (gmtime_r(reinterpret_cast<const time_t*>(&seconds), &tmValue)) {
            return Serialize(pBuffer, len, &tmValue, pOutLen);
        }
        return false;
    }

private:
    enum ParseState {
        PS_MDAY,
        PS_MONTH,
        PS_YEAR,
        PS_TIME,
        PS_GMT,
        PS_COMPLETE
    };

    /**
     *  Calculate the delta seconds between the GMT and local: Delta = GMT - LocalTime
     *  Since the time zone is 24 including the GMT: [GMT-11, ... GMT, ..., GMT+11],
     *  the value range [-11 * 3600, 11 * 3600], if Local is east to GMT zone, then the
     *  delta value is negative.
     */
    static tSecondTick CalcGMTLocalDeltaSeconds();

    static const size_t IMF_FIXDATE_LENGTH = 29;
    static const size_t IMF_FIXDATE_DATA_TIME_LENGTH = 25;  // Length of SP date1 SP time-of-day SP GMT
    static const size_t ASC_DATE_TIME_LENGTH = 20;          // Length of date3 SP time-of-day SP year

    static const char* s_GMTName;
    static const char* s_WeekDayNames[];
    static const char* s_MonthNames[];
    static const char* s_RFC850WeekDayNames[];

    static const CStringIndex* s_pWeekDayNameIndex;
    static const CStringIndex* s_pMonthNameIndex;
    static const CStringIndex* s_pRFC850WeekDayNameIndex;

    static const tSecondTick s_GMTLocalDeltaSeconds;
};

#endif