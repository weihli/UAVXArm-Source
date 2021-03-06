// ===============================================================================================
// =                                UAVX Quadrocopter Controller                                 =
// =                           Copyright (c) 2008 by Prof. Greg Egan                             =
// =                 Original V3.15 Copyright (c) 2007 Ing. Wolfgang Mahringer                   =
// =                     http://code.google.com/p/uavp-mods/ http://uavp.ch                      =
// ===============================================================================================

//    This is part of UAVX.

//    UAVX is free software: you can redistribute it and/or modify it under the terms of the GNU 
//    General Public License as published by the Free Software Foundation, either version 3 of the 
//    License, or (at your option) any later version.

//    UAVX is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY; without
//    even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
//    See the GNU General Public License for more details.

//    You should have received a copy of the GNU General Public License along with this program.  
//    If not, see http://www.gnu.org/licenses/

#include "UAVX.h"

#define FRSKY_V2_HUB
//#define FRSKY_CLEANFLIGHT

#if defined(FRSKY_V2_HUB)

/// FrSky Telemetry implementation by slipstream @ rcgroups rewritten by gke

#define FS_SENTINEL       0x5e

enum {
	// Data IDs  (BP = before decimal point; AP = after decimal point)

	FS_ID_GPS_ALT_BP = 0x01,
	FS_ID_TEMP1 = 0x02, // barometer deg C
	FS_ID_RPM = 0x03,
	FS_ID_FUEL = 0x04,
	FS_ID_TEMP2 = 0x05,
	FS_ID_VOLTS = 0x06, // cells

	// 0x07
	// 0x08

	FS_ID_GPS_ALT_AP = 0x09,
	FS_ID_BARO_ALT_BP = 0x10,

	// 0x0A
	// 0x0B
	// 0x0C
	// 0x0D
	// 0x0E
	// 0x0F // seems to be emitted when there is a buffer overrun in the Rx.

	FS_ID_GPS_SPEED_BP = 0x11,
	FS_ID_GPS_LONG_BP = 0x12,
	FS_ID_GPS_LAT_BP = 0x13,
	FS_ID_GPS_COURS_BP = 0x14,

	FS_ID_GPS_DAY_MONTH = 0x15,
	FS_ID_GPS_YEAR = 0x16,
	FS_ID_GPS_HOUR_MIN = 0x17,
	FS_ID_GPS_SEC = 0x18,

	FS_ID_GPS_SPEED_AP = 0x19, // +8 from BP
	FS_ID_GPS_LONG_AP = 0x1A,
	FS_ID_GPS_LAT_AP = 0x1B,
	FS_ID_GPS_COURS_AP = 0x1C,

	UAVX_ID_GPS_STAT = 0x1d,

	// 0x1e

	// 0x1f
	// 0x20

	FS_ID_BARO_ALT_AP = 0x21,

	FS_ID_GPS_LONG_EW = 0x22,
	FS_ID_GPS_LAT_NS = 0x23,

	FS_ID_ACCEL_X = 0x24, // m/s2
	FS_ID_ACCEL_Y = 0x25,
	FS_ID_ACCEL_Z = 0x26,

	FS_ID_CURRENT = 0x28,

	UAVX_ID_WHERE_DIST = 0x29, // metres the aircraft is way
	UAVX_ID_WHERE_BEAR = 0x2a, // bearing (deg) to aircraft
	UAVX_ID_WHERE_ELEV = 0x2b, // elevation (deg) of the aircraft above the horizon
	UAVX_ID_WHERE_HINT = 0x2c, // which to turn to come home intended for voice guidance

	UAVX_ID_COMPASS = 0x2d, // deg
	// 0x2e
	// 0x2f

	FS_ID_VARIO = 0x30, // cm/sec

	//--------------------

	// UAVX user defined

	UAVX_ID_GYRO_X = 0x31, // deg/sec
	UAVX_ID_GYRO_Y = 0x32,
	UAVX_ID_GYRO_Z = 0x33,

	UAVX_ID_PITCH = 0x34, // deg
	UAVX_ID_ROLL = 0x35,

	UAVX_ID_MAH = 0x36, // mAH battery consumption

	//--------------------

	FS_ID_VFAS = 0x39,
	FS_ID_VOLTS_BP = 0x3A,
	FS_ID_VOLTS_AP = 0x3B,
	FS_ID_FRSKY_LAST = 0x3F,

};

uint16 MakeFrac(real32 v, uint16 s) {
	return (Abs((int32)(v * s)) % s);
} // MakeFrac

void TxFrSkyHeader(uint8 s) {
	TxChar(s, FS_SENTINEL);
} // TxFrSkyHeader

void TxFrSky(uint8 s, uint8 data) {
	// byte stuffing
	if (data == 0x5e) {
		TxChar(s, 0x5d);
		TxChar(s, 0x3e);
	} else if (data == 0x5d) {
		TxChar(s, 0x5d);
		TxChar(s, 0x3d);
	} else
		TxChar(s, data);
} // TxFrSky

void TxFrSky16(uint8 s, int16 a) {
	TxFrSky(s, a);
	TxFrSky(s, (a >> 8) & 0xff);
} // TxFrSky16


void TxFrSkyPacket(uint8 s, uint8 id, int16 v) {
	TxFrSkyHeader(s);
	TxFrSky(s, id);
	TxFrSky16(s, v);
} // TxFrSkyPacket

