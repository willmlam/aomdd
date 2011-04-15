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
        throw GenericException("Inconsistent scopes");
    } else {
        *this = lhs;

        boost::unordered_map<int, unsigned int>::const_iterator it = rhs.varCard.begin();
        boost::unordered_map<int, unsigned int>::const_iterator lit = lhs.varCard.begin();
        switch (op) {
            case Scope::UNION:
                for (; it != rhs.varCard.end(); ++it)
                    AddVar(it->first, it->second);
                break;
            case Scope::INTERSECT:
                for (; lit != lhs.varCard.end(); ++lit) {
                    int i = lit->first;
                    if (!rhs.VarExists(i))
                        RemoveVar(i);
                }
                break;
            case Scope::DIFF:
                for (; it != rhs.varCard.end(); ++it)
                    RemoveVar(it->first);
                break;
            default:
                // Should never get here.
                assert(true);
        }
    }
}

Scope::~Scope() {
    varCard.clear();
}

inline bool Scope::AddVar(int i, unsigned int card) {
    if (card > 0 && varCard.find(i) == varCard.end()) {
        varCard.insert(make_pair<int, unsigned int> (i, card));
        ordering.push_back(i);
        return true;
    }
    return false;
}

inline bool Scope::RemoveVar(int i) {
    boost::unordered_map<int, unsigned int>::iterator it = varCard.find(i);
    if (it != varCard.end()) {
        varCard.erase(it);
        ordering.remove(i);
        return true;
    }
    return false;
}

inline bool Scope::VarExists(int i) const {
    return varCard.find(i) != varCard.end();
}

inline unsigned int Scope::GetVarCard(int i) const {
    boost::unordered_map<int, unsigned int>::const_iterator it = varCard.find(i);
    if (it != varCard.end())
        return it->second;
    return 0;
}

inline unsigned int Scope::GetCard() const {
    unsigned int totalCard = 1;

    boost::unordered_map<int, unsigned int>::const_iterator it = varCard.begin();
    for (; it != varCard.end(); ++it) {
        totalCard *= it->second;
    }
    return totalCard;
}

