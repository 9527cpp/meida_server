#ifndef __STREAM_HDMI_AUDIO_HPP__
#define __STREAM_HDMI_AUDIO_HPP__

#include "stream_base.hpp"
#include "mpi_ctx/mpi_ctx_intf.h"
#include <vector>

class stream_hdmi_audio : public stream_base {
public:
    stream_hdmi_audio();
    ~stream_hdmi_audio();

    int start(int chn);
    int stop(int chn);

    void add_listener(int chn, stream_listener *listener);
    void remove_listener(int chn, stream_listener *listener);

    void add_process_stream(int chn, stream_base *stream);
    void add_output_stream(int chn, stream_base *stream);

protected:
    void stream_data_loop() override;

private:
    void init_streams();
    void deinit_streams();

    stream_base *input_{nullptr};
    std::vector<stream_base *> outputs_;
    std::vector<stream_base *> process_streams_;
};

#endif /* __STREAM_HDMI_AUDIO_HPP__ */