void TxFrSkyPacketPair(uint8 s, uint8 id1, uint8 id2, int16 v, int8 frac) {
	TxFrSkyPacket(s, id1, v);
	TxFrSkyPacket(s, id2, MakeFrac(v, frac));
}

void TxFrSkyAcc(uint8 s) {
	uint8 a;

	for (a = X; a <= Z; a++)
		TxFrSkyPacket(s, FS_ID_ACCEL_X + a, Acc[a] * GRAVITY_MPS_S_R * 1000.0f);
} // TxFrSkyAcc

void TxFrSkyGyro(uint8 s) {
	uint8 a;

	for (a = Pitch; a <= Yaw; a++)
		TxFrSkyPacket(s, UAVX_ID_GYRO_X + a, RadiansToDegrees(Rate[a]));
} // TxFrSkyAcc

void TxFrSkyAttitude(uint8 s) {
	uint8 a;

	for (a = Pitch; a <= Roll; a++)
		TxFrSkyPacket(s, UAVX_ID_PITCH + a, RadiansToDegrees(A[a].Angle));

} // TxFrSkyAttitude

void TxFrSkyBaro(uint8 s) {

	TxFrSkyPacketPair(s, FS_ID_BARO_ALT_BP, FS_ID_BARO_ALT_AP, BaroAltitude
			- OriginAltitude, 10);
} //  TxFrSkyBaro

void TxFrSkyVario(uint8 s) {
	TxFrSkyPacket(s, FS_ID_VARIO, ROC * 100.0f);
} //  TxFrSkyBaro

void TxFrSkyTemperature1(uint8 s) {
	TxFrSkyPacket(s, FS_ID_TEMP1, BaroTemperature);
} // TxFrSkyTemperature1

void TxFrSkyFuel(uint8 s) {
	TxFrSkyPacket(
			s,
			FS_ID_FUEL,
			Limit(100 * ( 1.0f - BatteryChargeUsedmAH / BatteryCapacitymAH), 0, 100));
} // TxFrSkyFuel

void TxFrSkymAH(uint8 s) {
	TxFrSkyPacket(s, UAVX_ID_MAH, BatteryChargeUsedmAH);
} // TxFrSkymAH

void TxFrSkyTemperature2(uint8 s) {
	TxFrSkyPacket(s, FS_ID_TEMP2, MPU6XXXTemperature);
} // TxFrSkyTemperature2

void TxFrSkyTime(uint8 s) {
	uint32 seconds = mSClock() / 1000;
	uint8 minutes = (seconds / 60) % 60;

	// if we fly for more than an hour, something's wrong anyway
	TxFrSkyPacket(s, FS_ID_GPS_HOUR_MIN, minutes << 8);
	TxFrSkyPacket(s, FS_ID_GPS_SEC, seconds % 60);
} // TxFrSkyTime

void TxFrSkyWhere(uint8 s) {
	if ((Nav.Distance >= 0.0) && (Nav.Distance < 32000.0f)) {
		TxFrSkyPacket(s, UAVX_ID_WHERE_DIST, Nav.Distance);
		TxFrSkyPacket(s, UAVX_ID_WHERE_BEAR, RadiansToDegrees(Nav.Bearing));
		TxFrSkyPacket(s, UAVX_ID_WHERE_ELEV, RadiansToDegrees(Nav.Elevation));
		TxFrSkyPacket(s, UAVX_ID_WHERE_HINT, RadiansToDegrees(Nav.Hint));
	}
}


// FrSky uses NMEA form rather than computationally sensible decimal degrees

typedef struct {
	uint16 bp, ap;
} pair_rec;

static void GPStoDDDMM_MMMM(int32 L, pair_rec * c) {
	uint32 d, mf, dm, m;

	L = Abs(L);
	d = L / 10000000L;
	dm = (L % 10000000L) * 60;
	m = dm / 10000000L;
	mf = dm  -  m * 10000000L;

	c->bp = d * 100 + m;
	c->ap = mf / 1000L; // limited precision
}

void TxFrSkyGPSStat(uint8 s) {
	TxFrSkyPacket(s, UAVX_ID_GPS_STAT, GPS.noofsats * 1000 + GPS.fix * 100
			+ (F.GPSValid & 1) * 10 + (F.OriginValid & 1));
} // TxFrSkyGPSStat

void TxFrSkyGPSCoords(uint8 s) {
	pair_rec c;

	GPStoDDDMM_MMMM(GPS.C[NorthC].Raw, &c);
	TxFrSkyPacket(s, FS_ID_GPS_LAT_BP, c.bp);
	TxFrSkyPacket(s, FS_ID_GPS_LAT_AP, c.ap);
	TxFrSkyPacket(s, FS_ID_GPS_LAT_NS, GPS.C[NorthC].Raw < 0 ? 'S' : 'N');


	GPStoDDDMM_MMMM(GPS.C[EastC].Raw, &c);
	TxFrSkyPacket(s, FS_ID_GPS_LONG_BP, c.bp);
	TxFrSkyPacket(s, FS_ID_GPS_LONG_AP, c.ap);
	TxFrSkyPacket(s, FS_ID_GPS_LONG_EW, GPS.C[EastC].Raw < 0 ? 'W' : 'E');

} // TxFrSkyGPS