bool Scope::HasConsistentCard(const Scope &rhs) const {
    boost::unordered_map<int, unsigned int>::const_iterator it = varCard.begin();
    for (; it != varCard.end(); ++it) {
        int i = it->first;
        unsigned int card = it->second;
        if (rhs.VarExists(it->first) && rhs.GetVarCard(i) != card)
            return false;
    }
    it = rhs.varCard.begin();
    for (; it != rhs.varCard.end(); ++it) {
        int i = it->first;
        unsigned int card = it->second;
        if (VarExists(it->first) && GetVarCard(i) != card)
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
    if (tempOrdering.size() != ordering.size())
        throw GenericException("Inconsistent ordering");
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
    out << varCard.size();
    boost::unordered_map<int, unsigned int>::const_iterator it = varCard.begin();
    for (; it != varCard.end(); ++it)
        out << " " << it->first << " " << it->second;
    out << "  " << ordering.size();
    list<int>::const_iterator oit = ordering.begin();
    for (; oit != ordering.end(); ++oit)
        out << " " << *oit;

}

// Assignment class

Assignment::Assignment() {
}

Assignment::Assignment(const Scope &s) :
    Scope(s) {
    boost::unordered_map<int, unsigned int>::iterator it = varCard.begin();
    for (; it != varCard.end(); it++)
        varAssigns.insert(make_pair<int, int> (it->first, -1));
}

Assignment::Assignment(const Assignment &s) :
    Scope(s), varAssigns(s.varAssigns) {
}

Assignment::Assignment(const Assignment &lhs, const Assignment &rhs, oper op) :
    Scope(lhs, rhs, op) {
    boost::unordered_map<int, unsigned int>::iterator it = varCard.begin();
    for (; it != varCard.end(); it++)
        varAssigns.insert(make_pair<int, int> (it->first, -1));

    boost::unordered_map<int, int>::const_iterator lhsit = lhs.varAssigns.begin();
    boost::unordered_map<int, int>::const_iterator rhsit = rhs.varAssigns.begin();

    for (; lhsit != lhs.varAssigns.end(); ++lhsit)
        SetVal(lhsit->first, lhsit->second);

    // Only UNION and INTERSECT needs to consider rhs assignments
    if (op == Scope::UNION || op == Scope::INTERSECT) {
        for (; rhsit != rhs.varAssigns.end(); ++rhsit) {
            int var = rhsit->first;
            if (VarExists(var)) {
                if (GetVal(var) > UNKNOWN_VAL)
                    UnsetVal(var);
                else
                    SetVal(rhsit->first, rhsit->second);
            }
        }
    }
}

inline bool Assignment::AddVar(int i, unsigned int val) {
    if (Scope::AddVar(i, val)) {
        varAssigns.insert(make_pair<int, int> (i, -1));
        return true;
    }
    return false;
}

inline bool Assignment::RemoveVar(int i) {
    if (Scope::RemoveVar(i)) {
        boost::unordered_map<int, int>::iterator it = varAssigns.find(i);
        if (it != varAssigns.end()) {
            varAssigns.erase(it);
            return true;
        }
    }
    return false;
}

inline bool Assignment::SetVal(int i, unsigned int val) {
    boost::unordered_map<int, int>::iterator it = varAssigns.find(i);
    if (it != varAssigns.end() && val < GetVarCard(i)) {
        it->second = val;
        return true;
    }
    return false;
}

inline void Assignment::SetAllVal(unsigned int val) {
    boost::unordered_map<int, int>::iterator it = varAssigns.begin();
    for (; it != varAssigns.end(); ++it) {
        it->second = val;
    }
}

inline void Assignment::SetAssign(const Assignment &a) {
    const boost::unordered_map<int, int> &rhsAssigns = a.varAssigns;
    boost::unordered_map<int, int>::const_iterator it = varAssigns.begin();
    for (; it != varAssigns.end(); ++it) {
        int var = it->first;
        int val = a.GetVal(var);
        if (val != ERROR_VAL)
            SetVal(var, val);
    }
}

inline bool Assignment::UnsetVal(int i) {
    boost::unordered_map<int, int>::iterator it = varAssigns.find(i);
    if (it == varAssigns.end())
        return false;
    it->second = UNKNOWN_VAL;
    return true;
}

inline bool Assignment::UnsetAllVal() {
    boost::unordered_map<int, int>::iterator it = varAssigns.begin();
    for (; it != varAssigns.end(); ++it) {
        it->second = UNKNOWN_VAL;
    }
    return true;
}

inline int Assignment::GetVal(int i) const {
    boost::unordered_map<int, int>::const_iterator it = varAssigns.find(i);
    if (it == varAssigns.end())
        return ERROR_VAL;
    return it->second;
}

inline int Assignment::GetIndex() const {
    int idx = 0;
    unsigned int totalCard = GetCard();
    boost::unordered_map<int, int>::const_iterator it;
    list<int>::const_iterator listIt = ordering.begin();
    for (; listIt != ordering.end(); ++listIt) {
        unsigned int card = GetVarCard(*listIt);
        assert(card > 0);
        totalCard /= card;
        it = varAssigns.find(*listIt);
        if (it == varAssigns.end() || it->second == UNKNOWN_VAL) {
            return -1;
        }
        idx += totalCard * it->second;
    }
    return idx;
}

inline int Assignment::GetIndex(const list<int> &otherOrder) const {
    int idx = 0;
    unsigned int totalCard = GetCard();
    boost::unordered_map<int, int>::const_iterator it;
    list<int>::const_iterator listIt = otherOrder.begin();
    for (; listIt != otherOrder.end(); ++listIt) {
        unsigned int card = GetVarCard(*listIt);
        assert(card > 0);
        totalCard /= card;
        it = varAssigns.find(*listIt);
        if (it == varAssigns.end() || it->second == UNKNOWN_VAL) {
            return -1;
        }
        idx += totalCard * it->second;
    }
    return idx;
}

bool Assignment::Iterate() {
    boost::unordered_map<int, int>::iterator it;
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
        varCard = a.varCard;
        varAssigns = a.varAssigns;
    }
    return *this;
}

void Assignment::Save(ostream &out) const {
    Scope::Save(out);
    out << "  ";
    boost::unordered_map<int, int>::const_iterator it = varAssigns.begin();
    for (; it != varAssigns.end(); ++it)
        out << " " << it->first << " " << it->second;
}

} // end of aomdd namespace
