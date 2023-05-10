#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <queue>
#include <fstream>
#include <cctype>
#include <algorithm>
#include <sstream>
#include <map>
#include <set>

using namespace std;
string finalRes="FALSE";
string strip(string& str) {
    return str.substr(str.find_first_not_of(' '),str.find_last_not_of(' ')-str.find_first_not_of(' ')+1);
}
vector<string> Split(const string& str, const string del) {
    vector<string> res;
    int ini = 0;
    while (str.find(del, ini)!=string::npos) {
        int found = str.find(del, ini);
        res.push_back(str.substr(ini, found-ini));
        ini = found+del.length();
    }
    res.push_back(str.substr(ini, string::npos));
    for (auto& k:res) k=strip(k);
    return res;
}
pair<string, vector<string>> strPredToDSPred(string& str) {
    string baseStr = str.substr(0, str.find("(", 0));
    baseStr = strip(baseStr);
    vector<string> temArg = Split(str.substr(str.find("(", 0)+1, str.find(")", 0)-str.find("(", 0)-1), ",");
    pair<string, vector<string>> res ={baseStr, temArg};
    return res;
}
string dsPredToStrPred(pair<string, vector<string>>& ds) {
    string res=ds.first+"(";
    for (auto& each:ds.second) {
        res+=each+",";
    }
    res = res.substr(0, res.length()-1)+")";
    return res;
}
bool isVar(string str) {
    return str.length()==1 && islower(str[0])!=0;
}

struct sPred {
    string baseStr;
    vector<string> arg;
    struct sentence* sentencePtr;
};

struct sentence {
    vector<sPred*> sPredArr;
    unordered_map<string, unordered_map<sPred*, vector<int>>> relation;
    bool setCID() {
        return relation.empty();
    }
};

void plugIn(unordered_map<sPred *, vector<string>>&, string&, string&);

unordered_map<string, unordered_set<sPred*>> gBase;
unordered_set<string> cSentenceChk;
unordered_set<string> vSentenceChk;


string prin(vector<string> test) {
    string constId;
    for (string& each: test) {
        constId+= each + " | ";
    }
    constId = constId.substr(0, constId.length()-3);
    return constId;
}


unordered_set<string> symbolPoll;
void collectSymbol(string& str) {
    vector<string> term = Split(str.substr(str.find("(", 0)+1, str.find(")", 0)-str.find("(", 0)-1), ",");
    for (auto&k: term) {
        if (!isVar(k)) symbolPoll.insert(k);
    }
}

string generateSen(vector<sPred*>& predArr) {
    unordered_map<sPred*, vector<string>> tempStore;
    char curChar = 'a';
    for (auto& each: predArr) {
        tempStore[each] = each->arg;
    }

    for (auto& each: predArr) {
        for (auto& item: tempStore[each]) {
            if (isVar(item)) {
                string curStr = "*"+string {curChar};
                plugIn(tempStore, item, curStr);
                curChar++;
            }
        }
    }

    for (auto& qp: tempStore) {
        for (auto& each: qp.second) {
            if (each[0]=='*') each=each.substr(1, string::npos);;
        }
    }

    string final;
    for (auto& each: predArr) {
        pair<string, vector<string>> tep = {each->baseStr, tempStore[each]};
        final+= dsPredToStrPred(tep);
        final+=" | ";
    }
    return final.substr(0, final.length()-3);
}


void possibleSentence(map<string, unordered_set<sPred*>>& category, vector<sPred*>& resSen, vector<string>& res, map<string, unordered_set<sPred*>>::iterator& it, sPred* toInsert) {
    // insert
    resSen.push_back(toInsert);
    (*it).second.erase(toInsert);
    // check leaf node
    if ((*it).second.empty() && next(it)==category.end()) {
        res.push_back(generateSen(resSen));
        // restore
        resSen.pop_back();
        (*it).second.insert(toInsert);
        return;
    }
    // call stack
    bool flag = false;
    if ((*it).second.empty()) {
        it++;
        flag = true;
    }
    vector<sPred*> tem ((*it).second.begin(), (*it).second.end());
    for (auto& k: tem) {
        possibleSentence(category, resSen, res, it, k);
    }
    // restore
    resSen.pop_back();
    if (flag) it--;
    (*it).second.insert(toInsert);
}


