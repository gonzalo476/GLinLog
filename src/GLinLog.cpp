/*
  GLinlog Plugin for Nuke
  ------------------------------
  Copyright (c) 2025 Gonzalo Rojas
  This plugin is free to use, modify, and distribute.
  Provided "as is" without any warranty.

  https://github.com/NatronGitHub/openfx-misc/blob/master/Log2Lin/Log2Lin.cpp
*/

#include "include/GLinLog.h"

#include <string>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <iostream>

#include <DDImage/Channel.h>
#include <DDImage/PixelIop.h>
#include <DDImage/NukeWrapper.h>
#include <DDImage/Row.h>
#include <DDImage/Knobs.h>

enum {
    LOGTOLIN = 0, LINTOLOG
};

const char* GLinlogIop::modes[] = {
    "Log2Lin", "Lin2Log", nullptr
};

GLinlogIop::GLinlogIop(Node *n) : PixelIop(n)
{
    modeindex = 1;

    for (int i=0; i<3; ++i) {
        _blackpoint[i] = 95.0f;
        _whitepoint[i] = 685.0f;
        _gamma[i] = 0.6f;
        _offset[i] = std::pow(10., (_blackpoint[i] - _whitepoint[i]) * 0.002 / _gamma[i]);
        _gain[i] = 1. / (1. - _offset[i]);
    }
    
}

GLinlogIop::~GLinlogIop()
{

}

void GLinlogIop::knobs(Knob_Callback f)
{
    Enumeration_knob(f, &modeindex, modes, "operation", "operation");
    Color_knob(f, _blackpoint, IRange(0, 1023), "black");
    SetFlags(f, Knob::STORE_INTEGER);
    Color_knob(f, _whitepoint, IRange(0, 1023), "white");
    SetFlags(f, Knob::STORE_INTEGER);
    Color_knob(f, _gamma, IRange(0, 1), "gamma");
}

void GLinlogIop::_validate(bool for_real)
{
    copy_info(0);
    set_out_channels(Mask_RGB);
}

void GLinlogIop::in_channels(int, ChannelSet& mask) const
{
    ChannelSet done;
    foreach(c, mask) {
        if (colourIndex(c) < 3 && !(done & c)) {
            done.addBrothers(c, 3);
        }
    }
    mask += done;
}

static inline float lin2log(float cIn, float gain, float offset, float gamma, float whitepoint)
{
    float cOut = (std::log10(cIn / gain + offset) / (0.002 / gamma) + whitepoint) / 1023.;

    return cOut;
}

void GLinlogIop::pixel_engine(
    const Row& in, 
    int rowY, 
    int rowX, 
    int rowXBound, 
    ChannelMask outputChannels, 
    Row& out)
{
    int rowWidth = rowXBound - rowX;

    ChannelSet done;

    for (int i=0; i<3; ++i) {
        _offset[i] = std::pow(10., (_blackpoint[i] - _whitepoint[i]) * 0.002 / _gamma[i]);
        _gain[i] = 1. / (1. - _offset[i]);
    }

    foreach (z, outputChannels) {
        
        if (done & z) {
            continue;
        }

        if (colourIndex(z) >= 3) {
            out.copy(in, z, rowX, rowXBound);
            continue;
        }

        Channel rChannel = brother(z, 0);
        Channel gChannel = brother(z, 1);
        Channel bChannel = brother(z, 2);

        done += rChannel;
        done += gChannel;
        done += bChannel;

        const float *rIn = in[rChannel] + rowX;
        const float *gIn = in[gChannel] + rowX;
        const float *bIn = in[bChannel] + rowX;

        float *rOut = out.writable(rChannel) + rowX;
        float *gOut = out.writable(gChannel) + rowX;
        float *bOut = out.writable(bChannel) + rowX;

        if (rOut != rIn) memcpy(rOut, rIn, sizeof(float)*rowWidth);
        if (gOut != gIn) memcpy(gOut, gIn, sizeof(float)*rowWidth);
        if (bOut != bIn) memcpy(bOut, bIn, sizeof(float)*rowWidth);

        const float* END = rIn + (rowXBound - rowX);

        switch (modeindex)
        {
        case LOGTOLIN:
            while (rIn < END) {
                *rOut++ = lin2log(*rIn++, _gain[0], _offset[0], _gamma[0], _whitepoint[0]);
                *gOut++ = lin2log(*gIn++, _gain[1], _offset[1], _gamma[1], _whitepoint[1]);
                *bOut++ = lin2log(*bIn++, _gain[2], _offset[2], _gamma[2], _whitepoint[2]);
            }
            break;
        case LINTOLOG:
            while (rIn < END) {
                *rOut++ = lin2log(*rIn++, _gain[0], _offset[0], _gamma[0], _whitepoint[0]);
                *gOut++ = lin2log(*gIn++, _gain[1], _offset[1], _gamma[1], _whitepoint[1]);
                *bOut++ = lin2log(*bIn++, _gain[2], _offset[2], _gamma[2], _whitepoint[2]);
            }
            break;
        default:
            break;
        }
    }
}

const Op::Description GLinlogIop::description("GLinLog", build);

const char* GLinlogIop::Class() const
{
    return description.name;
}

const char* GLinlogIop::displayName() const
{
    return description.name;
}

const char* GLinlogIop::node_help() const
{
    return "GLinLog v1.0";
}

Op* build(Node *node)
{
    NukeWrapper *op = new NukeWrapper(new GLinlogIop(node));
    op->channels(Mask_RGB);
    return op;
}