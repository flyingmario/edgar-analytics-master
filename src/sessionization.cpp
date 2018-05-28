/*
 * Insight Data Engineering coding chanllenge 
 * Author: Bo Li 
 * Email: libo.ustc@gmail.com
 */

#include "sessionization.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>
#include <cstdio>
#include <algorithm>

using namespace std;

//Compare entry by its order in the input file, which is also chronological.
struct CompareSessionEntry {
	bool operator()(const SessionEntry& lhs, const SessionEntry& rhs) {
		return lhs.id < rhs.id;
	}	
};

SessionEntry::SessionEntry() : firstTS(0), lastTS(0), count(1) {}

SessionEntry::SessionEntry(int _id, string _ip, int _year, int _month, int _day,
    int _firstTS, int _lastTS) : id(_id), ip(_ip), year(_year), month(_month),
    day(_day), firstTS(_firstTS), lastTS(_lastTS), count(1) {}

//Parse each line in the input log file.
SessionEntry Sessionization::readSessionEntry(string& str, int seqid) {
	vector<string> arr;
	string s;
	istringstream iss(str);
	
	while (getline(iss, s, ',')) {
		arr.push_back(s);
	}

	SessionEntry se;
	se.id = seqid;

	for (int i = 0; i < arr.size(); ++i) {
		if (fieldMap[i] == "ip") {
			se.ip = arr[i];
		} else  if (fieldMap[i] == "date") {
			istringstream issDate(arr[i]);
			getline(issDate, s, '-');
			se.year = stoi(s);
			getline(issDate, s, '-');
			se.month = stoi(s);
			getline(issDate, s, '-');
			se.day = stoi(s);
		} else if (fieldMap[i] == "time") {
			istringstream issTime(arr[i]);
			getline(issTime, s, ':');
			se.firstTS += stoi(s) * 3600;
			getline(issTime, s, ':');
			se.firstTS += stoi(s) * 60;
			getline(issTime, s, ':');
			se.firstTS += stoi(s);

			se.lastTS = se.firstTS;
		}
	}

	return se;
}	

//Write to the output file in the given format.
void Sessionization::writeToOutput(vector<SessionEntry>& res, ofstream& ofs) {
	for (int i = 0; i < res.size(); ++i) {
		SessionEntry& se = res[i];
		char cf[20] = {0};
		char cl[20] = {0};

		sprintf(cf, "%d-%02d-%02d %02d:%02d:%02d", se.year, se.month,
		    se.day, se.firstTS / 3600, (se.firstTS % 3600) / 60,
		    se.firstTS % 60);
		sprintf(cl, "%d-%02d-%02d %02d:%02d:%02d", se.year, se.month,
		    se.day, se.lastTS / 3600, (se.lastTS % 3600) / 60,
		    se.lastTS % 60);

		string sfTS(cf);
		string slTS(cl);
		string str = se.ip + "," + sfTS + "," + slTS + "," +
		    to_string(se.lastTS - se.firstTS + 1) + "," +
		    to_string(se.count) + "\n";

		ofs << str;
	}
}

//Process the EDGAR weblog data, generate result of the time a particular user
//spends on EDGAR during a visit and the number of documents that user requests.
void Sessionization::sessionize(string fInput, string fInact, string fOutput) {
	//Clear up.
	mIPEntry.clear();
	mIPIter.clear();
	mLastVisit.clear();
	fieldMap.clear();

	string str;
	ifstream fsLog(fInput);
	ifstream fsInact(fInact);
	
	//Read inactivity period.
	if (!getline(fsInact, str)) {
		return;
	}
	inactivity_period = stoi(str);

	//Read header line.
	if (!getline(fsLog, str)) {
		return;
	}

	//Figure out the field names' order.
	string field;
	istringstream iss(str);
	while (getline(iss, field, ',')) {
		fieldMap.push_back(field);
	}

#ifdef DEBUG
	cout << "inactivity_period " << inactivity_period << endl;
	for (int i = 0; i < fieldMap.size(); ++i) {
		cout << fieldMap[i] << " ";
	}
	cout << endl;
#endif

	ofstream fsOutput;
	fsOutput.open(fOutput);
	
	int seqid = 1;
	while (getline(fsLog, str)) {
		SessionEntry se = readSessionEntry(str, seqid);
		++seqid;
		
		//Check expired sessions and output their stats if any.
		auto itLow = mLastVisit.lower_bound(se.lastTS -
		    inactivity_period);
		if (itLow != mLastVisit.end()) {
			vector<SessionEntry> res;
			for (auto it = mLastVisit.begin(); it !=itLow; ++it) {
				list<string>& l = it->second;
				for (string& strIP : l) {
					res.push_back(mIPEntry[strIP]);
					mIPEntry.erase(strIP);
					mIPIter.erase(strIP);
				}
			}
			mLastVisit.erase(mLastVisit.begin(), itLow);
			sort(res.begin(), res.end(), CompareSessionEntry());
			writeToOutput(res, fsOutput);
		}
		
		auto itSe = mIPEntry.find(se.ip);
		//This session entry exists and is still valid.
		if (itSe != mIPEntry.end() && itSe->second.lastTS !=
	  	    se.lastTS) {
			mLastVisit[itSe->second.lastTS].erase(mIPIter[se.ip]);
			if (mLastVisit[itSe->second.lastTS].empty()) {
				mLastVisit.erase(itSe->second.lastTS);
			}
			itSe->second.lastTS = se.lastTS;
			++itSe->second.count;
			mLastVisit[se.lastTS].push_back(se.ip);
			mIPIter[se.ip] = prev(mLastVisit[se.lastTS].end());
		} else if (itSe != mIPEntry.end() && itSe->second.lastTS ==
	       	    se.lastTS) {
			++itSe->second.count;
		} else {
			//Here comes a new session entry.
			mIPEntry[se.ip] = se;
			mLastVisit[se.lastTS].push_back(se.ip);
			mIPIter[se.ip] = prev(mLastVisit[se.lastTS].end());
		}
	}

	//We hit the end of log. All sessions terminate.
	vector<SessionEntry> res;
	for (auto it = mIPEntry.begin(); it != mIPEntry.end(); ++it) {
		res.push_back(it->second);
	}
	sort(res.begin(), res.end(), CompareSessionEntry());
	writeToOutput(res, fsOutput);

	fsOutput.close();
}

int main(int argc, char* argv[]) {
#ifdef DEBUG
	for (int i = 0; i < argc; ++i) {
		cout << argv[i] << " ";
	}
	cout << endl;
#endif

	if (argc != 4) {
		cout << "Incorrect input" << endl;
		return 1;
	}

	Sessionization ss;
	ss.sessionize(argv[1], argv[2], argv[3]);
	
	return 0;
}