pair<bool, string> checkVSentence(sentence* sen) {
    map<string, unordered_set<sPred*>> category;

    for (auto& predPtr: sen->sPredArr) {
        category[predPtr->baseStr].insert(predPtr);
    }

    vector<string> allSen;
    vector<sPred*> bench;

    auto it = category.begin();
    vector<sPred*> tem ((*it).second.begin(), (*it).second.end());
    for (auto&k: tem) {
        possibleSentence(category, bench, allSen, it, k);
    }

    for (auto& each: allSen) {
        if (vSentenceChk.count(each)==1) return {false, ""};
    }
    return {true, allSen[0]};
}



bool cCombination(set<string>& subCom, int curIdx, vector<string>& src) {
    // update
    subCom.insert(src[curIdx]);
    // check condition
    string curStr;
    for (auto&k : subCom) curStr+=k;
    if (cSentenceChk.count(curStr)==1) {
        return false;
    }
    // adj
    for (int i=curIdx+1;i<src.size();++i) {
        bool res = cCombination(subCom, i, src);
        if (!res) return false;
    }
    // restore
    subCom.erase(src[curIdx]);
    return true;
}


pair<bool, string> bridge(vector<string>& strPredArr) {
    // check if previous truth exist
    sort(strPredArr.begin(), strPredArr.end());
    set<string> tepBench;
    for (int i=0;i<strPredArr.size();++i) {
        bool temRes = cCombination(tepBench, i, strPredArr);
        if (!temRes) return {false, ""};
    }
    string curStr;
    for (auto&k: strPredArr) {
        curStr+=k;
    }
    return {true, curStr};
}

pair<bool, sentence*> insert(vector<string> senArr) {
    pair<bool, sentence*> res;
    // clean
    unordered_set<string> simplifiedBase;
    for (auto& each: senArr) {
        // each: predicate string version
        pair<string, vector<string>> ds = strPredToDSPred(each);
        string str = dsPredToStrPred(ds);
        string oppo = str[0]=='~'? str.substr(1, string::npos):"~"+str;
        simplifiedBase.insert(str);
        if (simplifiedBase.count(oppo)==1) return {false, nullptr}; // ok
    }

    auto sentencePtr = new sentence;
    for (string predStr: simplifiedBase) {
        collectSymbol(predStr); // store symbol
        pair<string, vector<string>> predDS = strPredToDSPred(predStr);
        auto sPredPtr = new sPred;
        *sPredPtr = {predDS.first, predDS.second, sentencePtr};
        gBase[predDS.first].insert(sPredPtr);

        for (int i=0;i<predDS.second.size();++i) {
            if (isVar(predDS.second[i])) {
                sentencePtr->relation[predDS.second[i]][sPredPtr].push_back(i);
            }
        }
        sentencePtr->sPredArr.push_back(sPredPtr);
    }
    // decide whether insert sentence
    if (sentencePtr->setCID()) {
        vector<string> strPredArr;
        for (sPred* pred: sentencePtr->sPredArr) {
            pair<string, vector<string>> ds = {pred->baseStr, pred->arg};
            strPredArr.push_back(dsPredToStrPred(ds));
        }
        auto re = bridge(strPredArr);
        if (!re.first) {
            for (auto& spred: sentencePtr->sPredArr) {
                gBase[spred->baseStr].erase(spred);
                delete spred;
            }
            delete sentencePtr;
            return {false, nullptr};
        }
        cSentenceChk.insert(re.second);
//        cout << re.second << endl;
    } else {// var sentence
        vector<string> strPredArr;
        for (sPred* pred: sentencePtr->sPredArr) {
            bool fl = false;
            for (auto& k: pred->arg) {
                if (isVar(k)) {
                    fl = true;
                    break;
                }
            }
            if (fl) continue;
            pair<string, vector<string>> ds = {pred->baseStr, pred->arg};
            strPredArr.push_back(dsPredToStrPred(ds));
        }

        if (!strPredArr.empty()) {
            auto re = bridge(strPredArr);
            if (!re.first) {
                for (auto& spred: sentencePtr->sPredArr) {
                    gBase[spred->baseStr].erase(spred);
                    delete spred;
                }
                delete sentencePtr;
                return {false, nullptr};
            }
        }

        pair<bool, string> pt = checkVSentence(sentencePtr);
        if (!pt.first) {
            for (auto& spred: sentencePtr->sPredArr) {
                gBase[spred->baseStr].erase(spred);
                delete spred;
            }
            delete sentencePtr;
            return {false, nullptr};
        }
        vSentenceChk.insert(pt.second);
//        cout << pt.second <<endl;
    }
    return {true, sentencePtr};
}