void TxFrSkyGPSSpeed(uint8 s) {
	TxFrSkyPacketPair(s, FS_ID_GPS_SPEED_BP, FS_ID_GPS_SPEED_AP, GPS.gspeed
			* 3.6f, 10);
} // TxFrSkyGPSSpeed

void TxFrSkyGPSAlt(uint8 s) {
	TxFrSkyPacketPair(s, FS_ID_GPS_ALT_BP, FS_ID_GPS_ALT_AP, GPS.altitude, 10);
} // TxFrSkyGPSAlt

void TxFrSkyCellVoltages(uint8 s) {

	static uint16 currentCell = 0;
	uint32 cellVoltage;
	uint16 payload;

	// A cell packet is formated this way: https://github.com/jcheger/frsky-arduino/blob/master/FrskySP/FrskySP.cpp
	// content    | length
	// ---------- | ------
	// volt[id]   | 12-bits
	// celltotal  | 4 bits
	// cellid     | 4 bits

	cellVoltage = (BatteryVolts * 500.0f) / BatteryCellCount;

	payload = ((cellVoltage & 0x0ff) << 8) | (currentCell << 4) | ((cellVoltage
			& 0xf00) >> 8);

	TxFrSkyPacket(s, FS_ID_VOLTS, payload);

	if (++currentCell >= BatteryCellCount)
		currentCell = 0;

} // TxFrSkyCellVoltages

void TxFrSkyVoltage(uint8 s) {
	TxFrSkyPacketPair(s, FS_ID_VOLTS_BP, FS_ID_VOLTS_AP, BatteryVolts * 0.5f,
			100);
} // TxFrSkyVoltage

void TxFrSkyCurrent(uint8 s) {
	TxFrSkyPacket(s, FS_ID_CURRENT, BatteryCurrent * 10);
} // TxFrSkyCurrent

void TxFrSkyGPSHeading(uint8 s) {

	TxFrSkyPacketPair(s, FS_ID_GPS_COURS_BP, FS_ID_GPS_COURS_AP,
			RadiansToDegrees(GPS.heading), 10);
	//RadiansToDegrees(Heading), 10);

} // TxFrSkyGPSHeading

void TxFrSkyCompassHeading(uint8 s) {

	TxFrSkyPacket(s, UAVX_ID_COMPASS, RadiansToDegrees(Heading));
} // TxFrSkyCompassHeading


void SendFrSkyTelemetry(uint8 s) {
	static uint8 FrameCount = 0;

	if (++FrameCount == 40) { // FRAME 3 every 8 seconds
		TxFrSkyTime(s); // 2
		TxFrSkyTemperature1(s); // 1
		//TxFrSkyTemperature2(s); // 1
		//TxFrSkyFuel(s);
		TxChar(s, FS_SENTINEL);

		FrameCount = 0;

	} else if ((FrameCount % 5) == 0) { // FRAME 2 every second
		if (F.GPSValid) {
			if (F.OriginValid)
				TxFrSkyWhere(s); // 4
			TxFrSkyGPSSpeed(s); // 2
			TxFrSkyGPSAlt(s); // 2
			TxFrSkyGPSHeading(s); // 2
			TxFrSkyGPSCoords(s); // 6
			TxChar(s, FS_SENTINEL);
		}
		TxFrSkyGPSStat(s); // 1
		//TxFrSkyCompassHeading(s); // 2 2-> 17
	} else { // FRAME 1 every 200mS
		TxFrSkyBaro(s); // 2
		TxFrSkyVario(s); // 1
		TxFrSkyVoltage(s); // 2
		TxFrSkyCellVoltages(s); // 1
		TxFrSkyCurrent(s); // 1
		TxFrSkymAH(s); // 1
		//TxFrSkyAcc(s); // 3 could add for coordinated turns?
		TxFrSkyGyro(s); // 3
		TxFrSkyAttitude(s); // 2 ~ 14
		TxChar(s, FS_SENTINEL);
	}

} // TxFrSkyTelemetry

#elif defined(FRSKY_CLEANFLIGHT)
//________________________________________________________________________

// Taranis SPort Port based on CleanFlight implementation

enum {
	FSSP_START_STOP = 0x7E, FSSP_DATA_FRAME = 0x10,

	// ID of sensor. Must be something that is polled by FrSky RX
	FSSP_SENSOR_ID1 = 0x1B,
	FSSP_SENSOR_ID2 = 0x0D,
	FSSP_SENSOR_ID3 = 0x34,
	FSSP_SENSOR_ID4 = 0x67,
	// there are 32 ID's polled by smartport master
	// remaining 3 bits are crc (according to comments in openTx code)
};

// these data identifiers are obtained from https://github.com/opentx/opentx/blob/master/radio/src/telemetry/frsky.h
enum {
	FSSP_ID_SPEED = 0x0830,
	FSSP_ID_VFAS = 0x0210,
	FSSP_ID_CURRENT = 0x0200,
	FSSP_ID_RPM = 0x050F,
	FSSP_ID_ALTITUDE = 0x0100,
	FSSP_ID_FUEL = 0x0600,
	FSSP_ID_ADC1 = 0xF102,
	FSSP_ID_ADC2 = 0xF103,
	FSSP_ID_LATLONG = 0x0800,
	FSSP_ID_CAP_USED = 0x0600,
	FSSP_ID_VARIO = 0x0110,
	FSSP_ID_CELLS = 0x0300,
	FSSP_ID_CELLS_LAST = 0x030F,
	FSSP_ID_HEADING = 0x0840,
	FSSP_ID_ACCX = 0x0700,
	FSSP_ID_ACCY = 0x0710,
	FSSP_ID_ACCZ = 0x0720,
	FSSP_ID_T1 = 0x0400,
	FSSP_ID_T2 = 0x0410,
	FSSP_ID_GPS_ALT = 0x0820,
	FSSP_ID_A3 = 0x0900,
	FSSP_ID_A4 = 0x0910,

