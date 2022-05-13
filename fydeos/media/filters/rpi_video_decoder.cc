// Copyright 2021 The FydeOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// Author: Yang Tsao<yang@fydeos.io>

#include "fydeos/media/filters/rpi_video_decoder.h"
#include "media/ffmpeg/ffmpeg_common.h"
#include "media/base/video_frame.h"
#include "media/ffmpeg/ffmpeg_decoding_loop.h"
#include "media/base/bind_to_current_loop.h"
#include "media/base/media_log.h"

namespace media {
namespace {
  const char kMMAL_H264[] = "h264_mmal";
  const char kMMAL_MPEG2[] = "mpeg2_mmal";
  const char kMMAL_MPEG4[] = "mpeg4_mmal";
  const char kMMAL_VC1[] = "vc1_mmal";
  const char kMMAL_vp8[] = "vp8_mmal";
  
const char * getDecoderName(AVCodecID id) {
    switch (id) {
			case AV_CODEC_ID_H264:
				return kMMAL_H264;
      case AV_CODEC_ID_MPEG2VIDEO:
        return kMMAL_MPEG2;
      case AV_CODEC_ID_MPEG4:
        return kMMAL_MPEG4;
      case AV_CODEC_ID_VC1:
        return kMMAL_VC1;
      case AV_CODEC_ID_VP8:
        return kMMAL_vp8;
      default:
        return kMMAL_H264;
    } 
}

static int GetVideoBufferImpl(struct AVCodecContext* s,
                              AVFrame* frame,
                              int flags) {
  RpiVideoDecoder* decoder = static_cast<RpiVideoDecoder*>(s->opaque);
  return decoder->GetVideoBuffer(s, frame, flags);
}

} // no spacename

SupportedVideoDecoderConfigs RpiVideoDecoder::SupportedConfigsForWebRTC() {
  SupportedVideoDecoderConfigs supported_configs;
  //H264
  supported_configs.emplace_back(/*profile_min=*/H264PROFILE_BASELINE,
                                   /*profile_max=*/H264PROFILE_MAX,
                                   /*coded_size_min=*/kDefaultSwDecodeSizeMin,
                                   /*coded_size_max=*/kDefaultSwDecodeSizeMax,
                                   /*allow_encrypted=*/false,
                                   /*require_encrypted=*/false);
  //VP8
  supported_configs.emplace_back(/*profile_min=*/VP8PROFILE_ANY,
                                   /*profile_max=*/VP8PROFILE_ANY,
                                   /*coded_size_min=*/kDefaultSwDecodeSizeMin,
                                   /*coded_size_max=*/kDefaultSwDecodeSizeMax,
                                   /*allow_encrypted=*/false,
                                   /*require_encrypted=*/false);
  //MPEG2
  supported_configs.emplace_back(/*profile_min=*/VP8PROFILE_ANY,
                                   /*profile_max=*/VP8PROFILE_ANY,
                                   /*coded_size_min=*/kDefaultSwDecodeSizeMin,
                                   /*coded_size_max=*/kDefaultSwDecodeSizeMax,
                                   /*allow_encrypted=*/false,
                                   /*require_encrypted=*/false);
  //OTHERS
  supported_configs.emplace_back(/*profile_min=*/VIDEO_CODEC_PROFILE_UNKNOWN,
                                   /*profile_max=*/VIDEO_CODEC_PROFILE_UNKNOWN,
                                   /*coded_size_min=*/kDefaultSwDecodeSizeMin,
                                   /*coded_size_max=*/kDefaultSwDecodeSizeMax,
                                   /*allow_encrypted=*/false,
                                   /*require_encrypted=*/false);
  return supported_configs;
}

bool RpiVideoDecoder::IsCodecSupported(VideoCodec codec) {
  switch (codec) {
    case VideoCodec::kH264:
    case VideoCodec::kVP8:
    case VideoCodec::kVC1:
    case VideoCodec::kMPEG2:
    case VideoCodec::kMPEG4:
      return true;
    default:
      return false;
  }
}

bool RpiVideoDecoder::OnNewFrame(AVFrame* frame) {
  // TODO(fbarchard): Work around for FFmpeg http://crbug.com/27675
  // The decoder is in a bad state and not decoding correctly.
  // Checking for NULL avoids a crash in CopyPlane().
  if (!frame->data[VideoFrame::kYPlane] || !frame->data[VideoFrame::kUPlane] ||
      !frame->data[VideoFrame::kVPlane]) {
    DLOG(ERROR) << "Video frame was produced yet has invalid frame data.";
    return false;
  }

  scoped_refptr<VideoFrame> video_frame =
      reinterpret_cast<VideoFrame*>(av_buffer_get_opaque(frame->buf[0]));
  video_frame->set_timestamp(
      base::TimeDelta::FromMicroseconds(frame->pts));
  video_frame->metadata().power_efficient = true;
  video_frame->metadata().allow_overlay = true;
  output_cb_.Run(video_frame);
  return true;
}

bool RpiVideoDecoder::FFmpegDecode(const DecoderBuffer& buffer) {
  // Create a packet for input data.
  // Due to FFmpeg API changes we no longer have const read-only pointers.
  VLOG(3) << __func__;
  AVPacket packet;
  av_init_packet(&packet);
  if (buffer.end_of_stream()) {
    packet.data = NULL;
    packet.size = 0;
  } else {
    packet.data = const_cast<uint8_t*>(buffer.data());
    packet.size = buffer.data_size();
    packet.pts = buffer.timestamp().InMicroseconds();

    DCHECK(packet.data);
    DCHECK_GT(packet.size, 0);

    // Let FFmpeg handle presentation timestamp reordering.
    codec_context_->reordered_opaque = buffer.timestamp().InMicroseconds();
  }

  switch (decoding_loop_->DecodePacket(
      &packet, base::BindRepeating(&RpiVideoDecoder::OnNewFrame,
                                   base::Unretained(this)))) {
    case FFmpegDecodingLoop::DecodeStatus::kSendPacketFailed:
      MEDIA_LOG(ERROR, media_log_)
          << "Failed to send video packet for decoding: "
          << buffer.AsHumanReadableString();
      return false;
    case FFmpegDecodingLoop::DecodeStatus::kFrameProcessingFailed:
      // OnNewFrame() should have already issued a MEDIA_LOG for this.
      return false;
    case FFmpegDecodingLoop::DecodeStatus::kDecodeFrameFailed:
      MEDIA_LOG(DEBUG, media_log_)
          << GetDecoderType() << " failed to decode a video frame: "
          << AVErrorToString(decoding_loop_->last_averror_code()) << ", at "
          << buffer.AsHumanReadableString();
      return false;
    case FFmpegDecodingLoop::DecodeStatus::kOkay:
      break;
  }

  return true;
}

bool RpiVideoDecoder::ConfigureDecoder(const VideoDecoderConfig& config,
                                       bool low_delay) {
  DCHECK(config.IsValidConfig());
  DCHECK(!config.is_encrypted());
  VLOG(2) << __func__;
  // Release existing decoder resources if necessary.
  if (codec_context_ && 
      codec_context_->codec_id == VideoCodecToCodecID(config.codec())) {
     avcodec_flush_buffers(codec_context_.get());
     return true;
  }

  ReleaseFFmpegResources();

  // Initialize AVCodecContext structure.
  codec_context_.reset(avcodec_alloc_context3(NULL));
  VideoDecoderConfigToAVCodecContext(config, codec_context_.get());

  codec_context_->thread_count = 1;
  codec_context_->thread_type = 0;
  codec_context_->opaque = this;
  codec_context_->get_buffer2 = GetVideoBufferImpl;

  if (decode_nalus_)
    codec_context_->flags2 |= AV_CODEC_FLAG2_CHUNKS;

  const AVCodec* codec = avcodec_find_decoder_by_name(
            getDecoderName(codec_context_->codec_id));
  if (!codec || avcodec_open2(codec_context_.get(), codec, NULL) < 0) {
    ReleaseFFmpegResources();
    return false;
  }

  decoding_loop_ = std::make_unique<FFmpegDecodingLoop>(codec_context_.get());
  return true;
}
} //media
