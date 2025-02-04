#include "ECSS_Configuration.hpp"
#ifdef SERVICE_MEMORY

#include <cerrno>
#include <etl/String.hpp>
#include "Services/MemoryManagementService.hpp"

MemoryManagementService::MemoryManagementService() : rawDataMemorySubservice(*this) {
	serviceType = MemoryManagementService::ServiceType;
}

MemoryManagementService::RawDataMemoryManagement::RawDataMemoryManagement(MemoryManagementService& parent)
    : mainService(parent) {}

void MemoryManagementService::loadRawData(Message& request) {
	/**
	 * Bear in mind that there is currently no error checking for invalid parameters.
	 * A future version will include error checking and the corresponding error report/notification,
	 * as the manual implies.
	 *
	 * @todo Add error checking and reporting for the parameters
	 * @todo Add failure reporting
	 */
	request.assertTC(MemoryManagementService::ServiceType, MemoryManagementService::MessageType::LoadRawMemoryDataAreas);
	auto memoryID = MemoryManagementService::MemoryID(request.read<MemoryId>());

	if (!memoryIdValidator(MemoryManagementService::MemoryID(memoryID))) {
		// TODO: Send a failed start of execution
		return;
	}

	ReadData readData[ECSSMaxStringSize];
	uint16_t iterationCount = request.readUint16();

	if (memoryID == MemoryManagementService::MemoryID::FLASH_MEMORY) {
		// TODO: Define FLASH specific access code when we transfer to embedded
	} else {
		for (std::size_t j = 0; j < iterationCount; j++) {
			StartAddress startAddress = request.read<StartAddress>();
			MemoryDataLength dataLength = request.readOctetString(readData);
			MemoryManagementChecksum checksum = request.readBits(8*sizeof(MemoryManagementChecksum));

			if (!dataValidator(readData, checksum, dataLength)) {
				ErrorHandler::reportError(request, ErrorHandler::ChecksumFailed);
				continue;
			}

			if (!addressValidator(memoryID, startAddress) ||
			    !addressValidator(memoryID, startAddress + dataLength)) {
				ErrorHandler::reportError(request, ErrorHandler::ChecksumFailed);
				continue;
			}

			for (std::size_t i = 0; i < dataLength; i++) {
				*(reinterpret_cast<uint8_t*>(startAddress) + i) = readData[i];
			}

			for (std::size_t i = 0; i < dataLength; i++) {
				readData[i] = *(reinterpret_cast<uint8_t*>(startAddress) + i);
			}

			if (checksum != CRCHelper::calculateCRC(readData, dataLength)) {
				ErrorHandler::reportError(request, ErrorHandler::ChecksumFailed);
			}
		}
	}
}

void MemoryManagementService::RawDataMemoryManagement::dumpRawData(Message& request) {
	if (!request.assertTC(ServiceType, MessageType::DumpRawMemoryData)) {
		return;
	}

	Message report = mainService.createTM(MemoryManagementService::MessageType::DumpRawMemoryDataReport);
	MemoryId memoryID = request.read<MemoryId>();

	if (memoryIdValidator(MemoryManagementService::MemoryID(memoryID))) {
		ReadData readData[ECSSMaxStringSize];
		uint16_t iterationCount = request.readUint16();

		report.append<MemoryId>(memoryID);
		report.appendUint16(iterationCount);

		for (std::size_t j = 0; j < iterationCount; j++) {
			StartAddress startAddress = request.read<StartAddress>();
			MemoryDataLength readLength = request.read<MemoryDataLength>();

			if (addressValidator(MemoryManagementService::MemoryID(memoryID), startAddress) &&
			    addressValidator(MemoryManagementService::MemoryID(memoryID), startAddress + readLength)) {
				for (std::size_t i = 0; i < readLength; i++) {
					readData[i] = *(reinterpret_cast<uint8_t*>(startAddress) + i);
				}

				report.append<StartAddress>(startAddress);
				report.appendOctetString(String<1024>(readData, readLength));
				report.appendBits(16, CRCHelper::calculateCRC(readData, readLength));
			} else {
				ErrorHandler::reportError(request, ErrorHandler::AddressOutOfRange);
			}
		}

		mainService.storeMessage(report);
		request.resetRead();
	} else {
		// TODO: Send a failed start of execution
	}
}

