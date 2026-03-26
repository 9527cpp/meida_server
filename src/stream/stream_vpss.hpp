#ifndef __STREAM_VPSS_HPP__
#define __STREAM_VPSS_HPP__

#include "stream_base.hpp"
#include <string>

class stream_vpss : public stream_base {
public:
    stream_vpss(int chn);
    virtual ~stream_vpss() = default;

protected:
    void stream_data_loop() override;

private:
    int chn_;
};

#endif /* __STREAM_VPSS_HPP__ */
