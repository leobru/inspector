#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <algorithm>

const int Nsusps = 5;
const int Nrooms = 6;

struct Suspect {
    int susp;
    Suspect(int s = Nsusps) : susp(s) { } 
    Suspect (char c) {
        if ('A' <= c && c < 'A' + Nsusps)
            susp = c - 'A';
        else if('a' <= c && c < 'a' + Nsusps)
            susp = c - 'a';
        else
            susp = Nsusps;
    }

    operator int() const { return susp; }
    
    operator std::string() const {
        static const char * names[] = {
            "Andrew", "Bob", "Charles", "David", "Evan", "BUG"
        };
        return names[susp];
    }
};

struct Location {
    int loc;
    Location(int l = Nrooms) : loc(l) { }
    Location (char c) {
        switch (c) {
        case 'B': case 'b': loc = 0; break;
        case 'S': case 's': loc = 1; break;
        case 'L': case 'l': loc = 2; break;
        case 'D': case 'd': loc = 3; break;
        case 'K': case 'k': loc = 4; break;
        case 'G': case 'g': loc = 5; break;
        default: loc = Nrooms;
        }
    }

    operator int() const { return loc; }
    
    operator std::string() const {
        static const char * rooms[] = {
            "Bedroom", "Study", "Living room", "Dining room", "Kitchen", "Greenhouse", "BUG"
        };
        return rooms[loc];
    }
};

Location hostLoc;
int deathTime;
Suspect culprit;

std::vector<Suspect> house[10][Nrooms];

void whoElse(const std::vector<Suspect> & who, Suspect x) {
    bool comma = false;
    for(uint i = 0; i < who.size(); ++i) {
        if (who[i] == x) continue;
        if (comma) {
            std::cout << ',';
        }
        std::cout << std::string(who[i]);
        comma = true;             
    }
}

void InitHouse() {
    srandom(time(NULL));
    for (int susp = 0; susp < Nsusps; ++susp) {
        int hour = 1;
        int room = random() % Nrooms;
        while (hour <= 9) {
            house[hour][room].push_back(susp);
            int whereTo = (random() % 3) - 1;
            ++hour;
            switch (room) {
            case 0:
                whereTo = std::abs(whereTo);
                break;
            case Nrooms-1:
                whereTo = -std::abs(whereTo);
                break;
            }
            room += whereTo;
        }
    }
    deathTime = random() % 9 + 1;
    do {
        hostLoc = int(random() % Nrooms);
    } while (house[deathTime][hostLoc].empty());
    culprit = house[deathTime][hostLoc][random() % house[deathTime][hostLoc].size()];

    std::cout <<
        "There was a murder in a country house between 1am and 9am\n"
        "during a party. The party host has been murdered. You have to\n"
        "interrogate five guests: Andrew, Bob, Charles, David and Evan.\n"
        "The questions you may ask are ``At what time were you in a given room?''\n"
        "and ``Where were you at a given hour?''.\n"
        "The plan of the house is:\n"
        "Bedroom - Study - Living room - Dining room - Kitchen - Greenhouse\n"
        "The questions are entered as <letter><letter> for the first question\n"
        "using initial letters of the suspect name and the room, or\n"
        "<letter><digit> for the second question (the time is given as one digit 1-9).\n"
        "The host had stayed in the same room both alive and dead;\n"
        "the guests could move from a room to an adjacent room between the hours.\n"
        "The innocent will answer truthfully, the culprit could lie.\n"
        "An accusation can be entered as <letter><digit><letter>, mentioning the time\n"
        "and the room.\n";
#ifdef DEBUG
    for (int hr = 1; hr <= 9; ++hr) {
        std::cout << "Hour " << hr << '\n';
        for (int r = 0; r < Nrooms; ++r) {
            whoElse(house[hr][r], Nsusps);
            std::cout << '\t';
        }
        std::cout << '\n';
    }
#endif
}

Location where(Suspect s, int hr) {
    for (int r = 0; r < Nrooms; ++r) {
        if (std::find(house[hr][r].begin(), house[hr][r].end(), s) != house[hr][r].end())
            return r;
    }
    std::cout << "Location of " << std::string(s) << " at " << hr << " o'clock not found!\n";
    exit(1);
}