void MemoryManagementService::RawDataMemoryManagement::checkRawData(Message& request) {
	if (!request.assertTC(ServiceType, MessageType::CheckRawMemoryData)) {
		return;
	}

	Message report = mainService.createTM(MemoryManagementService::MessageType::CheckRawMemoryDataReport);
	MemoryId memoryID = request.read<MemoryId>();

	if (memoryIdValidator(MemoryManagementService::MemoryID(memoryID))) {
		ReadData readData[ECSSMaxStringSize];
		uint16_t iterationCount = request.readUint16();

		report.append<MemoryId>(memoryID);
		report.appendUint16(iterationCount);

		for (std::size_t j = 0; j < iterationCount; j++) {
			StartAddress startAddress = request.read<StartAddress>();
			MemoryDataLength readLength = request.read<MemoryDataLength>();

			if (addressValidator(MemoryManagementService::MemoryID(memoryID), startAddress) &&
			    addressValidator(MemoryManagementService::MemoryID(memoryID), startAddress + readLength)) {
				for (std::size_t i = 0; i < readLength; i++) {
					readData[i] = *(reinterpret_cast<uint8_t*>(startAddress) + i);
				}

				report.append<StartAddress>(startAddress);
				report.append<MemoryDataLength>(readLength);
				report.appendBits(16, CRCHelper::calculateCRC(readData, readLength));
			} else {
				ErrorHandler::reportError(request, ErrorHandler::AddressOutOfRange);
			}
		}

		mainService.storeMessage(report);
		request.resetRead();
	} else {
		// TODO: Send a failed start of execution report
	}
}

bool MemoryManagementService::addressValidator(MemoryManagementService::MemoryID memId, uint64_t address) {
	bool validIndicator = false;

	switch (memId) {
		case MemoryManagementService::MemoryID::DTCMRAM:
			if ((address >= DTCMRAMLowerLim) && (address <= DTCMRAMUpperLim)) {
				validIndicator = true;
			}
			break;
		case MemoryManagementService::MemoryID::ITCMRAM:
			if ((address >= ITCMRAMLowerLim) && (address <= ITCMRAMUpperLim)) {
				validIndicator = true;
			}
			break;
		case MemoryManagementService::MemoryID::RAM_D1:
			if ((address >= RAMD1LowerLim) && (address <= RAMD1UpperLim)) {
				validIndicator = true;
			}
			break;
		case MemoryManagementService::MemoryID::RAM_D2:
			if ((address >= RAMD2LowerLim) && (address <= RAMD2UpperLim)) {
				validIndicator = true;
			}
			break;
		case MemoryManagementService::MemoryID::RAM_D3:
			if ((address >= RAMD3LowerLim) && (address <= RAMD3UpperLim)) {
				validIndicator = true;
			}
			break;
		case MemoryManagementService::MemoryID::FLASH_MEMORY:
			if ((address >= FlashLowerLim) && (address <= FlashUpperLim)) {
				validIndicator = true;
			}
			break;

		default:
			validIndicator = true; // TODO: Implemented so addresses from PC can be read. Remove.
			break;
	}

	return validIndicator;
}

inline bool MemoryManagementService::memoryIdValidator(MemoryManagementService::MemoryID memId) {
	return (memId == MemoryManagementService::MemoryID::RAM_D1) ||
	       (memId == MemoryManagementService::MemoryID::RAM_D2) ||
	       (memId == MemoryManagementService::MemoryID::RAM_D3) ||
	       (memId == MemoryManagementService::MemoryID::DTCMRAM) ||
	       (memId == MemoryManagementService::MemoryID::ITCMRAM) ||
	       (memId == MemoryManagementService::MemoryID::FLASH_MEMORY) ||
	       (memId == MemoryManagementService::MemoryID::EXTERNAL);
}

inline bool MemoryManagementService::dataValidator(const uint8_t* data, MemoryManagementChecksum checksum, MemoryDataLength length) {
	return (checksum == CRCHelper::calculateCRC(data, length));
}

void MemoryManagementService::execute(Message& message) {
	switch (message.messageType) {
		case LoadRawMemoryDataAreas:
			loadRawData(message);
			break;
		case DumpRawMemoryData:
			rawDataMemorySubservice.dumpRawData(message);
			break;
		case CheckRawMemoryData:
			rawDataMemorySubservice.checkRawData(message);
			break;
		default:
			ErrorHandler::reportInternalError(ErrorHandler::OtherMessageType);
	}
}

#endif