	FSSP_ID_RSSI = 0xf101,
	//	FSSP_ID_ADC1 = 0xf102,
	//	FSSP_ID_ADC2 = 0xf103,
	FSSP_ID_BATT = 0xf104,
	FSSP_ID_SWR = 0xf105,
	FSSP_ID_T1_FIRST = 0x0400,
	FSSP_ID_T1_LAST = 0x040f,
	FSSP_ID_T2_FIRST = 0x0410,
	FSSP_ID_T2_LAST = 0x041f,
	FSSP_ID_RPM_FIRST = 0x0500,
	FSSP_ID_RPM_LAST = 0x050f,
	FSSP_ID_FUEL_FIRST = 0x0600,
	FSSP_ID_FUEL_LAST = 0x060f,
	FSSP_ID_ALT_FIRST = 0x0100,
	FSSP_ID_ALT_LAST = 0x010f,
	FSSP_ID_VARIO_FIRST = 0x0110,
	FSSP_ID_VARIO_LAST = 0x011f,
	FSSP_ID_ACCX_FIRST = 0x0700,
	FSSP_ID_ACCX_LAST = 0x070f,
	FSSP_ID_ACCY_FIRST = 0x0710,
	FSSP_ID_ACCY_LAST = 0x071f,
	FSSP_ID_ACCZ_FIRST = 0x0720,
	FSSP_ID_ACCZ_LAST = 0x072f,
	FSSP_ID_CURR_FIRST = 0x0200,
	FSSP_ID_CURR_LAST = 0x020f,
	FSSP_ID_VFAS_FIRST = 0x0210,
	FSSP_ID_VFAS_LAST = 0x021f,
	FSSP_ID_CELLS_FIRST = 0x0300,
	//	FSSP_ID_CELLS_LAST = 0x030f,
	FSSP_ID_GPS_LONG_LATI_FIRST = 0x0800,
	FSSP_ID_GPS_LONG_LATI_LAST = 0x080f,
	FSSP_ID_GPS_ALT_FIRST = 0x0820,
	FSSP_ID_GPS_ALT_LAST = 0x082f,
	FSSP_ID_GPS_SPEED_FIRST = 0x0830,
	FSSP_ID_GPS_SPEED_LAST = 0x083f,
	FSSP_ID_GPS_COURS_FIRST = 0x0840,
	FSSP_ID_GPS_COURS_LAST = 0x084f,
	FSSP_ID_GPS_TIME_DATE_FIRST = 0x0850,
	FSSP_ID_GPS_TIME_DATE_LAST = 0x085f
};

// FrSky wrong IDs ?
//FS_BETA_VARIO_ID 0x8030
//FS_BETA_BARO_ALT_ID 0x8010

typedef uint8 portOptions; //zzz

enum {
	SPSTATE_UNINITIALIZED,
	SPSTATE_INITIALIZED,
	SPSTATE_WORKING,
	SPSTATE_TIMEDOUT,
};

const uint16 frSkyDataIdTable[] = {FSSP_ID_SPEED, FSSP_ID_VFAS,
	FSSP_ID_CURRENT,
	//FSSP_ID_RPM       ,
	FSSP_ID_ALTITUDE,
	FSSP_ID_FUEL,
	//FSSP_ID_ADC1      ,
	//FSSP_ID_ADC2      ,
	FSSP_ID_LATLONG,
	FSSP_ID_LATLONG, // twice
	//FSSP_ID_CAP_USED  ,
	FSSP_ID_VARIO, FSSP_ID_CELLS,
	//FSSP_ID_CELLS_LAST,
	FSSP_ID_HEADING, FSSP_ID_ACCX, FSSP_ID_ACCY, FSSP_ID_ACCZ, FSSP_ID_T1,
	FSSP_ID_T2, FSSP_ID_GPS_ALT,
	//FSSP_ID_A3	  ,
	FSSP_ID_A4, 0};

#define SMARTPORT_BAUD 57600
#define SMARTPORT_UART_MODE MODE_RXTX
#define SMARTPORT_SERVICE_TIMEOUT_US 1000 // max allowed time to find a value to send
#define SMARTPORT_NOT_CONNECTED_TIMEOUT_US 7000000L

static uint8 portConfig;

static boolean FrSkySPortTelemetryEnabled = false;
static boolean FrSkySPortPortSharing;

char FrSkySPortState = SPSTATE_UNINITIALIZED;
static uint8 FrSkySPortHasRequest = false;
static uint8 FrSkySPortIdCnt = 0;
static uint32 FrSkySPortLastRequestTime = 0;

