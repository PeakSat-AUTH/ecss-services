#include <cstring>
#include "ErrorHandler.hpp"
#include "MessageParser.hpp"
#include "macros.hpp"
#include "Services/TestService.hpp"
#include "Services/RequestVerificationService.hpp"

TestService TestService::instance;
RequestVerificationService RequestVerificationService::instance;

void MessageParser::execute(Message &message) {
	switch (message.serviceType) {
		case 1:
			RequestVerificationService::instance.execute(message);
			break;
		case 17:
			TestService::instance.execute(message);
			break;
		default:
			// cout is very bad for embedded systems
			std::cout << "This service hasn't been implemented yet or it doesn't exist";
			break;
	}
}

Message MessageParser::parse(uint8_t *data, uint32_t length) {
	assertI(length >= 6, ErrorHandler::UnacceptablePacket);

	uint16_t packetHeaderIdentification = (data[0] << 8) | data[1];
	uint16_t packetSequenceControl = (data[2] << 8) | data[3];
	uint16_t packetDataLength = (data[4] << 8) | data[5];

	// Individual fields of the CCSDS Space Packet primary header
	uint8_t versionNumber = data[0] >> 5;
	Message::PacketType packetType = ((data[0] & 0x10) == 0) ? Message::TM : Message::TC;
	uint8_t secondaryHeaderFlag = data[0] & static_cast<uint8_t>(0x08);
	uint16_t APID = packetHeaderIdentification & static_cast<uint16_t>(0x07ff);
	auto sequenceFlags = static_cast<uint8_t>(packetSequenceControl >> 14);

	// Returning an internal error, since the Message is not available yet
	assertI(versionNumber == 0, ErrorHandler::UnacceptablePacket);
	assertI(secondaryHeaderFlag == 1, ErrorHandler::UnacceptablePacket);
	assertI(sequenceFlags == 0x3, ErrorHandler::UnacceptablePacket);
	assertI(packetDataLength == length - 6, ErrorHandler::UnacceptablePacket);

	Message message(0, 0, packetType, APID);

	if (packetType == Message::TC) {
		parseTC(data + 6, packetDataLength, message);
	} else {
		assert(false); // Not implemented yet
	}

	return message;
}

void MessageParser::parseTC(uint8_t *data, uint16_t length, Message &message) {
	ErrorHandler::assertRequest(length >= 5, message, ErrorHandler::UnacceptableMessage);

	// Individual fields of the TC header
	uint8_t pusVersion = data[0] >> 4;
	uint8_t serviceType = data[1];
	uint8_t messageType = data[2];

	ErrorHandler::assertRequest(pusVersion == 2, message, ErrorHandler::UnacceptableMessage);

	// Remove the length of the header
	length -= 5;

	// Copy the data to the message
	// TODO: See if memcpy is needed for this
	message.serviceType = serviceType;
	message.messageType = messageType;
	memcpy(message.data, data + 5, length);
	message.dataSize = length;
}
