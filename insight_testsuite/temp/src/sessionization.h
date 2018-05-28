/*
 * Insight Data Engineering coding chanllenge 
 * Author: Bo Li 
 * Email: libo.ustc@gmail.com
 */

#ifndef _SESSIONIZATION
#define _SESSIONIZATION

#include <string>
#include <unordered_map>
#include <vector>
#include <map>
#include <list>

using namespace std;

struct SessionEntry {
	int id;		//id for sequential input
	string ip;
	int year;
	int month;
	int day;
	int firstTS;	//timestamp in seconds
	int lastTS;	//timestamp in seconds
	int count;
	
	SessionEntry();
	SessionEntry(int _id, string _ip, int _year, int _month, int _day,
	    int _firstTS, int _lastTS);
};

class Sessionization {
private:
	int inactivity_period;
	unordered_map<string, SessionEntry> mIPEntry;	//mapping between IP
 							//addr and session entry
	map<int, list<string>> mLastVisit;	//sorting by last visit
       						//timestamp
	//for fast reference when we delete an element from the list
	unordered_map<string, list<string>::iterator> mIPIter;
	vector<string> fieldMap;	//mapping between field name and
       					//its order in the input file

	SessionEntry readSessionEntry(string& str, int seqid);
	void writeToOutput(vector<SessionEntry>& res, ofstream& ofs);
public:
	void sessionize(string fInput, string fInact, string fOutput);
};

#endif
