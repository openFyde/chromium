// Copyright 2021 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// Author: Yang Tsao<yang@fydeos.io>

#ifndef MEDIA_FILTERS_RPI_VIDEO_DECODER_H_
#define MEDIA_FILTERS_RPI_VIDEO_DECODER_H_

#include "media/base/video_codecs.h"
#include "media/base/supported_video_decoder_config.h"
#include "media/filters/ffmpeg_video_decoder.h"

struct AVFrame;
namespace media {

class MEDIA_EXPORT RpiVideoDecoder : public FFmpegVideoDecoder {
public:
  static bool IsCodecSupported(VideoCodec codec);
  explicit RpiVideoDecoder(MediaLog* media_log)
    :FFmpegVideoDecoder(media_log) 
  {}

  bool IsPlatformDecoder() const override {  
    return true;
  }

  static SupportedVideoDecoderConfigs SupportedConfigsForWebRTC();

  VideoDecoderType GetDecoderType() const override {
    return VideoDecoderType::kRpi; 
  }
  
private:
  bool OnNewFrame(AVFrame* frame) override;
  bool FFmpegDecode(const DecoderBuffer& buffer) override;
  bool ConfigureDecoder(const VideoDecoderConfig& config, bool low_delay) override;

}; // RpiVideoDecoder

} // media

#endif //MEDIA_FILTERS_RPI_VIDEO_DECODER_H_
