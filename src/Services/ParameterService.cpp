#include "Services/ParameterService.hpp"

#define DEMOMODE

#ifdef DEMOMODE

#include <ctime>
#include <cstdlib>

#endif

ParameterService::ParameterService() {
#ifdef DEMOMODE
	// Test code, setting up some of the parameter fields

	time_t currTime = time(nullptr);
	struct tm *today = localtime(&currTime);

	Parameter test1, test2;

	test1.settingData = today->tm_hour;    // the current hour
	test1.ptc = 3;                         // unsigned int
	test1.pfc = 14;                        // 32 bits

	test2.settingData = today->tm_min;     // the current minute
	test2.ptc = 3;                         // unsigned int
	test2.pfc = 14;                        // 32 bits

	// MAKE SURE THE IDS ARE UNIQUE WHEN INSERTING!
	/**
	 * @todo: Make a separate insert() function for parameter insertion to protect from blunders
	 * if needed to
	 */

	paramsList.insert(std::make_pair(0, test1));
	paramsList.insert(std::make_pair(1, test2));

#endif
}

void ParameterService::reportParameterIds(Message paramIds) {
	Message reqParam(20, 2, Message::TM, 1);    // empty TM[20, 2] parameter report message

	if (paramIds.packetType == Message::TC && paramIds.serviceType == 20 &&
	    paramIds.messageType == 1) {
		uint16_t ids = paramIds.readUint16();
		reqParam.appendUint16(numOfValidIds(paramIds));   // include the number of valid IDs

		for (int i = 0; i < ids; i++) {
			uint16_t currId = paramIds.readUint16();      // current ID to be appended

			if (paramsList.find(currId) != paramsList.end()) {
				reqParam.appendUint16(currId);
				reqParam.appendUint32(paramsList[currId].settingData);
			}

			else {
								// generate failure of execution notification for ST[06]
				continue;       // ignore the invalid ID
			}
		}
	}

	storeMessage(reqParam);
}

void ParameterService::setParameterIds(Message newParamValues) {
	if (newParamValues.packetType == Message::TC && newParamValues.serviceType == 20 &&
	newParamValues.messageType == 3) {
		uint16_t ids = newParamValues.readUint16();  //get number of ID's

		for (int i = 0; i < ids; i++) {
			uint16_t currId = newParamValues.readUint16();

			if (paramsList.find(currId) != paramsList.end()) {
				paramsList[currId].settingData = newParamValues.readUint32();
			}

			else {
								// generate failure of execution notification for ST[06]
				continue;       // ignore the invalid ID
			}
		}
	}
}

uint16_t ParameterService::numOfValidIds(Message idMsg) {
	idMsg.resetRead();
	// start reading from the beginning of the idMsg object
	// (original obj. will not be influenced if this is called by value)

	uint16_t ids = idMsg.readUint16();        // first 16bits of the packet are # of IDs
	uint16_t validIds = 0;

	for (int i = 0; i < ids; i++) {
		uint16_t currId = idMsg.readUint16();

		if (idMsg.messageType == 3) {
			idMsg.readUint32();   //skip the 32bit settings blocks, we need only the IDs
		}

		if (paramsList.find(currId) != paramsList.end()) {
			validIds++;
		}
	}

	return validIds;
}
