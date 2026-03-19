#ifndef __STREAM_USB_VIDEO_HPP__
#define __STREAM_USB_VIDEO_HPP__

/* USB 视频流占位：可扩展为 /dev/videoX 等 */
class stream_usb_video {
public:
    stream_usb_video() = default;
    int stream_start() { return 0; }
    int stream_stop() { return 0; }
};

#endif /* __STREAM_USB_VIDEO_HPP__ */
