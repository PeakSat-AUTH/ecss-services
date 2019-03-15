#ifndef ECSS_SERVICES_TIMEMANAGEMENTSERVICE_HPP
#define ECSS_SERVICES_TIMEMANAGEMENTSERVICE_HPP


#include <Service.hpp>
#include "Helpers/TimeHelper.hpp"

/**
 * Implementation of the ST[09] time management.
 *
 * @notes
 * There is a noticeable difference between setting the time using GPS and setting the time
 * using space packets from the ground segment. The GPS module sent the actual time of UTC(123519
 * is 12:35:19 UTC),while space packets,for time configuration,sent the elapsed time units
 * (seconds,days depends on the time format) from a specific epoch(1 January 1958 00:00:00). Time
 * updates using GPS have nothing to do with this service, but  for consistency and simplicity we
 * are trying to set the time with a common way independently of the time source. This is also
 * the reason that we chose CDS time format(because it is UTC based, check class `TimeHelper`)
 *
 * About the GPS receiver,we assume that it outputs NMEA(message format) data
 *
 * @todo check if we need to follow the standard for time-management or we should send the time-data
 * like GPS
 * @todo check if the final GPS receiver support NMEA protocol
 * @todo When the time comes for the application processes we should consider this: All reports
 * generated by the application process that is identified by APID 0 are time reports
 * @todo Declare the time accuracy that the standard claims in the spacecraft
 * time reference section(6.9.3.d,e)
 */

class TimeManagementService : public Service {
public:
	TimeManagementService() {
		serviceType = 9;
	}

	/**
	 * TM[9,3] CDS time report.
	 *
	 * This function sends reports with the spacecraft time that is formatted according to the CDS
	 * time code format(check class `TimeHelper` for the format)
	 *
	 * @param TimeInfo the time information/data from the RTC(UTC format)
	 * @todo check if we need spacecraft time reference status
	 * @todo ECSS standard claims: <<The time reports generated by the time reporting subservice
	 * are spacecraft time packets. A spacecraft time packet does not carry the message type,
	 * consisting of the service type and message subtype.>> Check if we need to implement that
	 * or should ignore the standard?
	 */

	void cdsTimeReport(TimeAndDate &TimeInfo);

	/**
	 * TC[9,128] CDS time request.
	 *
	 * This function is a custom subservice(mission specific) with message type 128(as defined
	 * from the standard for custom message types, 5.3.3.1.f) and it parses the data of the
	 * time-management telecommand packet. This data is formatted according to the CDS time code
	 * format(check class `TimeHelper` for the format)
	 *
	 * @param message the message that will be parsed for its time-data. The data of the \p message
	 * should be a fixed size of 48 bits
	 */
	 TimeAndDate cdsTimeRequest(Message &message);
};


#endif //ECSS_SERVICES_TIMEMANAGEMENTSERVICE_HPP