void FrSkySPortDataReceive(uint8 s, uint16 c) {
	uint32 Now = mSClock();

	// look for a valid request sequence
	static uint8 lastChar;
	if (lastChar == FSSP_START_STOP) {
		FrSkySPortState = SPSTATE_WORKING;
		if (c == FSSP_SENSOR_ID1 && !serialAvailable(s)) {
			FrSkySPortLastRequestTime = Now;
			FrSkySPortHasRequest = true;
			// we only respond to these IDs
			// the X4R-SB does send other IDs, we ignore them, but take note of the time
		}
	}
	lastChar = c;
}

void FrSkySPortSendByte(uint8 s, uint8 c, uint16 *crcp) {
	// smart port escape sequence
	if (c == 0x7D || c == 0x7E) {
		TxChar(s, 0x7D);
		c ^= 0x20;
	}

	TxChar(s, c);

	if (crcp != NULL) {
		uint16 crc = *crcp;
		crc += c;
		crc += crc >> 8;
		crc &= 0x00FF;
		*crcp = crc;
	}
}

void TxFrSkySPort(uint8 s, uint16 id, uint32 val) {
	uint16 crc = 0;

	FrSkySPortSendByte(s, FSSP_DATA_FRAME, &crc);
	uint8 *u8p = (uint8*) &id;
	FrSkySPortSendByte(s, u8p[0], &crc);
	FrSkySPortSendByte(s, u8p[1], &crc);
	u8p = (uint8*) &val;
	FrSkySPortSendByte(s, u8p[0], &crc);
	FrSkySPortSendByte(s, u8p[1], &crc);
	FrSkySPortSendByte(s, u8p[2], &crc);
	FrSkySPortSendByte(s, u8p[3], &crc);
	FrSkySPortSendByte(s, 0xFF - (uint8) crc, NULL);
}

void initSPortPortTelemetry(void) {
	/*zzz
	 portConfig = 123; //findSerialPortConfig(FUNCTION_TELEMETRY_SMARTPORT);
	 FrSkySPortPortSharing = determinePortSharing(portConfig,
	 FUNCTION_TELEMETRY_SMARTPORT);
	 */
}

void freeSPortPortTelemetryPort(uint8 s) {
	/*zzz
	 closeSerialPort(s);
	 FrSkySPortSerial = NULL;

	 FrSkySPortState = SPSTATE_UNINITIALIZED;
	 FrSkySPortTelemetryEnabled = false;
	 */
}

void configureSPortPortTelemetryPort(uint8 s) {
	portOptions portOptions;

	if (portConfig) {

		/*
		 portOptions = SERIAL_BIDIR;

		 if (telemetryConfig()->telemetry_inversion)
		 portOptions |= SERIAL_INVERTED;

		 FrSkySPortSerial = openSerialPort(portConfig->identifier,
		 FUNCTION_TELEMETRY_SMARTPORT, NULL, SMARTPORT_BAUD,
		 SMARTPORT_UART_MODE, portOptions);
		 */
	}

	if (CurrTelType == FrSkyTelemetry) {
		FrSkySPortState = SPSTATE_INITIALIZED;
		FrSkySPortTelemetryEnabled = true;
		FrSkySPortLastRequestTime = mSClock();
	}
}

boolean canSendSPortPortTelemetry(void) {
	return (CurrTelType == FrSkyTelemetry) && (FrSkySPortState
			== SPSTATE_INITIALIZED || FrSkySPortState == SPSTATE_WORKING);
	return true;
}

boolean isSPortPortTimedOut(void) {
	return FrSkySPortState >= SPSTATE_TIMEDOUT;
}

boolean checkSPortPortTelemetryState(uint8 s) {
	boolean newTelemetryEnabledValue = true; //telemetryDetermineEnabledState(FrSkySPortPortSharing);

	if (newTelemetryEnabledValue == FrSkySPortTelemetryEnabled)
	return false;
	else {
		if (newTelemetryEnabledValue)
		configureSPortPortTelemetryPort(s);
		else
		freeSPortPortTelemetryPort(s);
		return true;
	}
}

