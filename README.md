Insight Data Engineering Coding Challenge solution overview 
Author: Bo Li
Email: libo.ustc@gmail.com

I use C++ to implement the code. Below are the key data structures in the solution.
	mIPEntry: hashmap mapping IP address to session data (unordered_map in C++).
	mLastVisit: sort the session by last visit timestamp using this binary search tree (map in C++). Elements with the same timestamps are stored in a list.
	mIPIter: hashmap mapping IP address to iterator of session in mLastVisit, used for quick delete operation from mLastVisit (unordered_map in C++).
	fieldMap: mapping field name in the header line to its order in the input entry.

Algorithm is straightforward. All live sessions are stored in a binary search tree (BST) ordered by the last visit timestamp. When we process a new weblog data entry of a certain timestamp, we check if any session just expires at that timestamp, record them in the output file and remove the expired sessions from the BST. If incoming session already exists which means it is live, we update its last visit timestamp and the BST accordingly. If we detect multiple user sessions ending at the same time, we write the results to the output file in the same order as the user's first request for that session appeared in the input file.

This method is scalable and easily extends for larger data size. Discussion is left for future due to the scope of this coding challenge.