void plugIn(unordered_map<sPred *, vector<string>>& storage, string& varName, string& value) {
    sentence* senPtr;
    for (auto& pred: storage) {
        senPtr = pred.first->sentencePtr;
        break;
    }
    for (auto &pa: senPtr->relation[varName]) {// each key-value
        for (auto &pb: pa.second) {
            storage[pa.first][pb] = value;
        }
    }
}


void processInput() {
    fstream fs;
    fs.open("input.txt", fstream::in);
    string query;
    getline(fs, query);
    string sentenceNum;
    getline(fs, sentenceNum);
    for (unsigned i=0;i<stoul(sentenceNum);++i) {
        string inputStr;
        getline(fs, inputStr);
        // process each line
        vector<string> splitArr = Split(inputStr, " => ");
        vector<vector<string>> cnf;
        if (splitArr.size()==1) {//no imp
            splitArr = Split(splitArr[0], " | ");
            vector<vector<string>> oneSentence;
            for (string& k: splitArr) {
                oneSentence.push_back(Split(k, " & "));
            }
            vector<vector<string>> temp;
            for (string& w: oneSentence[0]) {
                temp.push_back({w});
            }
            for (int k=1;k<oneSentence.size();++k) {
                vector<vector<string>> tempTwo;
                for (vector<string>& o: temp) {
                    for (string& p: oneSentence[k]) {
                        vector<string> tempThree = o;
                        tempThree.push_back(p);
                        tempTwo.push_back(tempThree);
                    }
                }
                temp = tempTwo;
            }
            cnf = temp;
        } else {// imp
            string secTerm = splitArr[1];
            splitArr = Split(splitArr[0], " | ");
            vector<vector<string>> oneSentence;
            for (string& k: splitArr) {
                oneSentence.push_back(Split(k, " & "));
            }
            for (vector<string>& o: oneSentence) {
                for (string& p: o) {
                    if (p[0]=='~') {
                        p = p.substr(1, string::npos);
                    } else {
                        p = "~"+p;
                    }
                }
                o.push_back(secTerm);
            }
            cnf = oneSentence;
        }
        // process cnf
        for (auto& sen: cnf) {
            insert(sen);
        }
    }
    string neQry = query[0]=='~'?query.substr(1, string::npos):"~"+query;
    vector<string> qry = {neQry};
    insert(qry);
    fs.close();
}


void randomVar(unordered_map<sPred *, vector<string>>& storage1, unordered_map<sPred *, vector<string>>& storage2) {
    char curChar = 'a';
    for (auto &ep: storage1) {// each remaining preds
        for (auto &item: ep.second) {// each arg of one pred
            if (isVar(item)) {// not *, not symbol
                string curStr = "*" + string{curChar};
                plugIn(storage1, item, curStr);
                curChar++;
            }
        }
    }

    for (auto &ep: storage2) {// each remaining preds
        for (auto &item: ep.second) {// each arg of one pred
            if (isVar(item)) { // not *, not symbol
                string curStr = "*" + string{curChar};
                plugIn(storage2, item, curStr);
                curChar++;
            }
        }
    }

    for (auto& qp: storage1) {
        for (auto& each: qp.second) {
            if (each[0]=='*') each=each.substr(1, string::npos);;
        }
    }

    for (auto& qp: storage2) {
        for (auto& each: qp.second) {
            if (each[0]=='*') each=each.substr(1, string::npos);;
        }
    }
}


