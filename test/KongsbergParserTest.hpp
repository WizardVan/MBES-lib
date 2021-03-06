/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   KongsbergParserTest.hpp
 * Author: emile
 *
 * Created on May 3, 2019, 2:02 PM
 */
#include "catch.hpp"
#include "../src/datagrams/DatagramEventHandler.hpp"
#include "../src/datagrams/kongsberg/KongsbergParser.hpp"
#include <math.h>

#pragma pack(1)

typedef struct {
    uint16_t numAttitudeEntries; //number of packets
    KongsbergAttitudeEntry attitude;
} KongsbergAttitudePacket;
#pragma pack()

TEST_CASE("test the function KongsbergParser::getName") {
    DatagramEventHandler handler;
    KongsbergParser parser(handler);
    REQUIRE(parser.getName(48) == "PU Id output datagrams");
    REQUIRE(parser.getName(49) == "PU Status output");
    REQUIRE(parser.getName(51) == "ExtraParameters 3");
    REQUIRE(parser.getName(53) == "Seabed image datagram");
    REQUIRE(parser.getName(65) == "Attitude datagram");
    REQUIRE(parser.getName(66) == "PU BIST result output");
    REQUIRE(parser.getName(67) == "Clock datagrams");
    REQUIRE(parser.getName(68) == "Depth datagram");
    REQUIRE(parser.getName(69) == "Single beam echo sounder depth datagram");
    REQUIRE(parser.getName(70) == "Raw range and beam angle datagrams");
    REQUIRE(parser.getName(71) == "Surface sound speed datagram");
    REQUIRE(parser.getName(72) == "Heading datagrams");
    REQUIRE(parser.getName(73) == "Installation parameters");
    REQUIRE(parser.getName(74) == "Mechanical transducer tilt datagrams");
    REQUIRE(parser.getName(75) == "Central beams echogram");
    REQUIRE(parser.getName(78) == "Raw range and beam angle 78 datagram");
    REQUIRE(parser.getName(79) == "Quality factor datagram 79");
    REQUIRE(parser.getName(80) == "Position datagrams");
    REQUIRE(parser.getName(82) == "Runtime parameters");
    REQUIRE(parser.getName(84) == "Tide datagram");
    REQUIRE(parser.getName(85) == "Sound speed profile datagram");
    REQUIRE(parser.getName(87) == "Kongsberg Maritime SSP output datagram");
    REQUIRE(parser.getName(88) == "XYZ 88");
    REQUIRE(parser.getName(89) == "Seabed image data 89 datagram");
    REQUIRE(parser.getName(102) == "Raw range and beam angle datagrams");
    REQUIRE(parser.getName(104) == "Depth (pressure) or height datagram");
    REQUIRE(parser.getName(105) == "Installation parameters");
    REQUIRE(parser.getName(107) == "Water column datagram");
    REQUIRE(parser.getName(108) == "Extra detections");
    REQUIRE(parser.getName(110) == "Network attitude velocity datagram 110");
    REQUIRE(parser.getName(114) == "Installation parameters or remote information");
    REQUIRE(parser.getName(115) == "Invalid tag");
}

TEST_CASE("test the Kongsberg parser with a file who doesn't exist") {
    DatagramEventHandler handler;
    KongsbergParser parser(handler);
    std::string file("blabla.all");
    std::string excep = "";
    try {
        parser.parse(file);
        REQUIRE(false);
    } catch (Exception * error) {
        excep = error->what();
        REQUIRE(true);
    }
}

TEST_CASE("test the Kongsberg parser with a invalid datagram") {
    DatagramEventHandler handler;
    KongsbergParser parser(handler);
    std::string file("test.txt");
    std::string excep = "";
    try {
        parser.parse(file);
        REQUIRE(false);
    } catch (Exception * error) {
        excep = error->what();
        REQUIRE(true);
    }
}

TEST_CASE("test the Kongsberg parser with a valid datagram") {
    DatagramEventHandler handler;
    KongsbergParser parser(handler);
    std::string file("test/data/all/0008_20160909_135801_Panopee.all");
    std::string excep = "";
    try {
        parser.parse(file);
        REQUIRE(true);
    } catch (Exception * error) {
        excep = error->what();
        REQUIRE(false);
    }
}

TEST_CASE("test the Kongsberg parser convertTime method") {
    
    static long datagramDate = 20200106;
    static long datagramTime = 3600*1000;
    
    // From https://www.epochconverter.com/
    static long testEpochMicro = 1578272400000*1000;
    
    class KongsbergParserTester : public KongsbergParser {
    public:

        KongsbergParserTester(DatagramEventHandler & processor) : KongsbergParser(processor) {

        }

        void testProcessConvertTime() {
            long epochMicro = convertTime(datagramDate, datagramTime);
            REQUIRE(epochMicro == testEpochMicro);
        }
    };
    
    DatagramEventHandler handler;
    KongsbergParserTester tester(handler);

    tester.testProcessConvertTime();
}

