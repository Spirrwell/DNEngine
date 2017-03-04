#ifndef ANIM_H
#define ANIM_H

#include "stdint.h"
#include "grp.h"

#include <string>

//From eduke32 code
typedef struct
{
    uint8_t* animbuf;
    void (*sound_func)(int32_t);
    uint16_t *sounds;
    int16_t numsounds;
    uint8_t framedelay;
    char animlock;
} dukeanim_t;

class ANMManager
{

};

class ANM
{

};

#endif