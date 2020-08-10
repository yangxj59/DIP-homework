#ifndef MDIVIEWCHILD_H
#define MDIVIEWCHILD_H

#include "mdichild.h"

class MdiViewChild : public MdiChild
{
public:
    MdiViewChild(MdiChild *owner);
    MdiChild *owner;
};

#endif // MDIVIEWCHILD_H
