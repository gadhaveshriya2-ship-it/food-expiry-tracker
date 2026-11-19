#pragma once

#include <cstddef>

enum class RemovalDisposition { Consumed, Wasted };

class SessionStatistics {
public:
    void record(RemovalDisposition disposition) noexcept {
        if (disposition == RemovalDisposition::Consumed) ++consumed_;
        else ++wasted_;
    }
    std::size_t consumed() const noexcept { return consumed_; }
    std::size_t wasted() const noexcept { return wasted_; }

private:
    std::size_t consumed_ = 0;
    std::size_t wasted_ = 0;
};
