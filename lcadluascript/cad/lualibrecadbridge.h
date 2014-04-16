
#ifndef LUALIBRECADBRIDGE_H
#define LUALIBRECADBRIDGE_H

#include <cad/operations/create.h>
#include <cad/operations/rotate.h>
#include <cad/document/abstractdocument.h>


struct lua_State;
void lua_openlckernel(lua_State*L);


class LuaLcOperationCreate : public lc::operation::Create
{
public:
    LuaLcOperationCreate(lc::AbstractDocument* document, const QString& layerName) : lc::operation::Create(document, layerName) {
    }

    void append(shared_ptr<lc::CADEntity> cadEntity) {
        lc::operation::Create::append(cadEntity);
    }
};

class LuaLcOperationRotate : public lc::operation::Rotate
{
public:
    LuaLcOperationRotate(lc::AbstractDocument* document, const lc::geo::Coordinate& rotation_center, const double rotation_angle, const long no_copies) : lc::operation::Rotate(document, rotation_center, rotation_angle, no_copies) {
    }

    void append(shared_ptr<lc::CADEntity> cadEntity) {
        lc::operation::Rotate::append(cadEntity);
    }
};

#endif // LUALIBRECADBRIDGE_H
