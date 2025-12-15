#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <windows.h>

#include "ec.hpp"
#include "driver.hpp"

EmbeddedController::EmbeddedController(
    BYTE scPort,
    BYTE dataPort,
    BYTE endianness,
    UINT16 retry,
    UINT16 timeout)
{
    this->scPort = scPort;
    this->dataPort = dataPort;
    this->endianness = endianness;
    this->retry = retry;
    this->timeout = timeout;

    this->driver = Driver();
    this->driverFileExist = driver.driverFileExist;

    if (this->driver.initialize())
        this->driverLoaded = TRUE;
}

VOID EmbeddedController::close()
{
    this->driver.deinitialize();
    this->driverLoaded = FALSE;
}

EC_DUMP EmbeddedController::dump()
{
    EC_DUMP _dump;
    for (UINT16 column = 0x00; column <= 0xF0; column += 0x10)
        for (UINT16 row = 0x00; row <= 0x0F; row++)
        {
            UINT16 address = column + row;
            _dump.insert(std::pair<BYTE, BYTE>(address, this->readByte(address)));
        }

    return _dump;
}

VOID EmbeddedController::printDump()
{
    std::stringstream stream;
    stream << std::hex << std::uppercase << std::setfill('0')
           << " # | 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F" << std::endl
           << "---|------------------------------------------------" << std::endl
           << "00 | ";

    for (auto const &[address, value] : this->dump())
    {
        UINT16 nextAddress = address + 0x01;
        stream << std::setw(2) << (UINT16)value << " ";
        if (nextAddress % 0x10 == 0x00) // End of row
            stream << std::endl
                   << nextAddress << " | ";
    }

    std::string result = stream.str();
    std::cout << std::endl
              << result.substr(0, result.size() - 7) // Removing last 7 characters
              << std::endl;
}

VOID EmbeddedController::saveDump(std::string output)
{
    std::ofstream file(output, std::ios::out | std::ios::binary);
    if (file)
    {
        for (auto const &[address, value] : this->dump())
            file << this->readByte(address);
        file.close();
    }
}

//BYTE EmbeddedController::readByte(BYTE bRegister)
//{
//    BYTE result = 0x00;
//    this->operation(READ, bRegister, &result);
//    return result;
//}

short int EmbeddedController::readByte(BYTE bRegister)
{
    BYTE result = 0x00;
    if (this->operation(READ, bRegister, &result))
        return result;
    return -1;
}


BOOL EmbeddedController::writeByte(BYTE bRegister, BYTE value)
{
    return this->operation(WRITE, bRegister, &value);
}


BOOL EmbeddedController::operation(BYTE mode, BYTE bRegister, BYTE *value)
{
    BOOL isRead = mode == READ;
    BYTE operationType = isRead ? RD_EC : WR_EC;

    for (UINT16 i = 0; i < this->retry; i++)
        if (this->status(EC_IBF)) // Wait until IBF is free
        {
            this->driver.writeIoPortByte(this->scPort, operationType); // Write operation type to the Status/Command port
            if (this->status(EC_IBF))                                  // Wait until IBF is free
            {
                this->driver.writeIoPortByte(this->dataPort, bRegister); // Write register address to the Data port
                if (this->status(EC_IBF))                                // Wait until IBF is free
                    if (isRead)
                    {
                        if (this->status(EC_OBF)) // Wait until OBF is full
                        {
                            *value = this->driver.readIoPortByte(this->dataPort); // Read from the Data port
                            return TRUE;
                        }
                    }
                    else
                    {
                        this->driver.writeIoPortByte(this->dataPort, *value); // Write to the Data port
                        return TRUE;
                    }
            }
        }

    return FALSE;
}

BOOL EmbeddedController::status(BYTE flag)
{
    BOOL done = flag == EC_OBF ? 0x01 : 0x00;
    for (UINT16 i = 0; i < this->timeout; i++)
    {
        BYTE result = this->driver.readIoPortByte(this->scPort);
        // First and second bit of returned value represent
        // the status of OBF and IBF flags respectively
        if (((done ? ~result : result) & flag) == 0)
            return TRUE;
    }

    return FALSE;
}



/// ³õÊ¼»¯ EC¡£
void EmbeddedController::EC_init()
{
    byte EC_CHIP_ID1 = DirectECRead(0x2000);
    if (EC_CHIP_ID1 == 0x55)
    {
        byte val = DirectECRead(0x1060);
        val = (byte)(val | 0x80);
        DirectECWrite(0x1060, val); // enable EC RAM
    }
}

//void DirectECWrite(uint16_t Addr, BYTE data, BYTE EC_ADDR_PORT = 0x4e, BYTE EC_DATA_PORT = 0x4f);
void EmbeddedController::DirectECWrite(uint16_t Addr, BYTE data, BYTE EC_ADDR_PORT, BYTE EC_DATA_PORT) {
    this->driver.writeIoPortByte(EC_ADDR_PORT, 0x2E);
    this->driver.writeIoPortByte(EC_DATA_PORT, 0x11);
    this->driver.writeIoPortByte(EC_ADDR_PORT, 0x2F);
    this->driver.writeIoPortByte(EC_DATA_PORT, static_cast<BYTE>((Addr >> 8) & 0xFF));
    
    this->driver.writeIoPortByte(EC_ADDR_PORT, 0x2E);
    this->driver.writeIoPortByte(EC_DATA_PORT, 0x10);
    this->driver.writeIoPortByte(EC_ADDR_PORT, 0x2F);
    this->driver.writeIoPortByte(EC_DATA_PORT, static_cast<BYTE>(Addr & 0xFF));
    
    this->driver.writeIoPortByte(EC_ADDR_PORT, 0x2E);
    this->driver.writeIoPortByte(EC_DATA_PORT, 0x12);
    this->driver.writeIoPortByte(EC_ADDR_PORT, 0x2F);
    this->driver.writeIoPortByte(EC_DATA_PORT, data);
}
//BYTE DirectECRead(uint16_t Addr, BYTE EC_ADDR_PORT = 0x4e, BYTE EC_DATA_PORT = 0x4f);
BYTE EmbeddedController::DirectECRead(uint16_t Addr, BYTE EC_ADDR_PORT, BYTE EC_DATA_PORT) {
    this->driver.writeIoPortByte(EC_ADDR_PORT, 0x2E);
    this->driver.writeIoPortByte(EC_DATA_PORT, 0x11);
    this->driver.writeIoPortByte(EC_ADDR_PORT, 0x2F);
    this->driver.writeIoPortByte(EC_DATA_PORT, static_cast<BYTE>((Addr >> 8) & 0xFF));
    
    this->driver.writeIoPortByte(EC_ADDR_PORT, 0x2E);
    this->driver.writeIoPortByte(EC_DATA_PORT, 0x10);
    this->driver.writeIoPortByte(EC_ADDR_PORT, 0x2F);
    this->driver.writeIoPortByte(EC_DATA_PORT, static_cast<BYTE>(Addr & 0xFF));
    
    this->driver.writeIoPortByte(EC_ADDR_PORT, 0x2E);
    this->driver.writeIoPortByte(EC_DATA_PORT, 0x12);
    this->driver.writeIoPortByte(EC_ADDR_PORT, 0x2F);
    return this->driver.readIoPortByte(EC_DATA_PORT);
}