void SendFrSkyTelemetry(uint8 s) {
	uint32 FrSkySPortLastServiceTime = uSClock();

	if (false) { // FrSkySPortTelemetryEnabled) {

		//if (canSendSPortPortTelemetry())
		//	while (serialAvailable(s)) {
		//		uint8 c = RxChar(s);
		//		FrSkySPortDataReceive(s, c);
		//	}

		uint32 Now = uSClock();

		// if timed out, reconfigure the UART back to normal so the GUI or CLI works
		if ((Now - FrSkySPortLastRequestTime)
				> SMARTPORT_NOT_CONNECTED_TIMEOUT_US)
		FrSkySPortState = SPSTATE_TIMEDOUT;
	} else {

		FrSkySPortHasRequest = true; // zzz

		while (FrSkySPortHasRequest) {
			// Ensure we won't get stuck in the loop if there happens to be nothing available to send in a timely manner - dump the slot if we loop in there for too long.
			if (false) //(uSClock() - FrSkySPortLastServiceTime) > SMARTPORT_SERVICE_TIMEOUT_US)
			FrSkySPortHasRequest = false;
			else {
				// we can send back any data we want, our table keeps track of the order and frequency of each data type we send

				static uint16 id = 0;

				id = frSkyDataIdTable[FrSkySPortIdCnt];
				if (id == 0) { // end of table reached, loop back
					FrSkySPortIdCnt = 0;
					id = frSkyDataIdTable[FrSkySPortIdCnt];
				}
				FrSkySPortIdCnt++;

				int32 tmpi;

				switch (id) {
					case FSSP_ID_SPEED:
					if (F.HaveGPS && F.GPSValid) {
						uint32 tmpui = (GPS.gspeed * 36 + 36 / 2) / 100;
						TxFrSkySPort(s, id, tmpui); // given in 0.1 m/s, provide in KM/H
						FrSkySPortHasRequest = false;
					}
					break;
					case FSSP_ID_VFAS:
					if (true) { // zzzfeature(FEATURE_VBAT)) {
						TxFrSkySPort(s, id, BatteryVolts * 10); // given in 0.1V, convert to volts
						FrSkySPortHasRequest = false;
					}
					break;
					case FSSP_ID_CELLS:
					if (true) //zzz feature(FEATURE_VBAT)
					//			&& telemetryConfig()->telemetry_send_cells)
					{
						/*
						 * A cell packet is formated this way: https://github.com/jcheger/frsky-arduino/blob/master/FrskySP/FrskySP.cpp
						 * content    | length
						 * ---------- | ------
						 * volt[id]   | 12-bits
						 * celltotal  | 4 bits
						 * cellid     | 4 bits
						 */
						static uint8 currentCell = 0; // Track current cell index number

						// Cells Data Payload
						uint32 payload = 0;
						payload |= ((uint16) (BatteryVolts * 100
										+ BatteryCellCount) / (BatteryCellCount * 2))
						& 0x0FFF; // Cell Voltage formatted for payload, modified code from cleanflight Frsky.c, TESTING NOTE: (uint16)(4.2 * 500.0) & 0x0FFF;
						payload <<= 4;
						payload |= (uint8) BatteryCellCount & 0x0F; // Cell Total Count formatted for payload
						payload <<= 4;
						payload |= (uint8) currentCell & 0x0F; // Current Cell Index Number formatted for payload

						// Send Payload
						TxFrSkySPort(s, id, payload);
						FrSkySPortHasRequest = false;

						// Incremental Counter
						currentCell++;
						currentCell %= BatteryCellCount; // Reset counter @ max index
					}
					break;
					case FSSP_ID_CURRENT:
					if (true) { //zzz feature(FEATURE_AMPERAGE_METER)) {
						//zzzamperageMeter *state = getAmperageMeter(
						//		batteryConfig()->amperageMeterSource);

						TxFrSkySPort(s, id, BatteryCurrent * 100); // given in 10mA steps, unknown requested unit
						FrSkySPortHasRequest = false;
					}
					break;
					//case FSSP_ID_RPM        :
					case FSSP_ID_ALTITUDE:
					if (F.BaroActive) {
						TxFrSkySPort(s, id, BaroAltitude); // unknown given unit, requested 100 = 1 meter
						FrSkySPortHasRequest = false;
					}
					break;
					case FSSP_ID_FUEL:
					if (true) { //zzzfeature(FEATURE_AMPERAGE_METER)) {
						/* zzz		amperageMeter *state = getAmperageMeter(
						 batteryConfig()->amperageMeterSource);
						 TxFrSkySPort(s, id, state->mAhDrawn); // given in mAh, unknown requested unit
						 */
						FrSkySPortHasRequest = false;
					}
					break;
					//case FSSP_ID_ADC1       :
					//case FSSP_ID_ADC2       :

					case FSSP_ID_LATLONG:
					if (F.HaveGPS && F.GPSValid) {
						uint32 tmpui = 0;
						// the same ID is sent twice, one for longitude, one for latitude
						// the MSB of the sent uint32 helps FrSky keep track
						// the even/odd bit of our counter helps us keep track
						if (FrSkySPortIdCnt & 1) {
							tmpui = abs(GPS.Raw[EastC]); // now we have unsigned value and one bit to spare
							tmpui = (tmpui + tmpui / 2) / 25 | 0x80000000; // 6/100 = 1.5/25, division by power of 2 is fast
							if (GPS.Raw[EastC] < 0)
							tmpui |= 0x40000000;
						} else {
							// now we have unsigned value and one bit to spare
							tmpui = (tmpui + tmpui / 2) / 25; // 6/100 = 1.5/25, division by power of 2 is fast
							if (GPS.Raw[NorthC] < 0)
							tmpui |= 0x40000000;
						}
						TxFrSkySPort(s, id, tmpui);
						FrSkySPortHasRequest = false;
					}
					break;

					//case FSSP_ID_CAP_USED:
					case FSSP_ID_VARIO:
					if (F.BaroActive) {
						TxFrSkySPort(s, id, ROC * 100); // unknown given unit but requested in 100 = 1m/s
						FrSkySPortHasRequest = false;
					}
					break;
					case FSSP_ID_HEADING:
					TxFrSkySPort(s, id, RadiansToDegrees(A[Yaw].Angle) * 10); // given in 10*deg, requested in 10000 = 100 deg
					FrSkySPortHasRequest = false;
					break;
					case FSSP_ID_ACCX:
					TxFrSkySPort(s, id, Acc[X] * 100);
					// unknown input and unknown output unit
					// we can only show 00.00 format, another digit won't display right on Taranis
					FrSkySPortHasRequest = false;
					break;
					case FSSP_ID_ACCY:
					TxFrSkySPort(s, id, Acc[Y] * 100);
					FrSkySPortHasRequest = false;
					break;
					case FSSP_ID_ACCZ:
					TxFrSkySPort(s, id, Acc[Z] * 100);
					FrSkySPortHasRequest = false;
					break;
					case FSSP_ID_T1:
					// we send all the flags as decimal digits for easy reading

					tmpi = 10000; // start off with at least one digit so the most significant 0 won't be cut off
					// the Taranis seems to be able to fit 5 digits on the screen
					// the Taranis seems to consider this number a signed 16 bit integer

					if ((State == Ready) || (State == Starting))
					tmpi += 1;
					if (State == Preflight)
					tmpi += 2;
					if (Armed())
					tmpi += 4;
					if (F.UsingAngleControl)
					tmpi += 10;
					if (!F.UsingAngleControl)
					tmpi += 20;
					if (F.Bypass)
					tmpi += 40;
					if (F.MagnetometerActive)
					tmpi += 100;
					if (F.BaroActive)
					tmpi += 200;
					if (F.RangefinderActive)
					tmpi += 400;
					if (NavState == HoldingStation)
					tmpi += 1000;
					if (NavState == ReturningHome)
					tmpi += 2000;
					//if (FLIGHT_MODE(HEADFREE_MODE))
					//tmpi += 4000;

					TxFrSkySPort(s, id, (uint32) tmpi);
					FrSkySPortHasRequest = false;
					break;
					case FSSP_ID_T2:
					if (F.HaveGPS) {
						// provide GPS lock status
						TxFrSkySPort(s, id, (F.GPSValid ? 1000 : 0)
								+ (F.OriginValid ? 2000 : 0) + GPS.noofsats);
						FrSkySPortHasRequest = false;
					} else if (F.HaveGPS) {
						TxFrSkySPort(s, id, 0);
						FrSkySPortHasRequest = false;
					}
					break;
					case FSSP_ID_GPS_ALT:
					if (F.HaveGPS && F.GPSValid) {
						TxFrSkySPort(s, id, GPS.altitude * 10); // given in 0.1m , requested in 10 = 1m (should be in mm, probably a bug in opentx, tested on 2.0.1.7)
						FrSkySPortHasRequest = false;
					}
					break;
					case FSSP_ID_A4:
					if (true) { //zzzfeature(FEATURE_VBAT)) {
						TxFrSkySPort(s, id, BatteryVolts * 10
								/ BatteryCellCount); //sending calculated average cell value with 0.01 precision
						FrSkySPortHasRequest = false;
					}
					break;

					default:
					break;
					// if nothing is sent, FrSkySPortHasRequest isn't cleared, we already incremented the counter, just loop back to the start
				} // switch
			}
		}
	}

}