void respHour(Suspect s, int hr) {
    Location l = where(s, hr);
    std::cout << std::string(s) << ": At " << hr << " o'clock I was in the " << std::string(l) << ".\n";
    if (l == hostLoc) {
        if (hr < deathTime) {
            std::cout << "The host, still alive, was also there.\n";
        } else if (hr == deathTime) {
            std::cout << "During that time, the host suddenly died.\n";
        }
    }
    if (house[hr][l].size() > 1) {
        whoElse(house[hr][l], s);
        std::cout << (house[hr][l].size() == 2 ? " was " : " were ") << "in the same room.\n";
    }
    std::vector<Suspect> adj;
    if (l != 0) adj.insert(adj.end(), house[hr][l-1].begin(), house[hr][l-1].end());
    if (l != Nrooms-1) adj.insert(adj.end(), house[hr][l+1].begin(), house[hr][l+1].end());
    std::sort(adj.begin(), adj.end());
    if (hr < deathTime && (hostLoc == l-1 || hostLoc == l+1)) {
        std::cout << "The host, still alive, was in one of the adjacent rooms.\n";
    }
    if (adj.size() == 0) {
        std::cout << "There were no guests in the adjacent rooms.\n";
    } else {
        whoElse(adj, Nsusps);
        std::cout << (adj.size() == 1 ? " was " : " were ") << "in the adjacent rooms.\n";
    }
}

void respRoom (Suspect s, Location l) {
    std::vector<int> when;
    for (int hr = 1; hr <= 9; ++hr) {
        if (s == culprit && hr == deathTime) {
            // Lying
            continue;
        }
        if (std::find(house[hr][l].begin(), house[hr][l].end(), s) != house[hr][l].end())
            when.push_back(hr);
    }
    if (when.empty()) {
        std::cout << std::string(s) << ": I never was in the " << std::string(l) << '\n';
    } else {
        std::cout << std::string(s) << ": I was in the " << std::string(l) << " at ";
        for (uint i = 0; i < when.size(); ++i) {
            std::cout << when[i] << (i == when.size()-1 ? " o'clock" : ", ");
        }
        std::cout << '\n';
    }
}

main() {
    InitHouse();

    int q;
    for (q = 1; !std::cin.eof() && q <= 10; ++q) {
        std::string inp;
        std::cout << "Question " << q << ": ";
        std::cin >> inp;
        switch (inp.size()) {
        case 1:
            if (inp[0] == 'Q' || inp[0] == 'q') { q = 11; continue; }
            // FALLTHRU
        default:
            std::cout << "To interrogate: <letter><digit> or <letter><letter>;\n"
                "To accuse: <letter><digit><letter>;\n"
                "To exit: q\n";
            --q;
            break;
        case 2:
            if (q == 10) {
                std::cout << "Now you must enter an accusation!\n";
                --q;
                continue;
            }
            if (Suspect(inp[0]) == Nsusps ||
                (Location(inp[1]) == Nrooms && (inp[1] < '1' || inp[1] > '9'))) {
                std::cout << "Interrogation format: [A-E][1-9] or [A-E][BSLDKG]\n";
                --q;
            } else if (Location(inp[1]) == Nrooms) {
                respHour(inp[0], inp[1] - '0');
            } else {
                respRoom(inp[0], inp[1]);
            }
            break;
        case 3:
            if (Suspect(inp[0]) == Nsusps ||
                Location(inp[2]) == Nrooms ||
                (inp[1] < '1' || inp[1] > '9')) {
                std::cout << "Accusation format: [A-E][1-9][BSLDKG]\n";
                --q;
                continue;
            }
            if (Suspect(inp[0]) == culprit &&
                inp[1] - '0' == deathTime &&
                Location(inp[2]) == hostLoc) {
                std::cout << "With your help, the murderer was tried and convicted!\n";
                exit(0);
            } else {
                std::cout << "Nope!\n";
            }
        }
    }
    std::cout << "You failed to find the culprit!\n";
    std::cout << "The host was murdered in the " << std::string(hostLoc) << " at "
              <<  deathTime << " o'clock by " << std::string(Suspect(culprit)) << ".\n";

}
