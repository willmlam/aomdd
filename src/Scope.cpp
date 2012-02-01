#include "Scope.h"
#include "base.h"
#include "utils.h"
using namespace std;

namespace aomdd {

Scope::Scope() {
}

Scope::Scope(const Scope &s) :
    varCard(s.varCard), ordering(s.ordering) {
}

Scope::Scope(const Scope &lhs, const Scope &rhs, oper op) {
    if (!lhs.HasConsistentCard(rhs)) {
        cerr << "Inconsistent scopes" << endl;
        exit(0);
    }
    else {
        *this = lhs;

        /*
        map<int, unsigned int>::const_iterator it =
                rhs.varCard.begin();
        map<int, unsigned int>::const_iterator lit =
                lhs.varCard.begin();
                */
        list<int>::const_iterator it = rhs.ordering.begin();
        list<int>::const_iterator lit = lhs.ordering.begin();
        switch (op) {
            case Scope::UNION:
                for (; it != rhs.ordering.end(); ++it)
                    AddVar(*it, rhs.GetVarCard(*it));
                break;
            case Scope::INTERSECT:
                for (; lit != lhs.ordering.end(); ++lit) {
                    if (!rhs.VarExists(*lit))
                        RemoveVar(*lit);
                }
                break;
            case Scope::DIFF:
                for (; it != rhs.ordering.end(); ++it)
                    RemoveVar(*it);
                break;
            default:
                // Should never get here.
                assert(true);
                break;
        }
    }
}

Scope::~Scope() {
    varCard.clear();
}

inline bool Scope::AddVar(int i, unsigned int card) {
    if (card > 0 && find(ordering.begin(), ordering.end(), i) == ordering.end()) {
        size_t j = varCard.size();
        if (i >= int(j)) {
            varCard.resize(i+1);
            for (; j < varCard.size(); ++j) {
                varCard[j] = 0;
            }
        }
        varCard[i] = card;
        ordering.push_back(i);
        return true;
    }
    return false;
}

inline bool Scope::RemoveVar(int i) {
    if (find(ordering.begin(), ordering.end(), i) != ordering.end()) {
        varCard[i] = 0;
        ordering.remove(i);
        return true;
    }
    return false;
}

inline void Scope::Clear() {
    varCard.clear();
    ordering.clear();
}

inline bool Scope::VarExists(int i) const {
    return find(ordering.begin(), ordering.end(), i) != ordering.end();
}

inline unsigned int Scope::GetNumVars() const {
    return ordering.size();
}

inline unsigned int Scope::GetVarCard(int i) const {
    /*
    map<int, unsigned int>::const_iterator it =
            varCard.find(i);
    if (it != varCard.end())
        return it->second;
    return 0;
    */
    return varCard[i];
}

inline unsigned long Scope::GetCard() const {
    unsigned long totalCard = 1;

    list<int>::const_iterator it =
            ordering.begin();
    for (; it != ordering.end(); ++it) {
        totalCard *= varCard[*it];
    }
    return totalCard;
}

inline unsigned long Scope::GetLogCard() const {
    unsigned long totalCard = 0;

    list<int>::const_iterator it =
            ordering.begin();
    for (; it != ordering.end(); ++it) {
        totalCard += log(varCard[*it]);
    }
    return totalCard;
}

map<int, unsigned int> Scope::GetCardExp() const {
    map<int, unsigned int> cardExp;

    list<int>::const_iterator it =
            ordering.begin();
    for (; it != ordering.end(); ++it) {
        ++cardExp[varCard[*it]];
    }
    return cardExp;
}



bool Scope::HasConsistentCard(const Scope &rhs) const {
    list<int>::const_iterator it = ordering.begin();
    for (; it != ordering.end(); ++it) {
        if (rhs.VarExists(*it) && rhs.GetVarCard(*it) != varCard[*it])
            return false;
    }
    it = rhs.ordering.begin();
    for (; it != rhs.ordering.end(); ++it) {
        if (VarExists(*it) && GetVarCard(*it) != rhs.varCard[*it])
            return false;
    }
    return true;
}

const list<int> &Scope::GetOrdering() const {
    return ordering;
}

void Scope::SetOrdering(const list<int> &newOrdering) {
    list<int> tempOrdering;
    list<int>::const_iterator it = newOrdering.begin();
    for (; it != newOrdering.end(); ++it) {
        if (VarExists(*it))
            tempOrdering.push_back(*it);
    }
    if (tempOrdering.size() != ordering.size()) {
        cerr<< "Inconsistent ordering" << endl;
        exit(0);
    }
    ordering = tempOrdering;
}

Scope Scope::operator+(const Scope &rhs) {
    return Scope(*this, rhs, Scope::UNION);
}

Scope Scope::operator*(const Scope &rhs) {
    return Scope(*this, rhs, Scope::INTERSECT);
}

Scope Scope::operator-(const Scope &rhs) {
    return Scope(*this, rhs, Scope::DIFF);
}

Scope& Scope::operator=(const Scope &s) {
    if (this != &s) {
        varCard = s.varCard;
        ordering = s.ordering;
    }
    return *this;
}

void Scope::Save(ostream &out) const {
    out << ordering.size();
    list<int>::const_iterator it = ordering.begin();
    for (; it != ordering.end(); ++it)
        out << " " << *it << " " << varCard[*it];

}

// Assignment class

Assignment::Assignment() {
}

Assignment::~Assignment() {
    varAssigns.clear();
}

Assignment::Assignment(const Scope &s) :
    Scope(s), varAssigns(vector<int>(varCard.size(), UNKNOWN_VAL)) {
}

Assignment::Assignment(const Assignment &s) :
    Scope(s), varAssigns(s.varAssigns) {
}

Assignment::Assignment(const Assignment &lhs, const Assignment &rhs, oper op) :
    Scope(lhs, rhs, op) {
    list<int>::iterator it = ordering.begin();

    /*
    for (; it != varCard.end(); it++)
        if (it->first > varAssigns.size()) varAssigns.resize(it->first+1);
//        varAssigns.insert(make_pair<int, int> (it->first, -1));
    for (unsigned int i = 0; i < varAssigns.size(); ++i)
        varAssigns[i] = UNKNOWN_VAL;
        */

    list<int>::const_iterator rhsit = rhs.ordering.begin();

    /*
    for (; lhsit != lhs.varAssigns.end(); ++lhsit)
        SetVal(lhsit->first, lhsit->second);
     */
    varAssigns.insert(varAssigns.begin(), lhs.varAssigns.begin(), lhs.varAssigns.end());

    // Only UNION and INTERSECT needs to consider rhs assignments
    if (op == Scope::UNION || op == Scope::INTERSECT) {
        for (; rhsit != rhs.ordering.end(); ++rhsit) {
            if (VarExists(*rhsit)) {
                if (varAssigns[*rhsit] > UNKNOWN_VAL)
                    UnsetVal(*rhsit);
                else
                    SetVal(*rhsit, rhs.varAssigns[*rhsit]);
            }
        }
    }
}

inline bool Assignment::AddVar(int i, unsigned int val) {
    if (Scope::AddVar(i, val)) {
        if (i >= int(varAssigns.size())) {
            size_t j = varAssigns.size();
            varAssigns.resize(i+1);
            for ( ;j < varAssigns.size(); ++j) {
                varAssigns[i] = UNKNOWN_VAL;
            }
        }
        return true;
    }
    return false;
}

inline bool Assignment::RemoveVar(int i) {
    if (Scope::RemoveVar(i)) {
        varAssigns[i] = -1;
        return true;
        /*
        map<int, int>::iterator it = varAssigns.find(i);
        if (it != varAssigns.end()) {
            varAssigns.erase(it);
            return true;
        }
        */
    }
    return false;
}

inline bool Assignment::SetVal(int i, unsigned int val) {
    /*
    map<int, int>::iterator it = varAssigns.find(i);
    if (it != varAssigns.end() && val < GetVarCard(i)) {
        it->second = val;
        return true;
    }
    return false;
    */
    varAssigns[i] = val;
    return true;
}

inline void Assignment::SetAllVal(unsigned int val) {
    list<int>::iterator it = ordering.begin();
    for (; it != ordering.end(); ++it) {
        SetVal(*it, val);
    }
}

inline void Assignment::SetAssign(const Assignment &a) {
//    const boost::unordered_map<int, int> &rhsAssigns = a.varAssigns;
    list<int>::const_iterator it = ordering.begin();
    for (; it != ordering.end(); ++it) {
        int val = a.GetVal(*it);
        if (val != ERROR_VAL)
            SetVal(*it, val);
    }
}

inline bool Assignment::UnsetVal(int i) {
    /*
    map<int, int>::iterator it = varAssigns.find(i);
    if (it == varAssigns.end())
        return false;
    it->second = UNKNOWN_VAL;
    */
    varAssigns[i] = UNKNOWN_VAL;
    return true;
}

inline bool Assignment::UnsetAllVal() {
    /*
    map<int, int>::iterator it = varAssigns.begin();
    for (; it != varAssigns.end(); ++it) {
        it->second = UNKNOWN_VAL;
    }
    */
    for (unsigned int i = 0; i < varAssigns.size(); ++i) {
        varAssigns[i] = UNKNOWN_VAL;
    }
    return true;
}

inline int Assignment::GetVal(int i) const {
    /*
    map<int, int>::const_iterator it = varAssigns.find(i);
    if (it == varAssigns.end())
        return ERROR_VAL;
        */
    if (i >= int(varAssigns.size())) return ERROR_VAL;
    return varAssigns[i];
}

inline int Assignment::GetIndex() const {
    int idx = 0;
    unsigned int totalCard = GetCard();
    map<int, int>::const_iterator it;
    list<int>::const_iterator listIt = ordering.begin();
    for (; listIt != ordering.end(); ++listIt) {
        unsigned int card = GetVarCard(*listIt);
        assert(card > 0);
        totalCard /= card;
        /*
        it = varAssigns.find(*listIt);
        if (it == varAssigns.end() || it->second == UNKNOWN_VAL) {
            return -1;
        }
        */
        idx += totalCard * varAssigns[*listIt];
    }
    return idx;
}

inline int Assignment::GetIndex(const list<int> &otherOrder) const {
    int idx = 0;
    unsigned int totalCard = GetCard();
    map<int, int>::const_iterator it;
    list<int>::const_iterator listIt = otherOrder.begin();
    for (; listIt != otherOrder.end(); ++listIt) {
        unsigned int card = GetVarCard(*listIt);
        assert(card > 0);
        totalCard /= card;
        /*
        it = varAssigns.find(*listIt);
        if (it == varAssigns.end() || it->second == UNKNOWN_VAL) {
            return -1;
        }
        */
        idx += totalCard * varAssigns[*listIt];
    }
    return idx;
}

bool Assignment::Iterate() {
    map<int, int>::iterator it;
    list<int>::reverse_iterator rit = ordering.rbegin();
    for (; rit != ordering.rend(); ++rit) {
        int val = GetVal(*rit);
        assert(val != ERROR_VAL);
        if (val == UNKNOWN_VAL)
            return false;
        if (++val < (int) GetVarCard(*rit)) {
            SetVal(*rit, val);
            return true;
        }
        val = 0;
        SetVal(*rit, val);
    }
    return false;
}

void Assignment::SetOrdering(const list<int> &newOrdering) {
    Scope::SetOrdering(newOrdering);
    UnsetAllVal();
}

Assignment Assignment::operator+(const Assignment &rhs) {
    return Assignment(*this, rhs, Scope::UNION);
}

Assignment Assignment::operator*(const Assignment &rhs) {
    return Assignment(*this, rhs, Scope::INTERSECT);
}

Assignment Assignment::operator-(const Assignment &rhs) {
    return Assignment(*this, rhs, Scope::DIFF);
}

Assignment& Assignment::operator=(const Assignment &a) {
    if (this != &a) {
        ordering = a.ordering;
        varCard = a.varCard;
        varAssigns = a.varAssigns;
    }
    return *this;
}

void Assignment::Save(ostream &out) const {
    Scope::Save(out);
    out << "  ";
    list<int>::const_iterator it = ordering.begin();
    for (; it != ordering.end(); ++it)
        out << " " << *it << " " << varAssigns[*it];
}

} // end of aomdd namespace