#else

//___________________________________________________________________________

// FrSky SPort to MAVLink originally

// Frsky Sensor-ID to use.
#define SENSOR_ID1			0x1B // ID of sensor. Must be something that is polled by FrSky RX
#define SENSOR_ID2			0x0D
#define SENSOR_ID3			0x34
#define SENSOR_ID4			0x67
// Frsky-specific
#define START_STOP			0x7e
#define DATA_FRAME			0x10

//#define _FrSkySPort_Serial	Serial1
//#define _FrSkySPort_C1		UART0_C1
//#define _FrSkySPort_C3		UART0_C3
//#define _FrSkySPort_S2		UART0_S2
//#define _FrSkySPort_BAUD	57600
#define   MAX_ID_COUNT		19

short crc; // used for crc calc of frsky-packet
uint8 lastRx;
uint32 FSSP_ID_count = 0;
uint8 cell_count = 0;
uint8 latlong_flag = 0;
uint32 latlong = 0;
uint8 first = 0;

void FrSkySPort_Init(void) {
	/*
	 _FrSkySPort_Serial.begin(_FrSkySPort_BAUD);
	 _FrSkySPort_C3 = 0x10; // Tx invert
	 _FrSkySPort_C1 = 0xA0; // Single wire mode
	 _FrSkySPort_S2 = 0x10; // Rx Invert
	 */
}

void FrSkySPort_SendByte(uint8 s, uint8 byte) {

	TxChar(s, byte);

	// CRC update
	crc += byte; //0-1FF
	crc += crc >> 8; //0-100
	crc &= 0x00ff;
	crc += crc >> 8; //0-0FF
	crc &= 0x00ff;
}

void FrSkySPort_SendCrc(uint8 s) {
	TxChar(s, 0xFF - crc);
	crc = 0; // CRC reset
}

void FrSkySPort_SendPackage(uint8 s, uint16 id, uint32 value) {

	//zzz	_FrSkySPort_C3 |= 32; //  Transmit direction, to S.Port
	FrSkySPort_SendByte(s, DATA_FRAME);
	uint8 *bytes = (uint8*) &id;
	FrSkySPort_SendByte(s, bytes[0]);
	FrSkySPort_SendByte(s, bytes[1]);
	bytes = (uint8*) &value;
	FrSkySPort_SendByte(s, bytes[0]);
	FrSkySPort_SendByte(s, bytes[1]);
	FrSkySPort_SendByte(s, bytes[2]);
	FrSkySPort_SendByte(s, bytes[3]);
	FrSkySPort_SendCrc(s);
	//zzz	_FrSkySPort_Serial.flush();
	//zzz	_FrSkySPort_C3 ^= 32; // Transmit direction, from S.Port

}

