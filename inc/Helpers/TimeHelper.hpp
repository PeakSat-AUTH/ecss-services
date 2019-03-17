#ifndef ECSS_SERVICES_TIMEHELPER_HPP
#define ECSS_SERVICES_TIMEHELPER_HPP

#include <cstdint>
#include <Message.hpp>

#define SECONDS_PER_MINUTE 60
#define SECONDS_PER_HOUR 3600
#define SECONDS_PER_DAY 86400

/**
 * The time and date provided from Real Time Clock (Real Time Clock).
 *
 * @notes
 * This struct is similar to the `struct tm` of <ctime> library but it is more embedded-friendly
 *
 * For the current implementation this struct takes dummy values, because RTC hasn't been
 * implemented
 */
struct TimeAndDate {
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
};

/**
 * This class formats the spacecraft time and cooperates closely with the ST[09] time management.
 *
 * The ECSS standard supports two time formats: the CUC and CSD that are described in CCSDS
 * 301.0-B-4 standard. The chosen time format is CDS and it is UTC-based (UTC: Coordinated
 * Universal Time). It consists of two main fields: the time code preamble field (P-field) and
 * the time specification field (T-field). The P-Field is the metadata for the T-Field. The
 * T-Field is consisted of two segments: 1) the `DAY` and the 2) `ms of day` segments.
 * The P-field won't be included in the code, because as the ECSS standards claims, it can be
 * just implicitly declared.
 *
 * @note
 * Since this code is UTC-based, the leap second correction must be made. The leap seconds that
 * have been occurred between timestamps should be considered if a critical time-difference is
 * needed
 *
 */
class TimeHelper {
public:
	static constexpr uint8_t DaysOfMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

	TimeHelper() = default;

	/**
	 * @param year The year that will be examined if it is a leap year (366 days)
	 * @return if the \p year is a leap year returns true and if it isn't returns false
	 */
	static bool IsLeapYear(uint16_t year);

	/**
     * Convert UTC date to elapsed seconds since Unix epoch (1/1/1970 00:00:00).
     *
     * This is a reimplemented mktime() of <ctime> library in an embedded systems way
     *
     * @note
     * This function can convert UTC dates after 1 January 2019 to elapsed seconds since Unix epoch
     *
     * @param TimeInfo the time information/data from the RTC (UTC format)
     * @return the elapsed seconds between a given UTC date (after the Unix epoch) and Unix epoch
     * @todo check if we need to change the epoch to the recommended one from the standard, 1
     * January 1958
     */
	static uint32_t mkUTCtime(struct TimeAndDate &TimeInfo);

	/**
     * Convert elapsed seconds since Unix epoch to UTC date.
     *
     * This is a reimplemented gmtime() of <ctime> library in an embedded systems way
     *
     * @note
     * This function can convert elapsed seconds since Unix epoch to UTC dates after 1 January 2019
     *
     * @param seconds elapsed seconds since Unix epoch
     * @return the UTC date based on the \p seconds
     * @todo check if we need to change the epoch to the recommended one from the standard, 1
     * January 1958
     */
	static struct TimeAndDate utcTime(uint32_t seconds);


	/**
	 * Generate the CDS time format (3.3 in CCSDS 301.0-B-4 standard).
	 *
	 * Converts a UTC date to CDS time format.
	 *
	 * @param TimeInfo is the data provided from RTC (UTC)
	 * @return TimeFormat the CDS time format. More specific, 48 bits are used for the T-field
	 * (16 for the `DAY` and 32 for the `ms of day`)
 	 * @todo time security for critical time operations
 	 * @todo declare the implicit P-field
 	 * @todo check if we need milliseconds
	 */
	static uint64_t generateCDStimeFormat(struct TimeAndDate &TimeInfo);

	/**
	 * Parse the CDS time format (3.3 in CCSDS 301.0-B-4 standard)
	 *
     * @param data time information provided from the ground segment. The length of the data is a
     * fixed size of 48 bits
	 * @return the UTC date
	 */
	static struct TimeAndDate parseCDStimeFormat(const uint8_t *data);
};


#endif //ECSS_SERVICES_TIMEHELPER_HPP
