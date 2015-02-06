#include <vector>
#include <tuple>
#include <iostream>
#include <iomanip>      // std::setw
#include <queue>
#include <opencv2/features2d/features2d.hpp>  // DMatch

using namespace std;
using namespace cv;

static const int NOT_ENGAGED = -1;

/**
 * Returns a stable marriage in the form an int array v,
 * where v[i] is the man married to woman i.
 */
void marriageMatch(const vector<vector<DMatch>>& acceptor_table,
                   const vector<vector<DMatch>>& proposor_table,
                   vector<DMatch>& matches)
{
    // Indicates that acceptor i is currently enganged
    // to the proposer v[i]
    vector<int> engagements(acceptor_table.size(), NOT_ENGAGED);

    // queue of proposers that are not currently engaged
    queue<int> free_proposers;

    // mark every proposer as free
    for (int i = 0; i < proposor_table.size(); i++) {
        free_proposers.push(i);
    }

    // next[i] is the index of the accecptor to whom
    // proposer[i] has not yet proposed
    vector<int> next(proposor_table.size(), 0);

    while (!free_proposers.empty()) {
        int p = free_proposers.front();
        free_proposers.pop();

        // Check if there are any further prefered
        // acceptors for this proposer
        if (next[p] >= proposor_table[p].size()) {
            continue;
        }

        assert(p < proposor_table.size());
        int a = proposor_table[p][next[p]++].trainIdx;

        assert(a < engagements.size());

        // if the acceptor has not been engaged yet,
        // the proposal get immediately accepted
        if (engagements[a] == NOT_ENGAGED) {
            // It is important to check, if the current propser
            // is contained in the preference list of the acceptor
            for (int i = 0; i < acceptor_table[a].size(); i++) {

                assert(i < acceptor_table[a].size());

                if (acceptor_table[a][i].trainIdx == p) {
                    engagements[a] = p;
                    break;
                }
            }
            // The proposer was not in the preference list. Therefore
            // the proposer stays free
            if (engagements[a] == NOT_ENGAGED) {
                free_proposers.push(p);
            }
        } else {
            // the current fiance of the 
            int fiance = engagements[a];

            for (int i = 0; i < acceptor_table[a].size(); i++) {
                assert(a == acceptor_table[a][i].queryIdx);

                int preference = acceptor_table[a][i].trainIdx;

                // the fiance has a higher preference
                // for the acceptor
                if (preference == fiance) {
                    free_proposers.push(p);
                    break;
                }
                // the current proposer has a higher preference
                // than the fiance
                else if (preference == p) {
                    engagements[a] = p;
                    free_proposers.push(fiance);
                    break;
                }
            }
        }
    }

    matches.clear();

    cout << "Resolve matches ... " << flush << endl;

    for (int i = 0; i < engagements.size(); i++) {
        if (engagements[i] != NOT_ENGAGED) {
            // cout << engagements[i] << endl;

            // ensure that the acceptor has this number of
            // neighbors
            // if (i < acceptor_table[i].size()) {

                assert(i < acceptor_table.size());

                // search for the DMatch where the related
                // train index (index of the keypoint in the other
                // image) equals the index of the husband
                for (int j = 0; j < acceptor_table[i].size(); j++) {

                    // cout << "  " << acceptor_table[i][j].trainIdx << endl;

                    if (acceptor_table[i][j].trainIdx == engagements[i]) {
                        matches.push_back(acceptor_table[i][j]);
                        break;
                    }
                }
            // }
        }
    }

    cout << "Done with " << matches.size() << flush << endl;
}