vector<string> finalSen(unordered_map<sPred *, vector<string>>& s1, unordered_map<sPred *, vector<string>>& s2, sPred* p1, sPred*p2) {
    randomVar(s1, s2);
    s1.erase(p1);
    s2.erase(p2);
    vector<string> myFinal;
    for (auto& qp: s1) {
        pair<string, vector<string>> ds = {qp.first->baseStr, qp.second};
        myFinal.push_back(dsPredToStrPred(ds));
    }

    for (auto& qp: s2) {
        pair<string, vector<string>> ds = {qp.first->baseStr, qp.second};
        myFinal.push_back(dsPredToStrPred(ds));
    }
    return myFinal;
}


void waterFlow2(vector<string>& board1, vector<string>&,
                vector<vector<pair<sPred*, string>>>&,
                int,
                sPred*,
                sPred*
);

void waterFlow1(vector<string>& board1, vector<string>& board2,
               vector<vector<pair<sPred*, string>>>& category,
               int idx,
               sPred* p1,
               sPred* p2
               ) {
    string varName = board1[idx];

    for (auto pa: p1->sentencePtr->relation[varName][p1]) {
        board1[pa] = "*";
    }

    category.back().push_back({p1, varName});

    for (auto pa: p1->sentencePtr->relation[varName][p1]) {
        if (isVar(board2[pa])) {
            waterFlow2(board1, board2, category, pa, p1, p2);
        }
    }
}


void waterFlow2(vector<string>& board1, vector<string>& board2,
                vector<vector<pair<sPred*, string>>>& category,
                int idx,
                sPred* p1,
                sPred* p2
) {
    string varName = board2[idx];

    for (auto pa: p2->sentencePtr->relation[varName][p2]) {
        board2[pa] = "*";
    }

    category.back().push_back({p2, varName});

    for (auto pa: p2->sentencePtr->relation[varName][p2]) {
        if (isVar(board1[pa])) {
            waterFlow1(board1, board2, category, pa, p1, p2);
        }
    }
}

void combination(int depth,
                 int curDepth,
                 vector<vector<string>>& res,
                 unordered_map<sPred *, vector<string>> store1,
                 unordered_map<sPred *, vector<string>> store2,
                 vector<vector<pair<sPred*, string>>>& category,
                 sPred* p1,
                 sPred* p2,
                 string value
                 ) {

    unordered_map<sPred*, unordered_map<sPred *, vector<string>>*> tempMap = {
            {p1, &store1},
            {p2, &store2}
    };

    // update
    for (auto& eachPair: category[depth]) {
        plugIn((*tempMap[eachPair.first]), eachPair.second, value);
    }

    if (curDepth == depth) {
        // push sentence
        res.push_back(finalSen(store1, store2, p1, p2));
        return;
    }
    // call stack
    for (string eachSym: symbolPoll) {
        combination(depth, curDepth+1, res, store1, store2, category, p1, p2, eachSym);
    }
}

vector<vector<string>> twoVarAlgo(unordered_map<sPred *, vector<string>>& store1, sPred* p1, unordered_map<sPred *, vector<string>>& store2, sPred* p2) {
    vector<string> board1 = store1[p1];
    vector<string> board2 = store2[p2];
    vector<vector<pair<sPred*, string>>> category;

    bool varExist = true;
    while (varExist) {
        varExist = false;
        for (int i=0;i<board1.size();++i) {
            if (isVar(board1[i])) {
                category.push_back({});
                waterFlow1(board1, board2, category, i, p1, p2);
                varExist = true;
                break;
            }
        }
    }

    vector<vector<string>> res;
    for (string eachSym: symbolPoll) {
        combination(category.size()-1, 0, res, store1, store2, category, p1, p2, eachSym);
    }
    return res;
}