TEST_CASE("test Kongsberg parser decoding") {

    //expected result of attitude extraction
    static double testHeading = 0.3;
    static double testPitch = 0.4;
    static double testRoll = 0.5;

    //representation of attitude in datagram
    static int16_t headingData = 30;
    static int16_t pitchData = 40;
    static int16_t rollData = 50;

    //Set date time to 2020-01-06 01:00:00
    static uint32_t year = 2020;
    static uint32_t month = 1;
    static uint32_t day = 6;
    static uint32_t millisSinceMidnight = 3600 * 1000; // 1 am
    
    //expected result of position extraction
    static double testLatitude = 48.3533333;
    static double testLongitude = -65.825;
    static double testOrthometricHeight = 18.893;
    static double testGeoidSeparation = -25.669;
    static double testHeight = testGeoidSeparation + testOrthometricHeight;
    
    //representation of attitude in datagram
    static int32_t latitudeData = testLatitude*LAT_FACTOR;
    static int32_t longitudeData = testLongitude*LON_FACTOR;
    static std::string nmeaPosition = "$GPGGA,,,,,,,,,18.893,M,-25.669,M,,*00";

    

    // From https://www.epochconverter.com/
    static uint64_t testMicroEpoch = 1578272400000 * 1000; // 2020-01-06 01:00:00 GMT
    static uint64_t testAttitudeTimestamp = 45 * 1000; // 45 milliseconds

    class KongsbergDatagramEventHandlerTester : public DatagramEventHandler {

        void processAttitude(uint64_t microEpoch, double heading, double pitch, double roll) {
            REQUIRE(microEpoch == testMicroEpoch + testAttitudeTimestamp);

            double angleTreshold = 1e-12;
            REQUIRE(abs(heading - testHeading) < angleTreshold);
            REQUIRE(abs(pitch - testPitch) < angleTreshold);
            REQUIRE(abs(roll - testRoll) < angleTreshold);
        }

        void processPosition(uint64_t microEpoch, double longitude, double latitude, double height) {
            REQUIRE(microEpoch == testMicroEpoch);
            double angleTreshold = 1e-12;
            REQUIRE(abs(longitude - testLongitude) < angleTreshold);
            REQUIRE(abs(latitude - testLatitude) < angleTreshold);
            
            double heightTreshold = 1e-9;
            REQUIRE(abs(height - testHeight) < heightTreshold);
        }

        void processPing(uint64_t microEpoch, long id, double beamAngle, double tiltAngle, double twoWayTravelTime, uint32_t quality, int32_t intensity) {

        }

        void processSwathStart(double surfaceSoundSpeed) {

        }

        void processSoundVelocityProfile(SoundVelocityProfile * svp) {

        }
    };

    class KongsbergParserTester : public KongsbergParser {
    public:

        KongsbergParserTester(DatagramEventHandler & processor) : KongsbergParser(processor) {

        }

        void testProcessAttitudeDatagram() {
            KongsbergHeader hdr = {0};

            //static values defined earlier in test case
            hdr.date = 10000 * year + 100 * month + day;
            hdr.time = millisSinceMidnight;

            uint16_t numAttitudeEntries = 1;

            KongsbergAttitudeEntry attitude = {0};
            attitude.deltaTime = 45; // milliseconds
            attitude.heading = headingData; // in 0.01 degrees as int16_t
            attitude.pitch = pitchData; // in 0.01 degrees as int16_t
            attitude.roll = rollData; // in 0.01 degrees as int16_t

            unsigned char * datagram = new unsigned char[ sizeof (KongsbergAttitudePacket) / sizeof (unsigned char) ];
            KongsbergAttitudePacket * attPacket = new (datagram) KongsbergAttitudePacket();
            
            attPacket->numAttitudeEntries = numAttitudeEntries;
            attPacket->attitude = attitude;

            processAttitudeDatagram(hdr, datagram);

            delete datagram;
        }
        
        void testProcessPositionDatagram() {
            KongsbergHeader hdr = {0};

            //static values defined earlier in test case
            hdr.date = 10000 * year + 100 * month + day;
            hdr.time = millisSinceMidnight;

            //TODO: Test position datagram decoding
        }
    };

    KongsbergDatagramEventHandlerTester handler;
    KongsbergParserTester tester(handler);

    tester.testProcessAttitudeDatagram();

}
