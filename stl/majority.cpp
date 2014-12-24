#include <vector>
#include <map>

class Solution {
public:
    int majorityElement(vector<int> &num) {
        map<int, int> mapRec;
        int max = 0;
        int maj = 0;
        for (int ii = 0; ii < num.size(); ii++) {
            if (1 == mapRec.count(num[ii])) {
                mapRec[num[ii]] = mapRec[num[ii]] + 1;
            } else {
                mapRec.insert(pair<int, int>(num[ii], 1));
            }
        }

        // find the max value
        map<int, int>::iterator iter;
        for (iter = mapRec.begin(); iter != mapRec.end(); iter++) {
            if (max < iter->second) {
                max = iter->second;
                maj = iter->first;
            }
        }
        return maj;
    }
};
