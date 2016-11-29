#include "PersistentParams.h"

#include <ctype.h>

#include "cinder/Filesystem.h"
#include <boost/foreach.hpp>

std::string PersistentParams::name2id( const std::string& name ) {
    std::string id = "";
    enum State { START, APPEND, UPCASE };
    State state(START);

    BOOST_FOREACH(char c, name) {
        switch(state) {
            case START:
                if (isalpha(c)) {
                    id += c;
                    state = APPEND;
                } else if (isdigit(c)) {
                    id = "_" + c;
                    state = APPEND;
                }
                break;
            case APPEND:
                if (isalnum(c)) {
                    id += c;
                } else {
                    state = UPCASE;
                }
                break;
            case UPCASE:
                if (islower(c)) {
                    id += toupper(c);
                    state = APPEND;
                } else if (isalnum(c)) {
                    id += c;
                    state = APPEND;
                }
                break;
        }
    }
    return id;
}

void PersistentParams::load(const std::string& fname)
{
    filename() = fname;
    if (fs::exists( fname )) {
        root() = XmlTree( loadFile(fname) );
    }
}

void PersistentParams::save() {
    BOOST_FOREACH(boost::function<void()> f, persistCallbacks())
        f();
    root().write( writeFile(filename()) );
}