pair<bool, vector<vector<string>>> can_unify(sPred* p1, sPred* p2) {
    unordered_map<sPred *, vector<string>> storage1;
    unordered_map<sPred *, vector<string>> storage2;
    for (auto eachPred: p1->sentencePtr->sPredArr) {
        storage1[eachPred] = eachPred->arg;
    }
    for (auto eachPred: p2->sentencePtr->sPredArr) {
        storage2[eachPred] = eachPred->arg;
    }

    unordered_map<sPred*, unordered_map<sPred *, vector<string>>*> tempMap = {
            {p1, &storage1},
            {p2, &storage2}
    };

    bool updated = true;
    while (updated) {
        updated = false;
        for (int i = 0; i < p1->arg.size(); ++i) {
            if (isVar(storage1[p1][i]) && isVar(storage2[p2][i])) {// both var
                continue;
            } else if (!isVar(storage1[p1][i]) && !isVar(storage2[p2][i])) {// both symbol
                if (storage1[p1][i] == storage2[p2][i]) {
                    continue;
                } else {
                    return {false, {}};
                }
            } else {// one symbol one var
                updated = true;
                sPred *varPtr;
                sPred *cPtr;
                if (!isVar(storage1[p1][i])) {//ptr1 is symbol, ptr2 is var
                    varPtr = p2;
                    cPtr = p1;
                } else {// ptr1 is var, ptr2 is symbol
                    varPtr = p1;
                    cPtr = p2;
                }
                string symbol = (*tempMap[cPtr])[cPtr][i];
                string var = (*tempMap[varPtr])[varPtr][i];
                plugIn((*tempMap[varPtr]), var, symbol);
            }
        }
    }

    // deal with 2 var case
    pair<bool, vector<vector<string>>> res = {true, {}};
    bool twoVar = false;
    for (int i=0;i<p1->arg.size();++i) {
        if (isVar(storage1[p1][i]) && isVar(storage2[p2][i])) {
            twoVar = true;
            res.second = twoVarAlgo(storage1, p1, storage2, p2);
            break;
        }
    }

    if (!twoVar) {
        res.second = {finalSen(storage1, storage2, p1, p2)};
    }
    // insert
    return res;
}

string hashStr(sPred* p1, sPred* p2) {
    vector<string> tep;
    stringstream ss;
    ss << p1;
    tep.push_back(ss.str());
    ss.str("");
    ss.clear();
    ss << p2;
    tep.push_back(ss.str());
    sort(tep.begin(), tep.end());
    return tep[0]+"-"+tep[1];
}

struct comp {
    bool operator() (const vector<string>& lhs, const vector<string>&rhs) const
    {
        return (lhs.size()>rhs.size());
    }
};



int main() {
    processInput();
    priority_queue<vector<string>, vector<vector<string>>, comp> myQ;
    // initial population
    unordered_set<string> reChk;
    for (auto &g1Pair: gBase) {
        string g1 = g1Pair.first;
        if (reChk.count(g1) == 1) {
            continue;
        }
        string oppoG = g1[0] == '~' ? g1.substr(1, string::npos) : "~" + g1;
        reChk.insert(g1);
        reChk.insert(oppoG);
        if (gBase.count(oppoG) == 1) {
            for (auto &p1: gBase[g1]) {
                for (auto &p2: gBase[oppoG]) {
                    pair<bool, vector<vector<string>>> output = can_unify(p1, p2);
                    if (output.first) { // can unify
                        for (auto& k: output.second) myQ.push(k);
                    }
                }
            }
        }
    }

    unordered_set<string> reCheck;

    while (!myQ.empty()) {
//        cout << myQ.size() << endl;
        if (myQ.top().empty()) {
            finalRes = "TRUE";
            break;
        }

        pair<bool, sentence*> insertion = insert(myQ.top());
        myQ.pop();

        if (!insertion.first) {
            continue;
        }
        // add unified output to queue
        for (auto& p1: insertion.second->sPredArr) {
            string g1 = p1->baseStr;
            string oppoG = g1[0] == '~' ? g1.substr(1, string::npos) : "~" + g1;
            if (gBase.count(oppoG) == 1) {
                for (auto &p2: gBase[oppoG]) {
                    string hashKey = hashStr(p1, p2);
                    if (reCheck.count(hashKey)==1) continue;
                    reCheck.insert(hashKey);
                    pair<bool, vector<vector<string>>> output = can_unify(p1, p2);
                    if (output.first) { // can unify
                        for (auto& k : output.second) myQ.push(k);
                    }
                }
            }
        }
    }
    cout << finalRes << endl;
    fstream fo;
    fo.open("output.txt", fstream::out);
    fo << finalRes;
    fo.close();
    return 0;
}