void FrSkySPort_Process(uint8 s) {
	uint8 data = 0;
	uint32 temp = 0;
	uint8 offset;

	while (serialAvailable(s)) {
		data = RxChar(s);
		if (lastRx == START_STOP && ((data == SENSOR_ID1) || (data
								== SENSOR_ID2) || (data == SENSOR_ID3) || (data == SENSOR_ID4))) {

			switch (FSSP_ID_count) {
				case 0:
				if (GPS.fix == 3) {
					FrSkySPort_SendPackage(s, FSSP_ID_SPEED, GPS.gspeed * 20); // from GPS converted to km/h
				}
				break;
				case 1:
				FrSkySPort_SendPackage(s, FSSP_ID_RPM, DesiredThrottle * 2); //  * 2 if number of blades on Taranis is set to 2
				break;
				case 2:
				FrSkySPort_SendPackage(s, FSSP_ID_CURRENT, BatteryCurrent * 100);
				break;
				case 3: // Sends the altitude value from barometer, first sent value used as zero altitude
				FrSkySPort_SendPackage(s, FSSP_ID_ALTITUDE, BaroAltitude * 100); // from barometer, 100 = 1m
				break;
				case 4: // Sends the ap_longitude value, setting bit 31 high
				if (GPS.fix == 3) {
					if (GPS.Raw[EastC] < 0)
					latlong = ((abs(GPS.Raw[EastC]) / 100) * 6)
					| 0xC0000000;
					else
					latlong = ((abs(GPS.Raw[EastC]) / 100) * 6)
					| 0x80000000;
					FrSkySPort_SendPackage(s, FSSP_ID_LATLONG, latlong);
				}
				break;
				case 5: // Sends the ap_latitude value, setting bit 31 low
				if (GPS.fix == 3) {
					if (GPS.Raw[NorthC] < 0)
					latlong = ((abs(GPS.Raw[NorthC]) / 100) * 6)
					| 0x40000000;
					else
					latlong = ((abs(GPS.Raw[NorthC]) / 100) * 6);
					FrSkySPort_SendPackage(s, FSSP_ID_LATLONG, latlong);
				}
				break;
				case 6: // Sends the compass heading
				FrSkySPort_SendPackage(s, FSSP_ID_HEADING,
						RadiansToDegrees(Heading) * 100); // 10000 = 100 deg
				break;
				case 7: // Sends the analog value from input A0 on Teensy 3.1
				//FrSkySPort_SendPackage(s, FSSP_ID_ADC2, adc2);
				break;
				case 8: // First 2 cells
				temp = ((uint32) (BatteryVolts / (BatteryCellCount * 2))
						& 0xFFF);
				FrSkySPort_SendPackage(s, FSSP_ID_CELLS, (temp << 20)
						| (temp << 8)); // Battery cell 0,1
				break;
				case 9: // Optional 3 and 4 Cells
				if (BatteryCellCount > 2) {
					offset = BatteryCellCount > 3 ? 0x02 : 0x01;
					temp = ((uint32) (BatteryVolts / (BatteryCellCount * 2))
							& 0xFFF);
					FrSkySPort_SendPackage(s, FSSP_ID_CELLS, (temp << 20) | (temp
									<< 8) | offset); // Battery cell 2,3
				}
				break;
				case 10: // Optional 5 and 6 Cells
				if (BatteryCellCount > 4) {
					offset = BatteryCellCount > 5 ? 0x04 : 0x03;
					temp = ((uint32) (BatteryVolts / (BatteryCellCount * 2))
							& 0xFFF);
					FrSkySPort_SendPackage(s, FSSP_ID_CELLS, (temp << 20) | (temp
									<< 8) | offset); // Battery cell 2,3
				}
				break;
				case 11:
				FrSkySPort_SendPackage(s, FSSP_ID_ACCX, Acc[X] * 100);
				break;
				case 12:
				FrSkySPort_SendPackage(s, FSSP_ID_ACCY, Acc[Y] * 100);
				break;
				case 13:
				FrSkySPort_SendPackage(s, FSSP_ID_ACCZ, Acc[Z] * 100);
				break;
				case 14: // Sends voltage as a VFAS value
				FrSkySPort_SendPackage(s, FSSP_ID_VFAS, BatteryVolts / 10);
				break;
				case 15:
				//FrSkySPort_SendPackage(s, FSSP_ID_T1, gps_status);
				break;
				case 16:
				//FrSkySPort_SendPackage(s, FSSP_ID_T2, ap_base_mode);
				break;
				case 17:
				FrSkySPort_SendPackage(s, FSSP_ID_VARIO, ROC * 100); // 100 = 1m/s
				break;
				case 18:
				//if(ap_fixtype==3) {
				FrSkySPort_SendPackage(s, FSSP_ID_GPS_ALT, GPS.altitude * 100); // from GPS,  100=1m
				// }
				break;
				case 19:
				//FrSkySPort_SendPackage(s, FSSP_ID_FUEL, ap_custom_mode);
				break;

			}
			FSSP_ID_count++;
			if (FSSP_ID_count > MAX_ID_COUNT)
			FSSP_ID_count = 0;
		}
		lastRx = data;
	}
}

#endif

