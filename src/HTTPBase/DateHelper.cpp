/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "DateHelper.h"
#include <cctype>
#include <cstring>
#include "Common/Macros.h"
#include "Common/CharHelper.h"
#include "Common/StringIndex.h"
#include "Tracker/Trace.h"

using std::memset;
using std::isdigit;

const char* CDateHelper::s_GMTName = "GMT";
const char* CDateHelper::s_WeekDayNames[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
const char* CDateHelper::s_MonthNames[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
const char* CDateHelper::s_RFC850WeekDayNames[] = {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
};

const CStringIndex* CDateHelper::s_pWeekDayNameIndex =
    CStringIndex::CreateInstance(COUNT_OF_ARRAY(s_WeekDayNames), s_WeekDayNames);
const CStringIndex* CDateHelper::s_pMonthNameIndex =
    CStringIndex::CreateInstance(COUNT_OF_ARRAY(s_MonthNames), s_MonthNames);
const CStringIndex* CDateHelper::s_pRFC850WeekDayNameIndex =
    CStringIndex::CreateInstance(COUNT_OF_ARRAY(s_RFC850WeekDayNames), s_RFC850WeekDayNames);

const tSecondTick CDateHelper::s_GMTLocalDeltaSeconds = CalcGMTLocalDeltaSeconds();

bool CDateHelper::GetHTTPDateByString(const char* pString, size_t len, struct tm* pOutDate)
{
    ASSERT(pString);

    const char* pComma = NSCharHelper::FindChar(',', pString, pString + len);
    memset(pOutDate, 0, sizeof(*pOutDate));
    if (pComma) {
        const char* pCur = pComma + 1;
        size_t length = pString + len - pCur;
        size_t daynameLen = pComma - pString;
        if ((pOutDate->tm_wday =
                s_pWeekDayNameIndex->GetIndexByString(pString, daynameLen)) != -1) {
            // IMF-fixdate format
            if (!GetDateByIMFString(pCur, length, pOutDate)) {
                return false;
            }
        } else if ((pOutDate->tm_wday =
            s_pRFC850WeekDayNameIndex->GetIndexByString(pString, daynameLen)) != -1) {
            // obsolete RFC 850 format
            if (!GetDateByRFC850String(pCur, length, pOutDate)) {
                return false;
            }
        } else {
            OUTPUT_ERROR_TRACE("Unknown Date Format.\n");
            return false;
        }
    } else {
        // ANSI C's asctime() format        
        const char* pSpace = NSCharHelper::FindChar(' ', pString, pString + len);
        if (pSpace == NULL ||
            (pOutDate->tm_wday = s_pWeekDayNameIndex->GetIndexByString(
                pString, pSpace - pString)) == -1) {
            OUTPUT_ERROR_TRACE("Wrong format of ANSI C's asctime (Week Day).\n");
            return false;
        }
        const char* pCur = pSpace + 1;
        size_t length = pString + len - pCur;
        if (!GetDateByASCString(pCur, length, pOutDate)) {
            return false;
        }
    }

    if (!IsValidDate(pOutDate)) {
        OUTPUT_ERROR_TRACE("Wrong Content of Date.\n");
        return false;
    }
    return true;
}

bool CDateHelper::GetCookieDateByString(
    const char* pString, size_t len, struct tm* pOutDate)
{
    const char* pComma = NSCharHelper::FindChar(',', pString, pString + len);
    memset(pOutDate, 0, sizeof(*pOutDate));
    if (pComma) {
        const char* pCur = pComma + 1;
        size_t length = pString + len - pCur;
        size_t daynameLen = pComma - pString;

        // IMF-fixdate format week day name.
        if ((pOutDate->tm_wday = s_pWeekDayNameIndex->GetIndexByString(
                pString, daynameLen)) != -1) {
            // RFC850 format date-time string.
            if (GetDateByRFC850String(pCur, length, pOutDate)) {
                if (IsValidDate(pOutDate)) {
                    return true;
                }
            }
        }
    }

    OUTPUT_ERROR_TRACE("Wrong Format of Date.\n");
    return false;
}

bool CDateHelper::GetTimeOfDayByString(
    const char* pString, size_t len, int* pOutHour, int* pOutMinute, int* pOutSecond)
{
    if (len == 8 &&
        pString[2] == ':' && pString[5] == ':' &&
        isdigit(pString[0]) && isdigit(pString[1]) &&
        isdigit(pString[3]) && isdigit(pString[4]) &&
        isdigit(pString[6]) && isdigit(pString[7])) {
        *pOutHour = (pString[0] - '0') * 10 + (pString[1] - '0');
        *pOutMinute = (pString[3] - '0') * 10 + (pString[4] - '0');
        *pOutSecond = (pString[6] - '0') * 10 + (pString[7] - '0');
        return true;
    }
    return false;
}

bool CDateHelper::IsValidDate(struct tm* pDate)
{
    if ((pDate->tm_mon >= 0 && pDate->tm_mon < 12) &&
        (pDate->tm_hour >= 0 && pDate->tm_hour < 24) &&
        (pDate->tm_min >= 0 && pDate->tm_min < 60) &&
        (pDate->tm_sec >= 0 && pDate->tm_sec < 60) &&
        (pDate->tm_mday > 0 && pDate->tm_mday <= 31)) {
        static const int s_MonthDay[2][12] = {
            { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 },
            { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
        };
        size_t index = IsLeapYear(pDate->tm_year + 1900) ? 1 : 0;
        return pDate->tm_mday <= s_MonthDay[index][pDate->tm_mon];
    }
    return false;
}

// ABNF:
//  asctime-date (exclude day-name) = date3 SP time-of-day SP year
//                     date3        = month SP ( 2DIGIT / ( SP 1DIGIT ))
bool CDateHelper::GetDateByASCString(
    const char* pString, size_t len, struct tm* pOutDate)
{
    if (len != ASC_DATE_TIME_LENGTH) {
        OUTPUT_ERROR_TRACE("Wrong format of ANSI C's asctime (Length).\n");
        return false;
    }

    const char* pCur = pString;
    const char* pEnd = pString + len;
    ParseState ps = PS_MONTH;
    bool bContinued = true;

    memset(pOutDate, 0, sizeof(*pOutDate));
    do {
        const char *pSectionEnd = NSCharHelper::FindChar(' ', pCur, pEnd);
        switch (ps) {
        case PS_MDAY:
            if (*pCur == ' ') {
                ++pCur;
            }
            if (pSectionEnd &&
                NSCharHelper::GetIntByString(
                    pCur, pSectionEnd - pCur, &pOutDate->tm_mday)) {
                pCur = pSectionEnd + 1;
                ps = PS_TIME;
            } else {
                bContinued = false;
            }
            break;
        case PS_MONTH:
            if (pSectionEnd &&
                (pOutDate->tm_mon = s_pMonthNameIndex->GetIndexByString(
                    pCur, pSectionEnd - pCur)) != -1) {
                pCur = pSectionEnd + 1;
                ps = PS_MDAY;
            } else {
                bContinued = false;
            }
            break;
        case PS_YEAR:
            if (NSCharHelper::GetIntByString(pCur, pEnd - pCur, &pOutDate->tm_year)) {
                pOutDate->tm_year -= 1900;
                ps = PS_COMPLETE;
            }
            bContinued = false;
            break;
        case PS_TIME:
            if (pSectionEnd &&
                GetTimeOfDayByString(
                    pCur,
                    pSectionEnd - pCur,
                    &pOutDate->tm_hour,
                    &pOutDate->tm_min,
                    &pOutDate->tm_sec)) {
                pCur = pSectionEnd + 1;
                ps = PS_YEAR;
            } else {
                bContinued = false;
            }
            break;
        case PS_COMPLETE:
            break;
        default:
            ASSERT(false);
            break;
        }
    } while (bContinued);

    if (ps != PS_COMPLETE) {
        OUTPUT_ERROR_TRACE("Wrong ASC time format in %d\n", ps);
        return false;
    }
    return true;
}

// ABNF:
//  IMF-fixdate (exclude day-name) = SP date1 SP time-of-day SP GMT
//                    date1        = day SP month SP year
bool CDateHelper::GetDateByIMFString(
    const char* pString, size_t len, struct tm* pOutDate)
{
    if (len != IMF_FIXDATE_DATA_TIME_LENGTH || *pString != ' ') {
        OUTPUT_ERROR_TRACE("Wrong format of IMF fix date.\n");
        return false;
    }

    const char* pCur = pString + 1;
    const char* pEnd = pString + len;
    ParseState ps = PS_MDAY;
    bool bContinued = true;

    memset(pOutDate, 0, sizeof(*pOutDate));
    do {
        const char *pSectionEnd = NSCharHelper::FindChar(' ', pCur, pEnd);
        switch (ps) {
        case PS_MDAY:
            if (pSectionEnd &&
                NSCharHelper::GetIntByString(
                    pCur, pSectionEnd - pCur, &pOutDate->tm_mday)) {
                pCur = pSectionEnd + 1;
                ps = PS_MONTH;
            } else {
                bContinued = false;
            }
            break;
        case PS_MONTH:
            if (pSectionEnd &&
                (pOutDate->tm_mon = s_pMonthNameIndex->GetIndexByString(
                    pCur, pSectionEnd - pCur)) != -1) {
                pCur = pSectionEnd + 1;
                ps = PS_YEAR;
            } else {
                bContinued = false;
            }
            break;
        case PS_YEAR:
            if (pSectionEnd &&
                NSCharHelper::GetIntByString(
                    pCur, pSectionEnd - pCur, &pOutDate->tm_year)) {
                pOutDate->tm_year -= 1900;
                pCur = pSectionEnd + 1;
                ps = PS_TIME;
            } else {
                bContinued = false;
            }
            break;
        case PS_TIME:
            if (pSectionEnd &&
                GetTimeOfDayByString(
                    pCur,
                    pSectionEnd - pCur,
                    &pOutDate->tm_hour,
                    &pOutDate->tm_min,
                    &pOutDate->tm_sec)) {
                pCur = pSectionEnd + 1;
                ps = PS_GMT;
            } else {
                bContinued = false;
            }
            break;
        case PS_GMT:
            if (memcmp(s_GMTName, pCur, pEnd - pCur) == 0) {
                ps = PS_COMPLETE;
            }
            bContinued = false;
            break;
        default:
            ASSERT(false);
            break;
        }
    } while (bContinued);

    if (ps != PS_COMPLETE) {
        OUTPUT_ERROR_TRACE("Wrong IMF fix date format in %d\n", ps);
        return false;
    }
    return true;
}

// ABNF:
//  rfc850-date (exclude day-name-l) = SP date2 SP time-of-day SP GMT
//                      date2        = day "-" month "-" 2DIGIT
bool CDateHelper::GetDateByRFC850String(
    const char* pString, size_t len, struct tm* pOutDate)
{
    if (*pString != ' ') {
        OUTPUT_ERROR_TRACE("Wrong format of RFC850 date.\n");
        return false;
    }

    const char* pCur = pString + 1;
    const char* pEnd = pString + len;
    ParseState ps = PS_MDAY;
    bool bContinued = true;
    const char* pAnchor = NULL;

    memset(pOutDate, 0, sizeof(*pOutDate));
    do {
        const char *pSectionEnd = NSCharHelper::FindChar(' ', pCur, pEnd);
        
        switch (ps) {
        case PS_MDAY:
            if (pSectionEnd) {
                pAnchor = NSCharHelper::FindChar('-', pCur, pSectionEnd);
            }
            if (!pAnchor ||
                !NSCharHelper::GetIntByString(
                    pCur, pAnchor - pCur, &pOutDate->tm_mday)) {
                bContinued = false;
                break;
            }

            pCur = pAnchor + 1;
            ps = PS_MONTH;

            // Parse Month
            pAnchor = NSCharHelper::FindChar('-', pCur, pSectionEnd);
            if (!pAnchor ||
                (pOutDate->tm_mon = s_pMonthNameIndex->GetIndexByString(
                    pCur, pAnchor - pCur)) == -1) {
                bContinued = false;
                break;
            }
            pCur = pAnchor + 1;
            ps = PS_YEAR;

            // Parse Year
            if (NSCharHelper::GetIntByString(
                pCur, pSectionEnd - pCur, &pOutDate->tm_year)) {
                if (pOutDate->tm_year >= 0 && pOutDate->tm_year <= 99) {
                    if (pOutDate->tm_year < 70) {
                        pOutDate->tm_year += 100;
                    }
                    
                    pCur = pSectionEnd + 1;
                    ps = PS_TIME;
                    break;
                }
            }
            bContinued = false;
            break;
        case PS_TIME:
            if (pSectionEnd &&
                GetTimeOfDayByString(
                    pCur,
                    pSectionEnd - pCur,
                    &pOutDate->tm_hour,
                    &pOutDate->tm_min,
                    &pOutDate->tm_sec)) {
                pCur = pSectionEnd + 1;
                ps = PS_GMT;
            } else {
                bContinued = false;
            }
            break;
        case PS_GMT:
            if (memcmp(s_GMTName, pCur, pEnd - pCur) == 0) {
                ps = PS_COMPLETE;
            }
            bContinued = false;
            break;
        default:
            ASSERT(false);
            break;
        }
    } while (bContinued);

    if (ps != PS_COMPLETE) {
        OUTPUT_ERROR_TRACE("Wrong RFC850 date format in %d\n", ps);
        return false;
    }
    return true;
}

bool CDateHelper::Serialize(
    char* pBuffer, size_t len, struct tm* pDate, std::size_t* pOutLen)
{
    size_t serializedLen = 0;
    const char* pWeekDayName = s_pWeekDayNameIndex->GetStringByIndex(pDate->tm_wday);
    const char* pMonthName = s_pMonthNameIndex->GetStringByIndex(pDate->tm_mon);

    ASSERT(pWeekDayName && pMonthName);
    serializedLen = snprintf(
        pBuffer, len,
        "%s, %02d %s %04d %02d:%02d:%02d %s",
        pWeekDayName,
        pDate->tm_mday,
        pMonthName,
        pDate->tm_year + 1900,
        pDate->tm_hour,
        pDate->tm_min,
        pDate->tm_sec,
        s_GMTName);
    ASSERT(serializedLen > 0);
    *pOutLen = serializedLen;
    return serializedLen == IMF_FIXDATE_LENGTH;
}

tSecondTick CDateHelper::CalcGMTLocalDeltaSeconds()
{
    time_t utcBeginSec = 0;
    struct tm utcTM;
    gmtime_r(&utcBeginSec, &utcTM);     // UTC begin seconds to UTC TM.
    // Consider UTC TM as the local TM.
    time_t delta = mktime(&utcTM);
    ASSERT(delta != -1);
    ASSERT(delta <= 11 * 3600 && delta >= -1 * 11 * 3600);
    return static_cast<tSecondTick>(delta);
}
