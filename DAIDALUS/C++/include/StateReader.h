/* 
 * StateReader
 *
 * Contact: George Hagen
 *
 * Copyright (c) 2011-2015 United States Government as represented by
 * the National Aeronautics and Space Administration.  No copyright
 * is claimed in the United States under Title 17, U.S.Code. All Other
 * Rights Reserved.
 */

#ifndef STATEREADER_H
#define STATEREADER_H

#include "ErrorLog.h"
#include "ErrorReporter.h"
#include "SeparatedInput.h"
#include "AircraftState.h"
#include "Position.h"
#include "Velocity.h"
#include "ParameterReader.h"
#include <string>
#include <vector>
#include <iostream>

namespace larcfm {

/**
 * This reads in and stores a set of aircraft states, possibly over time, (and parameters) from a file
 * The Aircraft states are stored in an ArrayList<AircraftState>.
 *
 * State files consist of comma or space-separated values, with one point per line.
 * Required columns include aircraft name, 3 position columns (either x[nmi]/y[nmi]/z[ft] or 
 * latitude[deg]/longitude[deg]/altitude[ft]) and
 * 3 velocity columns (either vx[kn]/vy[kn]/vz[fpm] or track[deg]/gs[kn]/vs[fpm]).
 *
 * An optional column is time [s].  If it is included, a "history" will be build if an aircraft has more than one entry.
 * If it is not included, only the last entry for an aircraft will be stored.
 *
 * It is necessary to include a header line that defines the column ordering.  The column definitions are not case sensitive.
 * There is also an optional header line, immediately following the column definition, that defines the unit type for each
 * column (the defaults are listed above).
 *
 * If points are consecutive for the same aircraft, subsequent name fields may be replaced with a double quotation mark (&quot).
 * The aircraft name is case sensitive, so US54A != Us54a != us54a.
 *
 * Any empty line or any line starting with a hash sign (#) is ignored.
 *
 * Files may also include parameter definitions prior to other data.  Parameter definitions are of the form &lt;key&gt; = &lt;value&gt;,
 * one per line, where &lt;key&gt; is a case-insensitive alphanumeric word and &lt;value&gt; is either a numeral or string.  The &lt;value&gt;
 * may include a unit, such as "dist = 50 [m]".  Note that parameters require a space on either side of the equals sign.
 * Note that it is possible to also update the stored parameter values (or store additional ones) through API calls.
 *
 * Parameters can be interpreted as double values, strings, or Boolean values, and the user is required to know which parameter is
 * interpreted as which type.
 *
 * If the optional parameter "filetype" is specified, its value must be "state" or "history" (no quotes) for this reader to accept the 
 * file without error.
 *
 */
class StateReader: public ErrorReporter, public ParameterReader {
private:
	void loadfile();

protected:
	// we store the heading indices in the following order:
	enum {
		NAME, LAT_SX, LON_SY, ALT_SZ, TRK_VX, GS_VY, VS_VZ, TM_CLK
	};
	static const int head_length = TM_CLK + 1;

	mutable ErrorLog error;
	SeparatedInput input;
	std::vector<AircraftState> states;
	bool hasRead;
	bool latlon;
	bool trkgsvs;
	bool clock;
	int head[head_length];

	bool interpretUnits;

	int altHeadings(const std::string& s1, const std::string& s2,
			const std::string& s3, const std::string& s4) const;
	int altHeadings(const std::string& s1, const std::string& s2,
			const std::string& s3) const;
	int altHeadings(const std::string& s1, const std::string& s2) const;
	double parseClockTime(const std::string& s) const;
	int getIndex(const std::string& s) const;

public:

	/** A new, empty StateReader.  This may be used to store parameters, but nothing else. */
	StateReader();

	/** Read a new file into an existing StateReader.  Parameters are preserved if they are not specified in the file. */
	virtual void open(const std::string& filename);

	/** Read a new stream into an existing StateReader.  Parameters are preserved if they are not specified in the file. */
	virtual void open(std::istream* ins);

	ParameterData& getParametersRef();

	/** Return the number of AircraftStates in the file */
	int size() const;
	/** Returns the i-th AircraftState in the file */
	AircraftState getAircraftState(int i) const;

	/** Returns the list of all AircraftStates in the file */
	std::vector<AircraftState> getAircraftStateList() const;

	/** Returns the (most recent) position of the i-th aircraft state in the file.  This is the raw position, and has not been through any projection. */
	Position getPosition(int ac) const;

	/** Returns the (most recent) velocity of the i-th aircraft state in the file.   This is the raw velocity, and has not been through any projection. */
	Velocity getVelocity(int ac) const;

	/** returns the string name of aircraft i */
	std::string getName(int ac) const;

	double getTime(int ac) const;

	bool isLatLon() const;

	// ErrorReporter Interface Methods

	bool hasError() const {
		return error.hasError() || input.hasError();
	}
	bool hasMessage() const {
		return error.hasMessage() || input.hasMessage();
	}
	std::string getMessage() {
		return error.getMessage() + input.getMessage();
	}
	std::string getMessageNoClear() const {
		return error.getMessageNoClear() + input.getMessageNoClear();
	}

};

}

#endif //STATEREADER_H
