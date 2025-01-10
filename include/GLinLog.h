/*
  GLinlog Plugin for Nuke
  ------------------------------
  Copyright (c) 2025 Gonzalo Rojas
  This plugin is free to use, modify, and distribute.
  Provided "as is" without any warranty.
*/

#ifndef GLINLOG_H
#define GLINLOG_H

#include <DDImage/Channel.h>
#include <DDImage/PixelIop.h>
#include <DDImage/NukeWrapper.h>
#include <DDImage/Row.h>
#include <DDImage/Knobs.h>

using namespace DD::Image;

class GLinlogIop : public PixelIop
{
  int modeindex;
  float _blackpoint[3];
  float _whitepoint[3];
  float _gamma[3];
  float _gain[3];
  float _offset[3];

public:
  static const char *modes[];

  GLinlogIop(Node *node);

  ~GLinlogIop() override;

  static const DD::Image::Op::Description description;

  const char *Class() const override;

  const char *displayName() const override;

  const char *node_help() const override;

  void knobs(Knob_Callback f) override;

  void in_channels(int n, ChannelSet &mask) const override;

  void pixel_engine(
      const Row &in,
      int rowY, int rowX, int rowXBound,
      ChannelMask outputChannels,
      Row &out) override;

  void _validate(bool for_real) override;
};

static DD::Image::Op* build(Node *node);

#endif // GLINLOG_H