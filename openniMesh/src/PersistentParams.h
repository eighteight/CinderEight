#pragma once
#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "cinder/params/Params.h"
#include "cinder/Xml.h"

using namespace ci;
using namespace ci::params;

class PersistentParams : public InterfaceGl {
public:
    /** Default constructor, doesn't wrap a window */
    PersistentParams() {}

    /** Create a persistent params window. */
    PersistentParams(const std::string &title,
                     const Vec2i &size,
                     const ColorA& color=ColorA(0.3f, 0.3f, 0.3f, 0.4f))
        : InterfaceGl(title, size, color), m_id(name2id(title)) {}

    /** Add a persistent parameter to the window.  Persistent parameter will be
     * initialized with saved value if found, oe with supplied default
     * otherwise
     */

    void addPersistentParam(const std::string& name, float* var, float defVal, const std::string& optionsStr="", bool readOnly=false)
    {
        addParam(name,var,optionsStr,readOnly);
        std::string id = name2id(name);
        *var = getXml().hasChild(id) ? getXml().getChild(id).getValue(defVal) : defVal;

        persistCallbacks().push_back(
                boost::bind( &PersistentParams::persistParam<float>, this, var, id ) );
    }

    /** Load persistent params from file. At the moment this only works when
     * called at application start up, before creating persistent parameteres.
     * Will remember the filename for saving later.
     */
    static void load(const std::string& path);

    /** Save persistent params (to the path passed to load before). */
    static void save();

protected:
    std::string m_id;

    // "manager"
    struct Manager {
        std::vector< boost::function< void() > > persistCallbacks;
        XmlTree root;
        std::string filename;

        Manager() {
            root = XmlTree::createDoc();
        }
    };

    static Manager& manager() {
        static Manager * m = new Manager();
        return *m;
    }

    static std::vector< boost::function< void() > >& persistCallbacks()
    {
        return manager().persistCallbacks;
    }
    static XmlTree& root()
    {
        return manager().root;
    }
    static std::string& filename()
    {
        return manager().filename;
    }

    // save current parameter value into an xml tree
    template<typename T>
    void persistParam(T * var, const std::string& paramId)
    {
        if (!getXml().hasChild(paramId))
            getXml().push_back(XmlTree(paramId,""));
        getXml().getChild(paramId).setValue(*var);
    }

    XmlTree& getXml() {
        if (!root().hasChild(m_id))
            root().push_back(XmlTree(m_id,""));
        return root().getChild(m_id);
    }


    static std::string name2id( const std::string& name );
};
